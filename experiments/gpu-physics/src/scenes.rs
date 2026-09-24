use crate::api::{
    b3_body_apply_linear_impulse, b3_body_mark_hidden, b3_create_body, b3_create_capsule_shape,
    b3_create_hull_shape, b3_create_revolute_joint, b3_create_sphere_shape, b3_create_weld_joint,
    b3_create_world, b3_default_body_def, b3_default_revolute_joint_def, b3_default_shape_def,
    b3_default_weld_joint_def, b3_default_world_def, b3_make_box_hull, b3_make_cube_hull,
    b3_make_quat_from_axis_angle, b3_world_set_contacts, b3_world_set_jacobi, BodyType, Capsule,
    Sphere, WorldId, B3_DEG_TO_RAD,
};
use crate::sim::GpuDevice;
use crate::types::{DemoConfig, DemoScene, SPHERE_RADIUS};

pub fn build_demo_world(gpu: GpuDevice, cfg: &DemoConfig) -> WorldId {
    let def = b3_default_world_def();
    let world = b3_create_world(gpu, &def);
    b3_world_set_contacts(world, cfg.contacts);
    b3_world_set_jacobi(world, cfg.jacobi);
    match cfg.scene {
        DemoScene::Spheres => {
            create_ground(world, 12.0);
            create_sphere_lattice(world, cfg.body_count);
        }
        DemoScene::Stack => {
            create_ground(world, 12.0);
            create_toy_box_stack(world);
        }
        DemoScene::SingleBox => {
            create_ground(world, 20.0);
            create_single_box(world);
        }
        DemoScene::BoxStack => {
            create_ground(world, 40.0);
            create_box_stack(world);
        }
        DemoScene::SphereStack => {
            create_ground(world, 15.0);
            create_sphere_stack(world);
        }
        DemoScene::CapsuleStack => {
            create_ground(world, 40.0);
            create_capsule_stack(world);
        }
        DemoScene::Revolute => {
            create_ground(world, 20.0);
            create_revolute(world);
        }
        DemoScene::Weld => {
            create_ground(world, 20.0);
            create_weld(world);
        }
        DemoScene::AnchoredMechanisms => {
            let ground = create_ground(world, 80.0);
            create_anchored_mechanisms(world, ground, cfg.body_count.max(1));
        }
        DemoScene::JointChain => {
            let ground = create_ground(world, 40.0);
            create_joint_chain(world, ground, cfg.body_count.max(2));
        }
        DemoScene::Pyramid => {
            create_ground(world, 20.0);
            create_pyramid(world);
        }
        DemoScene::Bounce => {
            create_ground(world, 20.0);
            create_bounce(world);
        }
        DemoScene::Mixed => {
            create_ground(world, 20.0);
            create_mixed(world);
        }
        DemoScene::Spinner => {
            create_ground(world, 20.0);
            create_spinner(world);
        }
        DemoScene::Ramp => {
            create_ground(world, 20.0);
            create_ramp(world);
        }
        DemoScene::Dominoes => {
            let rings = crate::types::scene_scale_count(
                cfg.scene,
                cfg.body_count,
                cfg.body_count_explicit,
            );
            create_ground(world, 80.0);
            create_dominoes(world, rings);
        }
        DemoScene::HighResistance => {
            create_high_resistance(world);
        }
        DemoScene::FallingCubes => create_falling_cubes(world, cfg.body_count.max(1)),
        DemoScene::MixedStacks => {
            let stacks = crate::types::scene_scale_count(
                cfg.scene,
                cfg.body_count,
                cfg.body_count_explicit,
            );
            create_mixed_stacks(world, stacks);
        }
    }
    world
}

pub fn create_ground(world: WorldId, extent: f32) -> crate::api::BodyId {
    let mut body_def = b3_default_body_def();
    body_def.position = [0.0, -1.0, 0.0];
    let ground = b3_create_body(world, &body_def);
    let hull = b3_make_box_hull(extent, 1.0, extent);
    let shape_def = b3_default_shape_def();
    b3_create_hull_shape(ground, &shape_def, &hull);
    b3_body_mark_hidden(ground);
    ground
}

fn create_sphere_lattice(world: WorldId, count: u32) {
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    let shape_def = b3_default_shape_def();
    let sphere = Sphere {
        center: [0.0, 0.0, 0.0],
        radius: SPHERE_RADIUS,
    };
    let cols = (count as f32).cbrt().ceil() as u32;
    let cols = cols.max(1);
    for i in 0..count {
        let x = i % cols;
        let yz = i / cols;
        let z = yz % cols;
        let y = yz / cols;
        let px = (x as f32 - 0.5 * (cols as f32 - 1.0)) * (SPHERE_RADIUS * 2.4);
        let py = 1.2 + y as f32 * (SPHERE_RADIUS * 2.4);
        let pz = (z as f32 - 0.5 * (cols as f32 - 1.0)) * (SPHERE_RADIUS * 2.4);
        body_def.position = [px, py, pz];
        let body = b3_create_body(world, &body_def);
        b3_create_sphere_shape(body, &shape_def, &sphere);
    }
}

fn create_toy_box_stack(world: WorldId) {
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    let shape_def = b3_default_shape_def();
    let hull = b3_make_cube_hull(0.5);
    for i in 0..6 {
        body_def.position = [0.0, 0.5 + i as f32 * 1.05, 0.0];
        let body = b3_create_body(world, &body_def);
        b3_create_hull_shape(body, &shape_def, &hull);
    }
}

fn create_single_box(world: WorldId) {
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 0.5, 0.0];
    let body = b3_create_body(world, &body_def);
    let shape_def = b3_default_shape_def();
    b3_create_hull_shape(body, &shape_def, &b3_make_cube_hull(0.5));
}

fn create_box_stack(world: WorldId) {
    let a = 0.5f32;
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    let mut shape_def = b3_default_shape_def();
    shape_def.rolling_resistance = 0.1;
    let cube = b3_make_box_hull(a, a, a);
    for i in 0..40 {
        body_def.position = [0.0, 1.5 * a + 2.5 * a * i as f32, 0.0];
        let body = b3_create_body(world, &body_def);
        b3_create_hull_shape(body, &shape_def, &cube);
    }
}

fn create_sphere_stack(world: WorldId) {
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    let mut shape_def = b3_default_shape_def();
    shape_def.rolling_resistance = 0.1;
    let r = 0.5f32;
    let sphere = Sphere {
        center: [0.0, 0.0, 0.0],
        radius: r,
    };
    let mut y = 1.5 * r;
    for _ in 0..30 {
        body_def.position = [0.0, y, 0.0];
        let body = b3_create_body(world, &body_def);
        b3_create_sphere_shape(body, &shape_def, &sphere);
        y += 3.0 * r;
    }
}

fn create_capsule_stack(world: WorldId) {
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.motion_locks.linear_z = true;
    body_def.motion_locks.angular_x = true;
    body_def.motion_locks.angular_y = true;
    body_def.motion_locks.angular_z = true;
    let shape_def = b3_default_shape_def();
    let r = 0.5f32;
    let capsule = Capsule {
        center1: [-1.0, 0.0, 0.0],
        center2: [1.0, 0.0, 0.0],
        radius: r,
    };
    let mut y = 1.5 * r;
    for _ in 0..20 {
        body_def.position = [0.0, y, 0.0];
        let body = b3_create_body(world, &body_def);
        b3_create_capsule_shape(body, &shape_def, &capsule);
        y += 2.0 * r;
    }
}

fn joint_anchor_body(world: WorldId) -> crate::api::BodyId {
    let mut body_def = b3_default_body_def();
    body_def.position = [0.0, -1.0, 0.0];
    b3_create_body(world, &body_def)
}

pub fn create_revolute(world: WorldId) {
    let ground_id = joint_anchor_body(world);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 4.0, 0.0];
    let body = b3_create_body(world, &body_def);
    let shape_def = b3_default_shape_def();
    b3_create_hull_shape(body, &shape_def, &b3_make_box_hull(0.5, 1.5, 0.25));
    let mut joint = b3_default_revolute_joint_def();
    joint.body_a = ground_id;
    joint.body_b = body;
    joint.local_anchor_a = [0.0, 6.5, 0.0];
    joint.local_anchor_b = [0.0, 1.5, 0.0];
    b3_create_revolute_joint(world, &joint);
}

fn create_weld(world: WorldId) {
    let ground_id = joint_anchor_body(world);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 4.0, 0.0];
    body_def.gravity_scale = 0.0;
    let body = b3_create_body(world, &body_def);
    let shape_def = b3_default_shape_def();
    b3_create_hull_shape(body, &shape_def, &b3_make_box_hull(0.5, 1.5, 0.25));
    let mut joint = b3_default_weld_joint_def();
    joint.body_a = ground_id;
    joint.body_b = body;
    joint.local_anchor_a = [0.0, 6.5, 0.0];
    joint.local_anchor_b = [0.0, 1.5, 0.0];
    joint.hertz = 240.0;
    b3_create_weld_joint(world, &joint);
}

/// Independent revolute pendulums sharing one static ground. `count` is the
/// number of writable bodies (and joints), not including the ground.
pub fn create_anchored_mechanisms(world: WorldId, ground: crate::api::BodyId, count: u32) {
    let shape_def = b3_default_shape_def();
    let hull = b3_make_box_hull(0.25, 1.0, 0.25);
    for i in 0..count {
        let x = (i as f32 - 0.5 * (count as f32 - 1.0)) * 1.2;
        let mut body_def = b3_default_body_def();
        body_def.body_type = BodyType::Dynamic;
        body_def.position = [x, 4.0, 0.0];
        let body = b3_create_body(world, &body_def);
        b3_create_hull_shape(body, &shape_def, &hull);
        let mut joint = b3_default_revolute_joint_def();
        joint.body_a = ground;
        joint.body_b = body;
        joint.local_anchor_a = [x, 6.0, 0.0];
        joint.local_anchor_b = [0.0, 1.0, 0.0];
        b3_create_revolute_joint(world, &joint);
    }
}

/// One connected revolute chain. `count` is the number of dynamic links.
pub fn create_joint_chain(world: WorldId, ground: crate::api::BodyId, count: u32) {
    let mut prev = ground;
    let shape_def = b3_default_shape_def();
    let hull = b3_make_box_hull(0.25, 0.4, 0.25);
    for i in 0..count {
        let mut body_def = b3_default_body_def();
        body_def.body_type = BodyType::Dynamic;
        body_def.position = [0.0, 0.5 + i as f32 * 0.9, 0.0];
        let body = b3_create_body(world, &body_def);
        b3_create_hull_shape(body, &shape_def, &hull);
        let mut joint = b3_default_revolute_joint_def();
        joint.body_a = prev;
        joint.body_b = body;
        joint.local_anchor_a = [0.0, 0.4, 0.0];
        joint.local_anchor_b = [0.0, -0.4, 0.0];
        b3_create_revolute_joint(world, &joint);
        prev = body;
    }
}

/// Box3D `Pyramid`: 10 rows of 0.5 cubes along X, tight in Z.
fn create_pyramid(world: WorldId) {
    let a = 0.5f32;
    let rows = 10i32;
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    let mut shape_def = b3_default_shape_def();
    shape_def.rolling_resistance = 0.1;
    let cube = b3_make_cube_hull(a);
    for i in 0..rows {
        let count = rows - i;
        for j in 0..count {
            let x = (2.0 * j as f32 - (count as f32 - 1.0)) * a;
            let y = a + 2.0 * a * i as f32;
            body_def.position = [x, y, 0.0];
            let body = b3_create_body(world, &body_def);
            b3_create_hull_shape(body, &shape_def, &cube);
        }
    }
}

/// Drop a bouncy sphere from y=5. Restitution 1 should keep it hopping.
fn create_bounce(world: WorldId) {
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 5.0, 0.0];
    let body = b3_create_body(world, &body_def);
    let mut shape_def = b3_default_shape_def();
    shape_def.restitution = 1.0;
    let sphere = Sphere {
        center: [0.0, 0.0, 0.0],
        radius: 0.5,
    };
    b3_create_sphere_shape(body, &shape_def, &sphere);
}

/// Side-by-side box / sphere / capsule so mixed collide is obvious.
fn create_mixed(world: WorldId) {
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    let shape_def = b3_default_shape_def();
    body_def.position = [-2.0, 3.0, 0.0];
    let box_id = b3_create_body(world, &body_def);
    b3_create_hull_shape(box_id, &shape_def, &b3_make_cube_hull(0.5));
    body_def.position = [0.0, 3.0, 0.0];
    let sph_id = b3_create_body(world, &body_def);
    b3_create_sphere_shape(
        sph_id,
        &shape_def,
        &Sphere {
            center: [0.0, 0.0, 0.0],
            radius: 0.5,
        },
    );
    body_def.position = [2.0, 3.0, 0.0];
    let cap_id = b3_create_body(world, &body_def);
    b3_create_capsule_shape(
        cap_id,
        &shape_def,
        &Capsule {
            center1: [-0.5, 0.0, 0.0],
            center2: [0.5, 0.0, 0.0],
            radius: 0.35,
        },
    );
}

/// Gravity off, spin about Z — gyro / angular damping check.
fn create_spinner(world: WorldId) {
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [0.0, 4.0, 0.0];
    body_def.gravity_scale = 0.0;
    body_def.angular_velocity = [0.0, 0.0, 8.0];
    let body = b3_create_body(world, &body_def);
    let shape_def = b3_default_shape_def();
    b3_create_hull_shape(body, &shape_def, &b3_make_box_hull(1.0, 0.15, 0.6));
}

/// Tilted static plank + a cube. Friction should slow the slide.
fn create_ramp(world: WorldId) {
    let mut ramp_def = b3_default_body_def();
    ramp_def.position = [0.0, 1.5, 0.0];
    ramp_def.rotation = b3_make_quat_from_axis_angle([0.0, 0.0, 1.0], -20.0 * B3_DEG_TO_RAD);
    let ramp = b3_create_body(world, &ramp_def);
    let shape_def = b3_default_shape_def();
    b3_create_hull_shape(ramp, &shape_def, &b3_make_box_hull(6.0, 0.2, 1.5));
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.position = [-4.0, 4.2, 0.0];
    let cube = b3_create_body(world, &body_def);
    b3_create_hull_shape(cube, &shape_def, &b3_make_cube_hull(0.4));
}

/// Overlapping static boxes plus `count` dynamic boxes in two-layer stacks.
/// Static degree is 2 per ground contact; this is the parallel-graph stress case.
/// Default demo uses 600 dynamics (300 two-box stacks).
fn mixed_stacks_ground_half_extent(count: u32) -> f32 {
    let per_layer = count.max(2).div_ceil(2);
    // Keep the original small fixture, but support every row at larger scales.
    // The CPU oracle uses the same square ground and three-metre edge margin.
    100.0_f32.max(3.0 * ((per_layer - 1) / 20) as f32 + 3.0)
}

pub(crate) fn mixed_stacks_position(count: u32, index: u32) -> [f32; 3] {
    let per_layer = count.max(2).div_ceil(2);
    let within_layer = index % per_layer;
    [3.0 * (within_layer % 20) as f32,
     0.5 + u32::from(index >= per_layer) as f32,
     3.0 * (within_layer / 20) as f32]
}

pub fn create_mixed_stacks(world: WorldId, count: u32) {
    let ground_half = mixed_stacks_ground_half_extent(count);
    let shape_def = b3_default_shape_def();
    for _ in 0..2 {
        let mut ground_def = b3_default_body_def();
        ground_def.position = [0.0, -1.0, 0.0];
        let ground = b3_create_body(world, &ground_def);
        b3_create_hull_shape(ground, &shape_def, &b3_make_box_hull(ground_half, 1.0, ground_half));
        b3_body_mark_hidden(ground);
    }
    let cube = b3_make_cube_hull(0.5);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    let count = count.max(2);
    for i in 0..count {
        body_def.position = mixed_stacks_position(count, i);
        let body = b3_create_body(world, &body_def);
        b3_create_hull_shape(body, &shape_def, &cube);
    }
}

/// Upstream `Shapes / High Resistance`: ten capsules with increasing rolling resistance.
pub fn create_high_resistance(world: WorldId) {
    create_ground(world, 50.0);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    body_def.rotation = b3_make_quat_from_axis_angle([0.0, 0.0, 1.0], 30.0 * B3_DEG_TO_RAD);
    let capsule = Capsule {
        center1: [0.0, -1.0, 0.0],
        center2: [0.0, 1.0, 0.0],
        radius: 0.5,
    };
    for index in 0..10 {
        body_def.position = [-22.0 + 5.0 * index as f32, 1.5, 0.0];
        let body = b3_create_body(world, &body_def);
        let mut shape_def = b3_default_shape_def();
        shape_def.rolling_resistance = 0.2 * index as f32;
        b3_create_capsule_shape(body, &shape_def, &capsule);
    }
}

/// Upstream `Dominoes`: `rings` concentric rings, 181 hull boxes each, impulse on α=0.
fn create_dominoes(world: WorldId, rings: u32) {
    let box_hull = b3_make_box_hull(0.2, 0.8, 0.05);
    let mut body_def = b3_default_body_def();
    body_def.body_type = BodyType::Dynamic;
    let shape_def = b3_default_shape_def();
    for ring in 0..rings {
        let radius = 7.0 + 1.1 * ring as f32;
        let mut alpha = 0.0f32;
        while alpha <= 360.0 {
            let rad = B3_DEG_TO_RAD * alpha;
            let (cosine, sine) = crate::api::b3_compute_cos_sin(rad);
            let mut position = [radius * cosine, 0.8, radius * sine];
            position[0] -= alpha / 630.0 * cosine;
            position[2] -= alpha / 630.0 * sine;
            body_def.position = position;
            body_def.rotation = b3_make_quat_from_axis_angle([0.0, 1.0, 0.0], -rad);
            let body = b3_create_body(world, &body_def);
            b3_create_hull_shape(body, &shape_def, &box_hull);
            if alpha == 0.0 {
                b3_body_apply_linear_impulse(
                    body,
                    [0.0, 0.0, 25.0],
                    [position[0], position[1] + 0.8, position[2]],
                    true,
                );
            }
            alpha += 2.0;
        }
    }
}

#[cfg(test)]
mod fixture_bounds_tests {
    use super::{mixed_stacks_ground_half_extent, mixed_stacks_position};

    #[test]
    fn scaled_mixed_stacks_keep_every_cube_over_ground() {
        assert_eq!(mixed_stacks_ground_half_extent(600), 100.0);
        for count in [2u32, 64, 600, 1024, 2048, 4095, 4096, 4097, 65534] {
            let half = mixed_stacks_ground_half_extent(count);
            let per_layer = count.div_ceil(2);
            for i in 0..count {
                let [x, y, z] = mixed_stacks_position(count, i);
                if i >= per_layer {
                    let lower = mixed_stacks_position(count, i - per_layer);
                    assert_eq!([x, y - 1.0, z], lower, "upper cube must have a lower support");
                }
                assert!(x + 0.5 < half && z + 0.5 < half,
                    "cube {i}/{count} at ({x}, {z}) extends beyond ground {half}");
            }
        }
    }
}

/// Falling-cubes v1; exact counterpart of native-samples/falling-cubes.h.
pub fn create_falling_cubes(world: WorldId, count: u32) {
    let columns = count.div_ceil(10);
    let mut width = 1;
    while width * width < columns { width += 1; }
    create_ground(world, 1.25 * width as f32 + 12.0);
    let shape = b3_default_shape_def();
    let cube = b3_make_cube_hull(0.5);
    let mut body = b3_default_body_def();
    body.body_type = BodyType::Dynamic;
    for i in 0..count {
        let layer = i / columns;
        let column = i % columns;
        let offset = if layer % 2 == 1 { 0.125 } else { -0.125 };
        let center = 0.625 * (width - 1) as f32;
        body.position = [1.25 * (column % width) as f32 - center + offset,
                         6.0 + 1.25 * layer as f32,
                         1.25 * (column / width) as f32 - center - offset];
        b3_create_hull_shape(b3_create_body(world, &body), &shape, &cube);
    }
}
