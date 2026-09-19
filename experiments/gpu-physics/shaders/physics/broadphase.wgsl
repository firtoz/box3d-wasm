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

fn pack_pair(ia: u32, ib: u32) -> u32 {
    let lo = min(ia, ib);
    let hi = max(ia, ib);
    return (hi << 16u) | lo;
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
    let ia = c.lifecycle.w & 0xffffu;
    let ib = c.lifecycle.w >> 16u;
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
    let packed = (key_a << 16u) | (key_b & 0xffffu);
    var h = pair_hash_mix(pair_hash_mix(key_a) ^ key_b);
    let base = joint_filter_base();
    for (var probe = 0u; probe < JOINT_FILTER_PROBE; probe++) {
        let idx = h & (JOINT_FILTER_CAP - 1u);
        let stored = scratch[base + idx * 2u];
        if (stored == EMPTY) {
            return false;
        }
        if (stored == packed) {
            return scratch[base + idx * 2u + 1u] != 0u;
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

fn push_pair(packed: u32) {
    // Deduplicate before append. Dense lattices emit the same neighboring pair
    // from several occupied cells; appending all duplicates exhausted PAIR_CAP
    // even though the final unique set fit. Sorting still canonicalizes order.
    var hash_slot = pair_hash_mix(packed) & (PAIR_CAP - 1u);
    for (var probe = 0u; probe < PAIR_CAP; probe++) {
        let stored = atomicLoad(&atom[ATOM_PAIR_SET + hash_slot]);
        if (stored == packed) {
            return;
        }
        if (stored == EMPTY) {
            let claimed = atomicCompareExchangeWeak(
                &atom[ATOM_PAIR_SET + hash_slot],
                EMPTY,
                packed,
            );
            if (claimed.exchanged) {
                let i = atomicAdd(&atom[ATOM_PAIR_N], 1u);
                if (i < PAIR_CAP) {
                    scratch[SCR_PAIRS + i] = packed;
                } else {
                    record_capacity_drop(ATOM_PAIR_DROPPED, ATOM_STICKY_PAIR_DROPPED);
                }
                return;
            }
            if (claimed.old_value == packed) {
                return;
            }
        }
        hash_slot = (hash_slot + 1u) & (PAIR_CAP - 1u);
    }
    record_capacity_drop(ATOM_PAIR_DROPPED, ATOM_STICKY_PAIR_DROPPED);
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
fn clear_broadphase(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i < params.shape_count) { update_fat_bounds(i); }
    if (i < HASH_BUCKETS) {
        atomicStore(&atom[ATOM_HASH + i], EMPTY);
    }
    if (i < PAIR_CAP) {
        scratch[SCR_PAIRS + i] = EMPTY;
        scratch[SCR_CONTACT_MARK + i] = 0u;
        // Latest-step event history must survive retirement and slot reuse.
        // Capture before narrowphase writes; child patches share their pair key.
        var previous_key = EMPTY;
        if (i < params.contact_capacity) {
            let old = contacts[i];
            if (old.a != EMPTY && old.count > 0u
                && (contact_persistent[i].lifecycle.y & CONTACT_TOUCHING) != 0u) {
                previous_key = contact_persistent[i].lifecycle.w;
            }
        }
        if (params.remap_history_step != params.physics_step) {
            scratch[SCR_PREVIOUS_TOUCHING + i] = previous_key;
        }
        atomicStore(&atom[ATOM_PAIR_SET + i], EMPTY);
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
const SCR_STATIC_LIST: u32 = SCR_RADIX_OUT;

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
fn collect_fat_statics(@builtin(global_invocation_id) gid: vec3<u32>) {
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
    if (out < PAIR_CAP) {
        scratch[SCR_STATIC_LIST + out] = i;
    }
}

@compute @workgroup_size(1)
fn finish_fat_statics() {
    scratch[SCR_STATIC_N] = atomicLoad(&atom[ATOM_STATIC_N]);
}

@compute @workgroup_size(64)
fn hash_insert(@builtin(global_invocation_id) gid: vec3<u32>) {
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
    let accepted = min(insert_count, MAX_INSERTS - min(first_slot, MAX_INSERTS));
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
                scratch[SCR_INS_BODY + slot] = i;
                scratch[SCR_INS_CELL + slot] = h;
                let old = atomicExchange(&atom[ATOM_HASH + h], slot);
                scratch[SCR_INS_NEXT + slot] = old;
            }
        }
    }
}

@compute @workgroup_size(1)
fn write_insert_indirect() {
    let n = min(atomicLoad(&atom[ATOM_INSERT_N]), MAX_INSERTS);
    scratch[SCR_INDIRECT_COLLIDE] = (n + 63u) / 64u;
    scratch[SCR_INDIRECT_COLLIDE + 1u] = 1u;
    scratch[SCR_INDIRECT_COLLIDE + 2u] = 1u;
    scratch[SCR_INDIRECT_COLLIDE + 3u] = 0u;
}

@compute @workgroup_size(256)
fn write_radix_indirect(@builtin(local_invocation_index) lid: u32) {
    let n = min(atomicLoad(&atom[ATOM_PAIR_N]), PAIR_CAP);
    if (lid == 0u) {
        scratch[SCR_PAIR_N] = n;
        let groups = max((n + RADIX_GROUP_SIZE - 1u) / RADIX_GROUP_SIZE, 1u);
        scratch[SCR_RADIX_GROUP_N] = groups;
        write_count_indirect(SCR_INDIRECT_RADIX, n, RADIX_GROUP_SIZE);
    }
    scratch[SCR_RADIX_BASE + lid] = 0u;
}

@compute @workgroup_size(64)
fn emit_hash_pairs(@builtin(global_invocation_id) gid: vec3<u32>) {
    let slot = gid.x;
    let nins = min(atomicLoad(&atom[ATOM_INSERT_N]), MAX_INSERTS);
    if (slot >= nins) {
        return;
    }
    let ia = scratch[SCR_INS_BODY + slot];
    let h = scratch[SCR_INS_CELL + slot];
    let a = load_collider(ia);
    var other = atomicLoad(&atom[ATOM_HASH + h]);
    var hops = 0u;
    loop {
        if (other == EMPTY || hops >= 2048u) {
            if (other != EMPTY) {
                record_capacity_drop(ATOM_HASH_HOP_DROPPED, ATOM_STICKY_HASH_HOP_DROPPED);
            }
            break;
        }
        let ib = scratch[SCR_INS_BODY + other];
        if (ib < ia) {
            let b = load_collider(ib);
            let shape_a = load_shape(ia);
            let shape_b = load_shape(ib);
            if (a._pad_island.y != b._pad_island.y
                && broadphase_pair_allowed(a, b, shape_a, shape_b)
                && !joint_disables_collision(a._pad_island.y, b._pad_island.y)
                && shapes_collide(shape_a, shape_b)
                && aabb_overlap(a, b)) {
                push_pair(pack_pair(ia, ib));
            }
        }
        other = scratch[SCR_INS_NEXT + other];
        hops = hops + 1u;
    }
}

@compute @workgroup_size(64)
fn emit_static_pairs(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= params.shape_count) {
        return;
    }
    let a = load_collider(i);
    if (is_static(a)) {
        return;
    }
    let nstatic = min(scratch[SCR_STATIC_N], PAIR_CAP);
    for (var j = 0u; j < nstatic; j++) {
        let jb = scratch[SCR_STATIC_LIST + j];
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
            push_pair(pack_pair(i, jb));
        }
    }
}

@compute @workgroup_size(1)
fn write_occupied_indirect() {
    let n = min(scratch[SCR_OCCUPIED_N], params.contact_capacity);
    scratch[SCR_INDIRECT_OCCUPIED] = (n + 63u) / 64u;
    scratch[SCR_INDIRECT_OCCUPIED + 1u] = 1u;
    scratch[SCR_INDIRECT_OCCUPIED + 2u] = 1u;
    scratch[SCR_INDIRECT_OCCUPIED + 3u] = 0u;
}

fn previous_pair_key(i: u32) -> u32 {
    let occupied_count = min(scratch[SCR_OCCUPIED_N], params.contact_capacity);
    if (i >= occupied_count) {
        return EMPTY;
    }
    let k = scratch[SCR_OCCUPIED_CONTACT + i];
    if (k >= params.contact_capacity) {
        return EMPTY;
    }
    let p = load_contact(k);
    let key = p.lifecycle.w;
    let ia = key & 0xffffu;
    let ib = key >> 16u;
    if (p.a == EMPTY || p.b == EMPTY || p.a == p.b
        || ia >= params.shape_count || ib >= params.shape_count) {
        return EMPTY;
    }
    let a = load_collider(ia);
    let b = load_collider(ib);
    let shape_a = load_shape(ia);
    let shape_b = load_shape(ib);
    if (!broadphase_pair_allowed(a, b, shape_a, shape_b)) {
        return EMPTY;
    }
    if (joint_disables_collision(a._pad_island.y, b._pad_island.y)) {
        return EMPTY;
    }
    if (!shapes_collide(shape_a, shape_b)) {
        return EMPTY;
    }
    if (!pair_lifetime_overlap(a, b, shape_a, shape_b)) {
        return EMPTY;
    }
    return key;
}

@compute @workgroup_size(64)
fn emit_prev_pairs(@builtin(global_invocation_id) gid: vec3<u32>) {
    let key=previous_pair_key(gid.x);
    if (key!=EMPTY) { push_pair(key); }
}

var<workgroup> radix_counts: array<atomic<u32>, 256>;
var<workgroup> radix_keys: array<u32, 256>;
var<workgroup> radix_scan_a: array<u32, 256>;
var<workgroup> radix_scan_b: array<u32, 256>;

fn retained_pair_count() -> u32 {
    return min(atomicLoad(&atom[ATOM_PAIR_N]), PAIR_CAP);
}

fn pair_is_unique(i: u32, n: u32) -> u32 {
    if (i >= n) {
        return 0u;
    }
    let p = scratch[SCR_PAIRS + i];
    if (p == EMPTY) {
        return 0u;
    }
    if (i == 0u) {
        return 1u;
    }
    return select(0u, 1u, p != scratch[SCR_PAIRS + i - 1u]);
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

@compute @workgroup_size(256)
fn compact_unique_histogram(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    let n = retained_pair_count();
    let keep = pair_is_unique(gid.x, n);
    let exclusive = workgroup_exclusive_scan_256(lid, keep);
    if (lid == 255u) {
        scratch[SCR_RADIX_BASE + group.x] = exclusive + keep;
    }
}

@compute @workgroup_size(256)
fn compact_unique_bases(@builtin(local_invocation_index) lid: u32) {
    // compact_unique_histogram writes only the live workgroup prefix.
    // Radix leftover offsets in the unused suffix must not be scanned.
    let groups = (retained_pair_count() + RADIX_GROUP_SIZE - 1u) / RADIX_GROUP_SIZE;
    let count = select(0u, scratch[SCR_RADIX_BASE + lid], lid < groups);
    let exclusive = workgroup_exclusive_scan_256(lid, count);
    scratch[SCR_RADIX_HIST + lid] = exclusive;
    if (lid == 255u) {
        let total = exclusive + count;
        scratch[SCR_POW2] = total;
        if (total > PAIR_CAP) {
            record_capacity_drop_n(ATOM_PAIR_DROPPED, ATOM_STICKY_PAIR_DROPPED, total - PAIR_CAP);
        }
        let bounded = min(total, PAIR_CAP);
        scratch[SCR_UNIQUE_N] = bounded;
        scratch[SCR_NCONTACTS] = bounded;
        write_count_indirect(SCR_INDIRECT_COLLIDE, bounded, 64u);
        write_count_indirect(SCR_INDIRECT_PREPARE, bounded, 64u);
    }
}

@compute @workgroup_size(256)
fn compact_unique_scatter(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    let n = retained_pair_count();
    let keep = pair_is_unique(gid.x, n);
    let local_exclusive = workgroup_exclusive_scan_256(lid, keep);
    if (keep != 0u) {
        let dst = scratch[SCR_RADIX_HIST + group.x] + local_exclusive;
        scratch[SCR_RADIX_OUT + dst] = scratch[SCR_PAIRS + gid.x];
    }
}

@compute @workgroup_size(256)
fn compact_unique_gather(@builtin(global_invocation_id) gid: vec3<u32>) {
    let w = min(scratch[SCR_UNIQUE_N], PAIR_CAP);
    if (gid.x < w) {
        scratch[SCR_PAIRS + gid.x] = scratch[SCR_RADIX_OUT + gid.x];
    }
}

fn radix_histogram_impl(gid: u32, lid: u32, group: u32, shift: u32, from_output: bool) {
    atomicStore(&radix_counts[lid], 0u);
    workgroupBarrier();
    let n = retained_pair_count();
    if (gid < n) {
        let key = select(
            scratch[SCR_PAIRS + gid],
            scratch[SCR_RADIX_OUT + gid],
            from_output,
        );
        atomicAdd(&radix_counts[(key >> shift) & 255u], 1u);
    }
    workgroupBarrier();
    scratch[SCR_RADIX_HIST + group * RADIX_BUCKETS + lid] =
        atomicLoad(&radix_counts[lid]);
}

@compute @workgroup_size(256)
fn radix_histogram_0(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    radix_histogram_impl(gid.x, lid, group.x, 0u, false);
}

@compute @workgroup_size(256)
fn radix_histogram_8(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    radix_histogram_impl(gid.x, lid, group.x, 8u, true);
}

@compute @workgroup_size(256)
fn radix_histogram_16(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    radix_histogram_impl(gid.x, lid, group.x, 16u, false);
}

@compute @workgroup_size(256)
fn radix_histogram_24(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    radix_histogram_impl(gid.x, lid, group.x, 24u, true);
}

fn radix_live_groups() -> u32 {
    return min(scratch[SCR_RADIX_GROUP_N], RADIX_GROUPS);
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
        total = total + scratch[SCR_RADIX_HIST + group * RADIX_BUCKETS + bucket];
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
    scratch[SCR_RADIX_BASE + bucket] = base;
}

@compute @workgroup_size(256)
fn radix_group_prefix(@builtin(local_invocation_index) bucket: u32) {
    let groups = radix_live_groups();
    if (groups == 0u) {
        return;
    }
    var offset = scratch[SCR_RADIX_BASE + bucket];
    for (var group = 0u; group < groups; group++) {
        let at = SCR_RADIX_HIST + group * RADIX_BUCKETS + bucket;
        let count = scratch[at];
        scratch[at] = offset;
        offset = offset + count;
    }
}

fn radix_scatter_impl(gid: u32, lid: u32, group: u32, shift: u32, from_output: bool) {
    let n = retained_pair_count();
    var key = EMPTY;
    if (gid < n) {
        key = select(
            scratch[SCR_PAIRS + gid],
            scratch[SCR_RADIX_OUT + gid],
            from_output,
        );
    }
    radix_keys[lid] = key;
    workgroupBarrier();
    if (gid < n) {
        let bucket = (key >> shift) & 255u;
        var rank = 0u;
        for (var i = 0u; i < lid; i++) {
            if (((radix_keys[i] >> shift) & 255u) == bucket) {
                rank = rank + 1u;
            }
        }
        let out = scratch[SCR_RADIX_HIST + group * RADIX_BUCKETS + bucket] + rank;
        if (out >= n) {
            record_capacity_drop(ATOM_PAIR_DROPPED, ATOM_STICKY_PAIR_DROPPED);
        } else if (from_output) {
            scratch[SCR_PAIRS + out] = key;
        } else {
            scratch[SCR_RADIX_OUT + out] = key;
        }
    }
}

@compute @workgroup_size(256)
fn radix_scatter_0(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    radix_scatter_impl(gid.x, lid, group.x, 0u, false);
}

@compute @workgroup_size(256)
fn radix_scatter_8(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    radix_scatter_impl(gid.x, lid, group.x, 8u, true);
}

@compute @workgroup_size(256)
fn radix_scatter_16(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    radix_scatter_impl(gid.x, lid, group.x, 16u, false);
}

@compute @workgroup_size(256)
fn radix_scatter_24(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    radix_scatter_impl(gid.x, lid, group.x, 24u, true);
}

// Read-only probes are safe in parallel. New contact-table claims remain
// ordered by the sorted pair list so tombstone reuse cannot change persistent
// pair identity with workgroup scheduling.
@compute @workgroup_size(64)
fn find_existing_contact_slots(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    let count = min(scratch[SCR_UNIQUE_N], PAIR_CAP);
    if (i >= count) {
        return;
    }
    let slot = find_contact_slot(scratch[SCR_PAIRS + i]);
    scratch[SCR_ACTIVE_CONTACT + i] = slot;
    if (slot != EMPTY) {
        if (!contact_chain_structure_valid(slot)) { return; }
        var member = slot;
        let count = max(contacts[slot].manifold_link.z, 1u);
        for (var j = 0u; j < count; j++) {
            scratch[SCR_CONTACT_MARK + member] = 1u;
            let next = contacts[member].manifold_link.x;
            if (next != 0u) { member = next - 1u; }
        }
    }
}

fn slot_is_free(slot: u32) -> u32 {
    if (slot >= params.contact_capacity) {
        return 0u;
    }
    return select(0u, 1u, contacts[slot].a == EMPTY && scratch[SCR_CONTACT_MARK + slot] == 0u);
}

@compute @workgroup_size(256)
fn alloc_free_histogram(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    let n = params.contact_capacity;
    let keep = select(0u, slot_is_free(gid.x), gid.x < n);
    let exclusive = workgroup_exclusive_scan_256(lid, keep);
    if (lid == 255u) {
        scratch[SCR_RADIX_BASE + group.x] = exclusive + keep;
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
    let groups = (params.contact_capacity + RADIX_GROUP_SIZE - 1u) / RADIX_GROUP_SIZE;
    let count = select(0u, scratch[SCR_RADIX_BASE + lid], lid < groups);
    let exclusive = workgroup_exclusive_scan_256(lid, count);
    scratch[SCR_RADIX_BASE + lid] = exclusive;
    if (lid == 255u) {
        scratch[SCR_FREE_N] = exclusive + count;
    }
}

@compute @workgroup_size(256)
fn alloc_free_scatter(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    let n = params.contact_capacity;
    let keep = select(0u, slot_is_free(gid.x), gid.x < n);
    let local_exclusive = workgroup_exclusive_scan_256(lid, keep);
    if (keep != 0u) {
        let dst = scratch[SCR_RADIX_BASE + group.x] + local_exclusive;
        scratch[SCR_RADIX_OUT + dst] = gid.x;
    }
}

fn pair_needs_slot(i: u32, n: u32) -> u32 {
    if (i >= n) {
        return 0u;
    }
    return select(0u, 1u, scratch[SCR_ACTIVE_CONTACT + i] == EMPTY);
}

@compute @workgroup_size(256)
fn alloc_missing_histogram(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    let n = min(scratch[SCR_UNIQUE_N], PAIR_CAP);
    let keep = pair_needs_slot(gid.x, n);
    let exclusive = workgroup_exclusive_scan_256(lid, keep);
    if (lid == 255u) {
        scratch[SCR_RADIX_BASE + group.x] = exclusive + keep;
    }
}

@compute @workgroup_size(256)
fn alloc_missing_bases(@builtin(local_invocation_index) lid: u32) {
    let n = min(scratch[SCR_UNIQUE_N], PAIR_CAP);
    let groups = (n + RADIX_GROUP_SIZE - 1u) / RADIX_GROUP_SIZE;
    let count = select(0u, scratch[SCR_RADIX_BASE + lid], lid < groups);
    let exclusive = workgroup_exclusive_scan_256(lid, count);
    scratch[SCR_RADIX_BASE + lid] = exclusive;
    if (lid == 255u) {
        scratch[SCR_MISSING_N] = exclusive + count;
    }
}

@compute @workgroup_size(256)
fn alloc_missing_scatter(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    let n = min(scratch[SCR_UNIQUE_N], PAIR_CAP);
    let keep = pair_needs_slot(gid.x, n);
    let local_exclusive = workgroup_exclusive_scan_256(lid, keep);
    if (keep != 0u) {
        let dst = scratch[SCR_RADIX_BASE + group.x] + local_exclusive;
        scratch[SCR_RADIX_HIST + dst] = gid.x;
    }
}

@compute @workgroup_size(64)
fn alloc_bind_slots(@builtin(global_invocation_id) gid: vec3<u32>) {
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
    let pair_i = scratch[SCR_RADIX_HIST + i];
    let slot = scratch[SCR_RADIX_OUT + i];
    let key = scratch[SCR_PAIRS + pair_i];
    scratch[SCR_ACTIVE_CONTACT + pair_i] = slot;
    scratch[SCR_CONTACT_MARK + slot] = 1u;
    if (!publish_contact_slot(key, slot)) {
        scratch[SCR_ACTIVE_CONTACT + pair_i] = EMPTY;
        scratch[SCR_CONTACT_MARK + slot] = 0u;
        record_contact_drop(8u);
    }
}

@compute @workgroup_size(64)
fn retire_stale_contacts(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    let occupied_count = min(scratch[SCR_OCCUPIED_N], params.contact_capacity);
    if (i >= occupied_count) {
        return;
    }
    let slot = scratch[SCR_OCCUPIED_CONTACT + i];
    if (slot >= params.contact_capacity || scratch[SCR_CONTACT_MARK + slot] != 0u) {
        return;
    }
    let c = load_contact(slot);
    if (c.a == EMPTY) {
        return;
    }
    let key = c.lifecycle.w;
    let ia = key & 0xffffu;
    let ib = key >> 16u;
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
        retire_contact_key(c.lifecycle.w);
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
            scratch[SCR_NEXT_OCCUPIED + out] = slot;
        }
    }
}

fn native_countable_root(slot: u32) -> bool {
    let key = contact_persistent[slot].lifecycle.w;
    let a = key & 0xffffu;
    let b = key >> 16u;
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
fn collect_occupied_contacts(@builtin(global_invocation_id) gid: vec3<u32>,
                             @builtin(local_invocation_index) lid: u32) {
    if (lid == 0u) {
        atomicStore(&metric_patch_sum, 0u);
        atomicStore(&metric_touching_sum, 0u);
        atomicStore(&metric_non_sensor_sum, 0u);
    }
    workgroupBarrier();
    let i = gid.x;
    let count = min(scratch[SCR_UNIQUE_N], PAIR_CAP);
    if (i < count) {
        let slot = scratch[SCR_ACTIVE_CONTACT + i];
        if (slot != EMPTY) {
            atomicAdd(&metric_patch_sum, max(contacts[slot].manifold_link.z, 1u));
            if (native_countable_root(slot)) { atomicAdd(&metric_non_sensor_sum, 1u); }
            if (contacts[slot].count > 0u && (contact_persistent[slot].lifecycle.y & CONTACT_TOUCHING) != 0u) {
                atomicAdd(&metric_touching_sum, 1u);
            }
            let out = atomicAdd(&atom[ATOM_OCCUPIED_N], 1u);
            if (out < params.contact_capacity) {
                scratch[SCR_NEXT_OCCUPIED + out] = slot;
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
    if (scratch[SCR_CONTACT_MARK + slot] == 0u) {
        return false;
    }
    let ia = life.w & 0xffffu;
    let ib = life.w >> 16u;
    return ia < params.shape_count && ib < params.shape_count
        && ((load_shape(ia).event_flags | load_shape(ib).event_flags) & SHAPE_IS_SENSOR) == 0u;
}

@compute @workgroup_size(1)
fn write_unique_indirect() {
    let n = min(scratch[SCR_UNIQUE_N], PAIR_CAP);
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
        (params.contact_capacity + RADIX_GROUP_SIZE - 1u) / RADIX_GROUP_SIZE,
        need_free_list,
    );
    scratch[SCR_INDIRECT_RADIX] = free_groups;
    scratch[SCR_INDIRECT_RADIX + 1u] = 1u;
    scratch[SCR_INDIRECT_RADIX + 2u] = 1u;
    scratch[SCR_INDIRECT_RADIX + 3u] = 0u;
    if (!need_free_list) {
        scratch[SCR_FREE_N] = 0u;
    }
}

@compute @workgroup_size(64)
fn graph_clear_meta(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= params.body_count) {
        return;
    }
    atomicStore(&atom[ATOM_JACOBI + i], 0u);
    atomicStore(&atom[ATOM_JACOBI + params.body_count + i], select(0u,EMPTY,(params.diagnostic_flags & DIAG_STATIC_DEGREE_TWO_PROOF)!=0u));
    atomicStore(&atom[ATOM_JACOBI + 2u * params.body_count + i], 0u);
    atomicStore(&atom[ATOM_JACOBI + 3u * params.body_count + i], 0u);
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
    return scratch[SCR_RADIX_HIST + unique_i] & 3u;
}

fn graph_dyn_body(unique_i: u32) -> u32 {
    return scratch[SCR_RADIX_HIST + unique_i] >> 2u;
}

@compute @workgroup_size(64)
fn graph_classify(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    let np = min(scratch[SCR_UNIQUE_N], PAIR_CAP);
    if (i >= np) {
        return;
    }
    let slot = scratch[SCR_ACTIVE_CONTACT + i];
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
            atomicOr(&atom[ATOM_JACOBI + 2u * params.body_count + h.a], 1u);
            atomicOr(&atom[ATOM_JACOBI + 2u * params.body_count + h.b], 1u);
        }
    }
    scratch[SCR_RADIX_HIST + i] = packed;
}

@compute @workgroup_size(64)
fn graph_mark_edges(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= params.joint_count) {
        return;
    }
    let j = joints[i];
    if (j.kind != JOINT_NONE && j.kind != JOINT_FILTER) {
        if (j.a < params.body_count) {
            atomicOr(&atom[ATOM_JACOBI + 2u * params.body_count + j.a], 1u);
        }
        if (j.b < params.body_count) {
            atomicOr(&atom[ATOM_JACOBI + 2u * params.body_count + j.b], 1u);
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
const SCR_GRAPH_PAIRED_PREFIX:u32=SCR_INS_CELL;
@compute @workgroup_size(256)
fn graph_compact_paired_histogram(@builtin(global_invocation_id) gid:vec3<u32>,
    @builtin(local_invocation_index) lid:u32,@builtin(workgroup_id) group:vec3<u32>) {
    let n=min(scratch[SCR_UNIQUE_N],PAIR_CAP);
    for (var kind=1u;kind<=2u;kind++) {
        let keep=graph_keep_kind(gid.x,n,kind);
        let exclusive=workgroup_exclusive_scan_256(lid,keep);
        if (lid==255u) {scratch[SCR_GRAPH_PAIRED_PREFIX+(kind-1u)*RADIX_GROUPS+group.x]=exclusive+keep;}
    }
}
@compute @workgroup_size(256)
fn graph_compact_paired_bases(@builtin(local_invocation_index) lid:u32) {
    let n=min(scratch[SCR_UNIQUE_N],PAIR_CAP);
    let groups=(n+RADIX_GROUP_SIZE-1u)/RADIX_GROUP_SIZE;
    for (var kind=1u;kind<=2u;kind++) {
        let base=SCR_GRAPH_PAIRED_PREFIX+(kind-1u)*RADIX_GROUPS;
        let count=select(0u,scratch[base+lid],lid<groups);
        let exclusive=workgroup_exclusive_scan_256(lid,count);
        scratch[base+lid]=exclusive;
        if (lid==255u) {scratch[select(SCR_GRAPH_STATIC_N,SCR_DYN_DYN_N,kind==2u)]=min(exclusive+count,PAIR_CAP);}
    }
}
@compute @workgroup_size(256)
fn graph_compact_paired_scatter(@builtin(global_invocation_id) gid:vec3<u32>,
    @builtin(local_invocation_index) lid:u32,@builtin(workgroup_id) group:vec3<u32>) {
    let n=min(scratch[SCR_UNIQUE_N],PAIR_CAP);
    for (var kind=1u;kind<=2u;kind++) {
        let keep=graph_keep_kind(gid.x,n,kind);
        let exclusive=workgroup_exclusive_scan_256(lid,keep);
        if (keep!=0u) {
            let dst=scratch[SCR_GRAPH_PAIRED_PREFIX+(kind-1u)*RADIX_GROUPS+group.x]+exclusive;
            if (dst<PAIR_CAP) {scratch[select(SCR_RADIX_OUT,SCR_NEXT_OCCUPIED,kind==2u)+dst]=gid.x;}
        }
    }
}

@compute @workgroup_size(64)
fn graph_count_static_degree(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    let n = min(scratch[SCR_GRAPH_STATIC_N], PAIR_CAP);
    if (i >= n) {
        return;
    }
    let unique_i = scratch[SCR_RADIX_OUT + i];
    let dyn = graph_dyn_body(unique_i);
    if (dyn < params.body_count) {
        let prev = atomicAdd(&atom[ATOM_JACOBI + 3u * params.body_count + dyn], 1u);
        atomicMax(&atom[atom_dyn_dyn()], prev + 1u);
        if ((params.diagnostic_flags & DIAG_STATIC_DEGREE_TWO_PROOF)!=0u) {
            atomicMin(&atom[ATOM_JACOBI+params.body_count+dyn],unique_i);
        }
    }
}

@compute @workgroup_size(1)
fn graph_finish_static_degree() {
    let n = min(scratch[SCR_GRAPH_STATIC_N], PAIR_CAP);
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
        scratch[SCR_INDIRECT_RADIX] = 0u;
        scratch[SCR_INDIRECT_RADIX + 1u] = 1u;
        scratch[SCR_INDIRECT_RADIX + 2u] = 1u;
        scratch[SCR_INDIRECT_RADIX + 3u] = 0u;
    }
    write_count_indirect(SCR_INDIRECT_STATIC, n, 64u);
}

fn graph_sort_count() -> u32 {
    return min(scratch[SCR_GRAPH_STATIC_N], PAIR_CAP);
}

fn graph_sort_src(from_color: bool) -> u32 {
    return select(SCR_RADIX_OUT, color_contact_base(), from_color);
}

fn graph_sort_dst(from_color: bool) -> u32 {
    return select(color_contact_base(), SCR_RADIX_OUT, from_color);
}

fn graph_static_color(rank: u32) -> u32 {
    if (rank >= OVERFLOW_COLOR - 1u) {
        return OVERFLOW_COLOR;
    }
    return OVERFLOW_COLOR - 1u - rank;
}

// Compact unique indices packed as (writable_body << 16) | unique_i.
@compute @workgroup_size(64)
fn graph_pack_static_keys(@builtin(global_invocation_id) gid: vec3<u32>) {
    if ((params.diagnostic_flags & DIAG_STATIC_DEGREE_TWO_PROOF)!=0u) {return;}
    let i = gid.x;
    let n = min(scratch[SCR_GRAPH_STATIC_N], PAIR_CAP);
    if (i >= n || scratch[SCR_GRAPH_STATIC_MAX] <= 1u) {
        return;
    }
    let unique_i = scratch[SCR_RADIX_OUT + i];
    let dyn = graph_dyn_body(unique_i);
    scratch[SCR_RADIX_OUT + i] = (dyn << 16u) | (unique_i & 0xffffu);
}

fn graph_radix_histogram_impl(gid: u32, lid: u32, group: u32, shift: u32, from_color: bool) {
    atomicStore(&radix_counts[lid], 0u);
    workgroupBarrier();
    let n = graph_sort_count();
    if (gid < n) {
        let key = scratch[graph_sort_src(from_color) + gid];
        atomicAdd(&radix_counts[(key >> shift) & 255u], 1u);
    }
    workgroupBarrier();
    scratch[SCR_RADIX_HIST + group * RADIX_BUCKETS + lid] =
        atomicLoad(&radix_counts[lid]);
}

@compute @workgroup_size(256)
fn graph_radix_histogram_0(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    graph_radix_histogram_impl(gid.x, lid, group.x, 0u, false);
}

@compute @workgroup_size(256)
fn graph_radix_histogram_8(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    graph_radix_histogram_impl(gid.x, lid, group.x, 8u, true);
}

@compute @workgroup_size(256)
fn graph_radix_histogram_16(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    graph_radix_histogram_impl(gid.x, lid, group.x, 16u, false);
}

@compute @workgroup_size(256)
fn graph_radix_histogram_24(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    graph_radix_histogram_impl(gid.x, lid, group.x, 24u, true);
}

fn graph_radix_scatter_impl(gid: u32, lid: u32, group: u32, shift: u32, from_color: bool) {
    let n = graph_sort_count();
    var key = EMPTY;
    if (gid < n) {
        key = scratch[graph_sort_src(from_color) + gid];
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
        let out = scratch[SCR_RADIX_HIST + group * RADIX_BUCKETS + bucket] + rank;
        if (out >= n) {
            record_contact_drop(10u);
        } else {
            scratch[graph_sort_dst(from_color) + out] = key;
        }
    }
}

@compute @workgroup_size(256)
fn graph_radix_scatter_0(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    graph_radix_scatter_impl(gid.x, lid, group.x, 0u, false);
}

@compute @workgroup_size(256)
fn graph_radix_scatter_8(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    graph_radix_scatter_impl(gid.x, lid, group.x, 8u, true);
}

@compute @workgroup_size(256)
fn graph_radix_scatter_16(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    graph_radix_scatter_impl(gid.x, lid, group.x, 16u, false);
}

@compute @workgroup_size(256)
fn graph_radix_scatter_24(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
    @builtin(workgroup_id) group: vec3<u32>,
) {
    graph_radix_scatter_impl(gid.x, lid, group.x, 24u, true);
}

@compute @workgroup_size(64)
fn graph_mark_static_starts(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    let n = min(scratch[SCR_GRAPH_STATIC_N], PAIR_CAP);
    if (i >= n || scratch[SCR_GRAPH_STATIC_MAX] <= 1u) {
        return;
    }
    let dyn = scratch[SCR_RADIX_OUT + i] >> 16u;
    var prev = EMPTY;
    if (i > 0u) {
        prev = scratch[SCR_RADIX_OUT + i - 1u] >> 16u;
    }
    if (i == 0u || dyn != prev) {
        atomicStore(&atom[ATOM_JACOBI + params.body_count + dyn], i);
    }
}

@compute @workgroup_size(64)
fn graph_encode_static_colors(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    let n = min(scratch[SCR_GRAPH_STATIC_N], PAIR_CAP);
    if (i >= n || scratch[SCR_GRAPH_STATIC_MAX] <= 1u) {
        return;
    }
    let key = scratch[SCR_RADIX_OUT + i];
    let dyn = key >> 16u;
    let unique_i = key & 0xffffu;
    let start = atomicLoad(&atom[ATOM_JACOBI + params.body_count + dyn]);
    let rank = i - start;
    let col = graph_static_color(rank);
    if (dyn < params.body_count && col < OVERFLOW_COLOR) {
        atomicOr(&atom[ATOM_JACOBI + dyn], 1u << col);
    }
    scratch[SCR_RADIX_OUT + i] = (col << 16u) | unique_i;
}

@compute @workgroup_size(64)
fn graph_mark_color_starts(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    let n = min(scratch[SCR_GRAPH_STATIC_N], PAIR_CAP);
    if (i >= n || scratch[SCR_GRAPH_STATIC_MAX] <= 1u) {
        return;
    }
    let col = scratch[SCR_RADIX_OUT + i] >> 16u;
    var prev = EMPTY;
    if (i > 0u) {
        prev = scratch[SCR_RADIX_OUT + i - 1u] >> 16u;
    }
    if (i == 0u || col != prev) {
        scratch[SCR_COLOR + col] = i;
    }
}

// Degree-one static edges share one reserved color with local index = compact i.
// Ranked edges were sorted by (body, pair) then (color, unique) so scatter is
// conflict-free without a scene-wide scalar loop.
@compute @workgroup_size(64)
fn graph_assign_static(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    let n = min(scratch[SCR_GRAPH_STATIC_N], PAIR_CAP);
    if (i >= n) {
        return;
    }
    if ((params.diagnostic_flags & DIAG_STATIC_DEGREE_TWO_PROOF)!=0u) {
        if (scratch[SCR_GRAPH_STATIC_MAX]>2u) {return;} // sticky proof failure already reported
        let unique_i=scratch[SCR_RADIX_OUT+i];
        let dyn=graph_dyn_body(unique_i);
        if (dyn>=params.body_count) {return;}
        let first=atomicLoad(&atom[ATOM_JACOBI+params.body_count+dyn]);
        let rank=select(1u,0u,unique_i==first);
        let col=graph_static_color(rank);
        atomicOr(&atom[ATOM_JACOBI+dyn],1u<<col);
        let local=atomicAdd(&atom[atom_graph_color()+col],1u);
        let slot=scratch[SCR_ACTIVE_CONTACT+unique_i];
        if (local<params.contact_capacity) {
            scratch[color_contact_base()+col*params.contact_capacity+local]=slot;
            store_graph_meta(slot,col,local);
        } else {record_contact_drop(9u);}
        return;
    }
    let max_deg = scratch[SCR_GRAPH_STATIC_MAX];
    if (max_deg <= 1u) {
        let unique_i = scratch[SCR_RADIX_OUT + i];
        let slot = scratch[SCR_ACTIVE_CONTACT + unique_i];
        let dyn = graph_dyn_body(unique_i);
        let col = OVERFLOW_COLOR - 1u;
        if (dyn < params.body_count) {
            atomicOr(&atom[ATOM_JACOBI + dyn], 1u << col);
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
    let key = scratch[SCR_RADIX_OUT + i];
    let col = key >> 16u;
    let unique_i = key & 0xffffu;
    let slot = scratch[SCR_ACTIVE_CONTACT + unique_i];
    let start = scratch[SCR_COLOR + col];
    let local = i - start;
    atomicAdd(&atom[atom_graph_color() + col], 1u);
    if (local < params.contact_capacity) {
        scratch[color_contact_base() + col * params.contact_capacity + local] = slot;
        store_graph_meta(slot, col, local);
    }
}

fn color_dynamic_contact(unique_i: u32) {
    let slot = scratch[SCR_ACTIVE_CONTACT + unique_i];
    let h = contacts[slot];
    // This entry is owned by the single canonical greedy walk. No other
    // invocation can change either occupancy mask between these loads/stores.
    let occupied = atomicLoad(&atom[ATOM_JACOBI + h.a])
        | atomicLoad(&atom[ATOM_JACOBI + h.b]);
    let available = (~occupied) & ((1u << DYNAMIC_COLOR_COUNT) - 1u);
    var col = OVERFLOW_COLOR;
    if (available != 0u) {
        col = firstTrailingBit(available);
        let bit = 1u << col;
        atomicOr(&atom[ATOM_JACOBI + h.a], bit);
        atomicOr(&atom[ATOM_JACOBI + h.b], bit);
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
    let n = min(scratch[SCR_DYN_DYN_N], PAIR_CAP);
    for (var i = 0u; i < n; i++) {
        color_dynamic_contact(scratch[SCR_NEXT_OCCUPIED + i]);
    }
    finish_dynamic_graph();
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
    let np_all = min(scratch[SCR_NCONTACTS], PAIR_CAP);
    if (params.solver_mode == SOLVER_JACOBI) {
        scratch[SCR_INDIRECT_COLLIDE] = (np_all + 63u) / 64u;
        scratch[SCR_INDIRECT_COLLIDE + 1u] = 1u;
        scratch[SCR_INDIRECT_COLLIDE + 2u] = 1u;
    } else {
        let use_one_group_wave = mx <= 1u;
        scratch[SCR_INDIRECT_COLLIDE] = select(0u, 1u, use_one_group_wave);
        scratch[SCR_INDIRECT_COLLIDE + 1u] = 1u;
        scratch[SCR_INDIRECT_COLLIDE + 2u] = 1u;
        for (var c = 0u; c < 24u; c++) {
            let n_col = scratch[SCR_COLOR + c];
            var groups = (n_col + 63u) / 64u;
            if (c == OVERFLOW_COLOR) {
                groups = select(0u, 1u, n_col > 0u);
            }
            let base = SCR_INDIRECT_COLOR + c * 4u;
            scratch[base] = select(groups, 0u, use_one_group_wave);
            scratch[base + 1u] = 1u;
            scratch[base + 2u] = 1u;
            scratch[base + 3u] = 0u;
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
fn finish_occupied_contacts(@builtin(global_invocation_id) gid: vec3<u32>,
                            @builtin(num_workgroups) groups: vec3<u32>) {
    let count = min(atomicLoad(&atom[ATOM_OCCUPIED_N]), params.contact_capacity);
    // Retirement and collection are complete before this dispatch. Publish
    // the new list without overwriting any input being read by those passes.
    // Strided ownership preserves list order across workgroups. Consumers run
    // after this dispatch completes; they cannot use the count mid-publication.
    for (var i = gid.x; i < count; i += groups.x * 64u) {
        scratch[SCR_OCCUPIED_CONTACT + i] = scratch[SCR_NEXT_OCCUPIED + i];
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
    let np = min(scratch[SCR_NCONTACTS], PAIR_CAP);
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
                && scratch[SCR_CONTACT_MARK + slot] != 0u
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
        let slot = scratch[SCR_ACTIVE_CONTACT + i];
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
        scratch[SCR_INDIRECT_COLLIDE] = (np + 63u) / 64u;
        scratch[SCR_INDIRECT_COLLIDE + 1u] = 1u;
        scratch[SCR_INDIRECT_COLLIDE + 2u] = 1u;
    } else {
        let use_one_group_wave = np > 0u && mx <= 1u;
        scratch[SCR_INDIRECT_COLLIDE] = select(0u, 1u, use_one_group_wave);
        scratch[SCR_INDIRECT_COLLIDE + 1u] = 1u;
        scratch[SCR_INDIRECT_COLLIDE + 2u] = 1u;
        for (var c = 0u; c < 24u; c++) {
            let n = scratch[SCR_COLOR + c];
            var groups = (n + 63u) / 64u;
            if (c == OVERFLOW_COLOR) {
                groups = select(0u, 1u, n > 0u);
            }
            let base = SCR_INDIRECT_COLOR + c * 4u;
            scratch[base] = select(groups, 0u, use_one_group_wave);
            scratch[base + 1u] = 1u;
            scratch[base + 2u] = 1u;
            scratch[base + 3u] = 0u;
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
fn clear_remapped_contact_hash(@builtin(global_invocation_id) gid: vec3<u32>) {
    if (gid.x < CONTACT_HASH_CAP) {
        atomicStore(&atom[ATOM_CONTACT_KEY + gid.x], EMPTY);
        atomicStore(&atom[ATOM_CONTACT_VALUE + gid.x], EMPTY);
    }
}

@compute @workgroup_size(64)
fn remap_contact_shapes(@builtin(global_invocation_id) gid: vec3<u32>) {
    let slot = gid.x;
    if (slot >= params.contact_capacity) { return; }
    var c = load_contact(slot);
    if (params.remap_capture != 0u) {
        var previous = EMPTY;
        if (c.a != EMPTY && c.count != 0u && (c.lifecycle.y & CONTACT_TOUCHING) != 0u) {
            previous = c.lifecycle.w;
        }
        scratch[SCR_PREVIOUS_TOUCHING + slot] = previous;
    }
    if (c.a == EMPTY) { return; }
    let old_a = c.lifecycle.w & 0xffffu;
    let old_b = c.lifecycle.w >> 16u;
    var a = EMPTY;
    var b = EMPTY;
    if (old_a < params.remap_old_count) { a = scratch[SCR_RADIX_OUT + old_a]; }
    if (old_b < params.remap_old_count) { b = scratch[SCR_RADIX_OUT + old_b]; }
    if (a == EMPTY || b == EMPTY) {
        // Every patch owns its slot; children retire independently in this pass.
        var retired = empty_contact();
        retired.lifecycle.x = c.lifecycle.x;
        store_contact(slot, retired);
    } else {
        c.lifecycle.w = min(a, b) | (max(a, b) << 16u);
        store_contact(slot, c);
    }
}

@compute @workgroup_size(64)
fn publish_remapped_contacts(@builtin(global_invocation_id) gid: vec3<u32>) {
    let slot = gid.x;
    if (slot >= params.contact_capacity) { return; }
    let c = contacts[slot];
    if (c.a != EMPTY && c.manifold_link.y == 0u) {
        if (!publish_contact_slot(contact_persistent[slot].lifecycle.w, slot)) {
            record_contact_drop(11u);
        }
    }
}

// Mutation command, one invocation per unique root. Children have exactly one
// root owner, so retiring a manifold chain cannot race another invocation.
@compute @workgroup_size(64)
fn retire_body_pair_contacts(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= min(scratch[SCR_UNIQUE_N], PAIR_CAP)) { return; }
    let slot = scratch[SCR_ACTIVE_CONTACT + i];
    if (slot >= params.contact_capacity) { return; }
    let c = load_contact(slot);
    let a = atomicLoad(&query[64u]);
    let b = atomicLoad(&query[65u]);
    if (!((c.a == a && c.b == b) || (c.a == b && c.b == a))) { return; }
    if (!retire_manifold_children(slot)) { return; }
    var retired = empty_contact();
    retired.lifecycle.x = c.lifecycle.x;
    store_contact(slot, retired);
    scratch[SCR_CONTACT_MARK + slot] = 0u;
    retire_contact_key(c.lifecycle.w);
}

// Bounded flat-scene pair matrix: a zero bit denotes an accepted pair.
// Row is the higher shape slot, giving the same packed-key order as radix sort.
@compute @workgroup_size(64)
fn pair_matrix_build(@builtin(global_invocation_id) gid: vec3<u32>) {
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
fn pair_matrix_previous(@builtin(global_invocation_id) gid: vec3<u32>) {
    let key=previous_pair_key(gid.x);if(key==EMPTY){return;}
    let lo=key&0xffffu;let hi=key>>16u;
    let stride=(params.shape_count+31u)/32u;
    atomicAnd(&atom[ATOM_HASH+hi*stride+lo/32u],~(1u<<(lo%32u)));
}
@compute @workgroup_size(64)
fn pair_matrix_count(@builtin(global_invocation_id) gid: vec3<u32>) {
    let row=gid.x;if(row>=params.shape_count){return;}
    let stride=(params.shape_count+31u)/32u;
    var count=0u;
    for(var w=0u;w<stride;w++){count+=countOneBits(~atomicLoad(&atom[ATOM_HASH+row*stride+w]));}
    scratch[SCR_INS_BODY+row]=count;
}
@compute @workgroup_size(256)
fn pair_matrix_bases(@builtin(local_invocation_index) lid: u32) {
    let first=lid*3u;
    let a=select(0u,scratch[SCR_INS_BODY+first],first<params.shape_count);
    let b=select(0u,scratch[SCR_INS_BODY+first+1u],first+1u<params.shape_count);
    let c=select(0u,scratch[SCR_INS_BODY+first+2u],first+2u<params.shape_count);
    let base=workgroup_exclusive_scan_256(lid,a+b+c);
    if(first<params.shape_count){scratch[SCR_INS_CELL+first]=base;}
    if(first+1u<params.shape_count){scratch[SCR_INS_CELL+first+1u]=base+a;}
    if(first+2u<params.shape_count){scratch[SCR_INS_CELL+first+2u]=base+a+b;}
    if(lid==255u){
        let total=base+a+b+c;let bounded=min(total,PAIR_CAP);
        if(total>PAIR_CAP){record_capacity_drop_n(ATOM_PAIR_DROPPED,ATOM_STICKY_PAIR_DROPPED,total-PAIR_CAP);}
        atomicStore(&atom[ATOM_PAIR_N],total);
        scratch[SCR_PAIR_N]=bounded;scratch[SCR_POW2]=total;
        scratch[SCR_UNIQUE_N]=bounded;scratch[SCR_NCONTACTS]=bounded;
        write_count_indirect(SCR_INDIRECT_COLLIDE,bounded,64u);
        write_count_indirect(SCR_INDIRECT_PREPARE,bounded,64u);
        write_count_indirect(SCR_INDIRECT_RADIX,bounded,RADIX_GROUP_SIZE);
    }
}
@compute @workgroup_size(64)
fn pair_matrix_scatter(@builtin(global_invocation_id) gid: vec3<u32>) {
    let hi=gid.x;if(hi>=params.shape_count){return;}
    let stride=(params.shape_count+31u)/32u;var dst=scratch[SCR_INS_CELL+hi];
    for(var w=0u;w<stride;w++){
        var accepted=~atomicLoad(&atom[ATOM_HASH+hi*stride+w]);
        while(accepted!=0u){
            let bit=firstTrailingBit(accepted);accepted &= accepted-1u;
            if(dst<PAIR_CAP){scratch[SCR_PAIRS+dst]=(hi<<16u)|(w*32u+bit);}
            dst++;
        }
    }
}
