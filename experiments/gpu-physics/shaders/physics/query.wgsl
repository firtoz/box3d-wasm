const Q_OX: u32 = 0u;
const Q_TX: u32 = 3u;
const Q_CAT: u32 = 6u;
const Q_MASK: u32 = 8u;
const Q_MAX: u32 = 10u;
const Q_FRAC: u32 = 12u;
const Q_WIN: u32 = 13u;
const Q_OVERFLOW: u32 = 14u;
const Q_HIT: u32 = 16u;
const Q_SHAPE: u32 = 17u;
const Q_POINT: u32 = 18u;
const Q_NORMAL: u32 = 21u;
const Q_FRAC_OUT: u32 = 24u;
const Q_BODY: u32 = 25u;
const Q_MAT: u32 = 26u;
const Q_CPU: u32 = 28u;
const Q_TRI: u32 = 29u;
const Q_OVERFLOW_OUT: u32 = 30u;
const RAY_TRI_NONE: u32 = 0xffffu;
const MESH_VISIT_CAP: u32 = 4096u;
const MESH_STACK_CAP: u32 = 64u;

struct RayCand {
    n: vec3<f32>,
    t: f32,
    tri: u32,
    mat_lo: u32,
    mat_hi: u32,
    overflow: u32,
}

fn q_f32(i: u32) -> f32 {
    return bitcast<f32>(atomicLoad(&query[i]));
}

fn store_q_f32(i: u32, v: f32) {
    atomicStore(&query[i], bitcast<u32>(v));
}

fn ray_finite3(v: vec3<f32>) -> bool {
    return all(v == v) && all(abs(v) < vec3<f32>(1e20));
}

fn query_bits_hit(shape_lo: u32, shape_hi: u32, query_lo: u32, query_hi: u32) -> bool {
    return ((shape_lo & query_lo) | (shape_hi & query_hi)) != 0u;
}

fn winner_key(shape: u32, tri: u32) -> u32 {
    return (shape << 16u) | (tri & RAY_TRI_NONE);
}

fn empty_cand() -> RayCand {
    return RayCand(vec3<f32>(0.0), 0.0, RAY_TRI_NONE, 0u, 0u, 0u);
}

fn ray_aabb_hit(origin: vec3<f32>, dir: vec3<f32>, t_max: f32, lo: vec3<f32>, hi: vec3<f32>) -> bool {
    var tmin = 0.0;
    var tmax = t_max;
    for (var a = 0u; a < 3u; a++) {
        if (abs(dir[a]) < 1e-12) {
            if (origin[a] < lo[a] || origin[a] > hi[a]) {
                return false;
            }
            continue;
        }
        let invd = 1.0 / dir[a];
        var t0 = (lo[a] - origin[a]) * invd;
        var t1 = (hi[a] - origin[a]) * invd;
        if (t0 > t1) {
            let tmp = t0;
            t0 = t1;
            t1 = tmp;
        }
        tmin = max(tmin, t0);
        tmax = min(tmax, t1);
        if (tmax < tmin) {
            return false;
        }
    }
    return true;
}

fn ray_sphere_entry(origin: vec3<f32>, dir: vec3<f32>, center: vec3<f32>, radius: f32, t_max: f32) -> vec2<f32> {
    let m = origin - center;
    let b = dot(m, dir);
    let c = dot(m, m) - radius * radius;
    let disc = b * b - c;
    if (disc < 0.0) {
        return vec2<f32>(-1.0, 0.0);
    }
    let t = -b - sqrt(disc);
    if (t <= 0.0 || t > t_max) {
        return vec2<f32>(-1.0, 0.0);
    }
    return vec2<f32>(t, 1.0);
}

fn ray_obb(origin: vec3<f32>, dir: vec3<f32>, rot: vec4<f32>, center: vec3<f32>, half: vec3<f32>, t_max: f32) -> vec4<f32> {
    let inv = quat_inv(rot);
    let o = quat_rotate(inv, origin - center);
    let d = quat_rotate(inv, dir);
    var tmin = 0.0;
    var tmax = t_max;
    var n = vec3<f32>(0.0);
    for (var a = 0u; a < 3u; a++) {
        if (abs(d[a]) < 1e-12) {
            if (o[a] < -half[a] || o[a] > half[a]) {
                return vec4<f32>(0.0);
            }
            continue;
        }
        let invd = 1.0 / d[a];
        var t0 = (-half[a] - o[a]) * invd;
        var t1 = (half[a] - o[a]) * invd;
        var n0 = -1.0;
        var n1 = 1.0;
        if (t0 > t1) {
            let tmp = t0;
            t0 = t1;
            t1 = tmp;
            let tn = n0;
            n0 = n1;
            n1 = tn;
        }
        if (t0 > tmin) {
            tmin = t0;
            n = vec3<f32>(0.0);
            n[a] = n0;
        }
        tmax = min(tmax, t1);
        if (tmax < tmin) {
            return vec4<f32>(0.0);
        }
    }
    if (tmin <= 0.0 || tmin > t_max) {
        return vec4<f32>(0.0);
    }
    let wn = quat_rotate(rot, n);
    let len = length(wn);
    if (len < 1e-12) {
        return vec4<f32>(0.0);
    }
    return vec4<f32>(wn / len, tmin);
}

fn ray_capsule(origin: vec3<f32>, dir: vec3<f32>, a: vec3<f32>, b: vec3<f32>, radius: f32, t_max: f32) -> vec4<f32> {
    let ba = b - a;
    let oa = origin - a;
    let baba = dot(ba, ba);
    let bard = dot(ba, dir);
    let baoa = dot(ba, oa);
    let rdoa = dot(dir, oa);
    let oaoa = dot(oa, oa);
    let a2 = baba - bard * bard;
    let b2 = baba * rdoa - baoa * bard;
    let c2 = baba * oaoa - baoa * baoa - radius * radius * baba;
    if (abs(a2) < 1e-12 || baba < 1e-12) {
        let sa = ray_sphere_entry(origin, dir, a, radius, t_max);
        let sb = ray_sphere_entry(origin, dir, b, radius, t_max);
        var t = t_max + 1.0;
        var c = a;
        var hit = false;
        if (sa.y > 0.0) {
            t = sa.x;
            c = a;
            hit = true;
        }
        if (sb.y > 0.0 && sb.x < t) {
            t = sb.x;
            c = b;
            hit = true;
        }
        if (!hit) {
            return vec4<f32>(0.0);
        }
        let p = origin + t * dir;
        let n = p - c;
        let ln = length(n);
        if (ln < 1e-12) {
            return vec4<f32>(0.0);
        }
        return vec4<f32>(n / ln, t);
    }
    let disc = b2 * b2 - a2 * c2;
    if (disc < 0.0) {
        return vec4<f32>(0.0);
    }
    let t = (-b2 - sqrt(disc)) / a2;
    let y = baoa + t * bard;
    if (t > 0.0 && t <= t_max && y > 0.0 && y < baba) {
        let p = origin + t * dir;
        let q = a + ba * (y / baba);
        let n = p - q;
        let ln = length(n);
        if (ln < 1e-12) {
            return vec4<f32>(0.0);
        }
        return vec4<f32>(n / ln, t);
    }
    let cap = select(b, a, y <= 0.0);
    let sph = ray_sphere_entry(origin, dir, cap, radius, t_max);
    if (sph.y <= 0.0) {
        return vec4<f32>(0.0);
    }
    let p = origin + sph.x * dir;
    let n = p - cap;
    let ln = length(n);
    if (ln < 1e-12) {
        return vec4<f32>(0.0);
    }
    return vec4<f32>(n / ln, sph.x);
}

fn ray_hull(origin: vec3<f32>, dir: vec3<f32>, xf_p: vec3<f32>, xf_q: vec4<f32>, shape: Shape, t_max: f32) -> vec4<f32> {
    let nplane = hull_plane_count(shape);
    if (nplane == 0u) {
        return vec4<f32>(0.0);
    }
    var t_enter = 0.0;
    var t_leave = t_max;
    var n_enter = vec3<f32>(0.0);
    for (var i = 0u; i < nplane; i++) {
        let pl = load_hull_plane(shape, i);
        let nw = quat_rotate(xf_q, pl.xyz);
        let offset = pl.w + dot(nw, xf_p);
        let denom = dot(nw, dir);
        let dist = offset - dot(nw, origin);
        if (abs(denom) < 1e-12) {
            if (dist < 0.0) {
                return vec4<f32>(0.0);
            }
            continue;
        }
        let t = dist / denom;
        if (denom < 0.0) {
            if (t > t_enter) {
                t_enter = t;
                n_enter = nw;
            }
        } else {
            t_leave = min(t_leave, t);
        }
        if (t_leave < t_enter) {
            return vec4<f32>(0.0);
        }
    }
    if (t_enter <= 0.0 || t_enter > t_max) {
        return vec4<f32>(0.0);
    }
    let ln = length(n_enter);
    if (ln < 1e-12) {
        return vec4<f32>(0.0);
    }
    return vec4<f32>(n_enter / ln, t_enter);
}

fn ray_triangle(origin: vec3<f32>, dir: vec3<f32>, a: vec3<f32>, b: vec3<f32>, c: vec3<f32>, t_max: f32) -> vec4<f32> {
    let ab = b - a;
    let ac = c - a;
    let n = cross(ab, ac);
    let denom = dot(n, dir);
    if (denom >= -1e-12) {
        return vec4<f32>(0.0);
    }
    let t = dot(n, a - origin) / denom;
    if (t <= 0.0 || t > t_max) {
        return vec4<f32>(0.0);
    }
    let p = origin + t * dir;
    let ap = p - a;
    let d00 = dot(ab, ab);
    let d01 = dot(ab, ac);
    let d11 = dot(ac, ac);
    let d20 = dot(ap, ab);
    let d21 = dot(ap, ac);
    let inv = 1.0 / (d00 * d11 - d01 * d01);
    let v = (d11 * d20 - d01 * d21) * inv;
    let w = (d00 * d21 - d01 * d20) * inv;
    let u = 1.0 - v - w;
    if (u < -1e-5 || v < -1e-5 || w < -1e-5) {
        return vec4<f32>(0.0);
    }
    let ln = length(n);
    if (ln < 1e-12) {
        return vec4<f32>(0.0);
    }
    return vec4<f32>(n / ln, t);
}

fn mesh_world_vertex(shape: Shape, xf_p: vec3<f32>, xf_q: vec4<f32>, index: u32) -> vec3<f32> {
    let local = load_mesh_vertex(shape, index);
    return xf_p + quat_rotate(xf_q, local);
}

fn ray_mesh(origin: vec3<f32>, dir: vec3<f32>, xf_p: vec3<f32>, xf_q: vec4<f32>, shape: Shape, t_max: f32) -> RayCand {
    var best = empty_cand();
    let inv = quat_inv(xf_q);
    let o_local = mesh_to_raw_point(shape, quat_rotate(inv, origin - xf_p));
    let d_local = mesh_to_raw_vector(shape, quat_rotate(inv, dir));
    var stack: array<u32, 64>;
    var stack_count = 0u;
    if (shape.topology_counts == 0u) {
        let ntri = shape.topology_slot;
        if (ntri > MESH_VISIT_CAP) {
            best.overflow = 1u;
            return best;
        }
        for (var i = 0u; i < ntri; i++) {
            let tri = load_mesh_triangle(shape, i);
            let p1 = mesh_world_vertex(shape, xf_p, xf_q, tri.x);
            let p2 = mesh_world_vertex(shape, xf_p, xf_q, tri.y);
            let p3 = mesh_world_vertex(shape, xf_p, xf_q, tri.z);
            let hit = ray_triangle(origin, dir, p1, p2, p3, select(best.t, t_max, best.t <= 0.0));
            if (hit.w > 0.0 && (best.t <= 0.0 || hit.w < best.t)) {
                best.n = hit.xyz;
                best.t = hit.w;
                best.tri = i;
                let mat = load_surface_material(shape, tri.w >> 8u);
                best.mat_lo = mat.user_material_id.x;
                best.mat_hi = mat.user_material_id.y;
            }
        }
        return best;
    }
    stack[0] = 0u;
    stack_count = 1u;
    var visits = 0u;
    loop {
        if (stack_count == 0u || visits >= MESH_VISIT_CAP) {
            if (visits >= MESH_VISIT_CAP && stack_count > 0u) {
                best.overflow = 1u;
            }
            break;
        }
        visits = visits + 1u;
        stack_count = stack_count - 1u;
        let node_index = stack[stack_count];
        if (node_index >= shape.topology_counts) {
            continue;
        }
        let lower = load_mesh_node_lower(shape, node_index);
        let upper = load_mesh_node_upper(shape, node_index);
        if (!ray_aabb_hit(o_local, d_local, t_max, lower.xyz, upper.xyz)) {
            continue;
        }
        let data = bitcast<u32>(lower.w);
        if ((data & 3u) != 3u) {
            let right = node_index + (data >> 2u);
            if (stack_count + 2u > MESH_STACK_CAP) {
                best.overflow = 1u;
                continue;
            }
            stack[stack_count] = right;
            stack[stack_count + 1u] = node_index + 1u;
            stack_count = stack_count + 2u;
            continue;
        }
        let triangle_count = data >> 2u;
        let triangle_offset = bitcast<u32>(upper.w);
        for (var leaf_index = 0u; leaf_index < triangle_count; leaf_index++) {
            let triangle_index = triangle_offset + leaf_index;
            if (triangle_index >= shape.topology_slot) {
                continue;
            }
            let tri = load_mesh_triangle(shape, triangle_index);
            let p1 = mesh_world_vertex(shape, xf_p, xf_q, tri.x);
            let p2 = mesh_world_vertex(shape, xf_p, xf_q, tri.y);
            let p3 = mesh_world_vertex(shape, xf_p, xf_q, tri.z);
            let tmax = select(best.t, t_max, best.t <= 0.0);
            let hit = ray_triangle(origin, dir, p1, p2, p3, tmax);
            if (hit.w > 0.0 && (best.t <= 0.0 || hit.w < best.t)) {
                best.n = hit.xyz;
                best.t = hit.w;
                best.tri = triangle_index;
                let mat = load_surface_material(shape, tri.w >> 8u);
                best.mat_lo = mat.user_material_id.x;
                best.mat_hi = mat.user_material_id.y;
            }
        }
    }
    return best;
}

fn ray_shape_hit(i: u32, origin: vec3<f32>, dir: vec3<f32>, t_max: f32) -> RayCand {
    var out = empty_cand();
    let shape = load_shape(i);
    if ((shape.event_flags & SHAPE_PUBLIC_PROXY) != 0u) { return out; }
    let cat_lo = atomicLoad(&query[Q_CAT]);
    let cat_hi = atomicLoad(&query[Q_CAT + 1u]);
    let mask_lo = atomicLoad(&query[Q_MASK]);
    let mask_hi = atomicLoad(&query[Q_MASK + 1u]);
    if (!query_bits_hit(shape.category_bits_lo, shape.category_bits_hi, mask_lo, mask_hi)
        || !query_bits_hit(shape.mask_bits_lo, shape.mask_bits_hi, cat_lo, cat_hi)) {
        return out;
    }
    if (shape.body_index >= params.body_count) {
        return out;
    }
    let body = load_body(shape.body_index);
    if ((body.flags & FLAG_DISABLED) != 0u) {
        return out;
    }
    let xf_q = normalize(body.rot);
    let xf_p = body_origin(body, shape.body_index);
    let center = xf_p + quat_rotate(xf_q, shape.local_center);
    var lo = vec3<f32>(0.0);
    var hi = vec3<f32>(0.0);
    if (shape.kind == KIND_SPHERE) {
        let r = vec3<f32>(shape.half.x);
        lo = center - r;
        hi = center + r;
    } else if (shape.kind == KIND_CAPSULE) {
        let a = xf_p + quat_rotate(xf_q, capsule_local_point(shape, 0u));
        let b = xf_p + quat_rotate(xf_q, capsule_local_point(shape, 1u));
        let r = vec3<f32>(shape.half.x);
        lo = min(a, b) - r;
        hi = max(a, b) + r;
    } else if (shape.kind == KIND_CONVEX_HULL) {
        let n = hull_point_count(shape);
        if (n == 0u) {
            return out;
        }
        var p0 = xf_p + quat_rotate(xf_q, load_hull_point(shape, 0u));
        lo = p0;
        hi = p0;
        for (var k = 1u; k < n; k++) {
            let p = xf_p + quat_rotate(xf_q, load_hull_point(shape, k));
            lo = min(lo, p);
            hi = max(hi, p);
        }
    } else if (shape.kind == KIND_MESH) {
        if (shape.topology_counts == 0u) {
            if (shape.topology_slot == 0u) {
                return out;
            }
        } else {
            let local_lo = load_mesh_node_lower(shape, 0u).xyz;
            let local_hi = load_mesh_node_upper(shape, 0u).xyz;
            lo = vec3<f32>(1.0e30);
            hi = vec3<f32>(-1.0e30);
            for (var c = 0u; c < 8u; c++) {
                let lp = vec3<f32>(
                    select(local_lo.x, local_hi.x, (c & 1u) != 0u),
                    select(local_lo.y, local_hi.y, (c & 2u) != 0u),
                    select(local_lo.z, local_hi.z, (c & 4u) != 0u),
                );
                let p = xf_p + quat_rotate(xf_q, mesh_to_body_point(shape, lp));
                lo = min(lo, p);
                hi = max(hi, p);
            }
        }
    } else {
        let x = abs(quat_rotate(xf_q, vec3<f32>(shape.half.x, 0.0, 0.0)));
        let y = abs(quat_rotate(xf_q, vec3<f32>(0.0, shape.half.y, 0.0)));
        let z = abs(quat_rotate(xf_q, vec3<f32>(0.0, 0.0, shape.half.z)));
        let e = x + y + z;
        lo = center - e;
        hi = center + e;
    }
    if (!(shape.kind == KIND_MESH && shape.topology_counts == 0u) && !ray_aabb_hit(origin, dir, t_max, lo, hi)) {
        return out;
    }
    out.mat_lo = shape.user_material_id.x;
    out.mat_hi = shape.user_material_id.y;
    if (shape.kind == KIND_SPHERE) {
        let sph = ray_sphere_entry(origin, dir, center, shape.half.x, t_max);
        if (sph.y > 0.0) {
            let p = origin + sph.x * dir;
            let n = p - center;
            let ln = length(n);
            if (ln >= 1e-12) {
                out.n = n / ln;
                out.t = sph.x;
            }
        }
    } else if (shape.kind == KIND_BOX) {
        let hit = ray_obb(origin, dir, xf_q, center, shape.half, t_max);
        out.n = hit.xyz;
        out.t = hit.w;
    } else if (shape.kind == KIND_CAPSULE) {
        let a = xf_p + quat_rotate(xf_q, capsule_local_point(shape, 0u));
        let b = xf_p + quat_rotate(xf_q, capsule_local_point(shape, 1u));
        let hit = ray_capsule(origin, dir, a, b, shape.half.x, t_max);
        out.n = hit.xyz;
        out.t = hit.w;
    } else if (shape.kind == KIND_CONVEX_HULL) {
        let hit = ray_hull(origin, dir, xf_p, xf_q, shape, t_max);
        out.n = hit.xyz;
        out.t = hit.w;
    } else if (shape.kind == KIND_MESH) {
        out = ray_mesh(origin, dir, xf_p, xf_q, shape, t_max);
    }
    return out;
}

fn ray_pass(gid: u32, stage: u32) {
    let origin = vec3<f32>(q_f32(Q_OX), q_f32(Q_OX + 1u), q_f32(Q_OX + 2u));
    let translation = vec3<f32>(q_f32(Q_TX), q_f32(Q_TX + 1u), q_f32(Q_TX + 2u));
    if (!ray_finite3(origin) || !ray_finite3(translation)) {
        return;
    }
    let t_len = length(translation);
    if (t_len < 1e-12) {
        return;
    }
    let dir = translation / t_len;
    let t_max = q_f32(Q_MAX) * t_len;
    if (gid >= params.shape_count) {
        return;
    }
    let hit = ray_shape_hit(gid, origin, dir, t_max);
    if (hit.overflow != 0u) {
        atomicOr(&query[Q_OVERFLOW], 1u);
    }
    if (hit.t <= 0.0) {
        return;
    }
    let fraction = hit.t / t_len;
    if (fraction <= 0.0 || fraction > q_f32(Q_MAX) || !(fraction == fraction)) {
        return;
    }
    let key = bitcast<u32>(fraction);
    let packed = winner_key(load_shape(gid)._pad_filter.y, hit.tri);
    if (stage == 0u) {
        atomicMin(&query[Q_FRAC], key);
        return;
    }
    if (key != atomicLoad(&query[Q_FRAC])) {
        return;
    }
    if (stage == 1u) {
        atomicMin(&query[Q_WIN], packed);
        return;
    }
    if (packed != atomicLoad(&query[Q_WIN])) {
        return;
    }
    let shape = load_shape(gid);
    atomicStore(&query[Q_HIT], 1u);
    atomicStore(&query[Q_SHAPE], gid);
    atomicStore(&query[Q_CPU], shape._pad_filter.y);
    atomicStore(&query[Q_BODY], shape.body_index);
    atomicStore(&query[Q_MAT], hit.mat_lo);
    atomicStore(&query[Q_MAT + 1u], hit.mat_hi);
    store_q_f32(Q_POINT, origin.x + hit.t * dir.x);
    store_q_f32(Q_POINT + 1u, origin.y + hit.t * dir.y);
    store_q_f32(Q_POINT + 2u, origin.z + hit.t * dir.z);
    store_q_f32(Q_NORMAL, hit.n.x);
    store_q_f32(Q_NORMAL + 1u, hit.n.y);
    store_q_f32(Q_NORMAL + 2u, hit.n.z);
    store_q_f32(Q_FRAC_OUT, fraction);
    atomicStore(&query[Q_TRI], hit.tri);
    atomicStore(&query[Q_OVERFLOW_OUT], atomicLoad(&query[Q_OVERFLOW]));
}

@compute @workgroup_size(64)
fn ray_closest(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    ray_pass(gid.x, 0u);
}

@compute @workgroup_size(64)
fn ray_closest_pick(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    ray_pass(gid.x, 1u);
}

@compute @workgroup_size(64)
fn ray_closest_commit(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    if (gid.x == 0u) {
        atomicStore(&query[Q_OVERFLOW_OUT], atomicLoad(&query[Q_OVERFLOW]));
    }
    ray_pass(gid.x, 2u);
}
