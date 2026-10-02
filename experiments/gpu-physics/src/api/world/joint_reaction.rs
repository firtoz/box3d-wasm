//! Native force/torque queries. Preserve upstream's diagnostic conventions,
//! including the wheel limit-value term and revolute's two axial terms.
use super::*;
type V3 = [f32; 3];
fn add(a: V3, b: V3) -> V3 {
    std::array::from_fn(|i| a[i] + b[i])
}
fn scale(s: f32, v: V3) -> V3 {
    v.map(|x| s * x)
}
fn cross(a: V3, b: V3) -> V3 {
    [
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    ]
}
fn dot(a: V3, b: V3) -> f32 {
    a[0] * b[0] + a[1] * b[1] + a[2] * b[2]
}
fn xyz(q: [f32; 4]) -> V3 {
    [q[0], q[1], q[2]]
}
fn mul(a: [f32; 4], b: [f32; 4]) -> [f32; 4] {
    let v = add(
        add(cross(xyz(a), xyz(b)), scale(a[3], xyz(b))),
        scale(b[3], xyz(a)),
    );
    [v[0], v[1], v[2], a[3] * b[3] - dot(xyz(a), xyz(b))]
}
fn normalize(v: V3) -> V3 {
    let length = vec3_length(v);
    if length < f32::EPSILON {
        [0.0; 3]
    } else {
        scale(1.0 / length, v)
    }
}
fn awake(b: &BodyGpu) -> bool {
    b.flags & (FLAG_STATIC | FLAG_DISABLED | FLAG_SLEEP) == 0
}
// Capture at submission: later edits and mirror refreshes must not rewrite
// the prepared frames of a completed step.
pub(super) fn prepare_frames(w: &mut WorldInner) {
    for (i, j) in w.joints.iter().enumerate() {
        if !matches!(j.kind, JOINT_PARALLEL | JOINT_REVOLUTE) {
            continue;
        }
        if let (Some(a), Some(b), Some(Some(meta))) = (
            w.step_start_bodies.get(j.a as usize),
            w.step_start_bodies.get(j.b as usize),
            w.joint_meta.get_mut(i),
        ) {
            meta.pending_reaction_frames = Some((
                [
                    mul(a.rot, j.frame_a_rotation),
                    mul(b.rot, j.frame_b_rotation),
                ],
                awake(a) || awake(b),
            ));
        }
    }
}
// Sleeping joints retain their prepared frames. Account for GPU wakeups,
// and consume each submission's snapshot only once.
pub(super) fn save_prepared_frames(w: &mut WorldInner, end: &[BodyGpu]) {
    for (i, j) in w.joints.iter().enumerate() {
        let Some(Some(meta)) = w.joint_meta.get_mut(i) else {
            continue;
        };
        let Some((frames, started_awake)) = meta.pending_reaction_frames.take() else {
            continue;
        };
        if started_awake
            || [j.a, j.b]
                .into_iter()
                .any(|b| end.get(b as usize).is_some_and(awake))
        {
            meta.reaction_frames = frames;
        }
    }
}
fn prepared_torque(frames: [[f32; 4]; 2], perp: [f32; 2]) -> V3 {
    let [a, b] = frames;
    let v = add(
        add(cross(xyz(b), xyz(a)), scale(a[3], xyz(b))),
        scale(-b[3], xyz(a)),
    );
    let s = a[3] * b[3] + dot(xyz(a), xyz(b));
    let x = scale(
        0.5,
        quat_rotate(a, add(scale(s, [1.0, 0.0, 0.0]), cross(v, [1.0, 0.0, 0.0]))),
    );
    let y = scale(
        0.5,
        quat_rotate(a, add(scale(s, [0.0, 1.0, 0.0]), cross(v, [0.0, 1.0, 0.0]))),
    );
    add(scale(perp[0], x), scale(perp[1], y))
}
fn reaction(id: JointId, torque: bool) -> V3 {
    if id.index1 <= 0 {
        return [0.0; 3];
    }
    with_world(
        world_id_from_joint(id),
        |w| {
            let index = id.index1 as usize - 1;
            let j = w.joints.get(index)?;
            if j.kind == JOINT_NONE {
                return None;
            }
            let meta = w.joint_meta.get(index)?.as_ref()?;
            let a = w.bodies.get(j.a as usize)?.as_ref()?;
            let b = w.bodies.get(j.b as usize)?.as_ref()?;
            let inv_h = w.reaction_inv_h;
            let world =
                |v| quat_rotate(a.gpu.rot, quat_rotate(j.frame_a_rotation, scale(inv_h, v)));
            Some(if torque {
                match j.kind {
                    JOINT_PARALLEL => {
                        scale(inv_h, prepared_torque(meta.reaction_frames, j.perp_impulse))
                    }
                    JOINT_REVOLUTE => {
                        let axial =
                            j.spring_impulse + j.motor_impulse + j.lower_impulse - j.upper_impulse;
                        let prepared = quat_rotate(meta.reaction_frames[0], [0.0, 0.0, 1.0]);
                        let current = quat_rotate(
                            a.gpu.rot,
                            quat_rotate(j.frame_a_rotation, [0.0, 0.0, 1.0]),
                        );
                        let impulse = add(
                            prepared_torque(meta.reaction_frames, j.perp_impulse),
                            scale(axial, prepared),
                        );
                        scale(inv_h, add(impulse, scale(axial, current)))
                    }
                    JOINT_SPHERICAL => {
                        let qa = mul(a.gpu.rot, j.frame_a_rotation);
                        let qb = mul(b.gpu.rot, j.frame_b_rotation);
                        let cone = quat_rotate(qa, [0.0, 0.0, 1.0]);
                        let twist = quat_rotate(qb, [0.0, 0.0, 1.0]);
                        let swing = normalize(cross(cone, twist));
                        let impulse = add(j.spring_angular_impulse, j.motor_angular_impulse);
                        let impulse = add(impulse, scale(j.lower_impulse - j.upper_impulse, twist));
                        scale(inv_h, add(impulse, scale(j.swing_impulse, swing)))
                    }
                    JOINT_WELD => scale(inv_h, j.weld_angular_impulse),
                    JOINT_MOTOR => scale(
                        inv_h,
                        add(j.angular_impulse, [j.motor_impulse, j._pad2[0], j._pad2[1]]),
                    ),
                    JOINT_PRISMATIC => world(j.angular_impulse),
                    JOINT_WHEEL => {
                        let q = mul(a.gpu.rot, j.frame_a_rotation);
                        // b3MakeMatrixFromQuat column z, preserving its arithmetic.
                        let z = [
                            2.0 * (q[0] * q[2] + q[1] * q[3]),
                            2.0 * (q[1] * q[2] - q[0] * q[3]),
                            1.0 - 2.0 * (q[0] * q[0] + q[1] * q[1]),
                        ];
                        scale(inv_h * j.motor_impulse, z)
                    }
                    _ => [0.0; 3],
                }
            } else {
                match j.kind {
                    JOINT_DISTANCE => {
                        let pa = add(body_origin(a), quat_rotate(a.gpu.rot, j.anchor_a));
                        let pb = add(body_origin(b), quat_rotate(b.gpu.rot, j.anchor_b));
                        let axis = normalize(std::array::from_fn(|i| pb[i] - pa[i]));
                        let v = j.weld_linear_impulse;
                        scale(
                            (v[0] + v[1] - v[2] + j.weld_angular_impulse[0]) * inv_h,
                            axis,
                        )
                    }
                    JOINT_REVOLUTE | JOINT_SPHERICAL => scale(inv_h, j.angular_impulse),
                    JOINT_WELD => scale(inv_h, j.weld_linear_impulse),
                    JOINT_MOTOR => scale(
                        inv_h,
                        add(
                            [j.impulse, j.perp_impulse[0], j.perp_impulse[1]],
                            [j.spring_impulse, j.lower_impulse, j.upper_impulse],
                        ),
                    ),
                    JOINT_PRISMATIC => world([
                        j.perp_impulse[0],
                        j.perp_impulse[1],
                        j.motor_impulse + j.lower_impulse + j.upper_impulse + j.spring_impulse,
                    ]),
                    JOINT_WHEEL => world([
                        j.perp_impulse[0],
                        j.perp_impulse[1],
                        j.lower_translation + j.upper_impulse + j.spring_impulse,
                    ]),
                    _ => [0.0; 3],
                }
            })
        },
    )
    .flatten()
    .unwrap_or([0.0; 3])
}
pub fn b3_joint_get_constraint_force(id: JointId) -> V3 {
    reaction(id, false)
}
pub fn b3_joint_get_constraint_torque(id: JointId) -> V3 {
    reaction(id, true)
}
