use crate::types::{BodyGpu, DemoScene, DUMP_CHECKPOINTS, FLAG_SLEEP, SPHERE_RADIUS};

const MAGIC: &[u8; 4] = b"GP3D";
const VERSION: u32 = 2;
/// World pivot for revolute/weld: static body at y=-1 plus local (0, 6.5, 0).
const JOINT_PIVOT: [f32; 3] = [0.0, 5.5, 0.0];
const REVOLUTE_ARM: f32 = 1.5;
const WELD_HOLD: [f32; 3] = [0.0, 4.0, 0.0];

pub fn encode_checkpoint(step: u32, bodies: &[BodyGpu]) -> Vec<u8> {
    let mut out = Vec::with_capacity(16 + bodies.len() * 12);
    out.extend_from_slice(MAGIC);
    out.extend_from_slice(&VERSION.to_le_bytes());
    out.extend_from_slice(&step.to_le_bytes());
    out.extend_from_slice(&(bodies.len() as u32).to_le_bytes());
    for b in bodies {
        for c in b.pos {
            out.extend_from_slice(&c.to_le_bytes());
        }
    }
    out
}

pub fn encode_run(checkpoints: &[(u32, Vec<BodyGpu>)]) -> Vec<u8> {
    let mut out = Vec::new();
    out.extend_from_slice(MAGIC);
    out.extend_from_slice(&VERSION.to_le_bytes());
    out.extend_from_slice(&(checkpoints.len() as u32).to_le_bytes());
    for (step, bodies) in checkpoints {
        out.extend_from_slice(&encode_checkpoint(*step, bodies));
    }
    out
}

pub fn should_dump(step: u32) -> bool {
    DUMP_CHECKPOINTS.contains(&step)
}

pub fn should_capture_state(scene: DemoScene, step: u32, max_step: u32) -> bool {
    should_dump(step)
        || step == max_step
        || (max_step > 300 && step % 300 == 0)
        || (scene == DemoScene::HighResistance && (300..=600).contains(&step))
}

/// Catastrophe checks (NaN, floor loss, runaway speed) plus scene rest/joint
/// bounds. High Resistance settling is a quality gate, not a catastrophe check.
pub fn physics_quality_error(scene: DemoScene, bodies: &[BodyGpu], step: u32) -> Option<String> {
    for (i, b) in bodies.iter().enumerate() {
        let finite = b
            .pos
            .iter()
            .chain(b.vel.iter())
            .chain(b.rot.iter())
            .chain(b.omega.iter())
            .all(|c| c.is_finite());
        if !finite {
            return Some(format!("non-finite state body {i} at step {step}"));
        }
        if b.inv_mass > 0.0 && b.pos[1] < -50.0 {
            return Some(format!(
                "body {i} fell through the world y={} at step {step}",
                b.pos[1]
            ));
        }
    }
    if let Some(speed) = max_speed(bodies) {
        if speed > 250.0 {
            return Some(format!("max speed {speed:.3} at step {step}"));
        }
    }
    if scene == DemoScene::Revolute {
        if let Some(err) = revolute_joint_error(bodies) {
            if err > 0.25 {
                return Some(format!("revolute joint_err={err:.3} at step {step}"));
            }
        }
    }
    if scene == DemoScene::Weld {
        if let Some(err) = weld_hold_error(bodies) {
            if err > 0.25 {
                return Some(format!("weld hold_err={err:.3} at step {step}"));
            }
        }
    }
    if is_vertical_stack(scene) && step >= 300 {
        if let Some((_, fallen, _)) = box_stack_column_error(bodies) {
            if fallen > 0 {
                return Some(format!("{fallen} stacked bodies fell at step {step}"));
            }
        }
    }
    if scene == DemoScene::HighResistance && step >= 300 {
        if let Some(err) = high_resistance_quality_error(bodies, step) {
            return Some(err);
        }
    }
    if scene == DemoScene::MixedStacks && step >= 60 {
        if let Some(err) = mixed_stacks_quality_error(bodies, step) {
            return Some(err);
        }
    }
    None
}

pub fn dumps_equal(a: &[u8], b: &[u8]) -> bool {
    a == b
}

pub fn first_mismatch(a: &[u8], b: &[u8]) -> Option<usize> {
    if a.len() != b.len() {
        return Some(a.len().min(b.len()));
    }
    a.iter().zip(b.iter()).position(|(x, y)| x != y)
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::types::BodyGpu;

    fn body_at(y: f32, dynamic: bool) -> BodyGpu {
        let mut b = BodyGpu::zeroed();
        b.pos = [0.0, y, 0.0];
        b.inv_mass = if dynamic { 0.001 } else { 0.0 };
        b
    }

    #[test]
    fn single_box_rest_matches_half_extent() {
        let bodies = [body_at(-1.0, false), body_at(0.5, true)];
        assert!(single_box_rest_error(&bodies).unwrap() < 1e-5);
    }

    #[test]
    fn rest_fields_cover_all_scene_kinds() {
        let mut dyn_body = BodyGpu::zeroed();
        dyn_body.inv_mass = 1.0;
        dyn_body.pos = [0.0, 4.0, 0.0];
        let weld = rest_json_fields(DemoScene::Weld, &[dyn_body]);
        assert!(weld.iter().any(|s| s.contains("hold_err")));
        assert!(weld.iter().any(|s| s.contains("speed")));
        let rev = rest_json_fields(DemoScene::Revolute, &[dyn_body]);
        assert!(rev.iter().any(|s| s.contains("joint_err")));
        dyn_body.pos = [0.0, 0.18, 0.0];
        let sph = rest_json_fields(DemoScene::Spheres, &[dyn_body]);
        assert!(sph.iter().any(|s| s.contains("floor_err")));
        dyn_body.pos = [0.0, 5.0, 0.0];
        let bounce = rest_json_fields(DemoScene::Bounce, &[dyn_body]);
        assert!(bounce.iter().any(|s| s.contains("bounce_y")));
        dyn_body.omega = [0.0, 0.0, 8.0];
        let spin = rest_json_fields(DemoScene::Spinner, &[dyn_body]);
        assert!(spin.iter().any(|s| s.contains("omega")));
        let pyr = rest_json_fields(DemoScene::Pyramid, &[dyn_body]);
        assert!(pyr.iter().any(|s| s.contains("fallen")));
        assert!(pyr.iter().any(|s| s.contains("xz_err")));
        let mixed = rest_json_fields(DemoScene::Mixed, &[dyn_body]);
        assert!(mixed.iter().any(|s| s.contains("min_y")));
        dyn_body.pos = [-4.0, 4.2, 0.0];
        let ramp = rest_json_fields(DemoScene::Ramp, &[dyn_body]);
        assert!(ramp.iter().any(|s| s.contains("slide_x")));
        assert!(ramp.iter().any(|s| s.contains("slide_y")));
        let hr = rest_json_fields(DemoScene::HighResistance, &[dyn_body]);
        assert!(hr.iter().any(|s| s.contains("capsules")));
        let mixed_st = rest_json_fields(DemoScene::MixedStacks, &[dyn_body]);
        assert!(mixed_st.iter().any(|s| s.contains("boxes")));
        let a = body_at(0.5, true);
        let b = body_at(1.5, true);
        let stack = rest_json_fields(DemoScene::Stack, &[body_at(-1.0, false), a, b]);
        assert!(stack.iter().any(|s| s.contains("gap_err")));
        assert!(stack.iter().any(|s| s.contains("order_gap")));
        assert!(stack.iter().any(|s| s.contains("fallen")));
    }

    #[test]
    fn physics_quality_rejects_nan_and_floor_loss() {
        let mut bad = body_at(0.5, true);
        bad.pos[0] = f32::NAN;
        assert!(physics_quality_error(DemoScene::BoxStack, &[bad], 10).is_some());
        let fallen = body_at(-80.0, true);
        assert!(physics_quality_error(DemoScene::BoxStack, &[fallen], 10).is_some());
        assert!(physics_quality_error(DemoScene::BoxStack, &[body_at(0.5, true)], 10).is_none());
    }

    #[test]
    fn high_resistance_sleep_does_not_use_awake_classification() {
        let mut bodies = Vec::new();
        for i in 0..10 {
            let mut b = BodyGpu::zeroed();
            b.inv_mass = 1.0;
            if i < 6 {
                b.pos = [0.0, 0.5, 0.0];
                b.rot = [0.0, 0.0, 0.70710677, 0.70710677];
            } else {
                b.pos = [0.0, 1.5, 0.0];
                b.rot = [0.0, 0.0, 0.0, 1.0];
            }
            if i == 5 {
                b.flags = crate::types::FLAG_SLEEP;
                b.pos = [0.0, 1.5, 0.0];
                b.rot = [0.0, 0.0, 0.0, 1.0];
            }
            bodies.push(b);
        }
        assert!(high_resistance_quality_error(&bodies, 300).is_none());
        bodies[5].pos[1] = -2.0;
        assert!(high_resistance_quality_error(&bodies, 300).is_some());
    }
}

/// Analytic Box3D rest: ground top y=0, cube half-extent 0.5 → COM y=0.5.
pub fn single_box_rest_error(bodies: &[BodyGpu]) -> Option<f32> {
    let b = bodies.iter().find(|b| b.inv_mass > 0.0)?;
    Some((b.pos[1] - 0.5).abs())
}

/// Consecutive dynamic cube gaps vs 1.0 (full height) and bottom COM vs 0.5.
pub fn box_stack_rest_error(bodies: &[BodyGpu]) -> Option<(f32, f32)> {
    let mut ys: Vec<f32> = bodies
        .iter()
        .filter(|b| b.inv_mass > 0.0)
        .map(|b| b.pos[1])
        .collect();
    if ys.len() < 2 {
        return None;
    }
    ys.sort_by(|a, b| a.partial_cmp(b).unwrap());
    let bottom = (ys[0] - 0.5).abs();
    let mut gap_err = 0.0f32;
    for w in ys.windows(2) {
        gap_err = gap_err.max((w[1] - w[0] - 1.0).abs());
    }
    Some((bottom, gap_err))
}

/// Creation-order column: max |Δy-1| between successive dynamic bodies, boxes that
/// went through the floor (y < 0.25), and max |xz| drift from the stack axis.
pub fn box_stack_column_error(bodies: &[BodyGpu]) -> Option<(f32, u32, f32)> {
    let dyns: Vec<&BodyGpu> = bodies.iter().filter(|b| b.inv_mass > 0.0).collect();
    if dyns.len() < 2 {
        return None;
    }
    let mut order_gap = 0.0f32;
    for w in dyns.windows(2) {
        order_gap = order_gap.max((w[1].pos[1] - w[0].pos[1] - 1.0).abs());
    }
    let fallen = dyns.iter().filter(|b| b.pos[1] < 0.25).count() as u32;
    let xz = dyns
        .iter()
        .map(|b| (b.pos[0] * b.pos[0] + b.pos[2] * b.pos[2]).sqrt())
        .fold(0.0f32, f32::max);
    Some((order_gap, fallen, xz))
}

pub fn max_speed(bodies: &[BodyGpu]) -> Option<f32> {
    let mut max = None;
    for b in bodies.iter().filter(|b| b.inv_mass > 0.0) {
        let s = (b.vel[0] * b.vel[0] + b.vel[1] * b.vel[1] + b.vel[2] * b.vel[2]).sqrt();
        max = Some(max.map_or(s, |m: f32| m.max(s)));
    }
    max
}

pub fn revolute_joint_error(bodies: &[BodyGpu]) -> Option<f32> {
    let b = bodies.iter().find(|b| b.inv_mass > 0.0)?;
    let dx = b.pos[0] - JOINT_PIVOT[0];
    let dy = b.pos[1] - JOINT_PIVOT[1];
    let dz = b.pos[2] - JOINT_PIVOT[2];
    let len = (dx * dx + dy * dy + dz * dz).sqrt();
    Some((len - REVOLUTE_ARM).abs())
}

pub fn weld_hold_error(bodies: &[BodyGpu]) -> Option<f32> {
    let b = bodies.iter().find(|b| b.inv_mass > 0.0)?;
    let dx = b.pos[0] - WELD_HOLD[0];
    let dy = b.pos[1] - WELD_HOLD[1];
    let dz = b.pos[2] - WELD_HOLD[2];
    Some((dx * dx + dy * dy + dz * dz).sqrt())
}

pub fn spheres_floor_error(bodies: &[BodyGpu]) -> Option<f32> {
    let mut min_y = None;
    for b in bodies.iter().filter(|b| b.inv_mass > 0.0) {
        min_y = Some(min_y.map_or(b.pos[1], |m: f32| m.min(b.pos[1])));
    }
    Some((min_y? - SPHERE_RADIUS).abs())
}

fn first_dynamic_y(bodies: &[BodyGpu]) -> Option<f32> {
    bodies.iter().find(|b| b.inv_mass > 0.0).map(|b| b.pos[1])
}

fn min_dynamic_y(bodies: &[BodyGpu]) -> Option<f32> {
    bodies
        .iter()
        .filter(|b| b.inv_mass > 0.0)
        .map(|b| b.pos[1])
        .fold(None, |m: Option<f32>, y| Some(m.map_or(y, |a| a.min(y))))
}

fn first_dynamic_xy(bodies: &[BodyGpu]) -> Option<(f32, f32)> {
    bodies
        .iter()
        .find(|b| b.inv_mass > 0.0)
        .map(|b| (b.pos[0], b.pos[1]))
}

fn max_omega(bodies: &[BodyGpu]) -> Option<f32> {
    let mut max = None;
    for b in bodies.iter().filter(|b| b.inv_mass > 0.0) {
        let s =
            (b.omega[0] * b.omega[0] + b.omega[1] * b.omega[1] + b.omega[2] * b.omega[2]).sqrt();
        max = Some(max.map_or(s, |m: f32| m.max(s)));
    }
    max
}

/// High Resistance capsules: local Y endpoints ±1, radius 0.5.
pub const HIGH_RESISTANCE_HALF_LEN: f32 = 1.0;
pub const HIGH_RESISTANCE_RADIUS: f32 = 0.5;
pub const HIGH_RESISTANCE_FLAT: u32 = 6;

pub fn capsule_abs_axis_y(rot: [f32; 4]) -> f32 {
    let q = rot;
    (1.0 - 2.0 * (q[0] * q[0] + q[2] * q[2])).abs()
}

pub fn capsule_support_y(body: &BodyGpu, half_len: f32, radius: f32) -> f32 {
    body.pos[1] - capsule_abs_axis_y(body.rot) * half_len - radius
}

pub fn high_resistance_is_flat(body: &BodyGpu) -> bool {
    capsule_abs_axis_y(body.rot) < 0.25
}

pub fn high_resistance_quality_error(bodies: &[BodyGpu], step: u32) -> Option<String> {
    let dyns: Vec<&BodyGpu> = bodies.iter().filter(|b| b.inv_mass > 0.0).collect();
    if dyns.len() != 10 {
        return Some(format!(
            "high-resistance expected 10 dynamic bodies, got {} at step {step}",
            dyns.len()
        ));
    }
    for (i, b) in dyns.iter().enumerate() {
        let lin = (b.vel[0] * b.vel[0] + b.vel[1] * b.vel[1] + b.vel[2] * b.vel[2]).sqrt();
        let ang = (b.omega[0] * b.omega[0] + b.omega[1] * b.omega[1] + b.omega[2] * b.omega[2]).sqrt();
        let asleep = b.flags & FLAG_SLEEP != 0;
        if !asleep && lin > 0.01 {
            return Some(format!(
                "high-resistance body {i} linear speed {lin:.4} at step {step}"
            ));
        }
        if !asleep && ang > 0.01 {
            return Some(format!(
                "high-resistance body {i} angular speed {ang:.4} at step {step}"
            ));
        }
        let bottom = capsule_support_y(b, HIGH_RESISTANCE_HALF_LEN, HIGH_RESISTANCE_RADIUS);
        if bottom < -0.02 || bottom > 0.02 {
            return Some(format!(
                "high-resistance body {i} support y={bottom:.4} at step {step}"
            ));
        }
        if asleep {
            continue;
        }
        let flat = high_resistance_is_flat(b);
        if i < HIGH_RESISTANCE_FLAT as usize && !flat {
            return Some(format!(
                "high-resistance body {i} should lie flat at step {step}"
            ));
        }
        if i >= HIGH_RESISTANCE_FLAT as usize && flat {
            return Some(format!(
                "high-resistance body {i} should rest tilted at step {step}"
            ));
        }
    }
    None
}

pub fn mixed_stacks_quality_error(bodies: &[BodyGpu], step: u32) -> Option<String> {
    let dyns: Vec<&BodyGpu> = bodies.iter().filter(|b| b.inv_mass > 0.0).collect();
    if dyns.len() < 2 {
        return Some(format!(
            "mixed-stacks expected at least 2 dynamic bodies, got {} at step {step}",
            dyns.len()
        ));
    }
    let count = dyns.len() as u32;
    let per_layer = (count + 1) / 2;
    let mut min_y = f32::MAX;
    let mut max_y = f32::MIN;
    for (i, b) in dyns.iter().enumerate() {
        let i = i as u32;
        let expected_y = 0.5 + u32::from(i >= per_layer) as f32;
        let spawn_x = 3.0 * (i % 20) as f32;
        let spawn_z = 3.0 * ((i % per_layer) / 20) as f32;
        min_y = min_y.min(b.pos[1]);
        max_y = max_y.max(b.pos[1]);
        if (b.pos[1] - expected_y).abs() > 0.02 {
            return Some(format!(
                "mixed-stacks body {i} y={} expected {expected_y} at step {step}",
                b.pos[1]
            ));
        }
        if (b.pos[0] - spawn_x).abs() > 0.02 || (b.pos[2] - spawn_z).abs() > 0.02 {
            return Some(format!(
                "mixed-stacks body {i} xz drift ({}, {}) at step {step}",
                b.pos[0], b.pos[2]
            ));
        }
        if b.pos[1] < 0.25 {
            return Some(format!(
                "mixed-stacks body {i} fell through y={} at step {step}",
                b.pos[1]
            ));
        }
        let lin = (b.vel[0] * b.vel[0] + b.vel[1] * b.vel[1] + b.vel[2] * b.vel[2]).sqrt();
        let ang = (b.omega[0] * b.omega[0] + b.omega[1] * b.omega[1] + b.omega[2] * b.omega[2]).sqrt();
        if lin > 0.05 {
            return Some(format!(
                "mixed-stacks body {i} speed {lin} at step {step}"
            ));
        }
        if ang > 0.05 {
            return Some(format!(
                "mixed-stacks body {i} omega {ang} at step {step}"
            ));
        }
        let identity = (b.rot[0]).abs() + (b.rot[1]).abs() + (b.rot[2]).abs() + (b.rot[3] - 1.0).abs();
        let neg = (b.rot[0]).abs() + (b.rot[1]).abs() + (b.rot[2]).abs() + (b.rot[3] + 1.0).abs();
        if identity.min(neg) > 0.05 {
            return Some(format!(
                "mixed-stacks body {i} quat {:?} at step {step}",
                b.rot
            ));
        }
    }
    if min_y < 0.45 || max_y > 1.6 {
        return Some(format!(
            "mixed-stacks COM y range {min_y}..{max_y} at step {step}"
        ));
    }
    None
}

pub fn pyramid_spread(bodies: &[BodyGpu]) -> Option<(u32, f32)> {
    let dyns: Vec<&BodyGpu> = bodies.iter().filter(|b| b.inv_mass > 0.0).collect();
    if dyns.is_empty() {
        return None;
    }
    let fallen = dyns.iter().filter(|b| b.pos[1] < 0.25).count() as u32;
    let xz = dyns
        .iter()
        .map(|b| (b.pos[0] * b.pos[0] + b.pos[2] * b.pos[2]).sqrt())
        .fold(0.0f32, f32::max);
    Some((fallen, xz))
}

fn is_vertical_stack(scene: DemoScene) -> bool {
    matches!(
        scene,
        DemoScene::BoxStack | DemoScene::SphereStack | DemoScene::CapsuleStack | DemoScene::Stack
    )
}

/// Max |Δpos|, |Δvel|, |Δq|, |Δω| between aligned body arrays (dynamic bodies only).
pub fn max_body_error(gpu: &[BodyGpu], cpu: &[BodyGpu]) -> Option<(f32, f32, f32, f32)> {
    if gpu.len() != cpu.len() || gpu.is_empty() {
        return None;
    }
    let mut pos = 0.0f32;
    let mut vel = 0.0f32;
    let mut quat = 0.0f32;
    let mut omega = 0.0f32;
    for (a, b) in gpu.iter().zip(cpu.iter()) {
        if a.inv_mass <= 0.0 && b.inv_mass <= 0.0 {
            continue;
        }
        let dp = (a.pos[0] - b.pos[0])
            .abs()
            .max((a.pos[1] - b.pos[1]).abs())
            .max((a.pos[2] - b.pos[2]).abs());
        let dv = (a.vel[0] - b.vel[0])
            .abs()
            .max((a.vel[1] - b.vel[1]).abs())
            .max((a.vel[2] - b.vel[2]).abs());
        let dw = (a.omega[0] - b.omega[0])
            .abs()
            .max((a.omega[1] - b.omega[1]).abs())
            .max((a.omega[2] - b.omega[2]).abs());
        let mut dq = (a.rot[0] - b.rot[0]).abs()
            + (a.rot[1] - b.rot[1]).abs()
            + (a.rot[2] - b.rot[2]).abs()
            + (a.rot[3] - b.rot[3]).abs();
        let dq_neg = (a.rot[0] + b.rot[0]).abs()
            + (a.rot[1] + b.rot[1]).abs()
            + (a.rot[2] + b.rot[2]).abs()
            + (a.rot[3] + b.rot[3]).abs();
        dq = dq.min(dq_neg);
        if !dp.is_finite() || !dv.is_finite() || !dq.is_finite() || !dw.is_finite() {
            return Some((f32::INFINITY, f32::INFINITY, f32::INFINITY, f32::INFINITY));
        }
        pos = pos.max(dp);
        vel = vel.max(dv);
        quat = quat.max(dq);
        omega = omega.max(dw);
    }
    Some((pos, vel, quat, omega))
}

/// Box3D oracle dump (`B3OR`): all frames, 144-byte `BodyGpu`.
pub fn read_b3or(path: &std::path::Path) -> Result<(u32, u32, Vec<BodyGpu>), String> {
    let bytes = std::fs::read(path).map_err(|e| format!("read {}: {e}", path.display()))?;
    if bytes.len() < 20 || &bytes[0..4] != b"B3OR" {
        return Err(format!("{} is not a B3OR dump", path.display()));
    }
    let version = u32::from_le_bytes(bytes[4..8].try_into().unwrap());
    if version != 1 {
        return Err(format!("unsupported B3OR version {version}"));
    }
    let frames = u32::from_le_bytes(bytes[8..12].try_into().unwrap());
    let body_count = u32::from_le_bytes(bytes[12..16].try_into().unwrap());
    let stride = u32::from_le_bytes(bytes[16..20].try_into().unwrap()) as usize;
    if stride != std::mem::size_of::<BodyGpu>() {
        return Err(format!(
            "B3OR stride {stride} != {}",
            std::mem::size_of::<BodyGpu>()
        ));
    }
    let need = 20 + frames as usize * body_count as usize * stride;
    if bytes.len() < need {
        return Err(format!(
            "{} truncated ({} bytes, need {need})",
            path.display(),
            bytes.len()
        ));
    }
    let mut bodies = Vec::with_capacity(frames as usize * body_count as usize);
    let mut off = 20;
    for _ in 0..frames {
        for _ in 0..body_count {
            let slice = &bytes[off..off + stride];
            let body: BodyGpu = *bytemuck::from_bytes(slice);
            bodies.push(body);
            off += stride;
        }
    }
    Ok((frames, body_count, bodies))
}

pub fn oracle_frame<'a>(
    bodies: &'a [BodyGpu],
    frame: u32,
    body_count: u32,
) -> Option<&'a [BodyGpu]> {
    let n = body_count as usize;
    let start = frame as usize * n;
    bodies.get(start..start + n)
}

pub fn rest_json_fields(scene: DemoScene, bodies: &[BodyGpu]) -> Vec<String> {
    let mut fields = Vec::new();
    if let Some(s) = max_speed(bodies) {
        fields.push(format!("\"speed\":{s:.6e}"));
    }
    if scene == DemoScene::SingleBox {
        if let Some(err) = single_box_rest_error(bodies) {
            fields.push(format!("\"y_err\":{err:.6e}"));
        }
    }
    if is_vertical_stack(scene) {
        if let Some((bottom, gap)) = box_stack_rest_error(bodies) {
            fields.push(format!("\"bottom_err\":{bottom:.6e}"));
            fields.push(format!("\"gap_err\":{gap:.6e}"));
        }
        if let Some((order_gap, fallen, xz)) = box_stack_column_error(bodies) {
            fields.push(format!("\"order_gap\":{order_gap:.6e}"));
            fields.push(format!("\"fallen\":{fallen}"));
            fields.push(format!("\"xz_err\":{xz:.6e}"));
        }
    }
    if scene == DemoScene::Spheres {
        if let Some(err) = spheres_floor_error(bodies) {
            fields.push(format!("\"floor_err\":{err:.6e}"));
        }
    }
    if scene == DemoScene::Revolute {
        if let Some(err) = revolute_joint_error(bodies) {
            fields.push(format!("\"joint_err\":{err:.6e}"));
        }
    }
    if scene == DemoScene::Weld {
        if let Some(err) = weld_hold_error(bodies) {
            fields.push(format!("\"hold_err\":{err:.6e}"));
        }
    }
    if scene == DemoScene::Bounce {
        if let Some(y) = first_dynamic_y(bodies) {
            fields.push(format!("\"bounce_y\":{y:.6e}"));
        }
    }
    if scene == DemoScene::Spinner {
        if let Some(w) = max_omega(bodies) {
            fields.push(format!("\"omega\":{w:.6e}"));
        }
    }
    if scene == DemoScene::Pyramid {
        if let Some((fallen, xz)) = pyramid_spread(bodies) {
            fields.push(format!("\"fallen\":{fallen}"));
            fields.push(format!("\"xz_err\":{xz:.6e}"));
        }
    }
    if scene == DemoScene::Mixed {
        if let Some(y) = min_dynamic_y(bodies) {
            fields.push(format!("\"min_y\":{y:.6e}"));
        }
    }
    if scene == DemoScene::Ramp {
        if let Some((x, y)) = first_dynamic_xy(bodies) {
            fields.push(format!("\"slide_x\":{x:.6e}"));
            fields.push(format!("\"slide_y\":{y:.6e}"));
        }
    }
    if scene == DemoScene::HighResistance {
        let dyns: Vec<&BodyGpu> = bodies.iter().filter(|b| b.inv_mass > 0.0).collect();
        fields.push(format!("\"capsules\":{}", dyns.len()));
        if let Some(b0) = dyns.first() {
            fields.push(format!("\"y0\":{:.6e}", b0.pos[1]));
            fields.push(format!(
                "\"support0\":{:.6e}",
                capsule_support_y(b0, HIGH_RESISTANCE_HALF_LEN, HIGH_RESISTANCE_RADIUS)
            ));
        }
    }
    if scene == DemoScene::MixedStacks {
        let dyns: Vec<&BodyGpu> = bodies.iter().filter(|b| b.inv_mass > 0.0).collect();
        fields.push(format!("\"boxes\":{}", dyns.len()));
        if let Some(y) = min_dynamic_y(bodies) {
            fields.push(format!("\"min_y\":{y:.6e}"));
        }
    }
    fields
}
