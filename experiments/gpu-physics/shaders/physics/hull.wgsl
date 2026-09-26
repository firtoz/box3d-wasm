// Box3D `s_boxHull` topology (8 verts, 24 half-edges, 6 faces).

const BOX_PT: array<vec3<f32>, 8> = array<vec3<f32>, 8>(
    vec3<f32>(1.0, 1.0, 1.0),
    vec3<f32>(-1.0, 1.0, 1.0),
    vec3<f32>(-1.0, -1.0, 1.0),
    vec3<f32>(1.0, -1.0, 1.0),
    vec3<f32>(1.0, 1.0, -1.0),
    vec3<f32>(-1.0, 1.0, -1.0),
    vec3<f32>(-1.0, -1.0, -1.0),
    vec3<f32>(1.0, -1.0, -1.0),
);

const BOX_FACE_N: array<vec3<f32>, 6> = array<vec3<f32>, 6>(
    vec3<f32>(-1.0, 0.0, 0.0),
    vec3<f32>(1.0, 0.0, 0.0),
    vec3<f32>(0.0, -1.0, 0.0),
    vec3<f32>(0.0, 1.0, 0.0),
    vec3<f32>(0.0, 0.0, -1.0),
    vec3<f32>(0.0, 0.0, 1.0),
);

const BOX_EDGE_NEXT: array<u32, 24> = array<u32, 24>(
    2u, 17u, 4u, 20u, 6u, 23u, 0u, 18u, 10u, 21u, 12u, 16u,
    14u, 19u, 8u, 22u, 7u, 9u, 11u, 5u, 15u, 1u, 3u, 13u,
);
const BOX_EDGE_TWIN: array<u32, 24> = array<u32, 24>(
    1u, 0u, 3u, 2u, 5u, 4u, 7u, 6u, 9u, 8u, 11u, 10u,
    13u, 12u, 15u, 14u, 17u, 16u, 19u, 18u, 21u, 20u, 23u, 22u,
);
const BOX_EDGE_ORIGIN: array<u32, 24> = array<u32, 24>(
    2u, 1u, 1u, 5u, 5u, 6u, 6u, 2u, 0u, 3u, 3u, 7u,
    7u, 4u, 4u, 0u, 3u, 2u, 6u, 7u, 1u, 0u, 4u, 5u,
);
const BOX_EDGE_FACE: array<u32, 24> = array<u32, 24>(
    0u, 5u, 0u, 3u, 0u, 4u, 0u, 2u, 1u, 5u, 1u, 2u,
    1u, 4u, 1u, 3u, 2u, 5u, 2u, 4u, 3u, 5u, 3u, 4u,
);
const BOX_FACE_EDGE: array<u32, 6> = array<u32, 6>(0u, 8u, 16u, 20u, 19u, 21u);
const BOX_VERT_EDGE: array<u32, 8> = array<u32, 8>(8u, 1u, 0u, 9u, 13u, 3u, 5u, 11u);
const AABB_MARGIN_FRAC: f32 = 0.125;
const MAX_AABB_MARGIN: f32 = 0.05;
const PARALLEL_EDGE_TOL: f32 = 0.005;

fn box_point_local(b: Body, i: u32) -> vec3<f32> {
    return BOX_PT[i] * b.half;
}

fn box_point(b: Body, i: u32) -> vec3<f32> {
    return b.pos + quat_rotate(b.rot, box_point_local(b, i));
}

fn to_body_local(b: Body, world: vec3<f32>) -> vec3<f32> {
    return quat_inv_rotate(b.rot, world - b.pos);
}

fn from_body_local(b: Body, local: vec3<f32>) -> vec3<f32> {
    return b.pos + quat_rotate(b.rot, local);
}

fn hull_face_n(b: Body, face: u32) -> vec3<f32> {
    return quat_rotate(b.rot, BOX_FACE_N[face]);
}

fn hull_face_half(b: Body, face: u32) -> f32 {
    let ax = face / 2u;
    if (ax == 1u) {
        return b.half.y;
    }
    if (ax == 2u) {
        return b.half.z;
    }
    return b.half.x;
}

fn box_aabb_extent(b: Body) -> vec3<f32> {
    return abs(box_axis(b, 0u)) * b.half.x
        + abs(box_axis(b, 1u)) * b.half.y
        + abs(box_axis(b, 2u)) * b.half.z;
}

fn body_aabb_pad(b: Body) -> f32 {
    // Box3D tight AABB includes speculative; fat AABB adds per-shape margin on dynamics.
    if (is_static(b)) {
        return SPECULATIVE;
    }
    return SPECULATIVE + min(MAX_AABB_MARGIN, AABB_MARGIN_FRAC * bound_radius(b));
}

fn collider_aabb_extent(b: Body) -> vec3<f32> {
    if (b.kind == KIND_BOX || b.kind == KIND_MESH || b.kind == KIND_CONVEX_HULL) {
        return box_aabb_extent(b);
    }
    if (b.kind == KIND_SPHERE) {
        return vec3<f32>(b.half.x);
    }
    return vec3<f32>(b.half.x + b.half.y);
}

fn aabb_overlap(a: Body, b: Body) -> bool {
    return all(fat_lower(a) <= fat_upper(b)) && all(fat_lower(b) <= fat_upper(a));
}

fn to_a(a: Body, p: vec3<f32>) -> vec3<f32> {
    return quat_inv_rotate(a.rot, p - a.pos);
}

fn dir_to_a(a: Body, d: vec3<f32>) -> vec3<f32> {
    return quat_inv_rotate(a.rot, d);
}

fn box_support_vertex(b: Body, dir: vec3<f32>) -> u32 {
    var best = 0u;
    var best_d = -1e9;
    for (var i = 0u; i < 8u; i++) {
        let s = dot(box_point(b, i), dir);
        if (s > best_d) {
            best_d = s;
            best = i;
        }
    }
    return best;
}

fn find_incident_face(b: Body, ref_n: vec3<f32>, vertex: u32) -> u32 {
    var min_edge = BOX_VERT_EDGE[vertex];
    var min_proj = 1e9;
    var edge_i = BOX_VERT_EDGE[vertex];
    let origin_p = box_point(b, vertex);
    for (var k = 0u; k < 8u; k++) {
        let twin_i = BOX_EDGE_TWIN[edge_i];
        let twin_o = BOX_EDGE_ORIGIN[twin_i];
        let axis = box_point(b, twin_o) - origin_p;
        let al = length(axis);
        if (al > 1e-12) {
            let proj = abs(dot(axis / al, ref_n));
            if (proj < min_proj) {
                min_proj = proj;
                min_edge = edge_i;
            }
        }
        edge_i = BOX_EDGE_NEXT[twin_i];
        if (edge_i == BOX_VERT_EDGE[vertex]) {
            break;
        }
    }
    let f1 = BOX_EDGE_FACE[min_edge];
    let f2 = BOX_EDGE_FACE[BOX_EDGE_TWIN[min_edge]];
    if (dot(hull_face_n(b, f1), ref_n) < dot(hull_face_n(b, f2), ref_n)) {
        return f1;
    }
    return f2;
}

fn hull_face_verts(b: Body, face: u32) -> array<vec3<f32>, 4> {
    var outv: array<vec3<f32>, 4>;
    var e = BOX_FACE_EDGE[face];
    for (var i = 0u; i < 4u; i++) {
        let nxt = BOX_EDGE_NEXT[e];
        // Box3D `b3BuildPolygon`: vertex at next->origin.
        outv[i] = box_point(b, BOX_EDGE_ORIGIN[nxt]);
        e = nxt;
    }
    return outv;
}

fn hull_face_edge_ids(face: u32) -> array<u32, 4> {
    var ids: array<u32, 4>;
    var e = BOX_FACE_EDGE[face];
    for (var i = 0u; i < 4u; i++) {
        ids[i] = e;
        e = BOX_EDGE_NEXT[e];
    }
    return ids;
}

struct EdgeQuery {
    n: vec3<f32>,
    sep: f32,
    index_a: u32,
    index_b: u32,
}

// Box3D `b3QueryEdgeDirections`: Gauss-map test in A, axis from B→A, store A→B.
fn gauss_edge_query(a: Body, b: Body) -> EdgeQuery {
    var q: EdgeQuery;
    q.n = vec3<f32>(0.0, 1.0, 0.0);
    q.sep = -1e9;
    q.index_a = EMPTY;
    q.index_b = EMPTY;
    let squared_tol = PARALLEL_EDGE_TOL * PARALLEL_EDGE_TOL;
    for (var i = 0u; i < 12u; i++) {
        let ea = 2u * i;
        let ta = BOX_EDGE_TWIN[ea];
        let qA = to_a(a, box_point(a, BOX_EDGE_ORIGIN[ta]));
        let eA = qA - to_a(a, box_point(a, BOX_EDGE_ORIGIN[ea]));
        let uA = BOX_FACE_N[BOX_EDGE_FACE[ea]];
        let vA = BOX_FACE_N[BOX_EDGE_FACE[ta]];
        for (var j = 0u; j < 12u; j++) {
            let eb = 2u * j;
            let tb = BOX_EDGE_TWIN[eb];
            let qB = to_a(a, box_point(b, BOX_EDGE_ORIGIN[tb]));
            let eB = qB - to_a(a, box_point(b, BOX_EDGE_ORIGIN[eb]));
            let uB = dir_to_a(a, hull_face_n(b, BOX_EDGE_FACE[eb]));
            let vB = dir_to_a(a, hull_face_n(b, BOX_EDGE_FACE[tb]));
            let cba = dot(uB, eA);
            let dba = dot(vB, eA);
            let adc = -dot(uA, eB);
            let bdc = -dot(vA, eB);
            if (cba * dba >= 0.0 || adc * bdc >= 0.0 || cba * bdc <= 0.0) {
                continue;
            }
            if (max(cba * cba, dba * dba) < squared_tol * dot(eA, eA)) {
                continue;
            }
            let t = cba / (cba - dba);
            var axis = mix(uB, vB, t);
            let len2 = dot(axis, axis);
            if (len2 < 1e-20) {
                continue;
            }
            axis = axis / sqrt(len2);
            let sep = dot(axis, qA - qB);
            if (sep > q.sep) {
                q.sep = sep;
                q.n = -axis;
                q.index_a = ea;
                q.index_b = eb;
            }
        }
    }
    return q;
}

fn closest_segments(p1: vec3<f32>, d1: vec3<f32>, p2: vec3<f32>, d2: vec3<f32>) -> vec2<f32> {
    let r = p1 - p2;
    let a = dot(d1, d1);
    let e = dot(d2, d2);
    let f = dot(d2, r);
    var s = 0.0;
    var t = 0.0;
    if (a < 1.1920929e-5 && e < 1.1920929e-5) {
        return vec2<f32>(0.0, 0.0);
    }
    if (a < 1.1920929e-5) {
        t = clamp(f / e, 0.0, 1.0);
    } else {
        let c = dot(d1, r);
        if (e < 1.1920929e-5) {
            s = clamp(-c / a, 0.0, 1.0);
        } else {
            let b = dot(d1, d2);
            let denom = a * e - b * b;
            if (denom > 1.17549435e-35) {
                s = clamp((b * f - c * e) / denom, 0.0, 1.0);
            }
            t = (b * s + f) / e;
            if (t < 0.0) {
                t = 0.0;
                s = clamp(-c / a, 0.0, 1.0);
            } else if (t > 1.0) {
                t = 1.0;
                s = clamp((b - c) / a, 0.0, 1.0);
            }
        }
    }
    return vec2<f32>(s, t);
}
