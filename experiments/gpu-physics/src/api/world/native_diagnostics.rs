//! Native diagnostic values and their availability, separate from solver policy.
use super::*;

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct NativeCounters {
    pub body_count: i32, pub shape_count: i32, pub contact_count: i32,
    pub joint_count: i32, pub island_count: i32,
    pub stack_used: i32, pub arena_capacity: i32, pub static_tree_height: i32,
    pub tree_height: i32, pub sat_call_count: i32, pub sat_cache_hit_count: i32,
    pub byte_count: i32, pub task_count: i32,
    pub color_counts: [i32; 24], pub manifold_counts: [i32; 8],
    pub awake_contact_count: i32, pub recycled_contact_count: i32,
    pub distance_iterations: i32, pub push_back_iterations: i32, pub root_iterations: i32,
}

impl Default for NativeCounters {
    fn default() -> Self {
        Self { body_count: -1, shape_count: -1, contact_count: -1, joint_count: -1,
            island_count: -1, stack_used: -1, arena_capacity: -1, static_tree_height: -1,
            tree_height: -1, sat_call_count: -1, sat_cache_hit_count: -1, byte_count: -1,
            task_count: -1, color_counts: [-1; 24], manifold_counts: [-1; 8],
            awake_contact_count: -1, recycled_contact_count: -1, distance_iterations: -1,
            push_back_iterations: -1, root_iterations: -1 }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct NativeCapacity {
    pub static_shape_count: i32, pub dynamic_shape_count: i32,
    pub static_body_count: i32, pub dynamic_body_count: i32, pub contact_count: i32,
}

impl NativeCapacity {
    pub(super) fn observe(&mut self, w: &WorldInner) {
        let mut current = Self::default();
        for body in w.bodies.iter().flatten() {
            if body.gpu.flags & FLAG_STATIC != 0 { current.static_body_count += 1; }
            else { current.dynamic_body_count += 1; }
        }
        for shape in w.shapes.iter().flatten().filter(|s| s.compound_parent == 0) {
            let Some(body) = w.bodies.get(shape.body_index.saturating_sub(1) as usize).and_then(Option::as_ref) else { continue; };
            if body.gpu.flags & FLAG_DISABLED != 0 { continue; }
            if body.gpu.flags & FLAG_STATIC != 0 { current.static_shape_count += 1; }
            else { current.dynamic_shape_count += 1; }
        }
        self.static_shape_count = self.static_shape_count.max(current.static_shape_count);
        self.dynamic_shape_count = self.dynamic_shape_count.max(current.dynamic_shape_count);
        self.static_body_count = self.static_body_count.max(current.static_body_count);
        self.dynamic_body_count = self.dynamic_body_count.max(current.dynamic_body_count);
    }
}

// b3Profile has 23 floats. Bits refer to its field order in the pinned header.
pub const NATIVE_PROFILE_MEASURED: u32 = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3)
    | (1 << 4) | (1 << 9) | (1 << 10);

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct NativeProfile {
    pub physics_step: u64, pub timestamp_step: u64,
    pub valid: u32, pub measured_fields: u32, pub values: [f32; 23],
}

impl Default for NativeProfile {
    fn default() -> Self {
        Self { physics_step: 0, timestamp_step: 0, valid: 0, measured_fields: 0, values: [f32::NAN; 23] }
    }
}

/// Status values shared with native_diagnostics.h. A failed world's diagnostic
/// read may return partial topology, but never hides its sticky physics failure.
pub const NATIVE_DIAGNOSTIC_INVALID: u32 = 0;
pub const NATIVE_DIAGNOSTIC_OK: u32 = 1;
pub const NATIVE_DIAGNOSTIC_BUSY: u32 = 2;
pub const NATIVE_DIAGNOSTIC_FAILED: u32 = 3;

#[cfg(not(target_arch = "wasm32"))]
fn native_world<T>(id: WorldId, f: impl FnOnce(&mut WorldInner) -> T) -> Result<T, u32> {
    let mut worlds = match WORLDS.try_lock() {
        Ok(worlds) => worlds,
        Err(std::sync::TryLockError::Poisoned(e)) => e.into_inner(),
        Err(std::sync::TryLockError::WouldBlock) => return Err(NATIVE_DIAGNOSTIC_BUSY),
    };
    let w = slot_mut(&mut worlds, id).ok_or(NATIVE_DIAGNOSTIC_INVALID)?;
    Ok(f(w))
}

#[cfg(not(target_arch = "wasm32"))]
pub(super) fn harvest_native_diagnostics(w: &mut WorldInner) {
    if let Some(sim) = w.sim.as_mut() {
        sim.finish_contact_status();
        w.maximum_capacity.contact_count = w.maximum_capacity.contact_count
            .max(i32::try_from(sim.native_contact_peak()).unwrap_or(i32::MAX));
        w.physics_invalid |= sim.physics_invalid();
        if let Some(message) = sim.sticky_fail_message() {
            w.gpu_fail = std::ffi::CString::new(message).ok();
            w.physics_invalid = true;
        }
    }
}

/// Profile reads poll existing timestamps; they never wait or download contacts.
#[cfg(not(target_arch = "wasm32"))]
pub fn b3_world_native_profile(id: WorldId) -> NativeProfile {
    native_world(id, |w| {
        if let Some(sim) = w.sim.as_mut() { sim.harvest_gpu_timestamps(false); }
        copy_gpu_clocks(w);
        let mut out = NativeProfile { physics_step: w.physics_step, timestamp_step: w.last_timestamp_step,
            valid: if w.physics_invalid { NATIVE_DIAGNOSTIC_FAILED } else { NATIVE_DIAGNOSTIC_OK },
            ..NativeProfile::default() };
        if w.last_timestamp_step != 0 {
            out.measured_fields = NATIVE_PROFILE_MEASURED;
            out.values[0] = w.last_gpu_device_ms;
            out.values[1] = w.last_gpu_broadphase_ms;
            out.values[2] = w.last_gpu_narrowphase_ms + w.last_gpu_graph_ms;
            out.values[3] = w.last_gpu_prepare_ms + w.last_gpu_solve_ms + w.last_gpu_integrate_ms;
            out.values[4] = w.last_gpu_prepare_ms;
            out.values[9] = w.last_gpu_solve_ms;
            out.values[10] = w.last_gpu_integrate_ms;
        }
        out
    }).unwrap_or_else(|valid| NativeProfile { valid, ..NativeProfile::default() })
}

/// Explicit, potentially blocking contact-registry diagnostics. Busy callbacks
/// fail closed instead of recursively acquiring the world's mutex.
#[cfg(not(target_arch = "wasm32"))]
pub fn b3_world_native_counters(id: WorldId) -> (u32, NativeCounters) {
    native_world(id, |w| {
        let mut out = NativeCounters::default();
        out.body_count = w.bodies.iter().flatten().count() as i32;
        out.shape_count = w.shapes.iter().flatten().filter(|s| s.compound_parent == 0).count() as i32;
        out.joint_count = w.joints.iter().filter(|j| j.kind != JOINT_NONE).count() as i32;
        if let Some(sim) = w.sim.as_mut() { sim.wait_completion(); }
        harvest_native_diagnostics(w);
        if !w.physics_invalid {
            sync_world_mirror_parts(w, id, true, true, true);
            harvest_native_diagnostics(w);
            if !w.physics_invalid {
                out.contact_count = w.contact_registry.len() as i32;
                out.manifold_counts = [0; 8];
                for entry in w.contact_registry.values() {
                    if !entry.manifolds.is_empty() {
                        out.manifold_counts[entry.manifolds.len().min(8) - 1] += 1;
                    }
                }
                if w.bodies.iter().all(Option::is_none) { out.island_count = 0; }
                else if w.physics_step != 0 && !w.scene_dirty && !w.bodies_dirty {
                    let mut islands = std::collections::HashSet::new();
                    for body in w.bodies.iter().flatten() {
                        if body.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC | FLAG_DISABLED) == 0 {
                            islands.insert(body.gpu.island_id);
                        }
                    }
                    out.island_count = islands.len() as i32;
                }
            }
        }
        out.byte_count = w.sim.as_ref().map_or(0, |sim|
            i32::try_from(sim.primary_buffer_bytes()).unwrap_or(-1));
        (if w.physics_invalid { NATIVE_DIAGNOSTIC_FAILED } else { NATIVE_DIAGNOSTIC_OK }, out)
    }).unwrap_or_else(|status| (status, NativeCounters::default()))
}

/// Topology peaks are sampled at positive step boundaries; the contact peak
/// samples supported non-sensor roots during occupied-list collection (pre-CCD).
/// These are occupancy counts, not reserved GPU buffer sizes.
#[cfg(not(target_arch = "wasm32"))]
pub fn b3_world_native_max_capacity(id: WorldId) -> (u32, NativeCapacity) {
    native_world(id, |w| {
        if let Some(sim) = w.sim.as_mut() { sim.wait_completion(); }
        harvest_native_diagnostics(w);
        (if w.physics_invalid { NATIVE_DIAGNOSTIC_FAILED } else { NATIVE_DIAGNOSTIC_OK }, w.maximum_capacity)
    }).unwrap_or_else(|status| (status, NativeCapacity::default()))
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct NativeAllocation {
    pub primary_buffer_bytes: u64,
    pub valid: u32,
    pub body_capacity: u32,
    pub shape_capacity: u32,
    pub joint_capacity: u32,
    pub contact_capacity: u32,
    pub physics_invalid: u32,
}

#[cfg(not(target_arch = "wasm32"))]
pub fn b3_world_native_allocation(id: WorldId) -> NativeAllocation {
    native_world(id, |w| {
        let mut out = NativeAllocation { valid: if w.physics_invalid { NATIVE_DIAGNOSTIC_FAILED }
            else { NATIVE_DIAGNOSTIC_OK }, physics_invalid: u32::from(w.physics_invalid),
            ..NativeAllocation::default() };
        if let Some(sim) = w.sim.as_ref() {
            out.primary_buffer_bytes = sim.primary_buffer_bytes();
            out.body_capacity = sim.caps.bodies;
            out.shape_capacity = sim.caps.shapes;
            out.joint_capacity = sim.caps.joints;
            out.contact_capacity = sim.reserved_contact_capacity();
        }
        out
    }).unwrap_or_else(|valid| NativeAllocation { valid, ..NativeAllocation::default() })
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct NativeShapeBounds {
    pub shape: ShapeId,
    pub body: BodyId,
    pub bounds: crate::api::Aabb,
}

/// Collect owned data under one lock, then allow C to print/visit it after unlock.
/// Include disabled shapes, and merge compound child bounds under the public ID.
#[cfg(not(target_arch = "wasm32"))]
pub fn b3_world_native_shape_bounds(id: WorldId, body_type: u32) -> Result<Vec<NativeShapeBounds>, u32> {
    if body_type > 2 { return Err(NATIVE_DIAGNOSTIC_INVALID); }
    native_world(id, |w| {
        if let Some(sim) = w.sim.as_mut() { sim.wait_completion(); }
        harvest_native_diagnostics(w);
        if w.physics_invalid { return Err(NATIVE_DIAGNOSTIC_FAILED); }
        sync_world_mirror_parts(w, id, false, false, false);
        let mut result: Vec<NativeShapeBounds> = Vec::new();
        let mut public_indices = std::collections::HashMap::new();
        for (index, shape) in w.shapes.iter().enumerate().filter_map(|(i, s)| s.as_ref().map(|s| (i, s))) {
            let Some(body) = w.bodies.get(shape.body_index.saturating_sub(1) as usize).and_then(Option::as_ref) else { continue; };
            let kind = if body.gpu.flags & FLAG_STATIC != 0 { 0 }
                else if body.gpu.flags & FLAG_KINEMATIC != 0 { 1 } else { 2 };
            if kind != body_type { continue; }
            let Some(host) = host_shape_direct(w, id.index1, index, shape) else { continue; };
            let bounds = crate::api::query::public_shape_aabb(&host);
            if let Some(&destination) = public_indices.get(&host.public_id) {
                let old: &mut NativeShapeBounds = &mut result[destination];
                for axis in 0..3 {
                    old.bounds.lower_bound[axis] = old.bounds.lower_bound[axis].min(bounds.lower_bound[axis]);
                    old.bounds.upper_bound[axis] = old.bounds.upper_bound[axis].max(bounds.upper_bound[axis]);
                }
            } else {
                public_indices.insert(host.public_id, result.len());
                result.push(NativeShapeBounds { shape: host.public_id, body: BodyId {
                    index1: shape.body_index, world0: id.index1, generation: body.generation }, bounds });
            }
        }
        Ok(result)
    })?
}

#[cfg(all(test, not(target_arch = "wasm32")))]
mod tests {
    use super::*;

    fn box_at(world: WorldId, kind: BodyType, position: [f32; 3]) -> (BodyId, ShapeId) {
        let body = b3_create_body(world, &BodyDef { body_type: kind, position,
            ..crate::api::b3_default_body_def() });
        let shape = b3_create_hull_shape(body, &crate::api::b3_default_shape_def(),
            &crate::api::b3_make_box_hull(0.5, 0.5, 0.5));
        (body, shape)
    }

    #[test]
    fn native_diagnostics_contract() {
        assert_eq!(std::mem::size_of::<NativeCounters>(), 200);
        assert_eq!(std::mem::offset_of!(NativeCounters, color_counts), 52);
        assert_eq!(std::mem::offset_of!(NativeCounters, manifold_counts), 148);
        assert_eq!(std::mem::size_of::<NativeProfile>(), 120);
        assert_eq!(std::mem::size_of::<NativeAllocation>(), 32);
        assert_eq!(std::mem::size_of::<NativeShapeBounds>(), 40);
        let gpu = pollster::block_on(GpuDevice::new(None)).expect("NVIDIA Vulkan GPU");
        let world = b3_create_world(gpu.clone(), &WorldDef { gravity: [0.0; 3], enable_sleep: false,
            ..crate::api::b3_default_world_def() });
        let empty = b3_world_native_counters(world);
        assert_eq!((empty.0, empty.1.body_count, empty.1.contact_count, empty.1.island_count), (1,0,0,0));
        assert_eq!(b3_world_native_max_capacity(world), (1,NativeCapacity::default()));
        let pending = b3_world_native_profile(world);
        assert_eq!((pending.valid, pending.measured_fields, pending.timestamp_step), (1,0,0));
        assert!(pending.values.iter().all(|v| v.is_nan()));
        assert_eq!(b3_world_native_allocation(world).primary_buffer_bytes, 0);
        {
            let _worlds = lock_worlds();
            assert_eq!(b3_world_native_profile(world).valid, NATIVE_DIAGNOSTIC_BUSY);
            assert_eq!(b3_world_native_counters(world).0, NATIVE_DIAGNOSTIC_BUSY);
            assert_eq!(b3_world_native_max_capacity(world).0, NATIVE_DIAGNOSTIC_BUSY);
            assert_eq!(b3_world_native_allocation(world).valid, NATIVE_DIAGNOSTIC_BUSY);
            assert_eq!(b3_world_native_shape_bounds(world,2).unwrap_err(), NATIVE_DIAGNOSTIC_BUSY);
        }
        box_at(world, BodyType::Static, [0.0,0.0,0.0]);
        let (touching, shape) = box_at(world, BodyType::Dynamic, [0.0,0.99,0.0]);
        box_at(world, BodyType::Dynamic, [5.0,2.0,0.0]);
        box_at(world, BodyType::Kinematic, [10.0,2.0,0.0]);
        assert_eq!(b3_world_native_counters(world).1.island_count, -1);
        b3_world_step_gpu(world, 0.0, 4);
        assert_eq!(b3_world_native_max_capacity(world).1, NativeCapacity::default());
        for _ in 0..4 { b3_world_step_gpu(world, 1.0/60.0, 4); }
        let (status, counters) = b3_world_native_counters(world);
        assert_eq!(status, 1);
        assert_eq!((counters.body_count,counters.shape_count,counters.joint_count), (4,4,0));
        assert_eq!(counters.contact_count, 1);
        assert_eq!(counters.island_count, 2);
        assert_eq!(counters.manifold_counts.iter().sum::<i32>(), 1);
        let allocation = b3_world_native_allocation(world);
        assert_eq!(allocation.valid, 1);
        assert!(allocation.primary_buffer_bytes > 0 && allocation.body_capacity >= 4);
        assert_eq!(counters.byte_count as u64, allocation.primary_buffer_bytes);
        let peak = b3_world_native_max_capacity(world).1;
        assert_eq!((peak.static_body_count,peak.dynamic_body_count,peak.static_shape_count,peak.dynamic_shape_count), (1,3,1,3));
        assert_eq!(peak.contact_count,1);
        let profile = b3_world_native_profile(world);
        assert_eq!(profile.valid,1);
        if profile.timestamp_step != 0 {
            assert_eq!(profile.measured_fields,NATIVE_PROFILE_MEASURED);
            assert!(profile.timestamp_step <= profile.physics_step);
        }
        for (field,value) in profile.values.iter().enumerate() {
            if profile.measured_fields & (1 << field) == 0 { assert!(value.is_nan()); }
            else { assert!(value.is_finite() && *value >= 0.0); }
        }
        let bounds = b3_world_native_shape_bounds(world,2).unwrap();
        let actual = bounds.iter().find(|bounds| bounds.shape == shape).unwrap();
        assert_eq!(actual.body,touching);
        assert_eq!(actual.bounds,crate::api::b3_shape_get_aabb(shape));
        assert_eq!(b3_world_native_shape_bounds(world,3).unwrap_err(),0);
        let isolated = b3_create_world(gpu,&crate::api::b3_default_world_def());
        assert_eq!(b3_world_native_max_capacity(isolated).1,NativeCapacity::default());
        b3_destroy_world(isolated);
        b3_destroy_body(touching);
        assert_eq!(b3_world_native_counters(world).1.contact_count,0);
        assert_eq!(b3_world_native_counters(world).1.island_count,-1);
        assert_eq!(b3_world_native_max_capacity(world).1,peak);
        b3_world_set_diagnostic_flags(world,crate::types::DIAG_FORCE_CAPACITY_LOSS);
        b3_world_step_gpu(world,1.0/60.0,4);
        assert_eq!(b3_world_native_counters(world).0, NATIVE_DIAGNOSTIC_FAILED);
        assert_eq!(b3_world_native_counters(world).1.contact_count,-1);
        assert_eq!(b3_world_native_max_capacity(world).0,NATIVE_DIAGNOSTIC_FAILED);
        assert_eq!(b3_world_native_allocation(world).valid,NATIVE_DIAGNOSTIC_FAILED);
        assert_eq!(b3_world_native_profile(world).valid,NATIVE_DIAGNOSTIC_FAILED);
        b3_world_clear_capacity_status(world);
        assert!(b3_world_physics_invalid(world));
        let step=b3_world_physics_step(world);
        b3_world_step_gpu(world,1.0/60.0,4);
        assert_eq!(b3_world_physics_step(world),step);
        b3_destroy_world(world);
        assert_eq!(b3_world_native_counters(world).0,0);
        assert_eq!(b3_world_native_profile(world).valid,0);
        println!("native diagnostic contract passed; profile/counters/peak/ownership/busy/sticky failure");
    }

    #[cfg(feature = "replay-diagnostics")]
    #[test]
    fn trace_native_diagnostic_peaks() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("NVIDIA Vulkan GPU");
        let world=b3_create_world(gpu,&WorldDef {gravity:[0.0;3],enable_sleep:false,
            ..crate::api::b3_default_world_def()});
        box_at(world,BodyType::Static,[0.0;3]);
        let (body,_) = box_at(world,BodyType::Dynamic,[0.0,0.99,0.0]);
        b3_world_ensure_gpu(world);
        with_world_mut_no_sync(world,|w| w.sim.as_mut().unwrap().hold_idle_status_test(true));
        for _ in 0..4 {b3_world_step_gpu(world,1.0/60.0,4);}
        b3_destroy_body(body); // Peak must survive unread steps and retirement.
        for i in 0..35 {box_at(world,BodyType::Dynamic,[5.0+2.0*i as f32,4.0,0.0]);}
        b3_world_step_gpu(world,1.0/60.0,4); // Force capacity growth.
        with_world_mut_no_sync(world,|w| w.sim.as_mut().unwrap().hold_idle_status_test(false));
        let peak=b3_world_native_max_capacity(world).1;
        assert_eq!(peak.contact_count,1);
        assert_eq!(peak.dynamic_body_count,35);
        assert_eq!(b3_world_native_counters(world).1.contact_count,0);
        let trace=std::env::temp_dir().join(format!("gpu-native-peak-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&trace);
        b3_world_write_core_state(world,&trace,5).unwrap();
        let frame:serde_json::Value=serde_json::from_str(&std::fs::read_to_string(&trace).unwrap()).unwrap();
        assert_eq!(frame["schema"],"gpu-core-state-v24");
        assert_eq!(frame["host_state"]["maximum_capacity"],serde_json::json!([1,35,1,35,1]));
        assert_eq!(frame["gpu_policy"]["native_contact_peak"],1);
        assert_eq!(frame["contact_allocation"]["native_contact_peak_device"],1);
        if let Some(path)=std::env::var_os("GPU_NATIVE_PEAK_TRACE") {std::fs::copy(&trace,path).unwrap();}
        std::fs::remove_file(trace).unwrap();
        b3_destroy_world(world);
        println!("native diagnostic peak/capture passed across unread steps, retirement and growth");
    }
}
