//! Safe Rust surface that mirrors Box3D `b3*` names and ID layouts.
//!
//! C ABI (`crate::c_abi`) re-exports a subset as `b3CreateWorld` / … for linking
//! one upstream sample. GPU-vs-CPU Box3D frames are not a bit-identical gate.

pub(crate) mod contact_data;
mod ids;
mod query;
mod types_api;
mod world;

pub use contact_data::{ContactData, Manifold, ManifoldPoint};
pub use ids::*;
pub use query::*;
pub use types_api::*;
pub use world::*;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn null_ids_are_zero() {
        assert_eq!(b3_null_world_id().index1, 0);
        assert_eq!(b3_null_world_id().generation, 0);
        assert_eq!(b3_null_body_id().index1, 0);
        assert_eq!(b3_null_shape_id().index1, 0);
        assert!(!b3_shape_id_is_valid(b3_null_shape_id()));
        assert!(!b3_joint_id_is_valid(b3_null_joint_id()));
        assert_eq!(b3_null_joint_id().index1, 0);
    }

    #[test]
    fn defaults_match_box3d_cpu() {
        let w = b3_default_world_def();
        assert_eq!(w.gravity, [0.0, -10.0, 0.0]);
        assert_eq!(w.hit_event_threshold, 1.0);
        assert!((w.contact_hertz - 30.0).abs() < 1e-6);
        assert!(w.enable_continuous);
        assert_eq!(w.capacity.dynamic_body_count, 0);
        assert_eq!(w.capacity.static_body_count, 0);
        let b = b3_default_body_def();
        assert_eq!(b.body_type, BodyType::Static);
        assert_eq!(b.rotation, [0.0, 0.0, 0.0, 1.0]);
        assert_eq!(b.gravity_scale, 1.0);
        assert!(b.is_awake);
        assert!(!b.is_bullet);
        assert!(!b.allow_fast_rotation);
        let s = b3_default_shape_def();
        assert!((s.density - 1000.0).abs() < 1e-3);
        assert!((s.friction - 0.6).abs() < 1e-6);
        assert_eq!(s.restitution, 0.0);
        assert_eq!(s.explosion_scale, 1.0);
        assert_eq!(s.filter.category_bits, 1);
        assert_eq!(s.filter.mask_bits, u64::MAX);
        assert_eq!(s.filter.group_index, 0);
        assert!(!s.is_sensor);
        assert!(!s.enable_sensor_events);
        assert!(!s.enable_contact_events);
        assert!(!s.enable_hit_events);
        assert!(!s.enable_custom_filtering);
        assert!(!s.enable_pre_solve_events);
        assert!(s.enable_speculative_contact);
        assert_eq!(s.user_material_id, 0);
        let query = b3_default_query_filter();
        assert_eq!(query.category_bits, u64::MAX);
        assert_eq!(query.mask_bits, u64::MAX);
        assert_eq!(query.id, 0);
        assert!(query.name.is_null());
        let explosion = b3_default_explosion_def();
        assert_eq!(explosion.mask_bits, u64::MAX);
        assert_eq!(explosion.position, [0.0; 3]);
        assert_eq!(explosion.radius, 0.0);
        assert_eq!(explosion.falloff, 0.0);
        assert_eq!(explosion.impulse_per_area, 0.0);
        let r = b3_default_revolute_joint_def();
        assert_eq!(r.local_rotation_a, [0.0, 0.0, 0.0, 1.0]);
        assert_eq!(r.local_rotation_b, [0.0, 0.0, 0.0, 1.0]);
        assert!((r.hertz - 60.0).abs() < 1e-6);
        assert!((r.damping - 2.0).abs() < 1e-6);
        assert!(!r.collide_connected);
        assert!(!r.enable_spring);
        assert!(!r.enable_limit);
        assert!(!r.enable_motor);
        let spherical = b3_default_spherical_joint_def();
        assert_eq!(spherical.local_rotation_a, [0.0, 0.0, 0.0, 1.0]);
        assert_eq!(spherical.local_rotation_b, [0.0, 0.0, 0.0, 1.0]);
        assert_eq!(spherical.target_rotation, [0.0, 0.0, 0.0, 1.0]);
        assert!((spherical.hertz - 60.0).abs() < 1e-6);
        assert!((spherical.damping - 2.0).abs() < 1e-6);
        assert!(!spherical.collide_connected);
        assert!(!spherical.enable_spring);
        assert!(!spherical.enable_cone_limit);
        assert!(!spherical.enable_twist_limit);
        assert!(!spherical.enable_motor);
        let prismatic = b3_default_prismatic_joint_def();
        assert_eq!(prismatic.local_axis_a, [1.0, 0.0, 0.0]);
        assert!((prismatic.hertz - 60.0).abs() < 1e-6);
        assert!((prismatic.damping - 2.0).abs() < 1e-6);
        assert!(!prismatic.enable_spring);
        assert!(!prismatic.enable_limit);
        assert!(!prismatic.enable_motor);
        let distance = b3_default_distance_joint_def();
        assert_eq!(distance.length, 1.0);
        assert_eq!(distance.lower_spring_force, -f32::MAX);
        assert_eq!(distance.upper_spring_force, f32::MAX);
        assert!(!distance.enable_spring);
        assert!(!distance.enable_limit);
        assert!(!distance.enable_motor);
        let parallel = b3_default_parallel_joint_def();
        assert_eq!(parallel.hertz, 1.0);
        assert_eq!(parallel.damping, 1.0);
        assert_eq!(parallel.max_torque, f32::MAX);
        let filter = b3_default_filter_joint_def();
        assert_eq!(filter.body_a, b3_null_body_id());
        assert_eq!(filter.body_b, b3_null_body_id());
        let weld = b3_default_weld_joint_def();
        assert!((weld.hertz - 60.0).abs() < 1e-6);
        assert!((weld.damping - 2.0).abs() < 1e-6);
        assert_eq!(weld.local_rotation_a, [0.0, 0.0, 0.0, 1.0]);
        assert_eq!(weld.local_rotation_b, [0.0, 0.0, 0.0, 1.0]);
        assert!(!weld.collide_connected);
        assert_eq!(weld.linear_hertz, 0.0);
        assert_eq!(weld.linear_damping_ratio, 0.0);
        assert_eq!(weld.angular_hertz, 0.0);
        assert_eq!(weld.angular_damping_ratio, 0.0);
        let wheel = b3_default_wheel_joint_def();
        assert_eq!(wheel.local_rotation_a, [0.0, 0.0, 0.0, 1.0]);
        assert_eq!(wheel.local_rotation_b, [0.0, 0.0, 0.0, 1.0]);
        assert!((wheel.hertz - 60.0).abs() < 1e-6);
        assert!((wheel.damping - 2.0).abs() < 1e-6);
        assert!(wheel.enable_suspension_spring);
        assert!((wheel.suspension_hertz - 1.0).abs() < 1e-6);
        assert!((wheel.suspension_damping_ratio - 0.7).abs() < 1e-6);
        assert!((wheel.steering_hertz - 1.0).abs() < 1e-6);
        assert!((wheel.steering_damping_ratio - 0.7).abs() < 1e-6);
        assert!(!wheel.enable_suspension_limit);
        assert!(!wheel.enable_spin_motor);
        assert!(!wheel.enable_steering);
        assert!(!wheel.enable_steering_limit);
        assert_eq!(B3_SECRET_COOKIE, 1_152_023);
    }

    #[test]
    fn all_joint_threshold_defaults_disable_events() {
        macro_rules! check {
            ($def:expr) => {{
                let def = $def;
                assert_eq!(def.force_threshold, f32::MAX);
                assert_eq!(def.torque_threshold, f32::MAX);
                assert_eq!(def.user_data, 0);
                assert!(!def.collide_connected);
            }};
        }
        check!(b3_default_revolute_joint_def());
        check!(b3_default_spherical_joint_def());
        check!(b3_default_prismatic_joint_def());
        check!(b3_default_distance_joint_def());
        check!(b3_default_parallel_joint_def());
        check!(b3_default_filter_joint_def());
        check!(b3_default_motor_joint_def());
        check!(b3_default_weld_joint_def());
        check!(b3_default_wheel_joint_def());
    }

    #[test]
    fn box_hull_half_extents() {
        let h = b3_make_box_hull(2.0, 0.5, 3.0);
        assert_eq!(h.half_extents, [2.0, 0.5, 3.0]);
        let c = b3_make_cube_hull(0.5);
        assert_eq!(c.half_extents, [0.5, 0.5, 0.5]);
        let q = b3_make_quat_from_axis_angle([0.0, 0.0, 1.0], -20.0 * B3_DEG_TO_RAD);
        assert!((q[2] + 0.17524168).abs() < 2e-6);
        assert!((q[3] - 0.9845255).abs() < 2e-6);
    }

    #[test]
    fn shape_filter_matches_box3d_group_override_and_masks() {
        let mut a = b3_default_filter();
        let mut b = b3_default_filter();
        a.category_bits = 1 << 40;
        a.mask_bits = 1 << 41;
        b.category_bits = 1 << 41;
        b.mask_bits = 1 << 40;
        assert!(b3_should_shapes_collide(a, b));
        b.mask_bits = 0;
        assert!(!b3_should_shapes_collide(a, b));
        a.group_index = 7;
        b.group_index = 7;
        assert!(b3_should_shapes_collide(a, b));
        a.group_index = -7;
        b.group_index = -7;
        assert!(!b3_should_shapes_collide(a, b));
    }

    #[test]
    fn sensor_event_abi_preserves_roles() {
        let sensor_shape_id = ShapeId {
            index1: 3,
            world0: 2,
            generation: 7,
        };
        let visitor_shape_id = ShapeId {
            index1: 5,
            world0: 2,
            generation: 11,
        };
        let begin = SensorBeginTouchEvent {
            sensor_shape_id,
            visitor_shape_id,
        };
        let end = SensorEndTouchEvent {
            sensor_shape_id,
            visitor_shape_id,
        };
        assert_eq!(begin.sensor_shape_id, sensor_shape_id);
        assert_eq!(begin.visitor_shape_id, visitor_shape_id);
        assert_eq!(end.sensor_shape_id, sensor_shape_id);
        assert_eq!(end.visitor_shape_id, visitor_shape_id);
        assert_eq!(core::mem::size_of_val(&begin), 16);
    }
}

#[cfg(all(feature = "replay-diagnostics", not(target_arch = "wasm32")))]
pub(crate) use world::seed_drag_snapshot;
