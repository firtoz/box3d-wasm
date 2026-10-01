//! Thin `extern "C"` that matches `box3d.h` for cohort samples. GPU engine is the Rust `b3_*` API.

use crate::api::{
    b3_body_allow_fast_rotation, b3_body_apply_angular_impulse, b3_body_apply_force,
    b3_body_apply_force_to_center, b3_body_apply_linear_impulse,
    b3_body_apply_linear_impulse_to_center, b3_body_apply_mass_from_shapes, b3_body_apply_torque,
    b3_body_get_angular_damping, b3_body_get_angular_velocity, b3_body_get_gravity_scale,
    b3_body_get_joint_count, b3_body_get_joints, b3_body_get_linear_damping, b3_body_get_linear_velocity,
    b3_body_get_local_center, b3_body_get_local_point, b3_body_get_local_point_velocity,
    b3_body_get_local_vector, b3_body_get_mass, b3_body_get_mass_data, b3_body_get_position,
    b3_body_get_rotation, b3_body_get_shape_count, b3_body_get_shapes, b3_body_get_transform,
    b3_body_get_type,
    b3_body_get_user_data, b3_body_get_world_center, b3_body_get_world_point,
    b3_body_get_world_point_velocity, b3_body_get_world_vector, b3_body_inv_mass, b3_body_is_awake,
    b3_body_is_bullet, b3_body_is_enabled, b3_body_is_fast_rotation_allowed, b3_body_is_static,
    b3_body_is_valid, b3_body_set_angular_damping, b3_body_set_angular_velocity, b3_body_set_awake,
    b3_body_set_bullet, b3_body_set_enabled, b3_body_set_gravity_scale, b3_body_set_linear_damping,
    b3_body_set_linear_velocity, b3_body_set_mass_data, b3_body_set_motion_locks, b3_body_get_motion_locks,
    b3_body_set_target_transform, b3_body_set_transform,
    b3_body_set_type, b3_body_set_user_data, b3_create_body,     b3_create_capsule_shape,
    b3_create_compound_parent, b3_create_convex_hull_shape, b3_create_distance_joint, b3_create_filter_joint,
    b3_create_height_field_shape, b3_create_hull_shape, b3_create_mesh_shape,
    b3_create_motor_joint, b3_create_parallel_joint, b3_create_prismatic_joint,
    b3_create_revolute_joint, b3_create_sphere_shape, b3_create_spherical_joint,
    b3_create_weld_joint, b3_create_world, b3_default_body_def, b3_default_distance_joint_def,
    b3_default_filter_joint_def, b3_default_motor_joint_def, b3_default_parallel_joint_def,
    b3_default_prismatic_joint_def, b3_default_revolute_joint_def, b3_default_spherical_joint_def,
    b3_default_weld_joint_def, b3_default_world_def, b3_destroy_body, b3_destroy_joint,
    b3_destroy_shape, b3_destroy_world, b3_joint_get_force_threshold,
    b3_joint_get_torque_threshold, b3_joint_get_user_data, b3_joint_is_valid,
    b3_joint_set_force_threshold, b3_joint_set_torque_threshold, b3_joint_set_user_data,
    b3_live_world_count, b3_shape_are_contact_events_enabled, b3_shape_are_hit_events_enabled,
    b3_shape_are_pre_solve_events_enabled, b3_shape_are_sensor_events_enabled,
    b3_shape_attach_compound_child_indexed, b3_shape_body,
    b3_shape_enable_speculative_contact, b3_shape_enable_contact_events, b3_shape_enable_custom_filtering, b3_shape_enable_hit_events,
    b3_shape_enable_pre_solve_events, b3_shape_enable_sensor_events, b3_shape_get_filter,
    b3_shape_get_mesh_material, b3_shape_get_sensor_capacity, b3_shape_get_sensor_data,
    b3_shape_get_surface_material, b3_shape_is_custom_filtering_enabled, b3_shape_is_sensor,
    b3_shape_is_valid, b3_shape_kind, b3_shape_set_explosion_scale, b3_shape_set_filter,
    b3_shape_set_mesh_material, b3_shape_set_mesh_material_count, b3_shape_set_sensor,
    b3_shape_set_friction, b3_shape_set_restitution, b3_shape_apply_wind,
    b3_shape_set_surface_material, b3_shape_set_user_material_id,
    b3_weld_joint_get_angular_damping_ratio, b3_weld_joint_get_angular_hertz,
    b3_weld_joint_get_linear_damping_ratio, b3_weld_joint_get_linear_hertz,
    b3_weld_joint_set_angular_damping_ratio, b3_weld_joint_set_angular_hertz,
    b3_weld_joint_set_linear_damping_ratio, b3_weld_joint_set_linear_hertz, b3_world_aabb,
    b3_world_body_event_ptrs, b3_world_contact_event_ptrs, b3_world_counts, b3_world_draw_items,
    b3_world_enable_continuous, b3_world_enable_sleeping, b3_world_get_gravity,
    b3_world_get_hit_event_threshold,
    b3_world_is_continuous_enabled, b3_world_is_sleeping_enabled, b3_world_is_valid,
    b3_world_joint_event_ptrs, b3_world_last_encode_ms, b3_world_last_fetch_ms,
    b3_world_last_collide_ms, b3_world_last_solve_ms, b3_world_last_integrate_ms,
    b3_world_last_prepare_ms, b3_world_last_device_ms, b3_world_last_timestamp_step,
    b3_world_physics_step, b3_world_last_solver_dispatches, b3_world_last_static_sort_dispatches,
    b3_world_pose_export, b3_world_pose_export_live, b3_world_gpu_fail, b3_world_set_gpu_fail,
    b3_world_health_scan, WorldHealthScan,
    b3_world_pose_snapshot_epoch,
    b3_world_last_query_profile, b3_world_sensor_event_ptrs, b3_world_set_custom_filter_callback,
    b3_world_set_friction_callback, b3_world_set_gravity, b3_world_set_hit_event_threshold,
    b3_world_set_pre_solve_callback, b3_world_set_restitution_callback, b3_world_step_gpu, BodyId,
    BodyType, BoxHull, Capsule, ConvexHull, CustomFilterCallback, DrawItemC, FrictionCallback,
    JointId, MassData, MeshNode, MotionLocks, PreSolveCallback, RestitutionCallback, ShapeDef, ShapeId, Sphere,
    SurfaceMaterial, WorldId,
};
use crate::sim::GpuDevice;
use crate::types::{
    FLAG_LOCK_ANG_X, FLAG_LOCK_ANG_Y, FLAG_LOCK_ANG_Z, FLAG_LOCK_LIN_X, FLAG_LOCK_LIN_Y,
    FLAG_LOCK_LIN_Z,
};
use std::sync::OnceLock;

#[repr(C)]
pub struct Vec3C {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}

#[no_mangle]
pub extern "C" fn gpu_b3_default_query_filter() -> crate::api::QueryFilter {
    crate::api::b3_default_query_filter()
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_get_sphere(id: ShapeId) -> Sphere {
    crate::api::b3_shape_get_sphere(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_get_capsule(id: ShapeId) -> Capsule {
    crate::api::b3_shape_get_capsule(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_get_density(id: ShapeId) -> f32 {
    crate::api::b3_shape_get_density(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_get_friction(id: ShapeId) -> f32 {
    crate::api::b3_shape_get_friction(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_get_restitution(id: ShapeId) -> f32 {
    crate::api::b3_shape_get_restitution(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_set_user_data(id: ShapeId, value: usize) {
    crate::api::b3_shape_set_user_data(id, value);
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_get_user_data(id: ShapeId) -> usize {
    crate::api::b3_shape_get_user_data(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_get_aabb(id: ShapeId) -> crate::api::Aabb {
    crate::api::b3_shape_get_aabb(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_compute_aabb(id: BodyId) -> crate::api::Aabb {
    crate::api::b3_body_compute_aabb(id)
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_shape_get_closest_point(id: ShapeId, target: Vec3C) -> Vec3C {
    let point = crate::api::b3_shape_get_closest_point(id, [target.x, target.y, target.z]);
    Vec3C {
        x: point[0],
        y: point[1],
        z: point[2],
    }
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_body_get_closest_point(
    id: BodyId,
    result: *mut Vec3C,
    target: Vec3C,
) -> f32 {
    let (distance, point) =
        crate::api::b3_body_get_closest_point(id, [target.x, target.y, target.z]);
    if let Some(result) = result.as_mut() {
        *result = Vec3C {
            x: point[0],
            y: point[1],
            z: point[2],
        };
    }
    distance
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_ray_cast(
    id: ShapeId,
    origin: Vec3C,
    translation: Vec3C,
) -> crate::api::CastOutput {
    crate::api::b3_shape_ray_cast(
        id,
        [origin.x, origin.y, origin.z],
        [translation.x, translation.y, translation.z],
    )
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_cast_ray(
    id: BodyId,
    origin: Vec3C,
    translation: Vec3C,
    filter: crate::api::QueryFilter,
    max_fraction: f32,
    transform: crate::api::WorldTransform,
) -> crate::api::BodyCastResult {
    crate::api::b3_body_cast_ray(
        id,
        [origin.x, origin.y, origin.z],
        [translation.x, translation.y, translation.z],
        filter,
        max_fraction,
        transform,
    )
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_body_cast_shape(
    id: BodyId,
    origin: Vec3C,
    proxy: *const crate::api::ShapeProxy,
    translation: Vec3C,
    filter: crate::api::QueryFilter,
    max_fraction: f32,
    can_encroach: bool,
    transform: crate::api::WorldTransform,
) -> crate::api::BodyCastResult {
    let Some(proxy) = proxy.as_ref() else {
        return crate::api::BodyCastResult::default();
    };
    crate::api::b3_body_cast_shape(
        id,
        [origin.x, origin.y, origin.z],
        proxy,
        [translation.x, translation.y, translation.z],
        filter,
        max_fraction,
        can_encroach,
        transform,
    )
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_body_overlap_shape(
    id: BodyId,
    origin: Vec3C,
    proxy: *const crate::api::ShapeProxy,
    filter: crate::api::QueryFilter,
    transform: crate::api::WorldTransform,
) -> bool {
    proxy.as_ref().is_some_and(|proxy| {
        crate::api::b3_body_overlap_shape(
            id,
            [origin.x, origin.y, origin.z],
            proxy,
            filter,
            transform,
        )
    })
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_overlap_aabb(
    id: WorldId,
    aabb: crate::api::Aabb,
    filter: crate::api::QueryFilter,
    callback: Option<crate::api::OverlapResultFcn>,
    context: *mut core::ffi::c_void,
) -> crate::api::TreeStats {
    crate::api::b3_world_overlap_aabb(id, aabb, filter, callback, context)
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_overlap_shape(
    id: WorldId,
    origin: Vec3C,
    proxy: *const crate::api::ShapeProxy,
    filter: crate::api::QueryFilter,
    callback: Option<crate::api::OverlapResultFcn>,
    context: *mut core::ffi::c_void,
) -> crate::api::TreeStats {
    let Some(proxy) = proxy.as_ref() else {
        return crate::api::TreeStats::default();
    };
    crate::api::b3_world_overlap_shape(
        id,
        [origin.x, origin.y, origin.z],
        proxy,
        filter,
        callback,
        context,
    )
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_cast_ray(
    id: WorldId,
    origin: Vec3C,
    translation: Vec3C,
    filter: crate::api::QueryFilter,
    callback: Option<crate::api::CastResultFcn>,
    context: *mut core::ffi::c_void,
) -> crate::api::TreeStats {
    crate::api::b3_world_cast_ray(
        id,
        [origin.x, origin.y, origin.z],
        [translation.x, translation.y, translation.z],
        filter,
        callback,
        context,
    )
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_cast_ray_closest(
    id: WorldId,
    origin: Vec3C,
    translation: Vec3C,
    filter: crate::api::QueryFilter,
) -> crate::api::RayResult {
    crate::api::b3_world_cast_ray_closest(
        id,
        [origin.x, origin.y, origin.z],
        [translation.x, translation.y, translation.z],
        filter,
    )
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_cast_shape(
    id: WorldId,
    origin: Vec3C,
    proxy: *const crate::api::ShapeProxy,
    translation: Vec3C,
    filter: crate::api::QueryFilter,
    callback: Option<crate::api::CastResultFcn>,
    context: *mut core::ffi::c_void,
) -> crate::api::TreeStats {
    let Some(proxy) = proxy.as_ref() else {
        return crate::api::TreeStats::default();
    };
    crate::api::b3_world_cast_shape(
        id,
        [origin.x, origin.y, origin.z],
        proxy,
        [translation.x, translation.y, translation.z],
        filter,
        callback,
        context,
    )
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_cast_mover(
    id: WorldId,
    origin: Vec3C,
    mover: *const Capsule,
    translation: Vec3C,
    filter: crate::api::QueryFilter,
    callback: Option<crate::api::MoverFilterFcn>,
    context: *mut core::ffi::c_void,
) -> f32 {
    let Some(mover) = mover.as_ref() else {
        return 1.0;
    };
    crate::api::b3_world_cast_mover(
        id,
        [origin.x, origin.y, origin.z],
        mover,
        [translation.x, translation.y, translation.z],
        filter,
        callback,
        context,
    )
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_collide_mover(
    id: WorldId,
    origin: Vec3C,
    mover: *const Capsule,
    filter: crate::api::QueryFilter,
    callback: Option<crate::api::PlaneResultFcn>,
    context: *mut core::ffi::c_void,
) {
    if let Some(mover) = mover.as_ref() {
        let _ = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
            crate::api::b3_world_collide_mover(
                id,
                [origin.x, origin.y, origin.z],
                mover,
                filter,
                callback,
                context,
            );
        }));
    }
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_body_collide_mover(
    id: BodyId,
    output: *mut crate::api::BodyPlaneResult,
    capacity: i32,
    origin: Vec3C,
    mover: *const Capsule,
    filter: crate::api::QueryFilter,
    transform: crate::api::WorldTransform,
) -> i32 {
    if output.is_null() || capacity <= 0 {
        return 0;
    }
    let Some(mover) = mover.as_ref() else {
        return 0;
    };
    let output = core::slice::from_raw_parts_mut(output, capacity as usize);
    crate::api::b3_body_collide_mover(
        id,
        output,
        [origin.x, origin.y, origin.z],
        mover,
        filter,
        transform,
    ) as i32
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_solve_planes(
    target_delta: Vec3C,
    planes: *mut crate::api::CollisionPlane,
    count: i32,
) -> crate::api::PlaneSolverResult {
    let planes = if planes.is_null() || count <= 0 {
        &mut []
    } else {
        core::slice::from_raw_parts_mut(planes, count as usize)
    };
    crate::api::b3_solve_planes([target_delta.x, target_delta.y, target_delta.z], planes)
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_clip_vector(
    vector: Vec3C,
    planes: *const crate::api::CollisionPlane,
    count: i32,
) -> Vec3C {
    let planes = if planes.is_null() || count <= 0 {
        &[]
    } else {
        core::slice::from_raw_parts(planes, count as usize)
    };
    let result = crate::api::b3_clip_vector([vector.x, vector.y, vector.z], planes);
    Vec3C {
        x: result[0],
        y: result[1],
        z: result[2],
    }
}

static SHARED_DEVICE_STARTUP_NS: std::sync::atomic::AtomicU64 = std::sync::atomic::AtomicU64::new(0);

/// Internal startup telemetry; the shared device is initialized at most once.
#[no_mangle]
pub extern "C" fn gpu_b3_shared_device_startup_ns() -> u64 {
    SHARED_DEVICE_STARTUP_NS.load(std::sync::atomic::Ordering::Acquire)
}

fn shared_gpu() -> Result<GpuDevice, String> {
    static GPU: OnceLock<Result<GpuDevice, String>> = OnceLock::new();
    GPU.get_or_init(|| {
        let started = std::time::Instant::now();
        let result = pollster::block_on(GpuDevice::new(None));
        SHARED_DEVICE_STARTUP_NS.store(started.elapsed().as_nanos().min(u64::MAX as u128) as u64,
            std::sync::atomic::Ordering::Release);
        result
    })
        .clone()
}

#[no_mangle]
pub extern "C" fn gpu_b3_create_world(gx: f32, gy: f32, gz: f32) -> WorldId {
    gpu_b3_create_world_with_capacity(gx, gy, gz, 0, 0, 0, 0)
}

#[no_mangle]
pub extern "C" fn gpu_b3_create_world_with_capacity(
    gx: f32, gy: f32, gz: f32, static_bodies: i32, dynamic_bodies: i32,
    static_shapes: i32, dynamic_shapes: i32,
) -> WorldId {
    match shared_gpu() {
        Ok(gpu) => {
            let mut def = b3_default_world_def();
            def.gravity = [gx, gy, gz];
            def.capacity.static_body_count = static_bodies.max(0);
            def.capacity.dynamic_body_count = dynamic_bodies.max(0);
            def.capacity.static_shape_count = static_shapes.max(0);
            def.capacity.dynamic_shape_count = dynamic_shapes.max(0);
            let world = b3_create_world(gpu, &def);
            // Sokol consumes current full mirrors for queries/drawing. Its
            // automatic pose-only copy is otherwise superseded without use.
            if std::env::var("GPU_PHYSICS_SAMPLES_DEMAND_POSES").as_deref() == Ok("1") {
                crate::api::b3_world_set_automatic_pose_snapshots(world, false);
            }
            world
        }
        Err(e) => {
            eprintln!("gpu_b3_create_world: {e}");
            WorldId {
                index1: 0,
                generation: 0,
            }
        }
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_destroy_world(id: WorldId) {
    b3_destroy_world(id);
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_is_valid(id: WorldId) -> bool {
    b3_world_is_valid(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_step(id: WorldId, dt: f32, sub_step_count: i32) {
    b3_world_step_gpu(id, dt, sub_step_count);
}

/// Drain the GPU queue and copy completed poses into host getters.
#[no_mangle]
pub extern "C" fn gpu_b3_world_wait(id: WorldId) {
    crate::api::b3_world_gpu_wait(id);
    #[cfg(not(target_arch = "wasm32"))]
    {
        let _ = pollster::block_on(crate::api::b3_world_sync_from_gpu(id));
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_last_encode_ms(id: WorldId) -> f32 {
    b3_world_last_encode_ms(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_last_fetch_ms(id: WorldId) -> f32 {
    b3_world_last_fetch_ms(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_last_collide_ms(id: WorldId) -> f32 {
    b3_world_last_collide_ms(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_last_solve_ms(id: WorldId) -> f32 {
    b3_world_last_solve_ms(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_last_integrate_ms(id: WorldId) -> f32 {
    b3_world_last_integrate_ms(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_last_prepare_ms(id: WorldId) -> f32 {
    b3_world_last_prepare_ms(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_last_device_ms(id: WorldId) -> f32 {
    b3_world_last_device_ms(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_last_timestamp_step(id: WorldId) -> u64 {
    b3_world_last_timestamp_step(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_physics_step(id: WorldId) -> u64 {
    b3_world_physics_step(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_pose_snapshot_epoch(id: WorldId) -> u64 {
    b3_world_pose_snapshot_epoch(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_pose_snapshot_step(id: WorldId) -> u64 {
    crate::api::b3_world_pose_snapshot_step(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_completed_step(id: WorldId) -> u64 {
    crate::api::b3_world_completed_step(id).unwrap_or(0)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_completed_known(id: WorldId) -> i32 {
    i32::from(crate::api::b3_world_completed_known(id))
}

#[repr(C)]
pub struct GpuQueryProfile {
    pub wait_ms: f32,
    pub dispatch_ms: f32,
    pub map_ms: f32,
    pub copied_bytes: u64,
    pub encode_ms: f32,
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_last_query_profile(id: WorldId) -> GpuQueryProfile {
    let p = b3_world_last_query_profile(id);
    GpuQueryProfile {
        wait_ms: p.wait_ms,
        dispatch_ms: p.exact_ms,
        map_ms: p.map_ms,
        copied_bytes: p.copied_bytes,
        encode_ms: p.encode_ms,
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_last_solver_dispatches(id: WorldId) -> u32 {
    b3_world_last_solver_dispatches(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_last_static_sort_dispatches(id: WorldId) -> u32 {
    b3_world_last_static_sort_dispatches(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_pose_export_live(id: WorldId) -> bool {
    b3_world_pose_export_live(id)
}

/// Writes the exporting device's 16-byte Vulkan UUID; callers must provide 16 writable bytes.
#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_pose_export_uuid(id: WorldId, out: *mut u8) -> bool {
    if out.is_null() { return false; }
    let Some(uuid) = crate::api::b3_world_pose_export_uuid(id) else { return false; };
    unsafe { std::ptr::copy_nonoverlapping(uuid.as_ptr(), out, uuid.len()); }
    true
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_pose_export(id: WorldId) -> crate::pose_gl::PoseExportC {
    b3_world_pose_export(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_gpu_fail(id: WorldId) -> *const std::os::raw::c_char {
    b3_world_gpu_fail(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_health_scan(id: WorldId) -> WorldHealthScan {
    let health = b3_world_health_scan(id);
    if std::env::var_os("GPU_PHYSICS_TRACE_CONTACTS").is_some() {
        let contacts = pollster::block_on(crate::api::b3_world_sync_contacts(id));
        let body = std::env::var("GPU_PHYSICS_TRACE_BODY").ok().and_then(|s| s.parse::<u32>().ok());
        eprintln!("gpu-contact-trace step={} origin_y={}..{}", crate::api::b3_world_physics_step(id), health.min_y, health.max_y);
        for c in contacts.iter().filter(|c| c.a != u32::MAX && c.count > 0 && body.is_none_or(|b| c.a + 1 == b || c.b + 1 == b)) {
            eprintln!("gpu-contact-trace {c:?}");
        }
    }
    health
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_visit_dynamic_bodies(
    id: WorldId,
    callback: extern "C" fn(BodyId, *mut std::ffi::c_void),
    context: *mut std::ffi::c_void,
) {
    // Release the world lock before the visitor calls ordinary body getters.
    for body in crate::api::b3_world_dynamic_body_ids(id) {
        callback(body, context);
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_visit_joint_health(
    id: WorldId,
    callback: extern "C" fn(JointId, BodyId, BodyId, bool, f32, bool, f32),
) {
    for (joint, body_a, body_b, constrained, error, angular_constrained, angular_error) in crate::api::b3_world_joint_health(id) {
        callback(joint, body_a, body_b, constrained, error, angular_constrained, angular_error);
    }
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_set_fail(id: WorldId, message: *const std::os::raw::c_char) {
    let text = if message.is_null() {
        "unsupported scene"
    } else {
        unsafe { std::ffi::CStr::from_ptr(message) }.to_str().unwrap_or("unsupported scene")
    };
    b3_world_set_gpu_fail(id, text);
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_set_custom_filter_callback(
    id: WorldId,
    callback: Option<CustomFilterCallback>,
    context: *mut std::ffi::c_void,
) {
    b3_world_set_custom_filter_callback(id, callback, context);
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_set_pre_solve_callback(
    id: WorldId,
    callback: Option<PreSolveCallback>,
    context: *mut std::ffi::c_void,
) {
    b3_world_set_pre_solve_callback(id, callback, context);
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_enable_warm_starting(id: WorldId, enable: bool) {
    crate::api::b3_world_enable_warm_starting(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_is_warm_starting_enabled(id: WorldId) -> bool {
    crate::api::b3_world_is_warm_starting_enabled(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_enable_speculative(id: WorldId, enable: bool) {
    crate::api::b3_world_enable_speculative(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_enable_sleeping(id: WorldId, enable: bool) {
    b3_world_enable_sleeping(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_is_sleeping_enabled(id: WorldId) -> bool {
    b3_world_is_sleeping_enabled(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_enable_continuous(id: WorldId, enable: bool) {
    b3_world_enable_continuous(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_is_continuous_enabled(id: WorldId) -> bool {
    b3_world_is_continuous_enabled(id)
}

fn locks_from_bits(bits: u32) -> crate::api::MotionLocks {
    crate::api::MotionLocks {
        linear_x: bits & FLAG_LOCK_LIN_X != 0,
        linear_y: bits & FLAG_LOCK_LIN_Y != 0,
        linear_z: bits & FLAG_LOCK_LIN_Z != 0,
        angular_x: bits & FLAG_LOCK_ANG_X != 0,
        angular_y: bits & FLAG_LOCK_ANG_Y != 0,
        angular_z: bits & FLAG_LOCK_ANG_Z != 0,
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_create_body(
    world: WorldId,
    body_type: i32,
    px: f32,
    py: f32,
    pz: f32,
    rx: f32,
    ry: f32,
    rz: f32,
    rw: f32,
    vx: f32,
    vy: f32,
    vz: f32,
    wx: f32,
    wy: f32,
    wz: f32,
    gravity_scale: f32,
    locks: u32,
) -> BodyId {
    let mut def = b3_default_body_def();
    def.body_type = match body_type {
        2 => BodyType::Dynamic,
        1 => BodyType::Kinematic,
        _ => BodyType::Static,
    };
    def.position = [px, py, pz];
    def.rotation = [rx, ry, rz, rw];
    def.linear_velocity = [vx, vy, vz];
    def.angular_velocity = [wx, wy, wz];
    def.gravity_scale = gravity_scale;
    def.motion_locks = locks_from_bits(locks);
    def.enable_sleep = locks & 8 != 0;
    def.enable_contact_recycling = locks & crate::types::FLAG_DISABLE_CONTACT_RECYCLING == 0;
    def.is_awake = locks & 4 == 0;
    b3_create_body(world, &def)
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_enable_contact_recycling(id: BodyId, enable: bool) {
    crate::api::b3_body_enable_contact_recycling(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_is_contact_recycling_enabled(id: BodyId) -> bool {
    crate::api::b3_body_is_contact_recycling_enabled(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_set_bullet(id: BodyId, bullet: bool) {
    b3_body_set_bullet(id, bullet);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_is_bullet(id: BodyId) -> bool {
    b3_body_is_bullet(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_allow_fast_rotation(id: BodyId, allow: bool) {
    b3_body_allow_fast_rotation(id, allow);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_is_fast_rotation_allowed(id: BodyId) -> bool {
    b3_body_is_fast_rotation_allowed(id)
}

fn shape_def(
    density: f32,
    friction: f32,
    restitution: f32,
    rolling: f32,
    update_body_mass: bool,
) -> ShapeDef {
    ShapeDef {
        density,
        friction,
        restitution,
        rolling_resistance: rolling,
        explosion_scale: 1.0,
        filter: crate::api::b3_default_filter(),
        is_sensor: false,
        enable_sensor_events: false,
        enable_contact_events: false,
        enable_hit_events: false,
        enable_custom_filtering: false,
        enable_pre_solve_events: false,
        enable_speculative_contact: true,
        user_material_id: 0,
        user_data: 0,
        update_body_mass,
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_set_explosion_scale(id: ShapeId, scale: f32) {
    b3_shape_set_explosion_scale(id, scale);
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_explode(
    id: WorldId,
    mask_bits: u64,
    x: f32,
    y: f32,
    z: f32,
    radius: f32,
    falloff: f32,
    impulse_per_area: f32,
) {
    crate::api::b3_world_explode(
        id,
        &crate::api::ExplosionDef {
            mask_bits,
            position: [x, y, z],
            radius,
            falloff,
            impulse_per_area,
        },
    );
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_set_filter(
    id: ShapeId,
    category_bits: u64,
    mask_bits: u64,
    group_index: i32,
    invoke_contacts: bool,
) {
    b3_shape_set_filter(
        id,
        crate::api::Filter {
            category_bits,
            mask_bits,
            group_index,
        },
        invoke_contacts,
    );
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_shape_get_filter(
    id: ShapeId,
    category_bits: *mut u64,
    mask_bits: *mut u64,
    group_index: *mut i32,
) {
    let filter = b3_shape_get_filter(id);
    if let Some(value) = category_bits.as_mut() {
        *value = filter.category_bits;
    }
    if let Some(value) = mask_bits.as_mut() {
        *value = filter.mask_bits;
    }
    if let Some(value) = group_index.as_mut() {
        *value = filter.group_index;
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_enable_speculative_contact(id: ShapeId, enable: bool) {
    b3_shape_enable_speculative_contact(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_enable_contact_events(id: ShapeId, enable: bool) {
    b3_shape_enable_contact_events(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_are_contact_events_enabled(id: ShapeId) -> bool {
    b3_shape_are_contact_events_enabled(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_enable_custom_filtering(id: ShapeId, enable: bool) {
    b3_shape_enable_custom_filtering(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_is_custom_filtering_enabled(id: ShapeId) -> bool {
    b3_shape_is_custom_filtering_enabled(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_enable_pre_solve_events(id: ShapeId, enable: bool) {
    b3_shape_enable_pre_solve_events(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_are_pre_solve_events_enabled(id: ShapeId) -> bool {
    b3_shape_are_pre_solve_events_enabled(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_is_sensor(id: ShapeId) -> bool {
    b3_shape_is_sensor(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_enable_sensor_events(id: ShapeId, enable: bool) {
    b3_shape_enable_sensor_events(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_are_sensor_events_enabled(id: ShapeId) -> bool {
    b3_shape_are_sensor_events_enabled(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_set_sensor(id: ShapeId, is_sensor: bool) {
    b3_shape_set_sensor(id, is_sensor);
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_shape_get_sensor_data(
    id: ShapeId,
    output: *mut ShapeId,
    capacity: i32,
) -> i32 {
    if output.is_null() || capacity <= 0 {
        return 0;
    }
    let output = std::slice::from_raw_parts_mut(output, capacity as usize);
    b3_shape_get_sensor_data(id, output) as i32
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_get_sensor_capacity(id: ShapeId) -> i32 {
    b3_shape_get_sensor_capacity(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_destroy_shape(id: ShapeId, update_body_mass: bool) {
    b3_destroy_shape(id, update_body_mass);
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_get_contact_events(
    id: WorldId,
    begin_events: *mut *const crate::api::ContactBeginTouchEvent,
    begin_count: *mut i32,
    end_events: *mut *const crate::api::ContactEndTouchEvent,
    end_count: *mut i32,
    hit_events: *mut *const crate::api::ContactHitEvent,
    hit_count: *mut i32,
) {
    let (begin_ptr, begin_len, end_ptr, end_len, hit_ptr, hit_len) =
        b3_world_contact_event_ptrs(id);
    if let Some(value) = begin_events.as_mut() {
        *value = begin_ptr;
    }
    if let Some(value) = begin_count.as_mut() {
        *value = begin_len;
    }
    if let Some(value) = end_events.as_mut() {
        *value = end_ptr;
    }
    if let Some(value) = end_count.as_mut() {
        *value = end_len;
    }
    if let Some(value) = hit_events.as_mut() {
        *value = hit_ptr;
    }
    if let Some(value) = hit_count.as_mut() {
        *value = hit_len;
    }
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_get_body_events(
    id: WorldId,
    events: *mut *const crate::api::BodyMoveEvent,
    count: *mut i32,
) {
    let (event_ptr, event_count) = b3_world_body_event_ptrs(id);
    if let Some(value) = events.as_mut() {
        *value = event_ptr;
    }
    if let Some(value) = count.as_mut() {
        *value = event_count;
    }
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_get_joint_events(
    id: WorldId,
    events: *mut *const crate::api::JointEvent,
    count: *mut i32,
) {
    let (event_ptr, event_count) = b3_world_joint_event_ptrs(id);
    if let Some(value) = events.as_mut() {
        *value = event_ptr;
    }
    if let Some(value) = count.as_mut() {
        *value = event_count;
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_set_hit_event_threshold(id: WorldId, value: f32) {
    b3_world_set_hit_event_threshold(id, value);
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_get_hit_event_threshold(id: WorldId) -> f32 {
    b3_world_get_hit_event_threshold(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_set_friction_callback(
    id: WorldId,
    callback: Option<FrictionCallback>,
) {
    b3_world_set_friction_callback(id, callback);
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_set_restitution_callback(
    id: WorldId,
    callback: Option<RestitutionCallback>,
) {
    b3_world_set_restitution_callback(id, callback);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_set_user_data(id: BodyId, user_data: usize) {
    b3_body_set_user_data(id, user_data);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_user_data(id: BodyId) -> usize {
    b3_body_get_user_data(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_joint_set_force_threshold(id: JointId, threshold: f32) {
    b3_joint_set_force_threshold(id, threshold);
}

#[no_mangle]
pub extern "C" fn gpu_b3_joint_get_force_threshold(id: JointId) -> f32 {
    b3_joint_get_force_threshold(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_joint_set_torque_threshold(id: JointId, threshold: f32) {
    b3_joint_set_torque_threshold(id, threshold);
}

#[no_mangle]
pub extern "C" fn gpu_b3_joint_get_torque_threshold(id: JointId) -> f32 {
    b3_joint_get_torque_threshold(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_joint_set_user_data(id: JointId, user_data: usize) {
    b3_joint_set_user_data(id, user_data);
}

#[no_mangle]
pub extern "C" fn gpu_b3_joint_get_user_data(id: JointId) -> usize {
    b3_joint_get_user_data(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_enable_hit_events(id: ShapeId, enable: bool) {
    b3_shape_enable_hit_events(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_are_hit_events_enabled(id: ShapeId) -> bool {
    b3_shape_are_hit_events_enabled(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_set_user_material_id(id: ShapeId, value: u64) {
    b3_shape_set_user_material_id(id, value);
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_set_surface_material(id: ShapeId, material: SurfaceMaterial) {
    b3_shape_set_surface_material(id, material);
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_get_surface_material(id: ShapeId) -> SurfaceMaterial {
    b3_shape_get_surface_material(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_set_mesh_material_count(id: ShapeId, count: i32) {
    if count > 0 {
        b3_shape_set_mesh_material_count(id, count as usize);
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_set_mesh_material(
    id: ShapeId,
    index: i32,
    material: SurfaceMaterial,
) {
    if index >= 0 {
        b3_shape_set_mesh_material(id, index as usize, material);
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_get_mesh_material(id: ShapeId, index: i32) -> SurfaceMaterial {
    if index < 0 {
        crate::api::b3_default_surface_material()
    } else {
        b3_shape_get_mesh_material(id, index as usize)
    }
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_get_sensor_events(
    id: WorldId,
    begin_events: *mut *const crate::api::SensorBeginTouchEvent,
    begin_count: *mut i32,
    end_events: *mut *const crate::api::SensorEndTouchEvent,
    end_count: *mut i32,
) {
    let (begin_ptr, begin_len, end_ptr, end_len) = b3_world_sensor_event_ptrs(id);
    if let Some(value) = begin_events.as_mut() {
        *value = begin_ptr;
    }
    if let Some(value) = begin_count.as_mut() {
        *value = begin_len;
    }
    if let Some(value) = end_events.as_mut() {
        *value = end_ptr;
    }
    if let Some(value) = end_count.as_mut() {
        *value = end_len;
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_create_sphere(
    body: BodyId,
    center_x: f32,
    center_y: f32,
    center_z: f32,
    radius: f32,
    density: f32,
    friction: f32,
    restitution: f32,
    rolling: f32,
    update_body_mass: bool,
) -> ShapeId {
    b3_create_sphere_shape(
        body,
        &shape_def(density, friction, restitution, rolling, update_body_mass),
        &Sphere {
            center: [center_x, center_y, center_z],
            radius,
        },
    )
}

#[no_mangle]
pub extern "C" fn gpu_b3_create_hull(
    body: BodyId,
    hx: f32,
    hy: f32,
    hz: f32,
    ox: f32,
    oy: f32,
    oz: f32,
    density: f32,
    friction: f32,
    restitution: f32,
    rolling: f32,
    update_body_mass: bool,
) -> ShapeId {
    b3_create_hull_shape(
        body,
        &shape_def(density, friction, restitution, rolling, update_body_mass),
        &BoxHull {
            half_extents: [hx, hy, hz],
            center: [ox, oy, oz],
        },
    )
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_create_mesh(
    body: BodyId,
    vertex_data: *const f32,
    vertex_count: i32,
    triangle_data: *const i32,
    triangle_count: i32,
    flags: *const u8,
    materials: *const u8,
    node_data: *const u8,
    node_count: i32,
    scale_x: f32,
    scale_y: f32,
    scale_z: f32,
    density: f32,
    friction: f32,
    restitution: f32,
    rolling: f32,
    update_body_mass: bool,
) -> ShapeId {
    if vertex_data.is_null()
        || triangle_data.is_null()
        || vertex_count <= 0
        || triangle_count <= 0
        || (node_count > 0 && node_data.is_null())
    {
        return crate::api::b3_null_shape_id();
    }
    let vertex_values =
        unsafe { std::slice::from_raw_parts(vertex_data, vertex_count as usize * 3) };
    let vertices: Vec<[f32; 3]> = vertex_values
        .chunks_exact(3)
        .map(|point| [point[0], point[1], point[2]])
        .collect();
    let triangle_values =
        unsafe { std::slice::from_raw_parts(triangle_data, triangle_count as usize * 3) };
    let triangles: Vec<[u32; 3]> = triangle_values
        .chunks_exact(3)
        .map(|triangle| [triangle[0] as u32, triangle[1] as u32, triangle[2] as u32])
        .collect();
    let flags = if flags.is_null() {
        &[][..]
    } else {
        unsafe { std::slice::from_raw_parts(flags, triangle_count as usize) }
    };
    let materials = if materials.is_null() {
        &[][..]
    } else {
        unsafe { std::slice::from_raw_parts(materials, triangle_count as usize) }
    };
    let mut nodes = Vec::with_capacity(node_count.max(0) as usize);
    if !node_data.is_null() {
        for index in 0..node_count.max(0) as usize {
            let ptr = unsafe { node_data.add(index * 32) };
            let read_f32 = |offset| unsafe { (ptr.add(offset) as *const f32).read_unaligned() };
            let read_u32 = |offset| unsafe { (ptr.add(offset) as *const u32).read_unaligned() };
            nodes.push(MeshNode {
                lower: [read_f32(0), read_f32(4), read_f32(8)],
                data: read_u32(12),
                upper: [read_f32(16), read_f32(20), read_f32(24)],
                triangle_offset: read_u32(28),
            });
        }
    }
    b3_create_mesh_shape(
        body,
        &shape_def(density, friction, restitution, rolling, update_body_mass),
        &vertices,
        &triangles,
        flags,
        materials,
        &nodes,
        [scale_x, scale_y, scale_z],
    )
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_create_mesh_instance(
    body: BodyId, source: ShapeId, position: *const f32,
    rotation: *const f32, scale: *const f32,
    density: f32, friction: f32, restitution: f32, rolling: f32,
) -> ShapeId {
    if position.is_null() || rotation.is_null() || scale.is_null() {
        return crate::api::b3_null_shape_id();
    }
    crate::api::b3_create_mesh_instance(body,
        &shape_def(density, friction, restitution, rolling, false), source,
        crate::api::WorldTransform { p: unsafe { position.cast::<[f32; 3]>().read_unaligned() },
                         q: unsafe { rotation.cast::<[f32; 4]>().read_unaligned() } },
        unsafe { scale.cast::<[f32; 3]>().read_unaligned() })
}

#[no_mangle]
pub extern "C" fn gpu_b3_create_compound_parent(
    body: BodyId,
    density: f32,
    friction: f32,
    restitution: f32,
    rolling: f32,
    is_sensor: bool,
) -> ShapeId {
    let mut def = shape_def(density, friction, restitution, rolling, false);
    def.is_sensor = is_sensor;
    b3_create_compound_parent(body, &def)
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_shape_set_compound_materials(
    parent: ShapeId, materials: *const SurfaceMaterial, count: i32,
) -> bool {
    if materials.is_null() || count <= 0 { return false; }
    crate::api::b3_shape_set_compound_materials(parent,
        unsafe { std::slice::from_raw_parts(materials, count as usize) })
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_get_material_count(id: ShapeId) -> i32 {
    i32::try_from(crate::api::b3_shape_get_material_count(id)).unwrap_or(0)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_set_compound_local_bounds(parent: ShapeId, bounds: crate::api::Aabb) -> bool {
    crate::api::b3_shape_set_compound_local_bounds(parent, bounds)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_attach_compound_child(parent: ShapeId, child: ShapeId, index: i32) -> bool {
    b3_shape_attach_compound_child_indexed(parent, child, index)
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_shape_attach_compound_child_materials(
    parent: ShapeId,
    child: ShapeId,
    index: i32,
    materials: *const i32,
) -> bool {
    if materials.is_null() {
        return false;
    }
    let map = std::slice::from_raw_parts(materials, 4);
    crate::api::b3_shape_attach_compound_child_materials(
        parent,
        child,
        index,
        [map[0], map[1], map[2], map[3]],
    )
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_create_height_field(
    body: BodyId,
    compressed_heights: *const u16,
    column_count: i32,
    row_count: i32,
    min_height: f32,
    height_scale: f32,
    scale_x: f32,
    scale_y: f32,
    scale_z: f32,
    cell_materials: *const u8,
    triangle_flags: *const u8,
    clockwise: bool,
    density: f32,
    friction: f32,
    restitution: f32,
    rolling: f32,
) -> ShapeId {
    let Ok(columns) = usize::try_from(column_count) else {
        return crate::api::b3_null_shape_id();
    };
    let Ok(rows) = usize::try_from(row_count) else {
        return crate::api::b3_null_shape_id();
    };
    let Some(point_count) = columns.checked_mul(rows) else {
        return crate::api::b3_null_shape_id();
    };
    let Some(cell_count) = columns
        .checked_sub(1)
        .and_then(|value| rows.checked_sub(1).and_then(|rows| value.checked_mul(rows)))
    else {
        return crate::api::b3_null_shape_id();
    };
    let Some(triangle_count) = cell_count.checked_mul(2) else {
        return crate::api::b3_null_shape_id();
    };
    if compressed_heights.is_null()
        || cell_materials.is_null()
        || triangle_flags.is_null()
        || point_count == 0
    {
        return crate::api::b3_null_shape_id();
    }
    let heights = unsafe { std::slice::from_raw_parts(compressed_heights, point_count) };
    let materials = unsafe { std::slice::from_raw_parts(cell_materials, cell_count) };
    let flags = unsafe { std::slice::from_raw_parts(triangle_flags, triangle_count) };
    b3_create_height_field_shape(
        body,
        &shape_def(density, friction, restitution, rolling, false),
        heights,
        columns,
        rows,
        min_height,
        height_scale,
        [scale_x, scale_y, scale_z],
        materials,
        flags,
        clockwise,
    )
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_replace_height_field(
    id: ShapeId,
    compressed_heights: *const u16,
    column_count: i32,
    row_count: i32,
    min_height: f32,
    height_scale: f32,
    scale_x: f32,
    scale_y: f32,
    scale_z: f32,
    cell_materials: *const u8,
    triangle_flags: *const u8,
    clockwise: bool,
) -> bool {
    let (Ok(columns), Ok(rows)) = (usize::try_from(column_count), usize::try_from(row_count))
    else {
        return false;
    };
    let Some(point_count) = columns.checked_mul(rows) else {
        return false;
    };
    let Some(cell_count) = columns
        .checked_sub(1)
        .and_then(|value| rows.checked_sub(1).and_then(|rows| value.checked_mul(rows)))
    else {
        return false;
    };
    let Some(triangle_count) = cell_count.checked_mul(2) else {
        return false;
    };
    if compressed_heights.is_null() || cell_materials.is_null() || triangle_flags.is_null() {
        return false;
    }
    crate::api::b3_replace_height_field_shape(
        id,
        unsafe { std::slice::from_raw_parts(compressed_heights, point_count) },
        columns,
        rows,
        min_height,
        height_scale,
        [scale_x, scale_y, scale_z],
        unsafe { std::slice::from_raw_parts(cell_materials, cell_count) },
        unsafe { std::slice::from_raw_parts(triangle_flags, triangle_count) },
        clockwise,
    )
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_replace_mesh(
    id: ShapeId,
    vertex_data: *const f32,
    vertex_count: i32,
    triangle_data: *const i32,
    triangle_count: i32,
    flags: *const u8,
    materials: *const u8,
    node_data: *const u8,
    node_count: i32,
    scale_x: f32,
    scale_y: f32,
    scale_z: f32,
) -> bool {
    if vertex_data.is_null()
        || triangle_data.is_null()
        || node_data.is_null()
        || vertex_count <= 0
        || triangle_count <= 0
        || node_count <= 0
    {
        return false;
    }
    let values = unsafe { std::slice::from_raw_parts(vertex_data, vertex_count as usize * 3) };
    let vertices: Vec<[f32; 3]> = values.chunks_exact(3).map(|p| [p[0], p[1], p[2]]).collect();
    let values = unsafe { std::slice::from_raw_parts(triangle_data, triangle_count as usize * 3) };
    let triangles: Vec<[u32; 3]> = values
        .chunks_exact(3)
        .map(|t| [t[0] as u32, t[1] as u32, t[2] as u32])
        .collect();
    let flags = if flags.is_null() {
        &[][..]
    } else {
        unsafe { std::slice::from_raw_parts(flags, triangle_count as usize) }
    };
    let materials = if materials.is_null() {
        &[][..]
    } else {
        unsafe { std::slice::from_raw_parts(materials, triangle_count as usize) }
    };
    let mut nodes = Vec::with_capacity(node_count as usize);
    for index in 0..node_count as usize {
        let ptr = unsafe { node_data.add(index * 32) };
        let read_f32 = |offset| unsafe { (ptr.add(offset) as *const f32).read_unaligned() };
        let read_u32 = |offset| unsafe { (ptr.add(offset) as *const u32).read_unaligned() };
        nodes.push(MeshNode {
            lower: [read_f32(0), read_f32(4), read_f32(8)],
            data: read_u32(12),
            upper: [read_f32(16), read_f32(20), read_f32(24)],
            triangle_offset: read_u32(28),
        });
    }
    crate::api::b3_replace_mesh_shape(
        id,
        &vertices,
        &triangles,
        flags,
        materials,
        &nodes,
        [scale_x, scale_y, scale_z],
    )
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_create_convex_hull(
    body: BodyId,
    point_data: *const f32,
    point_count: i32,
    plane_data: *const f32,
    plane_count: i32,
    edge_data: *const u8,
    half_edge_count: i32,
    hx: f32,
    hy: f32,
    hz: f32,
    ax: f32,
    ay: f32,
    az: f32,
    cx: f32,
    cy: f32,
    cz: f32,
    inner_radius: f32,
    volume: f32,
    ixx: f32,
    iyy: f32,
    izz: f32,
    ixy: f32,
    ixz: f32,
    iyz: f32,
    density: f32,
    friction: f32,
    restitution: f32,
    rolling: f32,
    update_body_mass: bool,
) -> ShapeId {
    let Some(hull) = (unsafe {
        read_convex_hull(
            point_data,
            point_count,
            plane_data,
            plane_count,
            edge_data,
            half_edge_count,
            hx,
            hy,
            hz,
            ax,
            ay,
            az,
            cx,
            cy,
            cz,
            inner_radius,
            volume,
            ixx,
            iyy,
            izz,
            ixy,
            ixz,
            iyz,
        )
    }) else {
        return crate::api::b3_null_shape_id();
    };
    b3_create_convex_hull_shape(
        body,
        &shape_def(density, friction, restitution, rolling, update_body_mass),
        &hull,
    )
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_set_convex_hull(
    id: ShapeId,
    point_data: *const f32,
    point_count: i32,
    plane_data: *const f32,
    plane_count: i32,
    edge_data: *const u8,
    half_edge_count: i32,
    hx: f32,
    hy: f32,
    hz: f32,
    ax: f32,
    ay: f32,
    az: f32,
    cx: f32,
    cy: f32,
    cz: f32,
    inner_radius: f32,
    volume: f32,
    ixx: f32,
    iyy: f32,
    izz: f32,
    ixy: f32,
    ixz: f32,
    iyz: f32,
) -> bool {
    let Some(hull) = (unsafe {
        read_convex_hull(
            point_data,
            point_count,
            plane_data,
            plane_count,
            edge_data,
            half_edge_count,
            hx,
            hy,
            hz,
            ax,
            ay,
            az,
            cx,
            cy,
            cz,
            inner_radius,
            volume,
            ixx,
            iyy,
            izz,
            ixy,
            ixz,
            iyz,
        )
    }) else {
        return false;
    };
    crate::api::set_convex_hull_geometry(id, &hull, true)
}
unsafe fn read_convex_hull(
    point_data: *const f32,
    point_count: i32,
    plane_data: *const f32,
    plane_count: i32,
    edge_data: *const u8,
    half_edge_count: i32,
    hx: f32,
    hy: f32,
    hz: f32,
    ax: f32,
    ay: f32,
    az: f32,
    cx: f32,
    cy: f32,
    cz: f32,
    inner_radius: f32,
    volume: f32,
    ixx: f32,
    iyy: f32,
    izz: f32,
    ixy: f32,
    ixz: f32,
    iyz: f32,
) -> Option<ConvexHull> {
    if point_data.is_null() || point_count < 4 || plane_data.is_null() || plane_count < 4 {
        return None;
    }
    let values = unsafe { std::slice::from_raw_parts(point_data, point_count as usize * 3) };
    let points: Vec<[f32; 3]> = values
        .chunks_exact(3)
        .map(|point| [point[0], point[1], point[2]])
        .collect();
    let plane_values = unsafe { std::slice::from_raw_parts(plane_data, plane_count as usize * 4) };
    let planes: Vec<[f32; 4]> = plane_values
        .chunks_exact(4)
        .map(|plane| [plane[0], plane[1], plane[2], plane[3]])
        .collect();
    let mut edge_directions = Vec::new();
    let mut half_edges = Vec::new();
    if !edge_data.is_null() && half_edge_count > 0 {
        let edges = unsafe { std::slice::from_raw_parts(edge_data, half_edge_count as usize * 4) };
        for i in 0..half_edge_count as usize {
            let edge = &edges[4 * i..4 * i + 4];
            half_edges.push([
                edge[0] as u32,
                edge[1] as u32,
                edge[2] as u32,
                edge[3] as u32,
            ]);
            let twin = edge[1] as usize;
            if i >= twin || twin >= half_edge_count as usize {
                continue;
            }
            let a = edge[2] as usize;
            let b = edges[4 * twin + 2] as usize;
            let (Some(a), Some(b)) = (points.get(a), points.get(b)) else {
                continue;
            };
            let mut direction = [b[0] - a[0], b[1] - a[1], b[2] - a[2]];
            let length = (direction[0] * direction[0]
                + direction[1] * direction[1]
                + direction[2] * direction[2])
                .sqrt();
            if length <= 1e-8 {
                continue;
            }
            for component in &mut direction {
                *component /= length;
            }
            if edge_directions.iter().any(|old: &[f32; 3]| {
                (old[0] * direction[0] + old[1] * direction[1] + old[2] * direction[2]).abs()
                    > 0.9999
            }) {
                continue;
            }
            edge_directions.push(direction);
        }
    }
    Some(ConvexHull {
        points,
        planes,
        edge_directions,
        half_edges,
        half_extents: [hx, hy, hz],
        aabb_center: [ax, ay, az],
        center: [cx, cy, cz],
        inner_radius,
        volume,
        central_inertia: [ixx, iyy, izz, ixy, ixz, iyz],
    })
}

#[no_mangle]
pub extern "C" fn gpu_b3_create_capsule(
    body: BodyId,
    x1: f32,
    y1: f32,
    z1: f32,
    x2: f32,
    y2: f32,
    z2: f32,
    radius: f32,
    density: f32,
    friction: f32,
    restitution: f32,
    rolling: f32,
    update_body_mass: bool,
) -> ShapeId {
    b3_create_capsule_shape(
        body,
        &shape_def(density, friction, restitution, rolling, update_body_mass),
        &Capsule {
            center1: [x1, y1, z1],
            center2: [x2, y2, z2],
            radius,
        },
    )
}

#[no_mangle]
pub extern "C" fn gpu_b3_create_revolute(
    world: WorldId,
    a: BodyId,
    b: BodyId,
    ax: f32,
    ay: f32,
    az: f32,
    bx: f32,
    by: f32,
    bz: f32,
    qa_x: f32,
    qa_y: f32,
    qa_z: f32,
    qa_w: f32,
    qb_x: f32,
    qb_y: f32,
    qb_z: f32,
    qb_w: f32,
    hertz: f32,
    damping: f32,
    collide_connected: bool,
    target_angle: f32,
    enable_spring: bool,
    spring_hertz: f32,
    spring_damping: f32,
    enable_limit: bool,
    lower_angle: f32,
    upper_angle: f32,
    enable_motor: bool,
    max_motor_torque: f32,
    motor_speed: f32,
) -> JointId {
    let mut def = b3_default_revolute_joint_def();
    def.body_a = a;
    def.body_b = b;
    def.local_anchor_a = [ax, ay, az];
    def.local_anchor_b = [bx, by, bz];
    def.local_rotation_a = [qa_x, qa_y, qa_z, qa_w];
    def.local_rotation_b = [qb_x, qb_y, qb_z, qb_w];
    def.hertz = hertz;
    def.damping = damping;
    def.collide_connected = collide_connected;
    def.target_angle = target_angle;
    def.enable_spring = enable_spring;
    def.spring_hertz = spring_hertz;
    def.spring_damping = spring_damping;
    def.enable_limit = enable_limit;
    def.lower_angle = lower_angle;
    def.upper_angle = upper_angle;
    def.enable_motor = enable_motor;
    def.max_motor_torque = max_motor_torque;
    def.motor_speed = motor_speed;
    b3_create_revolute_joint(world, &def)
}

#[no_mangle]
pub extern "C" fn gpu_b3_joint_set_constraint_tuning(id: JointId, hertz: f32, damping: f32) {
    crate::api::b3_joint_set_constraint_tuning(id, hertz, damping);
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_joint_get_constraint_tuning(
    id: JointId,
    hertz: *mut f32,
    damping: *mut f32,
) {
    let value = crate::api::b3_joint_get_constraint_tuning(id);
    if !hertz.is_null() {
        *hertz = value[0];
    }
    if !damping.is_null() {
        *damping = value[1];
    }
}

macro_rules! revolute_bool_abi {
    ($set_name:ident, $get_name:ident, $set:path, $get:path) => {
        #[no_mangle]
        pub extern "C" fn $set_name(id: JointId, value: bool) {
            $set(id, value);
        }

        #[no_mangle]
        pub extern "C" fn $get_name(id: JointId) -> bool {
            $get(id)
        }
    };
}

macro_rules! revolute_scalar_abi {
    ($set_name:ident, $get_name:ident, $set:path, $get:path) => {
        #[no_mangle]
        pub extern "C" fn $set_name(id: JointId, value: f32) {
            $set(id, value);
        }

        #[no_mangle]
        pub extern "C" fn $get_name(id: JointId) -> f32 {
            $get(id)
        }
    };
}

revolute_bool_abi!(
    gpu_b3_revolute_enable_spring,
    gpu_b3_revolute_is_spring_enabled,
    crate::api::b3_revolute_joint_enable_spring,
    crate::api::b3_revolute_joint_is_spring_enabled
);
revolute_scalar_abi!(
    gpu_b3_revolute_set_spring_hertz,
    gpu_b3_revolute_get_spring_hertz,
    crate::api::b3_revolute_joint_set_spring_hertz,
    crate::api::b3_revolute_joint_get_spring_hertz
);
revolute_scalar_abi!(
    gpu_b3_revolute_set_spring_damping,
    gpu_b3_revolute_get_spring_damping,
    crate::api::b3_revolute_joint_set_spring_damping,
    crate::api::b3_revolute_joint_get_spring_damping
);
revolute_scalar_abi!(
    gpu_b3_revolute_set_target_angle,
    gpu_b3_revolute_get_target_angle,
    crate::api::b3_revolute_joint_set_target_angle,
    crate::api::b3_revolute_joint_get_target_angle
);
revolute_bool_abi!(
    gpu_b3_revolute_enable_limit,
    gpu_b3_revolute_is_limit_enabled,
    crate::api::b3_revolute_joint_enable_limit,
    crate::api::b3_revolute_joint_is_limit_enabled
);
revolute_bool_abi!(
    gpu_b3_revolute_enable_motor,
    gpu_b3_revolute_is_motor_enabled,
    crate::api::b3_revolute_joint_enable_motor,
    crate::api::b3_revolute_joint_is_motor_enabled
);
revolute_scalar_abi!(
    gpu_b3_revolute_set_motor_speed,
    gpu_b3_revolute_get_motor_speed,
    crate::api::b3_revolute_joint_set_motor_speed,
    crate::api::b3_revolute_joint_get_motor_speed
);
revolute_scalar_abi!(
    gpu_b3_revolute_set_max_motor_torque,
    gpu_b3_revolute_get_max_motor_torque,
    crate::api::b3_revolute_joint_set_max_motor_torque,
    crate::api::b3_revolute_joint_get_max_motor_torque
);

#[no_mangle]
pub extern "C" fn gpu_b3_revolute_get_angle(id: JointId) -> f32 {
    crate::api::b3_revolute_joint_get_angle(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_revolute_get_motor_torque(id: JointId) -> f32 {
    crate::api::b3_revolute_joint_get_motor_torque(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_revolute_get_lower_limit(id: JointId) -> f32 {
    crate::api::b3_revolute_joint_get_lower_limit(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_revolute_get_upper_limit(id: JointId) -> f32 {
    crate::api::b3_revolute_joint_get_upper_limit(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_revolute_set_limits(id: JointId, lower: f32, upper: f32) {
    crate::api::b3_revolute_joint_set_limits(id, lower, upper);
}

#[no_mangle]
pub extern "C" fn gpu_b3_create_wheel(
    world: WorldId,
    a: BodyId,
    b: BodyId,
    ax: f32,
    ay: f32,
    az: f32,
    bx: f32,
    by: f32,
    bz: f32,
    qa_x: f32,
    qa_y: f32,
    qa_z: f32,
    qa_w: f32,
    qb_x: f32,
    qb_y: f32,
    qb_z: f32,
    qb_w: f32,
    hertz: f32,
    damping: f32,
    collide_connected: bool,
    enable_suspension: bool,
    suspension_hertz: f32,
    suspension_damping: f32,
    enable_suspension_limit: bool,
    lower_suspension: f32,
    upper_suspension: f32,
    enable_spin_motor: bool,
    max_spin_torque: f32,
    spin_speed: f32,
    enable_steering: bool,
    steering_hertz: f32,
    steering_damping: f32,
    target_steering: f32,
    max_steering_torque: f32,
    enable_steering_limit: bool,
    lower_steering: f32,
    upper_steering: f32,
) -> JointId {
    let mut def = crate::api::b3_default_wheel_joint_def();
    def.body_a = a;
    def.body_b = b;
    def.local_anchor_a = [ax, ay, az];
    def.local_anchor_b = [bx, by, bz];
    def.local_rotation_a = [qa_x, qa_y, qa_z, qa_w];
    def.local_rotation_b = [qb_x, qb_y, qb_z, qb_w];
    def.hertz = hertz;
    def.damping = damping;
    def.collide_connected = collide_connected;
    def.enable_suspension_spring = enable_suspension;
    def.suspension_hertz = suspension_hertz;
    def.suspension_damping_ratio = suspension_damping;
    def.enable_suspension_limit = enable_suspension_limit;
    def.lower_suspension_limit = lower_suspension;
    def.upper_suspension_limit = upper_suspension;
    def.enable_spin_motor = enable_spin_motor;
    def.max_spin_torque = max_spin_torque;
    def.spin_speed = spin_speed;
    def.enable_steering = enable_steering;
    def.steering_hertz = steering_hertz;
    def.steering_damping_ratio = steering_damping;
    def.target_steering_angle = target_steering;
    def.max_steering_torque = max_steering_torque;
    def.enable_steering_limit = enable_steering_limit;
    def.lower_steering_limit = lower_steering;
    def.upper_steering_limit = upper_steering;
    crate::api::b3_create_wheel_joint(world, &def)
}

revolute_bool_abi!(
    gpu_b3_wheel_enable_suspension,
    gpu_b3_wheel_is_suspension_enabled,
    crate::api::b3_wheel_joint_enable_suspension,
    crate::api::b3_wheel_joint_is_suspension_enabled
);
revolute_scalar_abi!(
    gpu_b3_wheel_set_suspension_hertz,
    gpu_b3_wheel_get_suspension_hertz,
    crate::api::b3_wheel_joint_set_suspension_hertz,
    crate::api::b3_wheel_joint_get_suspension_hertz
);
revolute_scalar_abi!(
    gpu_b3_wheel_set_suspension_damping,
    gpu_b3_wheel_get_suspension_damping,
    crate::api::b3_wheel_joint_set_suspension_damping_ratio,
    crate::api::b3_wheel_joint_get_suspension_damping_ratio
);
revolute_bool_abi!(
    gpu_b3_wheel_enable_suspension_limit,
    gpu_b3_wheel_is_suspension_limit_enabled,
    crate::api::b3_wheel_joint_enable_suspension_limit,
    crate::api::b3_wheel_joint_is_suspension_limit_enabled
);
revolute_bool_abi!(
    gpu_b3_wheel_enable_spin_motor,
    gpu_b3_wheel_is_spin_motor_enabled,
    crate::api::b3_wheel_joint_enable_spin_motor,
    crate::api::b3_wheel_joint_is_spin_motor_enabled
);
revolute_scalar_abi!(
    gpu_b3_wheel_set_spin_speed,
    gpu_b3_wheel_get_spin_speed_setting,
    crate::api::b3_wheel_joint_set_spin_motor_speed,
    crate::api::b3_wheel_joint_get_spin_motor_speed
);
revolute_scalar_abi!(
    gpu_b3_wheel_set_max_spin_torque,
    gpu_b3_wheel_get_max_spin_torque,
    crate::api::b3_wheel_joint_set_max_spin_torque,
    crate::api::b3_wheel_joint_get_max_spin_torque
);
revolute_bool_abi!(
    gpu_b3_wheel_enable_steering,
    gpu_b3_wheel_is_steering_enabled,
    crate::api::b3_wheel_joint_enable_steering,
    crate::api::b3_wheel_joint_is_steering_enabled
);
revolute_scalar_abi!(
    gpu_b3_wheel_set_steering_hertz,
    gpu_b3_wheel_get_steering_hertz,
    crate::api::b3_wheel_joint_set_steering_hertz,
    crate::api::b3_wheel_joint_get_steering_hertz
);
revolute_scalar_abi!(
    gpu_b3_wheel_set_steering_damping,
    gpu_b3_wheel_get_steering_damping,
    crate::api::b3_wheel_joint_set_steering_damping_ratio,
    crate::api::b3_wheel_joint_get_steering_damping_ratio
);
revolute_scalar_abi!(
    gpu_b3_wheel_set_max_steering_torque,
    gpu_b3_wheel_get_max_steering_torque,
    crate::api::b3_wheel_joint_set_max_steering_torque,
    crate::api::b3_wheel_joint_get_max_steering_torque
);
revolute_bool_abi!(
    gpu_b3_wheel_enable_steering_limit,
    gpu_b3_wheel_is_steering_limit_enabled,
    crate::api::b3_wheel_joint_enable_steering_limit,
    crate::api::b3_wheel_joint_is_steering_limit_enabled
);
revolute_scalar_abi!(
    gpu_b3_wheel_set_target_steering,
    gpu_b3_wheel_get_target_steering,
    crate::api::b3_wheel_joint_set_target_steering_angle,
    crate::api::b3_wheel_joint_get_target_steering_angle
);

#[no_mangle]
pub extern "C" fn gpu_b3_wheel_set_suspension_limits(id: JointId, lower: f32, upper: f32) {
    crate::api::b3_wheel_joint_set_suspension_limits(id, lower, upper);
}
#[no_mangle]
pub extern "C" fn gpu_b3_wheel_get_lower_suspension_limit(id: JointId) -> f32 {
    crate::api::b3_wheel_joint_get_lower_suspension_limit(id)
}
#[no_mangle]
pub extern "C" fn gpu_b3_wheel_get_upper_suspension_limit(id: JointId) -> f32 {
    crate::api::b3_wheel_joint_get_upper_suspension_limit(id)
}
#[no_mangle]
pub extern "C" fn gpu_b3_wheel_set_steering_limits(id: JointId, lower: f32, upper: f32) {
    crate::api::b3_wheel_joint_set_steering_limits(id, lower, upper);
}
#[no_mangle]
pub extern "C" fn gpu_b3_wheel_get_lower_steering_limit(id: JointId) -> f32 {
    crate::api::b3_wheel_joint_get_lower_steering_limit(id)
}
#[no_mangle]
pub extern "C" fn gpu_b3_wheel_get_upper_steering_limit(id: JointId) -> f32 {
    crate::api::b3_wheel_joint_get_upper_steering_limit(id)
}
#[no_mangle]
pub extern "C" fn gpu_b3_wheel_get_live_spin_speed(id: JointId) -> f32 {
    crate::api::b3_wheel_joint_get_spin_speed(id)
}
#[no_mangle]
pub extern "C" fn gpu_b3_wheel_get_spin_torque(id: JointId) -> f32 {
    crate::api::b3_wheel_joint_get_spin_torque(id)
}
#[no_mangle]
pub extern "C" fn gpu_b3_wheel_get_steering_angle(id: JointId) -> f32 {
    crate::api::b3_wheel_joint_get_steering_angle(id)
}
#[no_mangle]
pub extern "C" fn gpu_b3_wheel_get_steering_torque(id: JointId) -> f32 {
    crate::api::b3_wheel_joint_get_steering_torque(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_create_spherical(
    world: WorldId,
    a: BodyId,
    b: BodyId,
    ax: f32,
    ay: f32,
    az: f32,
    bx: f32,
    by: f32,
    bz: f32,
    qa_x: f32,
    qa_y: f32,
    qa_z: f32,
    qa_w: f32,
    qb_x: f32,
    qb_y: f32,
    qb_z: f32,
    qb_w: f32,
    hertz: f32,
    damping: f32,
    collide_connected: bool,
    enable_spring: bool,
    spring_hertz: f32,
    spring_damping: f32,
    target_x: f32,
    target_y: f32,
    target_z: f32,
    target_w: f32,
    enable_cone_limit: bool,
    cone_angle: f32,
    enable_twist_limit: bool,
    lower_twist_angle: f32,
    upper_twist_angle: f32,
    enable_motor: bool,
    max_motor_torque: f32,
    motor_x: f32,
    motor_y: f32,
    motor_z: f32,
) -> JointId {
    let mut def = b3_default_spherical_joint_def();
    def.body_a = a;
    def.body_b = b;
    def.local_anchor_a = [ax, ay, az];
    def.local_anchor_b = [bx, by, bz];
    def.local_rotation_a = [qa_x, qa_y, qa_z, qa_w];
    def.local_rotation_b = [qb_x, qb_y, qb_z, qb_w];
    def.hertz = hertz;
    def.damping = damping;
    def.collide_connected = collide_connected;
    def.enable_spring = enable_spring;
    def.spring_hertz = spring_hertz;
    def.spring_damping = spring_damping;
    def.target_rotation = [target_x, target_y, target_z, target_w];
    def.enable_cone_limit = enable_cone_limit;
    def.cone_angle = cone_angle;
    def.enable_twist_limit = enable_twist_limit;
    def.lower_twist_angle = lower_twist_angle;
    def.upper_twist_angle = upper_twist_angle;
    def.enable_motor = enable_motor;
    def.max_motor_torque = max_motor_torque;
    def.motor_velocity = [motor_x, motor_y, motor_z];
    b3_create_spherical_joint(world, &def)
}

macro_rules! spherical_bool_abi {
    ($set_name:ident, $get_name:ident, $set:path, $get:path) => {
        #[no_mangle]
        pub extern "C" fn $set_name(id: JointId, value: bool) {
            $set(id, value);
        }

        #[no_mangle]
        pub extern "C" fn $get_name(id: JointId) -> bool {
            $get(id)
        }
    };
}

macro_rules! spherical_scalar_abi {
    ($set_name:ident, $get_name:ident, $set:path, $get:path) => {
        #[no_mangle]
        pub extern "C" fn $set_name(id: JointId, value: f32) {
            $set(id, value);
        }

        #[no_mangle]
        pub extern "C" fn $get_name(id: JointId) -> f32 {
            $get(id)
        }
    };
}

spherical_bool_abi!(
    gpu_b3_spherical_enable_cone_limit,
    gpu_b3_spherical_is_cone_limit_enabled,
    crate::api::b3_spherical_joint_enable_cone_limit,
    crate::api::b3_spherical_joint_is_cone_limit_enabled
);
spherical_scalar_abi!(
    gpu_b3_spherical_set_cone_limit,
    gpu_b3_spherical_get_cone_limit,
    crate::api::b3_spherical_joint_set_cone_limit,
    crate::api::b3_spherical_joint_get_cone_limit
);
spherical_bool_abi!(
    gpu_b3_spherical_enable_twist_limit,
    gpu_b3_spherical_is_twist_limit_enabled,
    crate::api::b3_spherical_joint_enable_twist_limit,
    crate::api::b3_spherical_joint_is_twist_limit_enabled
);
spherical_bool_abi!(
    gpu_b3_spherical_enable_spring,
    gpu_b3_spherical_is_spring_enabled,
    crate::api::b3_spherical_joint_enable_spring,
    crate::api::b3_spherical_joint_is_spring_enabled
);
spherical_scalar_abi!(
    gpu_b3_spherical_set_spring_hertz,
    gpu_b3_spherical_get_spring_hertz,
    crate::api::b3_spherical_joint_set_spring_hertz,
    crate::api::b3_spherical_joint_get_spring_hertz
);
spherical_scalar_abi!(
    gpu_b3_spherical_set_spring_damping,
    gpu_b3_spherical_get_spring_damping,
    crate::api::b3_spherical_joint_set_spring_damping,
    crate::api::b3_spherical_joint_get_spring_damping
);
spherical_bool_abi!(
    gpu_b3_spherical_enable_motor,
    gpu_b3_spherical_is_motor_enabled,
    crate::api::b3_spherical_joint_enable_motor,
    crate::api::b3_spherical_joint_is_motor_enabled
);
spherical_scalar_abi!(
    gpu_b3_spherical_set_max_motor_torque,
    gpu_b3_spherical_get_max_motor_torque,
    crate::api::b3_spherical_joint_set_max_motor_torque,
    crate::api::b3_spherical_joint_get_max_motor_torque
);

#[no_mangle]
pub extern "C" fn gpu_b3_spherical_get_cone_angle(id: JointId) -> f32 {
    crate::api::b3_spherical_joint_get_cone_angle(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_spherical_get_twist_angle(id: JointId) -> f32 {
    crate::api::b3_spherical_joint_get_twist_angle(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_spherical_get_lower_twist_limit(id: JointId) -> f32 {
    crate::api::b3_spherical_joint_get_lower_twist_limit(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_spherical_get_upper_twist_limit(id: JointId) -> f32 {
    crate::api::b3_spherical_joint_get_upper_twist_limit(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_spherical_set_twist_limits(id: JointId, lower: f32, upper: f32) {
    crate::api::b3_spherical_joint_set_twist_limits(id, lower, upper);
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_spherical_set_target_rotation(
    id: JointId,
    x: f32,
    y: f32,
    z: f32,
    w: f32,
) {
    crate::api::b3_spherical_joint_set_target_rotation(id, [x, y, z, w]);
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_spherical_get_target_rotation(id: JointId, output: *mut f32) {
    if !output.is_null() {
        let value = crate::api::b3_spherical_joint_get_target_rotation(id);
        std::ptr::copy_nonoverlapping(value.as_ptr(), output, 4);
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_spherical_set_motor_velocity(id: JointId, x: f32, y: f32, z: f32) {
    crate::api::b3_spherical_joint_set_motor_velocity(id, [x, y, z]);
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_spherical_get_motor_velocity(id: JointId, output: *mut f32) {
    if !output.is_null() {
        let value = crate::api::b3_spherical_joint_get_motor_velocity(id);
        std::ptr::copy_nonoverlapping(value.as_ptr(), output, 3);
    }
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_spherical_get_motor_torque(id: JointId, output: *mut f32) {
    if !output.is_null() {
        let value = crate::api::b3_spherical_joint_get_motor_torque(id);
        std::ptr::copy_nonoverlapping(value.as_ptr(), output, 3);
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_create_prismatic(
    world: WorldId,
    a: BodyId,
    b: BodyId,
    ax: f32,
    ay: f32,
    az: f32,
    bx: f32,
    by: f32,
    bz: f32,
    axis_x: f32,
    axis_y: f32,
    axis_z: f32,
    qa_x: f32,
    qa_y: f32,
    qa_z: f32,
    qa_w: f32,
    qb_x: f32,
    qb_y: f32,
    qb_z: f32,
    qb_w: f32,
    hertz: f32,
    damping: f32,
    collide_connected: bool,
    enable_spring: bool,
    spring_hertz: f32,
    spring_damping: f32,
    target_translation: f32,
    enable_limit: bool,
    lower_translation: f32,
    upper_translation: f32,
    enable_motor: bool,
    max_motor_force: f32,
    motor_speed: f32,
) -> JointId {
    let mut def = b3_default_prismatic_joint_def();
    def.body_a = a;
    def.body_b = b;
    def.local_anchor_a = [ax, ay, az];
    def.local_anchor_b = [bx, by, bz];
    def.local_axis_a = [axis_x, axis_y, axis_z];
    def.local_rotation_a = [qa_x, qa_y, qa_z, qa_w];
    def.local_rotation_b = [qb_x, qb_y, qb_z, qb_w];
    def.hertz = hertz;
    def.damping = damping;
    def.collide_connected = collide_connected;
    def.enable_spring = enable_spring;
    def.spring_hertz = spring_hertz;
    def.spring_damping = spring_damping;
    def.target_translation = target_translation;
    def.enable_limit = enable_limit;
    def.lower_translation = lower_translation;
    def.upper_translation = upper_translation;
    def.enable_motor = enable_motor;
    def.max_motor_force = max_motor_force;
    def.motor_speed = motor_speed;
    b3_create_prismatic_joint(world, &def)
}

#[no_mangle]
pub extern "C" fn gpu_b3_create_distance(
    world: WorldId,
    a: BodyId,
    b: BodyId,
    ax: f32,
    ay: f32,
    az: f32,
    bx: f32,
    by: f32,
    bz: f32,
    hertz: f32,
    damping: f32,
    collide_connected: bool,
    length: f32,
    enable_spring: bool,
    lower_spring_force: f32,
    upper_spring_force: f32,
    spring_hertz: f32,
    spring_damping: f32,
    enable_limit: bool,
    min_length: f32,
    max_length: f32,
    enable_motor: bool,
    max_motor_force: f32,
    motor_speed: f32,
) -> JointId {
    let mut def = b3_default_distance_joint_def();
    def.body_a = a;
    def.body_b = b;
    def.local_anchor_a = [ax, ay, az];
    def.local_anchor_b = [bx, by, bz];
    def.hertz = hertz;
    def.damping = damping;
    def.collide_connected = collide_connected;
    def.length = length;
    def.enable_spring = enable_spring;
    def.lower_spring_force = lower_spring_force;
    def.upper_spring_force = upper_spring_force;
    def.spring_hertz = spring_hertz;
    def.spring_damping = spring_damping;
    def.enable_limit = enable_limit;
    def.min_length = min_length;
    def.max_length = max_length;
    def.enable_motor = enable_motor;
    def.max_motor_force = max_motor_force;
    def.motor_speed = motor_speed;
    b3_create_distance_joint(world, &def)
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_set_length(id: JointId, length: f32) {
    crate::api::b3_distance_joint_set_length(id, length);
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_get_length(id: JointId) -> f32 {
    crate::api::b3_distance_joint_get_length(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_enable_spring(id: JointId, enable: bool) {
    crate::api::b3_distance_joint_enable_spring(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_is_spring_enabled(id: JointId) -> bool {
    crate::api::b3_distance_joint_is_spring_enabled(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_set_spring_force_range(id: JointId, lower: f32, upper: f32) {
    crate::api::b3_distance_joint_set_spring_force_range(id, lower, upper);
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_get_spring_force_range(
    id: JointId,
    lower: *mut f32,
    upper: *mut f32,
) {
    let range = crate::api::b3_distance_joint_get_spring_force_range(id);
    unsafe {
        if !lower.is_null() {
            *lower = range[0];
        }
        if !upper.is_null() {
            *upper = range[1];
        }
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_set_spring_hertz(id: JointId, hertz: f32) {
    crate::api::b3_distance_joint_set_spring_hertz(id, hertz);
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_get_spring_hertz(id: JointId) -> f32 {
    crate::api::b3_distance_joint_get_spring_hertz(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_set_spring_damping(id: JointId, damping: f32) {
    crate::api::b3_distance_joint_set_spring_damping(id, damping);
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_get_spring_damping(id: JointId) -> f32 {
    crate::api::b3_distance_joint_get_spring_damping(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_enable_limit(id: JointId, enable: bool) {
    crate::api::b3_distance_joint_enable_limit(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_is_limit_enabled(id: JointId) -> bool {
    crate::api::b3_distance_joint_is_limit_enabled(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_set_length_range(id: JointId, min: f32, max: f32) {
    crate::api::b3_distance_joint_set_length_range(id, min, max);
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_get_min_length(id: JointId) -> f32 {
    crate::api::b3_distance_joint_get_min_length(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_get_max_length(id: JointId) -> f32 {
    crate::api::b3_distance_joint_get_max_length(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_get_current_length(id: JointId) -> f32 {
    crate::api::b3_distance_joint_get_current_length(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_enable_motor(id: JointId, enable: bool) {
    crate::api::b3_distance_joint_enable_motor(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_is_motor_enabled(id: JointId) -> bool {
    crate::api::b3_distance_joint_is_motor_enabled(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_set_motor_speed(id: JointId, speed: f32) {
    crate::api::b3_distance_joint_set_motor_speed(id, speed);
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_get_motor_speed(id: JointId) -> f32 {
    crate::api::b3_distance_joint_get_motor_speed(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_set_max_motor_force(id: JointId, force: f32) {
    crate::api::b3_distance_joint_set_max_motor_force(id, force);
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_get_max_motor_force(id: JointId) -> f32 {
    crate::api::b3_distance_joint_get_max_motor_force(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_distance_get_motor_force(id: JointId) -> f32 {
    crate::api::b3_distance_joint_get_motor_force(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_create_parallel(
    world: WorldId,
    a: BodyId,
    b: BodyId,
    qa_x: f32,
    qa_y: f32,
    qa_z: f32,
    qa_w: f32,
    qb_x: f32,
    qb_y: f32,
    qb_z: f32,
    qb_w: f32,
    hertz: f32,
    damping: f32,
    max_torque: f32,
    collide_connected: bool,
) -> JointId {
    let mut def = b3_default_parallel_joint_def();
    def.body_a = a;
    def.body_b = b;
    def.local_rotation_a = [qa_x, qa_y, qa_z, qa_w];
    def.local_rotation_b = [qb_x, qb_y, qb_z, qb_w];
    def.hertz = hertz;
    def.damping = damping;
    def.max_torque = max_torque;
    def.collide_connected = collide_connected;
    b3_create_parallel_joint(world, &def)
}

#[no_mangle]
pub extern "C" fn gpu_b3_parallel_set_spring_hertz(id: JointId, hertz: f32) {
    crate::api::b3_parallel_joint_set_spring_hertz(id, hertz);
}

#[no_mangle]
pub extern "C" fn gpu_b3_parallel_get_spring_hertz(id: JointId) -> f32 {
    crate::api::b3_parallel_joint_get_spring_hertz(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_parallel_set_spring_damping(id: JointId, damping: f32) {
    crate::api::b3_parallel_joint_set_spring_damping(id, damping);
}

#[no_mangle]
pub extern "C" fn gpu_b3_parallel_get_spring_damping(id: JointId) -> f32 {
    crate::api::b3_parallel_joint_get_spring_damping(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_parallel_set_max_torque(id: JointId, torque: f32) {
    crate::api::b3_parallel_joint_set_max_torque(id, torque);
}

#[no_mangle]
pub extern "C" fn gpu_b3_parallel_get_max_torque(id: JointId) -> f32 {
    crate::api::b3_parallel_joint_get_max_torque(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_create_motor(
    world: WorldId,
    a: BodyId,
    b: BodyId,
    anchor_ax: f32,
    anchor_ay: f32,
    anchor_az: f32,
    anchor_bx: f32,
    anchor_by: f32,
    anchor_bz: f32,
    qa_x: f32,
    qa_y: f32,
    qa_z: f32,
    qa_w: f32,
    qb_x: f32,
    qb_y: f32,
    qb_z: f32,
    qb_w: f32,
    linear_x: f32,
    linear_y: f32,
    linear_z: f32,
    max_velocity_force: f32,
    angular_x: f32,
    angular_y: f32,
    angular_z: f32,
    max_velocity_torque: f32,
    linear_hertz: f32,
    linear_damping: f32,
    max_spring_force: f32,
    angular_hertz: f32,
    angular_damping: f32,
    max_spring_torque: f32,
    collide_connected: bool,
) -> JointId {
    let mut def = b3_default_motor_joint_def();
    def.body_a = a;
    def.body_b = b;
    def.local_anchor_a = [anchor_ax, anchor_ay, anchor_az];
    def.local_anchor_b = [anchor_bx, anchor_by, anchor_bz];
    def.local_rotation_a = [qa_x, qa_y, qa_z, qa_w];
    def.local_rotation_b = [qb_x, qb_y, qb_z, qb_w];
    def.linear_velocity = [linear_x, linear_y, linear_z];
    def.max_velocity_force = max_velocity_force;
    def.angular_velocity = [angular_x, angular_y, angular_z];
    def.max_velocity_torque = max_velocity_torque;
    def.linear_hertz = linear_hertz;
    def.linear_damping = linear_damping;
    def.max_spring_force = max_spring_force;
    def.angular_hertz = angular_hertz;
    def.angular_damping = angular_damping;
    def.max_spring_torque = max_spring_torque;
    def.collide_connected = collide_connected;
    b3_create_motor_joint(world, &def)
}

#[no_mangle]
pub extern "C" fn gpu_b3_motor_set_linear_velocity(id: JointId, x: f32, y: f32, z: f32) {
    crate::api::b3_motor_joint_set_linear_velocity(id, [x, y, z]);
}

#[no_mangle]
pub extern "C" fn gpu_b3_motor_get_linear_velocity(id: JointId) -> Vec3C {
    let value = crate::api::b3_motor_joint_get_linear_velocity(id);
    Vec3C {
        x: value[0],
        y: value[1],
        z: value[2],
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_motor_set_angular_velocity(id: JointId, x: f32, y: f32, z: f32) {
    crate::api::b3_motor_joint_set_angular_velocity(id, [x, y, z]);
}

#[no_mangle]
pub extern "C" fn gpu_b3_motor_get_angular_velocity(id: JointId) -> Vec3C {
    let value = crate::api::b3_motor_joint_get_angular_velocity(id);
    Vec3C {
        x: value[0],
        y: value[1],
        z: value[2],
    }
}

macro_rules! motor_scalar_abi {
    ($set_name:ident, $get_name:ident, $set:path, $get:path) => {
        #[no_mangle]
        pub extern "C" fn $set_name(id: JointId, value: f32) {
            $set(id, value);
        }

        #[no_mangle]
        pub extern "C" fn $get_name(id: JointId) -> f32 {
            $get(id)
        }
    };
}

motor_scalar_abi!(
    gpu_b3_motor_set_max_velocity_force,
    gpu_b3_motor_get_max_velocity_force,
    crate::api::b3_motor_joint_set_max_velocity_force,
    crate::api::b3_motor_joint_get_max_velocity_force
);
motor_scalar_abi!(
    gpu_b3_motor_set_max_velocity_torque,
    gpu_b3_motor_get_max_velocity_torque,
    crate::api::b3_motor_joint_set_max_velocity_torque,
    crate::api::b3_motor_joint_get_max_velocity_torque
);
motor_scalar_abi!(
    gpu_b3_motor_set_linear_hertz,
    gpu_b3_motor_get_linear_hertz,
    crate::api::b3_motor_joint_set_linear_hertz,
    crate::api::b3_motor_joint_get_linear_hertz
);
motor_scalar_abi!(
    gpu_b3_motor_set_linear_damping,
    gpu_b3_motor_get_linear_damping,
    crate::api::b3_motor_joint_set_linear_damping,
    crate::api::b3_motor_joint_get_linear_damping
);
motor_scalar_abi!(
    gpu_b3_motor_set_angular_hertz,
    gpu_b3_motor_get_angular_hertz,
    crate::api::b3_motor_joint_set_angular_hertz,
    crate::api::b3_motor_joint_get_angular_hertz
);
motor_scalar_abi!(
    gpu_b3_motor_set_angular_damping,
    gpu_b3_motor_get_angular_damping,
    crate::api::b3_motor_joint_set_angular_damping,
    crate::api::b3_motor_joint_get_angular_damping
);
motor_scalar_abi!(
    gpu_b3_motor_set_max_spring_force,
    gpu_b3_motor_get_max_spring_force,
    crate::api::b3_motor_joint_set_max_spring_force,
    crate::api::b3_motor_joint_get_max_spring_force
);
motor_scalar_abi!(
    gpu_b3_motor_set_max_spring_torque,
    gpu_b3_motor_get_max_spring_torque,
    crate::api::b3_motor_joint_set_max_spring_torque,
    crate::api::b3_motor_joint_get_max_spring_torque
);

#[no_mangle]
pub extern "C" fn gpu_b3_create_filter(
    world: WorldId,
    a: BodyId,
    b: BodyId,
    collide_connected: bool,
) -> JointId {
    let mut def = b3_default_filter_joint_def();
    def.body_a = a;
    def.body_b = b;
    def.collide_connected = collide_connected;
    b3_create_filter_joint(world, &def)
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_enable_spring(id: JointId, enable: bool) {
    crate::api::b3_prismatic_joint_enable_spring(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_is_spring_enabled(id: JointId) -> bool {
    crate::api::b3_prismatic_joint_is_spring_enabled(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_set_spring_hertz(id: JointId, hertz: f32) {
    crate::api::b3_prismatic_joint_set_spring_hertz(id, hertz);
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_get_spring_hertz(id: JointId) -> f32 {
    crate::api::b3_prismatic_joint_get_spring_hertz(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_set_spring_damping(id: JointId, damping: f32) {
    crate::api::b3_prismatic_joint_set_spring_damping(id, damping);
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_get_spring_damping(id: JointId) -> f32 {
    crate::api::b3_prismatic_joint_get_spring_damping(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_set_target_translation(id: JointId, translation: f32) {
    crate::api::b3_prismatic_joint_set_target_translation(id, translation);
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_get_target_translation(id: JointId) -> f32 {
    crate::api::b3_prismatic_joint_get_target_translation(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_enable_limit(id: JointId, enable: bool) {
    crate::api::b3_prismatic_joint_enable_limit(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_is_limit_enabled(id: JointId) -> bool {
    crate::api::b3_prismatic_joint_is_limit_enabled(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_get_lower_limit(id: JointId) -> f32 {
    crate::api::b3_prismatic_joint_get_lower_limit(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_get_upper_limit(id: JointId) -> f32 {
    crate::api::b3_prismatic_joint_get_upper_limit(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_set_limits(id: JointId, lower: f32, upper: f32) {
    crate::api::b3_prismatic_joint_set_limits(id, lower, upper);
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_enable_motor(id: JointId, enable: bool) {
    crate::api::b3_prismatic_joint_enable_motor(id, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_is_motor_enabled(id: JointId) -> bool {
    crate::api::b3_prismatic_joint_is_motor_enabled(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_set_motor_speed(id: JointId, speed: f32) {
    crate::api::b3_prismatic_joint_set_motor_speed(id, speed);
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_get_motor_speed(id: JointId) -> f32 {
    crate::api::b3_prismatic_joint_get_motor_speed(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_set_max_motor_force(id: JointId, force: f32) {
    crate::api::b3_prismatic_joint_set_max_motor_force(id, force);
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_get_max_motor_force(id: JointId) -> f32 {
    crate::api::b3_prismatic_joint_get_max_motor_force(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_get_motor_force(id: JointId) -> f32 {
    crate::api::b3_prismatic_joint_get_motor_force(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_get_translation(id: JointId) -> f32 {
    crate::api::b3_prismatic_joint_get_translation(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_prismatic_get_speed(id: JointId) -> f32 {
    crate::api::b3_prismatic_joint_get_speed(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_create_weld(
    world: WorldId,
    a: BodyId,
    b: BodyId,
    ax: f32,
    ay: f32,
    az: f32,
    bx: f32,
    by: f32,
    bz: f32,
    qa_x: f32,
    qa_y: f32,
    qa_z: f32,
    qa_w: f32,
    qb_x: f32,
    qb_y: f32,
    qb_z: f32,
    qb_w: f32,
    hertz: f32,
    damping: f32,
    collide_connected: bool,
    linear_hertz: f32,
    linear_damping: f32,
    angular_hertz: f32,
    angular_damping: f32,
) -> JointId {
    let mut def = b3_default_weld_joint_def();
    def.body_a = a;
    def.body_b = b;
    def.local_anchor_a = [ax, ay, az];
    def.local_anchor_b = [bx, by, bz];
    def.local_rotation_a = [qa_x, qa_y, qa_z, qa_w];
    def.local_rotation_b = [qb_x, qb_y, qb_z, qb_w];
    def.hertz = hertz;
    def.damping = damping;
    def.collide_connected = collide_connected;
    def.linear_hertz = linear_hertz;
    def.linear_damping_ratio = linear_damping;
    def.angular_hertz = angular_hertz;
    def.angular_damping_ratio = angular_damping;
    b3_create_weld_joint(world, &def)
}

#[no_mangle]
pub extern "C" fn gpu_b3_weld_set_linear_hertz(id: JointId, value: f32) {
    b3_weld_joint_set_linear_hertz(id, value);
}

#[no_mangle]
pub extern "C" fn gpu_b3_weld_get_linear_hertz(id: JointId) -> f32 {
    b3_weld_joint_get_linear_hertz(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_weld_set_linear_damping(id: JointId, value: f32) {
    b3_weld_joint_set_linear_damping_ratio(id, value);
}

#[no_mangle]
pub extern "C" fn gpu_b3_weld_get_linear_damping(id: JointId) -> f32 {
    b3_weld_joint_get_linear_damping_ratio(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_weld_set_angular_hertz(id: JointId, value: f32) {
    b3_weld_joint_set_angular_hertz(id, value);
}

#[no_mangle]
pub extern "C" fn gpu_b3_weld_get_angular_hertz(id: JointId) -> f32 {
    b3_weld_joint_get_angular_hertz(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_weld_set_angular_damping(id: JointId, value: f32) {
    b3_weld_joint_set_angular_damping_ratio(id, value);
}

#[no_mangle]
pub extern "C" fn gpu_b3_weld_get_angular_damping(id: JointId) -> f32 {
    b3_weld_joint_get_angular_damping_ratio(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_position(body: BodyId, out: *mut f32) {
    let p = b3_body_get_position(body);
    if !out.is_null() {
        unsafe {
            *out.add(0) = p[0];
            *out.add(1) = p[1];
            *out.add(2) = p[2];
        }
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_rotation(body: BodyId, out: *mut f32) {
    let q = b3_body_get_rotation(body);
    if !out.is_null() {
        unsafe {
            *out.add(0) = q[0];
            *out.add(1) = q[1];
            *out.add(2) = q[2];
            *out.add(3) = q[3];
        }
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_transform(body: BodyId, pos: *mut f32, rot: *mut f32) {
    let (p, q) = b3_body_get_transform(body);
    if !pos.is_null() {
        unsafe {
            *pos.add(0) = p[0];
            *pos.add(1) = p[1];
            *pos.add(2) = p[2];
        }
    }
    if !rot.is_null() {
        unsafe {
            *rot.add(0) = q[0];
            *rot.add(1) = q[1];
            *rot.add(2) = q[2];
            *rot.add(3) = q[3];
        }
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_prepare_pose_snapshot(id: WorldId) {
    crate::api::b3_world_prepare_pose_snapshot(id);
}

#[no_mangle]
pub extern "C" fn gpu_b3_copy_draw_items(world: WorldId, out: *mut DrawItemC, cap: i32) -> i32 {
    if out.is_null() || cap <= 0 {
        return b3_world_draw_items(world, &mut []);
    }
    let slice = unsafe { std::slice::from_raw_parts_mut(out, cap as usize) };
    b3_world_draw_items(world, slice)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_bounds(world: WorldId, out: *mut f32) {
    let b = b3_world_aabb(world);
    if !out.is_null() {
        unsafe {
            for i in 0..6 {
                *out.add(i) = b[i];
            }
        }
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_counts(
    world: WorldId,
    bodies: *mut i32,
    shapes: *mut i32,
    joints: *mut i32,
) {
    let (b, s, j) = b3_world_counts(world);
    unsafe {
        if !bodies.is_null() {
            *bodies = b;
        }
        if !shapes.is_null() {
            *shapes = s;
        }
        if !joints.is_null() {
            *joints = j;
        }
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_live_world_count() -> i32 {
    b3_live_world_count()
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_is_valid(body: BodyId) -> bool {
    b3_body_is_valid(body)
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_body_get_shapes(body: BodyId, out: *mut ShapeId, capacity: i32) -> i32 {
    if out.is_null() || capacity <= 0 {
        return 0;
    }
    let output = unsafe { std::slice::from_raw_parts_mut(out, capacity as usize) };
    b3_body_get_shapes(body, output) as i32
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_shape_count(body: BodyId) -> i32 {
    b3_body_get_shape_count(body)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_set_gravity(world: WorldId, x: f32, y: f32, z: f32) {
    b3_world_set_gravity(world, [x, y, z]);
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_get_gravity(world: WorldId) -> Vec3C {
    let g = b3_world_get_gravity(world);
    Vec3C {
        x: g[0],
        y: g[1],
        z: g[2],
    }
}

fn vec3_out(v: [f32; 3]) -> Vec3C {
    Vec3C {
        x: v[0],
        y: v[1],
        z: v[2],
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_apply_force(
    body: BodyId,
    fx: f32,
    fy: f32,
    fz: f32,
    px: f32,
    py: f32,
    pz: f32,
    wake: bool,
) {
    b3_body_apply_force(body, [fx, fy, fz], [px, py, pz], wake);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_apply_force_to_center(body: BodyId, x: f32, y: f32, z: f32, wake: bool) {
    b3_body_apply_force_to_center(body, [x, y, z], wake);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_apply_torque(body: BodyId, x: f32, y: f32, z: f32, wake: bool) {
    b3_body_apply_torque(body, [x, y, z], wake);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_mass(body: BodyId) -> f32 {
    b3_body_get_mass(body)
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_mass_data(body: BodyId) -> MassData {
    b3_body_get_mass_data(body)
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_set_mass_data(body: BodyId, data: MassData) {
    b3_body_set_mass_data(body, data);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_world_point(body: BodyId, x: f32, y: f32, z: f32) -> Vec3C {
    vec3_out(b3_body_get_world_point(body, [x, y, z]))
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_local_point(body: BodyId, x: f32, y: f32, z: f32) -> Vec3C {
    vec3_out(b3_body_get_local_point(body, [x, y, z]))
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_world_vector(body: BodyId, x: f32, y: f32, z: f32) -> Vec3C {
    vec3_out(b3_body_get_world_vector(body, [x, y, z]))
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_local_vector(body: BodyId, x: f32, y: f32, z: f32) -> Vec3C {
    vec3_out(b3_body_get_local_vector(body, [x, y, z]))
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_world_point_velocity(body: BodyId, x: f32, y: f32, z: f32) -> Vec3C {
    vec3_out(b3_body_get_world_point_velocity(body, [x, y, z]))
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_local_point_velocity(body: BodyId, x: f32, y: f32, z: f32) -> Vec3C {
    vec3_out(b3_body_get_local_point_velocity(body, [x, y, z]))
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_is_awake(body: BodyId) -> bool {
    b3_body_is_awake(body)
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_is_enabled(body: BodyId) -> bool {
    b3_body_is_enabled(body)
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_set_enabled(body: BodyId, enable: bool) {
    b3_body_set_enabled(body, enable);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_set_motion_locks(
    body: BodyId,
    linear_x: bool,
    linear_y: bool,
    linear_z: bool,
    angular_x: bool,
    angular_y: bool,
    angular_z: bool,
) {
    b3_body_set_motion_locks(
        body,
        MotionLocks {
            linear_x,
            linear_y,
            linear_z,
            angular_x,
            angular_y,
            angular_z,
        },
    );
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_motion_locks(
    body: BodyId,
    linear_x: *mut bool,
    linear_y: *mut bool,
    linear_z: *mut bool,
    angular_x: *mut bool,
    angular_y: *mut bool,
    angular_z: *mut bool,
) {
    let locks = b3_body_get_motion_locks(body);
    unsafe {
        if !linear_x.is_null() {
            *linear_x = locks.linear_x;
        }
        if !linear_y.is_null() {
            *linear_y = locks.linear_y;
        }
        if !linear_z.is_null() {
            *linear_z = locks.linear_z;
        }
        if !angular_x.is_null() {
            *angular_x = locks.angular_x;
        }
        if !angular_y.is_null() {
            *angular_y = locks.angular_y;
        }
        if !angular_z.is_null() {
            *angular_z = locks.angular_z;
        }
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_local_center(body: BodyId) -> Vec3C {
    vec3_out(b3_body_get_local_center(body))
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_linear_damping(body: BodyId) -> f32 {
    b3_body_get_linear_damping(body)
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_angular_damping(body: BodyId) -> f32 {
    b3_body_get_angular_damping(body)
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_gravity_scale(body: BodyId) -> f32 {
    b3_body_get_gravity_scale(body)
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_joint_count(body: BodyId) -> i32 {
    b3_body_get_joint_count(body)
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_body_get_joints(body: BodyId, out: *mut JointId, capacity: i32) -> i32 {
    if out.is_null() || capacity <= 0 {
        return 0;
    }
    let output = unsafe { std::slice::from_raw_parts_mut(out, capacity as usize) };
    b3_body_get_joints(body, output) as i32
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_set_friction(shape: ShapeId, friction: f32) {
    b3_shape_set_friction(shape, friction);
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_set_restitution(shape: ShapeId, restitution: f32) {
    b3_shape_set_restitution(shape, restitution);
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_apply_wind(
    shape: ShapeId,
    wx: f32,
    wy: f32,
    wz: f32,
    drag: f32,
    lift: f32,
    max_speed: f32,
    wake: bool,
) {
    b3_shape_apply_wind(shape, [wx, wy, wz], drag, lift, max_speed, wake);
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_is_valid(shape: ShapeId) -> bool {
    b3_shape_is_valid(shape)
}

#[no_mangle]
pub extern "C" fn gpu_b3_joint_is_valid(joint: JointId) -> bool {
    b3_joint_is_valid(joint)
}

#[no_mangle]
pub extern "C" fn gpu_b3_destroy_joint(joint: JointId, wake_attached: bool) {
    b3_destroy_joint(joint, wake_attached);
}

#[no_mangle]
pub extern "C" fn gpu_b3_destroy_body(body: BodyId) {
    b3_destroy_body(body);
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_get_body(shape: ShapeId) -> BodyId {
    b3_shape_body(shape)
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_kind(shape: ShapeId) -> i32 {
    b3_shape_kind(shape)
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_is_static(body: BodyId) -> bool {
    b3_body_is_static(body)
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_inv_mass(body: BodyId) -> f32 {
    b3_body_inv_mass(body)
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_set_transform(
    body: BodyId,
    px: f32,
    py: f32,
    pz: f32,
    rx: f32,
    ry: f32,
    rz: f32,
    rw: f32,
) {
    b3_body_set_transform(body, [px, py, pz], [rx, ry, rz, rw]);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_set_linear_velocity(body: BodyId, x: f32, y: f32, z: f32) {
    b3_body_set_linear_velocity(body, [x, y, z]);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_set_angular_velocity(body: BodyId, x: f32, y: f32, z: f32) {
    b3_body_set_angular_velocity(body, [x, y, z]);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_linear_velocity(body: BodyId) -> Vec3C {
    let value = b3_body_get_linear_velocity(body);
    Vec3C {
        x: value[0],
        y: value[1],
        z: value[2],
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_angular_velocity(body: BodyId) -> Vec3C {
    let value = b3_body_get_angular_velocity(body);
    Vec3C {
        x: value[0],
        y: value[1],
        z: value[2],
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_apply_linear_impulse(
    body: BodyId,
    ix: f32,
    iy: f32,
    iz: f32,
    px: f32,
    py: f32,
    pz: f32,
    wake: bool,
) {
    b3_body_apply_linear_impulse(body, [ix, iy, iz], [px, py, pz], wake);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_apply_linear_impulse_to_center(
    body: BodyId,
    x: f32,
    y: f32,
    z: f32,
    wake: bool,
) {
    b3_body_apply_linear_impulse_to_center(body, [x, y, z], wake);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_apply_angular_impulse(
    body: BodyId,
    x: f32,
    y: f32,
    z: f32,
    wake: bool,
) {
    b3_body_apply_angular_impulse(body, [x, y, z], wake);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_type(body: BodyId) -> i32 {
    b3_body_get_type(body) as i32
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_set_type(body: BodyId, body_type: i32) {
    let body_type = match body_type {
        1 => BodyType::Kinematic,
        2 => BodyType::Dynamic,
        _ => BodyType::Static,
    };
    b3_body_set_type(body, body_type);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_set_target_transform(
    body: BodyId,
    px: f32,
    py: f32,
    pz: f32,
    rx: f32,
    ry: f32,
    rz: f32,
    rw: f32,
    time_step: f32,
    wake: bool,
) {
    b3_body_set_target_transform(body, [px, py, pz], [rx, ry, rz, rw], time_step, wake);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_set_awake(body: BodyId, awake: bool) {
    b3_body_set_awake(body, awake);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_set_linear_damping(body: BodyId, damping: f32) {
    b3_body_set_linear_damping(body, damping);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_set_angular_damping(body: BodyId, damping: f32) {
    b3_body_set_angular_damping(body, damping);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_set_gravity_scale(body: BodyId, scale: f32) {
    b3_body_set_gravity_scale(body, scale);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_world_center(body: BodyId, out: *mut f32) {
    let p = b3_body_get_world_center(body);
    if !out.is_null() {
        unsafe {
            *out.add(0) = p[0];
            *out.add(1) = p[1];
            *out.add(2) = p[2];
        }
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_apply_mass_from_shapes(body: BodyId) {
    b3_body_apply_mass_from_shapes(body);
}


#[no_mangle]
pub extern "C" fn b3Contact_IsValid(id: crate::api::ContactId) -> bool { crate::api::b3_contact_is_valid(id) }
#[no_mangle]
pub extern "C" fn b3Contact_GetData(id: crate::api::ContactId) -> crate::api::ContactData { crate::api::b3_contact_get_data(id) }
#[no_mangle]
pub extern "C" fn b3Body_GetContactCapacity(id: crate::api::BodyId) -> i32 { crate::api::b3_body_get_contact_capacity(id) }
#[no_mangle]
pub extern "C" fn b3Shape_GetContactCapacity(id: crate::api::ShapeId) -> i32 { crate::api::b3_shape_get_contact_capacity(id) }
#[no_mangle]
pub unsafe extern "C" fn b3Body_GetContactData(id: crate::api::BodyId, out: *mut crate::api::ContactData, capacity: i32) -> i32 {
    if out.is_null() || capacity <= 0 { return 0; }
    crate::api::b3_body_get_contact_data(id, unsafe { std::slice::from_raw_parts_mut(out, capacity as usize) })
}
#[no_mangle]
pub unsafe extern "C" fn b3Shape_GetContactData(id: crate::api::ShapeId, out: *mut crate::api::ContactData, capacity: i32) -> i32 {
    if out.is_null() || capacity <= 0 { return 0; }
    crate::api::b3_shape_get_contact_data(id, unsafe { std::slice::from_raw_parts_mut(out, capacity as usize) })
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_set_contact_recycle_distance(id: crate::api::WorldId, distance: f32) {
    crate::api::b3_world_set_contact_recycle_distance(id, distance);
}
#[no_mangle]
pub extern "C" fn gpu_b3_world_get_contact_recycle_distance(id: crate::api::WorldId) -> f32 {
    crate::api::b3_world_get_contact_recycle_distance(id)
}

/// Opt-in diagnostic; never used by the viewer or ordinary stepping.
#[cfg(not(target_arch = "wasm32"))]
#[no_mangle]
pub extern "C" fn gpu_b3_world_dump_mesh_candidates(id: WorldId) {
    let words = pollster::block_on(crate::api::b3_world_sync_mesh_candidate_words(id));
    if words.is_empty() { return; }
    let count = words[0] as usize;
    eprintln!("mesh-candidate-count {} overflow {}", count, count > 2048);
    for r in words[1..].chunks_exact(16).take(count.min(2048)) {
        if r[1] >= 10 {
            eprintln!("solver-trace {} {} {} {} {:?}", r[0],r[1],r[2],r[3],
                r[4..].iter().map(|v| f32::from_bits(*v)).collect::<Vec<_>>());
            continue;
        }

        eprintln!("mesh-candidate {} {} {} {} point {} {} {} sep {} normal {} {} {} patch {} tri {} feature {}",
            r[0],r[1],r[2],r[3], f32::from_bits(r[4]), f32::from_bits(r[5]), f32::from_bits(r[6]),
            f32::from_bits(r[7]), f32::from_bits(r[8]), f32::from_bits(r[9]), f32::from_bits(r[10]),
            f32::from_bits(r[11]),r[12],r[13]);
    }
}

#[repr(C)]
pub struct Matrix3C { pub columns: [[f32; 3]; 3] }

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_local_rotational_inertia(body: BodyId) -> Matrix3C {
    Matrix3C { columns: crate::api::b3_body_get_local_rotational_inertia(body) }
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_world_inverse_rotational_inertia(body: BodyId) -> Matrix3C {
    Matrix3C { columns: crate::api::b3_body_get_world_inverse_rotational_inertia(body) }
}

#[no_mangle]
pub extern "C" fn gpu_b3_shape_set_density(id: ShapeId, density: f32, update_mass: bool) {
    crate::api::b3_shape_set_density(id, density, update_mass);
}

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_world(id: BodyId) -> WorldId { crate::api::b3_body_get_world(id) }

#[no_mangle]
pub extern "C" fn gpu_b3_shape_get_world(id: ShapeId) -> WorldId { crate::api::b3_shape_get_world(id) }

#[no_mangle]
pub extern "C" fn gpu_b3_joint_get_world(id: JointId) -> WorldId { crate::api::b3_joint_get_world(id) }

#[no_mangle]
pub extern "C" fn gpu_b3_joint_get_kind(id: JointId) -> u32 { crate::api::b3_joint_get_kind(id) }

#[no_mangle]
pub extern "C" fn gpu_b3_joint_get_body(id: JointId, second: bool) -> BodyId { crate::api::b3_joint_get_body(id, second) }

#[no_mangle]
pub extern "C" fn gpu_b3_world_set_maximum_linear_speed(id: WorldId, speed: f32) {
    crate::api::b3_world_set_maximum_linear_speed(id, speed);
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_get_maximum_linear_speed(id: WorldId) -> f32 {
    crate::api::b3_world_get_maximum_linear_speed(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_joint_set_collide_connected(id: JointId, enable: bool) {
    crate::api::b3_joint_set_collide_connected(id, enable);
}
#[no_mangle]
pub extern "C" fn gpu_b3_joint_get_collide_connected(id: JointId) -> bool {
    crate::api::b3_joint_get_collide_connected(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_set_restitution_threshold(id: WorldId, value: f32) {
    crate::api::b3_world_set_restitution_threshold(id, value);
}
#[no_mangle]
pub extern "C" fn gpu_b3_world_get_restitution_threshold(id: WorldId) -> f32 {
    crate::api::b3_world_get_restitution_threshold(id)
}

#[cfg(not(target_arch = "wasm32"))]
#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_contact_metrics(id: WorldId, out: *mut crate::api::WorldContactMetrics) {
    if let Some(out) = out.as_mut() { *out = crate::api::b3_world_contact_metrics(id,false); }
}

/// Called on the sample loading worker, with UI world access suspended.
#[no_mangle]
pub extern "C" fn gpu_b3_world_prepare_loading(id: WorldId, stage: u32) {
    if stage == 1 { crate::loading::set("Preparing GPU buffers and common shaders"); crate::api::b3_world_ensure_gpu(id); }
    if stage == 2 { crate::loading::set("Preparing scene collision shader"); crate::api::b3_world_prepare_collision(id); }
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_loading_needed(id: WorldId) -> bool {
    crate::api::b3_world_loading_needed(id)
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_set_contact_tuning(id: WorldId, hertz: f32, damping: f32, speed: f32) -> () { crate::api::b3_world_set_contact_tuning(id, hertz, damping, speed) }

#[no_mangle]
pub extern "C" fn gpu_b3_world_set_user_data(id: WorldId, value: usize) -> () { crate::api::b3_world_set_user_data(id, value) }

#[no_mangle]
pub extern "C" fn gpu_b3_world_get_user_data(id: WorldId) -> usize { crate::api::b3_world_get_user_data(id) }

#[no_mangle]
pub extern "C" fn gpu_b3_world_get_awake_body_count(id: WorldId) -> i32 { crate::api::b3_world_get_awake_body_count(id) }

#[no_mangle]
pub extern "C" fn gpu_b3_body_enable_sleep(id: BodyId, enable: bool) -> () { crate::api::b3_body_enable_sleep(id, enable) }

#[no_mangle]
pub extern "C" fn gpu_b3_body_is_sleep_enabled(id: BodyId) -> bool { crate::api::b3_body_is_sleep_enabled(id) }

#[no_mangle]
pub extern "C" fn gpu_b3_body_set_sleep_threshold(id: BodyId, value: f32) -> () { crate::api::b3_body_set_sleep_threshold(id, value) }

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_sleep_threshold(id: BodyId) -> f32 { crate::api::b3_body_get_sleep_threshold(id) }

#[no_mangle]
pub extern "C" fn gpu_b3_body_enable_hit_events(id: BodyId, enable: bool) -> () { crate::api::b3_body_enable_hit_events(id, enable) }

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_body_set_name(id: BodyId, name: *const std::ffi::c_char) -> () { crate::api::b3_body_set_name(id, if name.is_null() { None } else { Some(std::ffi::CStr::from_ptr(name)) }) }

#[no_mangle]
pub extern "C" fn gpu_b3_body_get_name(id: BodyId) -> *const std::ffi::c_char { crate::api::b3_body_get_name(id) }

#[no_mangle]
pub extern "C" fn gpu_b3_joint_set_local_frame(id: JointId, second: bool,
    x: f32, y: f32, z: f32, qx: f32, qy: f32, qz: f32, qw: f32) {
    crate::api::b3_joint_set_local_frame(id, second, [x,y,z], [qx,qy,qz,qw]);
}
#[no_mangle]
pub unsafe extern "C" fn gpu_b3_joint_get_local_frame(id: JointId, second: bool, out: *mut f32) {
    if out.is_null() { return; }
    let (p,q) = crate::api::b3_joint_get_local_frame(id, second);
    std::ptr::copy_nonoverlapping(p.as_ptr(), out, 3);
    std::ptr::copy_nonoverlapping(q.as_ptr(), out.add(3), 4);
}
#[no_mangle]
pub extern "C" fn gpu_b3_joint_wake_bodies(id: JointId) { crate::api::b3_joint_wake_bodies(id); }

#[no_mangle]
pub extern "C" fn gpu_b3_shape_compute_mass_data(id: ShapeId) -> crate::api::MassData {
    crate::api::b3_shape_compute_mass_data(id)
}

/// Explicit diagnostic readback; never called by normal stepping or drawing.
#[cfg(not(target_arch = "wasm32"))]
#[no_mangle]
pub extern "C" fn gpu_b3_world_dump_phases(id: WorldId, frame: u32) {
    let words = pollster::block_on(crate::api::b3_world_sync_phase_words(id));
    if words.len() < 192 { return; }
    let count = (words.len() - 192) / 192;
    for phase in 0..24 {
        for body in 0..count {
            let at = 192 + (phase * count + body) * 8;
            eprintln!("GPUphase {} {} {} {:.9} {:.9} {:.9} {}", frame, phase, body,
                f32::from_bits(words[at]), f32::from_bits(words[at+1]),
                f32::from_bits(words[at+2]), words[at+3]);
            eprintln!("GPUvelocity {} {} {} {:.9e} {:.9e} {:.9e} {:.9e} {:.9e} {:.9e}", frame, phase, body,
                f32::from_bits(words[at]), f32::from_bits(words[at+4]), f32::from_bits(words[at+1]),
                f32::from_bits(words[at+5]), f32::from_bits(words[at+6]), f32::from_bits(words[at+7]));
        }
    }
}

#[no_mangle]
pub extern "C" fn gpu_b3_set_sphere(id: ShapeId, sphere: Sphere) -> bool {
    crate::api::b3_shape_set_sphere(id, &sphere)
}
#[no_mangle]
pub extern "C" fn gpu_b3_set_capsule(id: ShapeId, capsule: Capsule) -> bool {
    crate::api::b3_shape_set_capsule(id, &capsule)
}
#[no_mangle]
pub extern "C" fn gpu_b3_set_box_hull(
    id: ShapeId,
    hx: f32,
    hy: f32,
    hz: f32,
    ox: f32,
    oy: f32,
    oz: f32,
) -> bool {
    crate::api::set_box_hull_geometry(
        id,
        &BoxHull {
            half_extents: [hx, hy, hz],
            center: [ox, oy, oz],
        },
        true,
    )
}

#[no_mangle]
pub extern "C" fn gpu_b3_joint_get_linear_separation(id: JointId) -> f32 { crate::api::b3_joint_get_linear_separation(id) }

#[no_mangle]
pub extern "C" fn gpu_b3_joint_get_angular_separation(id: JointId) -> f32 { crate::api::b3_joint_get_angular_separation(id) }

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_joint_get_constraint_force(id: JointId, out: *mut f32) {
    if !out.is_null() { std::ptr::copy_nonoverlapping(crate::api::b3_joint_get_constraint_force(id).as_ptr(), out, 3); }
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_joint_get_constraint_torque(id: JointId, out: *mut f32) {
    if !out.is_null() { std::ptr::copy_nonoverlapping(crate::api::b3_joint_get_constraint_torque(id).as_ptr(), out, 3); }
}

/// Opt-in diagnostic output; unavailable in production builds without replay diagnostics.
#[cfg(all(feature = "replay-diagnostics", not(target_arch = "wasm32")))]
#[no_mangle]
pub extern "C" fn gpu_b3_world_diagnostic_contact_impulses(id: WorldId, first: u64, count: u32, clear: bool) -> bool {
    match crate::api::b3_world_diagnostic_contact_impulses(id,first,count,clear) {
        Ok((selected,nonzero)) => {eprintln!("contact-impulse-control clear={clear} selected={selected} nonzero={nonzero}"); selected>0 && nonzero>0},
        Err(error) => {eprintln!("contact-impulse-control: {error}");false}
    }
}

/// Opt-in diagnostic output; unavailable in production builds without replay diagnostics.
#[cfg(all(feature = "replay-diagnostics", not(target_arch = "wasm32")))]
#[no_mangle]
pub extern "C" fn gpu_b3_world_diagnostic_spherical_impulses(id: WorldId, first: u64, count: u32, clear: bool) -> bool {
    match crate::api::b3_world_diagnostic_spherical_impulses(id,first,count,clear) {
        Ok((selected,nonzero)) => {eprintln!("spherical-impulse-control clear={clear} selected={selected} nonzero={nonzero}"); selected>0 && nonzero>0},
        Err(error) => {eprintln!("spherical-impulse-control: {error}");false}
    }
}

/// Opt-in diagnostic output; unavailable in production builds without replay diagnostics.
#[cfg(all(feature = "replay-diagnostics", not(target_arch = "wasm32")))]
#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_write_core_state(id: WorldId, path: *const std::os::raw::c_char, frame: u32) -> bool {
    if path.is_null() { return false; }
    let Ok(path) = std::ffi::CStr::from_ptr(path).to_str() else { return false; };
    match crate::api::b3_world_write_core_state(id, std::path::Path::new(path), frame) {
        Ok(()) => true,
        Err(error) => { eprintln!("core-state capture: {error}"); false }
    }
}

/// Diagnostic fixture bridge matching the explicitly linked CPU reference hook.
#[cfg(all(feature="replay-diagnostics",not(target_arch="wasm32")))]
#[no_mangle]
pub unsafe extern "C" fn reference_set_spherical_cache(id: JointId, values: *const f32) -> bool {
    if values.is_null() {return false;}
    let values: [f32;12]=std::slice::from_raw_parts(values,12).try_into().unwrap();
    match crate::api::b3_joint_diagnostic_set_spherical_cache(id,values) {
        Ok(())=>true,
        Err(e)=>{eprintln!("spherical-cache-transplant: {e}");false}
    }
}

/// Diagnostic fixture bridge matching the explicitly linked CPU reference hook.
#[cfg(all(feature="replay-diagnostics",not(target_arch="wasm32")))]
#[no_mangle]
pub unsafe extern "C" fn reference_set_revolute_cache(id: JointId, values: *const f32) -> bool {
    if values.is_null() {return false;}
    let values: [f32;9]=std::slice::from_raw_parts(values,9).try_into().unwrap();
    match crate::api::b3_joint_diagnostic_set_revolute_cache(id,values) {
        Ok(())=>true,
        Err(e)=>{eprintln!("revolute-cache-transplant: {e}");false}
    }
}

// Native diagnostic extension: out parameters avoid aggregate-return ABI
// differences; C wrappers retain the pinned Box3D layout.
#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_native_profile(id: WorldId, out: *mut crate::api::NativeProfile) {
    if let Some(out) = out.as_mut() { *out = crate::api::b3_world_native_profile(id); }
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_native_counters(id: WorldId, out: *mut crate::api::NativeCounters) -> u32 {
    let Some(out) = out.as_mut() else { return crate::api::NATIVE_DIAGNOSTIC_INVALID; };
    let (status, counters) = crate::api::b3_world_native_counters(id);
    *out = counters;
    status
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_native_max_capacity(id: WorldId, out: *mut crate::api::NativeCapacity) -> u32 {
    let Some(out) = out.as_mut() else { return crate::api::NATIVE_DIAGNOSTIC_INVALID; };
    let (status, capacity) = crate::api::b3_world_native_max_capacity(id);
    *out = capacity;
    status
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_native_allocation(id: WorldId, out: *mut crate::api::NativeAllocation) {
    if let Some(out) = out.as_mut() { *out = crate::api::b3_world_native_allocation(id); }
}

#[no_mangle]
pub extern "C" fn gpu_b3_world_native_visit_shape_bounds(id: WorldId, body_type: u32,
    visitor: Option<unsafe extern "C" fn(*const crate::api::NativeShapeBounds, *mut std::ffi::c_void)>,
    context: *mut std::ffi::c_void) -> u32 {
    let Some(visitor) = visitor else { return crate::api::NATIVE_DIAGNOSTIC_INVALID; };
    match crate::api::b3_world_native_shape_bounds(id, body_type) {
        Ok(bounds) => {
            for record in &bounds { unsafe { visitor(record, context); } }
            crate::api::NATIVE_DIAGNOSTIC_OK
        }
        Err(status) => status,
    }
}
