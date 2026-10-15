//! Conservative bounds for the COM-linear, quaternion-nlerp mesh CCD sweep.
use super::*;
pub(super) fn mesh_convex_sweep_bounds(
    convex: &HostShape, start: WorldTransform, end: WorldTransform, com: [f32; 3],
) -> Option<Aabb> {
    let norm = |a: [f32; 4]| a.into_iter().map(|x| f64::from(x).powi(2)).sum::<f64>().sqrt();
    let na = norm(start.q);
    let nb = norm(end.q);
    if !(0.99..=1.01).contains(&na) || !(0.99..=1.01).contains(&nb)
        || convex.local_points.is_empty() || !convex.radius.is_finite() || convex.radius < 0.0
        || !start.p.into_iter().chain(end.p).chain(com).all(f32::is_finite) {
        return None;
    }
    // Same sign choice as the actual TOI nlerp, including its float32 dot order.
    let dot = ((start.q[0]*end.q[0]+start.q[1]*end.q[1])+start.q[2]*end.q[2])+start.q[3]*end.q[3];
    let sign = if dot < 0.0 { -1.0 } else { 1.0 };
    let chord = (0..4).map(|i| (sign*f64::from(start.q[i])/na-f64::from(end.q[i])/nb).powi(2)).sum::<f64>().sqrt();
    // A normalized positive blend stays on the endpoint arc. Its distance from
    // the normalized start is at most this endpoint chord. Include the raw
    // start's norm defect and float32 blend/square/sqrt/divide rounding.
    let delta = chord + (na-1.0).abs() + 256.0*f64::from(f32::EPSILON);
    let center = v(com);
    let rotated_center = toi::rotate_q(start.q, center);
    let mut lo = [f64::INFINITY;3];
    let mut hi = [f64::NEG_INFINITY;3];
    let mut add = |point: [f32;3], radius: f64| -> Option<()> {
        if !point.into_iter().all(f32::is_finite) || !radius.is_finite() { return None; }
        let r = (0..3).map(|i| (f64::from(point[i])-f64::from(com[i])).powi(2)).sum::<f64>().sqrt();
        let anchor = toi::rotate_q(start.q, v(point))-rotated_center;
        // For R(q)p = p + 2 v x (v x p + w p), the two cross products give
        // |R(q)p-R(a)p| <= 2 sqrt(2)(|q|+|a|)|p||q-a| < 8|p||q-a|.
        // The exact rotation is linear in p, so use the COM-relative radius;
        // separately cover rounding of the two original-frame rotations.
        let magnitude = 1.0 + point.into_iter().chain(com).chain(start.p).chain(end.p)
            .map(|x| f64::from(x).abs()).sum::<f64>();
        let padding = 8.0*r*delta + radius + 128.0*f64::from(f32::EPSILON)*magnitude;
        for i in 0..3 {
            lo[i] = lo[i].min(f64::from(start.p[i].min(end.p[i]))+f64::from(anchor[i])-padding);
            hi[i] = hi[i].max(f64::from(start.p[i].max(end.p[i]))+f64::from(anchor[i])+padding);
        }
        Some(())
    };
    // Preserve the TOI target/tolerance shell and the native centroid retry.
    let shell = f64::from(LINEAR_SLOP)*1.25;
    for &point in convex.local_points.iter() { add(point, f64::from(convex.radius)+shell)?; }
    let min_extent = if matches!(convex.kind, KIND_SPHERE | KIND_CAPSULE) {
        convex.radius
    } else if !convex.hull_planes.is_empty() {
        convex.hull_planes.iter().map(|p| p[3]-v([p[0],p[1],p[2]]).dot(v(convex.local_center))).fold(f32::MAX,f32::min)
    } else { convex.half_extents.into_iter().fold(f32::MAX,f32::min) };
    let fallback = (0.5*min_extent).max(LINEAR_SLOP)+LINEAR_SLOP;
    add(convex.local_center, f64::from(fallback)+shell)?;
    let lower_bound = lo.map(|x| x as f32);
    let upper_bound = hi.map(|x| x as f32);
    lower_bound.into_iter().chain(upper_bound).all(f32::is_finite)
        .then_some(Aabb { lower_bound, upper_bound })
}
