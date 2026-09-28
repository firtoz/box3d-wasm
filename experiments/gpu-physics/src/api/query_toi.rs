// SPDX-FileCopyrightText: 2026 Erin Catto
// SPDX-License-Identifier: MIT
// Separating-axis TOI adapted from Box3D distance.c. Refining to the exact
// target instead of accepting the native tolerance changes impact poses.
use super::*;

fn nlerp_rotation(a: [f32; 4], b: [f32; 4], t: f32) -> [f32; 4] {
    let mut qa = a;
    let dot = ((a[0] * b[0] + a[1] * b[1]) + a[2] * b[2]) + a[3] * b[3];
    if dot < 0.0 {
        qa = qa.map(|x| -x);
    }
    let blend: [f32; 4] = std::array::from_fn(|i| (1.0 - t) * qa[i] + t * b[i]);
    let square =
        ((blend[0] * blend[0] + blend[1] * blend[1]) + blend[2] * blend[2]) + blend[3] * blend[3];
    if square > 1000.0 * f32::MIN_POSITIVE {
        let scale = 1.0 / square.sqrt();
        blend.map(|x| scale * x)
    } else {
        [0.0, 0.0, 0.0, 1.0]
    }
}

#[derive(Clone, Copy)]
enum Separation {
    Vertices(V3),
    Edges(V3, V3),
    FaceA(V3, V3),
    FaceB(V3, V3),
}
// b3ShapeDistance uses matrix rotation for its GJK support points.
fn matrix_rotate_q(q: [f32; 4], p: V3) -> V3 {
    let xx = q[0] * q[0];
    let yy = q[1] * q[1];
    let zz = q[2] * q[2];
    let xy = q[0] * q[1];
    let xz = q[0] * q[2];
    let xw = q[0] * q[3];
    let yz = q[1] * q[2];
    let yw = q[1] * q[3];
    let zw = q[2] * q[3];
    let cx = V3::new(1.0 - 2.0 * (yy + zz), 2.0 * (xy + zw), 2.0 * (xz - yw));
    let cy = V3::new(2.0 * (xy - zw), 1.0 - 2.0 * (xx + zz), 2.0 * (yz + xw));
    let cz = V3::new(2.0 * (xz + yw), 2.0 * (yz - xw), 1.0 - 2.0 * (xx + yy));
    (cx * p.x + cy * p.y) + cz * p.z
}
pub(super) fn rotate_q(rotation: [f32; 4], p: V3) -> V3 {
    let v = V3::new(rotation[0], rotation[1], rotation[2]);
    p + 2.0 * v.cross(v.cross(p) + rotation[3] * p)
}
pub(super) fn inv_rotate_q(rotation: [f32; 4], p: V3) -> V3 {
    let v = V3::new(rotation[0], rotation[1], rotation[2]);
    p + 2.0 * v.cross(v.cross(p) - rotation[3] * p)
}
fn inv_mul_q(a: [f32; 4], b: [f32; 4]) -> [f32; 4] {
    let av = V3::new(a[0], a[1], a[2]);
    let bv = V3::new(b[0], b[1], b[2]);
    let v = (bv.cross(av) + a[3] * bv) - b[3] * av;
    [v.x, v.y, v.z, a[3] * b[3] + av.dot(bv)]
}
fn rotate(x: WorldTransform, p: V3) -> V3 {
    rotate_q(x.q, p)
}
pub(super) fn point(x: WorldTransform, p: V3) -> V3 {
    v(x.p) + rotate(x, p)
}
impl Separation {
    fn new(
        cache: Simplex,
        a: &Proxy,
        b: &Proxy,
        xa: WorldTransform,
        xb: WorldTransform,
        end_a: WorldTransform,
        end_b: WorldTransform,
        normal: V3,
    ) -> Self {
        let mut ia = Vec::new();
        let mut ib = Vec::new();
        for vertex in &cache.vertices[..cache.count] {
            if !ia.contains(&vertex.index_a) {
                ia.push(vertex.index_a);
            }
            if !ib.contains(&vertex.index_b) {
                ib.push(vertex.index_b);
            }
        }
        if cache.count == 3 && ia.len() == 3 {
            let p = [a.points[ia[0]], a.points[ia[1]], a.points[ia[2]]];
            let mut axis = (p[1] - p[0]).cross(p[2] - p[0]).normalize_or_zero();
            let center = ((p[0] + p[1]) + p[2]) * (1.0 / 3.0);
            let delta = (rotate(xb, b.points[ib[0]]) - rotate(xa, center)) + (v(xb.p) - v(xa.p));
            if delta.dot(rotate(xa, axis)) < 0.0 {
                axis = -axis;
            }
            return Self::FaceA(axis, center);
        }
        if cache.count == 3 && ib.len() == 3 {
            let p = [b.points[ib[0]], b.points[ib[1]], b.points[ib[2]]];
            let mut axis = (p[1] - p[0]).cross(p[2] - p[0]).normalize_or_zero();
            let center = ((p[0] + p[1]) + p[2]) * (1.0 / 3.0);
            let delta = (rotate(xa, a.points[ia[0]]) - rotate(xb, center)) - (v(xb.p) - v(xa.p));
            if delta.dot(rotate(xb, axis)) < 0.0 {
                axis = -axis;
            }
            return Self::FaceB(axis, center);
        }
        if ia.len() == 2 && ib.len() == 2 {
            let ea = (a.points[ia[1]] - a.points[ia[0]]).normalize_or_zero();
            let mut eb = (b.points[ib[1]] - b.points[ib[0]]).normalize_or_zero();
            let mut axis = rotate(xa, ea).cross(rotate(xb, eb));
            let tolerance: f32 = if cache.count == 2 { 0.05 } else { 0.005 };
            if axis.length_squared() < tolerance * tolerance {
                return Self::Vertices(normal);
            }
            let delta =
                (rotate(xb, b.points[ib[0]]) - rotate(xa, a.points[ia[0]])) + (v(xb.p) - v(xa.p));
            if delta.dot(axis) < 0.0 {
                axis = -axis;
                eb = -eb;
            }
            if rotate(end_a, ea).cross(rotate(end_b, eb)).dot(axis) < 0.0 {
                return Self::Vertices(axis.normalize_or_zero());
            }
            return Self::Edges(ea, eb);
        }
        Self::Vertices(normal)
    }
    fn axis(self, a: WorldTransform, b: WorldTransform) -> V3 {
        match self {
            Self::Vertices(n) => n,
            Self::Edges(ea, eb) => rotate(a, ea).cross(rotate(b, eb)).normalize_or_zero(),
            Self::FaceA(n, _) => rotate(a, n),
            Self::FaceB(n, _) => rotate(b, n),
        }
    }
    fn minimum(
        self,
        a: &Proxy,
        b: &Proxy,
        xa: WorldTransform,
        xb: WorldTransform,
    ) -> (f32, usize, usize) {
        let n = self.axis(xa, xb);
        let (ia, ib) = match self {
            Self::FaceA(..) => (0, support(&b.points, inv_rotate_q(xb.q, -n))),
            Self::FaceB(..) => (support(&a.points, inv_rotate_q(xa.q, -n)), 0),
            _ => (
                support(&a.points, inv_rotate_q(xa.q, n)),
                support(&b.points, inv_rotate_q(xb.q, -n)),
            ),
        };
        let s = match self {
            Self::Vertices(..) | Self::Edges(..) => {
                ((rotate(xb, b.points[ib]) - rotate(xa, a.points[ia])) + (v(xb.p) - v(xa.p))).dot(n)
            }
            _ => self.evaluate(a, b, xa, xb, ia, ib),
        };
        (s, ia, ib)
    }
    fn evaluate(
        self,
        a: &Proxy,
        b: &Proxy,
        xa: WorldTransform,
        xb: WorldTransform,
        ia: usize,
        ib: usize,
    ) -> f32 {
        let n = self.axis(xa, xb);
        match self {
            Self::FaceA(_, p) => (point(xb, b.points[ib]) - point(xa, p)).dot(n),
            Self::FaceB(_, p) => (point(xa, a.points[ia]) - point(xb, p)).dot(n),
            _ => (point(xb, b.points[ib]) - point(xa, a.points[ia])).dot(n),
        }
    }
}
pub(super) fn time_of_impact(
    shape_a: &HostShape,
    start_a: WorldTransform,
    end_a: WorldTransform,
    shape_b: &HostShape,
    start_b: WorldTransform,
    end_b: WorldTransform,
    max_fraction: f32,
) -> Option<SweepHit> {
    time_of_impact_sweeps(
        shape_a,
        start_a,
        end_a,
        shape_b,
        start_b,
        end_b,
        V3::ZERO,
        V3::ZERO,
        max_fraction,
    )
}
// Keep proxy vertices in body coordinates. Centers are interpolated first;
// rotating/subtracting the local COM afterwards preserves native sweep rounding.
pub(super) fn time_of_impact_sweeps(
    shape_a: &HostShape,
    start_a: WorldTransform,
    end_a: WorldTransform,
    shape_b: &HostShape,
    start_b: WorldTransform,
    end_b: WorldTransform,
    center_a: V3,
    center_b: V3,
    max_fraction: f32,
) -> Option<SweepHit> {
    if max_fraction <= 0.0 {
        return None;
    }
    let identity = WorldTransform {
        p: [0.0; 3],
        q: [0.0, 0.0, 0.0, 1.0],
    };
    let a = shape_proxy(shape_a, Some(identity));
    let b = shape_proxy(shape_b, Some(identity));
    if a.points.is_empty() || b.points.is_empty() {
        return None;
    }
    let target = LINEAR_SLOP.max(a.radius + b.radius - LINEAR_SLOP);
    let tolerance = 0.25 * LINEAR_SLOP;
    let origin = v(start_a.p);
    let recenter = |x: WorldTransform| WorldTransform {
        p: (v(x.p) - origin).to_array(),
        ..x
    };
    let start_a = recenter(start_a);
    let end_a = recenter(end_a);
    let start_b = recenter(start_b);
    let end_b = recenter(end_b);
    let interp = |a: WorldTransform, b: WorldTransform, center: V3, t: f32| {
        let rotation = nlerp_rotation(a.q, b.q, t);
        WorldTransform {
            p: ((1.0 - t) * v(a.p) + t * v(b.p) - rotate_q(rotation, center)).to_array(),
            q: rotation,
        }
    };
    let transforms = |t| {
        (
            interp(start_a, end_a, center_a, t),
            interp(start_b, end_b, center_b, t),
        )
    };
    // Native fast-edge checks use stored endpoint rotations, without nlerp.
    let end_a = WorldTransform {
        p: (v(end_a.p) - rotate_q(end_a.q, center_a)).to_array(),
        ..end_a
    };
    let end_b = WorldTransform {
        p: (v(end_b.p) - rotate_q(end_b.q, center_b)).to_array(),
        ..end_b
    };
    let mut t1 = 0.0;
    let mut cache = None;
    for outer in 0..25 {
        let (xa, xb) = transforms(t1);
        // GJK in A's local frame avoids cancellation of translated world witnesses.
        let relative_p = inv_rotate_q(xa.q, v(xb.p) - v(xa.p));
        let relative_q = inv_mul_q(xa.q, xb.q);
        let transformed_b = Proxy {
            points: b
                .points
                .iter()
                .map(|&p| relative_p + matrix_rotate_q(relative_q, p))
                .collect(),
            radius: b.radius,
        };
        let result = distance_cached(&a, &transformed_b, false, cache);
        cache = Some(result.simplex);
        if result.distance <= 0.0 {
            return None;
        }
        let normal = rotate(xa, result.normal);
        let hit = || SweepHit {
            fraction: t1,
            normal: normal.to_array(),
            point: ((point(xa, result.point_a) + a.radius * normal + point(xa, result.point_b)
                - b.radius * normal)
                * 0.5
                + origin)
                .to_array(),
        };
        if result.distance <= target + tolerance || outer == 24 {
            return (t1 > 0.0).then(hit);
        }
        let mut separation = Separation::new(result.simplex, &a, &b, xa, xb, end_a, end_b, normal);
        let mut t2 = max_fraction;
        for _ in 0..a.points.len() + b.points.len() {
            let (x2a, x2b) = transforms(t2);
            let (mut s2, ia, ib) = separation.minimum(&a, &b, x2a, x2b);
            if s2 - target > tolerance {
                return None;
            }
            if s2 >= target - tolerance {
                t1 = t2;
                break;
            }
            let mut s1 = separation.evaluate(&a, &b, xa, xb, ia, ib);
            if s1 <= target + tolerance {
                return (t1 > 0.0).then(hit);
            }
            let mut lo = t1;
            let mut hi = t2;
            let mut root_count = 0;
            for iteration in 0..50 {
                let t = if iteration & 1 != 0 {
                    lo + (target - s1) * (hi - lo) / (s2 - s1)
                } else {
                    0.5 * (lo + hi)
                };
                let (ra, rb) = transforms(t);
                let s = separation.evaluate(&a, &b, ra, rb, ia, ib);
                root_count += 1;
                if (s - target).abs() <= tolerance {
                    t2 = t;
                    break;
                }
                if s > target {
                    lo = t;
                    s1 = s;
                } else {
                    hi = t;
                    s2 = s;
                }
            }
            if root_count == 49 && matches!(separation, Separation::Edges(..)) {
                separation = Separation::Vertices(separation.axis(xa, xb));
                t2 = max_fraction;
            }
        }
    }
    unreachable!("outer iteration cap returns a conservative hit")
}
