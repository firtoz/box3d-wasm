// Quats, inertia, contact accessors, Box3D-style softness.

fn is_disabled(b: Body) -> bool {
    return (b.flags & FLAG_DISABLED) != 0u;
}

fn is_static(b: Body) -> bool {
    return (b.flags & FLAG_STATIC) != 0u;
}

fn is_non_dynamic(b: Body) -> bool {
    return is_static(b) || (b.flags & FLAG_KINEMATIC) != 0u || is_disabled(b);
}

fn is_immovable(b: Body) -> bool {
    return is_non_dynamic(b) || (b.flags & FLAG_SLEEP) != 0u;
}

fn joint_endpoint_writable(b: Body) -> bool {
    return !is_immovable(b);
}

fn apply_joint_torque(body: ptr<function, Body>, torque: vec3<f32>) {
    if (!joint_endpoint_writable(*body)) {
        return;
    }
    (*body).omega = (*body).omega + world_inv_inertia(*body, torque);
}

fn body_inv_mass(b: Body) -> f32 {
    return select(b.inv_mass, 0.0, is_immovable(b));
}

fn quat_rotate(q: vec4<f32>, v: vec3<f32>) -> vec3<f32> {
    return gyro_quat_rotate(q,v);
}

fn quat_inv(q: vec4<f32>) -> vec4<f32> {
    return vec4<f32>(-q.xyz, q.w);
}

fn quat_mul(a: vec4<f32>, b: vec4<f32>) -> vec4<f32> {
    return gyro_quat_mul(a,b);
}

fn quat_from_x_axis(axis: vec3<f32>) -> vec4<f32> {
    let n = normalize(axis);
    if (n.x < -0.999999) {
        return vec4<f32>(0.0, 1.0, 0.0, 0.0);
    }
    return normalize(vec4<f32>(0.0, -n.z, n.y, 1.0 + n.x));
}

fn load_collider(i: u32) -> Body {
    let shape = load_shape(i);
    var body = load_body(shape.body_index);
    let cold = load_body_cold(shape.body_index);
    body.pos = body.pos + quat_rotate(body.rot, shape.local_center - cold.local_center);
    if (shape.kind == KIND_CAPSULE && dot(shape.axis, shape.axis) > 1e-12) {
        body.rot = normalize(quat_mul(body.rot, quat_from_x_axis(shape.axis)));
    }
    body.kind = shape.kind;
    body.half = shape.half;
    body.restitution = shape.restitution;
    body.friction = shape.friction;
    body.rolling = shape.rolling;
    body._pad_island = vec2<u32>(i, shape.body_index);
    return body;
}

fn quat_inv_rotate(q: vec4<f32>, v: vec3<f32>) -> vec3<f32> {
    return quat_rotate(quat_inv(q), v);
}

fn solve3(col0: vec3<f32>, col1: vec3<f32>, col2: vec3<f32>, b: vec3<f32>) -> vec3<f32> {
    let det = gyro_dot3(col0, gyro_cross(col1, col2));
    if (abs(det) < 1e-12) {
        return vec3<f32>(0.0);
    }
    let inv = gyro_recip(det);
    return vec3<f32>(
        gyro_dot3(b, gyro_cross(col1, col2)) * inv,
        gyro_dot3(gyro_cross(col2, col0), b) * inv,
        gyro_dot3(gyro_cross(col0, col1), b) * inv,
    );
}

fn box_axis(b: Body, i: u32) -> vec3<f32> {
    var e = vec3<f32>(0.0);
    if (i == 0u) { e = vec3<f32>(1.0, 0.0, 0.0); }
    else if (i == 1u) { e = vec3<f32>(0.0, 1.0, 0.0); }
    else { e = vec3<f32>(0.0, 0.0, 1.0); }
    return quat_rotate(b.rot, e);
}

fn bound_radius(b: Body) -> f32 {
    if (b.kind == KIND_SPHERE) {
        return b.half.x;
    }
    if (b.kind == KIND_CAPSULE) {
        return b.half.x + b.half.y;
    }
    return length(b.half);
}

fn native_rotation_matrix(q: vec4<f32>) -> mat3x3<f32> {
    let xx=q.x*q.x; let yy=q.y*q.y; let zz=q.z*q.z;
    let xy=q.x*q.y; let xz=q.x*q.z; let xw=q.x*q.w;
    let yz=q.y*q.z; let yw=q.y*q.w; let zw=q.z*q.w;
    return mat3x3<f32>(
        vec3<f32>(1.0-2.0*(yy+zz),2.0*(xy+zw),2.0*(xz-yw)),
        vec3<f32>(2.0*(xy-zw),1.0-2.0*(xx+zz),2.0*(yz+xw)),
        vec3<f32>(2.0*(xz+yw),2.0*(yz-xw),1.0-2.0*(xx+yy)));
}

fn native_mul_mv(m: mat3x3<f32>, v: vec3<f32>) -> vec3<f32> {
    return (m[0]*v.x + m[1]*v.y) + m[2]*v.z;
}

fn world_inv_inertia_matrix(b: Body) -> mat3x3<f32> {
    if (is_immovable(b) || (b.flags & FLAG_FIXED_ROTATION) == FLAG_FIXED_ROTATION) {
        return mat3x3<f32>(vec3<f32>(0.0),vec3<f32>(0.0),vec3<f32>(0.0));
    }
    let r = native_rotation_matrix(b.rot);
    let o = b.inv_inertia_offdiag;
    let il = mat3x3<f32>(
        vec3<f32>(b.inv_inertia.x,o.x,o.y),
        vec3<f32>(o.x,b.inv_inertia.y,o.z),
        vec3<f32>(o.y,o.z,b.inv_inertia.z));
    let ri = mat3x3<f32>(native_mul_mv(r,il[0]),native_mul_mv(r,il[1]),native_mul_mv(r,il[2]));
    let rt = transpose(r);
    let iw = mat3x3<f32>(native_mul_mv(ri,rt[0]),native_mul_mv(ri,rt[1]),native_mul_mv(ri,rt[2]));
    return iw;
}

fn world_inv_inertia(b: Body, t: vec3<f32>) -> vec3<f32> {
    return native_mul_mv(world_inv_inertia_matrix(b),t);
}

// The native SIMD contact solver stores six symmetric coefficients and
// accumulates the last two products first; scalar joint math differs.
fn contact_inv_inertia(b: Body, v: vec3<f32>) -> vec3<f32> {
    let m = world_inv_inertia_matrix(b);
    return vec3<f32>(
        m[0].x*v.x + (m[0].y*v.y + m[0].z*v.z),
        m[0].y*v.x + (m[1].y*v.y + m[1].z*v.z),
        m[0].z*v.x + (m[1].z*v.y + m[2].z*v.z));
}

fn apply_contact_P(body: ptr<function, Body>, r: vec3<f32>, P: vec3<f32>, sign: f32) {
    if (is_immovable(*body)) { return; }
    (*body).vel = (*body).vel + sign * P * (*body).inv_mass;
    (*body).omega = (*body).omega + sign * contact_inv_inertia(*body, gyro_cross(r,P));
}

fn apply_P(body: ptr<function, Body>, r: vec3<f32>, P: vec3<f32>, sign: f32) {
    if (is_immovable(*body)) {
        return;
    }
    (*body).vel = (*body).vel + sign * P * (*body).inv_mass;
    (*body).omega = (*body).omega + sign * world_inv_inertia(*body, gyro_cross(r, P));
}

fn ra_at(c: Contact, i: u32) -> vec4<f32> {
    if (i == 0u) { return c.ra0; }
    if (i == 1u) { return c.ra1; }
    if (i == 2u) { return c.ra2; }
    return c.ra3;
}

fn rb_at(c: Contact, i: u32) -> vec4<f32> {
    if (i == 0u) { return c.rb0; }
    if (i == 1u) { return c.rb1; }
    if (i == 2u) { return c.rb2; }
    return c.rb3;
}

fn set_point(c: ptr<function, Contact>, i: u32, ra: vec4<f32>, rb: vec4<f32>) {
    if (i == 0u) { (*c).ra0 = ra; (*c).rb0 = rb; }
    else if (i == 1u) { (*c).ra1 = ra; (*c).rb1 = rb; }
    else if (i == 2u) { (*c).ra2 = ra; (*c).rb2 = rb; }
    else { (*c).ra3 = ra; (*c).rb3 = rb; }
}

fn capsule_axis(b: Body) -> vec3<f32> {
    return quat_rotate(b.rot, vec3<f32>(1.0, 0.0, 0.0)) * b.half.y;
}

fn closest_on_obb(world_p: vec3<f32>, box_b: Body) -> vec3<f32> {
    let d = world_p - box_b.pos;
    let local = quat_rotate(quat_inv(box_b.rot), d);
    var c = clamp(local, -box_b.half, box_b.half);
    // Interior: clamp is identity, dist=0, and sphere–box collide would drop the pair.
    if (all(c == local)) {
        let dx = box_b.half.x - abs(local.x);
        let dy = box_b.half.y - abs(local.y);
        let dz = box_b.half.z - abs(local.z);
        if (dx <= dy && dx <= dz) {
            c.x = select(-box_b.half.x, box_b.half.x, local.x >= 0.0);
        } else if (dy <= dz) {
            c.y = select(-box_b.half.y, box_b.half.y, local.y >= 0.0);
        } else {
            c.z = select(-box_b.half.z, box_b.half.z, local.z >= 0.0);
        }
    }
    return box_b.pos + quat_rotate(box_b.rot, c);
}

fn closest_on_segment(p: vec3<f32>, a: vec3<f32>, b: vec3<f32>) -> vec3<f32> {
    let ab = b - a;
    let t = clamp(dot(p - a, ab) / max(dot(ab, ab), 1e-8), 0.0, 1.0);
    return a + ab * t;
}

fn contact_softness(static_pair: bool) -> vec3<f32> {
    let h = params.dt;
    var hertz = params.contact_hertz;
    if (h > 1e-8) {
        hertz = min(hertz, 0.125 / h);
    }
    var zeta = params.contact_damping;
    if (static_pair) {
        hertz = 2.0 * hertz;
        zeta = 0.5 * zeta;
    }
    let omega = 6.28318530718 * hertz;
    let a1 = 2.0 * zeta + h * omega;
    let a2 = h * omega * a1;
    let a3 = 1.0 / (1.0 + a2);
    return vec3<f32>(omega / a1, a2 * a3, a3);
}

fn empty_contact() -> Contact {
    var c: Contact;
    c.a = EMPTY;
    c.b = EMPTY;
    c.color = 0u;
    c.count = 0u;
    c.n = vec3<f32>(0.0);
    c.friction = 0.0;
    c.ra0 = vec4<f32>(0.0);
    c.ra1 = vec4<f32>(0.0);
    c.ra2 = vec4<f32>(0.0);
    c.ra3 = vec4<f32>(0.0);
    c.rb0 = vec4<f32>(0.0);
    c.rb1 = vec4<f32>(0.0);
    c.rb2 = vec4<f32>(0.0);
    c.rb3 = vec4<f32>(0.0);
    c.friction_impulse = vec2<f32>(0.0);
    c.twist_impulse = 0.0;
    c.rolling = 0.0;
    c.center_a = vec3<f32>(0.0);
    c._pad_ca = 0.0;
    c.center_b = vec3<f32>(0.0);
    c._pad_cb = 0.0;
    c.rolling_impulse = vec3<f32>(0.0);
    c._pad_end = 0.0;
    c.tangent_velocity = vec3<f32>(0.0);
    c.material_index = 0u;
    c._tail0 = vec4<u32>(0u);
    c._tail1 = vec4<u32>(0u);
    c.cached_relative = vec4<f32>(0.0);
    c.cached_rotation_a = vec4<f32>(0.0);
    c.cached_rotation_b = vec4<f32>(0.0);
    c.lifecycle = vec4<u32>(0u);
    c.pair = vec4<u32>(EMPTY);
    c.prepared_normal_mass = vec4<f32>(0.0);
    c.prepared_lever_arm = vec4<f32>(0.0);
    c.total_normal_impulse = vec4<f32>(0.0);
    c.prepared_tangent_inv = vec4<f32>(0.0);
    c.prepared_softness = vec4<f32>(0.0);
    c.persistent_ra0 = vec4<f32>(0.0);
    c.persistent_ra1 = vec4<f32>(0.0);
    c.persistent_ra2 = vec4<f32>(0.0);
    c.persistent_ra3 = vec4<f32>(0.0);
    c.persistent_rb0 = vec4<f32>(0.0);
    c.persistent_rb1 = vec4<f32>(0.0);
    c.persistent_rb2 = vec4<f32>(0.0);
    c.persistent_rb3 = vec4<f32>(0.0);
    return c;
}

fn persistent_ra_at(c: Contact, i: u32) -> vec4<f32> {
    if (i == 0u) { return c.persistent_ra0; }
    if (i == 1u) { return c.persistent_ra1; }
    if (i == 2u) { return c.persistent_ra2; }
    return c.persistent_ra3;
}

fn persistent_rb_at(c: Contact, i: u32) -> vec4<f32> {
    if (i == 0u) { return c.persistent_rb0; }
    if (i == 1u) { return c.persistent_rb1; }
    if (i == 2u) { return c.persistent_rb2; }
    return c.persistent_rb3;
}

fn pack_sat(typ: u32, index_a: u32, index_b: u32) -> u32 {
    return typ | (index_a << 8u) | (index_b << 16u);
}

fn sat_type(p: u32) -> u32 {
    return p & 255u;
}

fn sat_index_a(p: u32) -> u32 {
    return (p >> 8u) & 255u;
}

fn sat_index_b(p: u32) -> u32 {
    return (p >> 16u) & 255u;
}

fn pack_feature(owner1: u32, index1: u32, owner2: u32, index2: u32) -> u32 {
    return (owner1 << 24u) | (index1 << 16u) | (owner2 << 8u) | index2;
}

fn flip_feature(p: u32) -> u32 {
    let o1 = (p >> 24u) & 1u;
    let i1 = (p >> 16u) & 255u;
    let o2 = (p >> 8u) & 1u;
    let i2 = p & 255u;
    return ((1u - o2) << 24u) | (i2 << 16u) | ((1u - o1) << 8u) | i1;
}

fn feat_at(c: Contact, i: u32) -> u32 {
    if (i == 0u) { return c._tail1.z; }
    if (i == 1u) { return c._tail1.w; }
    if (i == 2u) { return bitcast<u32>(c._pad_ca); }
    return bitcast<u32>(c._pad_cb);
}

fn set_feat_at(c: ptr<function, Contact>, i: u32, id: u32) {
    if (i == 0u) { (*c)._tail1.z = id; }
    else if (i == 1u) { (*c)._tail1.w = id; }
    else if (i == 2u) { (*c)._pad_ca = bitcast<f32>(id); }
    else { (*c)._pad_cb = bitcast<f32>(id); }
}

fn manifold_min_sep(c: Contact) -> f32 {
    var m = 1e9;
    for (var i = 0u; i < c.count; i++) {
        let ra = ra_at(c, i);
        let rb = rb_at(c, i);
        m = min(m, ra.w + dot(rb.xyz - ra.xyz, c.n));
    }
    return m;
}

fn point_feature(r: vec3<f32>, n: vec3<f32>) -> u32 {
    var t = vec3<f32>(1.0, 0.0, 0.0);
    if (abs(n.x) > 0.9) {
        t = vec3<f32>(0.0, 1.0, 0.0);
    }
    t = normalize(cross(n, t));
    let b = cross(n, t);
    let su = select(0u, 1u, dot(r, t) >= 0.0);
    let sv = select(0u, 2u, dot(r, b) >= 0.0);
    return su | sv;
}

fn pack_point_features(c: ptr<function, Contact>) {
    if ((*c)._tail1.z != 0u || (*c)._tail1.w != 0u
        || bitcast<u32>((*c)._pad_ca) != 0u || bitcast<u32>((*c)._pad_cb) != 0u) {
        return;
    }
    for (var i = 0u; i < (*c).count; i++) {
        let id = point_feature(ra_at(*c, i).xyz, (*c).n);
        set_feat_at(c, i, id);
    }
}

fn write_sat_cache(c: ptr<function, Contact>, typ: u32, index_a: u32, index_b: u32) {
    (*c)._tail1.x = pack_sat(typ, index_a, index_b);
    (*c)._tail1.y = bitcast<u32>(manifold_min_sep(*c));
}

fn pair_hash_mix(key: u32) -> u32 {
    var x = key;
    x = (x ^ (x >> 16u)) * 0x7feb352du;
    x = (x ^ (x >> 15u)) * 0x846ca68bu;
    return x ^ (x >> 16u);
}

// Identity words are prepared in a separate dispatch before publishing a slot.
// Hash entries contain only slot references: no partially published two-word key
// can become visible under WGSL relaxed atomics. Retired identities stay intact
// until the next allocation phase, after all readers have completed.
fn prepare_contact_identity(dense: u32, key: vec2<u32>) {
    atomicMax(&query[QUERY_CONTACT_HIGH_WATER], dense + 1u);
    atomicStore(&atom[atom_contact_identity() + 2u * dense], key.x);
    atomicStore(&atom[atom_contact_identity() + 2u * dense + 1u], key.y);
}

fn contact_identity(dense: u32) -> vec2<u32> {
    return vec2<u32>(atomicLoad(&atom[atom_contact_identity() + 2u * dense]),
        atomicLoad(&atom[atom_contact_identity() + 2u * dense + 1u]));
}

fn contact_identity_hash(key: vec2<u32>) -> u32 {
    return pair_hash_mix(pair_hash_mix(key.x) ^ key.y) & (contact_hash_cap() - 1u);
}

fn find_contact_identity(key: vec2<u32>) -> u32 {
    var hash_slot = contact_identity_hash(key);
    for (var probe = 0u; probe < contact_hash_cap(); probe++) {
        let stored = atomicLoad(&atom[atom_contact_key() + hash_slot]);
        if (stored < params.contact_capacity) {
            if (all(contact_identity(stored) == key)) { return stored; }
        }
        if (stored == EMPTY) { return EMPTY; }
        hash_slot = (hash_slot + 1u) & (contact_hash_cap() - 1u);
    }
    return EMPTY;
}

// Publication inputs contain one unique root per identity. Identity storage for
// every candidate must already be initialized, including competing publishers.
fn publish_contact_identity(key: vec2<u32>, dense: u32) -> bool {
    // Search past tombstones before insertion so republishing an existing key
    // cannot create a second reference hidden behind a retired bucket.
    loop {
        var hash_slot = contact_identity_hash(key);
        var vacant = EMPTY;
        var expected = EMPTY;
        for (var probe = 0u; probe < contact_hash_cap(); probe++) {
            let stored = atomicLoad(&atom[atom_contact_key() + hash_slot]);
            if (stored < params.contact_capacity) {
                if (all(contact_identity(stored) == key)) {
                    atomicStore(&atom[atom_contact_key() + hash_slot], dense);
                    return true;
                }
            } else if (vacant == EMPTY) {
                vacant = hash_slot;
                expected = stored;
            }
            if (stored == EMPTY) { break; }
            hash_slot = (hash_slot + 1u) & (contact_hash_cap() - 1u);
        }
        if (vacant == EMPTY) { return false; }
        let claimed = atomicCompareExchangeWeak(
            &atom[atom_contact_key() + vacant], expected, dense);
        if (claimed.exchanged) { return true; }
        // Re-probe after contention or a spurious weak-CAS failure.
    }
    return false;
}

fn retire_contact_identity(key: vec2<u32>) {
    var hash_slot = contact_identity_hash(key);
    for (var probe = 0u; probe < contact_hash_cap(); probe++) {
        let stored = atomicLoad(&atom[atom_contact_key() + hash_slot]);
        if (stored < params.contact_capacity) {
            if (all(contact_identity(stored) == key)) {
                atomicStore(&atom[atom_contact_key() + hash_slot], TOMBSTONE);
                return;
            }
        }
        if (stored == EMPTY) { return; }
        hash_slot = (hash_slot + 1u) & (contact_hash_cap() - 1u);
    }
}

fn find_contact_slot(key: vec2<u32>) -> u32 { return find_contact_identity(key); }
fn publish_contact_slot(key: vec2<u32>, dense: u32) -> bool { return publish_contact_identity(key, dense); }
fn retire_contact_key(key: vec2<u32>) { retire_contact_identity(key); }

fn find_prev_contact_key(key: vec2<u32>, a: u32, b: u32) -> Contact {
    let slot = find_contact_slot(key);
    if (slot == EMPTY) {
        return empty_contact();
    }
    let p = load_contact(slot);
    if ((p.a == a && p.b == b) || (p.a == b && p.b == a)) {
        return p;
    }
    return empty_contact();
}

fn find_prev_contact(a: u32, b: u32) -> Contact {
    return find_prev_contact_key(vec2<u32>(min(a, b), max(a, b)), a, b);
}

struct PointMatch { impulses: vec4<f32>, persisted: u32 }

fn persisted_point_bits(count: u32) -> u32 {
    return ((1u << min(count, 4u)) - 1u) << CONTACT_PERSISTED_SHIFT;
}

fn match_previous_points(c: Contact, p: Contact) -> PointMatch {
    var out = vec4<f32>(0.0);
    var persisted = 0u;
    if (p.a == EMPTY) {
        return PointMatch(out, persisted);
    }
    let swapped = p.a == c.b && p.b == c.a;
    let mesh_identity = any(c.point_triangles != vec4<u32>(0u)) || any(p.point_triangles != vec4<u32>(0u));
    let use_id = mesh_identity || ((c._tail1.z != 0u || c._tail1.w != 0u
            || bitcast<u32>(c._pad_ca) != 0u || bitcast<u32>(c._pad_cb) != 0u)
        && (p._tail1.z != 0u || p._tail1.w != 0u
            || bitcast<u32>(p._pad_ca) != 0u || bitcast<u32>(p._pad_cb) != 0u));
    var claimed = vec4<bool>(false);
    for (var k = 0u; k < c.count; k++) {
        var jn = 0.0;
        var matched = false;
        if (use_id) {
            let want = feat_at(c, k);
            for (var j = 0u; j < p.count; j++) {
                if (claimed[j]) {
                    continue;
                }
                var have = feat_at(p, j);
                if (swapped) {
                    have = flip_feature(have);
                }
                if (have == want && (!mesh_identity || (c.point_triangles[k] != 0u
                    && c.point_triangles[k] == p.point_triangles[j]))) {
                    jn = rb_at(p, j).w;
                    claimed[j] = true;
                    matched = true;
                    break;
                }
            }
        }
        // Hull clip ids change when SAT flips face A/B. Box3D still warm-starts
        // nearby points; skipping that fallback walks a stacked column.
        if (!matched && !mesh_identity) {
            let ra = ra_at(c, k).xyz;
            var best = 0.15;
            var best_j = EMPTY;
            for (var j = 0u; j < p.count; j++) {
                if (claimed[j]) {
                    continue;
                }
                var pra = ra_at(p, j).xyz;
                if (swapped) {
                    pra = rb_at(p, j).xyz;
                }
                let d = length(ra - pra);
                if (d < best) {
                    best = d;
                    jn = rb_at(p, j).w;
                    best_j = j;
                }
            }
            if (best_j != EMPTY) {
                claimed[best_j] = true;
                matched = true;
            }
        }
        if (matched) { persisted |= 1u << (CONTACT_PERSISTED_SHIFT + k); }
        if (k == 0u) { out.x = jn; }
        else if (k == 1u) { out.y = jn; }
        else if (k == 2u) { out.z = jn; }
        else { out.w = jn; }
    }
    return PointMatch(out, persisted);
}

fn warm_jn_from_previous(c: Contact, p: Contact) -> vec4<f32> {
    return match_previous_points(c, p).impulses;
}

// Structural validation also permits obsolete body IDs during retirement.
fn contact_chain_structure_valid(root: u32) -> bool {
    if (root >= params.contact_capacity) {
        record_contact_drop(0u);
        return false;
    }
    let first = contacts[root];
    let count = max(first.manifold_link.z, 1u);
    var valid = first.manifold_link.y == 0u && count <= params.contact_capacity;
    var slot = root;
    for (var i = 0u; i < min(count, params.contact_capacity) && valid; i++) {
        if (slot >= params.contact_capacity) { valid = false; break; }
        let c = contacts[slot];
        let parent = select(root + 1u, 0u, i == 0u);
        if (c.a != first.a || c.b != first.b || c.manifold_link.y != parent) {
            valid = false; break;
        }
        let next = c.manifold_link.x;
        if (i + 1u == count) { valid = next == 0u; }
        else if (next == 0u) { valid = false; }
        else { slot = next - 1u; }
    }
    if (!valid) { record_contact_drop(1u); }
    return valid;
}

// The root remains unchanged until cache transfer from all old patches is done.
// Slots retired here cannot re-enter this dispatch's free-list snapshot.
fn retire_manifold_children(root: u32) -> bool {
    if (!contact_chain_structure_valid(root)) { return false; }
    let first = contacts[root];
    var next = first.manifold_link.x;
    for (var i = 1u; i < max(first.manifold_link.z, 1u); i++) {
        let slot = next - 1u;
        let old = load_contact(slot);
        next = old.manifold_link.x;
        var empty = empty_contact();
        empty.lifecycle.x = old.lifecycle.x;
        store_contact(slot, empty);
        scratch[scr_contact_mark() + slot] = 0u;
    }
    contacts[root].manifold_link = vec4<u32>(0u);
    return true;
}

fn allocate_manifold_slot(root: u32) -> u32 {
    if (root >= params.contact_capacity) {
        record_contact_drop(2u);
        return EMPTY;
    }
    let index = atomicAdd(&atom[ATOM_MANIFOLD_ALLOC], 1u);
    if (index >= min(scratch[SCR_FREE_N], params.contact_capacity)) {
        record_contact_drop(3u);
        return EMPTY;
    }
    let slot = scratch[scr_radix_out() + index];
    if (slot >= params.contact_capacity || slot == root || contacts[slot].a != EMPTY) {
        record_contact_drop(4u);
        return EMPTY;
    }
    atomicMax(&query[QUERY_CONTACT_HIGH_WATER], slot + 1u);
    scratch[scr_contact_mark() + slot] = 1u;
    return slot;
}

// Sticky cause bits share the existing tiny status readback. Ray state occupies
// words 0..31 and joint-pair mutation words 64/65; word 66 is reserved here.
fn record_contact_drop(reason: u32) {
    atomicOr(&query[66u], 1u << reason);
    record_capacity_drop(ATOM_CONTACT_DROPPED, ATOM_STICKY_CONTACT_DROPPED);
}
