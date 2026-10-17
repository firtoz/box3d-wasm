use super::*;
use crate::api::*;

#[test]
fn device_loss_is_terminal_for_shared_worlds_and_does_not_poison_other_devices() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let device = gpu.device.clone();
    let wd = b3_default_world_def();
    let a = b3_create_world(gpu.clone(), &wd);
    let b = b3_create_world(gpu, &wd);
    let healthy = b3_create_world(pollster::block_on(GpuDevice::new(None)).unwrap(), &wd);
    for world in [a, b, healthy] {
        let mut bd = b3_default_body_def(); bd.body_type = BodyType::Dynamic;
        let body = b3_create_body(world, &bd);
        b3_create_sphere_shape(body, &b3_default_shape_def(), &Sphere { center: [0.0; 3], radius: 0.5 });
        b3_world_step_gpu(world, 1.0 / 60.0, 1);
        b3_world_gpu_wait(world);
        assert!(!b3_world_physics_invalid(world));
    }
    device.destroy();
    poll_until_idle(&device);
    for world in [a, b] {
        assert!(b3_world_physics_invalid(world));
        assert!(!b3_world_gpu_fail(world).is_null());
        assert_eq!(b3_world_native_profile(world).valid, NATIVE_DIAGNOSTIC_FAILED);
        let step = b3_world_physics_step(world);
        b3_world_step_gpu(world, 1.0 / 60.0, 1);
        assert_eq!(b3_world_physics_step(world), step);
        b3_world_clear_capacity_status(world);
        b3_native_api_clear_error();
        assert!(b3_world_physics_invalid(world), "diagnostic clearing must never heal device loss");
        assert!(!b3_world_gpu_fail(world).is_null());
        assert!(b3_world_is_valid(world), "failed worlds must remain destroyable");
        b3_destroy_world(world);
    }
    assert!(!b3_world_physics_invalid(healthy));
    let step = b3_world_physics_step(healthy);
    b3_world_step_gpu(healthy, 1.0 / 60.0, 1);
    b3_world_gpu_wait(healthy);
    assert!(b3_world_physics_step(healthy) > step);
    b3_destroy_world(healthy);
}
