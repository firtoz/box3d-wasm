override COLLISION_MESH_ENABLED: bool = true;

// Box–box follows Box3D `b3CollideHulls`: SAT face cache, then OBB SAT (clip-before-edge).

fn box_support_radius(b: Body, axis: vec3<f32>) -> f32 {
    return b.half.x * abs(dot(axis, box_axis(b, 0u)))
        + b.half.y * abs(dot(axis, box_axis(b, 1u)))
        + b.half.z * abs(dot(axis, box_axis(b, 2u)));
}

fn box_interval(b: Body, axis: vec3<f32>) -> vec2<f32> {
    let r = box_support_radius(b, axis);
    let m = dot(b.pos, axis);
    return vec2<f32>(m - r, m + r);
}

fn interval_overlap(a: vec2<f32>, b: vec2<f32>) -> f32 {
    return min(a.y, b.y) - max(a.x, b.x);
}

fn face_verts(b: Body, ax: u32, sgn: f32) -> array<vec3<f32>, 4> {
    let n = box_axis(b, ax) * sgn;
    var u_i = (ax + 1u) % 3u;
    var v_i = (ax + 2u) % 3u;
    let u = box_axis(b, u_i);
    let v = box_axis(b, v_i);
    var hu = b.half.x;
    var hv = b.half.y;
    if (u_i == 1u) { hu = b.half.y; }
    if (u_i == 2u) { hu = b.half.z; }
    if (v_i == 0u) { hv = b.half.x; }
    if (v_i == 1u) { hv = b.half.y; }
    if (v_i == 2u) { hv = b.half.z; }
    let c = b.pos + n * hull_face_half(b, 2u * ax);
    return array<vec3<f32>, 4>(
        c + u * hu + v * hv,
        c + u * hu - v * hv,
        c - u * hu - v * hv,
        c - u * hu + v * hv,
    );
}

fn box_face_n(b: Body, face: u32) -> vec3<f32> {
    return hull_face_n(b, face);
}

fn box_face_half(b: Body, face: u32) -> f32 {
    return hull_face_half(b, face);
}

fn hull_face_sep(ref_b: Body, other: Body, face: u32) -> f32 {
    let n = box_face_n(ref_b, face);
    let max_ref = dot(ref_b.pos, n) + box_face_half(ref_b, face);
    return (dot(other.pos, n) - box_support_radius(other, n)) - max_ref;
}

fn box_support_index(b: Body, dir: vec3<f32>) -> u32 {
    let local = quat_inv_rotate(b.rot, dir);
    var idx = 0u;
    if (local.x >= 0.0) { idx = idx | 1u; }
    if (local.y >= 0.0) { idx = idx | 2u; }
    if (local.z >= 0.0) { idx = idx | 4u; }
    return idx;
}

fn rolling_radius(b: Body) -> f32 {
    if (b.kind == KIND_SPHERE) {
        return b.half.x;
    }
    if (b.kind == KIND_CAPSULE) {
        return b.half.x;
    }
    // Collider proxies carry the live shape slot, including compound children.
    // A hull's inner radius need not equal its smallest bounding-box extent.
    if (b.kind == KIND_CONVEX_HULL) {
        return 0.25 * load_shape(b._pad_island.x).inner_radius;
    }
    if (b.kind == KIND_BOX) {
        return 0.25 * min(b.half.x, min(b.half.y, b.half.z));
    }
    return 0.0;
}

const MIX_PAIR_CAP: u32 = 4096u;

// Native mesh contacts use the full hull inner radius; the quarter-radius
// rule belongs to convex-convex contacts only.
fn mesh_rolling_radius(shape: Shape) -> f32 {
    if (shape.kind == KIND_CONVEX_HULL) { return shape.inner_radius; }
    if (shape.kind == KIND_BOX) { return min(shape.half.x, min(shape.half.y, shape.half.z)); }
    if (shape.kind == KIND_SPHERE || shape.kind == KIND_CAPSULE) { return shape.half.x; }
    return 0.0;
}

fn mixed_friction_of(a: Body, b: Body) -> f32 {
    let geometric = sqrt(max(a.friction * b.friction, 0.0));
    if (params.mix_pair_count == 0u) {
        return geometric;
    }
    let ia = a._pad_island.x;
    let ib = b._pad_island.x;
    let key = vec2<u32>(min(ia, ib), max(ia, ib));
    var slot = pair_hash_mix(pair_hash_mix(key.x) ^ key.y) & (MIX_PAIR_CAP - 1u);
    for (var probe = 0u; probe < 32u; probe++) {
        let base = params.mix_pair_base_u32 + slot * 8u;
        if (scene_words[base + 4u] == 0u) {
            break;
        }
        if (scene_words[base] == key.x && scene_words[base + 1u] == key.y) {
            return bitcast<f32>(scene_words[base + 2u]);
        }
        slot = (slot + 1u) & (MIX_PAIR_CAP - 1u);
    }
    return geometric;
}

fn mixed_restitution_of(a: Body, b: Body) -> f32 {
    let geometric = max(a.restitution, b.restitution);
    if (params.mix_pair_count == 0u) {
        return geometric;
    }
    let ia = a._pad_island.x;
    let ib = b._pad_island.x;
    let key = vec2<u32>(min(ia, ib), max(ia, ib));
    var slot = pair_hash_mix(pair_hash_mix(key.x) ^ key.y) & (MIX_PAIR_CAP - 1u);
    for (var probe = 0u; probe < 32u; probe++) {
        let base = params.mix_pair_base_u32 + slot * 8u;
        if (scene_words[base + 4u] == 0u) {
            break;
        }
        if (scene_words[base] == key.x && scene_words[base + 1u] == key.y) {
            return bitcast<f32>(scene_words[base + 3u]);
        }
        slot = (slot + 1u) & (MIX_PAIR_CAP - 1u);
    }
    return geometric;
}

// Native stores a world-space friction impulse between steps and projects it
// during preparation, including when the contact geometry is recycled.
fn reproject_contact_friction(old_normal: vec3<f32>, new_normal: vec3<f32>,
    impulse: vec2<f32>, endpoint_sign: f32) -> vec2<f32> {
    let old_t1 = perp(old_normal);
    let old_t2 = gyro_cross(old_t1, old_normal);
    let world_impulse = endpoint_sign * (old_t1 * impulse.x + old_t2 * impulse.y);
    let new_t1 = perp(new_normal);
    let new_t2 = gyro_cross(new_t1, new_normal);
    return vec2<f32>(gyro_dot3(world_impulse, new_t1), gyro_dot3(world_impulse, new_t2));
}

fn finish_manifold(c: ptr<function, Contact>, a: Body, b: Body, ia: u32, ib: u32) {
    let key = vec2<u32>(min(a._pad_island.x, b._pad_island.x), max(a._pad_island.x, b._pad_island.x));
    let previous = find_prev_contact_key(key, ia, ib);
    finish_manifold_from_previous(c, a, b, ia, ib, previous);
}

// Mesh piece matching selects a distinct old manifold before calling this.
// No implicit root lookup may duplicate the root's impulse into its children.
fn finish_manifold_from_previous(c: ptr<function, Contact>, a: Body, b: Body, ia: u32, ib: u32, p: Contact) {
    // Colliders are shape-origin; the solver uses body COM lever arms.
    // Packed ra.w is the COM-relative base so
    //   s = ra.w + gyro_dot3(rb.xyz - ra.xyz, n)
    // equals world separation along n:
    //   gyro_dot3((com_b + rb) - (com_a + ra), n).
    // Shifting xyz by shape-to-COM therefore subtracts the same projection
    // from ra.w so s, friction weights, and cached reconstruction stay invariant.
    // Box-face clipping supplies the original separation before solver-base
    // packing. Marker 3 carries raw separation and COM anchors; marker 4
    // carries packed separation and COM anchors for mesh/capsule witnesses.
    // Mesh finalization uses 5/6 for raw separation with origin/COM anchors.
    // These markers are transient; finalization writes cache validity 1.
    let mesh_raw = (*c).cached_relative.w == 5.0 || (*c).cached_relative.w == 6.0;
    let com_anchors = (*c).cached_relative.w == 3.0 || (*c).cached_relative.w == 4.0
        || (*c).cached_relative.w == 6.0;
    let has_raw_separations = (*c).cached_relative.w == 2.0 || (*c).cached_relative.w == 3.0 || mesh_raw;
    let capsule_pair = a.kind == KIND_CAPSULE && b.kind == KIND_CAPSULE;
    let raw_separations = vec4<f32>((*c).persistent_rb0.w, (*c).persistent_rb1.w,
        (*c).persistent_rb2.w, (*c).persistent_rb3.w);
    var separations = vec4<f32>(0.0);
    let com_a = load_body(ia);
    let com_b = load_body(ib);
    let da = select(a.pos - com_a.pos, vec3<f32>(0.0), com_anchors);
    let db = select(b.pos - com_b.pos, vec3<f32>(0.0), com_anchors);
    let base_shift = gyro_dot3(db - da, (*c).n);
    for (var i = 0u; i < (*c).count; i++) {
        let ra = ra_at(*c, i);
        let rb = rb_at(*c, i);
        let anchor_a = ra.xyz + da;
        let anchor_b = rb.xyz + db;
        var base = ra.w - base_shift;
        if ((capsule_pair && has_raw_separations) || mesh_raw) {
            base = raw_separations[i] - gyro_dot3(anchor_b - anchor_a, (*c).n);
        }
        set_point(c, i, vec4<f32>(anchor_a, base), vec4<f32>(anchor_b, rb.w));
    }
    let inv_tau = 1.0 / SPECULATIVE;
    var center_a = vec3<f32>(0.0);
    var center_b = vec3<f32>(0.0);
    var wsum = 0.0;
    var rel = vec4<f32>(0.0);
    for (var i = 0u; i < (*c).count; i++) {
        let ra = ra_at(*c, i);
        let rb = rb_at(*c, i);
        let s = select(ra.w + gyro_dot3(rb.xyz - ra.xyz, (*c).n), raw_separations[i], has_raw_separations);
        separations[i] = s;
        let weight = clamp(2.0 - s * inv_tau, MIN_FRICTION_WEIGHT, 1.0);
        center_a = center_a + ra.xyz * weight;
        center_b = center_b + rb.xyz * weight;
        wsum = wsum + weight;
        let vA = com_a.vel + gyro_cross(com_a.omega, ra.xyz);
        let vB = com_b.vel + gyro_cross(com_b.omega, rb.xyz);
        let vn = gyro_dot3(vB - vA, (*c).n);
        if (i == 0u) { rel.x = vn; }
        else if (i == 1u) { rel.y = vn; }
        else if (i == 2u) { rel.z = vn; }
        else { rel.w = vn; }
    }
    let inv = gyro_recip(max(wsum, MIN_FRICTION_WEIGHT));
    (*c).center_a = center_a * inv;
    (*c).center_b = center_b * inv;
    (*c)._pad_end = mixed_restitution_of(a, b);
    // Box3D contact.c: max(material) * max(shape radius).
    (*c).rolling = select(
        0.0,
        max(a.rolling, b.rolling) * max(rolling_radius(a), rolling_radius(b)),
        (params.diagnostic_flags & DIAG_DISABLE_ROLLING) == 0u,
    );
    (*c)._tail0 = vec4<u32>(bitcast<u32>(rel.x), bitcast<u32>(rel.y), bitcast<u32>(rel.z), bitcast<u32>(rel.w));
    let exact_features = a.kind == KIND_CAPSULE && b.kind == KIND_CAPSULE;
    if (!exact_features) { pack_point_features(c); }
    let matched = match_previous_points(*c, p, exact_features);
    let jn = matched.impulses;
    (*c).lifecycle.y = ((*c).lifecycle.y & ~CONTACT_PERSISTED_MASK) | matched.persisted;
    (*c).rb0.w = jn.x;
    if ((*c).count > 1u) { (*c).rb1.w = jn.y; }
    if ((*c).count > 2u) { (*c).rb2.w = jn.z; }
    if ((*c).count > 3u) { (*c).rb3.w = jn.w; }
    if (p.a != EMPTY) {
        (*c).color = p.color;
    }
    if (p.a != EMPTY && p.count > 0u) {
        let sign = select(-1.0, 1.0, p.a == ia && p.b == ib);
        (*c).friction_impulse = reproject_contact_friction(p.n, (*c).n, p.friction_impulse, sign);
        (*c).twist_impulse = p.twist_impulse;
        (*c).rolling_impulse = sign * p.rolling_impulse;
    }
    (*c).cached_relative = vec4<f32>(relative_body_origin(com_a, com_b, ia, ib), 1.0);
    (*c).cached_rotation_a = com_a.rot;
    (*c).cached_rotation_b = com_b.rot;
    (*c).persistent_ra0 = (*c).ra0;
    (*c).persistent_ra1 = (*c).ra1;
    (*c).persistent_ra2 = (*c).ra2;
    (*c).persistent_ra3 = (*c).ra3;
    (*c).persistent_rb0 = vec4<f32>((*c).rb0.xyz, separations.x);
    (*c).persistent_rb1 = vec4<f32>((*c).rb1.xyz, separations.y);
    (*c).persistent_rb2 = vec4<f32>((*c).rb2.xyz, separations.z);
    (*c).persistent_rb3 = vec4<f32>((*c).rb3.xyz, separations.w);
}

// Box3D `b3ClipPolygon`: keep/intersect and stamp reference-edge feature ids.
// Refine the shader division before constructing a clipped intersection.
// A one-ulp fraction error moves endpoints on long edges by several ulps.
fn clip_divide(a: f32, b: f32) -> f32 {
    let estimate = a / b;
    let residual = fma(-estimate,b,a);
    return fma(residual,gyro_recip(b),estimate);
}

fn clip_feat(
    pts: array<vec3<f32>, 8>,
    n_in: u32,
    plane_n: vec3<f32>,
    plane_c: f32,
) -> array<vec3<f32>, 8> {
    var outp: array<vec3<f32>, 8>;
    if (n_in == 0u) {
        return outp;
    }
    var n_out = 0u;
    var prev = pts[n_in - 1u];
    var d1 = dot(prev, plane_n) - plane_c;
    for (var i = 0u; i < n_in; i++) {
        let cur = pts[i];
        let d2 = dot(cur, plane_n) - plane_c;
        if (d1 <= 0.0 && d2 <= 0.0) {
            if (n_out < 8u) {
                outp[n_out] = cur;
                n_out = n_out + 1u;
            }
        } else if (d1 <= 0.0 && d2 > 0.0) {
            let t = clip_divide(d1,d1-d2);
            if (n_out < 8u) {
                // Preserve the native clip interpolation order.
                outp[n_out] = prev + t * (cur - prev);
                n_out = n_out + 1u;
            }
        } else if (d2 <= 0.0 && d1 > 0.0) {
            let t = clip_divide(d1,d1-d2);
            if (n_out < 8u) {
                // Preserve the native clip interpolation order.
                outp[n_out] = prev + t * (cur - prev);
                n_out = n_out + 1u;
            }
            if (n_out < 8u) {
                outp[n_out] = cur;
                n_out = n_out + 1u;
            }
        }
        prev = cur;
        d1 = d2;
    }
    return outp;
}

fn clip_feat_ids(
    pts: array<vec3<f32>, 8>,
    feats: array<u32, 8>,
    n_in: u32,
    plane_n: vec3<f32>,
    plane_c: f32,
    edge: u32,
    ref_owner: u32,
) -> array<u32, 8> {
    var outf: array<u32, 8>;
    if (n_in == 0u) {
        return outf;
    }
    var n_out = 0u;
    var prev = pts[n_in - 1u];
    var prev_f = feats[n_in - 1u];
    var d1 = dot(prev, plane_n) - plane_c;
    for (var i = 0u; i < n_in; i++) {
        let cur = pts[i];
        let cur_f = feats[i];
        let d2 = dot(cur, plane_n) - plane_c;
        if (d1 <= 0.0 && d2 <= 0.0) {
            if (n_out < 8u) {
                outf[n_out] = cur_f;
                n_out = n_out + 1u;
            }
        } else if (d1 <= 0.0 && d2 > 0.0) {
            if (n_out < 8u) {
                outf[n_out] = pack_feature(
                    (cur_f >> 24u) & 1u,
                    (cur_f >> 16u) & 255u,
                    ref_owner,
                    edge,
                );
                n_out = n_out + 1u;
            }
        } else if (d2 <= 0.0 && d1 > 0.0) {
            if (n_out < 8u) {
                outf[n_out] = pack_feature(ref_owner, edge, (prev_f >> 8u) & 1u, prev_f & 255u);
                n_out = n_out + 1u;
            }
            if (n_out < 8u) {
                outf[n_out] = cur_f;
                n_out = n_out + 1u;
            }
        }
        prev = cur;
        prev_f = cur_f;
        d1 = d2;
    }
    return outf;
}

fn clip_feat_count(
    pts: array<vec3<f32>, 8>,
    n_in: u32,
    plane_n: vec3<f32>,
    plane_c: f32,
) -> u32 {
    if (n_in == 0u) {
        return 0u;
    }
    var n_out = 0u;
    var prev = pts[n_in - 1u];
    var d1 = dot(prev, plane_n) - plane_c;
    for (var i = 0u; i < n_in; i++) {
        let cur = pts[i];
        let d2 = dot(cur, plane_n) - plane_c;
        if (d1 <= 0.0 && d2 <= 0.0) {
            n_out = n_out + 1u;
        } else if (d1 <= 0.0 && d2 > 0.0) {
            n_out = n_out + 1u;
        } else if (d2 <= 0.0 && d1 > 0.0) {
            n_out = n_out + 2u;
        }
        prev = cur;
        d1 = d2;
        if (n_out >= 8u) {
            return 8u;
        }
    }
    return n_out;
}

fn add_clip_points(
    c: ptr<function, Contact>,
    a: Body,
    b: Body,
    n: vec3<f32>,
    pts: array<vec3<f32>, 8>,
    feats: array<u32, 8>,
    np: u32,
    ref_half: f32,
    flip_ids: bool,
) {
    var cand_s: array<f32, 8>;
    var cand_p: array<vec3<f32>, 8>;
    var cand_f: array<u32, 8>;
    var n_cand = 0u;
    for (var i = 0u; i < np; i++) {
        let raw = pts[i];
        var s = gyro_dot3(raw, n) - ref_half;
        // Box3D keeps every clipped vertex; the manifold is rejected only if
        // min separation ≥ speculative (see `b3BuildFaceAContact`).
        cand_s[n_cand] = s;
        cand_p[n_cand] = raw - 0.5 * s * n;
        var fid = feats[i];
        if (flip_ids) {
            fid = flip_feature(fid);
        }
        cand_f[n_cand] = fid;
        n_cand = n_cand + 1u;
        if (n_cand == 8u) {
            break;
        }
    }
    if (n_cand == 0u) {
        (*c).count = 0u;
        return;
    }
    var min_s = cand_s[0];
    for (var i = 1u; i < n_cand; i++) {
        min_s = min(min_s, cand_s[i]);
    }
    if (min_s >= SPECULATIVE) {
        (*c).count = 0u;
        return;
    }
    var pick_s: array<f32, 4>;
    var pick_p: array<vec3<f32>, 4>;
    var pick_f: array<u32, 4>;
    var n_pick = 0u;
    if (n_cand <= 4u) {
        for (var i = 0u; i < n_cand; i++) {
            pick_s[i] = cand_s[i];
            pick_p[i] = cand_p[i];
            pick_f[i] = cand_f[i];
        }
        n_pick = n_cand;
    } else {
        // Box3D `b3ArbitraryPerp`: preserve its branch constants and operation
        // order because this direction decides which four face points survive.
        var search: vec3<f32>;
        if (n.x < -0.5 || n.x > 0.5) {
            search = vec3<f32>(0.67 * n.y - 0.42 * n.z, -0.67 * n.x, 0.42 * n.x);
        } else if (n.y < -0.5 || n.y > 0.5) {
            search = vec3<f32>(0.67 * n.y, -0.67 * n.x - 0.42 * n.z, 0.42 * n.y);
        } else {
            search = vec3<f32>(0.67 * n.z, -0.42 * n.z, -0.67 * n.x + 0.42 * n.y);
        }
        search = normalize(search);
        let bias = 0.95;
        let tol_sqr = SPECULATIVE * SPECULATIVE;
        var best = EMPTY;
        var best_score = -1e9;
        for (var i = 0u; i < n_cand; i++) {
            if (cand_s[i] > SPECULATIVE) { continue; }
            let score = -cand_s[i] + gyro_dot3(search, cand_p[i]);
            if (bias * score > best_score) {
                best_score = score;
                best = i;
            }
        }
        if (best == EMPTY) {
            (*c).count = 0u;
            return;
        }
        pick_s[0] = cand_s[best];
        pick_p[0] = cand_p[best];
        pick_f[0] = cand_f[best];
        n_pick = 1u;
        let pa = pick_p[0];
        n_cand = n_cand - 1u;
        cand_s[best] = cand_s[n_cand];
        cand_p[best] = cand_p[n_cand];
        cand_f[best] = cand_f[n_cand];
        best = EMPTY;
        best_score = 0.0;
        for (var i = 0u; i < n_cand; i++) {
            let d = cand_p[i] - pa;
            let v = d - n * gyro_dot3(d, n);
            let d2 = gyro_dot3(v, v);
            let sep = max(0.0, -cand_s[i]);
            let score = d2 + 4.0 * sep * sep;
            if (bias * score > best_score) {
                best_score = score;
                best = i;
            }
        }
        if (best != EMPTY && best_score >= tol_sqr) {
            pick_s[1] = cand_s[best];
            pick_p[1] = cand_p[best];
            pick_f[1] = cand_f[best];
            n_pick = 2u;
            let pb = pick_p[1];
            n_cand = n_cand - 1u;
            cand_s[best] = cand_s[n_cand];
            cand_p[best] = cand_p[n_cand];
            cand_f[best] = cand_f[n_cand];
            let ba = pb - pa;
            best = EMPTY;
            best_score = tol_sqr;
            var best_area = 0.0;
            for (var i = 0u; i < n_cand; i++) {
                let area = gyro_dot3(n, gyro_cross(ba, cand_p[i] - pa));
                let score = abs(area);
                if (bias * score >= best_score) {
                    best_score = score;
                    best = i;
                    best_area = area;
                }
            }
            if (best != EMPTY) {
                pick_s[2] = cand_s[best];
                pick_p[2] = cand_p[best];
                pick_f[2] = cand_f[best];
                n_pick = 3u;
                let pc = pick_p[2];
                n_cand = n_cand - 1u;
                cand_s[best] = cand_s[n_cand];
                cand_p[best] = cand_p[n_cand];
                cand_f[best] = cand_f[n_cand];
                let sgn = select(1.0, -1.0, best_area < 0.0);
                best = EMPTY;
                best_score = tol_sqr;
                for (var i = 0u; i < n_cand; i++) {
                    let p = cand_p[i];
                    let u1 = sgn * gyro_dot3(n, gyro_cross(p - pa, ba));
                    let u2 = sgn * gyro_dot3(n, gyro_cross(p - pb, pc - pb));
                    let u3 = sgn * gyro_dot3(n, gyro_cross(p - pc, pa - pc));
                    let score = max(u1, max(u2, u3));
                    if (bias * score > best_score) {
                        best_score = score;
                        best = i;
                    }
                }
                if (best != EMPTY) {
                    pick_s[3] = cand_s[best];
                    pick_p[3] = cand_p[best];
                    pick_f[3] = cand_f[best];
                    n_pick = 4u;
                }
            }
        }
    }
    for (var i = 0u; i < n_pick; i++) {
        let s = pick_s[i];
        var point_a = pick_p[i];
        if (flip_ids) {
            let relative_q = box_relative_rotation(a.rot, b.rot);
            let relative_p = quat_inv_rotate(a.rot, b.pos - a.pos);
            point_a = recycle_rotate(relative_q, point_a) + relative_p;
        }
        let rA = recycle_rotate(a.rot, point_a);
        let rB = rA + (a.pos - b.pos);
        // Offset is along the manifold normal (`c.n` = A→B), not the clip plane.
        let base = s - gyro_dot3(rB - rA, (*c).n);
        set_point(c, i, vec4<f32>(rA, base), vec4<f32>(rB, 0.0));
        set_feat_at(c, i, pick_f[i]);
        if (i == 0u) { (*c).persistent_rb0.w = s; }
        else if (i == 1u) { (*c).persistent_rb1.w = s; }
        else if (i == 2u) { (*c).persistent_rb2.w = s; }
        else { (*c).persistent_rb3.w = s; }

    }
    (*c).count = n_pick;
    (*c).cached_relative.w = 2.0;
}

fn box_relative_rotation(a: vec4<f32>, b: vec4<f32>) -> vec4<f32> {
    let t1 = gyro_cross(b.xyz, a.xyz);
    let t2 = t1 + a.w * b.xyz;
    return vec4<f32>(t2 - b.w * a.xyz, a.w * b.w + gyro_dot3(a.xyz, b.xyz));
}

fn clip_box_face(a: Body, b: Body, ia: u32, ib: u32, best_kind: u32, best_face: u32, best_n: vec3<f32>) -> Contact {
    var c = empty_contact();
    c.a = ia;
    c.b = ib;
    c.friction = mixed_friction_of(a, b);
    c.n = best_n;

    var incident_body = b;
    var incident_face = 0u;
    var inc_edges: array<u32, 4>;
    var ref_edges: array<u32, 4>;
    var ref_body = a;
    var ref_n = best_n;
    var inc_owner = 1u;
    var ref_owner = 0u;
    var ref_half = 0.0;
    if (best_kind == 0u) {
        let sup = box_support_vertex(b, -best_n);
        let inc_face = find_incident_face(b, best_n, sup);
        incident_body = b;
        incident_face = inc_face;
        inc_edges = hull_face_edge_ids(inc_face);
        ref_edges = hull_face_edge_ids(best_face);
        ref_body = a;
        ref_n = hull_face_n(a, best_face);
        if (dot(ref_n, best_n) < 0.0) {
            ref_n = -ref_n;
        }
        ref_half = hull_face_half(a, best_face);
        inc_owner = 1u;
        ref_owner = 0u;
    } else {
        let sup = box_support_vertex(a, best_n);
        let inc_face = find_incident_face(a, -best_n, sup);
        incident_body = a;
        incident_face = inc_face;
        inc_edges = hull_face_edge_ids(inc_face);
        ref_edges = hull_face_edge_ids(best_face);
        ref_body = b;
        ref_n = hull_face_n(b, best_face);
        if (dot(ref_n, -best_n) < 0.0) {
            ref_n = -ref_n;
        }
        ref_half = hull_face_half(b, best_face);
        // Box3D builds face-B by swapping the hulls, running the face-A
        // builder (incident features are shape B, reference features shape A),
        // then flipping the completed feature pair back to the original order.
        inc_owner = 1u;
        ref_owner = 0u;
    }

    var poly: array<vec3<f32>, 8>;
    var feats: array<u32, 8>;
    // Box3D clips in the reference hull's local frame so a box face is an exact
    // rectangle. World-space clipping shears that square as soon as either box
    // has a tiny rotation, which is what walks a column.
    let b_to_a_q = box_relative_rotation(a.rot, b.rot);
    let b_to_a_p = quat_inv_rotate(a.rot, b.pos - a.pos);
    // Native face manifolds transform the local plane normal without
    // renormalizing it. Preserve the same two transforms for face B.
    var normal_a = BOX_FACE_N[best_face];
    if (best_kind != 0u) { normal_a = -recycle_rotate(b_to_a_q, normal_a); }
    c.n = recycle_rotate(a.rot, normal_a);

    var incident_q = b_to_a_q;
    var incident_p = b_to_a_p;
    if (best_kind != 0u) {
        incident_q = quat_inv(b_to_a_q);
        incident_p = quat_inv_rotate(b_to_a_q, -b_to_a_p);
    }
    var incident_edge = BOX_FACE_EDGE[incident_face];
    for (var i = 0u; i < 4u; i++) {
        let next = BOX_EDGE_NEXT[incident_edge];
        let point = box_point_local(incident_body, BOX_EDGE_ORIGIN[next]);
        poly[i] = recycle_rotate(incident_q, point) + incident_p;
        incident_edge = next;
    }
    feats[0] = pack_feature(inc_owner, inc_edges[0], inc_owner, inc_edges[1]);
    feats[1] = pack_feature(inc_owner, inc_edges[1], inc_owner, inc_edges[2]);
    feats[2] = pack_feature(inc_owner, inc_edges[2], inc_owner, inc_edges[3]);
    feats[3] = pack_feature(inc_owner, inc_edges[3], inc_owner, inc_edges[0]);
    var np = 4u;
    let ref_n_local = BOX_FACE_N[best_face];
    for (var p = 0u; p < 4u; p++) {
        let e = ref_edges[p];
        let v1 = box_point_local(ref_body, BOX_EDGE_ORIGIN[e]);
        let v2 = box_point_local(ref_body, BOX_EDGE_ORIGIN[BOX_EDGE_NEXT[e]]);
        var tangent = v2 - v1;
        let tl = length(tangent);
        if (tl < 1e-12) {
            continue;
        }
        tangent = tangent / tl;
        let binormal = cross(tangent, ref_n_local);
        let plane_c = dot(v1, binormal);
        let next_f = clip_feat_ids(poly, feats, np, binormal, plane_c, e, ref_owner);
        let next = clip_feat(poly, np, binormal, plane_c);
        np = clip_feat_count(poly, np, binormal, plane_c);
        poly = next;
        feats = next_f;
        if (np < 3u) {
            c.a = EMPTY;
            return c;
        }
    }
    add_clip_points(&c, a, b, ref_n_local, poly, feats, np, ref_half, best_kind != 0u);
    if (c.count == 0u) {
        c.a = EMPTY;
    }
    return c;
}

fn try_cached_face(a: Body, b: Body, ia: u32, ib: u32) -> Contact {
    let p = find_prev_contact(ia, ib);
    if (p.a == EMPTY) {
        return empty_contact();
    }
    let typ = sat_type(p._tail1.x);
    var kind = 0u;
    var face = sat_index_a(p._tail1.x);
    var n = vec3<f32>(0.0);
    if (typ == SAT_FACE_A) {
        if (face > 5u) {
            return empty_contact();
        }
        n = box_face_n(a, face);
        kind = 0u;
    } else if (typ == SAT_FACE_B) {
        face = sat_index_b(p._tail1.x);
        if (face > 5u) {
            return empty_contact();
        }
        n = -box_face_n(b, face);
        kind = 1u;
    } else {
        return empty_contact();
    }
    var c = clip_box_face(a, b, ia, ib, kind, face, n);
    if (c.a == EMPTY || c.count == 0u) {
        return empty_contact();
    }
    if (manifold_min_sep(c) >= SPECULATIVE) {
        return empty_contact();
    }
    let cache_sep = bitcast<f32>(p._tail1.y);
    if (abs(manifold_min_sep(c) - cache_sep) >= LINEAR_SLOP) {
        return empty_contact();
    }
    // A cache hit retains the original SAT baseline, as b3BuildFace*Contact's
    // localCache does. Rebasing on each hit admits unlimited incremental drift
    // and can keep a tilted box face instead of switching to its support plane.
    c._tail1.x = p._tail1.x;
    c._tail1.y = p._tail1.y;
    finish_manifold(&c, a, b, ia, ib);
    return c;
}

fn collide_boxes(a: Body, b: Body, ia: u32, ib: u32) -> Contact {
    if ((params.diagnostic_flags & DIAG_DISABLE_SAT_CACHE) == 0u) {
        let cached = try_cached_face(a, b, ia, ib);
        if (cached.a != EMPTY) {
            return cached;
        }
    }
    var c = empty_contact();
    c.a = ia;
    c.b = ib;
    c.friction = mixed_friction_of(a, b);

    var sep_a = -1e9;
    var face_a = 0u;
    var n_a = vec3<f32>(0.0, 1.0, 0.0);
    for (var f = 0u; f < 6u; f++) {
        let sep = hull_face_sep(a, b, f);
        if (sep >= SPECULATIVE) {
            c.a = EMPTY;
            return c;
        }
        if (sep > sep_a) {
            sep_a = sep;
            face_a = f;
            n_a = box_face_n(a, f);
        }
    }
    var sep_b = -1e9;
    var face_b = 0u;
    var n_b = vec3<f32>(0.0, 1.0, 0.0);
    for (var f = 0u; f < 6u; f++) {
        let sep = hull_face_sep(b, a, f);
        if (sep >= SPECULATIVE) {
            c.a = EMPTY;
            return c;
        }
        if (sep > sep_b) {
            sep_b = sep;
            face_b = f;
            n_b = -box_face_n(b, f);
        }
    }
    // Box3D: face A only if it has strictly greater separation (tie → B).
    var best_kind = 0u;
    var best_n = n_a;
    var best_face = face_a;
    if (sep_a <= sep_b) {
        best_kind = 1u;
        best_n = n_b;
        best_face = face_b;
    }

    let eq = gauss_edge_query(a, b);
    // The cold SAT query may early-out on a valid separating edge, matching
    // `b3ComputeSeparatingAxis(..., earlyReturn=true)`. Cached face validation
    // deliberately does not repeat this test.
    if (eq.index_a != EMPTY && eq.sep >= SPECULATIVE) {
        c.a = EMPTY;
        return c;
    }
    c = clip_box_face(a, b, ia, ib, best_kind, best_face, best_n);
    let clip_sep = select(-1e9, manifold_min_sep(c), c.a != EMPTY && c.count > 0u);
    // Box3D replaces a face manifold when the edge axis is better by one slop.
    let want_edge =
        eq.index_a != EMPTY
        && (c.a == EMPTY || c.count == 0u || eq.sep > clip_sep + LINEAR_SLOP);
    if (want_edge) {
        let ea = eq.index_a;
        let eb = eq.index_b;
        let pA0 = box_point(a, BOX_EDGE_ORIGIN[ea]);
        let pA1 = box_point(a, BOX_EDGE_ORIGIN[BOX_EDGE_TWIN[ea]]);
        let pB0 = box_point(b, BOX_EDGE_ORIGIN[eb]);
        let pB1 = box_point(b, BOX_EDGE_ORIGIN[BOX_EDGE_TWIN[eb]]);
        let dA = pA1 - pA0;
        let dB = pB1 - pB0;
        let st = closest_segments(pA0, dA, pB0, dB);
        if (st.x >= 0.0 && st.x <= 1.0 && st.y >= 0.0 && st.y <= 1.0) {
            let q1 = pA0 + st.x * dA;
            let q2 = pB0 + st.y * dB;
            var n_edge = quat_rotate(a.rot, eq.n);
            let nl = length(n_edge);
            if (nl > 1e-8) {
                n_edge = n_edge / nl;
            }
            let p = 0.5 * (q1 + q2);
            let s = dot(n_edge, q2 - q1);
            var edge_c = empty_contact();
            edge_c.a = ia;
            edge_c.b = ib;
            edge_c.friction = mixed_friction_of(a, b);
            edge_c.n = n_edge;
            edge_c.count = 1u;
            let rA = p - a.pos;
            let rB = p - b.pos;
            let base = s - dot(rB - rA, n_edge);
            set_point(&edge_c, 0u, vec4<f32>(rA, base), vec4<f32>(rB, 0.0));
            set_feat_at(&edge_c, 0u, pack_feature(0u, ea, 1u, eb));
            write_sat_cache(&edge_c, SAT_EDGE, ea, eb);
            c = edge_c;
        }
    } else if (c.a != EMPTY) {
        let typ = select(SAT_FACE_A, SAT_FACE_B, best_kind == 1u);
        if (best_kind == 0u) {
            write_sat_cache(&c, typ, best_face, box_support_vertex(b, -best_n));
        } else {
            write_sat_cache(&c, typ, box_support_vertex(a, best_n), best_face);
        }
    }
    if (c.a == EMPTY || c.count == 0u) {
        c.a = EMPTY;
        return c;
    }
    finish_manifold(&c, a, b, ia, ib);
    return c;
}

struct Seg2 {
    p0: vec3<f32>,
    p1: vec3<f32>,
    n: u32,
    feature0: u32,
    feature1: u32,
}

fn clip_seg(s: Seg2, plane_n: vec3<f32>, plane_c: f32) -> Seg2 {
    var outp: Seg2;
    outp.n = 0u;
    if (s.n < 2u) {
        return outp;
    }
    let d1 = dot(s.p0, plane_n) - plane_c;
    let d2 = dot(s.p1, plane_n) - plane_c;
    if (d1 <= 0.0) {
        outp.p0 = s.p0;
        outp.feature0 = s.feature0;
        outp.n = 1u;
    }
    if (d2 <= 0.0) {
        if (outp.n == 0u) {
            outp.p0 = s.p1;
            outp.feature0 = s.feature1;
        } else {
            outp.p1 = s.p1;
            outp.feature1 = s.feature1;
        }
        outp.n = outp.n + 1u;
    }
    if (d1 * d2 < 0.0) {
        let t = clip_divide(d1,d1-d2);
        let hit = mix(s.p0, s.p1, t);
        let feature = select(s.feature1, s.feature0, d1 > 0.0);
        if (outp.n == 0u) {
            outp.p0 = hit;
            outp.feature0 = feature;
        } else if (outp.n == 1u) {
            outp.p1 = hit;
            outp.feature1 = feature;
        }
        outp.n = min(outp.n + 1u, 2u);
    }
    return outp;
}

fn clip_seg_to_hull_face(hull: Body, face: u32, p0: vec3<f32>, p1: vec3<f32>) -> Seg2 {
    var s: Seg2;
    s.p0 = p0;
    s.p1 = p1;
    s.n = 2u;
    let ref_n = hull_face_n(hull, face);
    let edges = hull_face_edge_ids(face);
    for (var i = 0u; i < 4u; i++) {
        let e = edges[i];
        let v1 = box_point(hull, BOX_EDGE_ORIGIN[e]);
        let v2 = box_point(hull, BOX_EDGE_ORIGIN[BOX_EDGE_NEXT[e]]);
        var tangent = v2 - v1;
        let tl = length(tangent);
        if (tl < 1e-12) {
            continue;
        }
        tangent = tangent / tl;
        let binormal = cross(tangent, ref_n);
        s = clip_seg(s, binormal, dot(v1, binormal));
        if (s.n < 2u) {
            return s;
        }
    }
    return s;
}

fn hull_capsule_face_sep(hull: Body, cap: Body, face: u32) -> f32 {
    let n = hull_face_n(hull, face);
    let plane_c = dot(hull.pos, n) + hull_face_half(hull, face);
    let ax = capsule_axis(cap);
    return min(dot(cap.pos - ax, n), dot(cap.pos + ax, n)) - plane_c;
}

fn add_sep_point(c: ptr<function, Contact>, a: Body, b: Body, n: vec3<f32>, p: vec3<f32>, s: f32, i: u32) {
    let rA = p - a.pos;
    let rB = p - b.pos;
    let base = s - dot(rB - rA, n);
    set_point(c, i, vec4<f32>(rA, base), vec4<f32>(rB, 0.0));
}

// Retain local lever-arm precision when converting a capsule manifold to world axes.
// Contact.c rotates narrow-phase normals/points with b3MakeMatrixFromQuat.
// Keep that arithmetic distinct from the cross-product rotation used for COMs.
fn contact_matrix_rotate(q: vec4<f32>, v: vec3<f32>) -> vec3<f32> {
    let xx=q.x*q.x; let yy=q.y*q.y; let zz=q.z*q.z;
    let xy=q.x*q.y; let xz=q.x*q.z; let xw=q.x*q.w;
    let yz=q.y*q.z; let yw=q.y*q.w; let zw=q.z*q.w;
    let cx=vec3<f32>(1.0-2.0*(yy+zz),2.0*(xy+zw),2.0*(xz-yw));
    let cy=vec3<f32>(2.0*(xy-zw),1.0-2.0*(xx+zz),2.0*(yz+xw));
    let cz=vec3<f32>(2.0*(xz+yw),2.0*(yz-xw),1.0-2.0*(xx+yy));
    return (v.x*cx+v.y*cy)+v.z*cz;
}

fn add_capsule_local_point(c: ptr<function, Contact>, rotation: vec4<f32>,
    point: vec3<f32>, separation: f32, i: u32) {
    let a=load_body((*c).a); let b=load_body((*c).b);
    let center_a=quat_rotate(a.rot,load_body_cold((*c).a).local_center);
    let center_b=quat_rotate(b.rot,load_body_cold((*c).b).local_center);
    let origin_a=body_origin(a,(*c).a); let origin_b=body_origin(b,(*c).b);
    let anchor=contact_matrix_rotate(rotation,point);
    let ra=anchor-center_a;
    let rb=(anchor+(origin_a-origin_b))-center_b;
    let base=separation-gyro_dot3(rb-ra,(*c).n);
    set_point(c,i,vec4<f32>(ra,base),vec4<f32>(rb,0.0));
    if (i==0u) { (*c).persistent_rb0.w=separation; }
    else { (*c).persistent_rb1.w=separation; }
    // Raw separation and already COM-relative anchors.
    (*c).cached_relative.w=3.0;
}

// Match b3PointToSegmentDistance, including exact endpoint returns.
fn capsule_closest_point(a: vec3<f32>, b: vec3<f32>, p: vec3<f32>) -> vec3<f32> {
    let ab = b - a;
    let alpha = gyro_dot3(ab, p - a);
    if (alpha <= 0.0) {
        return a;
    }
    let denominator = gyro_dot3(ab, ab);
    if (alpha > denominator) {
        return b;
    }
    return a + gyro_divide(alpha, denominator) * ab;
}

// Empty and populated capsule pairs must use the same reference body: the
// conservative recycling bound is evaluated in that body's local frame.
fn capsule_reference_reversed(flags_a: u32, flags_b: u32, shape_a: u32, shape_b: u32) -> bool {
    let anchored_a = (flags_a & (FLAG_STATIC | FLAG_KINEMATIC)) != 0u;
    let anchored_b = (flags_b & (FLAG_STATIC | FLAG_KINEMATIC)) != 0u;
    return (!anchored_a && anchored_b)
        || (anchored_a == anchored_b && shape_a < shape_b);
}

fn collide_capsules(a: Body, b: Body, ia: u32, ib: u32) -> Contact {
    var c = empty_contact();
    // Work in body A coordinates. World-space endpoints lose enough precision
    // under translation to select the opposite end of nearly parallel capsules.
    let sa = load_shape(a._pad_island.x);
    let sb = load_shape(b._pad_island.x);
    let ba = load_body(ia);
    let bb = load_body(ib);
    let ca = load_body_cold(ia).local_center;
    let cb = load_body_cold(ib).local_center;
    let relative_rot = quat_mul(quat_inv(ba.rot), bb.rot);
    let origin_a = body_origin(ba,ia);
    let origin_b = body_origin(bb,ib);
    let relative_pos = quat_inv_rotate(ba.rot,origin_b-origin_a);
    let a0 = capsule_local_point(sa, 0u);
    let a1 = capsule_local_point(sa, 1u);
    let b0 = relative_pos + quat_rotate(relative_rot, capsule_local_point(sb, 0u));
    let b1 = relative_pos + quat_rotate(relative_rot, capsule_local_point(sb, 1u));
    let da = a1 - a0;
    let db = b1 - b0;
    let st = closest_segments(a0, da, b0, db);
    let pA = a0 + st.x * da;
    let pB = b0 + st.y * db;
    let offset = pB - pA;
    let dist2 = gyro_dot3(offset, offset);
    let radius = a.half.x + b.half.x;
    let max_d = radius + SPECULATIVE;
    if (dist2 > max_d * max_d) {
        return c;
    }
    let min_d = 0.01 * LINEAR_SLOP;
    if (dist2 < min_d * min_d) {
        return c;
    }
    let la = gyro_sqrt(gyro_dot3(da, da));
    let lb = gyro_sqrt(gyro_dot3(db, db));
    if (la < 1e-4 || lb < 1e-4) {
        return c;
    }
    let eA = gyro_recip(la) * da;
    let eB = gyro_recip(lb) * db;
    let cr = gyro_cross(eA, eB);
    c.a = ia;
    c.b = ib;
    c.friction = mixed_friction_of(a, b);
    // Preserve native float32 multiplication at the parallelism boundary.
    let alpha_tol: f32 = 0.05;
    if (gyro_dot3(cr, cr) < alpha_tol * alpha_tol) {
        var seg: Seg2;
        seg.p0 = b0;
        seg.p1 = b1;
        seg.n = 2u;
        seg.feature0 = pack_feature(0u, 0u, 0u, 0u);
        seg.feature1 = pack_feature(0u, 1u, 0u, 1u);
        seg = clip_seg(seg, -eA, -dot(eA, a0));
        if (seg.n == 2u) {
            seg = clip_seg(seg, eA, dot(eA, a1));
        }
        if (seg.n == 2u) {
            let q0 = capsule_closest_point(a0, a1, seg.p0);
            let q1 = capsule_closest_point(a0, a1, seg.p1);
            let d0 = gyro_sqrt(gyro_dot3(seg.p0 - q0, seg.p0 - q0));
            let d1 = gyro_sqrt(gyro_dot3(seg.p1 - q1, seg.p1 - q1));
            if (d0 <= radius && d1 <= radius && d0 >= min_d && d1 >= min_d) {
                let n0 = gyro_recip(d0) * (seg.p0 - q0);
                let n1 = gyro_recip(d1) * (seg.p1 - q1);
                var n = n0 + n1;
                let nl = gyro_sqrt(gyro_dot3(n, n));
                if (nl > 1e-8) {
                    n = gyro_recip(nl) * n;
                } else {
                    n = n0;
                }
                c.n = contact_matrix_rotate(ba.rot, n);
                c.count = 2u;
                let p0 = 0.5 * ((seg.p0 + n0 * a.half.x + q0) - n * b.half.x);
                let p1 = 0.5 * ((seg.p1 + n1 * a.half.x + q1) - n * b.half.x);
                add_capsule_local_point(&c, ba.rot, p0, d0 - radius, 0u);
                add_capsule_local_point(&c, ba.rot, p1, d1 - radius, 1u);
                set_feat_at(&c, 0u, seg.feature0);
                set_feat_at(&c, 1u, seg.feature1);
                finish_manifold(&c, a, b, ia, ib);
                return c;
            }
        }
    }
    let dist = gyro_sqrt(dist2);
    let n = gyro_recip(dist) * offset;
    c.n = contact_matrix_rotate(ba.rot, n);
    c.count = 1u;
    let p = 0.5 * ((pA + n * a.half.x + pB) - n * b.half.x);
    add_capsule_local_point(&c, ba.rot, p, dist - radius, 0u);
    finish_manifold(&c, a, b, ia, ib);
    return c;
}

fn collide_capsule_box(cap: Body, hull: Body, ic: u32, ih: u32) -> Contact {
    var c = empty_contact();
    let rad = cap.half.x;
    var best_sep = -1e9;
    var best_face = 0u;
    for (var f = 0u; f < 6u; f++) {
        let sep = hull_capsule_face_sep(hull, cap, f);
        if (sep > rad + SPECULATIVE) {
            return c;
        }
        if (sep > best_sep) {
            best_sep = sep;
            best_face = f;
        }
    }
    let ax = capsule_axis(cap);
    let p0 = cap.pos - ax;
    let p1 = cap.pos + ax;
    let seg = clip_seg_to_hull_face(hull, best_face, p0, p1);
    let hn = hull_face_n(hull, best_face);
    let plane_c = dot(hull.pos, hn) + hull_face_half(hull, best_face);
    let n = -hn;
    c.a = ic;
    c.b = ih;
    c.friction = mixed_friction_of(cap, hull);
    if (seg.n == 2u) {
        let s0 = (dot(seg.p0, hn) - plane_c) - rad;
        let s1 = (dot(seg.p1, hn) - plane_c) - rad;
        if (s0 < SPECULATIVE || s1 < SPECULATIVE) {
            c.n = n;
            c.count = 2u;
            let q0 = seg.p0 - 0.5 * (s0 + 2.0 * rad) * hn;
            let q1 = seg.p1 - 0.5 * (s1 + 2.0 * rad) * hn;
            add_sep_point(&c, cap, hull, n, q0, s0, 0u);
            add_sep_point(&c, cap, hull, n, q1, s1, 1u);
            finish_manifold(&c, cap, hull, ic, ih);
            return c;
        }
    }
    let closest = closest_on_obb(cap.pos, hull);
    let mid = closest_on_segment(closest, p0, p1);
    let delta = mid - closest;
    let dist = length(delta);
    if (dist > 1e-8 && dist < rad + SPECULATIVE) {
        var nn = delta / dist;
        if (dot(hull.pos - cap.pos, nn) < 0.0) {
            nn = -nn;
        }
        c.n = nn;
        c.count = 1u;
        add_sep_point(&c, cap, hull, nn, closest, dist - rad, 0u);
        finish_manifold(&c, cap, hull, ic, ih);
        return c;
    }
    return empty_contact();
}

fn project_collider(shape: Shape, body: Body, axis: vec3<f32>) -> vec2<f32> {
    if (body.kind == KIND_CONVEX_HULL) {
        var lo = 1e30;
        var hi = -1e30;
        let count = hull_point_count(shape);
        for (var i = 0u; i < count; i++) {
            let local = load_hull_point(shape, i) - shape.local_center;
            let value = dot(body.pos + quat_rotate(body.rot, local), axis);
            lo = min(lo, value);
            hi = max(hi, value);
        }
        return vec2<f32>(lo, hi);
    }
    if (body.kind == KIND_SPHERE) {
        let center = dot(body.pos, axis);
        return vec2<f32>(center - body.half.x, center + body.half.x);
    }
    if (body.kind == KIND_CAPSULE) {
        let extent = abs(dot(capsule_axis(body), axis)) + body.half.x;
        let center = dot(body.pos, axis);
        return vec2<f32>(center - extent, center + extent);
    }
    let extent = abs(dot(box_axis(body, 0u), axis)) * body.half.x
        + abs(dot(box_axis(body, 1u), axis)) * body.half.y
        + abs(dot(box_axis(body, 2u), axis)) * body.half.z;
    let center = dot(body.pos, axis);
    return vec2<f32>(center - extent, center + extent);
}

fn support_collider(shape: Shape, body: Body, direction: vec3<f32>) -> vec3<f32> {
    if (body.kind == KIND_CONVEX_HULL) {
        var point = body.pos;
        var best = -1e30;
        let count = hull_point_count(shape);
        for (var i = 0u; i < count; i++) {
            let local = load_hull_point(shape, i) - shape.local_center;
            let world = body.pos + quat_rotate(body.rot, local);
            let value = dot(world, direction);
            if (value > best) {
                best = value;
                point = world;
            }
        }
        return point;
    }
    if (body.kind == KIND_SPHERE) {
        return body.pos + normalize(direction) * body.half.x;
    }
    if (body.kind == KIND_CAPSULE) {
        let half_axis = capsule_axis(body);
        let end = select(-half_axis, half_axis, dot(half_axis, direction) >= 0.0);
        return body.pos + end + normalize(direction) * body.half.x;
    }
    return body.pos
        + box_axis(body, 0u) * select(-body.half.x, body.half.x, dot(box_axis(body, 0u), direction) >= 0.0)
        + box_axis(body, 1u) * select(-body.half.y, body.half.y, dot(box_axis(body, 1u), direction) >= 0.0)
        + box_axis(body, 2u) * select(-body.half.z, body.half.z, dot(box_axis(body, 2u), direction) >= 0.0);
}

fn test_convex_axis(
    shape_a: Shape,
    shape_b: Shape,
    a: Body,
    b: Body,
    candidate: vec3<f32>,
    best_separation: ptr<function, f32>,
    best_normal: ptr<function, vec3<f32>>,
) -> bool {
    if (dot(candidate, candidate) < 1e-12) {
        return true;
    }
    var axis = normalize(candidate);
    if (dot(b.pos - a.pos, axis) < 0.0) {
        axis = -axis;
    }
    let pa = project_collider(shape_a, a, axis);
    let pb = project_collider(shape_b, b, axis);
    let separation = pb.x - pa.y;
    if (separation > SPECULATIVE) {
        return false;
    }
    if (separation > (*best_separation)) {
        (*best_separation) = separation;
        (*best_normal) = axis;
    }
    return true;
}

fn point_inside_collider(shape: Shape, body: Body, point: vec3<f32>, slop: f32) -> bool {
    if (body.kind == KIND_CONVEX_HULL) {
        let local = quat_inv_rotate(body.rot, point - body.pos) + shape.local_center;
        for (var i = 0u; i < hull_plane_count(shape); i++) {
            let plane = load_hull_plane(shape, i);
            if (dot(plane.xyz, local) - plane.w > slop) {
                return false;
            }
        }
        return true;
    }
    if (body.kind == KIND_BOX) {
        let local = quat_inv_rotate(body.rot, point - body.pos);
        return all(abs(local) <= body.half + vec3<f32>(slop));
    }
    if (body.kind == KIND_SPHERE) {
        let d = point - body.pos;
        return dot(d, d) <= (body.half.x + slop) * (body.half.x + slop);
    }
    let axis = capsule_axis(body);
    let closest = closest_on_segment(point, body.pos - axis, body.pos + axis);
    let d = point - closest;
    return dot(d, d) <= (body.half.x + slop) * (body.half.x + slop);
}

fn consider_convex_manifold_point(
    point: vec3<f32>,
    tangent1: vec3<f32>,
    tangent2: vec3<f32>,
    points: ptr<function, array<vec3<f32>, 4>>,
    scores: ptr<function, vec4<f32>>,
    valid: ptr<function, vec4<bool>>,
) {
    let values = vec4<f32>(
        dot(point, tangent1),
        -dot(point, tangent1),
        dot(point, tangent2),
        -dot(point, tangent2),
    );
    for (var i = 0u; i < 4u; i++) {
        if (!(*valid)[i] || values[i] > (*scores)[i]) {
            (*scores)[i] = values[i];
            (*points)[i] = point;
            (*valid)[i] = true;
        }
    }
}

struct ClipPolygon {
    points: array<vec3<f32>, 32>,
    features: array<u32, 32>,
    count: u32,
}

fn clip_polygon(input: ClipPolygon, plane_normal: vec3<f32>, plane_offset: f32) -> ClipPolygon {
    return clip_polygon_with_features(input, plane_normal, plane_offset, EMPTY, 0u);
}

fn clip_polygon_with_features(input: ClipPolygon, plane_normal: vec3<f32>, plane_offset: f32, edge: u32, ref_owner: u32) -> ClipPolygon {
    var output: ClipPolygon;
    output.count = 0u;
    if (input.count == 0u) {
        return output;
    }
    var previous = input.points[input.count - 1u];
    var previous_distance = dot(previous, plane_normal) - plane_offset;
    var previous_feature = input.features[input.count - 1u];
    // Preserve the existing generic clip tolerance; feature clips follow CPU planes.
    let tolerance = select(0.0, 1e-6, edge == EMPTY);
    for (var i = 0u; i < input.count; i++) {
        let current = input.points[i];
        let current_distance = dot(current, plane_normal) - plane_offset;
        let current_feature = input.features[i];
        if ((previous_distance <= tolerance) != (current_distance <= tolerance)) {
            let t = clip_divide(previous_distance,previous_distance-current_distance);
            if (output.count < 32u) {
                // Match b3ClipPolygon: weighted endpoints round differently.
                output.points[output.count] = previous + t * (current - previous);
                var feature = 0u;
                if (edge != EMPTY) {
                    if (previous_distance <= tolerance) {
                        feature = pack_feature((current_feature >> 24u) & 1u, (current_feature >> 16u) & 255u, ref_owner, edge);
                    } else {
                        feature = pack_feature(ref_owner, edge, (previous_feature >> 8u) & 1u, previous_feature & 255u);
                    }
                }
                output.features[output.count] = feature;
                output.count = output.count + 1u;
            }
        }
        if (current_distance <= tolerance && output.count < 32u) {
            output.points[output.count] = current;
            output.features[output.count] = current_feature;
            output.count = output.count + 1u;
        }
        previous = current;
        previous_feature = current_feature;
        previous_distance = current_distance;
    }
    return output;
}

fn find_face_edge(shape: Shape, face: u32) -> u32 {
    for (var i = 0u; i < hull_topology_count(shape); i++) {
        if (load_hull_topology(shape, i).w == face) {
            return i;
        }
    }
    return EMPTY;
}

fn hull_reduction_perp(v: vec3<f32>) -> vec3<f32> {
    if (abs(v.x) > 0.5) { return normalize(vec3<f32>(0.67*v.y - 0.42*v.z, -0.67*v.x, 0.42*v.x)); }
    if (abs(v.y) > 0.5) { return normalize(vec3<f32>(0.67*v.y, -0.67*v.x - 0.42*v.z, 0.42*v.y)); }
    return normalize(vec3<f32>(0.67*v.z, -0.42*v.z, -0.67*v.x + 0.42*v.y));
}

// Box3D's face-manifold reduction: preserve small polygons, otherwise retain
// an extreme touching point, a distant point, and points that expand the area.
// Score in the reference body's local frame, as the CPU does.
fn reduce_hull_polygon(polygon: ClipPolygon, body: Body, normal: vec3<f32>, reference_point: vec3<f32>) -> vec4<u32> {
    var chosen = vec4<u32>(EMPTY);
    var points: array<vec4<f32>, 32>;
    var remaining: array<u32, 32>;
    let local_normal = quat_rotate(quat_inv(body.rot), normal);
    for (var i = 0u; i < polygon.count; i++) {
        let separation = dot(polygon.points[i] - reference_point, normal);
        let midpoint = polygon.points[i] - 0.5 * separation * normal;
        points[i] = vec4<f32>(quat_rotate(quat_inv(body.rot), midpoint - body.pos), separation);
        remaining[i] = i;
    }
    if (polygon.count <= 4u) {
        for (var i = 0u; i < polygon.count; i++) { chosen[i] = i; }
        return chosen;
    }
    var count = polygon.count;
    let tol = SPECULATIVE * SPECULATIVE;
    let bias = 0.95;
    var signed_area = 0.0;
    for (var stage = 0u; stage < 4u; stage++) {
        var best = EMPTY;
        var best_score = select(select(tol, 0.0, stage == 1u), -1e30, stage == 0u);
        for (var i = 0u; i < count; i++) {
            let point = points[remaining[i]];
            var score = 0.0;
            var area = 0.0;
            if (stage == 0u) {
                if (point.w > SPECULATIVE) { continue; }
                score = -point.w + dot(hull_reduction_perp(local_normal), point.xyz);
            } else {
                let a = points[chosen[0]].xyz;
                let d = point.xyz - a;
                if (stage == 1u) {
                    let v = d - dot(d, local_normal) * local_normal;
                    let depth = max(0.0, -point.w);
                    score = dot(v, v) + 4.0 * depth * depth;
                } else {
                    let b = points[chosen[1]].xyz;
                    let ba = b - a;
                    if (stage == 2u) {
                        area = dot(local_normal, cross(ba, d));
                        score = abs(area);
                    } else {
                        let c = points[chosen[2]].xyz;
                        let sign = select(1.0, -1.0, signed_area < 0.0);
                        let u1 = sign * dot(local_normal, cross(d, ba));
                        let u2 = sign * dot(local_normal, cross(point.xyz - b, c - b));
                        let u3 = sign * dot(local_normal, cross(point.xyz - c, a - c));
                        score = max(u1, max(u2, u3));
                    }
                }
            }
            if (bias * score > best_score || (stage == 2u && bias * score == best_score)) {
                best_score = score;
                best = i;
                if (stage == 2u) { signed_area = area; }
            }
        }
        if (best == EMPTY || (stage == 1u && best_score < tol)) { break; }
        chosen[stage] = remaining[best];
        count = count - 1u;
        remaining[best] = remaining[count];
    }
    return chosen;
}

fn clipped_hull_manifold(
    shape_a: Shape,
    shape_b: Shape,
    a: Body,
    b: Body,
    ia: u32,
    ib: u32,
    reference_is_b: bool,
    reference_face: u32,
) -> Contact {
    var reference_shape = shape_a;
    var incident_shape = shape_b;
    var reference_body = a;
    var incident_body = b;
    if (reference_is_b) {
        reference_shape = shape_b;
        incident_shape = shape_a;
        reference_body = b;
        incident_body = a;
    }

    let reference_plane = load_hull_plane(reference_shape, reference_face);
    let reference_normal = quat_rotate(reference_body.rot, reference_plane.xyz);
    var incident_face = 0u;
    var incident_alignment = 1e30;
    for (var i = 0u; i < hull_plane_count(incident_shape); i++) {
        let candidate = quat_rotate(incident_body.rot, load_hull_plane(incident_shape, i).xyz);
        let alignment = dot(candidate, reference_normal);
        if (alignment < incident_alignment) {
            incident_alignment = alignment;
            incident_face = i;
        }
    }

    let reference_start = find_face_edge(reference_shape, reference_face);
    let incident_start = find_face_edge(incident_shape, incident_face);
    if (reference_start == EMPTY || incident_start == EMPTY) {
        return empty_contact();
    }

    var polygon: ClipPolygon;
    polygon.count = 0u;
    var edge = incident_start;
    for (var i = 0u; i < 32u; i++) {
        let topology = load_hull_topology(incident_shape, edge);
        let local = load_hull_point(incident_shape, topology.z) - incident_shape.local_center;
        polygon.points[polygon.count] = incident_body.pos + quat_rotate(incident_body.rot, local);
        polygon.count = polygon.count + 1u;
        edge = topology.x;
        if (edge == incident_start || polygon.count == 32u) {
            break;
        }
    }

    edge = reference_start;
    for (var i = 0u; i < 32u; i++) {
        let topology = load_hull_topology(reference_shape, edge);
        let next_topology = load_hull_topology(reference_shape, topology.x);
        let local_a = load_hull_point(reference_shape, topology.z) - reference_shape.local_center;
        let local_b = load_hull_point(reference_shape, next_topology.z) - reference_shape.local_center;
        let world_a = reference_body.pos + quat_rotate(reference_body.rot, local_a);
        let world_b = reference_body.pos + quat_rotate(reference_body.rot, local_b);
        let tangent = world_b - world_a;
        var side_normal = normalize(cross(tangent, reference_normal));
        if (dot(reference_body.pos - world_a, side_normal) > 0.0) {
            side_normal = -side_normal;
        }
        polygon = clip_polygon(polygon, side_normal, dot(side_normal, world_a) + 1e-6);
        edge = topology.x;
        if (edge == reference_start || polygon.count == 0u) {
            break;
        }
    }

    var c = empty_contact();
    c.a = ia;
    c.b = ib;
    c.n = select(reference_normal, -reference_normal, reference_is_b);
    c.friction = mixed_friction_of(a, b);
    let reference_topology = load_hull_topology(reference_shape, reference_start);
    let reference_local =
        load_hull_point(reference_shape, reference_topology.z) - reference_shape.local_center;
    let reference_point = reference_body.pos + quat_rotate(reference_body.rot, reference_local);
    var min_separation = 1e30;
    for (var i = 0u; i < polygon.count; i++) {
        min_separation = min(min_separation, dot(polygon.points[i] - reference_point, reference_normal));
    }
    if (min_separation >= SPECULATIVE) { return empty_contact(); }
    let selected = reduce_hull_polygon(polygon, reference_body, reference_normal, reference_point);
    for (var i = 0u; i < 4u; i++) {
        if (selected[i] == EMPTY) { continue; }
        let candidate = polygon.points[selected[i]];
        let point_separation = dot(candidate - reference_point, reference_normal);
        let point = candidate - 0.5 * point_separation * reference_normal;
        var duplicate = false;
        for (var j = 0u; j < c.count; j++) {
            let old = ra_at(c, j).xyz + a.pos;
            let d = point - old;
            duplicate = duplicate || dot(d, d) < 1e-8;
        }
        if (duplicate) {
            continue;
        }
        let r_a = point - a.pos;
        let r_b = point - b.pos;
        let base = point_separation - dot(r_b - r_a, c.n);
        set_point(&c, c.count, vec4<f32>(r_a, base), vec4<f32>(r_b, 0.0));
        c.count = c.count + 1u;
    }
    if (c.count > 0u) {
        finish_manifold(&c, a, b, ia, ib);
    }
    return c;
}

fn hull_world_point(shape: Shape, body: Body, vertex: u32) -> vec3<f32> {
    let local = load_hull_point(shape, vertex) - shape.local_center;
    return body.pos + quat_rotate(body.rot, local);
}

struct GaussEdge {
    q: vec3<f32>,
    e: vec3<f32>,
    u: vec3<f32>,
    v: vec3<f32>,
    valid: bool,
}

fn gauss_hull_edge(shape: Shape, body: Body, edge_index: u32) -> GaussEdge {
    var edge: GaussEdge;
    edge.valid = false;
    let count = hull_topology_count(shape);
    if (edge_index >= count) {
        return edge;
    }
    let half = load_hull_topology(shape, edge_index);
    if (half.y >= count || half.z >= hull_point_count(shape) || half.w >= hull_plane_count(shape)) {
        return edge;
    }
    let twin = load_hull_topology(shape, half.y);
    if (twin.z >= hull_point_count(shape) || twin.w >= hull_plane_count(shape)) {
        return edge;
    }
    let origin = hull_world_point(shape, body, half.z);
    edge.q = hull_world_point(shape, body, twin.z);
    edge.e = edge.q - origin;
    edge.u = quat_rotate(body.rot, load_hull_plane(shape, half.w).xyz);
    edge.v = quat_rotate(body.rot, load_hull_plane(shape, twin.w).xyz);
    edge.valid = true;
    return edge;
}

fn gauss_box_edge(body: Body, undirected: u32) -> GaussEdge {
    var edge: GaussEdge;
    edge.valid = false;
    if (undirected >= 12u) {
        return edge;
    }
    let index = 2u * undirected;
    let twin = BOX_EDGE_TWIN[index];
    let origin = box_point(body, BOX_EDGE_ORIGIN[index]);
    edge.q = box_point(body, BOX_EDGE_ORIGIN[twin]);
    edge.e = edge.q - origin;
    edge.u = hull_face_n(body, BOX_EDGE_FACE[index]);
    edge.v = hull_face_n(body, BOX_EDGE_FACE[twin]);
    edge.valid = true;
    return edge;
}

fn gauss_minkowski_axis(edge_a: GaussEdge, edge_b: GaussEdge) -> vec3<f32> {
    let cba = dot(edge_b.u, edge_a.e);
    let dba = dot(edge_b.v, edge_a.e);
    let adc = -dot(edge_a.u, edge_b.e);
    let bdc = -dot(edge_a.v, edge_b.e);
    if (cba * dba >= 0.0 || adc * bdc >= 0.0 || cba * bdc <= 0.0) {
        return vec3<f32>(0.0);
    }
    let squared_tol = PARALLEL_EDGE_TOL * PARALLEL_EDGE_TOL;
    if (max(cba * cba, dba * dba) < squared_tol * dot(edge_a.e, edge_a.e)) {
        return vec3<f32>(0.0);
    }
    let t = cba / (cba - dba);
    var axis = mix(edge_b.u, edge_b.v, t);
    let len2 = dot(axis, axis);
    if (len2 < 1e-20) {
        return vec3<f32>(0.0);
    }
    return axis;
}

fn best_hull_face(shape: Shape, body: Body, outward: vec3<f32>) -> u32 {
    var best = EMPTY;
    var best_alignment = -1e30;
    for (var i = 0u; i < hull_plane_count(shape); i++) {
        let alignment = dot(quat_rotate(body.rot, load_hull_plane(shape, i).xyz), outward);
        if (alignment > best_alignment) {
            best_alignment = alignment;
            best = i;
        }
    }
    return best;
}

fn test_gauss_edge_pair(
    shape_a: Shape,
    shape_b: Shape,
    a: Body,
    b: Body,
    edge_a: GaussEdge,
    edge_b: GaussEdge,
    best_separation: ptr<function, f32>,
    best_normal: ptr<function, vec3<f32>>,
) -> bool {
    if (!edge_a.valid || !edge_b.valid) {
        return true;
    }
    let axis = gauss_minkowski_axis(edge_a, edge_b);
    if (dot(axis, axis) < 1e-20) {
        return true;
    }
    return test_convex_axis(shape_a, shape_b, a, b, axis, best_separation, best_normal);
}

// A separated segment's closest polyhedron feature is an endpoint/face or
// segment/edge pair. Work in hull-local coordinates to retain small witnesses
// on off-center compound children. Core intersections retain the penetration
// path below; this result does not invent a normal for a zero-distance core.
struct HullCapsuleWitnessResult {
    contact: Contact,
    core_overlap: bool,
}
fn hull_capsule_witness(hull: Body, cap: Body, ih: u32, ic: u32) -> HullCapsuleWitnessResult {
    let shape = load_shape(hull._pad_island.x);
    let planes = hull_plane_count(shape);
    let edges = hull_topology_count(shape);
    var result: HullCapsuleWitnessResult;
    result.contact = empty_contact();
    result.core_overlap = false;
    if (planes == 0u || edges == 0u) { result.core_overlap = true; return result; }
    let axis = capsule_axis(cap);
    let p0 = quat_inv_rotate(hull.rot, cap.pos - axis - hull.pos) + shape.local_center;
    let p1 = quat_inv_rotate(hull.rot, cap.pos + axis - hull.pos) + shape.local_center;
    let segment = p1 - p0;
    let radius = cap.half.x;
    let limit = radius + SPECULATIVE;
    var enter = 0.0;
    var leave = 1.0;
    for (var f = 0u; f < planes; f++) {
        let plane = load_hull_plane(shape, f);
        let d0 = dot(plane.xyz, p0) - plane.w;
        let d1 = dot(plane.xyz, p1) - plane.w;
        if (min(d0, d1) > limit) { return result; }
        if (d0 > 0.0 && d1 > 0.0) { enter = 2.0; }
        else if (d0 > 0.0) { enter = max(enter, d0 / (d0 - d1)); }
        else if (d1 > 0.0) { leave = min(leave, d0 / (d0 - d1)); }
    }
    if (enter <= leave) { result.core_overlap = true; return result; }
    var best2 = limit * limit;
    var point_h = vec3<f32>(0.0);
    var point_c = vec3<f32>(0.0);
    var found = false;
    for (var f = 0u; f < planes; f++) {
        let plane = load_hull_plane(shape, f);
        for (var endpoint = 0u; endpoint < 2u; endpoint++) {
            let p = select(p0, p1, endpoint == 1u);
            let d = dot(plane.xyz, p) - plane.w;
            if (d * d >= best2) { continue; }
            let q = p - d * plane.xyz;
            var inside = true;
            for (var g = 0u; g < planes; g++) {
                let side = load_hull_plane(shape, g);
                if (dot(side.xyz, q) - side.w > 1e-6) { inside = false; break; }
            }
            if (inside) { best2 = d*d; point_h = q; point_c = p; found = true; }
        }
    }
    for (var e = 0u; e < edges; e++) {
        let half = load_hull_topology(shape, e);
        if (e >= half.y || half.y >= edges) { continue; }
        let twin = load_hull_topology(shape, half.y);
        let q0 = load_hull_point(shape, half.z);
        let q1 = load_hull_point(shape, twin.z);
        let st = closest_segments(q0, q1-q0, p0, segment);
        let q = q0 + st.x * (q1-q0);
        let p = p0 + st.y * segment;
        let d2 = dot(p-q, p-q);
        if (d2 < best2) { best2 = d2; point_h = q; point_c = p; found = true; }
    }
    if (!found) { return result; }
    if (best2 <= 1e-10) { result.core_overlap = true; return result; }
    let distance = sqrt(best2);
    let local_n = (point_c - point_h) / distance;
    let n = quat_rotate(hull.rot, local_n);
    var c = empty_contact();
    c.a = ih; c.b = ic; c.n = n; c.friction = mixed_friction_of(hull, cap);
    var face = EMPTY;
    var alignment = 0.998;
    for (var f = 0u; f < planes; f++) {
        let value = dot(load_hull_plane(shape, f).xyz, local_n);
        if (value > alignment) { face = f; alignment = value; }
    }
    if (face != EMPTY) {
        let plane = load_hull_plane(shape, face);
        let d0 = dot(plane.xyz, p0) - plane.w;
        let d1 = dot(plane.xyz, p1) - plane.w;
        // Clip the projected segment to the reference polygon. Evaluating all
        // hull planes on that face gives exactly its polygon half-spaces.
        let q0 = p0 - d0 * plane.xyz;
        let q1 = p1 - d1 * plane.xyz;
        var lo = 0.0;
        var hi = 1.0;
        for (var f = 0u; f < planes; f++) {
            if (f == face) { continue; }
            let side = load_hull_plane(shape, f);
            let a = dot(side.xyz, q0) - side.w;
            let b = dot(side.xyz, q1) - side.w;
            if (a > 0.0 && b > 0.0) { lo = 2.0; break; }
            if (a > 0.0) { lo = max(lo, a / (a-b)); }
            else if (b > 0.0) { hi = min(hi, a / (a-b)); }
        }
        if (lo <= hi) {
            let x0 = p0 + lo * segment;
            let x1 = p0 + hi * segment;
            let s0 = dot(plane.xyz, x0) - plane.w - radius;
            let s1 = dot(plane.xyz, x1) - plane.w - radius;
            if (min(s0,s1) <= SPECULATIVE) {
                c.n = quat_rotate(hull.rot, plane.xyz);
                let w0 = hull.pos + quat_rotate(hull.rot, x0 - 0.5*(s0+2.0*radius)*plane.xyz - shape.local_center);
                let w1 = hull.pos + quat_rotate(hull.rot, x1 - 0.5*(s1+2.0*radius)*plane.xyz - shape.local_center);
                add_sep_point(&c, hull, cap, c.n, w0, s0, 0u);
                c.count = 1u;
                if (dot(w1-w0,w1-w0) > 1e-10) { add_sep_point(&c, hull, cap, c.n, w1, s1, 1u); c.count = 2u; }
            }
        }
    }
    if (c.count == 0u) {
        let local_point = 0.5 * (point_h + point_c - radius * local_n);
        let point = hull.pos + quat_rotate(hull.rot, local_point - shape.local_center);
        c.count = 1u;
        add_sep_point(&c, hull, cap, n, point, distance-radius, 0u);
    }
    finish_manifold(&c, hull, cap, ih, ic);
    result.contact = c;
    return result;
}

struct HullEdgeQuery {
    separation: f32,
    normal: vec3<f32>,
    edge_a: u32,
    edge_b: u32,
}

fn query_hull_edges(sa: Shape, sb: Shape, a: Body, b: Body) -> HullEdgeQuery {
    var result: HullEdgeQuery;
    result.separation = -1e30;
    result.edge_a = EMPTY;
    result.edge_b = EMPTY;
    // CPU Gauss-map query uses B's negated normals and edge directions.
    for (var j = 0u; j < hull_topology_count(sb); j++) {
        if (j >= load_hull_topology(sb, j).y) { continue; }
        let eb = gauss_hull_edge(sb, b, j);
        if (!eb.valid) { continue; }
        let C = -eb.u;
        let D = -eb.v;
        let dc = -eb.e;
        for (var i = 0u; i < hull_topology_count(sa); i++) {
            if (i >= load_hull_topology(sa, i).y) { continue; }
            let ea = gauss_hull_edge(sa, a, i);
            if (!ea.valid) { continue; }
            let cba = dot(C, ea.e);
            let dba = dot(D, ea.e);
            let adc = dot(ea.u, dc);
            let bdc = dot(ea.v, dc);
            if (cba*dba >= -0.0001 || adc*bdc >= -0.0001 || cba*bdc >= -0.0001) { continue; }
            let tol = PARALLEL_EDGE_TOL * PARALLEL_EDGE_TOL * dot(ea.e, ea.e);
            if (max(cba*cba, dba*dba) <= tol) { continue; }
            let n = normalize(mix(C, D, cba / (cba - dba)));
            let separation = dot(n, eb.q - ea.q);
            if (separation > result.separation) {
                result.separation = separation;
                result.normal = n;
                result.edge_a = i;
                result.edge_b = j;
                if (separation > SPECULATIVE) { return result; }
            }
        }
    }
    return result;
}

fn hull_edge_manifold(sa: Shape, sb: Shape, a: Body, b: Body, ia: u32, ib: u32, query: HullEdgeQuery) -> Contact {
    if (query.edge_a == EMPTY) { return empty_contact(); }
    let ea = gauss_hull_edge(sa, a, query.edge_a);
    let eb = gauss_hull_edge(sb, b, query.edge_b);
    let pa = ea.q - ea.e;
    let pb = eb.q - eb.e;
    let w = pa - pb;
    let a11 = dot(ea.e, ea.e);
    let a12 = -dot(ea.e, eb.e);
    let a21 = -a12;
    let a22 = -dot(eb.e, eb.e);
    let b1 = -dot(ea.e, w);
    let b2 = -dot(eb.e, w);
    let det = a11*a22 - a12*a21;
    var s = 0.0;
    var t = 0.0;
    if (det*det < 1.17549435e-35) {
        s = dot(pb-pa, ea.e) / a11;
    } else {
        s = (a22*b1 - a12*b2) / det;
        t = (a11*b2 - a21*b1) / det;
    }
    if (s < 0.0 || s > 1.0 || t < 0.0 || t > 1.0) { return empty_contact(); }
    let p = pa + s*ea.e;
    let q = pb + t*eb.e;
    let separation = dot(query.normal, q-p);
    var c = empty_contact();
    c.a = ia; c.b = ib; c.count = 1u; c.n = query.normal; c.friction = mixed_friction_of(a,b);
    add_sep_point(&c, a, b, query.normal, 0.5*(p+q), separation, 0u);
    finish_manifold(&c, a, b, ia, ib);
    return c;
}

fn collide_convex_pair(a: Body, b: Body, ia: u32, ib: u32) -> Contact {
    if (a.kind == KIND_CONVEX_HULL && b.kind == KIND_CAPSULE) {
        let witness = hull_capsule_witness(a, b, ia, ib);
        if (!witness.core_overlap) { return witness.contact; }
    } else if (b.kind == KIND_CONVEX_HULL && a.kind == KIND_CAPSULE) {
        let witness = hull_capsule_witness(b, a, ib, ia);
        if (!witness.core_overlap) { return witness.contact; }
    }
    let shape_a = load_shape(a._pad_island.x);
    let shape_b = load_shape(b._pad_island.x);
    var face_separation_a = -1e30;
    var face_normal_a = vec3<f32>(0.0, 1.0, 0.0);
    var face_index_a = EMPTY;
    var face_separation_b = -1e30;
    var face_normal_b = vec3<f32>(0.0, 1.0, 0.0);
    var face_index_b = EMPTY;
    var edge_separation = -1e30;
    var edge_normal = vec3<f32>(0.0, 1.0, 0.0);
    var has_edge_axis = false;
    for (var i = 0u; i < hull_plane_count(shape_a); i++) {
        let plane = load_hull_plane(shape_a, i);
        let axis = quat_rotate(a.rot, plane.xyz);
        let old_separation = face_separation_a;
        if (!test_convex_axis(shape_a, shape_b, a, b, axis, &face_separation_a, &face_normal_a)) {
            return empty_contact();
        }
        if (face_separation_a > old_separation) {
            face_index_a = i;
        }
    }
    for (var i = 0u; i < hull_plane_count(shape_b); i++) {
        let plane = load_hull_plane(shape_b, i);
        let axis = -quat_rotate(b.rot, plane.xyz);
        let old_separation = face_separation_b;
        if (!test_convex_axis(shape_a, shape_b, a, b, axis, &face_separation_b, &face_normal_b)) {
            return empty_contact();
        }
        if (face_separation_b > old_separation) {
            face_index_b = i;
        }
    }
    if (a.kind == KIND_BOX) {
        for (var i = 0u; i < 3u; i++) {
            if (!test_convex_axis(shape_a, shape_b, a, b, box_axis(a, i), &face_separation_a, &face_normal_a)) {
                return empty_contact();
            }
        }
    }
    if (b.kind == KIND_BOX) {
        for (var i = 0u; i < 3u; i++) {
            if (!test_convex_axis(shape_a, shape_b, a, b, box_axis(b, i), &face_separation_b, &face_normal_b)) {
                return empty_contact();
            }
        }
    }
    // Boxes expose implicit hull topology so mixed box/hull faces get a full manifold.
    let hull_hull = (a.kind == KIND_CONVEX_HULL || a.kind == KIND_BOX)
        && (b.kind == KIND_CONVEX_HULL || b.kind == KIND_BOX)
        && hull_topology_count(shape_a) > 0u && hull_topology_count(shape_b) > 0u;
    var hull_edges: HullEdgeQuery;
    hull_edges.edge_a = EMPTY;
    if (hull_hull) {
        hull_edges = query_hull_edges(shape_a, shape_b, a, b);
        if (hull_edges.separation > SPECULATIVE) { return empty_contact(); }
    }
    if (!hull_hull && a.kind == KIND_CONVEX_HULL && hull_topology_count(shape_a) > 0u) {
        for (var i = 0u; i < hull_topology_count(shape_a); i++) {
            let twin_a = load_hull_topology(shape_a, i).y;
            if (i >= twin_a) {
                continue;
            }
            let edge_a = gauss_hull_edge(shape_a, a, i);
            if (!edge_a.valid) {
                continue;
            }
            if (b.kind == KIND_BOX) {
                for (var j = 0u; j < 12u; j++) {
                    has_edge_axis = true;
                    if (!test_gauss_edge_pair(
                        shape_a, shape_b, a, b, edge_a, gauss_box_edge(b, j),
                        &edge_separation, &edge_normal,
                    )) {
                        return empty_contact();
                    }
                }
            } else if (b.kind == KIND_CAPSULE) {
                has_edge_axis = true;
                if (!test_convex_axis(
                    shape_a, shape_b, a, b, cross(edge_a.e, capsule_axis(b)),
                    &edge_separation, &edge_normal,
                )) {
                    return empty_contact();
                }
            }
        }
    } else if (!hull_hull && b.kind == KIND_CONVEX_HULL && hull_topology_count(shape_b) > 0u) {
        for (var j = 0u; j < hull_topology_count(shape_b); j++) {
            let twin_b = load_hull_topology(shape_b, j).y;
            if (j >= twin_b) {
                continue;
            }
            let edge_b = gauss_hull_edge(shape_b, b, j);
            if (!edge_b.valid) {
                continue;
            }
            if (a.kind == KIND_BOX) {
                for (var i = 0u; i < 12u; i++) {
                    has_edge_axis = true;
                    if (!test_gauss_edge_pair(
                        shape_a, shape_b, a, b, gauss_box_edge(a, i), edge_b,
                        &edge_separation, &edge_normal,
                    )) {
                        return empty_contact();
                    }
                }
            } else if (a.kind == KIND_CAPSULE) {
                has_edge_axis = true;
                if (!test_convex_axis(
                    shape_a, shape_b, a, b, cross(capsule_axis(a), edge_b.e),
                    &edge_separation, &edge_normal,
                )) {
                    return empty_contact();
                }
            }
        }
    }
    var separation = face_separation_a;
    var normal = face_normal_a;
    // Match Box3D hull face selection; an added bias can choose a very different
    // normal for nearly tied penetration depths in overlapping asymmetric hulls.
    let reference_is_b = select(face_separation_b > face_separation_a + 0.5 * LINEAR_SLOP,
        face_separation_b >= face_separation_a, hull_hull);
    if (reference_is_b) {
        separation = face_separation_b;
        normal = face_normal_b;
    }
    let use_edge = has_edge_axis && !hull_hull && edge_separation > separation + 0.005;
    if (use_edge) {
        separation = edge_separation;
        normal = edge_normal;
    }
    if (!use_edge && hull_hull) {
        // SAT orients every face axis toward B-A, so opposite caps share the same
        // separation. Pick the face whose outward normal matches that axis (A's top
        // and B's bottom for a stack), not the first face that tied after flipping.
        let reference_face = select(
            best_hull_face(shape_a, a, normal),
            best_hull_face(shape_b, b, -normal),
            reference_is_b,
        );
        if (reference_face != EMPTY) {
            let clipped = clipped_hull_manifold(
                shape_a, shape_b, a, b, ia, ib, reference_is_b, reference_face,
            );
            var clip_separation = 1e30;
            for (var i = 0u; i < clipped.count; i++) {
                clip_separation = min(clip_separation, ra_at(clipped, i).w
                    + dot(rb_at(clipped, i).xyz - ra_at(clipped, i).xyz, clipped.n));
            }
            if (hull_edges.edge_a != EMPTY && (clipped.count == 0u || hull_edges.separation > clip_separation + LINEAR_SLOP)) {
                let edge_contact = hull_edge_manifold(shape_a, shape_b, a, b, ia, ib, hull_edges);
                if (edge_contact.count > 0u) { return edge_contact; }
            }
            return clipped;
        }
    }
    var c = empty_contact();
    c.a = ia;
    c.b = ib;
    c.count = 1u;
    c.n = normal;
    c.friction = mixed_friction_of(a, b);
    let projection_a = project_collider(shape_a, a, normal);
    let projection_b = project_collider(shape_b, b, normal);
    let midpoint = 0.5 * (a.pos + b.pos);
    let surface = 0.5 * (projection_a.y + projection_b.x);
    let center_point = midpoint + normal * (surface - dot(midpoint, normal));
    let tangent1 = perp(normal);
    let tangent2 = cross(tangent1, normal);
    var candidates: array<vec3<f32>, 4>;
    var scores = vec4<f32>(-1e30);
    var valid = vec4<bool>(false);
    let face_slop = max(0.01, 2.0 * SPECULATIVE);
    if (a.kind == KIND_CONVEX_HULL) {
        for (var i = 0u; i < hull_point_count(shape_a); i++) {
            let local = load_hull_point(shape_a, i) - shape_a.local_center;
            let point = a.pos + quat_rotate(a.rot, local);
            if (dot(point, normal) >= projection_a.y - face_slop
                && point_inside_collider(shape_b, b, point, face_slop)) {
                consider_convex_manifold_point(
                    point,
                    tangent1,
                    tangent2,
                    &candidates,
                    &scores,
                    &valid,
                );
            }
        }
    }
    if (b.kind == KIND_CONVEX_HULL) {
        for (var i = 0u; i < hull_point_count(shape_b); i++) {
            let local = load_hull_point(shape_b, i) - shape_b.local_center;
            let point = b.pos + quat_rotate(b.rot, local);
            if (dot(point, normal) <= projection_b.x + face_slop
                && point_inside_collider(shape_a, a, point, face_slop)) {
                consider_convex_manifold_point(
                    point,
                    tangent1,
                    tangent2,
                    &candidates,
                    &scores,
                    &valid,
                );
            }
        }
    }
    c.count = 0u;
    for (var i = 0u; i < 4u; i++) {
        if (!valid[i]) {
            continue;
        }
        let candidate = candidates[i]
            + normal * (surface - dot(candidates[i], normal));
        var duplicate = false;
        for (var j = 0u; j < c.count; j++) {
            let old = ra_at(c, j).xyz + a.pos;
            let d = candidate - old;
            duplicate = duplicate || dot(d, d) < 1e-8;
        }
        if (duplicate) {
            continue;
        }
        let r_a = candidate - a.pos;
        let r_b = candidate - b.pos;
        let base = separation - dot(r_b - r_a, normal);
        set_point(&c, c.count, vec4<f32>(r_a, base), vec4<f32>(r_b, 0.0));
        c.count = c.count + 1u;
    }
    if (c.count == 0u) {
        let r_a = center_point - a.pos;
        let r_b = center_point - b.pos;
        let base = separation - dot(r_b - r_a, normal);
        set_point(&c, 0u, vec4<f32>(r_a, base), vec4<f32>(r_b, 0.0));
        c.count = 1u;
    }
    finish_manifold(&c, a, b, ia, ib);
    return c;
}

struct TriangleClosest {
    point: vec3<f32>,
    feature: u32,
}

fn closest_triangle_point(p: vec3<f32>, a: vec3<f32>, b: vec3<f32>, c: vec3<f32>) -> TriangleClosest {
    let ab = b - a;
    let ac = c - a;
    let ap = p - a;
    let d1 = dot(ab, ap);
    let d2 = dot(ac, ap);
    if (d1 <= 0.0 && d2 <= 0.0) { return TriangleClosest(a, 4u); }
    let bp = p - b;
    let d3 = dot(ab, bp);
    let d4 = dot(ac, bp);
    if (d3 >= 0.0 && d4 <= d3) { return TriangleClosest(b, 5u); }
    let vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0) {
        return TriangleClosest(a + ab * d1 / (d1 - d3), 0u);
    }
    let cp = p - c;
    let d5 = dot(ab, cp);
    let d6 = dot(ac, cp);
    if (d6 >= 0.0 && d5 <= d6) { return TriangleClosest(c, 6u); }
    let vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0) {
        return TriangleClosest(a + ac * d2 / (d2 - d6), 2u);
    }
    let va = d3 * d6 - d5 * d4;
    if (va <= 0.0 && d4 - d3 >= 0.0 && d5 - d6 >= 0.0) {
        return TriangleClosest(b + (c - b) * (d4 - d3) / ((d4 - d3) + (d5 - d6)), 1u);
    }
    let inv = 1.0 / (va + vb + vc);
    return TriangleClosest(a + ab * vb * inv + ac * vc * inv, 3u);
}

fn mesh_vertex_allowed(vertex: u32, flags: u32) -> bool {
    let mask = (1u << vertex) | (1u << (vertex + 4u));
    return (flags & mask) == 0u;
}

fn mesh_feature_allowed(feature: u32, flags: u32) -> bool {
    if (feature < 3u) {
        return mesh_vertex_allowed(feature, flags);
    }
    if (feature == 4u) { return mesh_vertex_allowed(0u, flags) && mesh_vertex_allowed(2u, flags); }
    if (feature == 5u) { return mesh_vertex_allowed(0u, flags) && mesh_vertex_allowed(1u, flags); }
    if (feature == 6u) { return mesh_vertex_allowed(1u, flags) && mesh_vertex_allowed(2u, flags); }
    return true;
}

fn capsule_mesh_feature_allowed(feature: u32, flags: u32, covered_edges: u32) -> bool {
    // Faces establish coverage before tentative edges are considered. A flat
    // edge alone is not enough to reject an otherwise exposed contact.
    if (feature == 3u) { return true; }
    let flat_edges = flags & (flags >> 4u) & 7u;
    return flat_edges != 7u && (flat_edges & covered_edges) == 0u;
}

fn mesh_local_vertex(mesh_shape: Shape, mesh: Body, index: u32) -> vec3<f32> {
    let local = load_mesh_vertex(mesh_shape, index) - mesh_shape.local_center;
    return mesh.pos + quat_rotate(mesh.rot, local);
}

fn mesh_node_overlaps(mesh_shape: Shape, mesh: Body, convex: Body, node_index: u32) -> bool {
    let lower = load_mesh_node_lower(mesh_shape, node_index).xyz;
    let upper = load_mesh_node_upper(mesh_shape, node_index).xyz;
    let center = mesh_to_raw_point(mesh_shape, quat_inv_rotate(mesh.rot, convex.pos - mesh.pos) + mesh_shape.local_center);
    let radius = bound_radius(convex) + SPECULATIVE + length(convex.half);
    var extent = vec3<f32>(radius);
    if (mesh_shape.instance_flags != 0u) { extent = extent / abs(mesh_shape.axis); }
    return all(center + extent >= lower) && all(center - extent <= upper);
}

fn mesh_speculative_keep(convex: Shape, mesh: Shape) -> f32 {
    // Native's experimental switch applies only to hull/triangle contacts,
    // and either endpoint may disable it. Sphere/capsule speculation is retained.
    let hull = convex.kind == KIND_BOX || convex.kind == KIND_CONVEX_HULL;
    let disabled = ((convex.event_flags | mesh.event_flags) & SHAPE_DISABLE_SPECULATIVE) != 0u
        || (params.diagnostic_flags & SOLVER_DISABLE_SPECULATIVE) != 0u;
    if (hull && disabled) {
        // TOI lands at LINEAR_SLOP with a quarter-slop acceptance tolerance.
        // Its initial-contact fallback relies on a discrete support manifold;
        // removing that narrow shell lets the next fast step tunnel to the
        // centroid fallback. Retain CCD support, independent of the wider
        // experimental speculative range. Resting gap controls still use zero.
        let fast = (load_body(convex.body_index).flags | load_body(mesh.body_index).flags) & FLAG_FAST;
        if (params.enable_continuous != 0u && fast != 0u) {
            return 1.25 * LINEAR_SLOP;
        }
        return 0.0;
    }
    return SPECULATIVE;
}

fn mesh_push_child(
    stack: ptr<function, array<u32, 64>>,
    stack_count: ptr<function, u32>,
    mesh_shape: Shape,
    mesh: Body,
    convex: Body,
    node_index: u32,
) {
    if (!mesh_node_overlaps(mesh_shape, mesh, convex, node_index)) {
        return;
    }
    if (*stack_count < 64u) {
        (*stack)[*stack_count] = node_index;
        *stack_count = *stack_count + 1u;
        return;
    }
    record_contact_drop(12u);
}

// Bounded persistent-manifold reduction: maximize tangent-plane coverage
// with native separation hysteresis. A later triangle may replace an old point;
// visitation order must not lock the manifold to its first four vertices.
fn append_mesh_point(result: ptr<function, Contact>, ra: vec4<f32>, rb: vec4<f32>, n: vec3<f32>) -> bool {
    return append_mesh_point_with_identity(result, ra, rb, n, 0u, 0u);
}

fn append_mesh_point_with_identity(result: ptr<function, Contact>, ra: vec4<f32>, rb: vec4<f32>, n: vec3<f32>, triangle: u32, feature: u32) -> bool {
    if ((*result).count < 4u) {
        set_point(result, (*result).count, ra, rb);
        set_feat_at(result, (*result).count, feature);
        (*result).point_triangles[(*result).count] = triangle;
        (*result).count += 1u;
        return true;
    }
    var aa: array<vec4<f32>, 5>;
    var bb: array<vec4<f32>, 5>;
    var pp: array<vec3<f32>, 5>;
    var triangles: array<u32, 5>;
    var features: array<u32, 5>;
    var selected: array<bool, 5>;
    var deepest = 0u;
    var depth = 1e30;
    for (var i = 0u; i < 5u; i++) {
        if (i < 4u) { aa[i] = ra_at(*result, i); bb[i] = rb_at(*result, i); }
        else { aa[i] = ra; bb[i] = rb; }
        if (i < 4u) { triangles[i] = (*result).point_triangles[i]; features[i] = feat_at(*result, i); }
        else { triangles[i] = triangle; features[i] = feature; }
        let separation = aa[i].w + dot(bb[i].xyz - aa[i].xyz, n);
        if (separation < depth) { depth = separation; deepest = i; }
        pp[i] = aa[i].xyz - dot(aa[i].xyz, n) * n;
        selected[i] = false;
    }
    // Match native's coverage/separation priorities on this bounded candidate
    // set. Deepest-first greedily loses a second penetrating edge witness.
    var order: array<u32, 4>;
    var selected_count = 0u;
    let tolerance2 = 0.0625 * LINEAR_SLOP * LINEAR_SLOP;
    var first = 0u;
    var second = 1u;
    var score = 0.0;
    var best_depth = 1e30;
    for (var i = 0u; i < 5u; i++) {
        for (var j = i + 1u; j < 5u; j++) {
            let delta = pp[i] - pp[j];
            let candidate = dot(delta, delta);
            let separation = aa[i].w + dot(bb[i].xyz-aa[i].xyz,n)
                + aa[j].w + dot(bb[j].xyz-aa[j].xyz,n);
            if (candidate > score + tolerance2 ||
                (candidate >= score - tolerance2 && separation < best_depth - LINEAR_SLOP)) {
                first = i; second = j; score = candidate; best_depth = separation;
            }
        }
    }
    if (score < tolerance2) {
        selected[deepest] = true;
        order[0] = deepest;
        selected_count = 1u;
    } else {
        selected[first] = true;
        selected[second] = true;
        order[0] = first; order[1] = second;
        selected_count = 2u;
        var third = EMPTY;
        score = 0.0;
        best_depth = 1e30;
        let edge = pp[second] - pp[first];
        for (var i = 0u; i < 5u; i++) {
            if (selected[i]) { continue; }
            let candidate = abs(dot(cross(edge, pp[i]-pp[first]), n));
            let separation = aa[i].w + dot(bb[i].xyz-aa[i].xyz,n);
            if (candidate > score + tolerance2 ||
                (candidate >= score - tolerance2 && separation < best_depth - LINEAR_SLOP)) {
                third = i; score = candidate; best_depth = separation;
            }
        }
        if (third != EMPTY) {
            selected[third] = true;
            order[2] = third; selected_count = 3u;
            let a = pp[first];
            var b = pp[second];
            var c = pp[third];
            if (dot(cross(b-a,c-a), n) < 0.0) { let temp = b; b = c; c = temp; }
            var fourth = EMPTY;
            score = 0.0;
            best_depth = 1e30;
            for (var i = 0u; i < 5u; i++) {
                if (selected[i]) { continue; }
                let candidate = max(dot(cross(pp[i]-a,b-a),n),
                    max(dot(cross(pp[i]-b,c-b),n),dot(cross(pp[i]-c,a-c),n)));
                let separation = aa[i].w + dot(bb[i].xyz-aa[i].xyz,n);
                if (candidate > score + tolerance2 ||
                    (candidate >= score - tolerance2 && separation < best_depth - LINEAR_SLOP)) {
                    fourth = i; score = candidate; best_depth = separation;
                }
            }
            if (fourth != EMPTY) { selected[fourth] = true; order[3] = fourth; selected_count = 4u; }
        }
    }
    // Coverage must not erase a deeper interior penetration. Preserve the
    // existing deepest-witness safety invariant even for nonplanar candidates.
    if (!selected[deepest]) {
        var replace = 0u;
        var shallowest = -1e30;
        for (var i = 0u; i < selected_count; i++) {
            let j = order[i];
            let separation = aa[j].w + dot(bb[j].xyz-aa[j].xyz,n);
            if (separation > shallowest) { replace = i; shallowest = separation; }
        }
        selected[order[replace]] = false;
        order[replace] = deepest;
        selected[deepest] = true;
    }
    for (var out = 0u; out < selected_count; out++) {
        let i = order[out];
        set_point(result, out, aa[i], bb[i]);
        set_feat_at(result, out, features[i]);
        (*result).point_triangles[out] = triangles[i];
    }
    (*result).count = selected_count;
    return selected[4];
}

// Diagnostic-only bounded append buffer, allocated only with mesh-candidates.
// Records world-space points before reduction; overflow invalidates the run.
fn trace_solver_words(words: array<u32,16>) {
    if ((params.diagnostic_flags & DIAG_MESH_CANDIDATES) == 0u) { return; }
    let start = arrayLength(&atom) - MESH_TRACE_WORDS;
    let index = atomicAdd(&atom[start], 1u);
    if (index >= 2048u) {
        record_contact_drop(13u);
        return;
    }
    let base = start + 1u + 16u * index;
    for (var i=0u; i<16u; i++) { atomicStore(&atom[base+i],words[i]); }
}

fn trace_solver_values(stage: u32, a: u32, b: u32, x: vec4<f32>, y: vec4<f32>, z: vec4<f32>) {
    trace_solver_words(array<u32,16>(params.physics_step,stage,a,b,
        bitcast<u32>(x.x),bitcast<u32>(x.y),bitcast<u32>(x.z),bitcast<u32>(x.w),
        bitcast<u32>(y.x),bitcast<u32>(y.y),bitcast<u32>(y.z),bitcast<u32>(y.w),
        bitcast<u32>(z.x),bitcast<u32>(z.y),bitcast<u32>(z.z),bitcast<u32>(z.w)));
}

fn trace_mesh_candidate(stage: u32, mesh: Shape, convex: Shape, triangle: u32,
    point: vec3<f32>, separation: f32, normal: vec3<f32>, patch_depth: f32, feature: u32) {
    if ((params.diagnostic_flags & DIAG_MESH_CANDIDATES) == 0u) { return; }
    let words = array<u32,16>(params.physics_step, stage, mesh.body_index, convex.body_index,
        bitcast<u32>(point.x), bitcast<u32>(point.y), bitcast<u32>(point.z), bitcast<u32>(separation),
        bitcast<u32>(normal.x), bitcast<u32>(normal.y), bitcast<u32>(normal.z), bitcast<u32>(patch_depth),
        triangle, feature, 0u, 0u);
    trace_solver_words(words);
}

fn mesh_contact_base_separation(
    separation: f32, reference_normal: vec3<f32>,
    anchor_a: vec3<f32>, anchor_b: vec3<f32>,
) -> f32 {
    // Native clustering preserves each original separation. Do not project it
    // again onto the cluster normal, whose squared length may differ from one.
    // Keep scalar dot order: a one-ulp base error changes speculative bias.
    return separation - gyro_dot3(anchor_b - anchor_a, reference_normal);
}

fn consider_mesh_manifold_point(
    result: ptr<function, Contact>,
    mesh: Body,
    convex: Body,
    convex_shape: Shape,
    mesh_shape: Shape,
    input_point: vec3<f32>,
    separation: f32,
    patch_separation: f32,
    contact_normal: vec3<f32>,
    triangle_normal: vec3<f32>,
    material_index: u32,
    feature: u32,
    best_separation: ptr<function, f32>,
    best_normal: ptr<function, vec3<f32>>,
    material_samples: ptr<function, f32>,
    mixed_friction: ptr<function, f32>,
    mixed_restitution: ptr<function, f32>,
    mesh_tangent_velocity: ptr<function, vec3<f32>>,
    dominant_material: ptr<function, u32>,
) {
    var point = input_point;
    var r_a = point - mesh.pos;
    var r_b = point - convex.pos;
    if (convex.kind == KIND_CAPSULE) {
        // Capsule witnesses arrive rotated but relative to the body origin.
        // Native mesh_contact.c constructs anchors before adding world position.
        let body_a = load_body((*result).a);
        let body_b = load_body((*result).b);
        let center_a = quat_rotate(body_a.rot, load_body_cold((*result).a).local_center);
        let center_b = quat_rotate(body_b.rot, load_body_cold((*result).b).local_center);
        let origin_a = body_origin(body_a,(*result).a);
        let origin_b = body_origin(body_b,(*result).b);
        r_b = input_point - center_b;
        r_a = (input_point + (origin_b - origin_a)) - center_a;
        point = origin_b + input_point;
        // COM anchors with the existing packed separation representation.
        (*result).cached_relative.w = 4.0;
    }
    trace_mesh_candidate(0u, mesh_shape, convex_shape, (*result).manifold_link.w,
        point, separation, contact_normal, patch_separation, feature);
    // Capsule admission already used closest distance, as in native
    // triangle_manifold.c. Its clipped face can have every point farther than
    // speculative distance: applying a second face cutoff loses that manifold.
    let reject_separation = convex.kind != KIND_CAPSULE &&
        (patch_separation > mesh_speculative_keep(convex_shape, mesh_shape)
        || (mesh_speculative_keep(convex_shape, mesh_shape) == 0.0 && separation > 0.0));
    if (reject_separation || dot(contact_normal, triangle_normal) < -0.05) {
        return;
    }
    // Box3D mesh_contact.c classifies shallow hull-face contacts whose normal
    // differs substantially from the triangle as tentative. Prefer confirmed
    // surface/deep contacts over those edge candidates, regardless of BVH order.
    // _pad_end is temporary classification here; finish_manifold overwrites it
    // with restitution. Replace the WHOLE tentative piece, never its basis alone.
    let confirmed = dot(contact_normal, triangle_normal) > 0.5 || patch_separation < -2.0 * LINEAR_SLOP;
    if ((*result).count > 0u && (*result)._pad_end == 1.0 && !confirmed) {
        return;
    }
    if ((*result).count > 0u && (*result)._pad_end == 0.0 && confirmed) {
        (*result).count = 0u;
        for (var i = 0u; i < 4u; i++) { set_point(result, i, vec4<f32>(0.0), vec4<f32>(0.0)); }
        *best_separation = 1e30;
        *material_samples = 0.0;
        *mixed_friction = 0.0;
        *mixed_restitution = 0.0;
        *mesh_tangent_velocity = vec3<f32>(0.0);
    }
    // Never let a rejected triangle replace the basis of already packed points.
    // In particular a chamfer/wall visited after four floor points must not turn
    // those points into a wall manifold with floor-relative separation values.
    let aligned = (*result).count == 0u || dot(contact_normal, *best_normal) > 0.8;
    if (!aligned) {
        return;
    }
    if (convex.kind != KIND_CAPSULE) {
        for (var old = 0u; old < (*result).count; old++) {
            let d = ra_at(*result, old).xyz - r_a;
            if (dot(d, d) < 1e-8) { return; }
        }
    }
    if ((*result).count == 0u) {
        (*result)._pad_end = select(0.0, 1.0, confirmed);
        *best_normal = contact_normal;
    }
    if (separation < (*best_separation)) {
        *best_separation = separation;
        *dominant_material = material_index;
    }
    // All points in this manifold must use its single, fixed normal basis.
    let base = mesh_contact_base_separation(separation, *best_normal, r_a, r_b);
    trace_mesh_candidate(1u, mesh_shape, convex_shape, (*result).manifold_link.w,
        point, separation, contact_normal, patch_separation, feature);
    // Before finalization rb.w carries raw separation, not a solver impulse.
    // The reducer copies the whole point, preserving it through reordering.
    if (!append_mesh_point_with_identity(result, vec4<f32>(r_a, base), vec4<f32>(r_b, separation), *best_normal, (*result).manifold_link.w, feature)) {
        return;
    }
    let material = load_surface_material(mesh_shape, material_index);
    *material_samples = *material_samples + 1.0;
    *mixed_friction = *mixed_friction + sqrt(max(material.friction * convex_shape.friction, 0.0));
    *mixed_restitution = *mixed_restitution + max(material.restitution, convex_shape.restitution);
    *mesh_tangent_velocity = *mesh_tangent_velocity + material.tangent_velocity;
}

fn add_mesh_sample_contact(
    result: ptr<function, Contact>,
    mesh: Body,
    convex: Body,
    convex_shape: Shape,
    mesh_shape: Shape,
    sample: vec3<f32>,
    radius: f32,
    p1: vec3<f32>,
    p2: vec3<f32>,
    p3: vec3<f32>,
    triangle_normal: vec3<f32>,
    triangle_flags: u32,
    material_index: u32,
    best_separation: ptr<function, f32>,
    best_normal: ptr<function, vec3<f32>>,
    material_samples: ptr<function, f32>,
    mixed_friction: ptr<function, f32>,
    mixed_restitution: ptr<function, f32>,
    mesh_tangent_velocity: ptr<function, vec3<f32>>,
    dominant_material: ptr<function, u32>,
    sample_feature: u32,
) {
    var closest = closest_triangle_point(sample, p1, p2, p3);
    if (!mesh_feature_allowed(closest.feature, triangle_flags)) {
        let proj = sample - triangle_normal * dot(sample - p1, triangle_normal);
        closest = closest_triangle_point(proj, p1, p2, p3);
        if (closest.feature != 3u) {
            // An exact seam/vertex projection still has face support. Reject
            // outside projections, but do not discard both coplanar triangles
            // under a sphere/capsule solely because their shared edge is flat.
            let lateral = proj - closest.point;
            if (radius <= 0.0 || dot(lateral, lateral) > 1e-12) { return; }
            closest.feature = 3u;
        }
    }
    let delta = sample - closest.point;
    let distance = length(delta);
    var separation = distance - radius;
    var contact_normal = select(triangle_normal, delta / distance, distance > 1e-8);
    if (closest.feature == 3u) {
        separation = dot(delta, triangle_normal) - radius;
        contact_normal = triangle_normal;
    }
    // Hull samples and clipped hull faces must use the same surface witness.
    // Midpoints create a second representation of a penetrating hull vertex,
    // consuming manifold slots and changing its friction lever arm.
    let point = select(sample, 0.5 * (closest.point + sample - radius * contact_normal), radius > 0.0);
    consider_mesh_manifold_point(
        result, mesh, convex, convex_shape, mesh_shape, point, separation, separation, contact_normal,
        triangle_normal, material_index, pack_feature(0u, closest.feature, 1u, sample_feature), best_separation, best_normal, material_samples,
        mixed_friction, mixed_restitution, mesh_tangent_velocity, dominant_material,
    );
}

fn box_triangle_face_axis(box: Body, p1: vec3<f32>, p2: vec3<f32>, p3: vec3<f32>, normal: vec3<f32>) -> vec4<f32> {
    let axes = mat3x3<f32>(box_axis(box, 0u), box_axis(box, 1u), box_axis(box, 2u));
    let radius = dot(abs(transpose(axes) * normal), box.half);
    var best = vec4<f32>(normal, dot(box.pos - p1, normal) - radius);
    // Triangle face and OBB faces are the face-axis SAT candidates. Reject
    // separation on either side, but do not choose a back-facing response.
    for (var i = 0u; i < 3u; i++) {
        let axis = axes[i];
        let a = dot(p1 - box.pos, axis);
        let b = dot(p2 - box.pos, axis);
        let c = dot(p3 - box.pos, axis);
        let positive = -box.half[i] - max(a, max(b, c));
        let negative = min(a, min(b, c)) - box.half[i];
        let direction = select(axis, -axis, negative > positive);
        let separation = max(positive, negative);
        if (separation > SPECULATIVE) { return vec4<f32>(direction, separation); }
        if (separation > best.w && dot(direction, normal) > -0.25) {
            best = vec4<f32>(direction, separation);
        }
    }
    // Edge cross axes are required for rejection even when a face supplies
    // the manifold normal. Without these, disjoint edge pairs can collide.
    let edges = mat3x3<f32>(p2 - p1, p3 - p2, p1 - p3);
    for (var i = 0u; i < 3u; i++) {
        for (var j = 0u; j < 3u; j++) {
            let cross_axis = cross(axes[i], edges[j]);
            let length2 = dot(cross_axis, cross_axis);
            if (length2 < 1e-12) { continue; }
            let axis = cross_axis * inverseSqrt(length2);
            let extent = dot(abs(transpose(axes) * axis), box.half);
            let a = dot(p1 - box.pos, axis);
            let b = dot(p2 - box.pos, axis);
            let c = dot(p3 - box.pos, axis);
            let separation = max(-extent - max(a, max(b, c)), min(a, min(b, c)) - extent);
            if (separation > SPECULATIVE) { return vec4<f32>(axis, separation); }
        }
    }
    return best;
}

fn clip_hull_face_to_triangle(
    result: ptr<function, Contact>,
    mesh: Body,
    convex: Body,
    convex_shape: Shape,
    mesh_shape: Shape,
    p1: vec3<f32>,
    p2: vec3<f32>,
    p3: vec3<f32>,
    triangle_normal: vec3<f32>,
    material_index: u32,
    best_separation: ptr<function, f32>,
    best_normal: ptr<function, vec3<f32>>,
    material_samples: ptr<function, f32>,
    mixed_friction: ptr<function, f32>,
    mixed_restitution: ptr<function, f32>,
    mesh_tangent_velocity: ptr<function, vec3<f32>>,
    dominant_material: ptr<function, u32>,
) {
    var polygon: ClipPolygon;
    polygon.count = 0u;
    var reference_normal = triangle_normal;
    if (convex.kind == KIND_BOX) {
        let face = box_triangle_face_axis(convex, p1, p2, p3, triangle_normal);
        if (face.w > mesh_speculative_keep(convex_shape, mesh_shape)) { return; }
        reference_normal = face.xyz;
        // A triangle can lie entirely inside the incident face: no box corner
        // then projects inside it. Clip the face itself, as for a general hull.
        let local_n = quat_inv_rotate(convex.rot, reference_normal);
        var axis = 0u;
        if (abs(local_n.y) > abs(local_n.x)) { axis = 1u; }
        if (abs(local_n.z) > abs(local_n[axis])) { axis = 2u; }
        let u = (axis + 1u) % 3u;
        let v = (axis + 2u) % 3u;
        if (abs(local_n[axis]) > 0.99999) {
            // The box supplies the reference face: clip the TRIANGLE to its
            // side planes. Projecting the box face onto the triangle loses
            // edge contacts when the two planes are nearly perpendicular.
            polygon.count = 3u;
            polygon.points[0] = p1;
            polygon.points[1] = p2;
            polygon.points[2] = p3;
            polygon.features[0] = pack_feature(0u, 2u, 0u, 0u);
            polygon.features[1] = pack_feature(0u, 0u, 0u, 1u);
            polygon.features[2] = pack_feature(0u, 1u, 0u, 2u);
            let face = find_incident_face(convex, reference_normal, box_support_vertex(convex, -reference_normal));
            let edges = hull_face_edge_ids(face);
            for (var e = 0u; e < 4u; e++) {
                let edge = edges[e];
                let v1 = convex.pos + quat_rotate(convex.rot, box_point_local(convex, BOX_EDGE_ORIGIN[edge]));
                let v2 = convex.pos + quat_rotate(convex.rot, box_point_local(convex, BOX_EDGE_ORIGIN[BOX_EDGE_NEXT[edge]]));
                let side = normalize(cross(v2 - v1, -reference_normal));
                polygon = clip_polygon_with_features(polygon, side, dot(side, v1), edge, 1u);
            }
            var patch_separation = 1e30;
            for (var i = 0u; i < polygon.count; i++) {
                patch_separation = min(patch_separation,
                    dot(convex.pos - polygon.points[i], reference_normal) - convex.half[axis]);
            }
            for (var i = 0u; i < polygon.count; i++) {
                let separation = dot(convex.pos - polygon.points[i], reference_normal) - convex.half[axis];
                // Box3D projects triangle vertices onto the reference hull face.
                let point = polygon.points[i] + separation * reference_normal;
                consider_mesh_manifold_point(
                    result, mesh, convex, convex_shape, mesh_shape, point, separation, patch_separation, reference_normal,
                    triangle_normal, material_index, polygon.features[i], best_separation, best_normal, material_samples,
                    mixed_friction, mixed_restitution, mesh_tangent_velocity, dominant_material,
                );
            }
            return;
        }
        let incident_face = find_incident_face(convex, reference_normal, box_support_vertex(convex, -reference_normal));
        let verts = hull_face_verts(convex, incident_face);
        let edges = hull_face_edge_ids(incident_face);
        for (var i = 0u; i < 4u; i++) {
            polygon.points[i] = verts[i];
            polygon.features[i] = pack_feature(1u, edges[i], 1u, edges[(i + 1u) % 4u]);
        }
        polygon.count = 4u;
    } else {
    var incident_face = 0u;
    var incident_alignment = 1e30;
    for (var i = 0u; i < hull_plane_count(convex_shape); i++) {
        let candidate = quat_rotate(convex.rot, load_hull_plane(convex_shape, i).xyz);
        let alignment = dot(candidate, triangle_normal);
        if (alignment < incident_alignment) {
            incident_alignment = alignment;
            incident_face = i;
        }
    }
    let incident_start = find_face_edge(convex_shape, incident_face);
    if (incident_start == EMPTY) {
        return;
    }
    var previous_edge = incident_start;
    for (var i = 0u; i < 32u; i++) {
        let next = load_hull_topology(convex_shape, previous_edge).x;
        if (next == incident_start) { break; }
        previous_edge = next;
    }
    var edge = incident_start;
    for (var i = 0u; i < 32u; i++) {
        let topology = load_hull_topology(convex_shape, edge);
        let local = load_hull_point(convex_shape, topology.z) - convex_shape.local_center;
        polygon.points[polygon.count] = convex.pos + quat_rotate(convex.rot, local);
        polygon.features[polygon.count] = pack_feature(1u, previous_edge, 1u, edge);
        previous_edge = edge;
        polygon.count = polygon.count + 1u;
        edge = topology.x;
        if (edge == incident_start || polygon.count == 32u) {
            break;
        }
    }
    }
    var side12 = normalize(cross(p2 - p1, reference_normal));
    if (dot(p3 - p1, side12) > 0.0) {
        side12 = -side12;
    }
    polygon = clip_polygon_with_features(polygon, side12, dot(side12, p1), 0u, 0u);
    var side23 = normalize(cross(p3 - p2, reference_normal));
    if (dot(p1 - p2, side23) > 0.0) {
        side23 = -side23;
    }
    polygon = clip_polygon_with_features(polygon, side23, dot(side23, p2), 1u, 0u);
    var side31 = normalize(cross(p1 - p3, reference_normal));
    if (dot(p2 - p3, side31) > 0.0) {
        side31 = -side31;
    }
    polygon = clip_polygon_with_features(polygon, side31, dot(side31, p3), 2u, 0u);
    var patch_separation = 1e30;
    for (var i = 0u; i < polygon.count; i++) {
        patch_separation = min(patch_separation,
            dot(polygon.points[i] - p1, triangle_normal) / dot(reference_normal, triangle_normal));
    }
    for (var i = 0u; i < polygon.count; i++) {
        let separation = dot(polygon.points[i] - p1, triangle_normal) / dot(reference_normal, triangle_normal);
        // Triangle-reference contacts keep the incident hull point. Moving
        // it along the normal leaves normal torque unchanged but alters friction.
        let point = polygon.points[i];
        consider_mesh_manifold_point(
            result, mesh, convex, convex_shape, mesh_shape, point, separation, patch_separation, reference_normal,
            triangle_normal, material_index, polygon.features[i], best_separation, best_normal, material_samples,
            mixed_friction, mixed_restitution, mesh_tangent_velocity, dominant_material,
        );
    }
}

// Newly generated patches occupy disjoint free slots until every cache transfer
// is complete. The pair's original slot remains the graph/hash owner.
struct MeshPatchList { head: u32, tail: u32, count: u32 }

// Scratch patches share slots with persistent roots. Preserve the slot's root
// generation even while staging, publishing or discarding a child. Resetting
// it can resurrect an unread public handle when the same pair reuses the slot.
fn store_mesh_contact(slot: u32, contact: Contact) {
    var preserved = contact;
    preserved.lifecycle.x = contact_persistent[slot].lifecycle.x;
    store_contact(slot, preserved);
}

fn mesh_representative_normal(shape: Shape, mesh: Body, triangle_plus_one: u32) -> vec3<f32> {
    let t = load_mesh_triangle(shape, triangle_plus_one - 1u);
    let p1 = mesh_local_vertex(shape, mesh, t.x);
    let p2 = mesh_local_vertex(shape, mesh, t.y);
    let p3 = mesh_local_vertex(shape, mesh, t.z);
    return normalize(cross(p2 - p1, p3 - p1));
}

fn append_mesh_patch(list: ptr<function, MeshPatchList>, root: u32, candidate: Contact, shape: Shape, mesh: Body) -> bool {
    if (candidate.a == EMPTY || candidate.count == 0u) { return true; }
    let triangle_normal = mesh_representative_normal(shape, mesh, candidate.manifold_link.w);
    var slot = (*list).head;
    for (var i = 0u; i < (*list).count; i++) {
        var piece = load_contact(slot);
        if (piece.lifecycle.y == candidate.lifecycle.y && dot(piece.n, candidate.n) > 0.996
            && dot(mesh_representative_normal(shape, mesh, piece.manifold_link.w), triangle_normal) > 0.996) {
            if (candidate.cached_relative.w == 4.0) {
                // Keep the complete capsule cluster until native-style culling.
                // Different triangles may contribute coincident witnesses.
                let allocated = allocate_manifold_slot(root);
                if (allocated == EMPTY) { return false; }
                var raw = candidate;
                raw.pair.z = 0u;
                raw.pair.w = 0u;
                store_mesh_contact(allocated, raw);
                if (piece.pair.z == 0u) { piece.pair.z = allocated + 1u; }
                else { contact_persistent[piece.pair.w - 1u].pair.z = allocated + 1u; }
                piece.pair.w = allocated + 1u;
                piece._pad_end = max(piece._pad_end, candidate._pad_end);
                piece.lifecycle.z = max(piece.lifecycle.z, candidate.lifecycle.z);
                store_mesh_contact(slot, piece);
                return true;
            }
            for (var j = 0u; j < candidate.count; j++) {
                let ra = ra_at(candidate, j);
                let rb = rb_at(candidate, j);
                var duplicate = false;
                for (var k = 0u; k < piece.count; k++) {
                    let delta = ra_at(piece, k).xyz - ra.xyz;
                    duplicate = duplicate || dot(delta, delta) < 1e-8;
                }
                if (duplicate) { continue; }
                let separation = rb.w;
                let base = mesh_contact_base_separation(separation, piece.n, ra.xyz, rb.xyz);
                append_mesh_point_with_identity(&piece, vec4<f32>(ra.xyz, base), rb, piece.n,
                    candidate.point_triangles[j], feat_at(candidate, j));
            }
            piece._pad_end = max(piece._pad_end, candidate._pad_end);
            piece.lifecycle.z = max(piece.lifecycle.z, candidate.lifecycle.z);
            store_mesh_contact(slot, piece);
            return true;
        }
        slot = piece.manifold_link.x - 1u;
    }
    let allocated = allocate_manifold_slot(root);
    if (allocated == EMPTY) { return false; }
    var added = candidate;
    added.pair.z = 0u;
    added.pair.w = 0u;
    added.manifold_link.x = 0u;
    added.manifold_link.y = root + 1u;
    added.manifold_link.z = 0u;
    store_mesh_contact(allocated, added);
    if ((*list).count == 0u) { (*list).head = allocated; }
    else { contacts[(*list).tail].manifold_link.x = allocated + 1u; }
    (*list).tail = allocated;
    (*list).count += 1u;
    return true;
}

// Fresh capsule clusters use pair.z as a raw-block link and pair.w as the
// tail. These scratch links are cleared before a manifold becomes persistent.
fn discard_mesh_raw_points(root: u32) {
    var next = contact_persistent[root].pair.z;
    loop {
        if (next == 0u) { break; }
        let slot = next - 1u;
        next = contact_persistent[slot].pair.z;
        store_mesh_contact(slot, empty_contact());
        scratch[scr_contact_mark() + slot] = 0u;
    }
    contact_persistent[root].pair.z = 0u;
    contact_persistent[root].pair.w = 0u;
}

fn mesh_raw_point(root: u32, index: u32) -> vec2<u32> {
    var slot = root;
    var remaining = index;
    loop {
        let count = contacts[slot].count;
        if (remaining < count) { break; }
        remaining -= count;
        slot = contact_persistent[slot].pair.z - 1u;
    }
    return vec2<u32>(slot, remaining);
}

fn mesh_project_raw_point(root: u32, index: u32, origin: vec3<f32>, u: vec3<f32>, v: vec3<f32>) -> vec3<f32> {
    let location = mesh_raw_point(root, index);
    let point = persistent_ra_at(load_contact(location.x), location.y);
    let d = point.xyz - origin;
    return vec3<f32>(gyro_dot3(d,u), gyro_dot3(d,v), point.w);
}

fn mesh_cull_better(score: f32, separation: f32, best_score: f32, best_separation: f32) -> bool {
    let tolerance = 0.25 * LINEAR_SLOP;
    let tolerance2 = tolerance * tolerance;
    if (score > best_score + tolerance2) { return true; }
    if (score < best_score - tolerance2) { return false; }
    return separation < best_separation - LINEAR_SLOP;
}

fn mesh_cull_index(index: u32, swaps: array<vec2<u32>,3>, count: u32) -> u32 {
    for (var i = count; i > 0u; i--) {
        if (swaps[i-1u].x == index) { return swaps[i-1u].y; }
    }
    return index;
}

fn mesh_cross2(a: vec2<f32>, b: vec2<f32>) -> f32 {
    let x = a.x * b.y;
    let y = a.y * b.x;
    return x - y;
}

fn reduce_capsule_mesh_cluster(root: u32) -> Contact {
    var result = load_contact(root);
    var count = result.count;
    var next = result.pair.z;
    loop {
        if (next == 0u) { break; }
        count += contacts[next-1u].count;
        next = contact_persistent[next-1u].pair.z;
    }
    if (count <= 1u) { return result; }
    let normal = result.cached_rotation_a.xyz;
    var perpendicular = vec3<f32>(0.0,normal.z,-normal.y);
    if (normal.x < -0.5 || normal.x > 0.5) { perpendicular = vec3<f32>(normal.y,-normal.x,0.0); }
    let u = gyro_norm3(perpendicular);
    let v = gyro_cross(normal,u);
    let origin = result.persistent_ra0.xyz;
    var first = 0u;
    var second = 1u;
    var score = 0.0;
    var separation = bitcast<f32>(0x7f7fffffu);
    for (var i=0u; i<count; i++) {
        let p = mesh_project_raw_point(root,i,origin,u,v);
        for (var j=i+1u; j<count; j++) {
            let q = mesh_project_raw_point(root,j,origin,u,v);
            let d = p.xy-q.xy;
            let s = d.x*d.x + d.y*d.y;
            if (mesh_cull_better(s,p.z+q.z,score,separation)) {
                first=i; second=j; score=s; separation=p.z+q.z;
            }
        }
    }
    var order: array<u32,4>;
    var output_count = 2u;
    order[0]=first; order[1]=second;
    let tolerance = 0.25 * LINEAR_SLOP;
    if (score < tolerance*tolerance) {
        var deepest=0u;
        var depth=mesh_project_raw_point(root,0u,origin,u,v).z;
        for (var i=1u; i<count; i++) {
            let d=mesh_project_raw_point(root,i,origin,u,v).z;
            if (d<depth) { deepest=i; depth=d; }
        }
        order[0]=deepest; output_count=1u;
    } else if (count>2u) {
        // Model native swap-removal without imposing a fixed candidate limit.
        var swaps: array<vec2<u32>,3>;
        swaps[0]=vec2<u32>(second,count-1u);
        swaps[1]=vec2<u32>(first,mesh_cull_index(count-2u,swaps,1u));
        var remaining=count-2u;
        let a=mesh_project_raw_point(root,first,origin,u,v).xy;
        var b=mesh_project_raw_point(root,second,origin,u,v).xy;
        var third=EMPTY;
        var signed_area=0.0;
        score=0.0; separation=bitcast<f32>(0x7f7fffffu);
        for (var i=0u; i<remaining; i++) {
            let p=mesh_project_raw_point(root,mesh_cull_index(i,swaps,2u),origin,u,v);
            let area=mesh_cross2(b-a,p.xy-a);
            if (mesh_cull_better(abs(area),p.z,score,separation)) {
                third=i; signed_area=area; score=abs(area); separation=p.z;
            }
        }
        if (third != EMPTY) {
            order[2]=mesh_cull_index(third,swaps,2u); output_count=3u;
            if (remaining>1u) {
                swaps[2]=vec2<u32>(third,mesh_cull_index(remaining-1u,swaps,2u));
                remaining-=1u;
                var c=mesh_project_raw_point(root,order[2],origin,u,v).xy;
                if (signed_area<0.0) { let temp=b; b=c; c=temp; }
                score=0.0; separation=bitcast<f32>(0x7f7fffffu);
                var fourth=EMPTY;
                for (var i=0u; i<remaining; i++) {
                    let p=mesh_project_raw_point(root,mesh_cull_index(i,swaps,3u),origin,u,v);
                    let s=max(mesh_cross2(p.xy-a,b-a),max(mesh_cross2(p.xy-b,c-b),mesh_cross2(p.xy-c,a-c)));
                    if (mesh_cull_better(s,p.z,score,separation)) { fourth=i; score=s; separation=p.z; }
                }
                if (fourth != EMPTY) { order[3]=mesh_cull_index(fourth,swaps,3u); output_count=4u; }
            }
        }
    }
    for (var i=0u; i<output_count; i++) {
        let location=mesh_raw_point(root,order[i]);
        let source=load_contact(location.x);
        let ra=ra_at(source,location.y);
        let rb=rb_at(source,location.y);
        let base=mesh_contact_base_separation(rb.w,result.n,ra.xyz,rb.xyz);
        set_point(&result,i,vec4<f32>(ra.xyz,base),rb);
        set_feat_at(&result,i,feat_at(source,location.y));
        result.point_triangles[i]=source.point_triangles[location.y];
    }
    result.count=output_count;
    return result;
}

fn discard_mesh_patch_list(list: MeshPatchList) {
    var slot = list.head;
    for (var i = 0u; i < list.count; i++) {
        let next = contacts[slot].manifold_link.x;
        discard_mesh_raw_points(slot);
        store_mesh_contact(slot, empty_contact());
        scratch[scr_contact_mark() + slot] = 0u;
        slot = next - 1u;
    }
}

fn select_old_mesh_patch(root: u32, fresh: Contact) -> Contact {
    let count = max(contacts[root].manifold_link.z, 1u);
    var slot = root;
    var best = EMPTY;
    var best_dot = 0.995;
    for (var i = 0u; i < count; i++) {
        let old = load_contact(slot);
        if (old.a == fresh.a && old.b == fresh.b && old.count > 0u
            && (scratch[scr_contact_mark() + slot] & 2u) == 0u) {
            let alignment = dot(old.n, fresh.n);
            if (alignment > best_dot) { best = slot; best_dot = alignment; }
        }
        slot = old.manifold_link.x - 1u;
    }
    if (best == EMPTY) { return empty_contact(); }
    scratch[scr_contact_mark() + best] |= 2u;
    return load_contact(best);
}

// Native mesh_contact.c accepts triangle faces before tentative capsule
// edges/vertices. Keep that stable order even when the BVH visits an edge first.
fn order_capsule_mesh_patches(input: MeshPatchList) -> MeshPatchList {
    var parts = array<MeshPatchList,2>(MeshPatchList(EMPTY,EMPTY,0u), MeshPatchList(EMPTY,EMPTY,0u));
    var slot = input.head;
    for (var i = 0u; i < input.count; i++) {
        let next = contacts[slot].manifold_link.x;
        let part = select(1u, 0u, contact_persistent[slot].lifecycle.z == 1u);
        if (parts[part].count == 0u) { parts[part].head = slot; }
        else { contacts[parts[part].tail].manifold_link.x = slot + 1u; }
        contacts[slot].manifold_link.x = 0u;
        parts[part].tail = slot;
        parts[part].count += 1u;
        slot = next - 1u;
    }
    if (parts[0].count == 0u) { return parts[1]; }
    if (parts[1].count == 0u) { return parts[0]; }
    contacts[parts[0].tail].manifold_link.x = parts[1].head + 1u;
    return MeshPatchList(parts[0].head, parts[1].tail, input.count);
}

fn finalize_mesh_patch_list(input: MeshPatchList, root: u32, mesh: Body, convex: Body, ia: u32, ib: u32) -> Contact {
    var list = input;
    if (convex.kind == KIND_CAPSULE) { list = order_capsule_mesh_patches(input); }
    if (!contact_chain_structure_valid(root)) {
        discard_mesh_patch_list(list);
        return load_contact(root);
    }
    let old_color = contacts[root].color;
    var slot = list.head;
    for (var i = 0u; i < list.count; i++) {
        var piece = load_contact(slot);
        if (convex.kind == KIND_CAPSULE) {
            piece = reduce_capsule_mesh_cluster(slot);
            discard_mesh_raw_points(slot);
            piece.pair.z = 0u;
            piece.pair.w = 0u;
        }
        let previous = select_old_mesh_patch(root, piece);
        let friction = piece.friction;
        let restitution = piece._pad_end;
        let rolling = piece.rolling;
        // Native applies B3_MESH_REST_OFFSET to raw separation after clustering,
        // before solver preparation subtracts the COM anchor projection.
        // Subtracting from an already packed base loses low separation bits.
        for (var j = 0u; j < piece.count; j++) {
            let ra = ra_at(piece, j);
            let rb = rb_at(piece, j);
            let separation = rb.w - LINEAR_SLOP;
            if (j == 0u) { piece.persistent_rb0.w = separation; }
            else if (j == 1u) { piece.persistent_rb1.w = separation; }
            else if (j == 2u) { piece.persistent_rb2.w = separation; }
            else { piece.persistent_rb3.w = separation; }
            set_point(&piece, j, ra, vec4<f32>(rb.xyz, 0.0));
        }
        piece.cached_relative.w = select(5.0, 6.0, piece.cached_relative.w == 4.0);
        finish_manifold_from_previous(&piece, mesh, convex, ia, ib, previous);
        piece.friction = friction;
        piece._pad_end = restitution;
        piece.rolling = rolling;
        piece.color = old_color;
        store_mesh_contact(slot, piece);
        slot = piece.manifold_link.x - 1u;
    }
    if (!retire_manifold_children(root)) {
        discard_mesh_patch_list(list);
        return load_contact(root);
    }
    scratch[scr_contact_mark() + root] = 1u;
    if (list.count == 0u) { return empty_contact(); }
    var result = load_contact(list.head);
    result.manifold_link.y = 0u;
    result.manifold_link.z = list.count;
    store_mesh_contact(list.head, empty_contact());
    scratch[scr_contact_mark() + list.head] = 0u;
    return result;
}

// Box3D b3FindIncidentFace chooses between faces adjacent to the support
// vertex's most perpendicular edge. A global normal search can choose a face
// that does not contain the support vertex, producing an unrelated clip polygon.
fn hull_triangle_incident_face(shape: Shape, body: Body, normal: vec3<f32>) -> u32 {
    var vertex = EMPTY;
    var minimum = 1e30;
    for (var i = 0u; i < hull_point_count(shape); i++) {
        let projection = dot(hull_world_point(shape, body, i), normal);
        if (projection < minimum) {
            minimum = projection;
            vertex = i;
        }
    }
    var best_edge = EMPTY;
    var best_projection = 1e30;
    for (var i = 0u; i < hull_topology_count(shape); i++) {
        let edge = load_hull_topology(shape, i);
        if (edge.z != vertex) { continue; }
        let twin = load_hull_topology(shape, edge.y);
        let direction = hull_world_point(shape, body, twin.z) - hull_world_point(shape, body, vertex);
        let length_squared = dot(direction, direction);
        if (length_squared <= 1e-24) { continue; }
        let projection = abs(dot(direction * inverseSqrt(length_squared), normal));
        if (projection < best_projection) {
            best_projection = projection;
            best_edge = i;
        }
    }
    if (best_edge == EMPTY) { return EMPTY; }
    let edge = load_hull_topology(shape, best_edge);
    let twin = load_hull_topology(shape, edge.y);
    let n1 = quat_rotate(body.rot, load_hull_plane(shape, edge.w).xyz);
    let n2 = quat_rotate(body.rot, load_hull_plane(shape, twin.w).xyz);
    return select(twin.w, edge.w, dot(n1, normal) < dot(n2, normal));
}

struct TriangleHullResult { contact: Contact, separated: bool, }

fn hull_triangle_manifold(shape: Shape, body: Body, p1: vec3<f32>, p2: vec3<f32>, p3: vec3<f32>, normal: vec3<f32>, keep: f32) -> TriangleHullResult {
    var result: TriangleHullResult;
    result.contact = empty_contact();
    let tri = array<vec3<f32>,3>(p1,p2,p3);
    let edges = array<vec3<f32>,3>(p2-p1,p3-p2,p1-p3);
    let triangle_sep = dot(support_collider(shape,body,-normal)-p1,normal);
    if (triangle_sep > keep) { result.separated=true; return result; }
    var face_sep = -1e30;
    var face_index = EMPTY;
    var face_normal = normal;
    var face_point = body.pos;
    for (var i=0u; i<hull_plane_count(shape); i++) {
        let out = quat_rotate(body.rot,load_hull_plane(shape,i).xyz);
        let point = support_collider(shape,body,out);
        let separation = min(dot(p1-point,out),min(dot(p2-point,out),dot(p3-point,out)));
        if (separation > face_sep) {
            face_sep=separation; face_index=i; face_normal=-out; face_point=point;
        }
    }
    if (face_sep > keep) { result.separated=true; return result; }
    var edge_sep = -1e30;
    var edge_normal = normal;
    var hull_edge = EMPTY;
    var tri_edge = EMPTY;
    for (var i=0u; i<hull_topology_count(shape); i++) {
        if (i>=load_hull_topology(shape,i).y) { continue; }
        let edge=gauss_hull_edge(shape,body,i);
        if (!edge.valid) { continue; }
        for (var j=0u; j<3u; j++) {
            let cab=dot(edge.u,edges[j]);
            let dab=dot(edge.v,edges[j]);
            let bcd=dot(normal,edge.e);
            if (cab*dab>=0.0 || cab*bcd<=0.0) { continue; }
            if (max(cab*cab,dab*dab)<0.005*0.005*dot(edges[j],edges[j])) { continue; }
            let axis=normalize(mix(edge.u,edge.v,cab/(cab-dab)));
            let separation=dot(axis,tri[j]-edge.q);
            if (separation>edge_sep) {
                edge_sep=separation; edge_normal=-axis; hull_edge=i; tri_edge=j;
            }
        }
    }
    if (edge_sep>keep) { result.separated=true; return result; }
    var polygon: ClipPolygon;
    let hull_reference=face_index!=EMPTY && face_sep>=triangle_sep && dot(face_normal,normal)>=-0.25;
    var reference_normal=normal;
    var reference_point=p1;
    if (hull_reference) {
        reference_normal=face_normal;
        reference_point=face_point;
        polygon.count=3u;
        for(var i=0u;i<3u;i++) {
            polygon.points[i]=tri[i];
            polygon.features[i]=pack_feature(0u,(i+2u)%3u,0u,i);
        }
        let start=find_face_edge(shape,face_index);
        if(start!=EMPTY) {
            var edge=start;
            for(var i=0u;i<32u;i++) {
                let half=load_hull_topology(shape,edge);
                let next=load_hull_topology(shape,half.x);
                let a=hull_world_point(shape,body,half.z);
                let b=hull_world_point(shape,body,next.z);
                let side=normalize(cross(b-a,-reference_normal));
                polygon=clip_polygon_with_features(polygon,side,dot(side,a),edge,1u);
                edge=half.x;
                if(edge==start || polygon.count<3u) { break; }
            }
        } else { polygon.count=0u; }
        if(polygon.count<3u) { polygon.count=0u; }
    } else {
        let incident=hull_triangle_incident_face(shape,body,normal);
        let start=find_face_edge(shape,incident);
        if(start!=EMPTY) {
            var edge=start;
            for(var i=0u;i<32u;i++) {
                let half=load_hull_topology(shape,edge);
                let next=load_hull_topology(shape,half.x);
                polygon.points[polygon.count]=hull_world_point(shape,body,next.z);
                polygon.features[polygon.count]=pack_feature(1u,edge,1u,half.x);
                polygon.count++;
                edge=half.x;
                if(edge==start) { break; }
            }
        }
        for(var i=0u;i<3u;i++) {
            let side=normalize(cross(edges[i],normal));
            polygon=clip_polygon_with_features(polygon,side,dot(side,tri[i]),i,0u);
        }
    }
    var clipped=empty_contact();
    clipped.n=reference_normal;
    var clip_sep=select(1e30,face_sep,hull_reference);
    if(polygon.count>0u) {
        clip_sep=1e30;
        // Visit the complete clipped polygon before reducing to four contacts.
        // A fifth/later vertex may be the only penetrating support witness.
        for(var i=0u;i<polygon.count;i++) {
            let separation=select(dot(polygon.points[i]-reference_point,normal),
                dot(reference_point-polygon.points[i],reference_normal),hull_reference);
            clip_sep=min(clip_sep,separation);
            if(keep==0.0 && separation>0.0) { continue; }
            let point=select(polygon.points[i],polygon.points[i]+separation*reference_normal,hull_reference);
            // Equal temporary anchors make the reducer's separation equal
            // to the raw signed distance; callers rebase these world points.
            append_mesh_point_with_identity(&clipped, vec4<f32>(point,separation),
                vec4<f32>(point,0.0), reference_normal, 0u, polygon.features[i]);
        }
        if(clip_sep>keep) { clipped.count=0u; }
    }
    if(hull_edge!=EMPTY && ((clipped.count==0u && edge_sep>max(face_sep,triangle_sep)) || (clipped.count==1u && edge_sep>clip_sep+LINEAR_SLOP))) {
        clipped=empty_contact();
        let he=gauss_hull_edge(shape,body,hull_edge);
        let pa=tri[tri_edge]; let ea=edges[tri_edge];
        let pb=he.q-he.e; let eb=he.e;
        let w=pa-pb;
        let a11=dot(ea,ea); let a12=-dot(ea,eb); let a21=-a12; let a22=-dot(eb,eb);
        let b1=-dot(ea,w); let b2=-dot(eb,w);
        let det=a11*a22-a12*a21;
        var s=0.0; var t=0.0;
        if(det*det<1.17549435e-35) { s=dot(pb-pa,ea)/a11; }
        else { s=(a22*b1-a12*b2)/det; t=(a11*b2-a21*b1)/det; }
        if(s>=0.0 && s<=1.0 && t>=0.0 && t<=1.0) {
            clipped.count=1u; clipped.n=edge_normal;
            let point=0.5*(pa+s*ea+pb+t*eb);
            set_point(&clipped,0u,vec4<f32>(point,dot(edge_normal,pb-pa)),vec4<f32>(0.0));
            set_feat_at(&clipped,0u,pack_feature(0u,tri_edge,1u,hull_edge));
        }
    }
    result.contact=clipped;
    return result;
}

// Triangle/capsule face and edge construction follows Box3D triangle_manifold.c
// (MIT, Erin Catto). Segment/triangle witnesses are found analytically.
fn clip_triangle_segment(s: Seg2, plane_n: vec3<f32>, plane_c: f32) -> Seg2 {
    var outp: Seg2;
    outp.n = 0u;
    if (s.n < 2u) {
        return outp;
    }
    let d1 = dot(s.p0, plane_n) - plane_c;
    let d2 = dot(s.p1, plane_n) - plane_c;
    if (d1 <= 0.0) {
        outp.p0 = s.p0;
        outp.feature0 = s.feature0;
        outp.n = 1u;
    }
    if (d2 <= 0.0) {
        if (outp.n == 0u) {
            outp.p0 = s.p1;
            outp.feature0 = s.feature1;
        } else {
            outp.p1 = s.p1;
            outp.feature1 = s.feature1;
        }
        outp.n = outp.n + 1u;
    }
    if ((d1 > 0.0) != (d2 > 0.0)) {
        let t = clip_divide(d1,d1-d2);
        let hit = mix(s.p0, s.p1, t);
        let feature = select(s.feature1, s.feature0, d1 > 0.0);
        if (outp.n == 0u) {
            outp.p0 = hit;
            outp.feature0 = feature;
        } else if (outp.n == 1u) {
            outp.p1 = hit;
            outp.feature1 = feature;
        }
        outp.n = min(outp.n + 1u, 2u);
    }
    return outp;
}

struct TriangleCapsuleManifold {
    points: array<vec4<f32>, 2>,
    ids: vec2<u32>,
    normal: vec3<f32>,
    count: u32,
    feature: u32,
}

fn capsule_triangle_manifold(start: vec3<f32>, end: vec3<f32>, radius: f32,
    tri: array<vec3<f32>,3>, normal: vec3<f32>) -> TriangleCapsuleManifold {
    var out: TriangleCapsuleManifold;
    let direction = end - start;
    let plane_offset = gyro_dot3(normal, tri[0]);
    if (gyro_dot3(normal, 0.5 * (start + end)) - plane_offset < 0.0) { return out; }
    var clipped: Seg2;
    clipped.p0 = start; clipped.p1 = end; clipped.n = 2u;
    clipped.feature0 = 0u; clipped.feature1 = 0x00010001u;
    var previous = tri[2];
    for (var i=0u; i<3u; i++) {
        let tangent = gyro_norm3(tri[i] - previous);
        let side = gyro_cross(tangent, normal);
        clipped = clip_triangle_segment(clipped, side, gyro_dot3(side, previous));
        previous = tri[i];
        if (clipped.n != 2u) { break; }
    }
    let d0 = gyro_dot3(normal, clipped.p0) - plane_offset;
    let d1 = gyro_dot3(normal, clipped.p1) - plane_offset;
    let first = closest_triangle_point(start, tri[0], tri[1], tri[2]);
    var point_a = first.point;
    var point_b = start;
    var feature = first.feature;
    var squared = gyro_dot3(point_b-point_a, point_b-point_a);
    let last = closest_triangle_point(end, tri[0], tri[1], tri[2]);
    let last_squared = gyro_dot3(end-last.point, end-last.point);
    if (last_squared < squared) {
        squared=last_squared; point_a=last.point; point_b=end; feature=last.feature;
    }
    for (var i=0u; i<3u; i++) {
        let edge = tri[(i+1u)%3u] - tri[i];
        let fractions = closest_segments(tri[i], edge, start, direction);
        let pa = tri[i] + fractions.x * edge;
        let pb = start + fractions.y * direction;
        let distance = gyro_dot3(pb-pa,pb-pa);
        if (distance < squared) {
            squared=distance; point_a=pa; point_b=pb; feature=i;
            if (fractions.x <= 0.0) { feature=4u+i; }
            else if (fractions.x >= 1.0) { feature=4u+(i+1u)%3u; }
        }
    }
    if (clipped.n == 2u && min(d0,d1) <= 0.0 && max(d0,d1) >= 0.0) { squared=0.0; }
    let distance = gyro_sqrt(squared);
    if (distance > radius + SPECULATIVE) { return out; }
    var make_face = false;
    if (distance > 1.1920929e-5) {
        let axis = gyro_norm3(point_b-point_a);
        make_face = clipped.n == 2u && abs(gyro_dot3(normal,axis)) > 0.2;
        if (!make_face) {
            out.count=1u; out.normal=axis; out.feature=feature;
            out.points[0]=vec4<f32>(0.5*(point_a+point_b-radius*axis),distance-radius);
            out.ids.x=0u;
            return out;
        }
    } else {
        make_face = clipped.n == 2u && min(d0,d1) <= radius + SPECULATIVE;
    }
    if (make_face) {
        out.count=2u; out.normal=normal; out.feature=3u;
        out.points[0]=vec4<f32>(clipped.p0-0.5*(d0+radius)*normal,d0-radius);
        out.points[1]=vec4<f32>(clipped.p1-0.5*(d1+radius)*normal,d1-radius);
        out.ids=vec2<u32>(clipped.feature0,clipped.feature1);
    }
    if (distance > 1.1920929e-5) { return out; }
    // Deep overlap: compare capsule/triangle edge axes against the clipped face.
    var edge_separation = -3.402823466e38;
    var edge_index = EMPTY;
    var edge_normal = vec3<f32>(0.0);
    let a = gyro_dot3(direction,normal);
    previous = tri[2];
    for (var i=0u; i<3u; i++) {
        let side = gyro_norm3(gyro_cross(tri[i]-previous,normal));
        let b = gyro_dot3(direction,side);
        if (a*a+b*b >= 0.005*0.005*gyro_dot3(direction,direction)) {
            var axis: vec3<f32>;
            if (a*b <= 0.0) { let t=b/(b-a); axis=(1.0-t)*side+t*normal; }
            else { let t=b/(a+b); axis=(1.0-t)*side-t*normal; }
            axis=gyro_norm3(axis);
            let separation=gyro_dot3(axis,start-previous);
            if (separation > edge_separation) {
                edge_separation=separation; edge_normal=axis; edge_index=(i+2u)%3u;
            }
        }
        previous=tri[i];
    }
    if (edge_separation > radius) { out.count=0u; return out; }
    var face_separation=min(gyro_dot3(normal,start),gyro_dot3(normal,end))-plane_offset-radius;
    if (out.count==2u) { face_separation=min(out.points[0].w,out.points[1].w); }
    if (edge_index != EMPTY && (out.count==0u || edge_separation-radius > face_separation+LINEAR_SLOP)) {
        let p=tri[edge_index];
        let edge=tri[(edge_index+1u)%3u]-p;
        let r=p-start;
        let aa=gyro_dot3(edge,edge); let ee=gyro_dot3(direction,direction);
        let bb=gyro_dot3(edge,direction); let cc=gyro_dot3(edge,r); let ff=gyro_dot3(direction,r);
        let denominator=aa*ee-bb*bb;
        if (denominator > 1.17549435e-35) {
            let u=(bb*ff-cc*ee)/denominator;
            let v=(aa*ff-bb*cc)/denominator;
            if (u>=0.0 && u<=1.0 && v>=0.0 && v<=1.0) {
                out.count=1u; out.normal=edge_normal; out.feature=edge_index;
                out.points[0]=vec4<f32>(0.5*((p+u*edge)+(start+v*direction)-radius*edge_normal),edge_separation-radius);
                out.ids.x=pack_feature(0u,edge_index,1u,0u);
            }
        }
    }
    return out;
}

fn collide_mesh_triangle(mesh_shape: Shape, convex_shape: Shape, mesh: Body, convex: Body,
    mesh_body_index: u32, convex_body_index: u32, triangle_index: u32) -> Contact {
    var result = empty_contact();
    result.a = mesh_body_index;
    result.b = convex_body_index;
    result.friction = mixed_friction_of(mesh, convex);
    result.manifold_link.w = triangle_index + 1u;
    var best_separation = 1e30;
    var best_normal = vec3<f32>(0.0, 1.0, 0.0);
    var material_samples = 0.0;
    var mixed_friction = 0.0;
    var mixed_restitution = 0.0;
    var mesh_tangent_velocity = vec3<f32>(0.0);
    var dominant_material = 0u;
    let triangle = load_mesh_triangle(mesh_shape, triangle_index);
    let p1 = mesh_local_vertex(mesh_shape, mesh, triangle.x);
    let p2 = mesh_local_vertex(mesh_shape, mesh, triangle.y);
    let p3 = mesh_local_vertex(mesh_shape, mesh, triangle.z);
    let normal = normalize(cross(p2 - p1, p3 - p1));
    // Hulls retain shallow backside overlap, as triangle_manifold.c.
    // Sphere/capsule routines use a strict zero-side test instead.
    let backside_limit = select(0.0, -LINEAR_SLOP,
        convex.kind == KIND_BOX || convex.kind == KIND_CONVEX_HULL);
    if (convex.kind != KIND_CAPSULE && dot(convex.pos - p1, normal) < backside_limit) { return empty_contact(); }
    let material_index = triangle.w >> 8u;
    let flags = triangle.w & 0xffu;
    var radius = 0.0;
    var capsule_face = false;
    if (convex.kind == KIND_SPHERE) {
        add_mesh_sample_contact(
            &result, mesh, convex, convex_shape, mesh_shape, convex.pos, convex.half.x,
            p1, p2, p3, normal, flags, material_index, &best_separation, &best_normal,
            &material_samples, &mixed_friction, &mixed_restitution, &mesh_tangent_velocity,
            &dominant_material, 0u,
        );
    } else if (convex.kind == KIND_CAPSULE) {
        // Native mesh_contact.c transforms the triangle into the capsule's
        // body frame. Preserve the original endpoints and avoid rounding a
        // rotated proxy axis or subtracting large world-space coordinates.
        let capsule_body = load_body(convex_body_index);
        let mesh_body = load_body(mesh_body_index);
        let capsule_origin = body_origin(capsule_body,convex_body_index);
        let mesh_origin = body_origin(mesh_body,mesh_body_index);
        let relative_rotation = quat_mul(quat_inv(capsule_body.rot), mesh_body.rot);
        let relative_position = quat_inv_rotate(capsule_body.rot, mesh_origin - capsule_origin);
        let local_triangle = array<vec3<f32>,3>(
            contact_matrix_rotate(relative_rotation, load_mesh_vertex(mesh_shape, triangle.x)) + relative_position,
            contact_matrix_rotate(relative_rotation, load_mesh_vertex(mesh_shape, triangle.y)) + relative_position,
            contact_matrix_rotate(relative_rotation, load_mesh_vertex(mesh_shape, triangle.z)) + relative_position);
        let local_normal = gyro_norm3(gyro_cross(local_triangle[1]-local_triangle[0],
            local_triangle[2]-local_triangle[0]));
        radius = convex.half.x;
        let manifold = capsule_triangle_manifold(capsule_local_point(convex_shape,0u),
            capsule_local_point(convex_shape,1u),radius,local_triangle,local_normal);
        capsule_face = manifold.feature == 3u;
        let world_normal = contact_matrix_rotate(capsule_body.rot, manifold.normal);
        let triangle_normal = contact_matrix_rotate(capsule_body.rot, local_normal);
        if (manifold.count > 0u) {
            var patch_separation=1e30;
            for (var i=0u;i<manifold.count;i++) { patch_separation=min(patch_separation,manifold.points[i].w); }
            for (var i=0u;i<manifold.count;i++) {
                let old_count = result.count;
                consider_mesh_manifold_point(&result,mesh,convex,convex_shape,mesh_shape,
                    contact_matrix_rotate(capsule_body.rot,manifold.points[i].xyz),
                    manifold.points[i].w,patch_separation,world_normal,
                    triangle_normal,material_index,manifold.ids[i],&best_separation,&best_normal,
                    &material_samples,&mixed_friction,&mixed_restitution,&mesh_tangent_velocity,&dominant_material);
                if (result.count > old_count) {
                    if (old_count == 0u) { result.persistent_ra0 = manifold.points[i]; }
                    else { result.persistent_ra1 = manifold.points[i]; }
                }
            }
            result.cached_rotation_a = vec4<f32>(local_normal, 0.0);
        }
    } else if (convex.kind == KIND_CONVEX_HULL) {
        let query=hull_triangle_manifold(convex_shape,convex,p1,p2,p3,normal,mesh_speculative_keep(convex_shape,mesh_shape));
        if(query.separated) { return empty_contact(); }
        let raw=query.contact;
        if(raw.count>0u) {
            var patch_separation=1e30;
            for(var i=0u;i<raw.count;i++) { patch_separation=min(patch_separation,ra_at(raw,i).w); }
            for(var i=0u;i<raw.count;i++) {
                let point=ra_at(raw,i);
                consider_mesh_manifold_point(&result,mesh,convex,convex_shape,mesh_shape,
                    point.xyz,point.w,patch_separation,raw.n,normal,material_index,feat_at(raw,i),
                    &best_separation,&best_normal,&material_samples,&mixed_friction,&mixed_restitution,
                    &mesh_tangent_velocity,&dominant_material);
            }
        } else {
        let count = hull_point_count(convex_shape);
        for (var hull_index = 0u; hull_index < count; hull_index++) {
            let local = load_hull_point(convex_shape, hull_index) - convex_shape.local_center;
            let sample = convex.pos + quat_rotate(convex.rot, local);
            add_mesh_sample_contact(
                &result, mesh, convex, convex_shape, mesh_shape, sample, 0.0,
                p1, p2, p3, normal, flags, material_index, &best_separation, &best_normal,
                &material_samples, &mixed_friction, &mixed_restitution, &mesh_tangent_velocity,
                &dominant_material, hull_index,
            );
        }
        if (count == 0u) {
            add_mesh_sample_contact(
                &result, mesh, convex, convex_shape, mesh_shape,
                support_collider(convex_shape, convex, -normal), 0.0,
                p1, p2, p3, normal, flags, material_index, &best_separation, &best_normal,
                &material_samples, &mixed_friction, &mixed_restitution, &mesh_tangent_velocity,
                &dominant_material, 0u,
            );
        }
        clip_hull_face_to_triangle(
            &result, mesh, convex, convex_shape, mesh_shape, p1, p2, p3, normal,
            material_index, &best_separation, &best_normal, &material_samples,
            &mixed_friction, &mixed_restitution, &mesh_tangent_velocity, &dominant_material,
        );
        }
    } else if (convex.kind == KIND_BOX) {
        clip_hull_face_to_triangle(
            &result, mesh, convex, convex_shape, mesh_shape, p1, p2, p3, normal,
            material_index, &best_separation, &best_normal, &material_samples,
            &mixed_friction, &mixed_restitution, &mesh_tangent_velocity, &dominant_material,
        );
    } else {
        add_mesh_sample_contact(
            &result, mesh, convex, convex_shape, mesh_shape,
            support_collider(convex_shape, convex, -normal), 0.0,
            p1, p2, p3, normal, flags, material_index, &best_separation, &best_normal,
            &material_samples, &mixed_friction, &mixed_restitution, &mesh_tangent_velocity,
            &dominant_material, 0u,
        );
    }
    if (result.count == 0u) { return empty_contact(); }
    result.n = best_normal;
    result.lifecycle.y = u32(result._pad_end); // Temporary confirmed/tentative classification.
    // Temporary fresh-patch ordering metadata; commit_pair_manifold replaces it
    // with the persistent graph's local index after all patches are finalized.
    result.lifecycle.z = select(0u, 1u, capsule_face);
    if (material_samples > 0.0) {
        let inv_material_samples = 1.0 / material_samples;
        result.friction = mixed_friction * inv_material_samples;
        result._pad_end = mixed_restitution * inv_material_samples;
        result.tangent_velocity =
            quat_rotate(mesh.rot, mesh_tangent_velocity * inv_material_samples)
            - quat_rotate(convex.rot, convex_shape.tangent_velocity);
        result.material_index = min(dominant_material, max(mesh_shape.material_count, 1u) - 1u);
        result.rolling = select(
            0.0,
            convex_shape.rolling * mesh_rolling_radius(convex_shape),
            (params.diagnostic_flags & DIAG_DISABLE_ROLLING) == 0u,
        );
    }
    return result;
}

// Keep capsule triangles separate until face coverage is known. Filtering after
// clustering would lose the triangle identities needed to suppress flat seams.
fn stage_capsule_mesh_patch(list: ptr<function, MeshPatchList>, root: u32, candidate: Contact) -> bool {
    let slot = allocate_manifold_slot(root);
    if (slot == EMPTY) { return false; }
    var raw = candidate;
    raw.pair.z = 0u; raw.pair.w = 0u;
    raw.manifold_link.x = 0u;
    store_mesh_contact(slot, raw);
    if ((*list).count == 0u) { (*list).head = slot; }
    else { contacts[(*list).tail].manifold_link.x = slot + 1u; }
    (*list).tail = slot; (*list).count += 1u;
    return true;
}

fn filter_capsule_mesh_patches(list: ptr<function, MeshPatchList>, root: u32, shape: Shape, mesh: Body) -> bool {
    var slot = (*list).head;
    for (var i=0u; i<(*list).count; i++) {
        let candidate = load_contact(slot);
        let triangle = load_mesh_triangle(shape, candidate.manifold_link.w - 1u);
        var covered = 0u;
        let flags = triangle.w & 255u;
        let flat = flags & (flags >> 4u) & 7u;
        if (candidate.lifecycle.z == 0u && flat != 0u && flat != 7u) {
            var other_slot = (*list).head;
            for (var j=0u; j<(*list).count; j++) {
                let other = load_contact(other_slot);
                if (other.lifecycle.z == 1u) {
                    let face = load_mesh_triangle(shape, other.manifold_link.w - 1u).xyz;
                    for (var edge=0u; edge<3u; edge++) {
                        let a = triangle[edge]; let b = triangle[(edge+1u)%3u];
                        if (any(face == vec3<u32>(a)) && any(face == vec3<u32>(b))) {
                            covered |= 1u << edge;
                        }
                    }
                }
                other_slot = other.manifold_link.x - 1u;
            }
        }
        contact_persistent[slot].lifecycle.w = u32(capsule_mesh_feature_allowed(
            select(0u,3u,candidate.lifecycle.z == 1u),flags,covered));
        trace_mesh_candidate(2u,shape,shape,candidate.manifold_link.w,
            vec3<f32>(f32(flat),f32(covered),f32(candidate.lifecycle.z)),
            f32(contact_persistent[slot].lifecycle.w),candidate.n,0.0,flags);
        slot = candidate.manifold_link.x - 1u;
    }
    let input = *list;
    var output = MeshPatchList(EMPTY,EMPTY,0u);
    slot = input.head;
    for (var i=0u; i<input.count; i++) {
        var candidate = load_contact(slot);
        let next = candidate.manifold_link.x;
        let keep = candidate.lifecycle.w != 0u;
        candidate.lifecycle.w = 0u;
        // Naga 26 lowers function calls in both binary operands eagerly.
        // Keep this mutating append inside an explicit control-flow branch.
        if (keep) {
            if (!append_mesh_patch(&output,root,candidate,shape,mesh)) {
                discard_mesh_patch_list(MeshPatchList(slot,input.tail,input.count-i));
                discard_mesh_patch_list(output);
                *list = MeshPatchList(EMPTY,EMPTY,0u);
                return false;
            }
        }
        store_mesh_contact(slot,empty_contact());
        scratch[scr_contact_mark()+slot] = 0u;
        slot = next - 1u;
    }
    *list = output;
    return true;
}

fn collide_mesh_convex(
    mesh_shape: Shape,
    convex_shape: Shape,
    mesh: Body,
    convex: Body,
    mesh_body_index: u32,
    convex_body_index: u32,
    root_slot: u32,
) -> Contact {
    var list = MeshPatchList(EMPTY, EMPTY, 0u);
    var stack: array<u32, 64>;
    var stack_count = 1u;
    stack[0] = 0u;
    for (var visit = 0u; visit < 2048u && stack_count > 0u; visit++) {
        stack_count = stack_count - 1u;
        let node_index = stack[stack_count];
        if (node_index >= mesh_shape.topology_counts
            || !mesh_node_overlaps(mesh_shape, mesh, convex, node_index)) {
            continue;
        }
        let lower = load_mesh_node_lower(mesh_shape, node_index);
        let upper = load_mesh_node_upper(mesh_shape, node_index);
        let data = bitcast<u32>(lower.w);
        if ((data & 3u) != 3u) {
            let right = node_index + (data >> 2u);
            let left = node_index + 1u;
            mesh_push_child(&stack, &stack_count, mesh_shape, mesh, convex, right);
            mesh_push_child(&stack, &stack_count, mesh_shape, mesh, convex, left);
            continue;
        }
        let triangle_count = data >> 2u;
        let triangle_offset = bitcast<u32>(upper.w);
        for (var leaf_index = 0u; leaf_index < triangle_count; leaf_index++) {
            let triangle_index = triangle_offset + leaf_index;
            if (triangle_index >= mesh_shape.topology_slot) { continue; }
            let candidate = collide_mesh_triangle(mesh_shape, convex_shape, mesh, convex,
                mesh_body_index, convex_body_index, triangle_index);
            if (candidate.count > 0u) {
                var appended: bool;
                if (convex.kind == KIND_CAPSULE) {
                    appended = stage_capsule_mesh_patch(&list, root_slot, candidate);
                } else {
                    appended = append_mesh_patch(&list, root_slot, candidate, mesh_shape, mesh);
                }
                if (!appended) {
                    discard_mesh_patch_list(list);
                    return load_contact(root_slot);
                }
            }
        }
    }
    if (stack_count > 0u) {
        record_capacity_drop_n(ATOM_CONTACT_DROPPED, ATOM_STICKY_CONTACT_DROPPED, stack_count);
    }
    if (convex.kind == KIND_CAPSULE) {
        if (!filter_capsule_mesh_patches(&list,root_slot,mesh_shape,mesh)) {
            return load_contact(root_slot);
        }
    }
    return finalize_mesh_patch_list(list, root_slot, mesh, convex, mesh_body_index, convex_body_index);
}

fn collide_pair(a_in: Body, b_in: Body, ia_in: u32, ib_in: u32, root_slot: u32) -> Contact {
    if (a_in.kind == KIND_MESH || b_in.kind == KIND_MESH) {
        if (!COLLISION_MESH_ENABLED) {
            // Empty mesh geometry has no triangles. A nonempty mesh reaching
            // this variant is a scheduling error and must fail closed.
            if (params.mesh_triangle_count != 0u) { record_contact_drop(14u); }
            return empty_contact();
        }
        if (a_in.kind == KIND_MESH && b_in.kind != KIND_MESH) {
            return collide_mesh_convex(
                load_shape(a_in._pad_island.x),
                load_shape(b_in._pad_island.x),
                a_in,
                b_in,
                ia_in,
                ib_in, root_slot,
            );
        }
        if (b_in.kind == KIND_MESH && a_in.kind != KIND_MESH) {
            return collide_mesh_convex(
                load_shape(b_in._pad_island.x),
                load_shape(a_in._pad_island.x),
                b_in,
                a_in,
                ib_in,
                ia_in, root_slot,
            );
        }
        return empty_contact();
    }
    if (a_in.kind == KIND_CONVEX_HULL || b_in.kind == KIND_CONVEX_HULL) {
        return collide_convex_pair(a_in, b_in, ia_in, ib_in);
    }
    var a = a_in;
    var b = b_in;
    var ia = ia_in;
    var ib = ib_in;
    if ((b.kind == KIND_SPHERE && a.kind != KIND_SPHERE)
        || (a.kind == KIND_BOX && b.kind == KIND_CAPSULE)) {
        let tmpb = a;
        a = b;
        b = tmpb;
        let tmpi = ia;
        ia = ib;
        ib = tmpi;
    }

    if (a.kind == KIND_BOX && b.kind == KIND_BOX) {
        return collide_boxes(a, b, ia, ib);
    }
    if (a.kind == KIND_CAPSULE && b.kind == KIND_CAPSULE) {
        // A dynamic proxy queries the static/kinematic tree: the queried
        // shape becomes A. Between initially moving dynamic proxies, the
        // later proxy becomes A. Clipping nearly parallel capsules depends
        // on this choice of reference segment, not just the normal's sign.
        if (capsule_reference_reversed(a.flags, b.flags, a._pad_island.x, b._pad_island.x)) {
            return collide_capsules(b, a, ib, ia);
        }
        return collide_capsules(a, b, ia, ib);
    }
    if (a.kind == KIND_CAPSULE && b.kind == KIND_BOX) {
        return collide_capsule_box(a, b, ia, ib);
    }

    var c = empty_contact();
    c.a = ia;
    c.b = ib;
    c.friction = mixed_friction_of(a, b);
    var n = vec3<f32>(0.0, 1.0, 0.0);
    var depth = -1.0;
    var p = 0.5 * (a.pos + b.pos);
    let ka = a.kind;
    let kb = b.kind;
    if (ka == KIND_SPHERE && kb == KIND_SPHERE) {
        let offset = b.pos - a.pos;
        let dist2 = dot(offset, offset);
        let min_d = a.half.x + b.half.x;
        if (dist2 <= min_d * min_d) {
            var dist = sqrt(dist2);
            n = vec3<f32>(0.0, 1.0, 0.0);
            if (dist2 > 1000.0 * 1.175494e-38) {
                n = offset / dist;
            } else {
                dist = 0.0;
            }
            depth = min_d - dist;
            p = 0.5 * ((a.pos + n * a.half.x) + (b.pos - n * b.half.x));
        }
    } else if (ka == KIND_SPHERE && kb != KIND_SPHERE) {
        var closest: vec3<f32>;
        if (kb == KIND_CAPSULE) {
            let ax = capsule_axis(b);
            closest = closest_on_segment(a.pos, b.pos - ax, b.pos + ax);
        } else {
            closest = closest_on_obb(a.pos, b);
        }
        let delta = a.pos - closest;
        let dist = length(delta);
        let rad = a.half.x + select(0.0, b.half.x, kb == KIND_CAPSULE);
        if (dist > 1e-8 && dist <= rad + SPECULATIVE) {
            n = delta / dist;
            depth = rad - dist;
            p = closest + n * select(0.0, b.half.x, kb == KIND_CAPSULE);
        }
    }

    if (depth > -SPECULATIVE) {
        c.count = 1u;
        if (dot(b.pos - a.pos, n) < 0.0) {
            n = -n;
        }
        c.n = n;
        let rA = p - a.pos;
        let rB = p - b.pos;
        let s = -depth;
        let base = s - dot(rB - rA, n);
        set_point(&c, 0u, vec4<f32>(rA, base), vec4<f32>(rB, 0.0));
        finish_manifold(&c, a, b, ia, ib);
    } else {
        c.a = EMPTY;
    }
    return c;
}

fn pair_already(ia: u32, ib: u32, filled: u32) -> bool {
    for (var k = 0u; k < filled; k++) {
        let c = load_contact(k);
        if ((c.a == ia && c.b == ib) || (c.a == ib && c.b == ia)) {
            return true;
        }
    }
    return false;
}

fn pair_queued(ia: u32, ib: u32, npairs: u32) -> bool {
    for (var k = 0u; k < npairs; k++) {
        let packed = load_pair_words(SCR_PAIRS, k);
        let a = packed.y;
        let b = packed.x;
        if ((a == ia && b == ib) || (a == ib && b == ia)) {
            return true;
        }
    }
    return false;
}

// Box3D caches body-origin transforms, not the COM transforms used by TGS.
fn relative_body_origin(a: Body, b: Body, ia: u32, ib: u32) -> vec3<f32> {
    let center_a = load_body_cold(ia).local_center;
    let center_b = load_body_cold(ib).local_center;
    let origin_a = body_origin(a,ia);
    let origin_b = body_origin(b,ib);
    return quat_inv_rotate(a.rot, origin_b - origin_a);
}

fn contact_recycle_extent(i: u32) -> vec3<f32> {
    let extra = body_extra_offset(i);
    return vec3<f32>(scene_f32(extra + 4u), scene_f32(extra + 5u), scene_f32(extra + 6u));
}

fn recycle_rotation_arc(q: vec4<f32>, extent: vec3<f32>) -> vec3<f32> {
    let v = abs(q.xyz);
    return vec3<f32>(
        v.y * extent.z + v.z * extent.y,
        v.z * extent.x + v.x * extent.z,
        v.x * extent.y + v.y * extent.x,
    );
}

fn make_ghost_pair(ia: u32, ib: u32, a: Body, b: Body) -> Contact {
    var g = empty_contact();
    g.a = ia;
    g.b = ib;
    g.count = 0u;
    g.cached_relative = vec4<f32>(relative_body_origin(a, b, ia, ib), 1.0);
    g.cached_rotation_a = a.rot;
    g.cached_rotation_b = b.rot;
    return g;
}

// Contact invariant: `c.a`/`c.b`, anchors, normal, cached rotations/relative body origins,
// and impulse signs all describe the *manifold* endpoint order. Packed pair keys
// stay canonical (min,max shape ids) and do not define manifold orientation.
fn contact_pair_bodies_match(c: Contact, body_a: u32, body_b: u32) -> bool {
    return c.a != EMPTY && c.a == body_a && c.b == body_b;
}

fn contact_pair_bodies_reversed(c: Contact, body_a: u32, body_b: u32) -> bool {
    return c.a != EMPTY && c.a == body_b && c.b == body_a;
}

fn recycle_skip(p: Contact, a: Body, b: Body) -> bool {
    // Empty manifolds carry no separation bound proving that their pair stays
    // outside the speculative shell. In particular, CCD can stop a body inside
    // that shell after moving less than the recycle tolerance. Reusing an empty
    // result then leaves the next step without discrete support, while CCD
    // correctly delegates initial contact to the discrete solver.
    if (p.count == 0u) {
        return false;
    }
    if (((a.flags | b.flags) & FLAG_DISABLE_CONTACT_RECYCLING) != 0u) {
        return false;
    }
    if ((params.diagnostic_flags & DIAG_DISABLE_RECYCLING) != 0u) {
        return false;
    }
    if (((a.flags | b.flags) & FLAG_FIXED_ROTATION) == FLAG_FIXED_ROTATION) {
        return false;
    }
    if (p.a == EMPTY || p.cached_relative.w < 0.5) {
        return false;
    }
    let angle_a = dot(a.rot, p.cached_rotation_a);
    let angle_b = dot(b.rot, p.cached_rotation_b);
    let angular = min(angle_a * angle_a, angle_b * angle_b);
    if (angular <= CONTACT_RECYCLE_ANG) {
        return false;
    }
    let current_relative = relative_body_origin(a, b, p.a, p.b);
    let d = current_relative - p.cached_relative.xyz;
    let d2 = dot(d, d);
    let was_touching = p.count > 0u && (p.lifecycle.y & CONTACT_TOUCHING) != 0u;
    let recycle_tolerance = select(min(params.contact_recycle_distance, SPECULATIVE), params.contact_recycle_distance, was_touching);
    if (d2 >= recycle_tolerance * recycle_tolerance) {
        return false;
    }
    let distance = sqrt(d2);
    let slack = recycle_tolerance - distance;
    let cached_relative_rotation =
        quat_mul(quat_inv(p.cached_rotation_a), p.cached_rotation_b);
    let current_relative_rotation = quat_mul(quat_inv(a.rot), b.rot);
    let qr = quat_mul(quat_inv(cached_relative_rotation), current_relative_rotation);
    let extent_a = select(contact_recycle_extent(p.a), vec3<f32>(0.0), is_static(a));
    let extent_b = select(contact_recycle_extent(p.b), vec3<f32>(0.0), is_static(b));
    let arc = 2.0 * length(recycle_rotation_arc(qr, max(extent_a, extent_b)));
    return arc < slack;
}

// Native recycling uses b3MakeMatrixFromQuat followed by b3MulMV.
fn recycle_rotate(q: vec4<f32>, v: vec3<f32>) -> vec3<f32> {
    let xx=q.x*q.x; let yy=q.y*q.y; let zz=q.z*q.z;
    let xy=q.x*q.y; let xz=q.x*q.z; let xw=q.x*q.w;
    let yz=q.y*q.z; let yw=q.y*q.w; let zw=q.z*q.w;
    let cx=vec3<f32>(1.0-2.0*(yy+zz),2.0*(xy+zw),2.0*(xz-yw));
    let cy=vec3<f32>(2.0*(xy-zw),1.0-2.0*(xx+zz),2.0*(yz+xw));
    let cz=vec3<f32>(2.0*(xz+yw),2.0*(yz-xw),1.0-2.0*(xx+yy));
    return (v.x*cx + v.y*cy) + v.z*cz;
}

fn recycle_contact(p: Contact, a: Body, b: Body) -> Contact {
    if (p.count == 0u) {
        return p;
    }
    var c = p;
    if (!(is_immovable(a) && is_immovable(b))) {
        c.friction_impulse = reproject_contact_friction(p.n, p.n, p.friction_impulse, 1.0);
    }
    let qa = quat_mul(a.rot, quat_inv(p.cached_rotation_a));
    let qb = quat_mul(b.rot, quat_inv(p.cached_rotation_b));
    let dc = b.pos - a.pos;
    // Box3D recycle keeps the cached world normal and original COM anchors,
    // then updates only `mp->separation` (see physics_world.c). Rotating those
    // into the solver double-counts TGS deltas and tilts stacked columns.
    let n = p.n;
    c.n = n;
    var center_a = vec3<f32>(0.0);
    var center_b = vec3<f32>(0.0);
    var weight_sum = 0.0;
    for (var i = 0u; i < p.count; i++) {
        let ra0 = persistent_ra_at(p, i);
        let rb0 = persistent_rb_at(p, i);
        let rotated_a = recycle_rotate(qa, ra0.xyz);
        let rotated_b = recycle_rotate(qb, rb0.xyz);
        let old_s = rb0.w;
        let s = old_s + gyro_dot3(dc + (rotated_b - rotated_a), n);
        let base = s - gyro_dot3(rb0.xyz - ra0.xyz, n);
        let weight = clamp(2.0 - s * (1.0 / SPECULATIVE), MIN_FRICTION_WEIGHT, 1.0);
        center_a = center_a + weight * ra0.xyz;
        center_b = center_b + weight * rb0.xyz;
        weight_sum = weight_sum + weight;
        set_point(&c, i, vec4<f32>(ra0.xyz, base), vec4<f32>(rb0.xyz, rb_at(p, i).w));
    }
    let inv_weight = gyro_recip(max(weight_sum, MIN_FRICTION_WEIGHT));
    c.center_a = inv_weight * center_a;
    c.center_b = inv_weight * center_b;
    return c;
}

fn store_pair(slot: u32, key: vec2<u32>, man: Contact, ia: u32, ib: u32, a: Body, b: Body) {
    let previous = load_contact(slot);
    let generation = select(previous.lifecycle.x + 1u, previous.lifecycle.x, previous.a != EMPTY);
    let local_order = select(
        EMPTY,
        previous.lifecycle.z,
        previous.a != EMPTY && (previous.lifecycle.y & CONTACT_TOUCHING) != 0u,
    );
    let was_touching =
        previous.a != EMPTY && (previous.lifecycle.y & CONTACT_TOUCHING) != 0u;
    if (man.a != EMPTY && man.count > 0u) {
        var touching = man;
        var flags = CONTACT_ALIVE | CONTACT_TOUCHING | (man.lifecycle.y & CONTACT_PERSISTED_MASK);
        if (!was_touching) {
            flags = flags | CONTACT_START_TOUCHING;
        }
        touching.lifecycle = vec4<u32>(
            generation,
            flags,
            local_order,
            previous.color,
        );
        touching.pair = vec4<u32>(key, 0u, 0u);
        store_contact(slot, touching);
        var child = touching.manifold_link.x;
        for (var i = 1u; i < max(touching.manifold_link.z, 1u); i++) {
            let child_slot = child - 1u;
            var piece = load_contact(child_slot);
            piece.lifecycle = vec4<u32>(piece.lifecycle.x, CONTACT_ALIVE | CONTACT_TOUCHING | (piece.lifecycle.y & CONTACT_PERSISTED_MASK), EMPTY, 0u);
            piece.pair = vec4<u32>(key, 0u, 0u);
            piece.color = touching.color;
            child = piece.manifold_link.x;
            store_contact(child_slot, piece);
        }
        return;
    }
    var ghost = make_ghost_pair(ia, ib, a, b);
    if (load_shape(key.x).kind == KIND_CAPSULE && load_shape(key.y).kind == KIND_CAPSULE
        && capsule_reference_reversed(a.flags, b.flags, key.x, key.y)) {
        ghost = make_ghost_pair(ib, ia, b, a);
    }

    if (previous.a != EMPTY) {
        ghost.color = previous.color;
    }
    var flags = CONTACT_ALIVE;
    if (was_touching) {
        flags = flags | CONTACT_STOP_TOUCHING;
    }
    ghost.lifecycle = vec4<u32>(generation, flags, local_order, 0u);
    ghost.pair = vec4<u32>(key, 0u, 0u);
    store_contact(slot, ghost);
}

var<workgroup> wg_body_a: array<Body, 64>;
var<workgroup> wg_body_b: array<Body, 64>;

@compute @workgroup_size(64)
fn collide_pairs(
    @builtin(global_invocation_id) dispatch_gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let k = gid.x;
    let npairs = scratch[SCR_UNIQUE_N];
    var ia = 0u;
    var ib = 0u;
    var valid = false;
    if (k < npairs) {
        let packed = load_pair_words(SCR_PAIRS, k);
        ia = packed.x;
        ib = packed.y;
        valid = ia != ib && ia != EMPTY && ib != EMPTY
            && ia < params.shape_count && ib < params.shape_count
            && load_shape(ia).body_index != load_shape(ib).body_index;
    }
    if (valid) {
        wg_body_a[lid] = load_collider(ia);
        wg_body_b[lid] = load_collider(ib);
    }
    workgroupBarrier();
    if (!valid) {
        return;
    }
    let a = wg_body_a[lid];
    let b = wg_body_b[lid];
    let packed = load_pair_words(SCR_PAIRS, k);
    let shape_a = load_shape(ia);
    let shape_b = load_shape(ib);
    let body_a = load_body(shape_a.body_index);
    let body_b = load_body(shape_b.body_index);
    let sensor_pair = ((shape_a.event_flags | shape_b.event_flags) & SHAPE_IS_SENSOR) != 0u;
    let slot = scratch[scr_active_contact() + k];
    if (slot == EMPTY) {
        return;
    }
    let previous = find_prev_contact_key(packed, shape_a.body_index, shape_b.body_index);
    let prev_order_ok = contact_pair_bodies_match(previous, shape_a.body_index, shape_b.body_index)
        || contact_pair_bodies_reversed(previous, shape_a.body_index, shape_b.body_index);
    // physics_world.c: fast mesh contacts must refresh their triangle manifold.
    // The fast flag comes from the preceding GPU step, without a body download.
    let fast_mesh = params.enable_continuous != 0u
        && (shape_a.kind == KIND_MESH || shape_b.kind == KIND_MESH)
        && ((body_a.flags | body_b.flags) & FLAG_FAST) != 0u;
    let refresh_speculative = (params.diagnostic_flags & SOLVER_REFRESH_SPECULATIVE) != 0u
        && ((shape_a.kind == KIND_MESH && (shape_b.kind == KIND_BOX || shape_b.kind == KIND_CONVEX_HULL))
            || (shape_b.kind == KIND_MESH && (shape_a.kind == KIND_BOX || shape_a.kind == KIND_CONVEX_HULL)));
    if (!sensor_pair && prev_order_ok && !fast_mesh && !refresh_speculative && contact_chain_structure_valid(slot)) {
        let recycle_a = load_body(previous.a);
        let recycle_b = load_body(previous.b);
        let count = max(previous.manifold_link.z, 1u);
        var member = slot;
        var can_recycle = true;
        for (var i = 0u; i < count; i++) {
            let old = load_contact(member);
            can_recycle = can_recycle && recycle_skip(old, recycle_a, recycle_b);
            member = old.manifold_link.x - 1u;
        }
        if (can_recycle) {
            member = slot;
            for (var i = 0u; i < count; i++) {
                let old = load_contact(member);
                var recycled = recycle_contact(old, recycle_a, recycle_b);
                recycled.lifecycle.y = CONTACT_ALIVE | CONTACT_RECYCLED | persisted_point_bits(recycled.count);
                if (recycled.count > 0u && (old.lifecycle.y & CONTACT_TOUCHING) != 0u) {
                    recycled.lifecycle.y |= CONTACT_TOUCHING;
                }
                recycled.lifecycle.w = old.color;
                recycled.pair = vec4<u32>(packed, 0u, 0u);
                store_contact(member, recycled);
                member = old.manifold_link.x - 1u;
            }
            return;
        }
    }
    var man = collide_pair(a, b, shape_a.body_index, shape_b.body_index, slot);
    let contained = sensor_pair
        && (point_inside_collider(shape_a, a, b.pos, 0.0)
            || point_inside_collider(shape_b, b, a.pos, 0.0));
    if (contained && man.a == EMPTY) {
        man.a = shape_a.body_index;
        man.b = shape_b.body_index;
        man.count = 1u;
        man.n = vec3<f32>(1.0, 0.0, 0.0);
        man.ra0 = vec4<f32>(0.0, 0.0, 0.0, -1.0);
        man.rb0 = vec4<f32>(0.0);
    }
    if (sensor_pair && man.a != EMPTY) {
        var overlaps = contained;
        var witness = man;
        var piece = man;
        let manifold_count = max(man.manifold_link.z, 1u);
        for (var m = 0u; m < manifold_count; m++) {
            for (var j = 0u; j < piece.count; j++) {
                let ra = ra_at(piece, j);
                let rb = rb_at(piece, j);
                if (ra.w + dot(rb.xyz - ra.xyz, piece.n) <= 0.0) {
                    if (!overlaps) { witness = piece; }
                    overlaps = true;
                }
            }
            if (m + 1u < manifold_count) { piece = load_contact(piece.manifold_link.x - 1u); }
        }
        // Sensors expose a pair overlap, not solver manifolds. Reclaim generated
        // children after considering all normals, including speculative-only roots.
        if (manifold_count > 1u) {
            discard_mesh_patch_list(MeshPatchList(man.manifold_link.x - 1u, EMPTY, manifold_count - 1u));
        }
        if (overlaps) { man = witness; man.manifold_link = vec4<u32>(0u); }
        else { man = empty_contact(); }
    }
    // collide_pair finalizes fresh contacts in body COM coordinates exactly
    // once. Recycled contacts above already carry that same representation.
    // Empty pairs must cache COM transforms too: collider positions can differ
    // by local shape/COM offsets and cannot be compared with recycle_skip's bodies.
    store_pair(
        slot,
        packed,
        man,
        shape_a.body_index,
        shape_b.body_index,
        body_a,
        body_b,
    );
}
