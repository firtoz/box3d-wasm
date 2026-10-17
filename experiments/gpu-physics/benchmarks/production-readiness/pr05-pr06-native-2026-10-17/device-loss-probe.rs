use gpu_physics::{api::*, sim::GpuDevice};
fn main() {
    let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
    let device = gpu.device.clone();
    let world = b3_create_world(gpu, &b3_default_world_def());
    let mut bd = b3_default_body_def(); bd.body_type = BodyType::Dynamic;
    let body = b3_create_body(world, &bd);
    b3_create_sphere_shape(body, &b3_default_shape_def(), &Sphere { center: [0.0; 3], radius: 0.5 });
    b3_world_step_gpu(world, 1.0 / 60.0, 1);
    b3_world_gpu_wait(world);
    println!("before device destroy: invalid={}", b3_world_physics_invalid(world));
    device.destroy();
    gpu_physics::sim::poll_until_idle(&device);
    let invalid = b3_world_physics_invalid(world);
    let reported = !b3_world_gpu_fail(world).is_null();
    println!("after actual device destroy: invalid={invalid} reported={reported}");
    b3_destroy_world(world);
    if !invalid || !reported { std::process::exit(2); }
}
