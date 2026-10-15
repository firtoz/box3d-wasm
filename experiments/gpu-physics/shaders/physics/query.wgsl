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
    p: vec3<f32>,
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
    return RayCand(vec3<f32>(0.0), vec3<f32>(0.0), 0.0, RAY_TRI_NONE, 0u, 0u, 0u);
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

// Ray kernels use the original translation and return its fraction. Keeping
// fraction coordinates avoids normalizing before transforming to body space.
fn ray_query_inv_rotate(q: vec4<f32>, v: vec3<f32>) -> vec3<f32> {
    return v + 2.0 * gyro_cross(q.xyz, gyro_cross(q.xyz, v) - q.w * v);
}
fn ray_sphere_entry(origin: vec3<f32>, translation: vec3<f32>, center: vec3<f32>, radius: f32, max_fraction: f32) -> RayCand {
    let m = origin - center;
    let ray_length = gyro_sqrt(gyro_dot3(translation, translation));
    if (ray_length == 0.0) { return empty_cand(); }
    let dir = gyro_recip(ray_length) * translation;
    let projection = -gyro_dot3(m, dir);
    let closest = m + projection * dir;
    let gap2 = gyro_dot3(closest, closest);
    let radius2 = radius * radius;
    if (gap2 > radius2) { return empty_cand(); }
    let distance = projection - gyro_sqrt(radius2 - gap2);
    if (distance <= 0.0 || distance > max_fraction * ray_length) { return empty_cand(); }
    var hit = empty_cand();
    // Preserve the distance-based local hit before rounding a segment fraction.
    hit.n = gyro_norm3(m + distance * dir);
    hit.p = center + radius * hit.n;
    hit.t = min(gyro_divide(distance, ray_length), max_fraction);
    return hit;
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
        // Native hull queries divide each slab distance directly.
        var t0 = gyro_divide(-half[a] - o[a], d[a]);
        var t1 = gyro_divide(half[a] - o[a], d[a]);
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

fn ray_capsule(origin: vec3<f32>, dir: vec3<f32>, a: vec3<f32>, b: vec3<f32>, radius: f32, t_max: f32) -> RayCand {
    let ray_length = gyro_sqrt(gyro_dot3(dir, dir));
    if (ray_length == 0.0) { return empty_cand(); }
    let ray_axis = gyro_recip(ray_length) * dir;
    let delta = b - a;
    let axis_length2 = gyro_dot3(delta, delta);
    if (axis_length2 < 2.5e-9) {
        let center = 0.5 * (a + b);
        return ray_sphere_entry(origin, dir, center, radius, t_max);
    }
    let axis_length = gyro_sqrt(axis_length2);
    let axis = delta * (gyro_recip(axis_length));
    let relative = origin - a;
    let u = gyro_dot3(relative, axis);
    let radial = relative - u * axis;
    let radius2 = radius * radius;
    if (gyro_dot3(radial, radial) < radius2) {
        let center = a + clamp(u, 0.0, axis_length) * axis;
        return ray_sphere_entry(origin, dir, center, radius, t_max);
    }
    let alignment = gyro_dot3(axis, ray_axis);
    let end = u + t_max * gyro_dot3(dir, axis);
    if ((u < -radius && end < -radius) || (axis_length + radius < u && axis_length + radius < end)) {
        return empty_cand();
    }
    let determinant = 1.0 - alignment * alignment;
    var t = 0.0;
    if (determinant < 1.1920928955078125e-7) {
        let perpendicular = ray_axis - alignment * axis;
        let beta = gyro_dot3(radial, perpendicular);
        let gamma = gyro_dot3(radial, radial) - radius2;
        let discriminant = beta * beta - gyro_dot3(perpendicular, perpendicular) * gamma;
        if (beta >= 0.0 || discriminant < 0.0) { return empty_cand(); }
        t = gyro_divide(gamma, -beta + gyro_sqrt(discriminant));
    } else {
        // Solve near the closest approach of the ray and capsule axes.
        let inv = gyro_recip(determinant);
        let projection = gyro_dot3(relative, ray_axis);
        let axis_distance = (u - alignment * projection) * inv;
        let ray_distance = (alignment * u - projection) * inv;
        let gap = relative + ray_distance * ray_axis - axis_distance * axis;
        let gap2 = gyro_dot3(gap, gap);
        if (gap2 > radius2) { return empty_cand(); }
        t = ray_distance - gyro_sqrt((radius2 - gap2) * inv);
    }
    if (t <= 0.0 || t > t_max * ray_length) { return empty_cand(); }
    let on_axis = u + t * alignment;
    if (on_axis < 0.0 || on_axis > axis_length) {
        let center = select(b, a, on_axis < 0.0);
        return ray_sphere_entry(origin, dir, center, radius, t_max);
    }
    let point = relative + t * ray_axis;
    var hit = empty_cand();
    hit.n = gyro_norm3(point - on_axis * axis);
    hit.p = a + point;
    hit.t = min(gyro_divide(t, ray_length), t_max);
    return hit;
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
        let offset = pl.w + gyro_dot3(nw, xf_p);
        let denom = gyro_dot3(nw, dir);
        let dist = offset - gyro_dot3(nw, origin);
        if (abs(denom) < 1e-12) {
            if (dist < 0.0) {
                return vec4<f32>(0.0);
            }
            continue;
        }
        let t = gyro_divide(dist, denom);
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
    // Test the ray against oriented edges before reconstructing a hit point.
    // The rounded point can lie off a valid triangle on long rays.
    let v1 = gyro_dot3(gyro_cross(c - b, 0.5 * (b + c) - origin), dir);
    let v2 = gyro_dot3(gyro_cross(a - c, 0.5 * (c + a) - origin), dir);
    let v3 = gyro_dot3(gyro_cross(b - a, 0.5 * (a + b) - origin), dir);
    if (v1 < 0.0 || v2 < 0.0 || v3 < 0.0) { return vec4<f32>(0.0); }
    let n = gyro_cross(b - a, c - a);
    let denominator = gyro_dot3(n, dir);
    if (denominator >= 0.0) { return vec4<f32>(0.0); }
    // Native division rounding matters before long-ray point reconstruction.
    let fraction = gyro_divide(gyro_dot3(n, a - origin), denominator);
    if (fraction <= 0.0 || fraction >= t_max) { return vec4<f32>(0.0); }
    return vec4<f32>(gyro_norm3(n), fraction);
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

fn ray_query_contains(shape: Shape, point: vec3<f32>) -> bool {
    if (shape.kind == KIND_SPHERE) {
        let d = point - shape.local_center;
        return gyro_dot3(d, d) < shape.half.x * shape.half.x;
    }
    if (shape.kind == KIND_CAPSULE) {
        let a = capsule_local_point(shape, 0u);
        let delta = capsule_local_point(shape, 1u) - a;
        let length2 = gyro_dot3(delta, delta);
        let along = clamp(gyro_dot3(point - a, delta) / max(length2, 1e-30), 0.0, 1.0);
        let d = point - (a + along * delta);
        return gyro_dot3(d, d) < shape.half.x * shape.half.x;
    }
    if (shape.kind == KIND_BOX) { return all(abs(point - shape.local_center) <= shape.half); }
    if (shape.kind == KIND_CONVEX_HULL) {
        let count = hull_plane_count(shape);
        if (count == 0u) { return false; }
        for (var i = 0u; i < count; i++) {
            let plane = load_hull_plane(shape, i);
            if (gyro_dot3(plane.xyz, point) > plane.w) { return false; }
        }
        return true;
    }
    return false;
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
    let xf_q = body.rot;
    var xf_p = body_origin(body, shape.body_index);
    let coordinate_origin = vec3<f32>(q_f32(11u), q_f32(15u), q_f32(31u));
    // Keep zero axes bit-identical; round nonzero axes exactly as public getters.
    for (var axis = 0u; axis < 3u; axis++) {
        if (coordinate_origin[axis] != 0.0) { xf_p[axis] += coordinate_origin[axis]; }
    }
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
    let local_origin = ray_query_inv_rotate(xf_q, origin - xf_p);
    let local_dir = ray_query_inv_rotate(xf_q, dir);
    if ((shape.event_flags & SHAPE_COMPOUND_CHILD) != 0u) {
        let parent_index = shape._pad_filter.x;
        let parent = load_shape(parent_index);
        if (all(abs(local_origin - parent.local_center) <= parent.half)) {
            // CPU closest callbacks ignore the whole parent when its first
            // child hit is initial overlap, not just the overlapped child.
            for (var sibling = 0u; sibling < params.shape_count; sibling++) {
                let candidate = load_shape(sibling);
                if ((candidate.event_flags & SHAPE_COMPOUND_CHILD) != 0u
                    && candidate._pad_filter.x == parent_index
                    && ray_query_contains(candidate, local_origin)) { return out; }
            }
        }
    }
    out.mat_lo = shape.user_material_id.x;
    out.mat_hi = shape.user_material_id.y;
    if (shape.kind == KIND_SPHERE) {
        let hit = ray_sphere_entry(local_origin, local_dir, shape.local_center, shape.half.x, t_max);
        out.n = hit.n;
        out.p = hit.p;
        out.t = hit.t;
    } else if (shape.kind == KIND_BOX) {
        let hit = ray_obb(local_origin, local_dir, vec4<f32>(0.0, 0.0, 0.0, 1.0), shape.local_center, shape.half, t_max);
        out.n = hit.xyz;
        out.t = hit.w;
    } else if (shape.kind == KIND_CAPSULE) {
        let a = capsule_local_point(shape, 0u);
        let b = capsule_local_point(shape, 1u);
        let hit = ray_capsule(local_origin, local_dir, a, b, shape.half.x, t_max);
        out.n = hit.n;
        out.p = hit.p;
        out.t = hit.t;
    } else if (shape.kind == KIND_CONVEX_HULL) {
        let hit = ray_hull(local_origin, local_dir, vec3<f32>(0.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), shape, t_max);
        out.n = hit.xyz;
        out.t = hit.w;
    } else if (shape.kind == KIND_MESH) {
        out = ray_mesh(local_origin, local_dir, vec3<f32>(0.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), shape, t_max);
    }
    if (out.t > 0.0) {
        var point = local_origin + out.t * local_dir;
        if (shape.kind == KIND_SPHERE || shape.kind == KIND_CAPSULE) { point = out.p; }
        // Match the native world query's ray-relative output transform order.
        out.p = origin + ((xf_p - origin) + quat_rotate(xf_q, point));
        out.n = quat_rotate(xf_q, out.n);
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
    let dir = translation;
    let t_max = q_f32(Q_MAX);
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
    let fraction = hit.t;
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
    store_q_f32(Q_POINT, hit.p.x);
    store_q_f32(Q_POINT + 1u, hit.p.y);
    store_q_f32(Q_POINT + 2u, hit.p.z);
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
