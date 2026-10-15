//! Conservatively reject mesh BVH nodes outside a translating proxy's sweep.
use super::*;

// Intersection interval of two axis boxes, one translating. f64 slab division
// handles zero directions and endpoint/tangent equality without reciprocal NaNs.
fn interval(
    start: Aabb,
    delta: V3,
    max_fraction: f64,
    lo: [f64; 3],
    hi: [f64; 3],
    pad: f64,
) -> Option<(f64, f64)> {
    let mut enter = 0.0f64;
    let mut exit = max_fraction;
    for i in 0..3 {
        let a = lo[i] - f64::from(start.upper_bound[i]) - pad;
        let b = hi[i] - f64::from(start.lower_bound[i]) + pad;
        let d = f64::from(delta[i]);
        if d == 0.0 {
            if a > 0.0 || b < 0.0 {
                return None;
            }
            continue;
        }
        let (mut t0, mut t1) = (a / d, b / d);
        if t0 > t1 {
            std::mem::swap(&mut t0, &mut t1);
        }
        enter = enter.max(t0);
        exit = exit.min(t1);
        if enter > exit {
            return None;
        }
    }
    Some((enter, exit))
}

// The same affine transforms as mesh_triangle, with outward roundoff padding
// enclosing transformed interior vertices as well as the eight box corners.
fn node_world_bounds(
    shape: &HostShape,
    transform: WorldTransform,
    node: &crate::api::world::MeshNode,
) -> Option<([f64; 3], [f64; 3])> {
    if !node.lower.into_iter().chain(node.upper).all(f32::is_finite) {
        return None;
    }
    let mut lo = V3::splat(f32::INFINITY);
    let mut hi = V3::splat(f32::NEG_INFINITY);
    let mut magnitude = 1.0
        + transform
            .p
            .into_iter()
            .map(|x| f64::from(x).abs())
            .sum::<f64>();
    let raw = node
        .lower
        .into_iter()
        .chain(node.upper)
        .map(|x| f64::from(x).abs())
        .sum::<f64>();
    let rotation = transform
        .q
        .into_iter()
        .map(|x| f64::from(x).abs())
        .sum::<f64>();
    magnitude += raw * (1.0 + 8.0 * rotation * rotation);
    if let Some(instance) = shape.mesh_instance {
        let r = instance
            .rotation
            .into_iter()
            .map(|x| f64::from(x).abs())
            .sum::<f64>();
        magnitude += instance
            .position
            .into_iter()
            .map(|x| f64::from(x).abs())
            .sum::<f64>()
            * (1.0 + 8.0 * rotation * rotation);
        magnitude += raw
            * instance
                .scale
                .into_iter()
                .map(|x| f64::from(x).abs())
                .sum::<f64>()
            * (1.0 + 8.0 * r * r)
            * (1.0 + 8.0 * rotation * rotation);
    }
    for x in [node.lower[0], node.upper[0]] {
        for y in [node.lower[1], node.upper[1]] {
            for z in [node.lower[2], node.upper[2]] {
                let p = shape
                    .mesh_instance
                    .map_or([x, y, z], |i| i.point([x, y, z]));
                let p = transform_point(transform.p, transform.q, p);
                lo = lo.min(p);
                hi = hi.max(p);
            }
        }
    }
    let pad = 64.0 * f64::from(f32::EPSILON) * magnitude;
    if !pad.is_finite() || !lo.is_finite() || !hi.is_finite() {
        return None;
    }
    Some((
        lo.to_array().map(|x| f64::from(x) - pad),
        hi.to_array().map(|x| f64::from(x) + pad),
    ))
}

pub(super) fn mesh_cast_candidates(
    shape: &HostShape,
    transform: WorldTransform,
    moving: &Proxy,
    translation: V3,
    max_fraction: f32,
) -> Vec<usize> {
    let start = aabb_for_proxy(moving);
    let end = Aabb {
        lower_bound: (v(start.lower_bound) + max_fraction * translation).to_array(),
        upper_bound: (v(start.upper_bound) + max_fraction * translation).to_array(),
    };
    let swept = union_aabb(start, end);
    let fallback = || mesh_candidates(shape, transform, swept);
    if shape.mesh_nodes.is_empty()
        || moving.points.is_empty()
        || !moving.radius.is_finite()
        || moving.radius < 0.0
        || !(0.0..=1.0).contains(&max_fraction)
        || !start
            .lower_bound
            .into_iter()
            .chain(start.upper_bound)
            .chain(transform.p)
            .chain(transform.q)
            .all(f32::is_finite)
        || !translation.is_finite()
    {
        return fallback();
    }
    let Some((root_lo, root_hi)) = node_world_bounds(shape, transform, &shape.mesh_nodes[0]) else {
        return fallback();
    };
    let magnitude = start
        .lower_bound
        .into_iter()
        .chain(start.upper_bound)
        .map(|x| f64::from(x).abs())
        .sum::<f64>()
        + root_lo
            .into_iter()
            .chain(root_hi)
            .map(f64::abs)
            .sum::<f64>()
        + 1.0;
    let motion = translation
        .to_array()
        .into_iter()
        .map(|x| f64::from(x).abs())
        .sum::<f64>();
    // Coarse root clipping covers float32 p + fraction*translation even for a
    // huge end point. Its exit then bounds rounding near the actual mesh.
    let shell = 1.25 * f64::from(LINEAR_SLOP);
    let coarse_pad =
        shell + 64.0 * f64::from(f32::EPSILON) * (magnitude + f64::from(max_fraction) * motion);
    let Some((_, exit)) = interval(
        start,
        translation,
        f64::from(max_fraction),
        root_lo,
        root_hi,
        coarse_pad,
    ) else {
        return Vec::new();
    };
    let pad = shell + 64.0 * f64::from(f32::EPSILON) * (magnitude + exit * motion);
    mesh_candidates_filtered(shape, transform, swept, |node| {
        node_world_bounds(shape, transform, node)
            .is_none_or(|(lo, hi)| interval(start, translation, exit, lo, hi, pad).is_some())
    })
}
