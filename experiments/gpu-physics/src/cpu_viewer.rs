//! Linux-only real Box3D reference for matched renderer measurements.
use std::ffi::{c_void, CString};
use crate::{api::*, types::{BodyStateGpu, DemoConfig, DemoScene}};

type Destroy = unsafe extern "C" fn(*mut c_void);
type Step = unsafe extern "C" fn(*mut c_void, f32, i32);
type States = unsafe extern "C" fn(*mut c_void, *mut BodyStateGpu, u32) -> u32;
pub struct CpuViewer {
    library: *mut c_void,
    world: *mut c_void,
    destroy: Destroy,
    step: Step,
    states: States,
    records: Vec<BodyStateGpu>,
    pub buffer: wgpu::Buffer,
    pub steps: u64,
}
impl CpuViewer {
    pub fn from_env(device: &wgpu::Device, cfg: &DemoConfig, gpu_world: WorldId, sleep: bool) -> Option<Self> {
        let path = std::env::var("GPU_PHYSICS_CPU_REFERENCE").ok()?;
        assert!(matches!(cfg.scene, DemoScene::Dominoes | DemoScene::MixedStacks | DemoScene::FallingCubes), "CPU viewer supports Dominoes, mixed stacks and falling cubes only");
        assert!(cfg.contacts && !cfg.jacobi, "CPU reference requires ordinary contacts");
        unsafe {
            let path = CString::new(path).unwrap();
            let library = libc::dlopen(path.as_ptr(), libc::RTLD_NOW | libc::RTLD_LOCAL);
            assert!(!library.is_null(), "load CPU viewer bridge: {:?}", std::ffi::CStr::from_ptr(libc::dlerror()));
            unsafe fn symbol<T: Copy>(lib: *mut c_void, name: &str) -> T {
                let name = CString::new(name).unwrap();
                let ptr = libc::dlsym(lib, name.as_ptr());
                assert!(!ptr.is_null(), "missing CPU bridge symbol {name:?}");
                assert_eq!(std::mem::size_of::<T>(), std::mem::size_of_val(&ptr));
                std::mem::transmute_copy(&ptr)
            }
            let abi: unsafe extern "C" fn() -> u32 = symbol(library, "viewer_cpu_abi");
            assert_eq!(abi(), 2);
            let create: unsafe extern "C" fn(*const libc::c_char, u32, i32) -> *mut c_void = symbol(library, "viewer_cpu_create_workers");
            let count: unsafe extern "C" fn(*mut c_void) -> u32 = symbol(library, "viewer_cpu_count");
            let scene = CString::new(cfg.scene.slug()).unwrap();
            let scale = crate::types::scene_scale_count(cfg.scene, cfg.body_count, cfg.body_count_explicit);
            let workers: i32 = std::env::var("GPU_PHYSICS_CPU_WORKERS").unwrap_or_else(|_| "4".into()).parse().expect("CPU worker count");
            let world = create(scene.as_ptr(), scale, workers);
            assert!(!world.is_null(), "CPU scene creation failed");
            let set_sleep: unsafe extern "C" fn(*mut c_void, bool) -> bool = symbol(library, "viewer_cpu_set_sleeping");
            assert_eq!(set_sleep(world, sleep), sleep, "CPU sleep setting differs");
            let n = count(world);
            assert_eq!(n as i32, b3_world_counts(gpu_world).0, "CPU/GPU body counts differ");
            let records = vec![bytemuck::Zeroable::zeroed(); n as usize];
            let buffer = device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("real CPU Box3D render states"), size: n as u64 * std::mem::size_of::<BodyStateGpu>() as u64,
                usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_DST, mapped_at_creation: false,
            });
            let mut result = Self { library, world, destroy: symbol(library,"viewer_cpu_destroy"), step: symbol(library,"viewer_cpu_step"), states: symbol(library,"viewer_cpu_states"), records, buffer, steps: 0 };
            result.snapshot();
            // These named fixtures preserve creation order and have dense body slots.
            for id in b3_world_dynamic_body_ids(gpu_world) {
                let record = &result.records[id.index1 as usize - 1];
                let p = b3_body_get_world_center(id);
                let q = b3_body_get_rotation(id);
                let v = b3_body_get_linear_velocity(id);
                for i in 0..3 { assert!((p[i]-record.pos[i]).abs()<1e-5); assert!((v[i]-record.vel[i]).abs()<1e-5); }
                for i in 0..4 { assert!((q[i]-record.rot[i]).abs()<1e-5); }
            }
            eprintln!("CPU viewer: real Box3D, {n} bodies, initial transforms/velocities matched, {workers} CPU workers");
            Some(result)
        }
    }
    fn snapshot(&mut self) {
        let n = self.records.len() as u32;
        assert_eq!(unsafe { (self.states)(self.world, self.records.as_mut_ptr(), n) }, n);
    }
    pub fn seed_render_state(&self,queue:&wgpu::Queue) {queue.write_buffer(&self.buffer,0,bytemuck::cast_slice(&self.records));}
    pub fn advance(&mut self, queue: &wgpu::Queue) {
        unsafe { (self.step)(self.world, crate::types::FIXED_DT, crate::types::DEFAULT_SUB_STEPS as i32); }
        self.steps += 1;
        self.snapshot();
        queue.write_buffer(&self.buffer, 0, bytemuck::cast_slice(&self.records));
    }
}
impl Drop for CpuViewer {
    fn drop(&mut self) { unsafe { (self.destroy)(self.world); libc::dlclose(self.library); } }
}
