//! Independent progress storage: the UI must never take the physics world lock.
use std::sync::Mutex;
static CURRENT: Mutex<String> = Mutex::new(String::new());

pub(crate) fn set(message: &str) {
    if let Ok(mut value) = CURRENT.lock() { *value = message.to_owned(); }
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_loading_message(out: *mut u8, capacity: usize) {
    if out.is_null() || capacity == 0 { return; }
    let value = CURRENT.lock().unwrap_or_else(|e| e.into_inner());
    let n = value.len().min(capacity - 1);
    unsafe { std::ptr::copy_nonoverlapping(value.as_ptr(), out, n); *out.add(n) = 0; }
}

#[cfg(all(test, not(target_arch = "wasm32")))]
mod tests {
    use crate::{api::*, sim::GpuDevice};
    #[test]
    fn preparation_does_not_advance_physics_and_recreated_world_prepares_again() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
        for _ in 0..2 {
            let world = b3_create_world(gpu.clone(), &b3_default_world_def());
            let mut def = b3_default_body_def();
            def.body_type = BodyType::Dynamic;
            def.position = [1.0, 5.0, 2.0];
            let body = b3_create_body(world, &def);
            b3_create_hull_shape(body, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
            assert!(b3_world_loading_needed(world));
            std::thread::spawn(move || {
                b3_world_ensure_gpu(world);
                b3_world_prepare_collision(world);
            }).join().unwrap();
            assert!(!b3_world_loading_needed(world));
            assert_eq!(b3_world_physics_step(world), 0);
            assert_eq!(b3_world_pose_snapshot_step(world), 0);
            assert_eq!(b3_body_get_position(body), def.position);
            b3_destroy_world(world);
        }
        assert!(pollster::block_on(gpu.device.pop_error_scope()).is_none());
    }
}
