//! Native separation diagnostics. These deliberately follow joint.c's body-frame
//! conventions, including its single perpendicular direction for sliders.
use super::*;
use crate::types::{DISTANCE_ENABLE_LIMIT, DISTANCE_ENABLE_SPRING, PRISMATIC_ENABLE_LIMIT};

// Box3D's deterministic b3Atan2, not the platform atan2 approximation.
fn native_atan2(y: f32, x: f32) -> f32 {
    if x == 0.0 && y == 0.0 {
        return 0.0;
    }
    let ax = x.abs();
    let ay = y.abs();
    let a = ay.min(ax) / ay.max(ax);
    let s = a * a;
    let c = s * a;
    let q = s * s;
    let mut r = 0.024840285 * q + 0.18681418;
    let t = -0.094097948 * q - 0.33213072;
    r = r * s + t;
    r = r * c + a;
    if ay > ax {
        r = std::f32::consts::FRAC_PI_2 - r;
    }
    if x < 0.0 {
        r = std::f32::consts::PI - r;
    }
    if y < 0.0 {
        r = -r;
    }
    r
}
fn angle(q: [f32; 4]) -> f32 {
    2.0 * native_atan2(vec3_length([q[0], q[1], q[2]]), q[3])
}
fn twist(q: [f32; 4]) -> f32 {
    2.0 * if q[3] < 0.0 {
        native_atan2(-q[2], -q[3])
    } else {
        native_atan2(q[2], q[3])
    }
}
fn dot(a: [f32; 3], b: [f32; 3]) -> f32 {
    a[0] * b[0] + a[1] * b[1] + a[2] * b[2]
}

fn separation(id: JointId, angular: bool) -> f32 {
    if id.index1 <= 0 || id.generation != 1 {
        return 0.0;
    }
    with_world(
        WorldId {
            index1: id.world0,
            generation: 1,
        },
        |w| {
            let j = w.joints.get(id.index1 as usize - 1)?;
            if j.kind == JOINT_NONE {
                return None;
            }
            let a = w.bodies.get(j.a as usize)?.as_ref()?;
            let b = w.bodies.get(j.b as usize)?.as_ref()?;
            if angular {
                let qa = a.gpu.rot;
                let qb = b.gpu.rot;
                // b3InvMulQuat: cross, multiply-add, multiply-sub, then scalar dot.
                let mut q = [
                    (qb[1] * qa[2] - qb[2] * qa[1]) + qa[3] * qb[0] - qb[3] * qa[0],
                    (qb[2] * qa[0] - qb[0] * qa[2]) + qa[3] * qb[1] - qb[3] * qa[1],
                    (qb[0] * qa[1] - qb[1] * qa[0]) + qa[3] * qb[2] - qb[3] * qa[2],
                    qa[3] * qb[3] + dot([qa[0], qa[1], qa[2]], [qb[0], qb[1], qb[2]]),
                ];
                return Some(match j.kind {
                    JOINT_PARALLEL | JOINT_REVOLUTE => {
                        let t = twist(q);
                        if j.kind != JOINT_REVOLUTE
                            || j.flags & REVOLUTE_ENABLE_LIMIT == 0
                            || (j.lower_translation <= t && t <= j.upper_translation)
                        {
                            q[2] = 0.0;
                        }
                        angle(q)
                    }
                    JOINT_PRISMATIC => angle(q),
                    JOINT_SPHERICAL => {
                        let mut sum = 0.0;
                        if j.flags & SPHERICAL_ENABLE_CONE_LIMIT != 0 {
                            let swing = 2.0
                                * native_atan2(
                                    (q[0] * q[0] + q[1] * q[1]).sqrt(),
                                    (q[2] * q[2] + q[3] * q[3]).sqrt(),
                                );
                            sum += (swing - j.target_translation).max(0.0);
                        }
                        if j.flags & SPHERICAL_ENABLE_TWIST_LIMIT != 0 {
                            let t = twist(q);
                            sum += (j.lower_translation - t).max(0.0);
                            sum += (t - j.upper_translation).max(0.0);
                        }
                        sum
                    }
                    JOINT_WELD if j.weld_angular_hertz == 0.0 => angle(q),
                    // Upstream wheel angular separation asserts and returns zero.
                    // Match its release result; this is not wheel angular support.
                    _ => 0.0,
                });
            }
            let oa = body_origin(a);
            let ob = body_origin(b);
            let ra = quat_rotate(a.gpu.rot, j.anchor_a);
            let rb = quat_rotate(b.gpu.rot, j.anchor_b);
            // Native portable builds use float b3Pos, as does the GPU body storage.
            let dp = std::array::from_fn(|i| (ob[i] + rb[i]) - (oa[i] + ra[i]));
            Some(match j.kind {
                JOINT_DISTANCE => {
                    let length = vec3_length(dp);
                    if j.flags & DISTANCE_ENABLE_SPRING == 0 {
                        (length - j.target_translation).abs()
                    } else if j.flags & DISTANCE_ENABLE_LIMIT != 0 {
                        if length < j.lower_translation {
                            j.lower_translation - length
                        } else if length > j.upper_translation {
                            length - j.upper_translation
                        } else {
                            0.0
                        }
                    } else {
                        0.0
                    }
                }
                JOINT_PRISMATIC | JOINT_WHEEL => {
                    let axis = quat_rotate(a.gpu.rot, [1.0, 0.0, 0.0]);
                    let p = if axis[0] < -0.5 || axis[0] > 0.5 {
                        [axis[1], -axis[0], 0.0]
                    } else {
                        [0.0, axis[2], -axis[1]]
                    };
                    let inv = 1.0 / vec3_length(p);
                    let perp = dot(p.map(|v| inv * v), dp).abs();
                    let flag = if j.kind == JOINT_WHEEL {
                        WHEEL_ENABLE_SUSPENSION_LIMIT
                    } else {
                        PRISMATIC_ENABLE_LIMIT
                    };
                    let mut limit = 0.0;
                    if j.flags & flag != 0 {
                        let t = dot(axis, dp);
                        if t < j.lower_translation {
                            limit = j.lower_translation - t;
                        }
                        if j.upper_translation < t {
                            limit = t - j.upper_translation;
                        }
                    }
                    (perp * perp + limit * limit).sqrt()
                }
                JOINT_REVOLUTE | JOINT_SPHERICAL => vec3_length(dp),
                JOINT_WELD if j.weld_linear_hertz == 0.0 => vec3_length(dp),
                _ => 0.0,
            })
        },
    )
    .flatten()
    .unwrap_or(0.0)
}

pub fn b3_joint_get_linear_separation(id: JointId) -> f32 {
    separation(id, false)
}
pub fn b3_joint_get_angular_separation(id: JointId) -> f32 {
    separation(id, true)
}
