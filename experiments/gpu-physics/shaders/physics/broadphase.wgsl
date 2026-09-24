// Uniform-grid hash, atomic pair append, bitonic sort, unique compact.

fn cell_hash(ix: i32, iy: i32, iz: i32) -> u32 {
    let x = u32(ix) * 73856093u;
    let y = u32(iy) * 19349663u;
    let z = u32(iz) * 83492791u;
    return (x ^ y ^ z) & (HASH_BUCKETS - 1u);
}

fn cell_coord(v: f32, cs: f32) -> i32 {
    return i32(floor(v / max(cs, 1e-4)));
}

fn body_extent(b: Body) -> vec3<f32> {
    return collider_aabb_extent(b);
}

fn pack_pair(ia: u32, ib: u32) -> vec2<u32> {
    let lo = min(ia, ib);
    let hi = max(ia, ib);
    return vec2<u32>(lo, hi);
}

fn broadphase_pair_allowed(a: Body, b: Body, shape_a: Shape, shape_b: Shape) -> bool {
    if (is_disabled(a) || is_disabled(b)) {
        return false;
    }
    let sensor_a = (shape_a.event_flags & SHAPE_IS_SENSOR) != 0u;
    let sensor_b = (shape_b.event_flags & SHAPE_IS_SENSOR) != 0u;
    if (sensor_a != sensor_b) {
        var sensor = shape_b;
        var visitor = shape_a;
        var visitor_body = a;
        if (sensor_a) {
            sensor = shape_a;
            visitor = shape_b;
            visitor_body = b;
        }
        return (sensor.event_flags & SHAPE_ENABLE_SENSOR_EVENTS) != 0u
            && (visitor.event_flags & SHAPE_ENABLE_SENSOR_EVENTS) != 0u
            && !is_static(visitor_body);
    }
    if (sensor_a) {
        return false;
    }
    if ((shape_a.kind == KIND_MESH && !is_non_dynamic(a))
        || (shape_b.kind == KIND_MESH && !is_non_dynamic(b))) {
        return false;
    }
    return params.enable_contacts != 0u && !(is_non_dynamic(a) && is_non_dynamic(b));
}

// Native child contacts are discovered against child geometry but retained
// while the outer public shape proxies overlap. The original child still owns
// narrowphase, its manifold chain, and the GPU contact generation.
fn pair_lifetime_overlap(a: Body, b: Body, shape_a: Shape, shape_b: Shape) -> bool {
    var bounds_a = a;
    var bounds_b = b;
    if ((shape_a.event_flags & SHAPE_COMPOUND_CHILD) != 0u) {
        bounds_a = load_collider(shape_a._pad_filter.x);
    }
    if ((shape_b.event_flags & SHAPE_COMPOUND_CHILD) != 0u) {
        bounds_b = load_collider(shape_b._pad_filter.x);
    }
    return aabb_overlap(bounds_a, bounds_b);
}

fn contact_has_sensor(c: Contact) -> bool {
    let ia = c.pair.x;
    let ib = c.pair.y;
    return ia < params.shape_count && ib < params.shape_count
        && ((load_shape(ia).event_flags | load_shape(ib).event_flags) & SHAPE_IS_SENSOR) != 0u;
}

fn joint_disables_collision_scan(body_a: u32, body_b: u32) -> bool {
    for (var i = 0u; i < params.joint_count; i++) {
        let joint = joints[i];
        if ((joint.a == body_a && joint.b == body_b)
            || (joint.a == body_b && joint.b == body_a)) {
            if ((joint.flags & JOINT_COLLIDE_CONNECTED) == 0u) {
                return true;
            }
        }
    }
    return false;
}

fn joint_disables_collision(body_a: u32, body_b: u32) -> bool {
    if (params.joint_count == 0u) {
        return false;
    }
    if ((params.diagnostic_flags & (DIAG_JOINT_FILTER_SCAN | DIAG_JOINT_FILTER_OVERFLOW)) != 0u) {
        return joint_disables_collision_scan(body_a, body_b);
    }
    let key_a = min(body_a, body_b);
    let key_b = max(body_a, body_b);
    var h = pair_hash_mix(pair_hash_mix(key_a) ^ key_b);
    let base = joint_filter_base();
    for (var probe = 0u; probe < JOINT_FILTER_PROBE; probe++) {
        let idx = h & (JOINT_FILTER_CAP - 1u);
        let stored = scratch[base + idx * 3u];
        if (stored == EMPTY) {
            return false;
        }
        if (stored == key_a && scratch[base + idx * 3u + 1u] == key_b) {
            return scratch[base + idx * 3u + 2u] != 0u;
        }
        h = h + 1u;
    }
    return joint_disables_collision_scan(body_a, body_b);
}

fn shapes_collide(a: Shape, b: Shape) -> bool {
    if (a.group_index == b.group_index && a.group_index != 0) {
        return a.group_index > 0;
    }
    let a_accepts_b =
        (a.mask_bits_lo & b.category_bits_lo) != 0u
        || (a.mask_bits_hi & b.category_bits_hi) != 0u;
    let b_accepts_a =
        (a.category_bits_lo & b.mask_bits_lo) != 0u
        || (a.category_bits_hi & b.mask_bits_hi) != 0u;
    return a_accepts_b && b_accepts_a;
}

fn append_pair(packed: vec2<u32>) {
    let i = atomicAdd(&atom[ATOM_PAIR_N], 1u);
    if (i < pair_cap()) {
        store_pair_words(SCR_PAIRS, i, packed);
    } else {
        record_capacity_drop(ATOM_PAIR_DROPPED, ATOM_STICKY_PAIR_DROPPED);
    }
}

fn append_new_pair(packed: vec2<u32>) {
    // A retained root emits its pair exactly once in emit_prev_pairs. The hash
    // is immutable throughout candidate generation and supports full identities.
    let previous = find_contact_slot(packed);
    if (previous != EMPTY) {
        if (all(previous_pair_key_for_slot(previous) == packed)) { return; }
    }
    append_pair(packed);
}

fn lower_cell(b: Body) -> vec3<i32> {
    return vec3<i32>(floor(fat_lower(b) / max(params.cell_size, 0.25)));
}

fn cell_span(b: Body, lower: vec3<i32>) -> vec3<u32> {
    let upper = vec3<i32>(floor(fat_upper(b) / max(params.cell_size, 0.25)));
    return vec3<u32>(upper - lower + vec3<i32>(1));
}

fn insertion_cell(b: Body, ordinal: u32) -> vec3<i32> {
    let lower = lower_cell(b);
    let span = cell_span(b, lower);
    return lower + vec3<i32>(i32(ordinal / (span.y * span.z)),
        i32((ordinal / span.z) % span.y), i32(ordinal % span.z));
}

fn insertion_ordinal(b: Body, lower: vec3<i32>, cell: vec3<i32>) -> u32 {
    let span = cell_span(b, lower);
    let offset = vec3<u32>(cell - lower);
    return (offset.x * span.y + offset.y) * span.z + offset.z;
}

// Eight words per allocated shape, after the existing scratch work arrays.
fn fat_lower(b: Body) -> vec3<f32> {
    let base = params.fat_bounds_base + 8u * b._pad_island.x;
    return bitcast<vec3<f32>>(vec3<u32>(scratch[base], scratch[base+1u], scratch[base+2u]));
}
fn fat_upper(b: Body) -> vec3<f32> {
    let base = params.fat_bounds_base + 8u * b._pad_island.x;
    return bitcast<vec3<f32>>(vec3<u32>(scratch[base+4u], scratch[base+5u], scratch[base+6u]));
}
fn update_fat_collider(i: u32, shape: Shape, b: Body) {
    let child = (shape.event_flags & SHAPE_COMPOUND_CHILD) != 0u;
    // Compound child trees contain raw child bounds; public proxies include
    // speculative distance and preserve their larger box until it is escaped.
    let tight_pad = select(SPECULATIVE, 0.0, child);
    let e = collider_aabb_extent(b) + vec3<f32>(tight_pad);
    let lo = b.pos - e;
    let hi = b.pos + e;
    let base = params.fat_bounds_base + 8u * i;
    let initialized = scratch[base+3u] == params.fat_bounds_epoch;
    if (!child && initialized && all(lo >= fat_lower(b)) && all(hi <= fat_upper(b))) { return; }
    var margin = min(MAX_AABB_MARGIN, AABB_MARGIN_FRAC * bound_radius(b));
    if (is_static(b) && !initialized) { margin = SPECULATIVE; }
    if (child) { margin = 0.0; }
    let lower = bitcast<vec3<u32>>(lo - vec3<f32>(margin));
    let upper = bitcast<vec3<u32>>(hi + vec3<f32>(margin));
    scratch[base] = lower.x; scratch[base+1u] = lower.y; scratch[base+2u] = lower.z;
    scratch[base+4u] = upper.x; scratch[base+5u] = upper.y; scratch[base+6u] = upper.z;
    scratch[base+3u] = params.fat_bounds_epoch;
}

fn update_fat_bounds(i: u32) {
    let shape = load_shape(i);
    let current = load_collider(i);
    let base = params.fat_bounds_base + 8u * i;
    if (params.fat_commands_count != 0u && scratch[base+7u] != params.fat_commands_epoch) {
        let heads = params.fat_commands_base + 2u * shape.body_index;
        let first = params.fat_commands_base + scratch[heads];
        let count = scratch[heads+1u];
        for (var j = 0u; j < count; j++) {
            let cmd = first + 8u * j;
            let origin = bitcast<vec3<f32>>(vec3<u32>(scratch[cmd], scratch[cmd+1u], scratch[cmd+2u]));
            let rotation = bitcast<vec4<f32>>(vec4<u32>(scratch[cmd+4u], scratch[cmd+5u], scratch[cmd+6u], scratch[cmd+7u]));
            var pose = current;
            pose.pos = origin + quat_rotate(rotation, shape.local_center);
            pose.rot = rotation;
            if (shape.kind == KIND_CAPSULE && dot(shape.axis, shape.axis) > 1e-12) {
                pose.rot = normalize(quat_mul(rotation, quat_from_x_axis(shape.axis)));
            }
            update_fat_collider(i, shape, pose);
        }
        scratch[base+7u] = params.fat_commands_epoch;
    }
    update_fat_collider(i, shape, current);
}

@compute @workgroup_size(64)
fn clear_broadphase(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    if (i < params.shape_count) { update_fat_bounds(i); }
    if (i < HASH_BUCKETS) {
        atomicStore(&atom[ATOM_HASH + i], EMPTY);
    }
    // Pair producers overwrite every live key; radix consumers are count-bounded.
    // Keep the dirty contact range monotonic: retired slots can still contain
    // previous-touching event keys that must be cleared on the following step.
    let dirty_end = min(atomicLoad(&query[QUERY_CONTACT_HIGH_WATER]), params.contact_capacity);
    if (i < dirty_end) {
        scratch[scr_contact_mark() + i] = 0u;
        // Latest-step event history must survive retirement and slot reuse.
        // Capture before narrowphase writes; child patches share their pair key.
        var previous_key = vec2<u32>(EMPTY);
        if (i < params.contact_capacity) {
            let old = contacts[i];
            if (old.a != EMPTY && old.count > 0u
                && (contact_persistent[i].lifecycle.y & CONTACT_TOUCHING) != 0u) {
                previous_key = contact_persistent[i].pair.xy;
            }
        }
        if (params.remap_history_step != params.physics_step) {
            store_pair_words(scr_previous_touching(), i, previous_key);
        }
    }
    if (i == 0u) {
        if ((params.diagnostic_flags & DIAG_MESH_CANDIDATES) != 0u) {
            atomicStore(&atom[arrayLength(&atom) - MESH_TRACE_WORDS], 0u);
        }
        atomicStore(&atom[ATOM_PAIR_N], 0u);
        atomicStore(&atom[ATOM_INSERT_N], 0u);
        atomicStore(&atom[ATOM_PAIR_DROPPED], 0u);
        atomicStore(&atom[ATOM_INSERT_DROPPED], 0u);
        atomicStore(&atom[ATOM_CONTACT_DROPPED], 0u);
        atomicStore(&atom[ATOM_HASH_HOP_DROPPED], 0u);
        if ((params.diagnostic_flags & DIAG_FORCE_CAPACITY_LOSS) != 0u) {
            record_capacity_drop(ATOM_PAIR_DROPPED, ATOM_STICKY_PAIR_DROPPED);
            record_capacity_drop(ATOM_INSERT_DROPPED, ATOM_STICKY_INSERT_DROPPED);
            record_contact_drop(6u);
            record_capacity_drop(ATOM_HASH_HOP_DROPPED, ATOM_STICKY_HASH_HOP_DROPPED);
        }
        atomicStore(&atom[ATOM_STATIC_N], 0u);
        scratch[SCR_PAIR_N] = 0u;
        scratch[SCR_INSERT_N] = 0u;
        scratch[SCR_UNIQUE_N] = 0u;
        scratch[SCR_NCONTACTS] = 0u;
        scratch[SCR_STATIC_N] = 0u;
    }
}

const MAX_STATIC_HASH_CELLS: u32 = 8u;
fn scr_static_list() -> u32 { return scr_radix_out(); }

fn shape_insert_count(b: Body) -> u32 {
    let cs = max(params.cell_size, 0.25);
    let lo = fat_lower(b);
    let hi = fat_upper(b);
    let x0 = cell_coord(lo.x, cs);
    let x1 = cell_coord(hi.x, cs);
    let y0 = cell_coord(lo.y, cs);
    let y1 = cell_coord(hi.y, cs);
    let z0 = cell_coord(lo.z, cs);
    let z1 = cell_coord(hi.z, cs);
    return u32(x1 - x0 + 1) * u32(y1 - y0 + 1) * u32(z1 - z0 + 1);
}

@compute @workgroup_size(64)
fn collect_fat_statics(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    if (i >= params.shape_count) {
        return;
    }
    if ((load_shape(i).event_flags & SHAPE_PUBLIC_PROXY) != 0u) { return; }
    let b = load_collider(i);
    if (!is_static(b)) {
        return;
    }
    if (shape_insert_count(b) <= MAX_STATIC_HASH_CELLS) {
        return;
    }
    let out = atomicAdd(&atom[ATOM_STATIC_N], 1u);
    if (out < pair_cap()) {
        scratch[scr_static_list() + out] = i;
    }
}

@compute @workgroup_size(1)
fn finish_fat_statics() {
    scratch[SCR_STATIC_N] = atomicLoad(&atom[ATOM_STATIC_N]);
}

@compute @workgroup_size(64)
fn hash_insert(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    if (i >= params.shape_count || params.enable_contacts == 0u) {
        return;
    }
    if ((load_shape(i).event_flags & SHAPE_PUBLIC_PROXY) != 0u) { return; }
    let b = load_collider(i);
    if (is_static(b) && shape_insert_count(b) > MAX_STATIC_HASH_CELLS) {
        return;
    }
    let cs = max(params.cell_size, 0.25);
    let lo = fat_lower(b);
    let hi = fat_upper(b);
    let x0 = cell_coord(lo.x, cs);
    let x1 = cell_coord(hi.x, cs);
    let y0 = cell_coord(lo.y, cs);
    let y1 = cell_coord(hi.y, cs);
    let z0 = cell_coord(lo.z, cs);
    let z1 = cell_coord(hi.z, cs);
    let insert_count = u32(x1 - x0 + 1) * u32(y1 - y0 + 1) * u32(z1 - z0 + 1);
    let first_slot = atomicAdd(&atom[ATOM_INSERT_N], insert_count);
    let accepted = min(insert_count, params.insert_capacity - min(first_slot, params.insert_capacity));
    if (accepted < insert_count) {
        record_capacity_drop_n(ATOM_INSERT_DROPPED, ATOM_STICKY_INSERT_DROPPED, insert_count - accepted);
    }
    var local_slot = 0u;
    for (var ix = x0; ix <= x1; ix++) {
        for (var iy = y0; iy <= y1; iy++) {
            for (var iz = z0; iz <= z1; iz++) {
                if (local_slot >= accepted) {
                    return;
                }
                let slot = first_slot + local_slot;
                local_slot = local_slot + 1u;
                let h = cell_hash(ix, iy, iz);
                scratch[scr_ins_body() + slot] = i;
                // Preserve exact cell identity, not just its colliding hash bucket.
                scratch[scr_ins_cell() + slot] = local_slot - 1u;
                let old = atomicExchange(&atom[ATOM_HASH + h], slot);
                scratch[scr_ins_next() + slot] = old;
            }
        }
    }
}

@compute @workgroup_size(1)
fn write_insert_indirect() {
    let n = min(atomicLoad(&atom[ATOM_INSERT_N]), params.insert_capacity);
    write_group_indirect(SCR_INDIRECT_COLLIDE, (n + 63u) / 64u);
}

@compute @workgroup_size(256)
fn write_radix_indirect(@builtin(local_invocation_index) lid: u32) {
    let n = min(atomicLoad(&atom[ATOM_PAIR_N]), pair_cap());
    if (lid == 0u) {
        scratch[SCR_PAIR_N] = n;
        let groups = max((n + RADIX_GROUP_SIZE - 1u) / RADIX_GROUP_SIZE, 1u);
        scratch[SCR_RADIX_GROUP_N] = groups;
        write_count_indirect(SCR_INDIRECT_RADIX, n, RADIX_GROUP_SIZE);
    }
    scratch[scr_radix_base() + lid] = 0u;
}

@compute @workgroup_size(64)
fn emit_hash_pairs(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let slot = gid.x;
    let nins = min(atomicLoad(&atom[ATOM_INSERT_N]), params.insert_capacity);
    if (slot >= nins) {
        return;
    }
    let ia = scratch[scr_ins_body() + slot];
    let a = load_collider(ia);
    let a_lower = lower_cell(a);
    let cell = insertion_cell(a, scratch[scr_ins_cell() + slot]);
    let h = cell_hash(cell.x, cell.y, cell.z);
    var other = atomicLoad(&atom[ATOM_HASH + h]);
    var hops = 0u;
    loop {
        if (other == EMPTY || hops >= 2048u) {
            if (other != EMPTY) {
                record_capacity_drop(ATOM_HASH_HOP_DROPPED, ATOM_STICKY_HASH_HOP_DROPPED);
            }
            break;
        }
        let ib = scratch[scr_ins_body() + other];
        if (ib < ia) {
            let b = load_collider(ib);
            let shape_a = load_shape(ia);
            let shape_b = load_shape(ib);
            let b_lower = lower_cell(b);
            let owner = max(a_lower, b_lower);
            if (all(cell == owner)
                && scratch[scr_ins_cell() + other] == insertion_ordinal(b, b_lower, owner)
                && a._pad_island.y != b._pad_island.y
                && broadphase_pair_allowed(a, b, shape_a, shape_b)
                && !joint_disables_collision(a._pad_island.y, b._pad_island.y)
                && shapes_collide(shape_a, shape_b)
                && aabb_overlap(a, b)) {
                append_new_pair(pack_pair(ia, ib));
            }
        }
        other = scratch[scr_ins_next() + other];
        hops = hops + 1u;
    }
}

@compute @workgroup_size(64)
fn emit_static_pairs(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    if (i >= params.shape_count) {
        return;
    }
    let a = load_collider(i);
    if (is_static(a)) {
        return;
    }
    let nstatic = min(scratch[SCR_STATIC_N], pair_cap());
    for (var j = 0u; j < nstatic; j++) {
        let jb = scratch[scr_static_list() + j];
        let b = load_collider(jb);
        if (b._pad_island.y == a._pad_island.y) {
            continue;
        }
        let shape_a = load_shape(i);
        let shape_b = load_shape(jb);
        if (!broadphase_pair_allowed(a, b, shape_a, shape_b)) {
            continue;
        }
        if (joint_disables_collision(a._pad_island.y, b._pad_island.y)) {
            continue;
        }
        if (!shapes_collide(shape_a, shape_b)) {
            continue;
        }
        if (aabb_overlap(a, b)) {
            append_new_pair(pack_pair(i, jb));
        }
    }
}

@compute @workgroup_size(1)
fn write_occupied_indirect() {
    let n = min(scratch[SCR_OCCUPIED_N], params.contact_capacity);
    write_group_indirect(SCR_INDIRECT_OCCUPIED, (n + 63u) / 64u);
}

fn previous_pair_key(i: u32) -> vec2<u32> {
    let occupied_count = min(scratch[SCR_OCCUPIED_N], params.contact_capacity);
    if (i >= occupied_count) {
        return vec2<u32>(EMPTY);
    }
    let k = scratch[scr_occupied_contact() + i];
    if (k >= params.contact_capacity) {
        return vec2<u32>(EMPTY);
    }
    return previous_pair_key_for_slot(k);
}

fn previous_pair_key_for_slot(k: u32) -> vec2<u32> {
    let p = load_contact(k);
    if (p.manifold_link.y != 0u) { return vec2<u32>(EMPTY); }
    let key = p.pair.xy;
    let ia = key.x;
    let ib = key.y;
    if (p.a == EMPTY || p.b == EMPTY || p.a == p.b
        || ia >= params.shape_count || ib >= params.shape_count) {
        return vec2<u32>(EMPTY);
    }
    let a = load_collider(ia);
    let b = load_collider(ib);
    let shape_a = load_shape(ia);
    let shape_b = load_shape(ib);
    if (!broadphase_pair_allowed(a, b, shape_a, shape_b)) {
        return vec2<u32>(EMPTY);
    }
    if (joint_disables_collision(a._pad_island.y, b._pad_island.y)) {
        return vec2<u32>(EMPTY);
    }
    if (!shapes_collide(shape_a, shape_b)) {
        return vec2<u32>(EMPTY);
    }
    if (!pair_lifetime_overlap(a, b, shape_a, shape_b)) {
        return vec2<u32>(EMPTY);
    }
    return key;
}

@compute @workgroup_size(64)
fn emit_prev_pairs(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let key=previous_pair_key(gid.x);
    if (key.x!=EMPTY) { append_pair(key); }
}

var<workgroup> radix_counts: array<atomic<u32>, 256>;
var<workgroup> radix_keys: array<u32, 256>;
var<workgroup> radix_scan_a: array<u32, 256>;
var<workgroup> radix_scan_b: array<u32, 256>;

fn retained_pair_count() -> u32 {
    return min(atomicLoad(&atom[ATOM_PAIR_N]), pair_cap());
}

fn pair_is_unique(i: u32, n: u32) -> u32 {
    if (i >= n) {
        return 0u;
    }
    let p = load_pair_words(SCR_PAIRS, i);
    if (p.x == EMPTY) {
        return 0u;
    }
    if (i == 0u) {
        return 1u;
    }
    return select(0u, 1u, any(p != load_pair_words(SCR_PAIRS, i - 1u)));
}

fn workgroup_exclusive_scan_256(lid: u32, value: u32) -> u32 {
    radix_scan_a[lid] = value;
    workgroupBarrier();
    var source_a = true;
    for (var offset = 1u; offset < 256u; offset = offset << 1u) {
        var prior = 0u;
        if (source_a) {
            if (lid >= offset) {
                prior = radix_scan_a[lid - offset];
            }
            radix_scan_b[lid] = radix_scan_a[lid] + prior;
        } else {
            if (lid >= offset) {
                prior = radix_scan_b[lid - offset];
            }
            radix_scan_a[lid] = radix_scan_b[lid] + prior;
        }
        source_a = !source_a;
        workgroupBarrier();
    }
    let inclusive = select(radix_scan_b[lid], radix_scan_a[lid], source_a);
    return inclusive - value;
}

// Prefix any number of histogram groups in uniform 256-group chunks. All
// lanes carry the same total; barriers protect scan storage between chunks.
var<workgroup> group_scan_total: u32;
fn scan_group_counts(lid: u32, groups: u32, source: u32, destination: u32) -> u32 {
    var carry = 0u;
    for (var first = 0u; first < groups; first += 256u) {
        let group = first + lid;
        var count = 0u;
        if (group < groups) { count = scratch[source + group]; }
        let exclusive = workgroup_exclusive_scan_256(lid, count);
        if (group < groups) { scratch[destination + group] = carry + exclusive; }
        if (lid == 255u) { group_scan_total = exclusive + count; }
        workgroupBarrier();
        carry += group_scan_total;
        workgroupBarrier();
    }
    return carry;
}

@compute @workgroup_size(256)
fn compact_unique_histogram(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    let n = retained_pair_count();
    let keep = pair_is_unique(gid.x, n);
    let exclusive = workgroup_exclusive_scan_256(lid, keep);
    if (lid == 255u) {
        scratch[scr_radix_base() + group.x] = exclusive + keep;
    }
}

@compute @workgroup_size(256)
fn compact_unique_bases(@builtin(local_invocation_index) lid: u32) {
    // compact_unique_histogram writes only the live workgroup prefix.
    // Radix leftover offsets in the unused suffix must not be scanned.
    let groups = (retained_pair_count() + RADIX_GROUP_SIZE - 1u) / RADIX_GROUP_SIZE;
    let total = scan_group_counts(lid, groups, scr_radix_base(), scr_radix_hist());
    if (lid == 255u) {
        scratch[SCR_POW2] = total;
        if (total > pair_cap()) {
            record_capacity_drop_n(ATOM_PAIR_DROPPED, ATOM_STICKY_PAIR_DROPPED, total - pair_cap());
        }
        let bounded = min(total, pair_cap());
        scratch[SCR_UNIQUE_N] = bounded;
        scratch[SCR_NCONTACTS] = bounded;
        write_count_indirect(SCR_INDIRECT_COLLIDE, bounded, 64u);
        write_count_indirect(SCR_INDIRECT_PREPARE, bounded, 64u);
    }
}

@compute @workgroup_size(256)
fn compact_unique_scatter(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    let n = retained_pair_count();
    let keep = pair_is_unique(gid.x, n);
    let local_exclusive = workgroup_exclusive_scan_256(lid, keep);
    if (keep != 0u) {
        let dst = scratch[scr_radix_hist() + group.x] + local_exclusive;
        store_pair_words(scr_radix_out(), dst, load_pair_words(SCR_PAIRS, gid.x));
    }
}

@compute @workgroup_size(256)
fn compact_unique_gather(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let w = min(scratch[SCR_UNIQUE_N], pair_cap());
    if (gid.x < w) {
        store_pair_words(SCR_PAIRS, gid.x, load_pair_words(scr_radix_out(), gid.x));
    }
}

fn radix_histogram_impl(gid: u32, lid: u32, group: u32, shift: u32, from_output: bool) {
    let source_output = from_output != (shift >= 32u && params.shape_count > 65536u && params.shape_count <= 16777216u);
    atomicStore(&radix_counts[lid], 0u);
    workgroupBarrier();
    let n = retained_pair_count();
    if (gid < n) {
        let key = select(
            load_pair_words(SCR_PAIRS, gid),
            load_pair_words(scr_radix_out(), gid),
            source_output,
        );
        atomicAdd(&radix_counts[pair_digit(key, shift)], 1u);
    }
    workgroupBarrier();
    scratch[scr_radix_hist() + group * RADIX_BUCKETS + lid] =
        atomicLoad(&radix_counts[lid]);
}

@compute @workgroup_size(256)
fn radix_histogram_0(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    radix_histogram_impl(gid.x, lid, group.x, 0u, false);
}

@compute @workgroup_size(256)
fn radix_histogram_8(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    radix_histogram_impl(gid.x, lid, group.x, 8u, true);
}

@compute @workgroup_size(256)
fn radix_histogram_16(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    radix_histogram_impl(gid.x, lid, group.x, 16u, false);
}

@compute @workgroup_size(256)
fn radix_histogram_24(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    radix_histogram_impl(gid.x, lid, group.x, 24u, true);
}

fn radix_live_groups() -> u32 {
    return min(scratch[SCR_RADIX_GROUP_N], radix_groups());
}

@compute @workgroup_size(256)
fn radix_bucket_bases(@builtin(local_invocation_index) bucket: u32) {
    // Sum only the groups the current histogram dispatch initialized. Inactive
    // rows keep leftover prefixes from a larger pair list or the previous digit.
    let groups = radix_live_groups();
    if (groups == 0u) {
        return;
    }
    var total = 0u;
    for (var group = 0u; group < groups; group++) {
        total = total + scratch[scr_radix_hist() + group * RADIX_BUCKETS + bucket];
    }
    radix_scan_a[bucket] = total;
    workgroupBarrier();
    var source_a = true;
    for (var offset = 1u; offset < RADIX_BUCKETS; offset = offset << 1u) {
        var prior = 0u;
        if (source_a) {
            if (bucket >= offset) {
                prior = radix_scan_a[bucket - offset];
            }
            radix_scan_b[bucket] = radix_scan_a[bucket] + prior;
        } else {
            if (bucket >= offset) {
                prior = radix_scan_b[bucket - offset];
            }
            radix_scan_a[bucket] = radix_scan_b[bucket] + prior;
        }
        workgroupBarrier();
        source_a = !source_a;
    }
    // Eight scan rounds leave the inclusive totals in scan_a.
    var base = 0u;
    if (bucket > 0u) {
        base = radix_scan_a[bucket - 1u];
    }
    scratch[scr_radix_base() + bucket] = base;
}

@compute @workgroup_size(256)
fn radix_group_prefix(@builtin(local_invocation_index) bucket: u32) {
    let groups = radix_live_groups();
    if (groups == 0u) {
        return;
    }
    var offset = scratch[scr_radix_base() + bucket];
    for (var group = 0u; group < groups; group++) {
        let at = scr_radix_hist() + group * RADIX_BUCKETS + bucket;
        let count = scratch[at];
        scratch[at] = offset;
        offset = offset + count;
    }
}

fn radix_scatter_impl(gid: u32, lid: u32, group: u32, shift: u32, from_output: bool) {
    let source_output = from_output != (shift >= 32u && params.shape_count > 65536u && params.shape_count <= 16777216u);
    let n = retained_pair_count();
    var key = vec2<u32>(EMPTY);
    if (gid < n) {
        key = select(
            load_pair_words(SCR_PAIRS, gid),
            load_pair_words(scr_radix_out(), gid),
            source_output,
        );
    }
    radix_keys[lid] = pair_digit(key, shift);
    workgroupBarrier();
    if (gid < n) {
        let bucket = pair_digit(key, shift);
        var rank = 0u;
        for (var i = 0u; i < lid; i++) {
            if (radix_keys[i] == bucket) {
                rank = rank + 1u;
            }
        }
        let out = scratch[scr_radix_hist() + group * RADIX_BUCKETS + bucket] + rank;
        if (out >= n) {
            record_capacity_drop(ATOM_PAIR_DROPPED, ATOM_STICKY_PAIR_DROPPED);
        } else if (source_output) {
            store_pair_words(SCR_PAIRS, out, key);
        } else {
            store_pair_words(scr_radix_out(), out, key);
        }
    }
}

@compute @workgroup_size(256)
fn radix_scatter_0(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    radix_scatter_impl(gid.x, lid, group.x, 0u, false);
}

@compute @workgroup_size(256)
fn radix_scatter_8(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    radix_scatter_impl(gid.x, lid, group.x, 8u, true);
}

@compute @workgroup_size(256)
fn radix_scatter_16(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    radix_scatter_impl(gid.x, lid, group.x, 16u, false);
}

@compute @workgroup_size(256)
fn radix_scatter_24(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    radix_scatter_impl(gid.x, lid, group.x, 24u, true);
}

// Read-only probes are safe in parallel. New contact-table claims remain
// ordered by the sorted pair list so tombstone reuse cannot change persistent
// pair identity with workgroup scheduling.

@compute @workgroup_size(256)
fn radix_histogram_32(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    radix_histogram_impl(gid.x, lid, group.x, 32u, false);
}

@compute @workgroup_size(256)
fn radix_histogram_40(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    radix_histogram_impl(gid.x, lid, group.x, 40u, true);
}

@compute @workgroup_size(256)
fn radix_histogram_48(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    radix_histogram_impl(gid.x, lid, group.x, 48u, false);
}

@compute @workgroup_size(256)
fn radix_histogram_56(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    radix_histogram_impl(gid.x, lid, group.x, 56u, true);
}

@compute @workgroup_size(256)
fn radix_scatter_32(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    radix_scatter_impl(gid.x, lid, group.x, 32u, false);
}

@compute @workgroup_size(256)
fn radix_scatter_40(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    radix_scatter_impl(gid.x, lid, group.x, 40u, true);
}

@compute @workgroup_size(256)
fn radix_scatter_48(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    radix_scatter_impl(gid.x, lid, group.x, 48u, false);
}

@compute @workgroup_size(256)
fn radix_scatter_56(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    radix_scatter_impl(gid.x, lid, group.x, 56u, true);
}
@compute @workgroup_size(64)
fn find_existing_contact_slots(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    let count = min(scratch[SCR_UNIQUE_N], pair_cap());
    if (i >= count) {
        return;
    }
    let slot = find_contact_slot(load_pair_words(SCR_PAIRS, i));
    scratch[scr_active_contact() + i] = slot;
    if (slot != EMPTY) {
        if (!contact_chain_structure_valid(slot)) { return; }
        var member = slot;
        let count = max(contacts[slot].manifold_link.z, 1u);
        for (var j = 0u; j < count; j++) {
            scratch[scr_contact_mark() + member] = 1u;
            let next = contacts[member].manifold_link.x;
            if (next != 0u) { member = next - 1u; }
        }
    }
}

fn slot_is_free(slot: u32) -> u32 {
    if (slot >= params.contact_capacity) {
        return 0u;
    }
    return select(0u, 1u, contacts[slot].a == EMPTY && scratch[scr_contact_mark() + slot] == 0u);
}

// Convex pairs need exactly SCR_MISSING_N root slots. All slots at or above
// the high-water mark have never been allocated, so this prefix contains enough
// free entries while preserving the same lowest-free-slot assignment as a full
// scan. Mesh narrowphase may allocate additional child patches: retain the full
// pool for that path until its extra demand can be bounded independently.
fn free_scan_limit() -> u32 {
    if (params.mesh_triangle_count > 0u) { return params.contact_capacity; }
    let high = min(atomicLoad(&query[QUERY_CONTACT_HIGH_WATER]), params.contact_capacity);
    return high + min(scratch[SCR_MISSING_N], params.contact_capacity - high);
}

@compute @workgroup_size(256)
fn alloc_free_histogram(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    let n = free_scan_limit();
    let keep = select(0u, slot_is_free(gid.x), gid.x < n);
    let exclusive = workgroup_exclusive_scan_256(lid, keep);
    if (lid == 255u) {
        scratch[scr_radix_base() + group.x] = exclusive + keep;
    }
}

@compute @workgroup_size(256)
fn alloc_free_bases(@builtin(local_invocation_index) lid: u32) {
    if (scratch[SCR_MISSING_N] == 0u && params.mesh_triangle_count == 0u) {
        if (lid == 0u) {
            scratch[SCR_FREE_N] = 0u;
        }
        return;
    }
    let groups = (free_scan_limit() + RADIX_GROUP_SIZE - 1u) / RADIX_GROUP_SIZE;
    let total = scan_group_counts(lid, groups, scr_radix_base(), scr_radix_base());
    if (lid == 255u) {
        scratch[SCR_FREE_N] = total;
    }
}

@compute @workgroup_size(256)
fn alloc_free_scatter(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    let n = free_scan_limit();
    let keep = select(0u, slot_is_free(gid.x), gid.x < n);
    let local_exclusive = workgroup_exclusive_scan_256(lid, keep);
    if (keep != 0u) {
        let dst = scratch[scr_radix_base() + group.x] + local_exclusive;
        scratch[scr_radix_out() + dst] = gid.x;
    }
}

fn pair_needs_slot(i: u32, n: u32) -> u32 {
    if (i >= n) {
        return 0u;
    }
    return select(0u, 1u, scratch[scr_active_contact() + i] == EMPTY);
}

@compute @workgroup_size(256)
fn alloc_missing_histogram(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    let n = min(scratch[SCR_UNIQUE_N], pair_cap());
    let keep = pair_needs_slot(gid.x, n);
    let exclusive = workgroup_exclusive_scan_256(lid, keep);
    if (lid == 255u) {
        scratch[scr_radix_base() + group.x] = exclusive + keep;
    }
}

@compute @workgroup_size(256)
fn alloc_missing_bases(@builtin(local_invocation_index) lid: u32) {
    let n = min(scratch[SCR_UNIQUE_N], pair_cap());
    let groups = (n + RADIX_GROUP_SIZE - 1u) / RADIX_GROUP_SIZE;
    let total = scan_group_counts(lid, groups, scr_radix_base(), scr_radix_base());
    if (lid == 255u) {
        scratch[SCR_MISSING_N] = total;
    }
}

@compute @workgroup_size(256)
fn alloc_missing_scatter(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    let n = min(scratch[SCR_UNIQUE_N], pair_cap());
    let keep = pair_needs_slot(gid.x, n);
    let local_exclusive = workgroup_exclusive_scan_256(lid, keep);
    if (keep != 0u) {
        let dst = scratch[scr_radix_base() + group.x] + local_exclusive;
        scratch[scr_radix_hist() + dst] = gid.x;
    }
}

@compute @workgroup_size(64)
fn alloc_prepare_keys(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    if (i >= min(scratch[SCR_MISSING_N], scratch[SCR_FREE_N])) { return; }
    let pair_i = scratch[scr_radix_hist() + i];
    let slot = scratch[scr_radix_out() + i];
    prepare_contact_identity(slot, load_pair_words(SCR_PAIRS, pair_i));
}

@compute @workgroup_size(64)
fn alloc_bind_slots(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    let missing = scratch[SCR_MISSING_N];
    let free_n = scratch[SCR_FREE_N];
    if (i >= missing) {
        return;
    }
    if (i >= free_n) {
        record_contact_drop(7u);
        return;
    }
    let pair_i = scratch[scr_radix_hist() + i];
    let slot = scratch[scr_radix_out() + i];
    let key = load_pair_words(SCR_PAIRS, pair_i);
    scratch[scr_active_contact() + pair_i] = slot;
    scratch[scr_contact_mark() + slot] = 1u;
    if (!publish_contact_slot(key, slot)) {
        scratch[scr_active_contact() + pair_i] = EMPTY;
        scratch[scr_contact_mark() + slot] = 0u;
        record_contact_drop(8u);
    }
}

@compute @workgroup_size(64)
fn retire_stale_contacts(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    let occupied_count = min(scratch[SCR_OCCUPIED_N], params.contact_capacity);
    if (i >= occupied_count) {
        return;
    }
    let slot = scratch[scr_occupied_contact() + i];
    if (slot >= params.contact_capacity || scratch[scr_contact_mark() + slot] != 0u) {
        return;
    }
    let c = load_contact(slot);
    if (c.a == EMPTY) {
        return;
    }
    let key = c.pair.xy;
    let ia = key.x;
    let ib = key.y;
    var retire = c.a >= params.body_count || c.b >= params.body_count || c.a == c.b
        || ia >= params.shape_count || ib >= params.shape_count;
    if (!retire) {
        let a = load_collider(ia);
        let b = load_collider(ib);
        let shape_a = load_shape(ia);
        let shape_b = load_shape(ib);
        retire = !broadphase_pair_allowed(a, b, shape_a, shape_b)
            || joint_disables_collision(a._pad_island.y, b._pad_island.y)
            || !shapes_collide(shape_a, shape_b)
            || !pair_lifetime_overlap(a, b, shape_a, shape_b);
    }
    if (retire) {
        if (!retire_manifold_children(slot)) { return; }
        var retired = empty_contact();
        retired.lifecycle.x = c.lifecycle.x;
        store_contact(slot, retired);
        retire_contact_key(c.pair.xy);
    } else {
        // A retained pair can be absent from this frame's candidate list only
        // when candidate capacity was exhausted. Keep it discoverable so a
        // later non-overflowing frame can emit it again.
        atomicAdd(&query[67u], max(contacts[slot].manifold_link.z, 1u));
        if (contacts[slot].count > 0u && (contact_persistent[slot].lifecycle.y & CONTACT_TOUCHING) != 0u) {
            atomicAdd(&query[68u], 1u);
        }
        if (native_countable_root(slot)) { atomicAdd(&query[69u], 1u); }
        let out = atomicAdd(&atom[ATOM_OCCUPIED_N], 1u);
        if (out < params.contact_capacity) {
            scratch[scr_next_occupied() + out] = slot;
        }
    }
}

fn native_countable_root(slot: u32) -> bool {
    let key = contact_persistent[slot].pair.xy;
    let a = key.x;
    let b = key.y;
    if (a >= params.shape_count || b >= params.shape_count) { return false; }
    let sa = load_shape(a); let sb = load_shape(b);
    return ((sa.event_flags | sb.event_flags) & (SHAPE_IS_SENSOR | SHAPE_PUBLIC_PROXY)) == 0u
        && !(sa.kind == KIND_MESH && sb.kind == KIND_MESH);
}

@compute @workgroup_size(1)
fn begin_occupied_contacts() {
    atomicStore(&query[67u], 0u); // allocated root + child manifold slots
    atomicStore(&query[68u], 0u); // touching roots, including sensors
    atomicStore(&query[69u], 0u); // allocated non-sensor supported contact roots
    atomicStore(&atom[ATOM_OCCUPIED_N], 0u);
}

var<workgroup> metric_patch_sum: atomic<u32>;
var<workgroup> metric_touching_sum: atomic<u32>;
var<workgroup> metric_non_sensor_sum: atomic<u32>;

@compute @workgroup_size(64)
fn collect_occupied_contacts(@builtin(global_invocation_id) dispatch_gid: vec3<u32>,
                             @builtin(local_invocation_index) lid: u32) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    if (lid == 0u) {
        atomicStore(&metric_patch_sum, 0u);
        atomicStore(&metric_touching_sum, 0u);
        atomicStore(&metric_non_sensor_sum, 0u);
    }
    workgroupBarrier();
    let i = gid.x;
    let count = min(scratch[SCR_UNIQUE_N], pair_cap());
    if (i < count) {
        let slot = scratch[scr_active_contact() + i];
        if (slot != EMPTY) {
            atomicAdd(&metric_patch_sum, max(contacts[slot].manifold_link.z, 1u));
            if (native_countable_root(slot)) { atomicAdd(&metric_non_sensor_sum, 1u); }
            if (contacts[slot].count > 0u && (contact_persistent[slot].lifecycle.y & CONTACT_TOUCHING) != 0u) {
                atomicAdd(&metric_touching_sum, 1u);
            }
            let out = atomicAdd(&atom[ATOM_OCCUPIED_N], 1u);
            if (out < params.contact_capacity) {
                scratch[scr_next_occupied() + out] = slot;
            }
        }
    }
    workgroupBarrier();
    if (lid == 0u) {
        atomicAdd(&query[67u], atomicLoad(&metric_patch_sum));
        atomicAdd(&query[68u], atomicLoad(&metric_touching_sum));
        atomicAdd(&query[69u], atomicLoad(&metric_non_sensor_sum));
    }
}

fn graph_contact_live(slot: u32) -> bool {
    if (slot == EMPTY || slot >= params.contact_capacity) {
        return false;
    }
    let h = contacts[slot];
    if (h.a == EMPTY || h.count == 0u || h.manifold_link.y != 0u) {
        return false;
    }
    let life = contact_persistent[slot].lifecycle;
    if ((life.y & CONTACT_TOUCHING) == 0u) {
        return false;
    }
    if (scratch[scr_contact_mark() + slot] == 0u) {
        return false;
    }
    let ia = contact_persistent[slot].pair.x;
    let ib = contact_persistent[slot].pair.y;
    return ia < params.shape_count && ib < params.shape_count
        && ((load_shape(ia).event_flags | load_shape(ib).event_flags) & SHAPE_IS_SENSOR) == 0u;
}

@compute @workgroup_size(1)
fn write_unique_indirect() {
    let n = min(scratch[SCR_UNIQUE_N], pair_cap());
    write_count_indirect(SCR_INDIRECT_RADIX, n, RADIX_GROUP_SIZE);
}

@compute @workgroup_size(1)
fn write_alloc_indirects() {
    let missing = scratch[SCR_MISSING_N];
    // Reserve the prefix for primary pairs; children use only the remainder.
    atomicStore(&atom[ATOM_MANIFOLD_ALLOC], missing);
    let need_free_list = missing > 0u || params.mesh_triangle_count > 0u;
    write_count_indirect(SCR_INDIRECT_PREPARE, missing, 64u);
    let free_groups = select(
        0u,
        (free_scan_limit() + RADIX_GROUP_SIZE - 1u) / RADIX_GROUP_SIZE,
        need_free_list,
    );
    write_group_indirect(SCR_INDIRECT_RADIX, free_groups);
    if (!need_free_list) {
        scratch[SCR_FREE_N] = 0u;
    }
}

@compute @workgroup_size(64)
fn graph_clear_meta(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    if (i >= params.body_count) {
        return;
    }
    atomicStore(&atom[atom_jacobi() + i], 0u);
    atomicStore(&atom[atom_jacobi() + params.body_count + i], select(0u,EMPTY,(params.diagnostic_flags & DIAG_STATIC_DEGREE_TWO_PROOF)!=0u));
    atomicStore(&atom[atom_jacobi() + 2u * params.body_count + i], 0u);
    atomicStore(&atom[atom_jacobi() + 3u * params.body_count + i], 0u);
    scratch[color_body_base() + i] = 0u;
    scratch[fused_flag_base() + i] = 0u;
    let slots = fused_slots_base() + 8u * i;
    for (var k = 0u; k < 8u; k++) {
        scratch[slots + k] = EMPTY;
    }
}

@compute @workgroup_size(1)
fn graph_reset_colors() {
    for (var col = 0u; col < 24u; col++) {
        scratch[SCR_COLOR + col] = 0u;
        atomicStore(&atom[atom_graph_color() + col], 0u);
    }
    scratch[SCR_FUSED_N] = 0u;
    scratch[SCR_DYN_DYN_N] = 0u;
    scratch[SCR_GRAPH_STATIC_N] = 0u;
    scratch[SCR_GRAPH_STATIC_MAX] = 0u;
    atomicStore(&atom[atom_dyn_dyn()], 0u);
}

fn graph_kind(unique_i: u32) -> u32 {
    return scratch[scr_radix_hist() + unique_i] & 3u;
}

fn graph_dyn_body(unique_i: u32) -> u32 {
    return scratch[scr_radix_hist() + unique_i] >> 2u;
}

@compute @workgroup_size(64)
fn graph_classify(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    let np = min(scratch[SCR_UNIQUE_N], pair_cap());
    if (i >= np) {
        return;
    }
    let slot = scratch[scr_active_contact() + i];
    var packed = 0u;
    if (graph_contact_live(slot)) {
        let h = contacts[slot];
        let a_dyn = !is_non_dynamic(load_body(h.a));
        let b_dyn = !is_non_dynamic(load_body(h.b));
        if (a_dyn != b_dyn) {
            let dyn = select(h.b, h.a, a_dyn);
            packed = 1u | (dyn << 2u);
        } else if (a_dyn && b_dyn) {
            packed = 2u | (h.a << 2u);
            atomicOr(&atom[atom_jacobi() + 2u * params.body_count + h.a], 1u);
            atomicOr(&atom[atom_jacobi() + 2u * params.body_count + h.b], 1u);
        }
    }
    scratch[scr_radix_hist() + i] = packed;
}

@compute @workgroup_size(64)
fn graph_mark_edges(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    if (i >= params.joint_count) {
        return;
    }
    let j = joints[i];
    if (j.kind != JOINT_NONE && j.kind != JOINT_FILTER) {
        if (j.a < params.body_count) {
            atomicOr(&atom[atom_jacobi() + 2u * params.body_count + j.a], 1u);
        }
        if (j.b < params.body_count) {
            atomicOr(&atom[atom_jacobi() + 2u * params.body_count + j.b], 1u);
        }
    }
}

fn graph_keep_kind(i: u32, n: u32, kind: u32) -> u32 {
    if (i >= n) {
        return 0u;
    }
    return select(0u, 1u, graph_kind(i) == kind);
}

// Hash-cell IDs are dead after emit_hash_pairs. Reuse their first two group
// arrays here; neither output list may alias prefixes during parallel scatter.
fn scr_graph_paired_prefix() -> u32 { return scr_ins_cell(); }
@compute @workgroup_size(256)
fn graph_compact_paired_histogram(@builtin(global_invocation_id) dispatch_gid:vec3<u32>,
    @builtin(local_invocation_index) lid:u32,@builtin(workgroup_id) dispatch_group:vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    let n=min(scratch[SCR_UNIQUE_N],pair_cap());
    for (var kind=1u;kind<=2u;kind++) {
        let keep=graph_keep_kind(gid.x,n,kind);
        let exclusive=workgroup_exclusive_scan_256(lid,keep);
        if (lid==255u) {scratch[scr_graph_paired_prefix()+(kind-1u)*radix_groups()+group.x]=exclusive+keep;}
    }
}
@compute @workgroup_size(256)
fn graph_compact_paired_bases(@builtin(local_invocation_index) lid:u32) {
    let n=min(scratch[SCR_UNIQUE_N],pair_cap());
    let groups=(n+RADIX_GROUP_SIZE-1u)/RADIX_GROUP_SIZE;
    for (var kind=1u;kind<=2u;kind++) {
        let base=scr_graph_paired_prefix()+(kind-1u)*radix_groups();
        let total=scan_group_counts(lid,groups,base,base);
        if (lid==255u) {scratch[select(SCR_GRAPH_STATIC_N,SCR_DYN_DYN_N,kind==2u)]=min(total,pair_cap());}
    }
}
@compute @workgroup_size(256)
fn graph_compact_paired_scatter(@builtin(global_invocation_id) dispatch_gid:vec3<u32>,
    @builtin(local_invocation_index) lid:u32,@builtin(workgroup_id) dispatch_group:vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    let n=min(scratch[SCR_UNIQUE_N],pair_cap());
    for (var kind=1u;kind<=2u;kind++) {
        let keep=graph_keep_kind(gid.x,n,kind);
        let exclusive=workgroup_exclusive_scan_256(lid,keep);
        if (keep!=0u) {
            let dst=scratch[scr_graph_paired_prefix()+(kind-1u)*radix_groups()+group.x]+exclusive;
            if (dst<pair_cap()) {scratch[select(scr_radix_out(),scr_next_occupied(),kind==2u)+dst]=gid.x;}
        }
    }
}

@compute @workgroup_size(64)
fn graph_count_static_degree(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    let n = min(scratch[SCR_GRAPH_STATIC_N], pair_cap());
    if (i >= n) {
        return;
    }
    let unique_i = scratch[scr_radix_out() + i];
    let dyn = graph_dyn_body(unique_i);
    if (dyn < params.body_count) {
        let prev = atomicAdd(&atom[atom_jacobi() + 3u * params.body_count + dyn], 1u);
        atomicMax(&atom[atom_dyn_dyn()], prev + 1u);
        if ((params.diagnostic_flags & DIAG_STATIC_DEGREE_TWO_PROOF)!=0u) {
            atomicMin(&atom[atom_jacobi()+params.body_count+dyn],unique_i);
        }
    }
}

@compute @workgroup_size(1)
fn graph_finish_static_degree() {
    let n = min(scratch[SCR_GRAPH_STATIC_N], pair_cap());
    let max_deg = atomicLoad(&atom[atom_dyn_dyn()]);
    scratch[SCR_GRAPH_STATIC_MAX] = max_deg;
    if ((params.diagnostic_flags & DIAG_STATIC_DEGREE_ONE_PROOF) != 0u && max_deg > 1u && n > 0u) {
        atomicStore(&atom[ATOM_GRAPH_PROOF_FAIL], 1u);
        record_contact_drop(9u);
    }
    if ((params.diagnostic_flags & DIAG_STATIC_DEGREE_TWO_PROOF)!=0u && max_deg>2u && n>0u) {
        atomicStore(&atom[ATOM_GRAPH_PROOF_FAIL],1u);record_contact_drop(9u);
    }
    if (max_deg > 1u && n > 0u) {
        let groups = (n + RADIX_GROUP_SIZE - 1u) / RADIX_GROUP_SIZE;
        scratch[SCR_RADIX_GROUP_N] = groups;
        write_count_indirect(SCR_INDIRECT_RADIX, n, RADIX_GROUP_SIZE);
    } else {
        scratch[SCR_RADIX_GROUP_N] = 0u;
        write_group_indirect(SCR_INDIRECT_RADIX, 0u);
    }
    write_count_indirect(SCR_INDIRECT_STATIC, n, 64u);
}

fn graph_sort_count() -> u32 {
    return min(scratch[SCR_GRAPH_STATIC_N], pair_cap());
}

fn graph_sort_src(from_color: bool) -> u32 {
    return select(scr_radix_out(), color_contact_base(), from_color);
}

fn graph_sort_dst(from_color: bool) -> u32 {
    return select(color_contact_base(), scr_radix_out(), from_color);
}

fn graph_static_color(rank: u32) -> u32 {
    if (rank >= OVERFLOW_COLOR - 1u) {
        return OVERFLOW_COLOR;
    }
    return OVERFLOW_COLOR - 1u - rank;
}

// Carry full unique indices through both stable sorts. contacts[].color is a
// temporary full-width body key during the first sort, then the actual color.
// The overflow list is not populated yet; preserve the canonical input order
// there so the second stable sort has (color, unique_i) order, not (color, body).
fn graph_sort_key(unique_i: u32) -> u32 {
    return contacts[scratch[scr_active_contact() + unique_i]].color;
}
fn graph_static_order_base() -> u32 {
    return color_contact_base() + OVERFLOW_COLOR * params.contact_capacity;
}
@compute @workgroup_size(64)
fn graph_pack_static_keys(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    if ((params.diagnostic_flags & DIAG_STATIC_DEGREE_TWO_PROOF)!=0u) {return;}
    let i = gid.x;
    let n = min(scratch[SCR_GRAPH_STATIC_N], pair_cap());
    if (i >= n || scratch[SCR_GRAPH_STATIC_MAX] <= 1u) {
        return;
    }
    let unique_i = scratch[scr_radix_out() + i];
    let dyn = graph_dyn_body(unique_i);
    scratch[graph_static_order_base() + i] = unique_i;
    contacts[scratch[scr_active_contact() + unique_i]].color = dyn;
}

fn graph_radix_histogram_impl(gid: u32, lid: u32, group: u32, shift: u32, from_color: bool) {
    atomicStore(&radix_counts[lid], 0u);
    workgroupBarrier();
    let n = graph_sort_count();
    if (gid < n) {
        let key = graph_sort_key(scratch[graph_sort_src(from_color) + gid]);
        atomicAdd(&radix_counts[(key >> shift) & 255u], 1u);
    }
    workgroupBarrier();
    scratch[scr_radix_hist() + group * RADIX_BUCKETS + lid] =
        atomicLoad(&radix_counts[lid]);
}

@compute @workgroup_size(256)
fn graph_radix_histogram_0(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    graph_radix_histogram_impl(gid.x, lid, group.x, 0u, false);
}

@compute @workgroup_size(256)
fn graph_radix_histogram_8(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    graph_radix_histogram_impl(gid.x, lid, group.x, 8u, true);
}

@compute @workgroup_size(256)
fn graph_radix_histogram_16(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    graph_radix_histogram_impl(gid.x, lid, group.x, 16u, false);
}

@compute @workgroup_size(256)
fn graph_radix_histogram_24(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    graph_radix_histogram_impl(gid.x, lid, group.x, 24u, true);
}

fn graph_radix_scatter_impl(gid: u32, lid: u32, group: u32, shift: u32, from_color: bool) {
    let n = graph_sort_count();
    var unique_i = EMPTY;
    var key = EMPTY;
    if (gid < n) {
        unique_i = scratch[graph_sort_src(from_color) + gid];
        key = graph_sort_key(unique_i);
    }
    radix_keys[lid] = key;
    workgroupBarrier();
    if (gid < n) {
        let bucket = (key >> shift) & 255u;
        var rank = 0u;
        for (var j = 0u; j < lid; j++) {
            if (((radix_keys[j] >> shift) & 255u) == bucket) {
                rank = rank + 1u;
            }
        }
        let out = scratch[scr_radix_hist() + group * RADIX_BUCKETS + bucket] + rank;
        if (out >= n) {
            record_contact_drop(10u);
        } else {
            scratch[graph_sort_dst(from_color) + out] = unique_i;
        }
    }
}

@compute @workgroup_size(256)
fn graph_radix_scatter_0(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    graph_radix_scatter_impl(gid.x, lid, group.x, 0u, false);
}

@compute @workgroup_size(256)
fn graph_radix_scatter_8(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    graph_radix_scatter_impl(gid.x, lid, group.x, 8u, true);
}

@compute @workgroup_size(256)
fn graph_radix_scatter_16(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    graph_radix_scatter_impl(gid.x, lid, group.x, 16u, false);
}

@compute @workgroup_size(256)
fn graph_radix_scatter_24(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) dispatch_group: vec3<u32>,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 256u), 0u, 0u);
    let group = vec3<u32>(linear_workgroup_id(dispatch_group), 0u, 0u);
    if (group.x >= radix_groups()) { return; }
    graph_radix_scatter_impl(gid.x, lid, group.x, 24u, true);
}

@compute @workgroup_size(64)
fn graph_mark_static_starts(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    let n = min(scratch[SCR_GRAPH_STATIC_N], pair_cap());
    if (i >= n || scratch[SCR_GRAPH_STATIC_MAX] <= 1u) {
        return;
    }
    let dyn = graph_sort_key(scratch[scr_radix_out() + i]);
    var prev = EMPTY;
    if (i > 0u) {
        prev = graph_sort_key(scratch[scr_radix_out() + i - 1u]);
    }
    if (i == 0u || dyn != prev) {
        atomicStore(&atom[atom_jacobi() + params.body_count + dyn], i);
    }
}

@compute @workgroup_size(64)
fn graph_encode_static_colors(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    let n = min(scratch[SCR_GRAPH_STATIC_N], pair_cap());
    if (i >= n || scratch[SCR_GRAPH_STATIC_MAX] <= 1u) {
        return;
    }
    let unique_i = scratch[scr_radix_out() + i];
    let dyn = graph_sort_key(unique_i);
    let start = atomicLoad(&atom[atom_jacobi() + params.body_count + dyn]);
    let rank = i - start;
    let col = graph_static_color(rank);
    if (dyn < params.body_count && col < OVERFLOW_COLOR) {
        atomicOr(&atom[atom_jacobi() + dyn], 1u << col);
    }
    contacts[scratch[scr_active_contact() + unique_i]].color = col;
    scratch[scr_radix_out() + i] = scratch[graph_static_order_base() + i];
}

@compute @workgroup_size(64)
fn graph_mark_color_starts(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    let n = min(scratch[SCR_GRAPH_STATIC_N], pair_cap());
    if (i >= n || scratch[SCR_GRAPH_STATIC_MAX] <= 1u) {
        return;
    }
    let col = graph_sort_key(scratch[scr_radix_out() + i]);
    var prev = EMPTY;
    if (i > 0u) {
        prev = graph_sort_key(scratch[scr_radix_out() + i - 1u]);
    }
    if (i == 0u || col != prev) {
        scratch[SCR_COLOR + col] = i;
    }
}

// Degree-one static edges share one reserved color with local index = compact i.
// Ranked edges were sorted by (body, pair) then (color, unique) so scatter is
// conflict-free without a scene-wide scalar loop.
@compute @workgroup_size(64)
fn graph_assign_static(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    let n = min(scratch[SCR_GRAPH_STATIC_N], pair_cap());
    if (i >= n) {
        return;
    }
    if ((params.diagnostic_flags & DIAG_STATIC_DEGREE_TWO_PROOF)!=0u) {
        if (scratch[SCR_GRAPH_STATIC_MAX]>2u) {return;} // sticky proof failure already reported
        let unique_i=scratch[scr_radix_out()+i];
        let dyn=graph_dyn_body(unique_i);
        if (dyn>=params.body_count) {return;}
        let first=atomicLoad(&atom[atom_jacobi()+params.body_count+dyn]);
        let rank=select(1u,0u,unique_i==first);
        let col=graph_static_color(rank);
        atomicOr(&atom[atom_jacobi()+dyn],1u<<col);
        let local=atomicAdd(&atom[atom_graph_color()+col],1u);
        let slot=scratch[scr_active_contact()+unique_i];
        if (local<params.contact_capacity) {
            scratch[color_contact_base()+col*params.contact_capacity+local]=slot;
            store_graph_meta(slot,col,local);
        } else {record_contact_drop(9u);}
        return;
    }
    let max_deg = scratch[SCR_GRAPH_STATIC_MAX];
    if (max_deg <= 1u) {
        let unique_i = scratch[scr_radix_out() + i];
        let slot = scratch[scr_active_contact() + unique_i];
        let dyn = graph_dyn_body(unique_i);
        let col = OVERFLOW_COLOR - 1u;
        if (dyn < params.body_count) {
            atomicOr(&atom[atom_jacobi() + dyn], 1u << col);
        }
        if (i < params.contact_capacity) {
            scratch[color_contact_base() + col * params.contact_capacity + i] = slot;
            store_graph_meta(slot, col, i);
        }
        if (i == 0u) {
            atomicStore(&atom[atom_graph_color() + col], n);
        }
        return;
    }
    let unique_i = scratch[scr_radix_out() + i];
    let col = graph_sort_key(unique_i);
    let slot = scratch[scr_active_contact() + unique_i];
    let start = scratch[SCR_COLOR + col];
    let local = i - start;
    atomicAdd(&atom[atom_graph_color() + col], 1u);
    if (local < params.contact_capacity) {
        scratch[color_contact_base() + col * params.contact_capacity + local] = slot;
        store_graph_meta(slot, col, local);
    }
}

fn color_dynamic_contact(unique_i: u32) {
    let slot = scratch[scr_active_contact() + unique_i];
    let h = contacts[slot];
    // This entry is owned by the single canonical greedy walk. No other
    // invocation can change either occupancy mask between these loads/stores.
    let occupied = atomicLoad(&atom[atom_jacobi() + h.a])
        | atomicLoad(&atom[atom_jacobi() + h.b]);
    let available = (~occupied) & ((1u << DYNAMIC_COLOR_COUNT) - 1u);
    var col = OVERFLOW_COLOR;
    if (available != 0u) {
        col = firstTrailingBit(available);
        let bit = 1u << col;
        atomicOr(&atom[atom_jacobi() + h.a], bit);
        atomicOr(&atom[atom_jacobi() + h.b], bit);
    }
    let local = atomicAdd(&atom[atom_graph_color() + col], 1u);
    if (local < params.contact_capacity) {
        scratch[color_contact_base() + col * params.contact_capacity + local] = slot;
        store_graph_meta(slot, col, local);
    }
}

@compute @workgroup_size(1)
fn graph_assign_dynamic() {
    // Pair-key greedy walk. Per-island packing was a measured no-go: mixed
    // stacks paid extra compact cost and Dominoes is one writable component.
    let n = min(scratch[SCR_DYN_DYN_N], pair_cap());
    for (var i = 0u; i < n; i++) {
        color_dynamic_contact(scratch[scr_next_occupied() + i]);
    }
    finish_dynamic_graph();
}

// Keep the canonical greedy order, but cooperatively fetch and publish each
// batch. Only its at-most-512 endpoints occupy shared memory, so this cache does
// not impose a world-size limit. One lane makes the ordered color decisions;
// the others hide global-memory latency without racing on endpoint masks.
var<workgroup> graph_batch_keys: array<atomic<u32>, 1024>;
var<workgroup> graph_batch_masks: array<atomic<u32>, 1024>;
var<workgroup> graph_batch_a: array<u32, 256>;
var<workgroup> graph_batch_b: array<u32, 256>;
var<workgroup> graph_batch_slots: array<u32, 256>;
var<workgroup> graph_batch_choices: array<vec2<u32>, 256>;
var<workgroup> graph_batch_counts: array<u32, 24>;
var<workgroup> graph_batch_n: u32;

fn graph_batch_endpoint(body: u32) -> u32 {
    var at = pair_hash_mix(body) & 1023u;
    loop {
        let insert = atomicCompareExchangeWeak(&graph_batch_keys[at], EMPTY, body);
        if (insert.exchanged) {
            atomicStore(&graph_batch_masks[at], atomicLoad(&atom[atom_jacobi() + body]));
            return at;
        }
        if (insert.old_value == body) { return at; }
        // A weak exchange may fail spuriously on an empty entry. Retry there
        // instead of allowing duplicate owners for the same endpoint.
        if (insert.old_value != EMPTY) { at = (at + 1u) & 1023u; }
    }
    return 0u;
}

// Cache the coloring function, not contacts or solver state. Each batch stores
// its endpoints, incoming masks, chosen colors and within-batch local indices.
// Global color offsets may change and are added afresh on every invocation.
// This shares the optional memo allocation with the small-world kernel; distinct
// validity tags make switching between implementations invalidate old contents.
const GRAPH_BATCH_MEMO_TAG: u32 = 0x424b5431u;
var<workgroup> graph_batch_start_counts: array<u32, 24>;
var<workgroup> graph_batch_added: array<atomic<u32>, 24>;
var<workgroup> graph_batch_changed: atomic<u32>;
var<workgroup> graph_batch_reuse: u32;
var<workgroup> graph_batch_cache_ready: u32;
var<workgroup> graph_batch_buckets: array<u32, 256>;
var<workgroup> graph_batch_first_bucket: u32;
var<workgroup> graph_batch_partition: atomic<u32>;
var<workgroup> graph_batch_partition_count: u32;

fn graph_batch_memo_base() -> u32 {
    return 261u + 54u * (params.shape_base_u32 / 32u) + 2u * params.contact_capacity;
}
// Four cached chunks per 128-body range. Denser ranges still use the exact
// greedy fallback for their remaining chunks. Edge order is never changed.
fn graph_batch_memo_groups() -> u32 { return 4u * ((params.shape_base_u32 / 32u + 127u) / 128u); }
fn graph_batch_memo_edges() -> u32 { return graph_batch_memo_base() + 8u + graph_batch_memo_groups(); }
fn graph_batch_memo_end() -> u32 { return graph_batch_memo_edges() + 6u * 256u * graph_batch_memo_groups(); }

@compute @workgroup_size(256)
fn graph_assign_dynamic_batched(@builtin(local_invocation_index) lane: u32) {
    let memo = graph_batch_memo_base();
    let cache = arrayLength(&query) >= graph_batch_memo_end()
        && (params.diagnostic_flags & DIAG_REBUILD_GRAPH) == 0u;
    if (lane < 24u) {
        graph_batch_counts[lane] = atomicLoad(&atom[atom_graph_color() + lane]);
    }
    if (lane == 0u) {
        graph_batch_n = min(scratch[SCR_DYN_DYN_N], pair_cap());
        graph_batch_cache_ready = 0u;
        if (cache) {
            graph_batch_cache_ready = select(0u, 1u, atomicLoad(&query[memo]) == GRAPH_BATCH_MEMO_TAG);
            if (graph_batch_cache_ready == 0u) {
                atomicStore(&query[memo + 1u], 0u);
                atomicStore(&query[memo + 2u], 0u);
            }
        }
    }
    workgroupBarrier();
    let n = workgroupUniformLoad(&graph_batch_n);
    let ready = workgroupUniformLoad(&graph_batch_cache_ready) != 0u;
    if (cache && !ready) {
        // Invalidate unvisited batches too, including after switching from the
        // small-world memo layout and later growing the active edge list.
        for (var batch = lane; batch < graph_batch_memo_groups(); batch += 256u) {
            atomicStore(&query[memo + 8u + batch], 0u);
        }
    }
    storageBarrier();
    var base = 0u;
    var previous_bucket = EMPTY;
    var chunk = 0u;
    loop {
        if (base >= n) { break; }
        let remaining = min(256u, n - base);
        for (var at = lane; at < 1024u; at += 256u) {
            atomicStore(&graph_batch_keys[at], EMPTY);
        }
        if (lane < 24u) {
            graph_batch_start_counts[lane] = graph_batch_counts[lane];
            atomicStore(&graph_batch_added[lane], 0u);
        }
        if (lane == 0u) { atomicStore(&graph_batch_partition, remaining); }
        workgroupBarrier();
        if (lane < remaining) {
            let slot = scratch[scr_active_contact() + scratch[scr_next_occupied() + base + lane]];
            let h = contacts[slot];
            graph_batch_slots[lane] = slot;
            graph_batch_a[lane] = graph_batch_endpoint(h.a);
            graph_batch_b[lane] = graph_batch_endpoint(h.b);
            let bucket = (max(max(h.a, h.b), 1u) - 1u) / 128u;
            graph_batch_buckets[lane] = bucket;
            if (lane == 0u) { graph_batch_first_bucket = bucket; }
        }
        workgroupBarrier();
        let bucket = workgroupUniformLoad(&graph_batch_first_bucket);
        if (cache && lane < remaining && graph_batch_buckets[lane] != bucket) {
            atomicMin(&graph_batch_partition, lane);
        }
        workgroupBarrier();
        if (lane == 0u) { graph_batch_partition_count = atomicLoad(&graph_batch_partition); }
        workgroupBarrier();
        let count = workgroupUniformLoad(&graph_batch_partition_count);
        if (bucket == previous_bucket) { chunk++; } else { chunk = 0u; }
        previous_bucket = bucket;
        let cache_batch = 4u * bucket + chunk;
        let batch_cache = cache && chunk < 4u;
        let header = memo + 8u + cache_batch;
        if (lane == 0u) {
            atomicStore(&graph_batch_changed, 1u);
            if (batch_cache && ready) {
                atomicStore(&graph_batch_changed, select(1u, 0u, atomicLoad(&query[header]) == count));
            }
        }
        workgroupBarrier();
        if (batch_cache && lane < count) {
            let a = graph_batch_a[lane];
            let b = graph_batch_b[lane];
            let record = graph_batch_memo_edges() + 6u * (cache_batch * 256u + lane);
            let body_a = atomicLoad(&graph_batch_keys[a]);
            let body_b = atomicLoad(&graph_batch_keys[b]);
            let mask_a = atomicLoad(&graph_batch_masks[a]) & ((1u << DYNAMIC_COLOR_COUNT) - 1u);
            let mask_b = atomicLoad(&graph_batch_masks[b]) & ((1u << DYNAMIC_COLOR_COUNT) - 1u);
            if (atomicLoad(&query[record]) != body_a || atomicLoad(&query[record + 1u]) != body_b
                || atomicLoad(&query[record + 2u]) != mask_a || atomicLoad(&query[record + 3u]) != mask_b) {
                atomicStore(&graph_batch_changed, 1u);
            }
            // Each lane owns one record; no other lane reads this record.
            atomicStore(&query[record], body_a);
            atomicStore(&query[record + 1u], body_b);
            atomicStore(&query[record + 2u], mask_a);
            atomicStore(&query[record + 3u], mask_b);
        }
        workgroupBarrier();
        if (lane == 0u) { graph_batch_reuse = select(0u, 1u, atomicLoad(&graph_batch_changed) == 0u); }
        workgroupBarrier();
        let reuse = workgroupUniformLoad(&graph_batch_reuse) != 0u;
        if (reuse) {
            if (lane < count) {
                let record = graph_batch_memo_edges() + 6u * (cache_batch * 256u + lane);
                let col = atomicLoad(&query[record + 4u]);
                let local = atomicLoad(&query[record + 5u]);
                graph_batch_choices[lane] = vec2<u32>(col, graph_batch_start_counts[col] + local);
                atomicAdd(&graph_batch_added[col], 1u);
                if (col < DYNAMIC_COLOR_COUNT) {
                    atomicOr(&graph_batch_masks[graph_batch_a[lane]], 1u << col);
                    atomicOr(&graph_batch_masks[graph_batch_b[lane]], 1u << col);
                }
            }
        } else if (lane == 0u) {
            for (var i = 0u; i < count; i++) {
                let a = graph_batch_a[i];
                let b = graph_batch_b[i];
                let mask_a = atomicLoad(&graph_batch_masks[a]);
                let mask_b = atomicLoad(&graph_batch_masks[b]);
                let available = (~(mask_a | mask_b)) & ((1u << DYNAMIC_COLOR_COUNT) - 1u);
                var col = OVERFLOW_COLOR;
                if (available != 0u) {
                    col = firstTrailingBit(available);
                    // Only lane zero writes masks in the fallback walk.
                    atomicStore(&graph_batch_masks[a], mask_a | (1u << col));
                    atomicStore(&graph_batch_masks[b], mask_b | (1u << col));
                }
                graph_batch_choices[i] = vec2<u32>(col, graph_batch_counts[col]);
                graph_batch_counts[col]++;
            }
        }
        workgroupBarrier();
        if (reuse && lane < 24u) {
            graph_batch_counts[lane] += atomicLoad(&graph_batch_added[lane]);
        }
        if (lane < count) {
            let choice = graph_batch_choices[lane];
            let slot = graph_batch_slots[lane];
            if (choice.y < params.contact_capacity) {
                scratch[color_contact_base() + choice.x * params.contact_capacity + choice.y] = slot;
                store_graph_meta(slot, choice.x, choice.y);
            } else { record_contact_drop(9u); }
            if (batch_cache) {
                let record = graph_batch_memo_edges() + 6u * (cache_batch * 256u + lane);
                atomicStore(&query[record + 4u], choice.x);
                atomicStore(&query[record + 5u], choice.y - graph_batch_start_counts[choice.x]);
            }
        }
        if (batch_cache && lane == 0u) {
            atomicStore(&query[header], count);
            atomicAdd(&query[memo + select(2u, 1u, reuse)], 1u);
        }
        for (var at = lane; at < 1024u; at += 256u) {
            let body = atomicLoad(&graph_batch_keys[at]);
            if (body != EMPTY) { atomicStore(&atom[atom_jacobi() + body], atomicLoad(&graph_batch_masks[at])); }
        }
        storageBarrier();
        workgroupBarrier();
        base += count;
    }
    if (lane < 24u) {
        atomicStore(&atom[atom_graph_color() + lane], graph_batch_counts[lane]);
    }
    if (cache && lane == 0u) { atomicStore(&query[memo], GRAPH_BATCH_MEMO_TAG); }
    storageBarrier();
    workgroupBarrier();
    if (lane == 0u) { finish_dynamic_graph(); }
}

fn finish_dynamic_graph() {
    var mx = 0u;
    var colored = 0u;
    for (var col = 0u; col < 24u; col++) {
        let count = atomicLoad(&atom[atom_graph_color() + col]);
        scratch[SCR_COLOR + col] = count;
        colored = colored + count;
        if (col < OVERFLOW_COLOR) {
            mx = max(mx, (count + 63u) / 64u);
        }
    }
    let np_all = min(scratch[SCR_NCONTACTS], pair_cap());
    if (params.solver_mode == SOLVER_JACOBI) {
        write_group_indirect(SCR_INDIRECT_COLLIDE, (np_all + 63u) / 64u);
    } else {
        let use_one_group_wave = mx <= 1u;
        write_group_indirect(SCR_INDIRECT_COLLIDE, select(0u, 1u, use_one_group_wave));
        for (var c = 0u; c < 24u; c++) {
            let n_col = scratch[SCR_COLOR + c];
            var groups = (n_col + 63u) / 64u;
            if (c == OVERFLOW_COLOR) {
                groups = select(0u, 1u, n_col > 0u);
            }
            let base = SCR_INDIRECT_COLOR + c * 4u;
            write_group_indirect(base, select(groups, 0u, use_one_group_wave));
        }
    }
    scratch[SCR_INDIRECT_COLLIDE + 3u] = 0u;
    write_count_indirect(SCR_INDIRECT_PREPARE, np_all, 64u);
    write_count_indirect(
        SCR_INDIRECT_ISLAND,
        max(np_all, params.joint_count),
        ISLAND_WORKGROUP_SIZE,
    );
}

@compute @workgroup_size(64)
fn finish_occupied_contacts(@builtin(global_invocation_id) dispatch_gid: vec3<u32>,
                            @builtin(num_workgroups) groups: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let count = min(atomicLoad(&atom[ATOM_OCCUPIED_N]), params.contact_capacity);
    // Retirement and collection are complete before this dispatch. Publish
    // the new list without overwriting any input being read by those passes.
    // Strided ownership preserves list order across workgroups. Consumers run
    // after this dispatch completes; they cannot use the count mid-publication.
    for (var i = gid.x; i < count; i += groups.x * groups.y * 64u) {
        scratch[scr_occupied_contact() + i] = scratch[scr_next_occupied() + i];
    }
    if (gid.x != 0u) { return; }
    scratch[SCR_OCCUPIED_N] = count;
    // Dynamic colors wider than one workgroup remain parallel next step.
    // Static colors always remain parallel; query word 71 is this hint only.
    var prefix = 0u;
    for (var col = 0u; col < DYNAMIC_COLOR_COUNT; col++) {
        if (scratch[SCR_COLOR + col] > 64u) { prefix = col + 1u; }
    }
    atomicStore(&query[71u], prefix);
}

@compute @workgroup_size(1)
fn color_and_compact() {
    let np = min(scratch[SCR_NCONTACTS], pair_cap());
    let occupancy = color_body_base();
    // Box3D removes stopped constraints with swap-with-last. This preserves
    // insertion order for untouched constraints and updates exactly one moved
    // local index instead of rebuilding every color from pair-key order.
    for (var col = 0u; col < 24u; col++) {
        var i = 0u;
        loop {
            let count = scratch[SCR_COLOR + col];
            if (i >= count) {
                break;
            }
            let slot = scratch[color_contact_base() + col * params.contact_capacity + i];
            var c = load_contact(slot);
            let keep = c.a != EMPTY
                && c.manifold_link.y == 0u
                && c.count > 0u
                && (c.lifecycle.y & CONTACT_TOUCHING) != 0u
                && !contact_has_sensor(c)
                && scratch[scr_contact_mark() + slot] != 0u
                && (params.diagnostic_flags & DIAG_REBUILD_GRAPH) == 0u;
            if (keep) {
                i = i + 1u;
                continue;
            }
            if (col != OVERFLOW_COLOR && c.a != EMPTY) {
                let bit = ~(1u << col);
                if (!is_non_dynamic(load_body(c.a))) {
                    scratch[occupancy + c.a] = scratch[occupancy + c.a] & bit;
                }
                if (!is_non_dynamic(load_body(c.b))) {
                    scratch[occupancy + c.b] = scratch[occupancy + c.b] & bit;
                }
            }
            let last = count - 1u;
            if (i != last) {
                let moved_slot =
                    scratch[color_contact_base() + col * params.contact_capacity + last];
                scratch[color_contact_base() + col * params.contact_capacity + i] = moved_slot;
                var moved = load_contact(moved_slot);
                moved.lifecycle.z = i;
                store_contact(moved_slot, moved);
            }
            scratch[SCR_COLOR + col] = last;
            if (c.a != EMPTY) {
                c.lifecycle.z = EMPTY;
                store_contact(slot, c);
            }
        }
    }
    // Add only newly touching contacts. Existing contacts retain both their
    // color and local index until the stop-touching transition above.
    for (var i = 0u; i < np; i++) {
        let slot = scratch[scr_active_contact() + i];
        if (slot == EMPTY) {
            continue;
        }
        var c = load_contact(slot);
        if (c.a == EMPTY || c.count == 0u || c.manifold_link.y != 0u || contact_has_sensor(c)) {
            continue;
        }
        let local = c.lifecycle.z;
        if (local != EMPTY
            && c.color < 24u
            && local < scratch[SCR_COLOR + c.color]
            && scratch[color_contact_base() + c.color * params.contact_capacity + local] == slot) {
            continue;
        }
        let a_dyn = !is_non_dynamic(load_body(c.a));
        let b_dyn = !is_non_dynamic(load_body(c.b));
        var col = OVERFLOW_COLOR;
        if (a_dyn && b_dyn) {
            for (var i = 0u; i < DYNAMIC_COLOR_COUNT; i++) {
                let bit = 1u << i;
                if ((scratch[occupancy + c.a] & bit) != 0u
                    || (scratch[occupancy + c.b] & bit) != 0u) {
                    continue;
                }
                scratch[occupancy + c.a] = scratch[occupancy + c.a] | bit;
                scratch[occupancy + c.b] = scratch[occupancy + c.b] | bit;
                col = i;
                break;
            }
        } else {
            let dyn = select(c.b, c.a, a_dyn);
            var i = OVERFLOW_COLOR - 1u;
            loop {
                if (i < 1u) {
                    break;
                }
                let bit = 1u << i;
                if ((scratch[occupancy + dyn] & bit) == 0u) {
                    scratch[occupancy + dyn] = scratch[occupancy + dyn] | bit;
                    col = i;
                    break;
                }
                i = i - 1u;
            }
        }
        c.color = col;
        let cnt = scratch[SCR_COLOR + col];
        if (cnt < params.contact_capacity) {
            scratch[color_contact_base() + col * params.contact_capacity + cnt] = slot;
            scratch[SCR_COLOR + col] = cnt + 1u;
            c.lifecycle.z = cnt;
        }
        store_contact(slot, c);
    }
    var mx = 0u;
    for (var c = 0u; c < OVERFLOW_COLOR; c++) {
        mx = max(mx, (scratch[SCR_COLOR + c] + 63u) / 64u);
    }
    if (params.solver_mode == SOLVER_JACOBI) {
        write_group_indirect(SCR_INDIRECT_COLLIDE, (np + 63u) / 64u);
    } else {
        let use_one_group_wave = np > 0u && mx <= 1u;
        write_group_indirect(SCR_INDIRECT_COLLIDE, select(0u, 1u, use_one_group_wave));
        for (var c = 0u; c < 24u; c++) {
            let n = scratch[SCR_COLOR + c];
            var groups = (n + 63u) / 64u;
            if (c == OVERFLOW_COLOR) {
                groups = select(0u, 1u, n > 0u);
            }
            let base = SCR_INDIRECT_COLOR + c * 4u;
            write_group_indirect(base, select(groups, 0u, use_one_group_wave));
        }
    }
    scratch[SCR_INDIRECT_COLLIDE + 3u] = 0u;
    write_count_indirect(SCR_INDIRECT_PREPARE, np, 64u);
    write_count_indirect(
        SCR_INDIRECT_ISLAND,
        max(np, params.joint_count),
        ISLAND_WORKGROUP_SIZE,
    );
    if ((params.diagnostic_flags & DIAG_PHASE_CAPTURE) != 0u) {
        var errors = 0u;
        for (var col = 0u; col < 24u; col++) {
            let count = scratch[SCR_COLOR + col];
            for (var i = 0u; i < count; i++) {
                let slot = scratch[color_contact_base() + col * params.contact_capacity + i];
                let c = load_contact(slot);
                if (c.a == EMPTY || c.count == 0u || c.color != col || c.lifecycle.z != i) {
                    errors = errors + 1u;
                }
                if (col == OVERFLOW_COLOR) {
                    continue;
                }
                for (var j = i + 1u; j < count; j++) {
                    let d =
                        load_contact(
                            scratch[color_contact_base() + col * params.contact_capacity + j]
                        );
                    if ((!is_non_dynamic(load_body(c.a)) && (c.a == d.a || c.a == d.b))
                        || (!is_non_dynamic(load_body(c.b)) && (c.b == d.a || c.b == d.b))) {
                        errors = errors + 1u;
                    }
                }
            }
        }
        for (var slot = 0u; slot < params.contact_capacity; slot++) {
            let c = load_contact(slot);
            if (c.a == EMPTY || c.count == 0u
                || (c.lifecycle.y & CONTACT_TOUCHING) == 0u) {
                continue;
            }
            if (c.color >= 24u || c.lifecycle.z >= scratch[SCR_COLOR + c.color]
                || scratch[
                    color_contact_base() + c.color * params.contact_capacity + c.lifecycle.z
                ] != slot) {
                errors = errors + 1u;
            }
        }
        scratch[SCR_BSTRIDE] = errors;
    }
}

// Topology changes only: preserve physical slot ownership while replacing dense
// collider keys. Mapping entries identify exact surviving shape generations.
@compute @workgroup_size(64)
fn clear_remapped_contact_hash(@builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(num_workgroups) groups: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    for (var i = gid.x; i < contact_hash_cap(); i += groups.x * groups.y * 64u) {
        atomicStore(&atom[atom_contact_key() + i], EMPTY);
        atomicStore(&atom[atom_contact_identity() + i], EMPTY);
    }
}

@compute @workgroup_size(64)
fn remap_contact_shapes(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let slot = gid.x;
    if (slot >= params.contact_capacity) { return; }
    var c = load_contact(slot);
    if (params.remap_capture != 0u) {
        var previous = vec2<u32>(EMPTY);
        if (c.a != EMPTY && c.count != 0u && (c.lifecycle.y & CONTACT_TOUCHING) != 0u) {
            previous = c.pair.xy;
        }
        store_pair_words(scr_previous_touching(), slot, previous);
    }
    if (c.a == EMPTY) { return; }
    let old_a = c.pair.x;
    let old_b = c.pair.y;
    var a = EMPTY;
    var b = EMPTY;
    if (old_a < params.remap_old_count) { a = scratch[scr_radix_out() + old_a]; }
    if (old_b < params.remap_old_count) { b = scratch[scr_radix_out() + old_b]; }
    if (a == EMPTY || b == EMPTY) {
        // Every patch owns its slot; children retire independently in this pass.
        var retired = empty_contact();
        retired.lifecycle.x = c.lifecycle.x;
        store_contact(slot, retired);
    } else {
        c.pair = vec4<u32>(min(a, b), max(a, b), 0u, 0u);
        store_contact(slot, c);
    }
}

@compute @workgroup_size(64)
fn prepare_contact_hash_keys(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let slot = gid.x;
    if (slot >= params.contact_capacity) { return; }
    if (contacts[slot].a != EMPTY && contacts[slot].manifold_link.y == 0u) {
        prepare_contact_identity(slot, contact_persistent[slot].pair.xy);
    }
}

@compute @workgroup_size(64)
fn publish_remapped_contacts(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let slot = gid.x;
    if (slot >= params.contact_capacity) { return; }
    let c = contacts[slot];
    if (c.a != EMPTY && c.manifold_link.y == 0u) {
        if (!publish_contact_slot(contact_persistent[slot].pair.xy, slot)) {
            record_contact_drop(11u);
        }
    }
}

// Mutation command, one invocation per unique root. Children have exactly one
// root owner, so retiring a manifold chain cannot race another invocation.
@compute @workgroup_size(64)
fn retire_body_pair_contacts(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    if (i >= min(scratch[SCR_UNIQUE_N], pair_cap())) { return; }
    let slot = scratch[scr_active_contact() + i];
    if (slot >= params.contact_capacity) { return; }
    let c = load_contact(slot);
    let a = atomicLoad(&query[64u]);
    let b = atomicLoad(&query[65u]);
    if (atomicLoad(&query[66u]) == 1u) {
        if ((c.pair.x) != a && (c.pair.y) != a) { return; }
    } else if (!((c.a == a && c.b == b) || (c.a == b && c.b == a))) { return; }
    if (!retire_manifold_children(slot)) { return; }
    var retired = empty_contact();
    retired.lifecycle.x = c.lifecycle.x;
    store_contact(slot, retired);
    scratch[scr_contact_mark() + slot] = 0u;
    retire_contact_key(c.pair.xy);
}

// Bounded flat-scene pair matrix: a zero bit denotes an accepted pair.
// Row is the higher shape slot, giving the same packed-key order as radix sort.
@compute @workgroup_size(64)
fn pair_matrix_build(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let stride=(params.shape_count+31u)/32u;
    let word=gid.x;
    if (word>=params.shape_count*stride) {return;}
    let hi=word/stride;let base=(word%stride)*32u;
    var rejected=EMPTY;
    if (base<hi) {
        let a=load_collider(hi);let sa=load_shape(hi);
        for(var bit=0u;bit<32u && base+bit<hi;bit++) {
            let lo=base+bit;let b=load_collider(lo);let sb=load_shape(lo);
            if (a._pad_island.y!=b._pad_island.y && broadphase_pair_allowed(a,b,sa,sb)
                && !joint_disables_collision(a._pad_island.y,b._pad_island.y)
                && shapes_collide(sa,sb) && aabb_overlap(a,b)) {
                rejected &= ~(1u<<bit);
            }
        }
    }
    atomicStore(&atom[ATOM_HASH+word],rejected);
}
@compute @workgroup_size(64)
fn pair_matrix_previous(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let key=previous_pair_key(gid.x);if(key.x==EMPTY){return;}
    let lo=key.x;let hi=key.y;
    let stride=(params.shape_count+31u)/32u;
    atomicAnd(&atom[ATOM_HASH+hi*stride+lo/32u],~(1u<<(lo%32u)));
}
@compute @workgroup_size(64)
fn pair_matrix_count(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let row=gid.x;if(row>=params.shape_count){return;}
    let stride=(params.shape_count+31u)/32u;
    var count=0u;
    for(var w=0u;w<stride;w++){count+=countOneBits(~atomicLoad(&atom[ATOM_HASH+row*stride+w]));}
    scratch[scr_ins_body()+row]=count;
}
@compute @workgroup_size(256)
fn pair_matrix_bases(@builtin(local_invocation_index) lid: u32) {
    let first=lid*3u;
    let a=select(0u,scratch[scr_ins_body()+first],first<params.shape_count);
    let b=select(0u,scratch[scr_ins_body()+first+1u],first+1u<params.shape_count);
    let c=select(0u,scratch[scr_ins_body()+first+2u],first+2u<params.shape_count);
    let base=workgroup_exclusive_scan_256(lid,a+b+c);
    if(first<params.shape_count){scratch[scr_ins_cell()+first]=base;}
    if(first+1u<params.shape_count){scratch[scr_ins_cell()+first+1u]=base+a;}
    if(first+2u<params.shape_count){scratch[scr_ins_cell()+first+2u]=base+a+b;}
    if(lid==255u){
        let total=base+a+b+c;let bounded=min(total,pair_cap());
        if(total>pair_cap()){record_capacity_drop_n(ATOM_PAIR_DROPPED,ATOM_STICKY_PAIR_DROPPED,total-pair_cap());}
        atomicStore(&atom[ATOM_PAIR_N],total);
        scratch[SCR_PAIR_N]=bounded;scratch[SCR_POW2]=total;
        scratch[SCR_UNIQUE_N]=bounded;scratch[SCR_NCONTACTS]=bounded;
        write_count_indirect(SCR_INDIRECT_COLLIDE,bounded,64u);
        write_count_indirect(SCR_INDIRECT_PREPARE,bounded,64u);
        write_count_indirect(SCR_INDIRECT_RADIX,bounded,RADIX_GROUP_SIZE);
    }
}
@compute @workgroup_size(64)
fn pair_matrix_scatter(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let hi=gid.x;if(hi>=params.shape_count){return;}
    let stride=(params.shape_count+31u)/32u;var dst=scratch[scr_ins_cell()+hi];
    for(var w=0u;w<stride;w++){
        var accepted=~atomicLoad(&atom[ATOM_HASH+hi*stride+w]);
        while(accepted!=0u){
            let bit=firstTrailingBit(accepted);accepted &= accepted-1u;
            if(dst<pair_cap()){store_pair_words(SCR_PAIRS,dst,vec2<u32>(w*32u+bit,hi));}
            dst++;
        }
    }
}
