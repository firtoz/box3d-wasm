//! Synchronous convex queries over the host-side mirror.
//!
//! The GPU remains authoritative for stepping. Native `b3World_Step` performs
//! its readback before returning, so all queries below see post-step poses.

#[path = "query_toi.rs"]
mod toi;

use core::ffi::{c_char, c_void};
use std::sync::Arc;

use glam::{Quat, Vec3 as V3};

use super::{
    Aabb, BodyCastResult, BodyId, BodyPlaneResult, Capsule, CastOutput, CastResultFcn,
    CollisionPlane, Filter, MoverFilterFcn, OverlapResultFcn, Plane, PlaneResult, PlaneResultFcn,
    PlaneSolverResult, QueryFilter, RayResult, ShapeId, ShapeProxy, Sphere, TreeStats, WorldId,
    WorldTransform,
};
use crate::types::{KIND_CAPSULE, KIND_MESH, KIND_SPHERE, LINEAR_SLOP};

const OVERLAP_SLOP: f32 = 10.0 * f32::EPSILON;
const MAX_GJK_ITERS: usize = 32;
const MAX_CAST_ITERS: usize = 20;

#[derive(Clone, Copy, Debug)]
pub(super) struct MeshInstance {
    pub position: [f32; 3],
    pub rotation: [f32; 4],
    pub scale: [f32; 3],
}
impl MeshInstance {
    pub fn point(self, point: [f32; 3]) -> [f32; 3] {
        (v(self.position) + q(self.rotation) * (v(self.scale) * v(point))).to_array()
    }
    fn inverse_point(self, point: V3) -> V3 {
        (q(self.rotation).conjugate() * (point - v(self.position))) / v(self.scale)
    }
    fn reflected(self) -> bool {
        self.scale.iter().product::<f32>() < 0.0
    }
}

#[derive(Clone)]
pub(super) struct HostShape {
    // Internal collider identity is retained for geometry lookup and cache refits.
    pub id: ShapeId,
    pub public_id: ShapeId,
    pub child_index: i32,
    pub compound_material_indices: [i32; 4],
    pub kind: u32,
    pub body_origin: [f32; 3],
    pub body_rotation: [f32; 4],
    pub local_points: Arc<Vec<[f32; 3]>>,
    pub local_center: [f32; 3],
    pub radius: f32,
    pub half_extents: [f32; 3],
    pub hull_planes: Arc<Vec<[f32; 4]>>,
    pub hull_topology: Arc<Vec<[u32; 4]>>,
    pub mesh_triangles: Arc<Vec<[u32; 4]>>,
    pub mesh_triangle_ids: Arc<Vec<i32>>,
    pub mesh_nodes: Arc<Vec<super::world::MeshNode>>,
    pub mesh_instance: Option<MeshInstance>,
    pub filter: Filter,
    pub friction: f32,
    pub restitution: f32,
    pub user_material_id: u64,
    pub user_data: usize,
}

#[derive(Clone)]
pub(super) struct HostBody {
    pub origin: [f32; 3],
    pub shapes: Vec<HostShape>,
}

#[derive(Clone, Default)]
pub(super) struct HostWorld {
    pub shapes: Vec<HostShape>,
    pub groups: Arc<Vec<Vec<usize>>>,
}

#[derive(Clone)]
struct Proxy {
    points: Vec<V3>,
    radius: f32,
}

#[derive(Clone, Copy, Default)]
struct Vertex {
    a: V3,
    b: V3,
    w: V3,
    weight: f32,
    index_a: usize,
    index_b: usize,
}

#[derive(Clone, Copy, Default)]
struct Simplex {
    vertices: [Vertex; 4],
    count: usize,
}

#[derive(Clone, Copy, Default)]
struct DistanceOutput {
    simplex: Simplex,
    point_a: V3,
    point_b: V3,
    normal: V3,
    distance: f32,
}

#[derive(Clone, Copy, Debug)]
pub(super) struct SweepHit {
    pub fraction: f32,
    pub point: [f32; 3],
    pub normal: [f32; 3],
}

#[derive(Clone, Copy, Debug)]
pub(super) struct MeshSweepHit {
    pub hit: SweepHit,
    pub triangle_index: i32,
}

pub(super) struct ExplosionGeometry {
    pub closest_point: [f32; 3],
    pub direction: [f32; 3],
    pub distance: f32,
    pub area: f32,
}

fn v(value: [f32; 3]) -> V3 {
    V3::from_array(value)
}

fn arc_vec<T>(value: Vec<T>) -> Arc<Vec<T>> {
    Arc::new(value)
}

fn q(value: [f32; 4]) -> Quat {
    Quat::from_array(value)
}

fn transform_point(position: [f32; 3], rotation: [f32; 4], point: [f32; 3]) -> V3 {
    v(position) + q(rotation) * v(point)
}

fn shape_proxy(shape: &HostShape, transform: Option<WorldTransform>) -> Proxy {
    let (position, rotation) = transform
        .map(|xf| (xf.p, xf.q))
        .unwrap_or((shape.body_origin, shape.body_rotation));
    Proxy {
        points: shape
            .local_points
            .iter()
            .map(|&point| transform_point(position, rotation, shape.mesh_instance.map_or(point, |instance| instance.point(point))))
            .collect(),
        radius: shape.radius,
    }
}

fn nlerp_rotation(a: [f32; 4], b: [f32; 4], t: f32) -> [f32; 4] {
    let qa = q(a).normalize();
    let mut qb = q(b).normalize();
    if qa.dot(qb) < 0.0 {
        qb = -qb;
    }
    (qa * (1.0 - t) + qb * t).normalize().to_array()
}

fn interpolate_transform(a: WorldTransform, b: WorldTransform, t: f32) -> WorldTransform {
    WorldTransform {
        p: (v(a.p) + t * (v(b.p) - v(a.p))).to_array(),
        q: nlerp_rotation(a.q, b.q, t),
    }
}

fn rotation_angle(a: [f32; 4], b: [f32; 4]) -> f32 {
    let dot = q(a).normalize().dot(q(b).normalize()).abs().clamp(0.0, 1.0);
    2.0 * dot.acos()
}

fn sweep_radius(shape: &HostShape) -> f32 {
    shape
        .local_points
        .iter()
        .map(|point| v(*point).length() + shape.radius)
        .fold(shape.radius, f32::max)
}

// For fixed orientations, every support point translates linearly. If the
// same plane separates the cores beyond the TOI target at both endpoints,
// it separates them throughout the interval. Recheck support extrema rather
// than trusting an approximate GJK distance as a separation certificate.
fn fixed_orientation_sweep_separated(
    shape_a: &HostShape, start_a: WorldTransform, end_a: WorldTransform,
    shape_b: &HostShape, start_b: WorldTransform, end_b: WorldTransform,
    max_fraction: f32, axis: V3, target: f32,
) -> bool {
    let same_rotation = |a: [f32;4], b: [f32;4]| a == b || a == b.map(|x| -x);
    // Do not use acos(dot)==0: float rounding can hide a real small rotation.
    if !same_rotation(start_a.q,end_a.q) || !same_rotation(start_b.q,end_b.q) {
        return false;
    }
    let Some(axis) = axis.try_normalize() else { return false; };
    for fraction in [0.0,max_fraction] {
        let a = shape_proxy(shape_a,Some(interpolate_transform(start_a,end_a,fraction)));
        let b = shape_proxy(shape_b,Some(interpolate_transform(start_b,end_b,fraction)));
        if a.points.is_empty() || b.points.is_empty() { return false; }
        let origin = a.points[0];
        let upper_a = a.points.iter().map(|p| (*p-origin).dot(axis)).fold(f32::NEG_INFINITY,f32::max);
        let lower_b = b.points.iter().map(|p| (*p-origin).dot(axis)).fold(f32::INFINITY,f32::min);
        let magnitude = a.points.iter().chain(&b.points).map(|p| p.abs().max_element()).fold(1.0,f32::max);
        // Cover transformed-point/subtraction/projection rounding, especially
        // far from origin. An inconclusive plane retains conservative TOI.
        let margin = 32.0 * f32::EPSILON * magnitude;
        if !upper_a.is_finite() || !lower_b.is_finite() || lower_b-upper_a <= target+margin {
            return false;
        }
    }
    true
}

/// Bounded conservative advancement for two rigid convex sweeps.
///
/// The global translational-plus-angular motion bound makes every advance
/// conservative even when separation is non-monotonic (for example a rapidly
/// spinning thin stick). GJK supplies the separating distance at each pose.
pub(super) fn rigid_time_of_impact(
    shape_a: &HostShape,
    start_a: WorldTransform,
    end_a: WorldTransform,
    shape_b: &HostShape,
    start_b: WorldTransform,
    end_b: WorldTransform,
    max_fraction: f32,
) -> Option<SweepHit> {
    rigid_time_of_impact_bounded(shape_a, start_a, end_a, shape_b, start_b, end_b, max_fraction, 512)
}

fn rigid_time_of_impact_bounded(
    shape_a: &HostShape, start_a: WorldTransform, end_a: WorldTransform,
    shape_b: &HostShape, start_b: WorldTransform, end_b: WorldTransform,
    max_fraction: f32, iteration_limit: usize,
) -> Option<SweepHit> {
    let total_radius = shape_a.radius + shape_b.radius;
    let target = LINEAR_SLOP.max(total_radius - LINEAR_SLOP);
    let tolerance = 0.25 * LINEAR_SLOP;
    let motion_bound = (v(end_a.p) - v(start_a.p)).length()
        + (v(end_b.p) - v(start_b.p)).length()
        + rotation_angle(start_a.q, end_a.q) * sweep_radius(shape_a)
        + rotation_angle(start_b.q, end_b.q) * sweep_radius(shape_b);
    if motion_bound <= f32::EPSILON || max_fraction <= 0.0 {
        return None;
    }

    let at = |fraction: f32| {
        let proxy_a = shape_proxy(
            shape_a,
            Some(interpolate_transform(start_a, end_a, fraction)),
        );
        let proxy_b = shape_proxy(
            shape_b,
            Some(interpolate_transform(start_b, end_b, fraction)),
        );
        distance(&proxy_a, &proxy_b, false)
    };

    let initial = at(0.0);
    // Box3D does not clamp an already-overlapping body. Treat the target
    // shell as initial contact as well so speculative contacts remain discrete.
    if initial.distance <= target + tolerance {
        return None;
    }

    if fixed_orientation_sweep_separated(shape_a,start_a,end_a,shape_b,start_b,end_b,
                                         max_fraction,initial.normal,target) {
        return None;
    }

    let mut fraction = 0.0f32;
    let mut separated = initial;
    for _ in 0..iteration_limit {
        // Advance toward the target, not its outer acceptance shell. Stopping
        // on that shell can round just outside it on the next step and produce
        // a spurious near-zero TOI that truncates tangential motion.
        let advance = ((separated.distance - target) / motion_bound).max(1.0e-6);
        let next_fraction = (fraction + advance).min(max_fraction);
        if next_fraction <= fraction {
            return None;
        }
        let next = at(next_fraction);
        if next.distance <= target {
            let mut lower = fraction;
            let mut upper = next_fraction;
            let mut hit = next;
            for _ in 0..24 {
                let middle = 0.5 * (lower + upper);
                let candidate = at(middle);
                if candidate.distance > target {
                    lower = middle;
                } else {
                    upper = middle;
                    hit = candidate;
                }
            }
            if upper <= 0.0 {
                return None;
            }
            let normal = hit
                .normal
                .try_normalize()
                .unwrap_or_else(|| (hit.point_b - hit.point_a).normalize_or_zero());
            return Some(SweepHit {
                fraction: upper,
                point: ((hit.point_a + shape_a.radius * normal + hit.point_b
                    - shape_b.radius * normal)
                    * 0.5)
                    .to_array(),
                normal: normal.to_array(),
            });
        }
        if next_fraction >= max_fraction {
            return None;
        }
        fraction = next_fraction;
        separated = next;
    }
    if std::env::var_os("GPU_PHYSICS_TRACE_TOI_FAILURES").is_some() {
        eprintln!("toi-iteration-limit shape={} fraction={} separation={} start={:?} end={:?}", shape_b.id.index1, fraction, separated.distance, start_b.p, end_b.p);
    }
    // This is unresolved, not a proven miss. As with Box3D's failed TOI state,
    // keep the last conservative advance so an iteration cap cannot allow the
    // remainder of an unchecked sweep to cross a collider.
    let normal = separated.normal.try_normalize()
        .unwrap_or_else(|| (separated.point_b - separated.point_a).normalize_or_zero());
    Some(SweepHit {
        fraction,
        point: ((separated.point_a + shape_a.radius * normal + separated.point_b
            - shape_b.radius * normal) * 0.5).to_array(),
        normal: normal.to_array(),
    })
}

/// Runs rigid convex TOI against the one-sided triangles selected by the mesh
/// BVH. Results are ordered by fraction and source triangle index so callers
/// can continue after a pre-solve veto without repeating the tree traversal when
/// `collect_all_hits` is set. Otherwise retain native closest-hit interval clipping.
pub(super) fn rigid_mesh_time_of_impacts(
    mesh: &HostShape,
    mesh_transform: WorldTransform,
    convex: &HostShape,
    convex_start: WorldTransform,
    convex_end: WorldTransform,
    convex_local_center: [f32; 3],
    max_fraction: f32,
    is_sensor: bool,
    collect_all_hits: bool,
) -> Vec<MeshSweepHit> {
    static TRACE_TOI_SHAPE: std::sync::OnceLock<Option<i32>> = std::sync::OnceLock::new();
    let trace = *TRACE_TOI_SHAPE.get_or_init(|| std::env::var("GPU_PHYSICS_TRACE_TOI_SHAPE").ok().and_then(|s| s.parse().ok())) == Some(convex.id.index1);
    let local_center = v(convex_local_center);
    let radius = convex.local_points.iter()
        .map(|&point| (v(point)-local_center).length()+convex.radius)
        .fold(convex.radius,f32::max);
    let start = v(convex_start.p);
    let end = v(convex_end.p);
    let bounds = Aabb {
        lower_bound: (start.min(end) - V3::splat(radius)).to_array(),
        upper_bound: (start.max(end) + V3::splat(radius)).to_array(),
    };
    // Preserve the native BVH leaf order: each earlier hit tightens the next
    // root interval, and sorting triangles changes the accepted TOI rounding.
    let candidates = mesh_candidates(mesh, mesh_transform, bounds);
    // The BVH result is already bounded by uploaded mesh geometry. Process
    // every candidate; truncating this list silently loses continuous contacts.

    // Recenter as b3SolveContinuous does, before constructing mesh-local
    // centroids or TOI sweeps. The BVH lookup above still uses world bounds.
    let origin = v(convex_start.p);
    let mesh_transform = WorldTransform {p:(v(mesh_transform.p)-origin).to_array(),..mesh_transform};
    let convex_start = WorldTransform {p:[0.0;3],..convex_start};
    let convex_end = WorldTransform {p:(v(convex_end.p)-origin).to_array(),..convex_end};
    let start_pose = WorldTransform {p:(v(convex_start.p)-toi::rotate_q(convex_start.q,local_center)).to_array(),..convex_start};
    let end_pose = WorldTransform {p:(v(convex_end.p)-toi::rotate_q(convex_end.q,local_center)).to_array(),..convex_end};
    let convex_start_proxy = Proxy {
        points:convex.local_points.iter().map(|&p|toi::point(start_pose,v(p))).collect(),
        radius:convex.radius,
    };
    let convex_center = toi::inv_rotate_q(mesh_transform.q,toi::point(start_pose,v(convex.local_center))-v(mesh_transform.p));
    let end_center = toi::inv_rotate_q(mesh_transform.q,toi::point(end_pose,v(convex.local_center))-v(mesh_transform.p));
    let min_extent = if matches!(convex.kind, KIND_SPHERE | KIND_CAPSULE) {
        convex.radius
    } else if !convex.hull_planes.is_empty() {
        convex.hull_planes.iter().map(|p| p[3] - v([p[0],p[1],p[2]]).dot(v(convex.local_center)))
            .fold(f32::MAX, f32::min)
    } else {
        convex.half_extents.into_iter().fold(f32::MAX, f32::min)
    };
    let fallback_radius = (0.5 * min_extent).max(LINEAR_SLOP);
    let identity = WorldTransform {
        p: [0.0; 3],
        q: [0.0, 0.0, 0.0, 1.0],
    };
    let mut hits = Vec::new();
    let mut query_fraction = max_fraction;
    for index in candidates {
        let Some([a, b, c]) = mesh_triangle(mesh, identity, index) else {
            continue;
        };
        let triangle_normal = (b - a).cross(c - a).normalize_or_zero();
        if triangle_normal.length_squared() <= f32::EPSILON
            || triangle_normal.dot(convex_center - a) < 0.0
        {
            continue;
        }
        // Match b3MeshTimeOfImpactFcn: a solid triangle cannot arrest a
        // glancing sweep whose centroid remains safely in front of its plane.
        let offset1 = triangle_normal.dot(convex_center - a);
        let offset2 = triangle_normal.dot(end_center - a);
        if !is_sensor && offset1 - offset2 < fallback_radius && offset2 > fallback_radius {
            continue;
        }
        let triangle = HostShape {
            id: mesh.id,
            public_id: mesh.public_id,
            child_index: mesh.child_index,
            compound_material_indices: mesh.compound_material_indices,
            kind: mesh.kind,
            body_origin: [0.0; 3],
            body_rotation: identity.q,
            local_points: arc_vec(vec![a.to_array(), b.to_array(), c.to_array()]),
            local_center: [0.0; 3],
            radius: 0.0,
            half_extents: [0.0; 3],
            hull_planes: arc_vec(Vec::new()),
            hull_topology: arc_vec(Vec::new()),
            mesh_triangles: arc_vec(Vec::new()),
            mesh_triangle_ids: arc_vec(Vec::new()),
            mesh_nodes: arc_vec(Vec::new()),
            mesh_instance: None,
            filter: mesh.filter,
            friction: mesh.friction,
            restitution: mesh.restitution,
            user_material_id: mesh.user_material_id,
            user_data: mesh.user_data,
        };
        let mut hit = toi::time_of_impact_sweeps(
            &triangle, mesh_transform, mesh_transform, convex, convex_start, convex_end,
            V3::ZERO, local_center, query_fraction,
        );
        if hit.is_none() {
            // Box3D b3MeshTimeOfImpactFcn retries an initial-contact TOI using
            // a small sphere at the shape centroid. Discrete contacts may stop
            // a rotating tip without stopping the centroid crossing the mesh.
            let initial = distance(&Proxy {points:triangle.local_points.iter().map(|&p|toi::point(mesh_transform,v(p))).collect(),radius:0.0}, &convex_start_proxy, false);
            let target = LINEAR_SLOP.max(convex.radius - LINEAR_SLOP);
            if trace { eprintln!("mesh-toi shape={} triangle={} start={:?} end={:?} initial={} target={} fallback={}", convex.id.index1, index, convex_start.p, convex_end.p, initial.distance, target, initial.distance <= target + 0.25 * LINEAR_SLOP); }

            if initial.distance <= target + 0.25 * LINEAR_SLOP {
                let mut centroid = convex.clone();
                centroid.local_points = arc_vec(vec![convex.local_center]);
                centroid.radius = fallback_radius + LINEAR_SLOP;
                hit = toi::time_of_impact_sweeps(
                    &triangle, mesh_transform, mesh_transform, &centroid, convex_start, convex_end,
                    V3::ZERO, local_center, query_fraction,
                );
                if trace { eprintln!("mesh-toi-fallback triangle={} verts={:?}/{:?}/{:?} center={:?} radius={} fraction={:?}", index, a, b, c, centroid.local_center, centroid.radius, hit.as_ref().map(|h| h.fraction)); }
            }
        }
        let Some(mut hit) = hit else { continue; };
        if !collect_all_hits {
            if hit.fraction >= query_fraction { continue; }
            query_fraction = hit.fraction;
        }
        if trace { eprintln!("mesh-toi-hit triangle={} fraction={} normal={:?} face={:?} dot={}", index, hit.fraction, hit.normal, triangle_normal, v(hit.normal).dot(triangle_normal)); }
        // Sidedness was established from the starting centroid above, as in
        // Box3D b3MeshTimeOfImpactFcn. A positive conservative TOI must survive
        // even when GJK's witness normal degenerates at contact onset (or is
        // an edge normal). Rejecting it here lets thin rotating hulls tunnel.
        hit.normal = toi::rotate_q(mesh_transform.q,triangle_normal).to_array();
        hit.point = (v(hit.point)+origin).to_array();
        hits.push(MeshSweepHit {
            hit,
            triangle_index: mesh
                .mesh_triangle_ids
                .get(index)
                .copied()
                .unwrap_or(index as i32),
        });
    }
    hits.sort_by(|a, b| {
        a.hit
            .fraction
            .total_cmp(&b.hit.fraction)
            .then_with(|| a.triangle_index.cmp(&b.triangle_index))
    });
    if !collect_all_hits { hits.truncate(1); }
    hits
}

fn local_shape_proxy(shape: &HostShape) -> Proxy {
    Proxy {
        points: shape.local_points.iter().copied().map(|point| v(shape.mesh_instance.map_or(point, |instance| instance.point(point)))).collect(),
        radius: shape.radius,
    }
}

unsafe fn input_proxy(proxy: &ShapeProxy, origin: [f32; 3]) -> Proxy {
    Proxy {
        points: proxy
            .as_slice()
            .iter()
            .map(|&point| v(origin) + v(point))
            .collect(),
        radius: proxy.radius.max(0.0),
    }
}

fn support(points: &[V3], direction: V3) -> usize {
    let origin = points[0];
    let mut best = 0;
    let mut projection = 0.0;
    for (index, point) in points.iter().enumerate().skip(1) {
        let candidate = direction.dot(*point - origin);
        if candidate > projection {
            best = index;
            projection = candidate;
        }
    }
    best
}

fn edge_coords(a: V3, b: V3) -> [f32; 3] {
    let ab = b - a;
    [b.dot(ab), -a.dot(ab), ab.length_squared()]
}

fn tri_coords(a: V3, b: V3, c: V3) -> [f32; 4] {
    let n = (b - a).cross(c - a);
    [
        b.cross(c).dot(n),
        c.cross(a).dot(n),
        a.cross(b).dot(n),
        n.length_squared(),
    ]
}

fn triple(a: V3, b: V3, c: V3) -> f32 {
    a.cross(b).dot(c)
}

fn tet_coords(a: V3, b: V3, c: V3, d: V3) -> [f32; 5] {
    let divisor = triple(b - a, c - a, d - a);
    let sign = if divisor < 0.0 { -1.0 } else { 1.0 };
    [
        sign * triple(b, c, d),
        sign * triple(a, d, c),
        sign * triple(a, b, d),
        sign * triple(a, c, b),
        sign * divisor,
    ]
}

fn solve2(simplex: &mut Simplex) -> bool {
    let a = simplex.vertices[0];
    let b = simplex.vertices[1];
    let weights = edge_coords(a.w, b.w);
    if weights[1] <= 0.0 {
        simplex.count = 1;
        simplex.vertices[0].weight = 1.0;
    } else if weights[0] <= 0.0 {
        simplex.count = 1;
        simplex.vertices[0] = b;
        simplex.vertices[0].weight = 1.0;
    } else {
        if weights[2] <= 0.0 {
            return false;
        }
        simplex.vertices[0].weight = weights[0] / weights[2];
        simplex.vertices[1].weight = weights[1] / weights[2];
    }
    true
}

fn solve3(simplex: &mut Simplex) -> bool {
    let a = simplex.vertices[0];
    let b = simplex.vertices[1];
    let c = simplex.vertices[2];
    let ab = edge_coords(a.w, b.w);
    let bc = edge_coords(b.w, c.w);
    let ca = edge_coords(c.w, a.w);
    if ab[1] <= 0.0 && ca[0] <= 0.0 {
        simplex.count = 1;
        simplex.vertices[0] = a;
        simplex.vertices[0].weight = 1.0;
        return true;
    }
    if bc[1] <= 0.0 && ab[0] <= 0.0 {
        simplex.count = 1;
        simplex.vertices[0] = b;
        simplex.vertices[0].weight = 1.0;
        return true;
    }
    if ca[1] <= 0.0 && bc[0] <= 0.0 {
        simplex.count = 1;
        simplex.vertices[0] = c;
        simplex.vertices[0].weight = 1.0;
        return true;
    }
    let abc = tri_coords(a.w, b.w, c.w);
    let set_edge = |simplex: &mut Simplex, x: Vertex, y: Vertex, weights: [f32; 3]| {
        if weights[2] <= 0.0 {
            return false;
        }
        simplex.count = 2;
        simplex.vertices[0] = x;
        simplex.vertices[1] = y;
        simplex.vertices[0].weight = weights[0] / weights[2];
        simplex.vertices[1].weight = weights[1] / weights[2];
        true
    };
    if abc[2] <= 0.0 && ab[0] > 0.0 && ab[1] > 0.0 {
        return set_edge(simplex, a, b, ab);
    }
    if abc[0] <= 0.0 && bc[0] > 0.0 && bc[1] > 0.0 {
        return set_edge(simplex, b, c, bc);
    }
    if abc[1] <= 0.0 && ca[0] > 0.0 && ca[1] > 0.0 {
        return set_edge(simplex, c, a, ca);
    }
    if abc[3] <= 0.0 {
        return false;
    }
    simplex.vertices[0].weight = abc[0] / abc[3];
    simplex.vertices[1].weight = abc[1] / abc[3];
    simplex.vertices[2].weight = abc[2] / abc[3];
    true
}

fn solve4(simplex: &mut Simplex) -> bool {
    let a = simplex.vertices[0];
    let b = simplex.vertices[1];
    let c = simplex.vertices[2];
    let d = simplex.vertices[3];
    let vertices = [a, b, c, d];

    // Pick the closest valid outside face. If no face sees the origin, the
    // origin is inside the tetrahedron.
    let faces = [(0, 2, 1, 3), (0, 1, 3, 2), (0, 3, 2, 1), (1, 2, 3, 0)];
    let mut best: Option<(f32, [Vertex; 3])> = None;
    for (ia, ib, ic, opposite) in faces {
        let va = vertices[ia];
        let vb = vertices[ib];
        let vc = vertices[ic];
        let vo = vertices[opposite];
        let normal = (vb.w - va.w).cross(vc.w - va.w);
        if normal.dot(-va.w) * normal.dot(vo.w - va.w) >= 0.0 {
            continue;
        }
        let mut face = Simplex {
            count: 3,
            ..Default::default()
        };
        face.vertices[0] = va;
        face.vertices[1] = vb;
        face.vertices[2] = vc;
        if !solve3(&mut face) {
            continue;
        }
        let closest = simplex_closest(&face);
        let candidate = [face.vertices[0], face.vertices[1], face.vertices[2]];
        if best.is_none_or(|(distance, _)| closest.length_squared() < distance) {
            best = Some((closest.length_squared(), candidate));
            simplex.count = face.count;
            simplex.vertices[..face.count].copy_from_slice(&face.vertices[..face.count]);
        }
    }
    if best.is_some() {
        return true;
    }

    let weights = tet_coords(a.w, b.w, c.w, d.w);
    // Roundoff in a nearly coplanar tetrahedron can hide every outside
    // face. Negative barycentric weights do not certify an interior point.
    // Keep the previous valid simplex instead of manufacturing overlap.
    if weights[4] <= 0.0 || weights[..4].iter().any(|weight| *weight < 0.0) {
        return false;
    }
    for i in 0..4 {
        simplex.vertices[i].weight = weights[i] / weights[4];
    }
    true
}

fn simplex_closest(simplex: &Simplex) -> V3 {
    simplex.vertices[..simplex.count]
        .iter()
        .fold(V3::ZERO, |sum, vertex| sum + vertex.weight * vertex.w)
}

fn witnesses(simplex: &Simplex) -> (V3, V3) {
    if simplex.count == 4 {
        let point = simplex.vertices[..4]
            .iter()
            .fold(V3::ZERO, |sum, vertex| sum + vertex.weight * vertex.a);
        return (point, point);
    }
    simplex.vertices[..simplex.count]
        .iter()
        .fold((V3::ZERO, V3::ZERO), |(a, b), vertex| {
            (a + vertex.weight * vertex.a, b + vertex.weight * vertex.b)
        })
}

fn distance(proxy_a: &Proxy, proxy_b: &Proxy, use_radii: bool) -> DistanceOutput {
    distance_cached(proxy_a, proxy_b, use_radii, None)
}

fn distance_cached(proxy_a: &Proxy, proxy_b: &Proxy, use_radii: bool, cache: Option<Simplex>) -> DistanceOutput {
    if proxy_a.points.is_empty() || proxy_b.points.is_empty() {
        return DistanceOutput::default();
    }
    let mut simplex = Simplex {
        count: 1,
        ..Default::default()
    };
    simplex.vertices[0] = Vertex {
        a: proxy_a.points[0],
        b: proxy_b.points[0],
        w: proxy_b.points[0] - proxy_a.points[0],
        weight: 1.0,
        index_a: 0,
        index_b: 0,
    };
    if let Some(cached) = cache.filter(|c| c.count > 0 && c.count < 4) {
        simplex = cached;
        for vertex in &mut simplex.vertices[..simplex.count] {
            vertex.a = proxy_a.points[vertex.index_a];
            vertex.b = proxy_b.points[vertex.index_b];
            vertex.w = vertex.b - vertex.a;
        }
    }
    let mut backup = simplex;
    let mut previous_distance = f32::MAX;
    let mut normal = V3::ZERO;
    for _ in 0..MAX_GJK_ITERS {
        let solved = match simplex.count {
            1 => {
                simplex.vertices[0].weight = 1.0;
                true
            }
            2 => solve2(&mut simplex),
            3 => solve3(&mut simplex),
            4 => solve4(&mut simplex),
            _ => false,
        };
        if !solved {
            simplex = backup;
            break;
        }
        if simplex.count == 4 {
            let (point_a, point_b) = witnesses(&simplex);
            return DistanceOutput {
                simplex,
                point_a,
                point_b,
                ..Default::default()
            };
        }
        let closest = simplex_closest(&simplex);
        let distance_squared = closest.length_squared();
        if distance_squared >= previous_distance {
            simplex = backup;
            break;
        }
        previous_distance = distance_squared;
        let direction = match simplex.count {
            1 => -simplex.vertices[0].w,
            2 => {
                let ab = simplex.vertices[1].w - simplex.vertices[0].w;
                ab.cross(-simplex.vertices[0].w).cross(ab)
            }
            3 => {
                let a = simplex.vertices[0].w;
                let n = (simplex.vertices[1].w - a).cross(simplex.vertices[2].w - a);
                if n.dot(a) < 0.0 {
                    n
                } else {
                    -n
                }
            }
            _ => V3::ZERO,
        };
        if direction.length_squared() < 1000.0 * f32::MIN_POSITIVE {
            let (point_a, point_b) = witnesses(&simplex);
            return DistanceOutput {
                simplex,
                point_a,
                point_b,
                ..Default::default()
            };
        }
        normal = -direction;
        let index_a = support(&proxy_a.points, -direction);
        let index_b = support(&proxy_b.points, direction);
        if simplex.vertices[..simplex.count]
            .iter()
            .any(|vertex| vertex.index_a == index_a && vertex.index_b == index_b)
        {
            break;
        }
        backup = simplex;
        let a = proxy_a.points[index_a];
        let b = proxy_b.points[index_b];
        simplex.vertices[simplex.count] = Vertex {
            a,
            b,
            w: b - a,
            weight: 0.0,
            index_a,
            index_b,
        };
        simplex.count += 1;
    }

    let (mut point_a, mut point_b) = witnesses(&simplex);
    let mut separation = (point_b - point_a).length();
    normal = normal
        .try_normalize()
        .unwrap_or_else(|| (point_b - point_a).try_normalize().unwrap_or(V3::ZERO));
    if use_radii && normal != V3::ZERO {
        separation = (separation - proxy_a.radius - proxy_b.radius).max(0.0);
        point_a += proxy_a.radius * normal;
        point_b -= proxy_b.radius * normal;
    }
    DistanceOutput {
        simplex,
        point_a,
        point_b,
        normal,
        distance: separation,
    }
}

fn hull_projected_area(shape: &HostShape, direction: V3) -> f32 {
    if shape.kind == crate::types::KIND_BOX {
        let h = v(shape.half_extents);
        let d = direction.abs();
        return 4.0 * (h.y * h.z * d.x + h.x * h.z * d.y + h.x * h.y * d.z);
    }

    let mut area = 0.0;
    let mut visited = vec![false; shape.hull_topology.len()];
    for start in 0..shape.hull_topology.len() {
        if visited[start] {
            continue;
        }
        let face = shape.hull_topology[start][3];
        let mut indices = Vec::new();
        let mut edge = start;
        loop {
            if edge >= shape.hull_topology.len()
                || visited[edge]
                || shape.hull_topology[edge][3] != face
            {
                break;
            }
            visited[edge] = true;
            indices.push(shape.hull_topology[edge][2] as usize);
            edge = shape.hull_topology[edge][0] as usize;
            if edge == start {
                break;
            }
        }
        if edge != start || indices.len() < 3 {
            continue;
        }
        let Some(&p1) = shape.local_points.get(indices[0]) else {
            continue;
        };
        let mut p2 = match shape.local_points.get(indices[1]) {
            Some(point) => *point,
            None => continue,
        };
        for &index in indices.iter().skip(2) {
            let Some(&p3) = shape.local_points.get(index) else {
                break;
            };
            area += (v(p2) - v(p1)).cross(v(p3) - v(p1)).dot(direction).max(0.0);
            p2 = p3;
        }
    }
    if area > 0.0 {
        return 0.5 * area;
    }

    // Some imported test hulls omit half-edge loops. Reconstruct each face
    // from its plane and vertices while retaining Box3D's projected-face sum.
    for plane in shape.hull_planes.iter() {
        let normal = V3::new(plane[0], plane[1], plane[2]);
        let projection = normal.dot(direction);
        if projection <= 0.0 {
            continue;
        }
        let mut points: Vec<V3> = shape
            .local_points
            .iter()
            .copied()
            .map(v)
            .filter(|point| (normal.dot(*point) - plane[3]).abs() <= 2.0e-4)
            .collect();
        if points.len() < 3 {
            continue;
        }
        let center = points.iter().copied().sum::<V3>() / points.len() as f32;
        let tangent = if normal.x.abs() > normal.z.abs() {
            V3::new(-normal.y, normal.x, 0.0).normalize()
        } else {
            V3::new(0.0, -normal.z, normal.y).normalize()
        };
        let bitangent = normal.cross(tangent);
        points.sort_by(|a, b| {
            let ra = *a - center;
            let rb = *b - center;
            ra.dot(bitangent)
                .atan2(ra.dot(tangent))
                .total_cmp(&rb.dot(bitangent).atan2(rb.dot(tangent)))
        });
        let mut face_area = 0.0;
        for i in 0..points.len() {
            face_area += (points[i] - center)
                .cross(points[(i + 1) % points.len()] - center)
                .dot(normal);
        }
        area += 0.5 * face_area.abs() * projection;
    }
    area
}

pub(super) fn explosion_geometry(shape: &HostShape, local_position: [f32; 3]) -> ExplosionGeometry {
    let point = Proxy {
        points: vec![v(local_position)],
        radius: 0.0,
    };
    let output = distance(&local_shape_proxy(shape), &point, true);
    let closest = if output.distance == 0.0 {
        v(shape.local_center)
    } else {
        output.point_a
    };
    let offset = closest - v(local_position);
    let direction = if offset.length_squared() > 100.0 * f32::EPSILON * f32::EPSILON {
        offset.normalize()
    } else {
        V3::X
    };
    let area = match shape.kind {
        KIND_SPHERE => core::f32::consts::PI * shape.radius * shape.radius,
        KIND_CAPSULE => {
            let axis = v(shape.local_points[1]) - v(shape.local_points[0]);
            core::f32::consts::PI * shape.radius * shape.radius
                + 2.0 * shape.radius * axis.cross(direction).length()
        }
        crate::types::KIND_BOX | crate::types::KIND_CONVEX_HULL => {
            hull_projected_area(shape, direction)
        }
        _ => 0.0,
    };
    ExplosionGeometry {
        closest_point: closest.to_array(),
        direction: direction.to_array(),
        distance: output.distance,
        area,
    }
}

fn shape_cast(
    target: &Proxy,
    moving: &Proxy,
    translation: V3,
    max_fraction: f32,
    can_encroach: bool,
) -> CastOutput {
    let total_radius = target.radius + moving.radius;
    let mut target_distance = LINEAR_SLOP.max(total_radius - LINEAR_SLOP);
    let tolerance = 0.25 * LINEAR_SLOP;
    let mut alpha = 0.0;
    let mut shifted = moving.clone();
    let mut output = CastOutput {
        triangle_index: -1,
        child_index: -1,
        material_index: 0,
        ..Default::default()
    };
    for iteration in 0..MAX_CAST_ITERS {
        output.iterations += 1;
        let result = distance(target, &shifted, false);
        if result.distance < target_distance + tolerance {
            if iteration == 0 {
                if can_encroach && result.distance > 2.0 * LINEAR_SLOP {
                    target_distance = result.distance - LINEAR_SLOP;
                } else {
                    output.hit = true;
                    output.point =
                        ((result.point_a + target.radius * result.normal + result.point_b
                            - moving.radius * result.normal)
                            * 0.5)
                            .to_array();
                    return output;
                }
            } else {
                output.fraction = alpha;
                output.normal = result.normal.to_array();
                output.point = (result.point_a + target.radius * result.normal).to_array();
                output.hit = true;
                return output;
            }
        }
        if result.distance <= 0.0 || result.normal == V3::ZERO {
            return output;
        }
        let denominator = translation.dot(result.normal);
        if denominator >= 0.0 {
            return output;
        }
        alpha += (target_distance - result.distance) / denominator;
        if alpha >= max_fraction {
            return output;
        }
        shifted.points.clear();
        shifted.points.extend(
            moving
                .points
                .iter()
                .map(|point| *point + alpha * translation),
        );
    }
    output
}

fn ray_cast(target: &Proxy, origin: V3, translation: V3, max_fraction: f32) -> CastOutput {
    let ray = Proxy {
        points: vec![origin],
        radius: 0.0,
    };
    let mut shifted = ray.clone();
    let mut alpha = 0.0;
    let mut output = CastOutput {
        triangle_index: -1,
        child_index: -1,
        material_index: 0,
        ..Default::default()
    };
    for iteration in 0..MAX_GJK_ITERS {
        output.iterations += 1;
        let result = distance(target, &shifted, true);
        if result.distance <= 4.0 * f32::EPSILON {
            output.hit = true;
            output.fraction = alpha;
            output.point = if iteration == 0 {
                origin.to_array()
            } else {
                result.point_a.to_array()
            };
            output.normal = result.normal.to_array();
            return output;
        }
        if result.normal == V3::ZERO {
            return output;
        }
        let denominator = translation.dot(result.normal);
        if denominator >= 0.0 {
            return output;
        }
        alpha -= result.distance / denominator;
        if alpha > max_fraction {
            return output;
        }
        shifted.points[0] = origin + alpha * translation;
    }
    output
}

fn should_query(shape: Filter, query: QueryFilter) -> bool {
    (shape.category_bits & query.mask_bits) != 0 && (shape.mask_bits & query.category_bits) != 0
}

fn perpendicular(axis: V3) -> V3 {
    let p = if axis.x < -0.5 || axis.x > 0.5 {
        V3::new(axis.y, -axis.x, 0.0)
    } else {
        V3::new(0.0, axis.z, -axis.y)
    };
    p.try_normalize().unwrap_or(V3::Y)
}

fn mover_proxy(origin: [f32; 3], mover: &Capsule) -> Proxy {
    let origin = v(origin);
    Proxy {
        points: vec![origin + v(mover.center1), origin + v(mover.center2)],
        radius: mover.radius.max(0.0),
    }
}

fn collide_mover(
    shape: &HostShape,
    transform: WorldTransform,
    mover: &Proxy,
) -> Option<PlaneResult> {
    if mover.points.len() != 2 {
        return None;
    }
    if shape.kind == KIND_MESH {
        let bounds = aabb_for_proxy(mover);
        let center = 0.5 * (mover.points[0] + mover.points[1]);
        let mut best: Option<PlaneResult> = None;
        for index in mesh_candidates(shape, transform, bounds) {
            let Some([a, b, c]) = mesh_triangle(shape, transform, index) else {
                continue;
            };
            let normal = (b - a).cross(c - a).normalize_or_zero();
            if normal.dot(center - a) < 0.0 {
                continue;
            }
            for &endpoint in &mover.points {
                let point = closest_on_triangle(endpoint, a, b, c);
                let distance = endpoint.distance(point);
                let push = mover.radius - distance;
                if push >= 0.0 && best.as_ref().is_none_or(|p| push > p.plane.offset) {
                    best = Some(PlaneResult {
                        plane: Plane {
                            normal: normal.to_array().into(),
                            offset: push,
                        },
                        point: point.to_array().into(),
                    });
                }
            }
        }
        return best;
    }
    let target = shape_proxy(shape, Some(transform));
    let output = distance(&target, mover, false);
    let total_radius = target.radius + mover.radius;
    if output.distance > total_radius {
        return None;
    }

    // Box3D b3CollideMoverAndHull returns no plane when the capsule axis
    // intersects the hull. An invented depenetration plane changes the mover's
    // velocity clipping and the impulses it applies to other dynamic bodies.
    let hull = shape.kind == crate::types::KIND_BOX
        || shape.kind == crate::types::KIND_CONVEX_HULL;
    if hull && output.distance == 0.0 {
        return None;
    }
    let normal_threshold = if hull { 0.0 } else { LINEAR_SLOP };
    let (normal, separation, point) = if output.distance > normal_threshold
        && output.normal.length_squared() > f32::EPSILON * f32::EPSILON
    {
        (output.normal.normalize(), output.distance, output.point_a)
    } else if shape.kind == crate::types::KIND_BOX || shape.kind == crate::types::KIND_CONVEX_HULL {
        return None;
    } else {
        let axis = (mover.points[1] - mover.points[0])
            .try_normalize()
            .unwrap_or(V3::ZERO);
        let normal = if axis == V3::ZERO {
            V3::Y
        } else {
            perpendicular(axis)
        };
        let point = if shape.kind == KIND_SPHERE {
            target.points[0]
        } else {
            output.point_a
        };
        (normal, 0.0, point)
    };

    Some(PlaneResult {
        plane: Plane {
            normal: normal.to_array().into(),
            offset: total_radius - separation,
        },
        point: point.to_array().into(),
    })
}

pub fn b3_solve_planes(target_delta: [f32; 3], planes: &mut [CollisionPlane]) -> PlaneSolverResult {
    for plane in planes.iter_mut() {
        plane.push = 0.0;
    }
    let mut delta = v(target_delta);
    let mut iteration = 0;
    while iteration < 20 {
        let mut total_push = 0.0;
        for plane in planes.iter_mut() {
            let normal = v(plane.plane.normal.into());
            let separation = normal.dot(delta) - plane.plane.offset + LINEAR_SLOP;
            let push = -separation;
            let accumulated = plane.push;
            plane.push = (plane.push + push).clamp(0.0, plane.push_limit);
            let incremental = plane.push - accumulated;
            delta += incremental * normal;
            total_push += incremental.abs();
        }
        if total_push < LINEAR_SLOP {
            break;
        }
        iteration += 1;
    }
    PlaneSolverResult {
        delta: delta.to_array().into(),
        iteration_count: iteration,
    }
}

pub fn b3_clip_vector(vector: [f32; 3], planes: &[CollisionPlane]) -> [f32; 3] {
    let mut result = v(vector);
    for plane in planes {
        if plane.push == 0.0 || !plane.clip_velocity {
            continue;
        }
        let normal = v(plane.plane.normal.into());
        result -= result.dot(normal).min(0.0) * normal;
    }
    result.to_array()
}

fn aabb_for_proxy(proxy: &Proxy) -> Aabb {
    if proxy.points.is_empty() {
        return Aabb::default();
    }
    let mut lower = proxy.points[0];
    let mut upper = lower;
    for point in proxy.points.iter().skip(1) {
        lower = lower.min(*point);
        upper = upper.max(*point);
    }
    let radius = V3::splat(proxy.radius);
    Aabb {
        lower_bound: (lower - radius).to_array(),
        upper_bound: (upper + radius).to_array(),
    }
}

fn aabb_overlaps(a: Aabb, b: Aabb) -> bool {
    (0..3).all(|axis| {
        a.lower_bound[axis] <= b.upper_bound[axis] && b.lower_bound[axis] <= a.upper_bound[axis]
    })
}

fn ray_aabb(origin: V3, inv_dir: V3, max_t: f32, lo: [f32; 3], hi: [f32; 3]) -> bool {
    let mut tmin = 0.0f32;
    let mut tmax = max_t;
    for axis in 0..3 {
        let mut t0 = (lo[axis] - origin[axis]) * inv_dir[axis];
        let mut t1 = (hi[axis] - origin[axis]) * inv_dir[axis];
        if t0 > t1 {
            core::mem::swap(&mut t0, &mut t1);
        }
        tmin = tmin.max(t0);
        tmax = tmax.min(t1);
        if tmax < tmin {
            return false;
        }
    }
    true
}

#[derive(Clone, Copy)]
struct BvhNode {
    lower: [f32; 3],
    upper: [f32; 3],
    left: i32,
    right: i32,
}

#[derive(Clone, Default)]
pub(super) struct QueryIndex {
    pub topology: u64,
    pub state: u64,
    pub shapes: Vec<HostShape>,
    pub groups: Arc<Vec<Vec<usize>>>,
    nodes: Vec<BvhNode>,
}

#[cfg(feature="replay-diagnostics")]
impl QueryIndex {
    pub(super) fn diagnostic_nodes(&self) -> Vec<([f32;3],[f32;3],i32,i32)> {
        self.nodes.iter().map(|n|(n.lower,n.upper,n.left,n.right)).collect()
    }
}

// Group collider slots without changing their internal identities. Public shape
// callbacks operate on the whole compound, including callbacks that do not clip.
fn public_shape_groups(
    shapes: &[HostShape],
    indices: impl IntoIterator<Item = usize>,
) -> Vec<Vec<usize>> {
    let mut lookup = std::collections::HashMap::new();
    let mut groups: Vec<Vec<usize>> = Vec::new();
    for index in indices {
        let id = shapes[index].public_id;
        let key = (id.index1, id.world0, id.generation);
        let group = *lookup.entry(key).or_insert_with(|| {
            groups.push(Vec::new());
            groups.len() - 1
        });
        groups[group].push(index);
    }
    groups
}

fn group_bounds(shapes: &[HostShape], group: &[usize]) -> Aabb {
    let mut bounds = aabb_for_proxy(&shape_proxy(&shapes[group[0]], None));
    for &index in &group[1..] {
        let child = aabb_for_proxy(&shape_proxy(&shapes[index], None));
        bounds.lower_bound = v(bounds.lower_bound).min(v(child.lower_bound)).to_array();
        bounds.upper_bound = v(bounds.upper_bound).max(v(child.upper_bound)).to_array();
    }
    bounds
}

impl QueryIndex {
    pub fn rebuild(shapes: Vec<HostShape>) -> Self {
        let bounds: Vec<Aabb> = shapes
            .iter()
            .map(|shape| aabb_for_proxy(&shape_proxy(shape, None)))
            .collect();
        let mut order: Vec<usize> = (0..shapes.len()).collect();
        let mut nodes = Vec::new();
        if !order.is_empty() {
            let n = order.len();
            build_bvh(&bounds, &mut order, 0, n, &mut nodes);
        }
        Self {
            topology: 0,
            state: 0,
            groups: Arc::new(public_shape_groups(&shapes, 0..shapes.len())),
            shapes,
            nodes,
        }
    }

    pub fn refit(&mut self) {
        if self.nodes.is_empty() {
            return;
        }
        let bounds: Vec<Aabb> = self
            .shapes
            .iter()
            .map(|shape| aabb_for_proxy(&shape_proxy(shape, None)))
            .collect();
        refit_bvh(&mut self.nodes, 0, &bounds);
    }

    pub fn ray_candidates(&self, origin: V3, translation: V3, max_fraction: f32) -> (Vec<usize>, u32) {
        if self.nodes.is_empty() {
            return ((0..self.shapes.len()).collect(), 0);
        }
        let inv = V3::new(
            if translation.x.abs() > 1e-12 {
                1.0 / translation.x
            } else {
                f32::INFINITY
            },
            if translation.y.abs() > 1e-12 {
                1.0 / translation.y
            } else {
                f32::INFINITY
            },
            if translation.z.abs() > 1e-12 {
                1.0 / translation.z
            } else {
                f32::INFINITY
            },
        );
        let mut hits = Vec::new();
        let mut node_visits = 0u32;
        let mut stack = vec![0i32];
        while let Some(index) = stack.pop() {
            let Ok(node_index) = usize::try_from(index) else {
                continue;
            };
            let Some(node) = self.nodes.get(node_index) else {
                continue;
            };
            node_visits += 1;
            if !ray_aabb(origin, inv, max_fraction, node.lower, node.upper) {
                continue;
            }
            if node.right < 0 {
                hits.push((-node.right - 1) as usize);
            } else {
                stack.push(node.left);
                stack.push(node.right);
            }
        }
        hits.sort_unstable();
        (hits, node_visits)
    }

    pub(super) fn snapshot_ray(&self, origin: V3, translation: V3) -> (HostWorld, u32) {
        let (candidates, visits) = self.ray_candidates(origin, translation, 1.0);
        // Own only BVH candidates; immutable geometry stays shared through Arc.
        // External callbacks must never borrow the locked world's query index.
        let shapes: Vec<HostShape> = candidates.into_iter()
            .map(|i| self.shapes[i].clone()).collect();
        let groups = Arc::new(public_shape_groups(&shapes, 0..shapes.len()));
        (HostWorld { shapes, groups }, visits)
    }

    pub(super) fn snapshot_aabb(&self, query: Aabb) -> HostWorld {
        let (candidates, _) = self.aabb_candidates(query);
        // aabb_candidates sorts by collider slot, preserving native child order.
        // Clone only candidates, then release the world lock before callbacks.
        let shapes: Vec<HostShape> = candidates.into_iter()
            .map(|i| self.shapes[i].clone()).collect();
        let groups = Arc::new(public_shape_groups(&shapes, 0..shapes.len()));
        HostWorld { shapes, groups }
    }

    fn aabb_candidates(&self, query: Aabb) -> (Vec<usize>, u32) {
        if self.nodes.is_empty() {
            return ((0..self.shapes.len()).collect(), 0);
        }
        let mut hits = Vec::new();
        let mut node_visits = 0u32;
        let mut stack = vec![0i32];
        while let Some(index) = stack.pop() {
            let Ok(node_index) = usize::try_from(index) else {
                continue;
            };
            let Some(node) = self.nodes.get(node_index) else {
                continue;
            };
            node_visits += 1;
            let node_aabb = Aabb {
                lower_bound: node.lower,
                upper_bound: node.upper,
            };
            if !aabb_overlaps(query, node_aabb) {
                continue;
            }
            if node.right < 0 {
                hits.push((-node.right - 1) as usize);
            } else {
                stack.push(node.left);
                stack.push(node.right);
            }
        }
        hits.sort_unstable();
        (hits, node_visits)
    }
}

fn union_aabb(a: Aabb, b: Aabb) -> Aabb {
    Aabb {
        lower_bound: [
            a.lower_bound[0].min(b.lower_bound[0]),
            a.lower_bound[1].min(b.lower_bound[1]),
            a.lower_bound[2].min(b.lower_bound[2]),
        ],
        upper_bound: [
            a.upper_bound[0].max(b.upper_bound[0]),
            a.upper_bound[1].max(b.upper_bound[1]),
            a.upper_bound[2].max(b.upper_bound[2]),
        ],
    }
}

fn build_bvh(
    bounds: &[Aabb],
    order: &mut [usize],
    start: usize,
    end: usize,
    nodes: &mut Vec<BvhNode>,
) -> i32 {
    let count = end.saturating_sub(start);
    if count == 0 {
        return -1;
    }
    if count == 1 {
        let leaf = order[start];
        let aabb = bounds[leaf];
        let index = nodes.len() as i32;
        nodes.push(BvhNode {
            lower: aabb.lower_bound,
            upper: aabb.upper_bound,
            left: -1,
            right: -(leaf as i32 + 1),
        });
        return index;
    }
    let mut lo = bounds[order[start]];
    for slot in order[start + 1..end].iter() {
        lo = union_aabb(lo, bounds[*slot]);
    }
    let extent = [
        lo.upper_bound[0] - lo.lower_bound[0],
        lo.upper_bound[1] - lo.lower_bound[1],
        lo.upper_bound[2] - lo.lower_bound[2],
    ];
    let axis = extent
        .iter()
        .enumerate()
        .max_by(|a, b| a.1.partial_cmp(b.1).unwrap_or(std::cmp::Ordering::Equal))
        .map(|(i, _)| i)
        .unwrap_or(0);
    order[start..end].sort_by(|a, b| {
        let ca = 0.5 * (bounds[*a].lower_bound[axis] + bounds[*a].upper_bound[axis]);
        let cb = 0.5 * (bounds[*b].lower_bound[axis] + bounds[*b].upper_bound[axis]);
        ca.partial_cmp(&cb).unwrap_or(std::cmp::Ordering::Equal)
    });
    let mid = start + count / 2;
    let index = nodes.len();
    nodes.push(BvhNode {
        lower: lo.lower_bound,
        upper: lo.upper_bound,
        left: -1,
        right: -1,
    });
    let left = build_bvh(bounds, order, start, mid, nodes);
    let right = build_bvh(bounds, order, mid, end, nodes);
    nodes[index].left = left;
    nodes[index].right = right;
    index as i32
}

fn refit_bvh(nodes: &mut [BvhNode], index: i32, bounds: &[Aabb]) -> Aabb {
    if index < 0 {
        return Aabb::default();
    }
    let node = nodes[index as usize];
    let aabb = if node.right < 0 {
        bounds[(-node.right - 1) as usize]
    } else {
        union_aabb(
            refit_bvh(nodes, node.left, bounds),
            refit_bvh(nodes, node.right, bounds),
        )
    };
    nodes[index as usize].lower = aabb.lower_bound;
    nodes[index as usize].upper = aabb.upper_bound;
    aabb
}

fn inverse_transform_point(transform: WorldTransform, point: V3) -> V3 {
    q(transform.q).conjugate() * (point - v(transform.p))
}

fn mesh_triangle(shape: &HostShape, transform: WorldTransform, index: usize) -> Option<[V3; 3]> {
    let mut triangle = *shape.mesh_triangles.get(index)?;
    if shape.mesh_instance.is_some_and(MeshInstance::reflected) {
        triangle.swap(1, 2);
    }
    let mut points = [V3::ZERO; 3];
    for i in 0..3 {
        let raw = *shape.local_points.get(triangle[i] as usize)?;
        let local = shape
            .mesh_instance
            .map_or(raw, |instance| instance.point(raw));
        points[i] = transform_point(transform.p, transform.q, local);
    }
    Some(points)
}

fn mesh_candidates(shape: &HostShape, transform: WorldTransform, world_bounds: Aabb) -> Vec<usize> {
    if shape.mesh_nodes.is_empty() {
        return (0..shape.mesh_triangles.len()).collect();
    }
    let lo = v(world_bounds.lower_bound);
    let hi = v(world_bounds.upper_bound);
    let mut local_lo = V3::splat(f32::MAX);
    let mut local_hi = V3::splat(f32::MIN);
    for x in [lo.x, hi.x] {
        for y in [lo.y, hi.y] {
            for z in [lo.z, hi.z] {
                // Mesh vertices and BVH bounds are body-origin-relative, as is this inverse.
                let p = inverse_transform_point(transform, V3::new(x, y, z));
                let p = shape.mesh_instance.map_or(p, |instance| instance.inverse_point(p));
                local_lo = local_lo.min(p);
                local_hi = local_hi.max(p);
            }
        }
    }
    let mut result = Vec::new();
    let mut stack = vec![0usize];
    while let Some(index) = stack.pop() {
        let Some(node) = shape.mesh_nodes.get(index) else {
            continue;
        };
        if (0..3).any(|axis| node.lower[axis] > local_hi[axis] || local_lo[axis] > node.upper[axis])
        {
            continue;
        }
        if node.data & 3 == 3 {
            let count = node.data >> 2;
            for triangle in node.triangle_offset..node.triangle_offset.saturating_add(count) {
                if (triangle as usize) < shape.mesh_triangles.len() {
                    result.push(triangle as usize);
                }
            }
        } else {
            stack.push(index.saturating_add((node.data >> 2) as usize));
            stack.push(index + 1);
        }
    }
    result
}

fn closest_on_triangle(point: V3, a: V3, b: V3, c: V3) -> V3 {
    let ab = b - a;
    let ac = c - a;
    let ap = point - a;
    let d1 = ab.dot(ap);
    let d2 = ac.dot(ap);
    if d1 <= 0.0 && d2 <= 0.0 {
        return a;
    }
    let bp = point - b;
    let d3 = ab.dot(bp);
    let d4 = ac.dot(bp);
    if d3 >= 0.0 && d4 <= d3 {
        return b;
    }
    let vc = d1 * d4 - d3 * d2;
    if vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0 {
        return a + (d1 / (d1 - d3)) * ab;
    }
    let cp = point - c;
    let d5 = ab.dot(cp);
    let d6 = ac.dot(cp);
    if d6 >= 0.0 && d5 <= d6 {
        return c;
    }
    let vb = d5 * d2 - d1 * d6;
    if vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0 {
        return a + (d2 / (d2 - d6)) * ac;
    }
    let va = d3 * d6 - d5 * d4;
    if va <= 0.0 && d4 - d3 >= 0.0 && d5 - d6 >= 0.0 {
        return b + ((d4 - d3) / ((d4 - d3) + (d5 - d6))) * (c - b);
    }
    let denom = 1.0 / (va + vb + vc);
    a + vb * denom * ab + vc * denom * ac
}

fn mesh_closest_point(shape: &HostShape, transform: WorldTransform, point: V3) -> V3 {
    let bounds = Aabb {
        lower_bound: (point - V3::splat(1.0e6)).to_array(),
        upper_bound: (point + V3::splat(1.0e6)).to_array(),
    };
    let mut closest = point;
    let mut distance = f32::MAX;
    for index in mesh_candidates(shape, transform, bounds) {
        let Some([a, b, c]) = mesh_triangle(shape, transform, index) else {
            continue;
        };
        let candidate = closest_on_triangle(point, a, b, c);
        let d = candidate.distance_squared(point);
        if d < distance {
            distance = d;
            closest = candidate;
        }
    }
    closest
}

fn mesh_ray_cast(
    shape: &HostShape,
    transform: WorldTransform,
    origin: V3,
    translation: V3,
    max_fraction: f32,
) -> CastOutput {
    let end = origin + max_fraction * translation;
    let bounds = Aabb {
        lower_bound: origin.min(end).to_array(),
        upper_bound: origin.max(end).to_array(),
    };
    let mut output = CastOutput::default();
    output.fraction = max_fraction;
    output.triangle_index = -1;
    output.child_index = -1;
    for index in mesh_candidates(shape, transform, bounds) {
        let Some([a, b, c]) = mesh_triangle(shape, transform, index) else {
            continue;
        };
        let normal = (b - a).cross(c - a).normalize_or_zero();
        let denominator = normal.dot(translation);
        if denominator >= -f32::EPSILON {
            continue;
        }
        let fraction = normal.dot(a - origin) / denominator;
        if fraction < 0.0 || fraction > output.fraction {
            continue;
        }
        let point = origin + fraction * translation;
        let closest = closest_on_triangle(point, a, b, c);
        if closest.distance_squared(point) <= 1.0e-8 {
            output.hit = true;
            output.fraction = fraction;
            output.point = point.to_array();
            output.normal = normal.to_array();
            output.triangle_index = shape
                .mesh_triangle_ids
                .get(index)
                .copied()
                .unwrap_or(index as i32);
            output.material_index = (shape.mesh_triangles[index][3] >> 8) as i32;
        }
    }
    output
}

fn mesh_overlap_proxy(shape: &HostShape, transform: WorldTransform, proxy: &Proxy) -> bool {
    let bounds = aabb_for_proxy(proxy);
    let center = proxy.points.iter().copied().sum::<V3>() / proxy.points.len().max(1) as f32;
    mesh_candidates(shape, transform, bounds)
        .into_iter()
        .any(|index| {
            let Some([a, b, c]) = mesh_triangle(shape, transform, index) else {
                return false;
            };
            let normal = (b - a).cross(c - a).normalize_or_zero();
            if normal.dot(center - a) < 0.0 {
                return false;
            }
            let triangle = Proxy {
                points: vec![a, b, c],
                radius: 0.0,
            };
            distance(&triangle, proxy, true).distance < OVERLAP_SLOP
        })
}

fn mesh_shape_cast(
    shape: &HostShape,
    transform: WorldTransform,
    moving: &Proxy,
    translation: V3,
    max_fraction: f32,
    can_encroach: bool,
) -> CastOutput {
    let start = aabb_for_proxy(moving);
    let end = Aabb {
        lower_bound: (v(start.lower_bound) + max_fraction * translation).to_array(),
        upper_bound: (v(start.upper_bound) + max_fraction * translation).to_array(),
    };
    let swept = Aabb {
        lower_bound: v(start.lower_bound).min(v(end.lower_bound)).to_array(),
        upper_bound: v(start.upper_bound).max(v(end.upper_bound)).to_array(),
    };
    let center = moving.points.iter().copied().sum::<V3>() / moving.points.len().max(1) as f32;
    let mut best = CastOutput::default();
    best.fraction = max_fraction;
    best.triangle_index = -1;
    best.child_index = -1;
    for index in mesh_candidates(shape, transform, swept) {
        let Some([a, b, c]) = mesh_triangle(shape, transform, index) else {
            continue;
        };
        let normal = (b - a).cross(c - a).normalize_or_zero();
        if normal.dot(center - a) < 0.0 || normal.dot(translation) >= 0.0 {
            continue;
        }
        let triangle = Proxy {
            points: vec![a, b, c],
            radius: 0.0,
        };
        let mut hit = shape_cast(&triangle, moving, translation, best.fraction, can_encroach);
        if hit.hit && hit.fraction <= best.fraction {
            hit.normal = normal.to_array();
            hit.triangle_index = shape
                .mesh_triangle_ids
                .get(index)
                .copied()
                .unwrap_or(index as i32);
            hit.material_index = (shape.mesh_triangles[index][3] >> 8) as i32;
            best = hit;
        }
    }
    best
}

pub fn b3_shape_get_sphere(id: ShapeId) -> Sphere {
    super::world::query_shape(id)
        .filter(|shape| shape.kind == KIND_SPHERE)
        .map(|shape| Sphere {
            center: shape.local_points[0],
            radius: shape.radius,
        })
        .unwrap_or(Sphere {
            center: [0.0; 3],
            radius: 0.0,
        })
}

pub fn b3_shape_get_capsule(id: ShapeId) -> Capsule {
    super::world::query_shape(id)
        .filter(|shape| shape.kind == KIND_CAPSULE && shape.local_points.len() == 2)
        .map(|shape| Capsule {
            center1: shape.local_points[0],
            center2: shape.local_points[1],
            radius: shape.radius,
        })
        .unwrap_or(Capsule {
            center1: [0.0; 3],
            center2: [0.0; 3],
            radius: 0.0,
        })
}

pub fn b3_shape_get_friction(id: ShapeId) -> f32 {
    super::world::query_shape(id).map_or(0.0, |shape| shape.friction)
}

pub fn b3_shape_get_restitution(id: ShapeId) -> f32 {
    super::world::query_shape(id).map_or(0.0, |shape| shape.restitution)
}

pub fn b3_shape_get_user_data(id: ShapeId) -> usize {
    super::world::query_shape(id).map_or(0, |shape| shape.user_data)
}

fn public_shape_aabb(shape: &HostShape) -> Aabb {
    let mut aabb = aabb_for_proxy(&shape_proxy(shape, None));
    // Box3D exposes the speculative AABB, not the tight geometry bounds.
    for axis in 0..3 {
        aabb.lower_bound[axis] -= crate::types::SPECULATIVE_DISTANCE;
        aabb.upper_bound[axis] += crate::types::SPECULATIVE_DISTANCE;
    }
    aabb
}

pub fn b3_shape_get_aabb(id: ShapeId) -> Aabb {
    super::world::query_shape(id)
        .map(|shape| public_shape_aabb(&shape))
        .unwrap_or_default()
}

pub fn b3_body_compute_aabb(id: BodyId) -> Aabb {
    let Some(body) = super::world::query_body(id) else {
        return Aabb::default();
    };
    if body.shapes.is_empty() {
        return Aabb {
            lower_bound: body.origin,
            upper_bound: body.origin,
        };
    }
    let mut result = b3_shape_get_aabb(body.shapes[0].id);
    for shape in body.shapes.iter().skip(1) {
        let aabb = public_shape_aabb(shape);
        for axis in 0..3 {
            result.lower_bound[axis] = result.lower_bound[axis].min(aabb.lower_bound[axis]);
            result.upper_bound[axis] = result.upper_bound[axis].max(aabb.upper_bound[axis]);
        }
    }
    result
}

pub fn b3_shape_get_closest_point(id: ShapeId, target: [f32; 3]) -> [f32; 3] {
    let Some(shape) = super::world::query_shape(id) else {
        return [0.0; 3];
    };
    if shape.kind == KIND_MESH {
        return mesh_closest_point(
            &shape,
            WorldTransform {
                p: shape.body_origin,
                q: shape.body_rotation,
            },
            v(target),
        )
        .to_array();
    }
    let point = Proxy {
        points: vec![v(target)],
        radius: 0.0,
    };
    let result = distance(&shape_proxy(&shape, None), &point, true);
    if result.distance == 0.0 {
        target
    } else {
        result.point_a.to_array()
    }
}

pub fn b3_body_get_closest_point(id: BodyId, target: [f32; 3]) -> (f32, [f32; 3]) {
    let Some(body) = super::world::query_body(id) else {
        return (0.0, [0.0; 3]);
    };
    let mut closest = body.origin;
    let mut closest_distance = f32::MAX;
    for shape in &body.shapes {
        let point = b3_shape_get_closest_point(shape.id, target);
        let d = v(point).distance(v(target));
        if d < closest_distance {
            closest_distance = d;
            closest = point;
        }
    }
    (closest_distance, closest)
}

// Match native sphere-ray initial-overlap and zero-length conventions. The
// world closest-hit API has a separate fraction-zero filter.
fn direct_sphere_ray(
    center: V3,
    radius: f32,
    origin: V3,
    translation: V3,
    max_fraction: f32,
) -> CastOutput {
    let offset = origin - center;
    let radius2 = radius * radius;
    let mut result = CastOutput::default();
    let length = translation.length();
    if length == 0.0 {
        if offset.length_squared() < radius2 {
            result.hit = true;
            result.point = origin.to_array();
        }
        return result;
    }
    let direction = translation / length;
    let t = -offset.dot(direction);
    let closest = offset + t * direction;
    if closest.length_squared() > radius2 {
        return result;
    }
    let distance = t - (radius2 - closest.length_squared()).sqrt();
    if distance < 0.0 || distance > max_fraction * length {
        if offset.length_squared() < radius2 {
            result.hit = true;
            result.point = origin.to_array();
        }
        return result;
    }
    let relative_hit = offset + distance * direction;
    result.hit = true;
    result.fraction = distance / length;
    result.point = (center + relative_hit).to_array();
    result.normal = relative_hit.normalize_or_zero().to_array();
    result
}

pub fn b3_shape_ray_cast(id: ShapeId, origin: [f32; 3], translation: [f32; 3]) -> CastOutput {
    let mut best = CastOutput::default();
    let mut fraction = 1.0;
    for shape in super::world::query_shape_parts(id) {
        let mut output = if shape.kind == KIND_MESH {
            mesh_ray_cast(
                &shape,
                WorldTransform {
                    p: shape.body_origin,
                    q: shape.body_rotation,
                },
                v(origin),
                v(translation),
                fraction,
            )
        } else if shape.kind == KIND_SPHERE {
            direct_sphere_ray(
                transform_point(
                    shape.body_origin,
                    shape.body_rotation,
                    shape.local_points[0],
                ),
                shape.radius,
                v(origin),
                v(translation),
                fraction,
            )
        } else {
            ray_cast(
                &shape_proxy(&shape, None),
                v(origin),
                v(translation),
                fraction,
            )
        };
        if !output.hit || output.fraction < 0.0 || output.fraction > fraction {
            continue;
        }
        output.child_index = shape.child_index.max(0);
        if shape.kind != KIND_MESH {
            output.triangle_index = 0;
        }
        if shape.child_index >= 0 {
            let local_material = if shape.kind == KIND_MESH {
                output.material_index.clamp(0, 3) as usize
            } else {
                0
            };
            output.material_index = shape.compound_material_indices[local_material];
        }
        fraction = output.fraction;
        best = output;
    }
    best
}

pub fn b3_body_cast_ray(
    id: BodyId,
    origin: [f32; 3],
    translation: [f32; 3],
    filter: QueryFilter,
    max_fraction: f32,
    body_transform: WorldTransform,
) -> BodyCastResult {
    let Some(body) = super::world::query_body(id) else {
        return BodyCastResult::default();
    };
    let mut result = BodyCastResult::default();
    let mut fraction = max_fraction;
    for shape in &body.shapes {
        if !should_query(shape.filter, filter) {
            continue;
        }
        let output = if shape.kind == KIND_MESH {
            mesh_ray_cast(shape, body_transform, v(origin), v(translation), fraction)
        } else {
            ray_cast(
                &shape_proxy(shape, Some(body_transform)),
                v(origin),
                v(translation),
                fraction,
            )
        };
        if output.hit && output.fraction <= fraction {
            fraction = output.fraction;
            result = BodyCastResult {
                shape_id: shape.public_id,
                point: output.point,
                normal: output.normal,
                fraction,
                triangle_index: output.triangle_index,
                user_material_id: shape.user_material_id,
                iterations: output.iterations,
                hit: true,
            };
        }
    }
    result
}

pub unsafe fn b3_body_cast_shape(
    id: BodyId,
    origin: [f32; 3],
    proxy: &ShapeProxy,
    translation: [f32; 3],
    filter: QueryFilter,
    max_fraction: f32,
    can_encroach: bool,
    body_transform: WorldTransform,
) -> BodyCastResult {
    let Some(body) = super::world::query_body(id) else {
        return BodyCastResult::default();
    };
    let moving = input_proxy(proxy, origin);
    let mut result = BodyCastResult::default();
    let mut fraction = max_fraction;
    for shape in &body.shapes {
        if !should_query(shape.filter, filter) {
            continue;
        }
        let output = if shape.kind == KIND_MESH {
            mesh_shape_cast(
                shape,
                body_transform,
                &moving,
                v(translation),
                fraction,
                can_encroach,
            )
        } else {
            shape_cast(
                &shape_proxy(shape, Some(body_transform)),
                &moving,
                v(translation),
                fraction,
                can_encroach,
            )
        };
        if output.hit && output.fraction <= fraction {
            fraction = output.fraction;
            result = BodyCastResult {
                shape_id: shape.public_id,
                point: output.point,
                normal: output.normal,
                fraction,
                triangle_index: output.triangle_index,
                user_material_id: shape.user_material_id,
                iterations: output.iterations,
                hit: true,
            };
        }
    }
    result
}

pub unsafe fn b3_body_overlap_shape(
    id: BodyId,
    origin: [f32; 3],
    proxy: &ShapeProxy,
    filter: QueryFilter,
    body_transform: WorldTransform,
) -> bool {
    let Some(body) = super::world::query_body(id) else {
        return false;
    };
    let query = input_proxy(proxy, origin);
    body.shapes.iter().any(|shape| {
        should_query(shape.filter, filter)
            && if shape.kind == KIND_MESH {
                mesh_overlap_proxy(shape, body_transform, &query)
            } else {
                distance(&shape_proxy(shape, Some(body_transform)), &query, true).distance
                    < OVERLAP_SLOP
            }
    })
}

pub unsafe fn b3_world_overlap_aabb(
    id: WorldId,
    aabb: Aabb,
    filter: QueryFilter,
    callback: Option<OverlapResultFcn>,
    context: *mut c_void,
) -> TreeStats {
    let Some(world) = super::world::query_world(id) else {
        return TreeStats::default();
    };
    let mut stats = TreeStats::default();
    for group in world.groups.iter() {
        let shape = &world.shapes[group[0]];
        stats.node_visits += 1;
        if !should_query(shape.filter, filter) {
            continue;
        }
        if aabb_overlaps(aabb, group_bounds(&world.shapes, group)) {
            stats.leaf_visits += 1;
            if callback.is_some_and(|f| !f(shape.public_id, context)) {
                break;
            }
        }
    }
    stats
}

// Conservative candidate padding covers narrowphase overlap tolerance and
// rounding differences between world-space BVH and origin-relative mover math.
fn query_candidate_bounds(proxy: &Proxy) -> Aabb {
    let mut bounds = aabb_for_proxy(proxy);
    for axis in 0..3 {
        let magnitude = bounds.lower_bound[axis].abs().max(bounds.upper_bound[axis].abs()).max(1.0);
        let pad = OVERLAP_SLOP + 8.0 * f32::EPSILON * magnitude;
        bounds.lower_bound[axis] -= pad;
        bounds.upper_bound[axis] += pad;
    }
    bounds
}

pub unsafe fn b3_world_overlap_shape(
    id: WorldId,
    origin: [f32; 3],
    proxy: &ShapeProxy,
    filter: QueryFilter,
    callback: Option<OverlapResultFcn>,
    context: *mut c_void,
) -> TreeStats {
    let query = input_proxy(proxy, origin);
    let Some(world) = super::world::query_world_aabb(id, query_candidate_bounds(&query)) else {
        return TreeStats::default();
    };
    let mut stats = TreeStats::default();
    for group in world.groups.iter() {
        let shape = &world.shapes[group[0]];
        stats.node_visits += 1;
        if !should_query(shape.filter, filter) {
            continue;
        }
        stats.leaf_visits += 1;
        let overlaps = group.iter().any(|&index| {
            let shape = &world.shapes[index];
            if shape.kind == KIND_MESH {
                mesh_overlap_proxy(
                    shape,
                    WorldTransform {
                        p: shape.body_origin,
                        q: shape.body_rotation,
                    },
                    &query,
                )
            } else {
                distance(&shape_proxy(shape, None), &query, true).distance < OVERLAP_SLOP
            }
        });
        if overlaps && callback.is_some_and(|f| !f(shape.public_id, context)) {
            break;
        }
    }
    stats
}

unsafe fn cast_world(
    world: &HostWorld,
    moving: &Proxy,
    translation: [f32; 3],
    filter: QueryFilter,
    callback: Option<CastResultFcn>,
    context: *mut c_void,
    can_encroach: bool,
) -> TreeStats {
    let mut stats = TreeStats::default();
    let mut max_fraction = 1.0;
    for group in world.groups.iter() {
        let mut best: Option<(&HostShape, CastOutput)> = None;
        let mut child_fraction = max_fraction;
        for &index in group {
            let shape = &world.shapes[index];
            stats.node_visits += 1;
            if !should_query(shape.filter, filter) {
                continue;
            }
            stats.leaf_visits += 1;
            let output = if shape.kind == KIND_MESH {
                mesh_shape_cast(
                    shape,
                    WorldTransform {
                        p: shape.body_origin,
                        q: shape.body_rotation,
                    },
                    moving,
                    v(translation),
                    child_fraction,
                    can_encroach,
                )
            } else {
                shape_cast(
                    &shape_proxy(shape, None),
                    moving,
                    v(translation),
                    child_fraction,
                    can_encroach,
                )
            };
            if !output.hit {
                continue;
            }
            if best.is_none() || output.fraction < child_fraction {
                child_fraction = output.fraction;
                best = Some((shape, output));
            }
        }
        let Some((shape, output)) = best else {
            continue;
        };
        let response = callback.map_or(max_fraction, |f| {
            f(
                shape.public_id,
                super::Vec3::from(output.point),
                super::Vec3::from(output.normal),
                output.fraction,
                shape.user_material_id,
                output.triangle_index,
                shape.child_index,
                context,
            )
        });
        if response == 0.0 {
            break;
        }
        if (0.0..=1.0).contains(&response) {
            max_fraction = response;
        }
    }
    stats
}

pub unsafe fn b3_world_cast_ray(
    id: WorldId,
    origin: [f32; 3],
    translation: [f32; 3],
    filter: QueryFilter,
    callback: Option<CastResultFcn>,
    context: *mut c_void,
) -> TreeStats {
    let Some((snapshot, mut profile)) = super::world::query_ray_snapshot(id, origin, translation) else {
        return TreeStats::default();
    };
    let node_visits = profile.visited_shapes;
    let mut stats = TreeStats {
        node_visits: node_visits as i32,
        leaf_visits: 0,
    };
    let mut max_fraction = 1.0;
    let exact_start = std::time::Instant::now();
    let mut exact = 0u32;
    let mut rejected = 0u32;
    for group in snapshot.groups.iter() {
        let mut best: Option<(&HostShape, CastOutput)> = None;
        let mut child_fraction = max_fraction;
        for &shape_index in group {
            let Some(shape) = snapshot.shapes.get(shape_index) else {
                continue;
            };
            if !should_query(shape.filter, filter) {
                rejected += 1;
                continue;
            }
            stats.leaf_visits += 1;
            exact += 1;
            let output = if shape.kind == KIND_MESH {
                mesh_ray_cast(
                    shape,
                    WorldTransform {
                        p: shape.body_origin,
                        q: shape.body_rotation,
                    },
                    v(origin),
                    v(translation),
                    child_fraction,
                )
            } else {
                ray_cast(
                    &shape_proxy(shape, None),
                    v(origin),
                    v(translation),
                    child_fraction,
                )
            };
            if !output.hit {
                continue;
            }
            if best.is_none() || output.fraction < child_fraction {
                child_fraction = output.fraction;
                best = Some((shape, output));
            }
        }
        let Some((shape, output)) = best else {
            continue;
        };
        let cb_start = std::time::Instant::now();
        let response = callback.map_or(max_fraction, |f| {
            f(
                shape.public_id,
                super::Vec3::from(output.point),
                super::Vec3::from(output.normal),
                output.fraction,
                shape.user_material_id,
                output.triangle_index,
                shape.child_index,
                context,
            )
        });
        profile.callback_ms += cb_start.elapsed().as_secs_f32() * 1e3;
        if response == 0.0 {
            break;
        }
        if (0.0..=1.0).contains(&response) {
            max_fraction = response;
        }
    }
    profile.exact_ms = (exact_start.elapsed().as_secs_f32() * 1e3 - profile.callback_ms).max(0.0);
    profile.exact_casts = exact;
    profile.rejected_shapes = rejected;
    super::world::publish_query_profile(id, profile);
    stats
}

pub unsafe fn b3_world_cast_ray_closest(
    id: WorldId,
    origin: [f32; 3],
    translation: [f32; 3],
    filter: QueryFilter,
) -> RayResult {
    #[cfg(not(target_arch = "wasm32"))]
    {
        super::world::gpu_cast_ray_closest(id, origin, translation, filter)
    }
    #[cfg(target_arch = "wasm32")]
    {
        let _ = (id, origin, translation, filter);
        RayResult::default()
    }
}

pub unsafe fn b3_world_cast_shape(
    id: WorldId,
    origin: [f32; 3],
    proxy: &ShapeProxy,
    translation: [f32; 3],
    filter: QueryFilter,
    callback: Option<CastResultFcn>,
    context: *mut c_void,
) -> TreeStats {
    let moving = input_proxy(proxy, origin);
    let mut end = moving.clone();
    for point in &mut end.points { *point += v(translation); }
    let swept = union_aabb(query_candidate_bounds(&moving), query_candidate_bounds(&end));
    let Some(world) = super::world::query_world_aabb(id, swept) else {
        return TreeStats::default();
    };
    cast_world(
        &world,
        &moving,
        translation,
        filter,
        callback,
        context,
        false,
    )
}

pub unsafe fn b3_world_cast_mover(
    id: WorldId,
    origin: [f32; 3],
    mover: &Capsule,
    translation: [f32; 3],
    filter: QueryFilter,
    callback: Option<MoverFilterFcn>,
    context: *mut c_void,
) -> f32 {
    let Some(world) = super::world::query_world(id) else {
        return 1.0;
    };
    let moving = mover_proxy(origin, mover);
    let start_aabb = aabb_for_proxy(&moving);
    let mut end = moving.clone();
    for point in &mut end.points {
        *point += v(translation);
    }
    let end_aabb = aabb_for_proxy(&end);
    let swept_aabb = Aabb {
        lower_bound: v(start_aabb.lower_bound)
            .min(v(end_aabb.lower_bound))
            .to_array(),
        upper_bound: v(start_aabb.upper_bound)
            .max(v(end_aabb.upper_bound))
            .to_array(),
    };
    let mut fraction = 1.0;
    for group in world.groups.iter() {
        let shape = &world.shapes[group[0]];
        if !should_query(shape.filter, filter) {
            continue;
        }
        if !aabb_overlaps(swept_aabb, group_bounds(&world.shapes, group)) {
            continue;
        }
        if callback.is_some_and(|f| !f(shape.public_id, context)) {
            continue;
        }
        for &index in group {
            let shape = &world.shapes[index];
            let output = if shape.kind == KIND_MESH {
                mesh_shape_cast(
                    shape,
                    WorldTransform {
                        p: shape.body_origin,
                        q: shape.body_rotation,
                    },
                    &moving,
                    v(translation),
                    fraction,
                    mover.radius > 0.0,
                )
            } else {
                shape_cast(
                    &shape_proxy(shape, None),
                    &moving,
                    v(translation),
                    fraction,
                    mover.radius > 0.0,
                )
            };
            if output.hit && output.fraction > 0.0 && output.fraction < fraction {
                fraction = output.fraction;
            }
        }
    }
    fraction
}

pub unsafe fn b3_world_collide_mover(
    id: WorldId,
    origin: [f32; 3],
    mover: &Capsule,
    filter: QueryFilter,
    callback: Option<PlaneResultFcn>,
    context: *mut c_void,
) {
    let world_mover = mover_proxy(origin, mover);
    let Some(world) = super::world::query_world_aabb(id, query_candidate_bounds(&world_mover)) else {
        return;
    };
    let relative_mover = mover_proxy([0.0; 3], mover);
    let mover_aabb = aabb_for_proxy(&relative_mover);
    let origin = v(origin);
    for group in world.groups.iter() {
        let mut planes = Vec::new();
        for &index in group {
            let shape = &world.shapes[index];
            if !should_query(shape.filter, filter) {
                continue;
            }
            let relative_transform = WorldTransform {
                p: (v(shape.body_origin) - origin).to_array(),
                q: shape.body_rotation,
            };
            if !aabb_overlaps(
                mover_aabb,
                aabb_for_proxy(&shape_proxy(shape, Some(relative_transform))),
            ) {
                continue;
            }
            let Some(plane) = collide_mover(shape, relative_transform, &relative_mover) else {
                continue;
            };
            planes.push(plane);
            if planes.len() == 64 {
                break;
            } // Native per-public-shape plane batch capacity.
        }
        if planes.is_empty() {
            continue;
        }
        let shape = &world.shapes[group[0]];
        if callback.is_some_and(|f| {
            !f(
                shape.public_id,
                planes.as_ptr(),
                planes.len() as i32,
                context,
            )
        }) {
            break;
        }
    }
}

pub unsafe fn b3_body_collide_mover(
    id: BodyId,
    output: &mut [BodyPlaneResult],
    origin: [f32; 3],
    mover: &Capsule,
    filter: QueryFilter,
    body_transform: WorldTransform,
) -> usize {
    if output.is_empty() {
        return 0;
    }
    let Some(body) = super::world::query_body(id) else {
        return 0;
    };
    let relative_mover = mover_proxy([0.0; 3], mover);
    let relative_transform = WorldTransform {
        p: (v(body_transform.p) - v(origin)).to_array(),
        q: body_transform.q,
    };
    let mut count = 0;
    for shape in &body.shapes {
        if !should_query(shape.filter, filter) {
            continue;
        }
        let Some(result) = collide_mover(shape, relative_transform, &relative_mover) else {
            continue;
        };
        output[count] = BodyPlaneResult {
            shape_id: shape.public_id,
            result,
        };
        count += 1;
        if count == output.len() {
            break;
        }
    }
    count
}

pub fn query_filter_name(_filter: QueryFilter) -> *const c_char {
    // Tags are carried for ABI compatibility. Recording is intentionally out
    // of scope for the GPU experiment.
    core::ptr::null()
}

#[cfg(test)]
mod tests {
    include!("../fixtures/toi_sweeps.rs");
    use super::*;

    fn proxy(points: &[[f32; 3]], radius: f32) -> Proxy {
        Proxy {
            points: points.iter().copied().map(v).collect(),
            radius,
        }
    }

    #[test]
    fn sphere_ray_hit_miss_and_initial_overlap() {
        let sphere = proxy(&[[0.0, 0.0, 0.0]], 1.0);
        let hit = ray_cast(&sphere, V3::new(-3.0, 0.0, 0.0), V3::X * 6.0, 1.0);
        assert!(hit.hit);
        assert!((hit.fraction - 1.0 / 3.0).abs() < 1.0e-6);
        assert!(hit.normal[0] < -0.99);

        let miss = ray_cast(&sphere, V3::new(-3.0, 0.0, 0.0), V3::Y * 6.0, 1.0);
        assert!(!miss.hit);

        let overlap = ray_cast(&sphere, V3::ZERO, V3::X, 1.0);
        assert!(overlap.hit);
        assert_eq!(overlap.fraction, 0.0);
    }

    #[test]
    fn convex_overlap_and_cast() {
        let box_points = [
            [-1.0, -1.0, -1.0],
            [1.0, -1.0, -1.0],
            [-1.0, 1.0, -1.0],
            [1.0, 1.0, -1.0],
            [-1.0, -1.0, 1.0],
            [1.0, -1.0, 1.0],
            [-1.0, 1.0, 1.0],
            [1.0, 1.0, 1.0],
        ];
        let hull = proxy(&box_points, 0.0);
        let sphere = proxy(&[[2.5, 0.0, 0.0]], 0.5);
        assert!(distance(&hull, &sphere, true).distance > 0.9);
        let hit = shape_cast(&hull, &sphere, V3::new(-2.0, 0.0, 0.0), 1.0, false);
        assert!(hit.hit);
        assert!(hit.fraction > 0.45 && hit.fraction < 0.55);
    }

    #[test]
    fn query_filter_ignores_groups() {
        let shape = Filter {
            category_bits: 0x4,
            mask_bits: 0x2,
            group_index: -9,
        };
        let query = QueryFilter {
            category_bits: 0x2,
            mask_bits: 0x4,
            ..Default::default()
        };
        assert!(should_query(shape, query));
    }

    fn test_shape(kind: u32, points: Vec<[f32; 3]>, radius: f32) -> HostShape {
        HostShape {
            id: ShapeId::default(),
            public_id: ShapeId::default(),
            child_index: -1,
            compound_material_indices: [0; 4],
            kind,
            body_origin: [0.0; 3],
            body_rotation: [0.0, 0.0, 0.0, 1.0],
            local_points: arc_vec(points),
            local_center: [0.0; 3],
            radius,
            half_extents: [0.5; 3],
            hull_planes: arc_vec(Vec::new()),
            hull_topology: arc_vec(Vec::new()),
            mesh_triangles: arc_vec(Vec::new()),
            mesh_triangle_ids: arc_vec(Vec::new()),
            mesh_nodes: arc_vec(Vec::new()),
            mesh_instance: None,
            filter: Filter {
                category_bits: 1,
                mask_bits: u64::MAX,
                group_index: 0,
            },
            friction: 0.0,
            restitution: 0.0,
            user_material_id: 0,
            user_data: 0,
        }
    }

    #[test]
    fn spatial_snapshot_preserves_hits_and_child_order_after_refit() {
        let shapes: Vec<_> = (0..512).map(|i| {
            let mut shape = test_shape(KIND_SPHERE, vec![[0.0; 3]], 1.0);
            shape.id.index1 = i + 1;
            shape.public_id.index1 = 700;
            shape.child_index = i;
            shape.body_origin = [3.0 * i as f32, 0.0, 0.0];
            shape
        }).collect();
        let mut index = QueryIndex::rebuild(shapes);
        for x in [-1.0, 0.0, 1.6000001, 99.0, 767.5, 1533.0] {
            let query = proxy(&[[x, 0.0, 0.0]], 0.6);
            let snapshot = index.snapshot_aabb(query_candidate_bounds(&query));
            let expected: Vec<_> = index.shapes.iter().filter(|shape|
                distance(&shape_proxy(shape, None), &query, true).distance < OVERLAP_SLOP
            ).map(|shape| shape.child_index).collect();
            let actual: Vec<_> = snapshot.shapes.iter().filter(|shape|
                distance(&shape_proxy(shape, None), &query, true).distance < OVERLAP_SLOP
            ).map(|shape| shape.child_index).collect();
            assert_eq!(actual, expected);
            assert!(snapshot.shapes.len() <= 2, "must not copy the 512-child compound");
            assert!(snapshot.groups.len() <= 1);
        }
        for shape in &mut index.shapes { shape.body_origin[0] += 10000.0; }
        index.refit();
        assert!(index.snapshot_aabb(query_candidate_bounds(&proxy(&[[0.0; 3]], 0.6))).shapes.is_empty());
        let moved = index.snapshot_aabb(query_candidate_bounds(&proxy(&[[10000.0, 0.0, 0.0]], 0.6)));
        assert_eq!(moved.shapes[0].child_index, 0);
        assert_eq!(moved.shapes[0].public_id.index1, 700);
    }

    #[test]
    fn ray_snapshot_prunes_and_retains_owned_compound_candidates() {
        let shapes: Vec<_> = (0..512).map(|i| {
            let mut shape = test_shape(KIND_SPHERE, vec![[0.0; 3]], 1.0);
            shape.id.index1 = i + 1;
            shape.public_id.index1 = 700;
            shape.child_index = i;
            shape.body_origin = [3.0 * i as f32, 0.0, 0.0];
            shape
        }).collect();
        let mut index = QueryIndex::rebuild(shapes);
        let (snapshot, visits) = index.snapshot_ray(v([-2.0, 0.0, 0.0]), v([7.0, 0.0, 0.0]));
        assert!(visits > 0);
        assert_eq!(snapshot.shapes.iter().map(|s| s.child_index).collect::<Vec<_>>(), vec![0, 1, 2]);
        assert_eq!(&*snapshot.groups, &vec![vec![0, 1, 2]]);
        // A callback may issue another query after a refit; the outer ownership
        // must remain stable even if the original index is replaced or dropped.
        for shape in &mut index.shapes { shape.body_origin[0] += 10000.0; }
        index.refit();
        assert!(index.snapshot_ray(v([-2.0, 0.0, 0.0]), v([7.0, 0.0, 0.0])).0.shapes.is_empty());
        drop(index);
        assert_eq!(snapshot.shapes[0].body_origin, [0.0; 3]);
        assert_eq!(snapshot.shapes[0].public_id.index1, 700);
        assert!(ray_cast(&shape_proxy(&snapshot.shapes[0], None), v([-2.0, 0.0, 0.0]), v([7.0, 0.0, 0.0]), 1.0).hit);
    }

    #[test]
    fn gpu_convex_sweeps_match_cpu_conservative_advancement() {
        use crate::ccd::{SweepJob,SweepProxy};
        let gpu=pollster::block_on(crate::sim::GpuDevice::new(None)).unwrap();
        let identity=WorldTransform{p:[0.0;3],q:[0.0,0.0,0.0,1.0]};
        let wall=test_shape(crate::types::KIND_BOX,vec![
            [-10.0,-10.0,9.9],[10.0,-10.0,9.9],[-10.0,10.0,9.9],[10.0,10.0,9.9],
            [-10.0,-10.0,10.1],[10.0,-10.0,10.1],[-10.0,10.0,10.1],[10.0,10.0,10.1]],0.0);
        let sphere=test_shape(KIND_SPHERE,vec![[0.0;3]],0.5);
        let capsule=test_shape(KIND_CAPSULE,vec![[-1.0,0.0,0.0],[1.0,0.0,0.0]],0.2);
        let box_shape=test_shape(crate::types::KIND_BOX,vec![
            [-1.0,-0.1,-0.1],[1.0,-0.1,-0.1],[-1.0,0.1,-0.1],[1.0,0.1,-0.1],
            [-1.0,-0.1,0.1],[1.0,-0.1,0.1],[-1.0,0.1,0.1],[1.0,0.1,0.1]],0.0);
        let mut cases=Vec::new();
        for z in [8.0,9.403,9.405,10.0,12.0] {
            for end_z in [8.0,9.403,10.0,12.0] {
                cases.push((&wall,identity,identity,&sphere,
                    WorldTransform{p:[0.0,0.0,z],..identity},WorldTransform{p:[2.0,0.0,end_z],..identity},512));
            }
        }
        // Non-monotonic rotational motion, capsules, core hulls, swapped order.
        for shape in [&capsule,&box_shape] {
            for angle in [0.2,0.7,1.4,2.5] {
                let start=WorldTransform{p:[0.0,0.0,9.0],..identity};
                let end=WorldTransform{q:Quat::from_rotation_y(angle).to_array(),..start};
                cases.push((&wall,identity,identity,shape,start,end,512));
                cases.push((shape,start,end,&wall,identity,identity,512));
            }
        }
        // Deterministic convex/convex sweeps, including translating and rotating
        // targets, off-axis impacts, and broadphase-sized world coordinates.
        let mut seed=0x6d2b79f5u32;
        let mut random=|| { seed=seed.wrapping_mul(1664525).wrapping_add(1013904223); (seed>>8) as f32 / 16777216.0 };
        for i in 0..128 {
            let offset=if i%4==0 { [100.0,-30.0,50.0] } else { [0.0;3] };
            let a0=WorldTransform{p:offset,q:Quat::from_rotation_z(random()*2.0).to_array()};
            let a1=WorldTransform{p:[offset[0]+random()*0.4,offset[1],offset[2]],q:Quat::from_rotation_z(random()*2.0).to_array()};
            let b0=WorldTransform{p:[offset[0]-3.0,offset[1]+random()*2.0-1.0,offset[2]+random()*0.4],q:Quat::from_rotation_y(random()*2.0).to_array()};
            let b1=WorldTransform{p:[offset[0]+3.0,offset[1]+random()*2.0-1.0,offset[2]+random()*0.4],q:Quat::from_rotation_x(random()*2.0).to_array()};
            cases.push((&box_shape,a0,a1,if i%2==0 {&capsule} else {&box_shape},b0,b1,512));
        }
        for limit in [0,1,512] {
            cases.push((&sphere,identity,identity,&sphere,
                WorldTransform{p:[5.0,0.0,0.0],..identity},
                WorldTransform{q:[0.0,1.0,0.0,0.0],..identity},limit));
        }
        let mut points=Vec::new(); let mut jobs=Vec::new(); let mut expected=Vec::new();
        for &(a,a0,a1,b,b0,b1,iterations) in &cases {
            let mut proxy=|shape:&HostShape| {
                let first=points.len() as u32;
                points.extend(shape.local_points.iter().map(|p|[p[0],p[1],p[2],0.0]));
                SweepProxy{first,count:shape.local_points.len() as u32,radius:shape.radius,unused:0}
            };
            jobs.push(SweepJob{a:proxy(a),b:proxy(b),a0:a0.into(),a1:a1.into(),b0:b0.into(),b1:b1.into(),max_fraction:1.0,iterations,unused:[0;2]});
            expected.push(rigid_time_of_impact_bounded(a,a0,a1,b,b0,b1,1.0,iterations as usize));
        }
        let actual=crate::ccd::test_sweeps(&gpu,&points,&jobs);
        for (i,(gpu,cpu)) in actual.iter().zip(expected).enumerate() {
            assert_eq!(gpu.status!=0,cpu.is_some(),"case {i}: {gpu:?} CPU {cpu:?}");
            if let Some(cpu)=cpu {
                assert!((gpu.fraction-cpu.fraction).abs()<2e-4,"case {i}: {gpu:?} CPU {cpu:?}");
                assert!(gpu.point.iter().chain(gpu.normal.iter()).all(|x|x.is_finite()));
                // Witness points may differ along a shared planar feature;
                // the fraction and supporting normal define the same stop.
                if v(cpu.normal).length_squared()>0.5 && v([gpu.normal[0],gpu.normal[1],gpu.normal[2]]).length_squared()>0.5 {
                    assert!(v(cpu.normal).dot(v([gpu.normal[0],gpu.normal[1],gpu.normal[2]]))>0.99,"case {i} normal: {gpu:?} CPU {cpu:?}");
                }
            }
        }
        assert_eq!(actual[actual.len()-3].status,2,"zero iteration budget must report unresolved, never miss");
        assert_eq!(actual[actual.len()-2].status,2,"exhausted advancement must report unresolved");
    }

    #[test]
    fn ccd_targets_separation_instead_of_outer_tolerance_shell() {
        let wall = test_shape(crate::types::KIND_BOX, vec![
            [-10.0, -10.0, 9.9], [10.0, -10.0, 9.9],
            [-10.0, 10.0, 9.9], [10.0, 10.0, 9.9],
            [-10.0, -10.0, 10.1], [10.0, -10.0, 10.1],
            [-10.0, 10.0, 10.1], [10.0, 10.0, 10.1],
        ], 0.0);
        let sphere = test_shape(KIND_SPHERE, vec![[0.0; 3]], 0.5);
        let identity = WorldTransform { p: [0.0; 3], q: [0.0, 0.0, 0.0, 1.0] };
        let start = WorldTransform { p: [0.0, 0.0, 8.0], ..identity };
        let end = WorldTransform { p: [2.0, 0.0, 10.0], ..identity };
        let hit = rigid_time_of_impact(&wall, identity, identity, &sphere, start, end, 1.0).unwrap();
        // Native target = radius - linear slop = .495, hence center z=9.405.
        assert!((hit.fraction - 0.7025).abs() < 2e-5, "fraction={}", hit.fraction);
        let contact = interpolate_transform(start, end, hit.fraction);
        let tangent = WorldTransform { p: [contact.p[0] + 2.0, 0.0, contact.p[2]], ..identity };
        assert!(rigid_time_of_impact(&wall, identity, identity, &sphere, contact, tangent, 1.0).is_none(),
                "existing contact must not truncate a tangential sweep");
    }

    #[test]
    fn near_shell_parallel_ccd_sweep_is_not_truncated() {
        let wall = test_shape(crate::types::KIND_BOX, vec![
            [-10.0,-10.0,9.9],[10.0,-10.0,9.9],[-10.0,10.0,9.9],[10.0,10.0,9.9],
            [-10.0,-10.0,10.1],[10.0,-10.0,10.1],[-10.0,10.0,10.1],[10.0,10.0,10.1],
        ],0.0);
        let sphere = test_shape(KIND_SPHERE, vec![[0.0;3]],0.5);
        let identity = WorldTransform {p:[0.0;3],q:[0.0,0.0,0.0,1.0]};
        let start = WorldTransform {p:[0.0,0.0,9.403],..identity};
        let end = WorldTransform {p:[2.0,0.0,9.403],..identity};
        let hit = rigid_time_of_impact(&wall,identity,identity,&sphere,start,end,1.0);
        assert!(hit.is_none(), "parallel sweep has no TOI: {hit:?}");
        let closing = WorldTransform {p:[2.0,0.0,10.0],..identity};
        let hit = rigid_time_of_impact(&wall,identity,identity,&sphere,start,closing,1.0).unwrap();
        // Executed native oracle: hit fraction 0.00334964064.
        assert!((hit.fraction-0.00334964064).abs()<2e-5, "closing fraction={}",hit.fraction);
        assert!(rigid_time_of_impact(&sphere,start,end,&wall,identity,identity,1.0).is_none());
        let negative_q = WorldTransform {q:[0.0,0.0,0.0,-1.0],..end};
        assert!(rigid_time_of_impact(&wall,identity,identity,&sphere,start,negative_q,1.0).is_none());
        let moving_wall = WorldTransform {p:[3.0,0.0,0.0],..identity};
        let moving_sphere = WorldTransform {p:[5.0,0.0,9.403],..identity};
        assert!(rigid_time_of_impact(&wall,identity,moving_wall,&sphere,start,moving_sphere,1.0).is_none());
        let rotating_wall = WorldTransform {q:[0.0,0.00001,0.0,1.0],..identity};
        assert!(!fixed_orientation_sweep_separated(&wall,identity,rotating_wall,&sphere,start,end,
                                                   1.0,V3::NEG_Z,0.495), "tiny rotations must not use the fixed-plane proof");
    }

    #[test]
    fn exhausted_toi_keeps_the_last_safe_advance() {
        let sphere = test_shape(KIND_SPHERE, vec![[0.0;3]], 1.0);
        let a = WorldTransform { p: [0.0;3], q: [0.0,0.0,0.0,1.0] };
        let start = WorldTransform { p: [5.0,0.0,0.0], ..a };
        // Rotation deliberately inflates the conservative bound for a sphere.
        let end = WorldTransform { p: [0.0;3], q: [0.0,1.0,0.0,0.0] };
        let hit = rigid_time_of_impact_bounded(&sphere,a,a,&sphere,start,end,1.0,1)
            .expect("iteration exhaustion must not become a miss");
        assert!(hit.fraction > 0.0 && hit.fraction < 0.6);
        assert!(5.0 * (1.0-hit.fraction) > 2.0, "last advance was not separated");
        assert!(hit.normal.iter().all(|v| v.is_finite()));
        let away = WorldTransform { p: [6.0,0.0,0.0], ..end };
        assert!(rigid_time_of_impact(&sphere,a,a,&sphere,start,away,1.0).is_none());
    }

    #[test]
    fn mesh_ccd_rotating_capsule_matches_native_root_tolerance() {
        // Independent native TOI from Falling Ragdolls frame-87 impact replay.
        // Exact-distance advancement stopped at .24536, beyond CPU's accepted
        // root (.197007388, distance .0711199418 with a .07 target).
        let mesh = test_shape(KIND_MESH, vec![
            [1.79328263,4.31570101,-0.38371712],
            [0.0,4.67345047,-0.38371712], [0.0,4.75,0.0],
        ],0.0);
        let center = V3::new(0.0380314998,0.00741250021,-0.203988001);
        let capsule = test_shape(KIND_CAPSULE, vec![
            (V3::new(-0.00182,0.0,0.0100710001)-center).to_array(),
            (V3::new(0.0778829977,0.0148250004,-0.418047011)-center).to_array(),
        ],0.075);
        let mesh_pose = WorldTransform { p:[-0.661173344,-4.884233,0.073975563], q:[0.0,0.0,0.0,1.0] };
        let start = WorldTransform { p:[0.0;3], q:[-0.648249686,0.0804651082,-0.0817971975,0.752733052] };
        let end = WorldTransform { p:[0.0185241699,-0.0609383583,0.0230240822], q:[-0.433602571,0.0712451488,-0.0161264017,0.898138642] };
        for offset in [V3::ZERO,V3::new(7.5,0.0,7.5)] {
            let shift = |x:WorldTransform| WorldTransform { p:(v(x.p)+offset).to_array(), ..x };
            let hit = toi::time_of_impact(&mesh,shift(mesh_pose),shift(mesh_pose),&capsule,shift(start),shift(end),1.0)
                .expect("rotating capsule must hit the triangle edge");
            assert!((hit.fraction-0.197007388).abs()<2e-5, "offset={offset:?}, hit={hit:?}");
        }
    }

    #[test]
    fn mesh_ccd_keeps_positive_toi_with_degenerate_witness_normal() {
        // Seed 52977 Mesh Drop, body 593 at the start/end of step 60.
        let mut mesh = test_shape(KIND_MESH,
            vec![[2.0,0.0,0.0],[2.0,-0.172874376,-1.0],[1.0,-0.279481649,-1.0]], 0.0);
        mesh.mesh_triangles = arc_vec(vec![[0,1,2,0]]);
        let mut points = Vec::new();
        for x in [-0.02,0.02] { for y in [-0.2,0.2] { for z in [-0.04,0.04] { points.push([x,y,z]); } } }
        let mut thin = test_shape(crate::types::KIND_BOX, points, 0.0);
        thin.half_extents = [0.02,0.2,0.04];
        let identity = WorldTransform { p: [0.0;3], q: [0.0,0.0,0.0,1.0] };
        let start = WorldTransform { p: [1.8409071,-0.0489229038,-0.52459085], q: [-0.632165909,0.730147243,0.252432853,-0.0594050735] };
        let end = WorldTransform { p: [1.80890012,-0.154382035,-0.531171501], q: [-0.783485293,0.544712007,0.0601755306,-0.29294771] };
        let hits = rigid_mesh_time_of_impacts(&mesh, identity, &thin, start, end, [0.0;3], 1.0, false, false);
        assert_eq!(hits.len(), 1, "discarded conservative forward hit at contact onset");
        // Executed native b3TimeOfImpact for these proxies/sweeps returns
        // 0.0439814366 (distance .00492012). The old <.01 check encoded the
        // premature outer-tolerance clamp, not the native target crossing.
        assert!((hits[0].hit.fraction - 0.0439814366).abs() < 0.005);
        let at_hit = interpolate_transform(start, end, hits[0].hit.fraction);
        let separation = distance(&shape_proxy(&mesh, Some(identity)),
            &shape_proxy(&thin, Some(at_hit)), false).distance;
        assert!(separation > 0.0 && (separation - LINEAR_SLOP).abs() < 1e-4,
                "TOI must stop separated at the target: {separation}");
        assert!(hits[0].hit.normal[1] > 0.97);
    }

    #[test]
    fn mesh_ccd_glancing_rotation_uses_centroid_plane_filter_except_for_sensors() {
        let mut mesh = test_shape(KIND_MESH,
            vec![[-10.0,0.0,-10.0],[0.0,0.0,10.0],[10.0,0.0,-10.0]], 0.0);
        mesh.mesh_triangles = arc_vec(vec![[0,1,2,0]]);
        let capsule = test_shape(KIND_CAPSULE, vec![[-2.0,0.0,0.0],[2.0,0.0,0.0]], 0.1);
        let identity = WorldTransform { p: [0.0;3], q: [0.0,0.0,0.0,1.0] };
        let start = WorldTransform { p: [0.0,1.0,0.0], ..identity };
        let end = WorldTransform { q: glam::Quat::from_rotation_z(std::f32::consts::FRAC_PI_2).to_array(), ..start };
        // The tip crosses the triangle but the centroid remains one metre
        // above it. Native solid-mesh CCD leaves this to discrete contacts;
        // sensor sweeps still report the crossing.
        assert!(rigid_mesh_time_of_impacts(&mesh, identity, &capsule, start, end, [0.0;3], 1.0, false, false).is_empty());
        let hits = rigid_mesh_time_of_impacts(&mesh, identity, &capsule, start, end, [0.0;3], 1.0, true, true);
        assert_eq!(hits.len(), 1);
        assert!(hits[0].hit.fraction > 0.0 && hits[0].hit.fraction < 1.0);
    }

    #[test]
    fn mesh_ccd_initial_tip_contact_still_stops_centroid_crossing() {
        let mut mesh = test_shape(KIND_MESH, vec![[-2.0,0.0,-2.0],[0.0,0.0,2.0],[2.0,0.0,-2.0]], 0.0);
        mesh.mesh_triangles = arc_vec(vec![[0,1,2,0]]);
        let mut points = Vec::new();
        for x in [-0.02,0.02] { for y in [-0.2,0.2] { for z in [-0.04,0.04] { points.push([x,y,z]); } } }
        let mut thin = test_shape(crate::types::KIND_BOX, points, 0.0);
        thin.half_extents = [0.02,0.2,0.04];
        let identity = WorldTransform { p: [0.0;3], q: [0.0,0.0,0.0,1.0] };
        let start = WorldTransform { p: [0.0,0.2,0.0], ..identity };
        let end = WorldTransform { p: [0.0,-0.2,0.0], ..identity };
        let hits = rigid_mesh_time_of_impacts(&mesh, identity, &thin, start, end, [0.0;3], 1.0, false, false);
        assert_eq!(hits.len(), 1, "initial tip contact must not bypass mesh CCD");
        assert!(hits[0].hit.fraction > 0.4 && hits[0].hit.fraction < 0.5);
        assert!(hits[0].hit.normal[1] > 0.99);
        let away = WorldTransform { p: [0.0,0.6,0.0], ..identity };
        assert!(rigid_mesh_time_of_impacts(&mesh, identity, &thin, start, away, [0.0;3], 1.0, false, false).is_empty());
        assert!(rigid_mesh_time_of_impacts(&mesh, identity, &thin, end, start, [0.0;3], 1.0, false, false).is_empty());
    }

    #[test]
    fn mover_deep_overlap_primitives_and_hull() {
        let transform = WorldTransform {
            p: [0.0; 3],
            q: [0.0, 0.0, 0.0, 1.0],
        };
        let mover = proxy(&[[-1.0, 0.0, 0.0], [1.0, 0.0, 0.0]], 0.2);
        let sphere = test_shape(KIND_SPHERE, vec![[0.0; 3]], 0.5);
        let sphere_plane = collide_mover(&sphere, transform, &mover).unwrap();
        assert!((v(sphere_plane.plane.normal.into()).length() - 1.0).abs() < 1.0e-6);
        assert!((sphere_plane.plane.offset - 0.7).abs() < 1.0e-5);

        let capsule = test_shape(KIND_CAPSULE, vec![[0.0, 0.0, -1.0], [0.0, 0.0, 1.0]], 0.3);
        let capsule_plane = collide_mover(&capsule, transform, &mover).unwrap();
        assert!((v(capsule_plane.plane.normal.into()).length() - 1.0).abs() < 1.0e-6);
        assert!((capsule_plane.plane.offset - 0.5).abs() < 1.0e-5);

        let mut hull = test_shape(
            crate::types::KIND_CONVEX_HULL,
            vec![
                [-0.5, -0.5, -0.5],
                [0.5, -0.5, -0.5],
                [-0.5, 0.5, -0.5],
                [0.5, 0.5, -0.5],
                [-0.5, -0.5, 0.5],
                [0.5, -0.5, 0.5],
                [-0.5, 0.5, 0.5],
                [0.5, 0.5, 0.5],
            ],
            0.0,
        );
        hull.hull_planes = arc_vec(vec![
            [1.0, 0.0, 0.0, 0.5],
            [-1.0, 0.0, 0.0, 0.5],
            [0.0, 1.0, 0.0, 0.5],
            [0.0, -1.0, 0.0, 0.5],
            [0.0, 0.0, 1.0, 0.5],
            [0.0, 0.0, -1.0, 0.5],
        ]);
        assert!(collide_mover(&hull, transform, &mover).is_none(),
            "Box3D deliberately returns no hull plane for axis penetration");
        // A separated axis closer than linear slop still has a valid plane.
        let shallow = proxy(&[[-0.2, 0.501, 0.0], [0.2, 0.501, 0.0]], 0.2);
        let plane = collide_mover(&hull, transform, &shallow).unwrap();
        assert!(v(plane.plane.normal.into()).y > 0.999);
        assert!((plane.plane.offset - 0.199).abs() < 1.0e-5);
    }

    #[test]
    fn plane_solver_and_clipping_match_box3d() {
        let mut planes = [
            CollisionPlane {
                plane: Plane {
                    normal: [1.0, 0.0, 0.0].into(),
                    offset: 0.2,
                },
                push_limit: f32::MAX,
                push: 9.0,
                clip_velocity: true,
            },
            CollisionPlane {
                plane: Plane {
                    normal: [0.0, 1.0, 0.0].into(),
                    offset: 0.3,
                },
                push_limit: 0.1,
                push: 9.0,
                clip_velocity: true,
            },
        ];
        let result = b3_solve_planes([0.0; 3], &mut planes);
        let delta: [f32; 3] = result.delta.into();
        assert!((delta[0] - 0.195).abs() < 1.0e-6);
        assert!((delta[1] - 0.1).abs() < 1.0e-6);
        assert_eq!(b3_clip_vector([-2.0, -3.0, 1.0], &planes), [0.0, 0.0, 1.0]);
    }
    #[test]
    fn direct_compound_ray_rejects_stale_parent() {
        use crate::api::*;
        let gpu = pollster::block_on(crate::sim::GpuDevice::new(None)).expect("gpu");
        let world = b3_create_world(gpu, &b3_default_world_def());
        let body = b3_create_body(world, &b3_default_body_def());
        let def = b3_default_shape_def();
        let parent = b3_create_compound_parent(body, &def);
        let child = b3_create_sphere_shape(
            body,
            &def,
            &Sphere {
                center: [0.0; 3],
                radius: 0.5,
            },
        );
        assert!(b3_shape_attach_compound_child(parent, child));
        let cast = |id| b3_shape_ray_cast(id, [-2.0, 0.0, 0.0], [4.0, 0.0, 0.0]);
        assert!(cast(parent).hit);
        assert_eq!(b3_shape_get_density(parent), def.density);
        let stale = ShapeId {
            generation: parent.generation.wrapping_add(1),
            ..parent
        };
        assert!(!cast(stale).hit);
        assert_eq!(b3_shape_get_density(stale), 0.0);
        b3_destroy_shape(parent, false);
        assert!(!cast(parent).hit);
        let replacement = b3_create_compound_parent(body, &def);
        let child = b3_create_sphere_shape(
            body,
            &def,
            &Sphere {
                center: [0.0; 3],
                radius: 0.5,
            },
        );
        assert!(b3_shape_attach_compound_child(replacement, child));
        assert!(cast(replacement).hit);
        assert!(!cast(parent).hit);
        b3_destroy_world(world);
    }

}
