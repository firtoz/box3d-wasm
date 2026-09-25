mod joint_reaction;
pub use joint_reaction::*;
mod joint_separation;
pub use joint_separation::*;
mod contact_api;
mod shape_geometry;
pub use shape_geometry::*;
pub use contact_api::*;
use std::{collections::HashMap, sync::Arc, sync::Mutex};

use bytemuck::Zeroable;

use super::query::{
    explosion_geometry, rigid_mesh_time_of_impacts, rigid_time_of_impact, HostBody, HostShape,
    HostWorld, QueryIndex, MeshInstance,
};
use crate::api::{
    b3_null_body_id, b3_null_joint_id, b3_null_shape_id, b3_null_world_id, BodyDef, BodyId,
    BodyMoveEvent, BodyType, BoxHull, Capsule, ContactBeginTouchEvent, ContactEndTouchEvent,
    ContactHitEvent, ContactId, ConvexHull, CustomFilterCallback, DistanceJointDef, ExplosionDef,
    Filter, FilterJointDef, FrictionCallback, JointEvent, JointId, MassData, MotionLocks, MotorJointDef, ParallelJointDef,
    PreSolveCallback, PrismaticJointDef, QueryFilter, RayResult, RestitutionCallback, RevoluteJointDef,
    SensorBeginTouchEvent, SensorEndTouchEvent, ShapeDef, ShapeId, Sphere, SphericalJointDef,
    SurfaceMaterial, Vec3, WeldJointDef, WheelJointDef, WorldDef, WorldId, WorldTransform,
};
use crate::sim::{pack_scene_bytes, poll_until_idle, GpuDevice, GpuSim};
use crate::types::{
    box_mass, gpu_is_non_dynamic, sphere_mass, BodyGpu, GpuSceneCaps, JointGpu, MixPairGpu, ShapeGpu,
    SurfaceMaterialGpu, TopologyBounds, CONTACT_START_TOUCHING, CONTACT_STOP_TOUCHING, CONTACT_TOUCHING,
    DEFAULT_SUB_STEPS, FIXED_DT, FLAG_ALLOW_FAST_ROTATION, FLAG_BULLET, FLAG_DISABLED, FLAG_DISABLE_CONTACT_RECYCLING, FLAG_HIDDEN,
    FLAG_KINEMATIC, FLAG_LOCK_ANG_X, FLAG_LOCK_ANG_Y, FLAG_LOCK_ANG_Z, FLAG_LOCK_LIN_X,
    FLAG_LOCK_LIN_Y, FLAG_LOCK_LIN_Z, FLAG_SLEEP, FLAG_SLEEP_ENABLED, FLAG_STATIC, JOINT_COLLIDE_CONNECTED,
    JOINT_DISTANCE, JOINT_FILTER, JOINT_MOTOR, JOINT_NONE, JOINT_PARALLEL, JOINT_PRISMATIC,
    JOINT_REVOLUTE, JOINT_SPHERICAL, JOINT_WELD, JOINT_WHEEL, KIND_BOX, KIND_CAPSULE,
    KIND_CONVEX_HULL, KIND_MESH, KIND_SPHERE, LINEAR_SLOP, REVOLUTE_ENABLE_LIMIT,
    REVOLUTE_ENABLE_MOTOR, REVOLUTE_ENABLE_SPRING, SHAPE_ENABLE_CONTACT_EVENTS,
    SHAPE_ENABLE_CUSTOM_FILTERING, SHAPE_ENABLE_HIT_EVENTS, SHAPE_ENABLE_PRE_SOLVE_EVENTS,
    SHAPE_PUBLIC_PROXY, SHAPE_COMPOUND_CHILD, SHAPE_DISABLE_SPECULATIVE, SHAPE_ENABLE_SENSOR_EVENTS, SHAPE_IS_SENSOR, SPHERICAL_ENABLE_CONE_LIMIT,
    SPHERICAL_ENABLE_MOTOR, SPHERICAL_ENABLE_SPRING, SPHERICAL_ENABLE_TWIST_LIMIT,
    WHEEL_ENABLE_SPIN_MOTOR, WHEEL_ENABLE_STEERING, WHEEL_ENABLE_STEERING_LIMIT,
    WHEEL_ENABLE_SUSPENSION_LIMIT, WHEEL_ENABLE_SUSPENSION_SPRING,
};

struct CpuBody {
    name: Option<std::ffi::CString>,
    sleep_threshold: f32,
    generation: u16,
    user_data: usize,
    gpu: BodyGpu,
    /// Original mass; inverse-mass round trips lose precision in public queries.
    mass: f32,
    /// Center of mass in the body's local frame.
    local_center: [f32; 3],
    /// Symmetric local inertia tensor: xx, yy, zz, xy, xz, yz.
    local_inertia: [f32; 6],
    has_shape: bool,
    /// Live shape slots in creation order, including compound proxies.
    shape_indices: Vec<usize>,
    /// Capsule local half-axis (from midpoint toward center2).
    axis: [f32; 3],
    min_extent: f32,
    max_extent: f32,
    force: [f32; 3],
    torque: [f32; 3],
    /// Host writes newer than `GpuSim.snapshot_epoch` must not be overwritten
    /// by a previous-step render snapshot or GPU mirror.
    host_epoch: u64,
}

#[derive(Clone)]
struct CpuShape {
    #[allow(dead_code)]
    generation: u16,
    /// Public owner body. Compatibility proxy bodies used by the current GPU
    /// backend must never leak through the shape API.
    body_index: i32,
    kind: u32,
    /// Public kind. Height fields share the mesh GPU representation;
    /// baked compounds are host-only parents with hidden child colliders.
    public_kind: u32,
    /// Public compound shape index1, or 0 for a standalone shape.
    compound_parent: i32,
    /// Stable native child ordinal; deleting a sibling must not renumber it.
    compound_child_index: i32,
    next_compound_child_index: i32,
    compound_material_indices: [i32; 4],
    half: [f32; 3],
    axis: [f32; 3],
    density: f32,
    friction: f32,
    restitution: f32,
    rolling: f32,
    tangent_velocity: [f32; 3],
    explosion_scale: f32,
    filter: Filter,
    event_flags: u32,
    user_material_id: u64,
    custom_color: u32,
    mesh_materials: Vec<SurfaceMaterial>,
    user_data: usize,
    inner_radius: f32,
    hull_points: Vec<[f32; 3]>,
    hull_planes: Vec<[f32; 4]>,
    hull_edges: Vec<[f32; 3]>,
    hull_topology: Vec<[u32; 4]>,
    mesh_vertices: Arc<Vec<[f32; 3]>>,
    mesh_triangles: Arc<Vec<[u32; 4]>>,
    mesh_triangle_ids: Arc<Vec<i32>>,
    mesh_nodes: Arc<Vec<MeshNode>>,
    mesh_scale: [f32; 3],
    mesh_instance: Option<MeshInstance>,
    mass: f32,
    unit_mass: f32,
    unit_inertia: [f32; 6],
    geometry_center: [f32; 3],
    local_center: [f32; 3],
    /// Central inertia tensor in the owner's local frame.
    local_inertia: [f32; 6],
}

#[derive(Clone, Copy, Debug, Default)]
pub struct MeshNode {
    pub lower: [f32; 3],
    pub upper: [f32; 3],
    pub data: u32,
    pub triangle_offset: u32,
}

impl CpuShape {
    fn gpu(
        &self,
        body_map: &[u32],
        hull_slot: u32,
        plane_slot: u32,
        edge_slot: u32,
        topology_slot: u32,
        material_slot: u32,
        shape_index: u32,
        cpu_index: u32,
    ) -> Option<ShapeGpu> {
        let source = usize::try_from(self.body_index - 1).ok()?;
        let body_index = *body_map.get(source)?;
        if body_index == u32::MAX {
            return None;
        }
        let mesh = self.kind == KIND_MESH;
        Some(ShapeGpu {
            body_index,
            plane_slot,
            kind: self.kind,
            topology_counts: if mesh {
                self.mesh_nodes.len() as u32
            } else {
                (self.hull_points.len() as u32 & 0xff)
                    | ((self.hull_planes.len() as u32 & 0xff) << 8)
                    | ((self.hull_edges.len() as u32 & 0xff) << 16)
                    | ((self.hull_topology.len() as u32 & 0xff) << 24)
            },
            local_center: self.geometry_center,
            topology_slot: if mesh {
                self.mesh_triangles.len() as u32
            } else {
                topology_slot
            },
            half: self.half,
            rolling: self.rolling,
            axis: if mesh { self.mesh_scale } else { self.axis },
            friction: self.friction,
            restitution: self.restitution,
            inner_radius: self.inner_radius,
            hull_slot,
            edge_slot,
            category_bits_lo: self.filter.category_bits as u32,
            category_bits_hi: (self.filter.category_bits >> 32) as u32,
            mask_bits_lo: self.filter.mask_bits as u32,
            mask_bits_hi: (self.filter.mask_bits >> 32) as u32,
            group_index: self.filter.group_index,
            event_flags: self.event_flags
                | if self.public_kind == PUBLIC_KIND_COMPOUND { SHAPE_PUBLIC_PROXY } else { 0 }
                | if self.compound_parent != 0 { SHAPE_COMPOUND_CHILD } else { 0 },
            material_slot,
            material_count: self.mesh_materials.len() as u32,
            tangent_velocity: self.tangent_velocity,
            custom_color: self.custom_color,
            user_material_id_lo: self.user_material_id as u32,
            user_material_id_hi: (self.user_material_id >> 32) as u32,
            _pad_filter: [shape_index, cpu_index],
            instance_position: self.mesh_instance.map_or([0.0; 3], |instance| instance.position),
            instance_flags: u32::from(self.mesh_instance.is_some()),
            instance_rotation: self.mesh_instance.map_or([0.0, 0.0, 0.0, 1.0], |instance| instance.rotation),
        })
    }
}

// Preserve GPU collider order while exposing only public shape identities.
// Bounds-only parent slots and hidden child slots map to their public owner.
fn gpu_public_shapes<'a>(shapes: &'a [Option<CpuShape>], bodies: &[Option<CpuBody>]) -> Vec<(usize, &'a CpuShape)> {
    shapes.iter().enumerate().filter_map(|(source, slot)| {
        let shape = slot.as_ref()?;
        bodies.get(usize::try_from(shape.body_index - 1).ok()?)?.as_ref()?;
        if shape.compound_parent == 0 { return Some((source, shape)); }
        let parent_index = usize::try_from(shape.compound_parent - 1).ok()?;
        let parent = shapes.get(parent_index)?.as_ref()?;
        debug_assert_eq!(parent.public_kind, PUBLIC_KIND_COMPOUND);
        debug_assert_eq!(parent.body_index, shape.body_index);
        Some((parent_index, parent))
    }).collect()
}

// Native compound contacts distinguish child ordinals within a public pair.
// Keep full public slot widths rather than packing a third index into 64 bits.
type ContactKey = (u64, i32);
fn keyed_contact(pair: u64, packed: u64, children: &[i32]) -> ContactKey {
    let a = children.get((packed & 0xffff_ffff) as usize).copied().unwrap_or(-1);
    let b = children.get((packed >> 32) as usize).copied().unwrap_or(-1);
    (pair, a.max(b))
}

#[derive(Clone, Copy)]
struct LiveContact {
    shape_id_a: ShapeId,
    shape_id_b: ShapeId,
    contact_id: ContactId,
    events_enabled: bool,
}

#[derive(Clone, Copy)]
struct CpuJoint {
    reaction_frames: [[f32; 4]; 2],
    pending_reaction_frames: Option<([[f32; 4]; 2], bool)>,
    generation: u16,
    force_threshold: f32,
    torque_threshold: f32,
    user_data: usize,
}

impl CpuJoint {
    fn new(force_threshold: f32, torque_threshold: f32, user_data: usize) -> Self {
        Self {
            reaction_frames: [[0.0; 4]; 2],
            pending_reaction_frames: None,
            generation: 1,
            force_threshold,
            torque_threshold,
            user_data,
        }
    }
}

// Classification depends on dirty-guarded host metadata and force accumulators, not evolving GPU poses.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
struct SceneCapabilities {
    pending_forces: bool,
    shape_events: bool,
    ccd_shapes: bool,
    component_bodies: bool,
    has_bullet: bool,
}

struct WorldInner {
    user_data: usize,
    generation: u16,
    def: WorldDef,
    enable_contacts: bool,
    custom_filter_callback: Option<CustomFilterCallback>,
    custom_filter_context: usize,
    pre_solve_callback: Option<PreSolveCallback>,
    pre_solve_context: usize,
    friction_callback: Option<FrictionCallback>,
    restitution_callback: Option<RestitutionCallback>,
    bodies: Vec<Option<CpuBody>>,
    free_bodies: Vec<usize>,
    body_generations: Vec<u16>,
    shapes: Vec<Option<CpuShape>>,
    joints: Vec<JointGpu>,
    joint_meta: Vec<Option<CpuJoint>>,
    sim: Option<GpuSim>,
    gpu: GpuDevice,
    scene_dirty: bool,
    bodies_dirty: bool,
    pending_fat_transforms: HashMap<(i32, u16), Vec<[f32; 8]>>,
    jacobi: bool,
    last_substep_h: f32,
    reaction_inv_h: f32,
    contact_recycle_distance: f32,
    contact_registry: HashMap<ContactKey, contact_api::ContactEntry>,
    contact_by_id: HashMap<(i32, u32), ContactKey>,
    contact_by_body: HashMap<i32, Vec<ContactKey>>,
    contact_by_shape: HashMap<i32, Vec<ContactKey>>,
    contact_snapshot_key: Option<(u64, u64, u64)>,
    contact_snapshot_copies: u64,
    live_contacts: HashMap<ContactKey, LiveContact>,
    contact_shape_ids: std::sync::Arc<Vec<ShapeId>>,
    previous_contact_shape_ids: std::sync::Arc<Vec<ShapeId>>,
    contact_child_ordinals: Arc<Vec<i32>>,
    previous_contact_child_ordinals: Arc<Vec<i32>>,
    contact_shape_revision: u64,
    contact_begin_events: Vec<ContactBeginTouchEvent>,
    contact_end_events: Vec<ContactEndTouchEvent>,
    contact_hit_events: Vec<ContactHitEvent>,
    deferred_contact_end_events: Vec<ContactEndTouchEvent>,
    sensor_overlaps: HashMap<i32, Vec<ShapeId>>,
    sensor_begin_events: Vec<SensorBeginTouchEvent>,
    sensor_end_events: Vec<SensorEndTouchEvent>,
    deferred_sensor_end_events: Vec<SensorEndTouchEvent>,
    continuous_sensor_hits: Vec<(ShapeId, ShapeId)>,
    step_start_bodies: Vec<BodyGpu>,
    body_move_events: Vec<BodyMoveEvent>,
    joint_events: Vec<JointEvent>,
    /// GPU poses have not been copied into the CPU mirror since the last submit.
    gpu_mirror_stale: bool,
    last_gpu_encode_ms: f32,
    last_gpu_fetch_ms: f32,
    last_gpu_collide_ms: f32,
    last_gpu_broadphase_ms: f32,
    last_gpu_narrowphase_ms: f32,
    last_gpu_graph_ms: f32,
    last_gpu_solve_ms: f32,
    last_gpu_integrate_ms: f32,
    last_gpu_prepare_ms: f32,
    last_gpu_device_ms: f32,
    last_timestamp_step: u64,
    physics_step: u64,
    pose_export_live: bool,
    gpu_fail: Option<std::ffi::CString>,
    physics_invalid: bool,
    topology_dirty: bool,
    cached_topology: Option<TopologyBounds>,
    scene_capabilities: Option<SceneCapabilities>,
    query_topology: u64,
    query_state: u64,
    query_index: Option<QueryIndex>,
    last_query_profile: super::QueryProfile,
    diagnostic_flags_override: Option<u32>,
    component_tgs_requested: bool,
    gpu_ccd_requested: bool,
    gpu_resident_requested: bool,
    automatic_pose_snapshots: bool,
    gpu_idle_requested: bool,
    gpu_ccd_cache_key: Option<(u64, bool, bool, bool)>,
    post_ccd_pending: bool,
    events_pending: bool,
}

fn mark_scene_dirty(w: &mut WorldInner) {
    w.scene_dirty = true;
    w.topology_dirty = true;
    w.query_topology = w.query_topology.saturating_add(1);
}

static WORLDS: Mutex<Vec<Option<WorldInner>>> = Mutex::new(Vec::new());

fn lock_worlds() -> std::sync::MutexGuard<'static, Vec<Option<WorldInner>>> {
    WORLDS.lock().unwrap_or_else(|e| e.into_inner())
}

pub fn b3_create_world(gpu: GpuDevice, def: &WorldDef) -> WorldId {
    let mut worlds = lock_worlds();
    let inner = WorldInner {
        user_data: 0,
        generation: 1,
        def: *def,
        enable_contacts: true,
        custom_filter_callback: None,
        custom_filter_context: 0,
        pre_solve_callback: None,
        pre_solve_context: 0,
        friction_callback: None,
        restitution_callback: None,
        bodies: Vec::new(),
        free_bodies: Vec::new(),
        body_generations: Vec::new(),
        shapes: Vec::new(),
        joints: Vec::new(),
        joint_meta: Vec::new(),
        sim: None,
        gpu,
        scene_dirty: true,
        bodies_dirty: false,
        pending_fat_transforms: HashMap::new(),
        jacobi: false,
        last_substep_h: FIXED_DT / DEFAULT_SUB_STEPS as f32,
        reaction_inv_h: 0.0,
        contact_recycle_distance: 0.05,
        contact_registry: HashMap::new(),
        contact_by_id: HashMap::new(),
        contact_by_body: HashMap::new(),
        contact_by_shape: HashMap::new(),
        contact_snapshot_key: None,
        contact_snapshot_copies: 0,
        live_contacts: HashMap::new(),
        contact_shape_ids: std::sync::Arc::default(),
        previous_contact_shape_ids: std::sync::Arc::default(),
        contact_child_ordinals: Arc::default(),
        previous_contact_child_ordinals: Arc::default(),
        contact_shape_revision: u64::MAX,
        contact_begin_events: Vec::new(),
        contact_end_events: Vec::new(),
        contact_hit_events: Vec::new(),
        deferred_contact_end_events: Vec::new(),
        sensor_overlaps: HashMap::new(),
        sensor_begin_events: Vec::new(),
        sensor_end_events: Vec::new(),
        deferred_sensor_end_events: Vec::new(),
        continuous_sensor_hits: Vec::new(),
        step_start_bodies: Vec::new(),
        body_move_events: Vec::new(),
        joint_events: Vec::new(),
        gpu_mirror_stale: false,
        last_gpu_encode_ms: 0.0,
        last_gpu_fetch_ms: 0.0,
        last_gpu_collide_ms: 0.0,
        last_gpu_broadphase_ms: 0.0,
        last_gpu_narrowphase_ms: 0.0,
        last_gpu_graph_ms: 0.0,
        last_gpu_solve_ms: 0.0,
        last_gpu_integrate_ms: 0.0,
        last_gpu_prepare_ms: 0.0,
        last_gpu_device_ms: 0.0,
        last_timestamp_step: 0,
        physics_step: 0,
        pose_export_live: false,
        gpu_fail: None,
        physics_invalid: false,
        topology_dirty: true,
        cached_topology: None,
        scene_capabilities: None,
        query_topology: 1,
        query_state: 1,
        query_index: None,
        last_query_profile: super::QueryProfile::default(),
        diagnostic_flags_override: None,
        component_tgs_requested: std::env::var("GPU_PHYSICS_COMPONENT_TGS").as_deref()==Ok("1"),
        gpu_ccd_requested: std::env::var("GPU_PHYSICS_GPU_CCD").is_ok_and(|value| value=="1"),
        gpu_resident_requested: std::env::var("GPU_PHYSICS_RESIDENT").is_ok_and(|value| value=="0")==false,
        automatic_pose_snapshots: true,
        gpu_idle_requested: std::env::var("GPU_PHYSICS_IDLE").as_deref()!=Ok("0"),
        gpu_ccd_cache_key: None,
        post_ccd_pending: false,
        events_pending: false,
    };
    for (i, slot) in worlds.iter_mut().enumerate() {
        if slot.is_none() {
            let gen = 1u16;
            *slot = Some(inner);
            return WorldId {
                index1: (i + 1) as u16,
                generation: gen,
            };
        }
    }
    worlds.push(Some(inner));
    WorldId {
        index1: worlds.len() as u16,
        generation: 1,
    }
}

pub fn b3_destroy_world(id: WorldId) {
    if id.index1 == 0 {
        return;
    }
    let mut worlds = lock_worlds();
    let i = id.index1 as usize - 1;
    if i < worlds.len() {
        if let Some(w) = worlds[i].as_mut() {
            if let Some(sim) = w.sim.as_ref() {
                poll_until_idle(&sim.device);
                sim.save_pipeline_cache();
            }
        }
        worlds[i] = None;
    }
}

pub fn b3_world_is_valid(id: WorldId) -> bool {
    if id.index1 == 0 {
        return false;
    }
    let worlds = lock_worlds();
    let i = id.index1 as usize - 1;
    worlds
        .get(i)
        .and_then(|s| s.as_ref())
        .is_some_and(|w| w.generation == id.generation)
}

pub fn b3_world_set_contacts(id: WorldId, enable: bool) {
    with_world_mut(id, |w| w.enable_contacts = enable);
}

pub fn b3_world_set_custom_filter_callback(
    id: WorldId,
    callback: Option<CustomFilterCallback>,
    context: *mut std::ffi::c_void,
) {
    with_world_mut(id, |w| {
        w.custom_filter_callback = callback;
        w.custom_filter_context = context as usize;
    });
}

pub fn b3_world_set_pre_solve_callback(
    id: WorldId,
    callback: Option<PreSolveCallback>,
    context: *mut std::ffi::c_void,
) {
    with_world_mut(id, |w| {
        w.pre_solve_callback = callback;
        w.pre_solve_context = context as usize;
    });
}

pub fn b3_world_enable_sleeping(id: WorldId, enable: bool) {
    with_world_mut(id, |w| {
        w.def.enable_sleep = enable;
        if let Some(sim) = w.sim.as_mut() {
            sim.set_sleep_enabled(enable);
        }
    });
}

pub fn b3_world_is_sleeping_enabled(id: WorldId) -> bool {
    with_world(id, |w| w.def.enable_sleep).unwrap_or(false)
}

pub fn b3_world_set_gravity(id: WorldId, gravity: [f32; 3]) {
    with_world_mut(id, |w| {
        w.def.gravity = gravity;
    });
}

pub fn b3_world_get_gravity(id: WorldId) -> [f32; 3] {
    with_world(id, |w| w.def.gravity).unwrap_or([0.0, -10.0, 0.0])
}

pub fn b3_world_enable_continuous(id: WorldId, enable: bool) {
    with_world_mut(id, |w| w.def.enable_continuous = enable);
}

pub fn b3_world_is_continuous_enabled(id: WorldId) -> bool {
    with_world(id, |w| w.def.enable_continuous).unwrap_or(false)
}

pub fn b3_world_set_friction_callback(id: WorldId, callback: Option<FrictionCallback>) {
    with_world_mut(id, |w| {
        w.friction_callback = callback;
        mark_scene_dirty(w);
    });
}

pub fn b3_world_set_restitution_callback(id: WorldId, callback: Option<RestitutionCallback>) {
    with_world_mut(id, |w| {
        w.restitution_callback = callback;
        mark_scene_dirty(w);
    });
}

/// Host-evaluated friction/restitution callbacks, hashed into a 4096-slot table.
fn build_mix_pairs(w: &WorldInner, shapes: &[ShapeGpu]) -> Result<Vec<MixPairGpu>, &'static str> {
    let mut table = vec![MixPairGpu::zeroed(); crate::types::MIX_PAIR_CAP as usize];
    if w.friction_callback.is_none() && w.restitution_callback.is_none() {
        return Ok(table);
    }
    if !callback_mix_pairs_supported(shapes.len() as u32) {
        return Err("material callbacks exceed the supported 4096 shape-pair table");
    }
    if shapes.iter().any(|shape| shape.kind == KIND_MESH) {
        return Err("material callbacks on mesh/height-field triangles are not supported");
    }
    let cap = crate::types::MIX_PAIR_CAP as usize;
    for (ia, a) in shapes.iter().enumerate() {
        for (ib, b) in shapes.iter().enumerate().skip(ia) {
            let lo = ia.min(ib) as u32;
            let hi = ia.max(ib) as u32;
            let key = [lo, hi];
            let material_a = u64::from(a.user_material_id_lo) | (u64::from(a.user_material_id_hi) << 32);
            let material_b = u64::from(b.user_material_id_lo) | (u64::from(b.user_material_id_hi) << 32);
            let friction = mix_friction(w.friction_callback, a.friction, material_a, b.friction, material_b);
            let restitution =
                mix_restitution(w.restitution_callback, a.restitution, material_a, b.restitution, material_b);
            let mut slot = (crate::types::pair_hash_mix(crate::types::pair_hash_mix(lo) ^ hi) as usize) & (cap - 1);
            let mut stored = false;
            for _ in 0..32 {
                if table[slot].occupied == 0 || table[slot].key == key {
                    table[slot] = MixPairGpu {
                        key,
                        friction,
                        restitution,
                        occupied: 1,
                        _pad: [0;3],
                    };
                    stored = true;
                    break;
                }
                slot = (slot + 1) & (cap - 1);
            }
            if !stored {
                return Err("material callback table exceeded the 32-probe shader lookup bound");
            }
        }
    }
    Ok(table)
}

pub const CALLBACK_MIX_PAIR_CAP: u32 = 4096;

pub fn callback_mix_pairs_supported(shape_count: u32) -> bool {
    let n = u64::from(shape_count);
    n.saturating_mul(n.saturating_add(1)) / 2 <= u64::from(CALLBACK_MIX_PAIR_CAP)
}

pub fn mix_friction(
    callback: Option<FrictionCallback>,
    friction_a: f32,
    material_a: u64,
    friction_b: f32,
    material_b: u64,
) -> f32 {
    if let Some(callback) = callback {
        unsafe { callback(friction_a, material_a, friction_b, material_b) }
    } else {
        (friction_a * friction_b).max(0.0).sqrt()
    }
}

pub fn mix_restitution(
    callback: Option<RestitutionCallback>,
    restitution_a: f32,
    material_a: u64,
    restitution_b: f32,
    material_b: u64,
) -> f32 {
    if let Some(callback) = callback {
        unsafe { callback(restitution_a, material_a, restitution_b, material_b) }
    } else {
        restitution_a.max(restitution_b)
    }
}

pub fn b3_world_set_jacobi(id: WorldId, jacobi: bool) {
    with_world_mut(id, |w| {
        w.jacobi = jacobi;
        w.topology_dirty = true;
        if let Some(sim) = w.sim.as_mut() {
            sim.set_solver_mode(jacobi);
        }
        apply_solver_topology(w);
    });
}

pub fn b3_create_body(world: WorldId, def: &BodyDef) -> BodyId {
    with_world_mut(world, |w| {
        let static_body = def.body_type == BodyType::Static;
        let kinematic_body = def.body_type == BodyType::Kinematic;
        let gpu = BodyGpu {
            pos: def.position,
            inv_mass: 0.0,
            vel: if static_body { [0.0; 3] } else { def.linear_velocity },
            kind: KIND_SPHERE,
            half: [0.0; 3],
            flags: (if static_body { FLAG_STATIC } else { 0 })
                | (if kinematic_body { FLAG_KINEMATIC } else { 0 })
                | (if def.enable_sleep {
                    FLAG_SLEEP_ENABLED
                } else {
                    0
                })
                | (if !def.is_awake && def.enable_sleep && !static_body && !kinematic_body {
                    FLAG_SLEEP
                } else {
                    0
                })
                | (if def.is_bullet { FLAG_BULLET } else { 0 })
                | (if def.allow_fast_rotation {
                    FLAG_ALLOW_FAST_ROTATION
                } else {
                    0
                })
                | (if def.enable_contact_recycling { 0 } else { FLAG_DISABLE_CONTACT_RECYCLING })
                | def.motion_locks.to_flags(),
            rot: def.rotation,
            omega: if static_body { [0.0; 3] } else { def.angular_velocity },
            restitution: 0.0,
            inv_inertia: [0.0; 3],
            friction: 0.6,
            gravity_scale: def.gravity_scale,
            linear_damping: def.linear_damping,
            angular_damping: def.angular_damping,
            rolling: 0.0,
            dp: [0.0; 3],
            sleep_time: 0.0,
            dq: [0.0, 0.0, 0.0, 1.0],
            island_id: u32::MAX,
            sleep_velocity: 0.0,
            sleep_threshold: 0.05,
            _pad_island: 0,
        };
        let epoch = snapshot_epoch(w);
        let mut cpu = CpuBody {
            name: None,
            sleep_threshold: 0.05,
            generation: 1,
            user_data: def.user_data,
            gpu,
            mass: 0.0,
            local_center: [0.0; 3],
            local_inertia: [0.0; 6],
            has_shape: false,
            shape_indices: Vec::new(),
            axis: [0.0; 3],
            min_extent: f32::MAX,
            max_extent: 0.0,
            force: [0.0; 3],
            torque: [0.0; 3],
            host_epoch: epoch.saturating_add(1),
        };
        if let Some(i) = w.free_bodies.pop() {
            if i >= crate::types::MAX_BODY_SLOTS as usize {
                w.free_bodies.push(i);
                w.gpu_fail = std::ffi::CString::new(
                    "body slot exceeds 30-bit graph classification range",
                )
                .ok();
                return b3_null_body_id();
            }
            if let Some(slot) = w.bodies.get_mut(i) {
                let generation = w.body_generations[i].wrapping_add(1);
                w.body_generations[i] = generation;
                cpu.generation = generation;
                *slot = Some(cpu);
                mark_scene_dirty(w);
                return BodyId {
                    index1: (i + 1) as i32,
                    world0: world.index1,
                    generation,
                };
            }
        }
        if w.bodies.len() >= crate::types::MAX_BODY_SLOTS as usize {
            w.gpu_fail = std::ffi::CString::new(
                "body count exceeds 30-bit graph classification range",
            )
            .ok();
            return b3_null_body_id();
        }
        w.bodies.push(Some(cpu));
        w.body_generations.push(1);
        mark_scene_dirty(w);
        BodyId {
            index1: w.bodies.len() as i32,
            world0: world.index1,
            generation: 1,
        }
    })
    .unwrap_or_else(b3_null_body_id)
}

fn quat_rotate(q: [f32; 4], v: [f32; 3]) -> [f32; 3] {
    // Box3D b3RotateVector: preserve vectors along the rotation axis even
    // when a stored unit quaternion's norm rounds slightly away from one.
    let cross = |a: [f32; 3], b: [f32; 3]| [
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    ];
    let u = [q[0], q[1], q[2]];
    let t1 = cross(u, v);
    let t2 = [t1[0] + q[3] * v[0], t1[1] + q[3] * v[1], t1[2] + q[3] * v[2]];
    let t3 = cross(u, t2);
    [v[0] + 2.0 * t3[0], v[1] + 2.0 * t3[1], v[2] + 2.0 * t3[2]]
}

fn quat_mul(a: [f32; 4], b: [f32; 4]) -> [f32; 4] {
    [
        a[3] * b[0] + b[3] * a[0] + a[1] * b[2] - a[2] * b[1],
        a[3] * b[1] + b[3] * a[1] + a[2] * b[0] - a[0] * b[2],
        a[3] * b[2] + b[3] * a[2] + a[0] * b[1] - a[1] * b[0],
        a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2],
    ]
}

fn vec3_length(v: [f32; 3]) -> f32 {
    (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]).sqrt()
}

fn joint_reaction_impulses(bodies: &[Option<CpuBody>], joint: &JointGpu) -> (f32, f32) {
    match joint.kind {
        JOINT_PARALLEL => (
            0.0,
            (joint.perp_impulse[0] * joint.perp_impulse[0]
                + joint.perp_impulse[1] * joint.perp_impulse[1])
                .sqrt(),
        ),
        JOINT_DISTANCE => (
            (joint.impulse + joint.lower_impulse - joint.upper_impulse + joint.motor_impulse).abs(),
            0.0,
        ),
        JOINT_MOTOR => (
            vec3_length([
                joint.impulse + joint.spring_impulse,
                joint.perp_impulse[0] + joint.lower_impulse,
                joint.perp_impulse[1] + joint.upper_impulse,
            ]),
            vec3_length([
                joint.angular_impulse[0] + joint.motor_impulse,
                joint.angular_impulse[1] + joint._pad2[0],
                joint.angular_impulse[2] + joint._pad2[1],
            ]),
        ),
        JOINT_PRISMATIC => (
            vec3_length([
                joint.motor_impulse + joint.lower_impulse - joint.upper_impulse,
                joint.perp_impulse[0],
                joint.perp_impulse[1],
            ]),
            vec3_length(joint.angular_impulse),
        ),
        JOINT_REVOLUTE => (
            vec3_length(joint.angular_impulse),
            vec3_length([
                joint.perp_impulse[0],
                joint.perp_impulse[1],
                joint.motor_impulse + joint.lower_impulse - joint.upper_impulse,
            ]),
        ),
        JOINT_SPHERICAL => {
            let qa = bodies
                .get(joint.a as usize)
                .and_then(Option::as_ref)
                .map(|body| quat_mul(body.gpu.rot, joint.frame_a_rotation))
                .unwrap_or([0.0, 0.0, 0.0, 1.0]);
            let qb = bodies
                .get(joint.b as usize)
                .and_then(Option::as_ref)
                .map(|body| quat_mul(body.gpu.rot, joint.frame_b_rotation))
                .unwrap_or([0.0, 0.0, 0.0, 1.0]);
            let cone_axis = quat_rotate(qa, [0.0, 0.0, 1.0]);
            let twist_axis = quat_rotate(qb, [0.0, 0.0, 1.0]);
            let cross = [
                cone_axis[1] * twist_axis[2] - cone_axis[2] * twist_axis[1],
                cone_axis[2] * twist_axis[0] - cone_axis[0] * twist_axis[2],
                cone_axis[0] * twist_axis[1] - cone_axis[1] * twist_axis[0],
            ];
            let cross_length = vec3_length(cross);
            let swing_axis = if cross_length > 0.0 {
                cross.map(|value| value / cross_length)
            } else {
                [0.0; 3]
            };
            let twist_impulse = joint.lower_impulse - joint.upper_impulse;
            let angular = [
                joint.spring_angular_impulse[0]
                    + joint.motor_angular_impulse[0]
                    + twist_impulse * twist_axis[0]
                    + joint.swing_impulse * swing_axis[0],
                joint.spring_angular_impulse[1]
                    + joint.motor_angular_impulse[1]
                    + twist_impulse * twist_axis[1]
                    + joint.swing_impulse * swing_axis[1],
                joint.spring_angular_impulse[2]
                    + joint.motor_angular_impulse[2]
                    + twist_impulse * twist_axis[2]
                    + joint.swing_impulse * swing_axis[2],
            ];
            (vec3_length(joint.angular_impulse), vec3_length(angular))
        }
        JOINT_WELD => (
            vec3_length(joint.weld_linear_impulse),
            vec3_length(joint.weld_angular_impulse),
        ),
        JOINT_WHEEL => {
            let axial = joint.spring_impulse + joint.lower_impulse - joint.upper_impulse;
            (
                vec3_length([joint.perp_impulse[0], joint.perp_impulse[1], axial]),
                joint.motor_impulse.abs(),
            )
        }
        _ => (0.0, 0.0),
    }
}

fn collect_joint_events(w: &mut WorldInner, world0: u16) {
    let inv_h = if w.last_substep_h > 0.0 {
        1.0 / w.last_substep_h
    } else {
        0.0
    };
    for (index, joint) in w.joints.iter().enumerate() {
        let Some(meta) = w.joint_meta.get(index).and_then(|meta| *meta) else {
            continue;
        };
        if joint.kind == JOINT_NONE
            || joint.kind == JOINT_FILTER
            || (meta.force_threshold == f32::MAX && meta.torque_threshold == f32::MAX)
        {
            continue;
        }
        let awake = [joint.a, joint.b].into_iter().any(|body_index| {
            w.bodies
                .get(body_index as usize)
                .and_then(Option::as_ref)
                .is_some_and(|body| {
                    body.gpu.flags & FLAG_STATIC == 0 && body.gpu.flags & FLAG_SLEEP == 0
                })
        });
        if !awake {
            continue;
        }
        let (linear_impulse, angular_impulse) = joint_reaction_impulses(&w.bodies, joint);
        let force = linear_impulse * inv_h;
        let torque = angular_impulse * inv_h;
        if force >= meta.force_threshold || torque >= meta.torque_threshold {
            w.joint_events.push(JointEvent {
                joint_id: JointId {
                    index1: index as i32 + 1,
                    world0,
                    generation: meta.generation,
                },
                user_data: meta.user_data,
            });
        }
    }
}

// Native b3MakeBoxHull stores unit-density central inertia, then shape mass
// multiplies by density. Do not round-trip through inverse inertia here: that
// changes the compound tensor by an ulp and alters sensitive gyro trajectories.
fn box_central_inertia(half: [f32; 3], density: f32) -> [f32; 6] {
    let volume = box_mass(half, 1.0);
    let d = half.map(|h| h - (-h));
    [
        density * (volume * (d[1] * d[1] + d[2] * d[2]) / 12.0),
        density * (volume * (d[0] * d[0] + d[2] * d[2]) / 12.0),
        density * (volume * (d[0] * d[0] + d[1] * d[1]) / 12.0),
        0.0, 0.0, 0.0,
    ]
}

fn invert_symmetric(m: [f32; 6]) -> [f32; 6] {
    let [a, b, c, d, e, f] = m;
    let det = a * (b * c - f * f) - d * (d * c - e * f) + e * (d * f - b * e);
    if det.abs() <= 1e-12 {
        return [0.0; 6];
    }
    let inv_det = 1.0 / det;
    [
        (b * c - f * f) * inv_det,
        (a * c - e * e) * inv_det,
        (a * b - d * d) * inv_det,
        (e * f - d * c) * inv_det,
        (d * f - b * e) * inv_det,
        (d * e - a * f) * inv_det,
    ]
}

fn body_origin(cpu: &CpuBody) -> [f32; 3] {
    let offset = quat_rotate(cpu.gpu.rot, cpu.local_center);
    [
        cpu.gpu.pos[0] - offset[0],
        cpu.gpu.pos[1] - offset[1],
        cpu.gpu.pos[2] - offset[2],
    ]
}

fn snapshot_epoch(w: &WorldInner) -> u64 {
    w.sim.as_ref().map(|sim| sim.pose_snapshot_epoch()).unwrap_or(0)
}

fn apply_body_mass_from_shapes_inner(w: &mut WorldInner, body: BodyId) {
    let mut mass = 0.0f32;
    let mut weighted_center = [0.0f32; 3];
    for shape in body_shapes(w, body) {
        if shape.body_index != body.index1 || shape.mass <= 0.0 {
            continue;
        }
        mass += shape.mass;
        for (sum, center) in weighted_center.iter_mut().zip(shape.local_center) {
            *sum += shape.mass * center;
        }
    }
    let center = if mass > 0.0 { weighted_center.map(|value| value / mass) } else { [0.0; 3] };
    let mut inertia = [0.0f32; 6];
    for shape in body_shapes(w, body) {
        if shape.body_index != body.index1 || shape.mass <= 0.0 {
            continue;
        }
        let d = [
            shape.local_center[0] - center[0],
            shape.local_center[1] - center[1],
            shape.local_center[2] - center[2],
        ];
        inertia[0] += shape.local_inertia[0] + shape.mass * (d[1] * d[1] + d[2] * d[2]);
        inertia[1] += shape.local_inertia[1] + shape.mass * (d[0] * d[0] + d[2] * d[2]);
        inertia[2] += shape.local_inertia[2] + shape.mass * (d[0] * d[0] + d[1] * d[1]);
        inertia[3] += shape.local_inertia[3] - shape.mass * d[0] * d[1];
        inertia[4] += shape.local_inertia[4] - shape.mass * d[0] * d[2];
        inertia[5] += shape.local_inertia[5] - shape.mass * d[1] * d[2];
    }

    let Some(cpu) = body_mut(w, body) else {
        return;
    };
    let old_center = cpu.gpu.pos;
    let origin = body_origin(cpu);
    let static_body = (cpu.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC)) != 0;
    let center = if static_body { [0.0; 3] } else { center };
    let inertia = if static_body { [0.0; 6] } else { inertia };
    cpu.mass = if static_body { 0.0 } else { mass };
    cpu.local_center = center;
    cpu.local_inertia = inertia;
    let world_center = quat_rotate(cpu.gpu.rot, center);
    cpu.gpu.pos = [
        origin[0] + world_center[0],
        origin[1] + world_center[1],
        origin[2] + world_center[2],
    ];
    let center_shift = [
        cpu.gpu.pos[0] - old_center[0],
        cpu.gpu.pos[1] - old_center[1],
        cpu.gpu.pos[2] - old_center[2],
    ];
    let angular = cpu.gpu.omega;
    cpu.gpu.vel[0] += angular[1] * center_shift[2] - angular[2] * center_shift[1];
    cpu.gpu.vel[1] += angular[2] * center_shift[0] - angular[0] * center_shift[2];
    cpu.gpu.vel[2] += angular[0] * center_shift[1] - angular[1] * center_shift[0];
    if static_body {
        cpu.gpu.inv_mass = 0.0;
        cpu.gpu.inv_inertia = [0.0; 3];
    } else {
        cpu.gpu.inv_mass = if mass > 0.0 { 1.0 / mass } else { 0.0 };
        let inverse = invert_symmetric(inertia);
        cpu.gpu.inv_inertia = [inverse[0], inverse[1], inverse[2]];
    }
    mark_scene_dirty(w);
    update_body_extents(w, body);
}

fn body_shapes(w: &WorldInner, body: BodyId) -> impl Iterator<Item = &CpuShape> {
    body_ref(w, body).into_iter().flat_map(|b| b.shape_indices.iter())
        .filter_map(|&index| w.shapes.get(index).and_then(Option::as_ref))
}

fn update_body_extents(w: &mut WorldInner, body: BodyId) {
    let indices = body_ref(w, body).map(|b| b.shape_indices.clone()).unwrap_or_default();
    update_body_extents_indices(w, body, indices, true);
}

fn update_body_extents_range(w: &mut WorldInner, body: BodyId, range: std::ops::Range<usize>, reset: bool) {
    update_body_extents_indices(w, body, range, reset);
}

fn update_body_extents_indices(w: &mut WorldInner, body: BodyId, indices: impl IntoIterator<Item = usize>, reset: bool) {
    let Some((local_center, previous_min, previous_max)) = body_ref(w, body)
        .map(|body| (body.local_center, body.min_extent, body.max_extent)) else {
        return;
    };
    let center = glam::Vec3::from_array(local_center);
    let mut min_extent = if reset { f32::MAX } else { previous_min };
    let mut max_extent = if reset { 0.0 } else { previous_max };
    for shape in indices.into_iter()
        .filter_map(|index| w.shapes.get(index).and_then(Option::as_ref))
        .filter(|shape| shape.body_index == body.index1)
    {
        let radius = if matches!(shape.kind, KIND_SPHERE | KIND_CAPSULE) {
            shape.half[0]
        } else {
            0.0
        };
        let shape_min = match shape.kind {
            KIND_SPHERE | KIND_CAPSULE => radius,
            KIND_BOX => shape.half.into_iter().fold(f32::MAX, f32::min),
            KIND_CONVEX_HULL => shape.inner_radius,
            _ => f32::MAX,
        };
        min_extent = min_extent.min(shape_min.max(LINEAR_SLOP));
        let points: Vec<[f32; 3]> = match shape.kind {
            KIND_SPHERE => vec![shape.geometry_center],
            KIND_CAPSULE => vec![
                [
                    shape.geometry_center[0] - shape.axis[0],
                    shape.geometry_center[1] - shape.axis[1],
                    shape.geometry_center[2] - shape.axis[2],
                ],
                [
                    shape.geometry_center[0] + shape.axis[0],
                    shape.geometry_center[1] + shape.axis[1],
                    shape.geometry_center[2] + shape.axis[2],
                ],
            ],
            KIND_BOX => vec![[
                local_center[0]
                    + (shape.geometry_center[0] - local_center[0]).abs()
                    + shape.half[0],
                local_center[1]
                    + (shape.geometry_center[1] - local_center[1]).abs()
                    + shape.half[1],
                local_center[2]
                    + (shape.geometry_center[2] - local_center[2]).abs()
                    + shape.half[2],
            ]],
            KIND_CONVEX_HULL => shape.hull_points.clone(),
            KIND_MESH => shape.mesh_vertices.as_ref().clone(),
            _ => Vec::new(),
        };
        for point in points {
            max_extent = max_extent.max((glam::Vec3::from_array(point) - center).length() + radius);
        }
    }
    if let Some(cpu) = body_mut(w, body) {
        cpu.min_extent = min_extent;
        cpu.max_extent = max_extent;
    }
}

fn attach_shape(
    w: &mut WorldInner,
    body: BodyId,
    world: WorldId,
    kind: u32,
    half: [f32; 3],
    local: [f32; 3],
    geometry_center: [f32; 3],
    axis: [f32; 3],
    mass: f32,
    local_inertia: [f32; 6],
    inner_radius: f32,
    hull_points: Vec<[f32; 3]>,
    hull_planes: Vec<[f32; 4]>,
    hull_edges: Vec<[f32; 3]>,
    hull_topology: Vec<[u32; 4]>,
    def: &ShapeDef,
) -> ShapeId {
    let Some(cpu) = body_mut(w, body) else {
        return b3_null_shape_id();
    };
    let first_shape = !cpu.has_shape;
    if first_shape {
        // Keep the first shape mirrored for rendering. Collision uses the
        // shape table, so additional shapes no longer create welded bodies.
        cpu.gpu.kind = kind;
        cpu.gpu.half = half;
        cpu.gpu.restitution = def.restitution;
        cpu.gpu.friction = def.friction;
        cpu.gpu.rolling = def.rolling_resistance;
        cpu.has_shape = true;
        cpu.axis = axis;
    }
    mark_scene_dirty(w);
    let shape = push_shape(
        w,
        body,
        world,
        kind,
        half,
        axis,
        mass,
        local,
        geometry_center,
        local_inertia,
        inner_radius,
        hull_points,
        hull_planes,
        hull_edges,
        hull_topology,
        def,
    );
    if def.update_body_mass && first_shape {
        if let Some(cpu) = body_mut(w, body) {
            let old_center = cpu.gpu.pos;
            let origin = body_origin(cpu);
            let static_body = (cpu.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC)) != 0;
            let local = if static_body || mass == 0.0 { [0.0; 3] } else { local };
            cpu.mass = if static_body { 0.0 } else { mass };
            cpu.local_center = local;
            cpu.local_inertia = if static_body { [0.0; 6] } else { local_inertia };
            let world_center = quat_rotate(cpu.gpu.rot, local);
            cpu.gpu.pos = [
                origin[0] + world_center[0],
                origin[1] + world_center[1],
                origin[2] + world_center[2],
            ];
            let center_shift = [
                cpu.gpu.pos[0] - old_center[0],
                cpu.gpu.pos[1] - old_center[1],
                cpu.gpu.pos[2] - old_center[2],
            ];
            let angular = cpu.gpu.omega;
            cpu.gpu.vel[0] += angular[1] * center_shift[2] - angular[2] * center_shift[1];
            cpu.gpu.vel[1] += angular[2] * center_shift[0] - angular[0] * center_shift[2];
            cpu.gpu.vel[2] += angular[0] * center_shift[1] - angular[1] * center_shift[0];
            if static_body || mass <= 1e-8 {
                cpu.gpu.inv_mass = 0.0;
                cpu.gpu.inv_inertia = [0.0; 3];
            } else {
                cpu.gpu.inv_mass = 1.0 / mass;
                let inverse = invert_symmetric(local_inertia);
                cpu.gpu.inv_inertia = [inverse[0], inverse[1], inverse[2]];
            }
        }
        update_body_extents(w, body);
    } else if def.update_body_mass {
        // Recomputing the COM also recomputes every shape's extent about it.
        apply_body_mass_from_shapes_inner(w, body);
    } else if first_shape {
        // A compound proxy may already exist even before the first collider.
        update_body_extents(w, body);
    } else if shape.index1 > 0 {
        // Append with unchanged COM: min/max can only extend. Avoid rescanning
        // every prior compound child (and copying all its geometry) per append.
        let index = shape.index1 as usize - 1;
        update_body_extents_range(w, body, index..index + 1, first_shape);
    }
    shape
}

pub fn b3_create_sphere_shape(body: BodyId, def: &ShapeDef, sphere: &Sphere) -> ShapeId {
    let world = world_id_from_body(body);
    with_world_mut(world, |w| {
        let r = sphere.radius.max(1e-6);
        let mass = sphere_mass(r, def.density);
        let i = 0.4 * mass * r * r;
        attach_shape(
            w,
            body,
            world,
            KIND_SPHERE,
            [r, r, r],
            sphere.center,
            sphere.center,
            [0.0; 3],
            mass,
            [i, i, i, 0.0, 0.0, 0.0],
            r,
            Vec::new(),
            Vec::new(),
            Vec::new(),
            Vec::new(),
            def,
        )
    })
    .unwrap_or_else(b3_null_shape_id)
}

pub fn b3_create_hull_shape(body: BodyId, def: &ShapeDef, hull: &BoxHull) -> ShapeId {
    let world = world_id_from_body(body);
    with_world_mut(world, |w| {
        let mass = box_mass(hull.half_extents, def.density);
        attach_shape(
            w,
            body,
            world,
            KIND_BOX,
            hull.half_extents,
            hull.center,
            hull.center,
            [0.0; 3],
            mass,
            box_central_inertia(hull.half_extents, def.density),
            0.0,
            Vec::new(),
            Vec::new(),
            Vec::new(),
            Vec::new(),
            def,
        )
    })
    .unwrap_or_else(b3_null_shape_id)
}

pub fn b3_create_convex_hull_shape(body: BodyId, def: &ShapeDef, hull: &ConvexHull) -> ShapeId {
    let world = world_id_from_body(body);
    with_world_mut(world, |w| {
        let mass = hull.volume * def.density;
        let inertia = hull
            .central_inertia
            .map(|component| component * def.density);
        let id = attach_shape(
            w,
            body,
            world,
            KIND_CONVEX_HULL,
            hull.half_extents,
            hull.center,
            hull.aabb_center,
            [0.0; 3],
            mass,
            inertia,
            hull.inner_radius,
            hull.points.clone(),
            hull.planes.clone(),
            hull.edge_directions.clone(),
            hull.half_edges.clone(),
            def,
        );
        if let Some(shape) = w.shapes.get_mut(id.index1.saturating_sub(1) as usize).and_then(Option::as_mut) {
            shape.unit_mass = hull.volume;
            shape.unit_inertia = hull.central_inertia;
        }
        id
    })
    .unwrap_or_else(b3_null_shape_id)
}

pub fn b3_create_capsule_shape(body: BodyId, def: &ShapeDef, capsule: &Capsule) -> ShapeId {
    let world = world_id_from_body(body);
    with_world_mut(world, |w| {
        let dx = capsule.center2[0] - capsule.center1[0];
        let dy = capsule.center2[1] - capsule.center1[1];
        let dz = capsule.center2[2] - capsule.center1[2];
        let h = (dx * dx + dy * dy + dz * dz).sqrt();
        let half_len = 0.5 * h;
        let r = capsule.radius;
        let mass_data =
            crate::types::compute_capsule_mass(capsule.center1, capsule.center2, r, def.density);
        attach_shape(
            w,
            body,
            world,
            KIND_CAPSULE,
            [r, half_len, r],
            mass_data.center,
            mass_data.center,
            [0.5 * dx, 0.5 * dy, 0.5 * dz],
            mass_data.mass,
            mass_data.inertia,
            r,
            Vec::new(),
            Vec::new(),
            Vec::new(),
            Vec::new(),
            def,
        )
    })
    .unwrap_or_else(b3_null_shape_id)
}

pub fn b3_create_mesh_shape(
    body: BodyId,
    def: &ShapeDef,
    vertices: &[[f32; 3]],
    triangles: &[[u32; 3]],
    flags: &[u8],
    materials: &[u8],
    nodes: &[MeshNode],
    scale: [f32; 3],
    ) -> ShapeId {
    let _ = nodes;
    if vertices.is_empty()
        || triangles.is_empty()
        || scale
            .iter()
            .any(|value| !value.is_finite() || value.abs() < 1.0e-6)
        || triangles
            .iter()
            .flatten()
            .any(|&index| index as usize >= vertices.len())
    {
        return b3_null_shape_id();
    }
    let world = world_id_from_body(body);
    with_world_mut(world, |w| {
        let scaled: Vec<[f32; 3]> = vertices
            .iter()
            .map(|p| [p[0] * scale[0], p[1] * scale[1], p[2] * scale[2]])
            .collect();
        let mut lower = [f32::MAX; 3];
        let mut upper = [f32::MIN; 3];
        for point in &scaled {
            for axis in 0..3 {
                lower[axis] = lower[axis].min(point[axis]);
                upper[axis] = upper[axis].max(point[axis]);
            }
        }
        let center = [
            0.5 * (lower[0] + upper[0]),
            0.5 * (lower[1] + upper[1]),
            0.5 * (lower[2] + upper[2]),
        ];
        let half = [
            (0.5 * (upper[0] - lower[0])).max(0.05),
            (0.5 * (upper[1] - lower[1])).max(0.05),
            (0.5 * (upper[2] - lower[2])).max(0.05),
        ];
        let inverted = scale[0] * scale[1] * scale[2] < 0.0;
        let mut packed_triangles: Vec<[u32; 4]> = triangles
            .iter()
            .enumerate()
            .map(|(index, triangle)| {
                let source_flags = flags.get(index).copied().unwrap_or(0);
                let edge_flags = if inverted {
                    ((source_flags & 0x10) >> 4)
                        | ((source_flags & 0x20) >> 4)
                        | ((source_flags & 0x40) >> 4)
                } else {
                    source_flags
                };
                let material = materials.get(index).copied().unwrap_or(0);
                if inverted {
                    [
                        triangle[0],
                        triangle[2],
                        triangle[1],
                        u32::from(edge_flags) | (u32::from(material) << 8),
                    ]
                } else {
                    [
                        triangle[0],
                        triangle[1],
                        triangle[2],
                        u32::from(edge_flags) | (u32::from(material) << 8),
                    ]
                }
            })
            .collect();
        let mut packed_ids: Vec<i32> = (0..packed_triangles.len())
            .map(|index| i32::try_from(index).unwrap_or(i32::MAX))
            .collect();
        let scaled_nodes: Vec<MeshNode> = build_triangle_bvh(&scaled, &mut packed_triangles, &mut packed_ids)
            .unwrap_or_default();
        let id = attach_shape(
            w,
            body,
            world,
            KIND_MESH,
            half,
            center,
            center,
            [0.0; 3],
            0.0,
            [0.0; 6],
            0.0,
            Vec::new(),
            Vec::new(),
            Vec::new(),
            Vec::new(),
            def,
        );
        if let Some(shape) = w
            .shapes
            .get_mut(id.index1.saturating_sub(1) as usize)
            .and_then(Option::as_mut)
        {
            shape.mesh_vertices = Arc::new(scaled);
            shape.mesh_triangles = Arc::new(packed_triangles);
            shape.mesh_triangle_ids = Arc::new(packed_ids);
            shape.mesh_nodes = Arc::new(scaled_nodes);
            shape.mesh_scale = scale;
        }
        if id.index1 > 0 {
            let index = id.index1 as usize - 1;
            update_body_extents_range(w, body, index..index + 1, false);
        }
        mark_scene_dirty(w);
        id
    })
    .unwrap_or_else(b3_null_shape_id)
}

/// Clone immutable, unscaled mesh geometry with an independent affine instance.
/// The source must be a raw instance or a unit-scale mesh; baked scaled sources
/// cannot recover canonical winding/edge flags and are deliberately rejected.
pub fn b3_create_mesh_instance(
    body: BodyId,
    def: &ShapeDef,
    source: ShapeId,
    transform: WorldTransform,
    scale: [f32; 3],
) -> ShapeId {
    if body.world0 != source.world0
        || !transform
            .p
            .iter()
            .chain(transform.q.iter())
            .all(|v| v.is_finite())
        || scale.iter().any(|v| !v.is_finite() || v.abs() < 1e-6)
        || !glam::Quat::from_array(transform.q).length_squared().is_finite()
        || glam::Quat::from_array(transform.q).length_squared() < 1e-12
    {
        return b3_null_shape_id();
    }
    let world = world_id_from_body(body);
    with_world_mut(world, |w| {
        if body_ref(w, body).is_none() {
            return b3_null_shape_id();
        }
        let Some(source_index) = source.index1.checked_sub(1).map(|v| v as usize) else {
            return b3_null_shape_id();
        };
        let Some(geometry) = w.shapes.get(source_index).and_then(Option::as_ref) else {
            return b3_null_shape_id();
        };
        if geometry.generation != source.generation
            || geometry.kind != KIND_MESH
            || (geometry.mesh_instance.is_none() && geometry.mesh_scale != [1.0; 3])
        {
            return b3_null_shape_id();
        }
        let Some(root) = geometry.mesh_nodes.first().copied() else {
            return b3_null_shape_id();
        };
        let vertices = geometry.mesh_vertices.clone();
        let triangles = geometry.mesh_triangles.clone();
        let ids = geometry.mesh_triangle_ids.clone();
        let nodes = geometry.mesh_nodes.clone();
        let instance = MeshInstance {
            position: transform.p,
            rotation: glam::Quat::from_array(transform.q).normalize().to_array(),
            scale,
        };
        let mut lower = glam::Vec3::splat(f32::MAX);
        let mut upper = glam::Vec3::splat(f32::MIN);
        for x in [root.lower[0], root.upper[0]] {
            for y in [root.lower[1], root.upper[1]] {
                for z in [root.lower[2], root.upper[2]] {
                    let point = glam::Vec3::from_array(instance.point([x, y, z]));
                    if !point.is_finite() {
                        return b3_null_shape_id();
                    }
                    lower = lower.min(point);
                    upper = upper.max(point);
                }
            }
        }
        let center = (lower * 0.5 + upper * 0.5).to_array();
        let half = (upper * 0.5 - lower * 0.5)
            .max(glam::Vec3::splat(0.05))
            .to_array();
        let id = attach_shape(
            w,
            body,
            world,
            KIND_MESH,
            half,
            center,
            center,
            [0.0; 3],
            0.0,
            [0.0; 6],
            0.0,
            Vec::new(),
            Vec::new(),
            Vec::new(),
            Vec::new(),
            def,
        );
        if id.index1 == 0 {
            return id;
        }
        let shape = w.shapes[id.index1 as usize - 1].as_mut().unwrap();
        shape.mesh_vertices = vertices;
        shape.mesh_triangles = triangles;
        shape.mesh_triangle_ids = ids;
        shape.mesh_nodes = nodes;
        shape.mesh_scale = scale;
        shape.mesh_instance = Some(instance);
        if id.index1 > 0 {
            let index = id.index1 as usize - 1;
            update_body_extents_range(w, body, index..index + 1, false);
        }
        mark_scene_dirty(w);
        id
    })
    .unwrap_or_else(b3_null_shape_id)
}

pub fn b3_create_compound_parent(body: BodyId, def: &ShapeDef) -> ShapeId {
    let world = world_id_from_body(body);
    with_world_mut(world, |w| {
        let Some(cpu) = body_ref(w, body) else {
            return b3_null_shape_id();
        };
        if cpu.gpu.flags & FLAG_STATIC == 0 || def.is_sensor {
            return b3_null_shape_id();
        }
        mark_scene_dirty(w);
        let id = push_shape(
            w,
            body,
            world,
            KIND_BOX,
            [0.0; 3],
            [0.0; 3],
            0.0,
            [0.0; 3],
            [0.0; 3],
            [0.0; 6],
            0.0,
            Vec::new(),
            Vec::new(),
            Vec::new(),
            Vec::new(),
            def,
        );
        if let Some(shape) = w
            .shapes
            .get_mut(id.index1.saturating_sub(1) as usize)
            .and_then(Option::as_mut)
        {
            shape.public_kind = PUBLIC_KIND_COMPOUND;
            shape.mass = 0.0;
        }
        id
    })
    .unwrap_or_else(b3_null_shape_id)
}

pub fn b3_shape_attach_compound_child(parent: ShapeId, child: ShapeId) -> bool {
    attach_compound_child(parent, child, None, [0; 4])
}

pub fn b3_shape_attach_compound_child_indexed(parent: ShapeId, child: ShapeId, index: i32) -> bool {
    if index < 0 { return false; }
    attach_compound_child(parent, child, Some(index), [0; 4])
}

pub(crate) fn b3_shape_attach_compound_child_materials(
    parent: ShapeId,
    child: ShapeId,
    index: i32,
    materials: [i32; 4],
) -> bool {
    if index < 0 {
        return false;
    }
    attach_compound_child(parent, child, Some(index), materials)
}

fn attach_compound_child(
    parent: ShapeId,
    child: ShapeId,
    requested: Option<i32>,
    materials: [i32; 4],
) -> bool {
    if parent.index1 == 0
        || child.index1 == 0
        || parent.world0 != child.world0
        || parent.index1 == child.index1
    {
        return false;
    }
    let world = WorldId {
        index1: parent.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        let filter;
        let event_flags;
        let child_index;
        let parent_body;
        {
            let Some(parent_shape) = w
                .shapes
                .get(parent.index1.saturating_sub(1) as usize)
                .and_then(Option::as_ref)
            else {
                return false;
            };
            if parent_shape.generation != parent.generation
                || parent_shape.public_kind != PUBLIC_KIND_COMPOUND
            {
                return false;
            }
            child_index = requested.unwrap_or(parent_shape.next_compound_child_index);
            // Baked imports append in native child order. A monotonic ordinal
            // prevents duplicates/reuse without an O(scene size) scan per child.
            if child_index < parent_shape.next_compound_child_index {
                return false;
            }
            parent_body = parent_shape.body_index;
            filter = parent_shape.filter;
            event_flags = parent_shape.event_flags & !SHAPE_IS_SENSOR;
        }
        let Some(next_index) = child_index.checked_add(1) else {
            return false;
        };
        let Some(child_shape) = w
            .shapes
            .get_mut(child.index1.saturating_sub(1) as usize)
            .and_then(Option::as_mut)
        else {
            return false;
        };
        if child_shape.generation != child.generation
            || child_shape.compound_parent != 0
            || child_shape.body_index != parent_body
        {
            return false;
        }
        child_shape.compound_parent = parent.index1;
        child_shape.compound_child_index = child_index;
        child_shape.compound_material_indices = materials;
        child_shape.filter = filter;
        child_shape.event_flags = event_flags | (child_shape.event_flags & SHAPE_IS_SENSOR);
        if let Some(parent) = w.shapes[parent.index1 as usize - 1].as_mut() {
            parent.next_compound_child_index = parent.next_compound_child_index.max(next_index);
        }
        mark_scene_dirty(w);
        true
    })
    .unwrap_or(false)
}

const PUBLIC_KIND_HEIGHT_FIELD: u32 = 5;
const PUBLIC_KIND_COMPOUND: u32 = 6;
const MAX_HEIGHT_FIELD_POINTS: usize = 16_777_216;
const MAX_HEIGHT_FIELD_TRIANGLES: usize = 33_554_430;

fn triangle_bounds(
    vertices: &[[f32; 3]],
    triangle: &[u32; 4],
) -> Option<([f32; 3], [f32; 3], [f32; 3])> {
    let a = glam::Vec3::from_array(*vertices.get(triangle[0] as usize)?);
    let b = glam::Vec3::from_array(*vertices.get(triangle[1] as usize)?);
    let c = glam::Vec3::from_array(*vertices.get(triangle[2] as usize)?);
    Some((
        a.min(b).min(c).to_array(),
        a.max(b).max(c).to_array(),
        ((a + b + c) / 3.0).to_array(),
    ))
}

fn build_triangle_bvh(
    vertices: &[[f32; 3]],
    triangles: &mut Vec<[u32; 4]>,
    triangle_ids: &mut Vec<i32>,
) -> Option<Vec<MeshNode>> {
    fn build(
        vertices: &[[f32; 3]],
        source: &[[u32; 4]],
        source_ids: &[i32],
        indices: &mut [usize],
        ordered: &mut Vec<[u32; 4]>,
        ordered_ids: &mut Vec<i32>,
        nodes: &mut Vec<MeshNode>,
    ) -> Option<usize> {
        let node_index = nodes.len();
        nodes.push(MeshNode::default());
        let mut lower = [f32::MAX; 3];
        let mut upper = [f32::MIN; 3];
        let mut centroid_lower = [f32::MAX; 3];
        let mut centroid_upper = [f32::MIN; 3];
        for &index in indices.iter() {
            let (lo, hi, centroid) = triangle_bounds(vertices, source.get(index)?)?;
            for axis in 0..3 {
                lower[axis] = lower[axis].min(lo[axis]);
                upper[axis] = upper[axis].max(hi[axis]);
                centroid_lower[axis] = centroid_lower[axis].min(centroid[axis]);
                centroid_upper[axis] = centroid_upper[axis].max(centroid[axis]);
            }
        }
        if indices.len() <= 8 {
            let offset = u32::try_from(ordered.len()).ok()?;
            for &index in indices.iter() {
                ordered.push(*source.get(index)?);
                ordered_ids.push(*source_ids.get(index)?);
            }
            nodes[node_index] = MeshNode {
                lower,
                upper,
                data: (u32::try_from(indices.len()).ok()? << 2) | 3,
                triangle_offset: offset,
            };
            return Some(node_index);
        }
        let extents = [
            centroid_upper[0] - centroid_lower[0],
            centroid_upper[1] - centroid_lower[1],
            centroid_upper[2] - centroid_lower[2],
        ];
        let axis = if extents[1] > extents[0] && extents[1] >= extents[2] {
            1
        } else if extents[2] > extents[0] {
            2
        } else {
            0
        };
        indices.sort_unstable_by(|a, b| {
            let ca = triangle_bounds(vertices, &source[*a])
                .map(|value| value.2[axis])
                .unwrap_or(0.0);
            let cb = triangle_bounds(vertices, &source[*b])
                .map(|value| value.2[axis])
                .unwrap_or(0.0);
            ca.total_cmp(&cb).then_with(|| a.cmp(b))
        });
        let middle = indices.len() / 2;
        let (left, right) = indices.split_at_mut(middle);
        let left_index = build(
            vertices,
            source,
            source_ids,
            left,
            ordered,
            ordered_ids,
            nodes,
        )?;
        debug_assert_eq!(left_index, node_index + 1);
        let right_index = build(
            vertices,
            source,
            source_ids,
            right,
            ordered,
            ordered_ids,
            nodes,
        )?;
        nodes[node_index] = MeshNode {
            lower,
            upper,
            data: u32::try_from(right_index.checked_sub(node_index)?).ok()? << 2,
            triangle_offset: 0,
        };
        Some(node_index)
    }

    if triangles.is_empty() || triangles.len() != triangle_ids.len() {
        return None;
    }
    let source = std::mem::take(triangles);
    let source_ids = std::mem::take(triangle_ids);
    let mut indices: Vec<usize> = (0..source.len()).collect();
    let mut ordered = Vec::with_capacity(source.len());
    let mut ordered_ids = Vec::with_capacity(source.len());
    let mut nodes = Vec::with_capacity(source.len().div_ceil(8) * 2);
    build(
        vertices,
        &source,
        &source_ids,
        &mut indices,
        &mut ordered,
        &mut ordered_ids,
        &mut nodes,
    )?;
    *triangles = ordered;
    *triangle_ids = ordered_ids;
    Some(nodes)
}

pub fn b3_create_height_field_shape(
    body: BodyId,
    def: &ShapeDef,
    compressed_heights: &[u16],
    column_count: usize,
    row_count: usize,
    min_height: f32,
    height_scale: f32,
    scale: [f32; 3],
    cell_materials: &[u8],
    triangle_flags: &[u8],
    clockwise: bool,
) -> ShapeId {
    let Some(point_count) = column_count.checked_mul(row_count) else {
        eprintln!("height field dimensions overflow");
        return b3_null_shape_id();
    };
    let Some(cell_count) = column_count.checked_sub(1).and_then(|columns| {
        row_count
            .checked_sub(1)
            .and_then(|rows| columns.checked_mul(rows))
    }) else {
        return b3_null_shape_id();
    };
    let Some(full_triangle_count) = cell_count.checked_mul(2) else {
        eprintln!("height field triangle count overflow");
        return b3_null_shape_id();
    };
    if column_count < 2
        || row_count < 2
        || point_count > MAX_HEIGHT_FIELD_POINTS
        || full_triangle_count > MAX_HEIGHT_FIELD_TRIANGLES
        || compressed_heights.len() != point_count
        || cell_materials.len() != cell_count
        || triangle_flags.len() != full_triangle_count
        || !min_height.is_finite()
        || !height_scale.is_finite()
        || height_scale < 0.0
        || scale
            .iter()
            .any(|value| !value.is_finite() || *value <= 0.0)
    {
        eprintln!(
            "invalid height field: {column_count}x{row_count}, points={point_count}, triangles={full_triangle_count}"
        );
        return b3_null_shape_id();
    }
    let world = world_id_from_body(body);
    with_world_mut(world, |w| {
        if !body_ref(w, body).is_some_and(|body| {
            body.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC) != 0
        }) {
            eprintln!("height field shapes require a static or kinematic body");
            return b3_null_shape_id();
        }
        let mut vertices = Vec::with_capacity(point_count);
        for row in 0..row_count {
            for column in 0..column_count {
                let index = row * column_count + column;
                let height = min_height + height_scale * f32::from(compressed_heights[index]);
                vertices.push([
                    scale[0] * column as f32,
                    scale[1] * height,
                    scale[2] * row as f32,
                ]);
            }
        }
        let mut lower = [f32::MAX; 3];
        let mut upper = [f32::MIN; 3];
        for point in &vertices {
            for axis in 0..3 {
                lower[axis] = lower[axis].min(point[axis]);
                upper[axis] = upper[axis].max(point[axis]);
            }
        }
        let mut triangles = Vec::with_capacity(full_triangle_count);
        let mut triangle_ids = Vec::with_capacity(full_triangle_count);
        for row in 0..row_count - 1 {
            for column in 0..column_count - 1 {
                let cell = row * (column_count - 1) + column;
                let material = cell_materials[cell];
                if material == u8::MAX {
                    continue;
                }
                let i11 = row * column_count + column;
                let i12 = i11 + 1;
                let i21 = i11 + column_count;
                let i22 = i21 + 1;
                let source = 2 * cell;
                let mut append = |a: usize, b: usize, c: usize, flags: u8| {
                    let flags = if clockwise {
                        let edge1 = flags & (1 | 8);
                        let edge3 = flags & (4 | 32);
                        (flags & !(1 | 4 | 8 | 32)) | (edge1 << 2) | (edge3 >> 2)
                    } else {
                        flags
                    };
                    let (b, c) = if clockwise { (c, b) } else { (b, c) };
                    triangles.push([
                        a as u32,
                        b as u32,
                        c as u32,
                        u32::from(flags) | (u32::from(material) << 8),
                    ]);
                };
                append(i11, i21, i12, triangle_flags[source]);
                triangle_ids.push(i32::try_from(source).unwrap_or(i32::MAX));
                append(i22, i12, i21, triangle_flags[source + 1]);
                triangle_ids.push(i32::try_from(source + 1).unwrap_or(i32::MAX));
            }
        }
        let nodes = if triangles.is_empty() {
            vec![MeshNode {
                lower,
                upper,
                data: 3,
                triangle_offset: 0,
            }]
        } else {
            let Some(nodes) = build_triangle_bvh(&vertices, &mut triangles, &mut triangle_ids)
            else {
                eprintln!("height field BVH construction failed");
                return b3_null_shape_id();
            };
            nodes
        };
        let center = [
            0.5 * (lower[0] + upper[0]),
            0.5 * (lower[1] + upper[1]),
            0.5 * (lower[2] + upper[2]),
        ];
        let half = [
            (0.5 * (upper[0] - lower[0])).max(0.05),
            (0.5 * (upper[1] - lower[1])).max(0.05),
            (0.5 * (upper[2] - lower[2])).max(0.05),
        ];
        let id = attach_shape(
            w,
            body,
            world,
            KIND_MESH,
            half,
            center,
            center,
            [0.0; 3],
            0.0,
            [0.0; 6],
            0.0,
            Vec::new(),
            Vec::new(),
            Vec::new(),
            Vec::new(),
            def,
        );
        if let Some(shape) = w
            .shapes
            .get_mut(id.index1.saturating_sub(1) as usize)
            .and_then(Option::as_mut)
        {
            shape.public_kind = PUBLIC_KIND_HEIGHT_FIELD;
            shape.mesh_vertices = Arc::new(vertices);
            shape.mesh_triangles = Arc::new(triangles);
            shape.mesh_triangle_ids = Arc::new(triangle_ids);
            shape.mesh_nodes = Arc::new(nodes);
        }
        if id.index1 > 0 {
            let index = id.index1 as usize - 1;
            update_body_extents_range(w, body, index..index + 1, false);
        }
        mark_scene_dirty(w);
        id
    })
    .unwrap_or_else(b3_null_shape_id)
}

#[allow(clippy::too_many_arguments)]
pub fn b3_replace_height_field_shape(
    id: ShapeId,
    compressed_heights: &[u16],
    column_count: usize,
    row_count: usize,
    min_height: f32,
    height_scale: f32,
    scale: [f32; 3],
    cell_materials: &[u8],
    triangle_flags: &[u8],
    clockwise: bool,
) -> bool {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    let Some((body, def)) = with_world(world, |w| {
        let shape = w.shapes.get(id.index1.checked_sub(1)? as usize)?.as_ref()?;
        if shape.generation != id.generation || shape.public_kind != PUBLIC_KIND_HEIGHT_FIELD {
            return None;
        }
        Some((
            live_body_id(w, id.world0, shape.body_index),
            ShapeDef {
                density: shape.density,
                friction: shape.friction,
                restitution: shape.restitution,
                rolling_resistance: shape.rolling,
                explosion_scale: shape.explosion_scale,
                filter: shape.filter,
                is_sensor: shape.event_flags & SHAPE_IS_SENSOR != 0,
                enable_sensor_events: shape.event_flags & SHAPE_ENABLE_SENSOR_EVENTS != 0,
                enable_contact_events: shape.event_flags & SHAPE_ENABLE_CONTACT_EVENTS != 0,
                enable_hit_events: shape.event_flags & SHAPE_ENABLE_HIT_EVENTS != 0,
                enable_custom_filtering: shape.event_flags & SHAPE_ENABLE_CUSTOM_FILTERING != 0,
                enable_pre_solve_events: shape.event_flags & SHAPE_ENABLE_PRE_SOLVE_EVENTS != 0,
                enable_speculative_contact: shape.event_flags & SHAPE_DISABLE_SPECULATIVE == 0,
                user_material_id: shape.user_material_id,
                user_data: shape.user_data,
                update_body_mass: false,
            },
        ))
    })
    .flatten() else {
        return false;
    };
    let replacement = b3_create_height_field_shape(
        body,
        &def,
        compressed_heights,
        column_count,
        row_count,
        min_height,
        height_scale,
        scale,
        cell_materials,
        triangle_flags,
        clockwise,
    );
    if replacement.index1 == 0 {
        return false;
    }
    with_world_mut(world, |w| {
        let replacement_index = replacement.index1 as usize - 1;
        let Some(mut cooked) = w.shapes.get_mut(replacement_index).and_then(Option::take) else {
            return false;
        };
        let Some(shape) = w
            .shapes
            .get_mut(id.index1 as usize - 1)
            .and_then(Option::as_mut)
        else {
            return false;
        };
        shape.half = cooked.half;
        shape.geometry_center = cooked.geometry_center;
        shape.local_center = cooked.local_center;
        shape.mesh_vertices = std::mem::take(&mut cooked.mesh_vertices);
        shape.mesh_triangles = std::mem::take(&mut cooked.mesh_triangles);
        shape.mesh_triangle_ids = std::mem::take(&mut cooked.mesh_triangle_ids);
        shape.mesh_nodes = std::mem::take(&mut cooked.mesh_nodes);
        if let Some(body) = w
            .bodies
            .get_mut(shape.body_index.saturating_sub(1) as usize)
            .and_then(Option::as_mut)
        {
            body.gpu.half = shape.half;
        }
        update_body_extents(w, body);
        mark_scene_dirty(w);
        true
    })
    .unwrap_or(false)
}

pub fn b3_replace_mesh_shape(
    id: ShapeId,
    vertices: &[[f32; 3]],
    triangles: &[[u32; 3]],
    flags: &[u8],
    materials: &[u8],
    nodes: &[MeshNode],
    scale: [f32; 3],
) -> bool {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    let Some((body, mut def)) = with_world(world, |w| {
        let shape = w.shapes.get(id.index1.checked_sub(1)? as usize)?.as_ref()?;
        if shape.generation != id.generation || shape.kind != KIND_MESH {
            return None;
        }
        Some((
            BodyId {
                index1: shape.body_index,
                world0: id.world0,
                generation: w
                    .bodies
                    .get(shape.body_index.checked_sub(1)? as usize)?
                    .as_ref()?
                    .generation,
            },
            ShapeDef {
                density: shape.density,
                friction: shape.friction,
                restitution: shape.restitution,
                rolling_resistance: shape.rolling,
                explosion_scale: shape.explosion_scale,
                filter: shape.filter,
                is_sensor: shape.event_flags & SHAPE_IS_SENSOR != 0,
                enable_sensor_events: shape.event_flags & SHAPE_ENABLE_SENSOR_EVENTS != 0,
                enable_contact_events: shape.event_flags & SHAPE_ENABLE_CONTACT_EVENTS != 0,
                enable_hit_events: shape.event_flags & SHAPE_ENABLE_HIT_EVENTS != 0,
                enable_custom_filtering: shape.event_flags & SHAPE_ENABLE_CUSTOM_FILTERING != 0,
                enable_pre_solve_events: shape.event_flags & SHAPE_ENABLE_PRE_SOLVE_EVENTS != 0,
                enable_speculative_contact: shape.event_flags & SHAPE_DISABLE_SPECULATIVE == 0,
                user_material_id: shape.user_material_id,
                user_data: shape.user_data,
                update_body_mass: false,
            },
        ))
    })
    .flatten() else {
        return false;
    };
    def.update_body_mass = false;
    let temporary = b3_create_mesh_shape(
        body, &def, vertices, triangles, flags, materials, nodes, scale,
    );
    if temporary.index1 == 0 {
        return false;
    }
    with_world_mut(world, |w| {
        let old_index = id.index1 as usize - 1;
        let temporary_index = temporary.index1 as usize - 1;
        let Some(mut replacement) = w.shapes.get_mut(temporary_index).and_then(Option::take) else {
            return false;
        };
        let Some(old) = w.shapes.get_mut(old_index).and_then(Option::as_mut) else {
            return false;
        };
        old.half = replacement.half;
        old.geometry_center = replacement.geometry_center;
        old.local_center = replacement.local_center;
        old.mesh_vertices = std::mem::take(&mut replacement.mesh_vertices);
        old.mesh_triangles = std::mem::take(&mut replacement.mesh_triangles);
        old.mesh_triangle_ids = std::mem::take(&mut replacement.mesh_triangle_ids);
        old.mesh_nodes = std::mem::take(&mut replacement.mesh_nodes);
        old.mesh_scale = replacement.mesh_scale;
        old.mesh_instance = replacement.mesh_instance;
        if let Some(body) = w
            .bodies
            .get_mut(old.body_index.saturating_sub(1) as usize)
            .and_then(Option::as_mut)
        {
            body.gpu.kind = KIND_MESH;
            body.gpu.half = old.half;
        }
        update_body_extents(w, body);
        mark_scene_dirty(w);
        true
    })
    .unwrap_or(false)
}

fn gpu_index(id: BodyId) -> u32 {
    (id.index1.max(1) as u32) - 1
}

fn stored_origin_anchor(anchor: [f32; 3]) -> [f32; 3] {
    // Box3D joint frames are body-origin. The solver subtracts the live COM.
    // Do not bake local_center here: ApplyMassFromShapes after CreateJoint
    // must keep the same stored anchors.
    anchor
}

fn joint_r(body: &CpuBody, origin_anchor: [f32; 3]) -> [f32; 3] {
    quat_rotate(
        body.gpu.rot,
        [
            origin_anchor[0] - body.local_center[0],
            origin_anchor[1] - body.local_center[1],
            origin_anchor[2] - body.local_center[2],
        ],
    )
}

pub fn b3_create_revolute_joint(world: WorldId, def: &RevoluteJointDef) -> JointId {
    with_world_mut(world, |w| {
        let anchor_a = stored_origin_anchor(def.local_anchor_a);
        let anchor_b = stored_origin_anchor(def.local_anchor_b);
        w.joints.push(JointGpu {
            a: gpu_index(def.body_a),
            b: gpu_index(def.body_b),
            kind: JOINT_REVOLUTE,
            _pad0: 0,
            anchor_a,
            hertz: def.hertz,
            anchor_b,
            damping: def.damping,
            frame_a_rotation: def.local_rotation_a,
            frame_b_rotation: def.local_rotation_b,
            flags: u32::from(def.enable_spring) * REVOLUTE_ENABLE_SPRING
                | u32::from(def.enable_limit) * REVOLUTE_ENABLE_LIMIT
                | u32::from(def.enable_motor) * REVOLUTE_ENABLE_MOTOR
                | u32::from(def.collide_connected) * JOINT_COLLIDE_CONNECTED,
            spring_hertz: def.spring_hertz.max(0.0),
            spring_damping: def.spring_damping.max(0.0),
            target_translation: def.target_angle,
            lower_translation: def
                .lower_angle
                .min(def.upper_angle)
                .clamp(-0.99 * std::f32::consts::PI, 0.99 * std::f32::consts::PI),
            upper_translation: def
                .upper_angle
                .max(def.lower_angle)
                .clamp(-0.99 * std::f32::consts::PI, 0.99 * std::f32::consts::PI),
            max_motor_force: def.max_motor_torque.max(0.0),
            motor_speed: def.motor_speed,
            ..JointGpu::default()
        });
        w.joint_meta.push(Some(CpuJoint::new(
            def.force_threshold,
            def.torque_threshold,
            def.user_data,
        )));
        mark_scene_dirty(w);
        JointId {
            index1: w.joints.len() as i32,
            world0: world.index1,
            generation: 1,
        }
    })
    .unwrap_or_else(b3_null_joint_id)
}

pub fn b3_create_spherical_joint(world: WorldId, def: &SphericalJointDef) -> JointId {
    with_world_mut(world, |w| {
        let anchor_a = stored_origin_anchor(def.local_anchor_a);
        let anchor_b = stored_origin_anchor(def.local_anchor_b);
        w.joints.push(JointGpu {
            a: gpu_index(def.body_a),
            b: gpu_index(def.body_b),
            kind: JOINT_SPHERICAL,
            _pad0: 0,
            anchor_a,
            hertz: def.hertz,
            anchor_b,
            damping: def.damping,
            frame_a_rotation: def.local_rotation_a,
            frame_b_rotation: def.local_rotation_b,
            flags: u32::from(def.enable_spring) * SPHERICAL_ENABLE_SPRING
                | u32::from(def.enable_cone_limit) * SPHERICAL_ENABLE_CONE_LIMIT
                | u32::from(def.enable_twist_limit) * SPHERICAL_ENABLE_TWIST_LIMIT
                | u32::from(def.enable_motor) * SPHERICAL_ENABLE_MOTOR
                | u32::from(def.collide_connected) * JOINT_COLLIDE_CONNECTED,
            spring_hertz: def.spring_hertz.max(0.0),
            spring_damping: def.spring_damping.max(0.0),
            target_rotation: def.target_rotation,
            target_translation: def.cone_angle.clamp(0.0, 0.5 * std::f32::consts::PI),
            lower_translation: def
                .lower_twist_angle
                .min(def.upper_twist_angle)
                .clamp(-0.99 * std::f32::consts::PI, 0.99 * std::f32::consts::PI),
            upper_translation: def
                .upper_twist_angle
                .max(def.lower_twist_angle)
                .clamp(-0.99 * std::f32::consts::PI, 0.99 * std::f32::consts::PI),
            max_motor_force: def.max_motor_torque.max(0.0),
            motor_angular_velocity: def.motor_velocity,
            ..JointGpu::default()
        });
        w.joint_meta.push(Some(CpuJoint::new(
            def.force_threshold,
            def.torque_threshold,
            def.user_data,
        )));
        mark_scene_dirty(w);
        JointId {
            index1: w.joints.len() as i32,
            world0: world.index1,
            generation: 1,
        }
    })
    .unwrap_or_else(b3_null_joint_id)
}

pub fn b3_create_prismatic_joint(world: WorldId, def: &PrismaticJointDef) -> JointId {
    with_world_mut(world, |w| {
        let anchor_a = stored_origin_anchor(def.local_anchor_a);
        let anchor_b = stored_origin_anchor(def.local_anchor_b);
        w.joints.push(JointGpu {
            a: gpu_index(def.body_a),
            b: gpu_index(def.body_b),
            kind: JOINT_PRISMATIC,
            _pad0: 0,
            anchor_a,
            hertz: def.hertz,
            anchor_b,
            damping: def.damping,
            axis: def.local_axis_a,
            impulse: 0.0,
            frame_a_rotation: def.local_rotation_a,
            frame_b_rotation: def.local_rotation_b,
            flags: u32::from(def.enable_spring) * crate::types::PRISMATIC_ENABLE_SPRING
                | u32::from(def.enable_limit) * crate::types::PRISMATIC_ENABLE_LIMIT
                | u32::from(def.enable_motor) * crate::types::PRISMATIC_ENABLE_MOTOR
                | u32::from(def.collide_connected) * JOINT_COLLIDE_CONNECTED,
            spring_hertz: def.spring_hertz,
            spring_damping: def.spring_damping,
            target_translation: def.target_translation,
            lower_translation: def.lower_translation,
            upper_translation: def.upper_translation,
            max_motor_force: def.max_motor_force,
            motor_speed: def.motor_speed,
            ..JointGpu::default()
        });
        w.joint_meta.push(Some(CpuJoint::new(
            def.force_threshold,
            def.torque_threshold,
            def.user_data,
        )));
        mark_scene_dirty(w);
        JointId {
            index1: w.joints.len() as i32,
            world0: world.index1,
            generation: 1,
        }
    })
    .unwrap_or_else(b3_null_joint_id)
}

pub fn b3_create_distance_joint(world: WorldId, def: &DistanceJointDef) -> JointId {
    with_world_mut(world, |w| {
        let min_length = def.min_length.max(LINEAR_SLOP);
        let anchor_a = stored_origin_anchor(def.local_anchor_a);
        let anchor_b = stored_origin_anchor(def.local_anchor_b);
        w.joints.push(JointGpu {
            a: gpu_index(def.body_a),
            b: gpu_index(def.body_b),
            kind: JOINT_DISTANCE,
            anchor_a,
            hertz: def.hertz,
            anchor_b,
            damping: def.damping,
            perp_impulse: [def.lower_spring_force, def.upper_spring_force],
            flags: u32::from(def.enable_spring) * crate::types::DISTANCE_ENABLE_SPRING
                | u32::from(def.enable_limit) * crate::types::DISTANCE_ENABLE_LIMIT
                | u32::from(def.enable_motor) * crate::types::DISTANCE_ENABLE_MOTOR
                | u32::from(def.collide_connected) * JOINT_COLLIDE_CONNECTED,
            spring_hertz: def.spring_hertz,
            spring_damping: def.spring_damping,
            target_translation: def.length.max(LINEAR_SLOP),
            lower_translation: min_length,
            upper_translation: def.max_length.max(min_length),
            max_motor_force: def.max_motor_force,
            motor_speed: def.motor_speed,
            ..JointGpu::default()
        });
        w.joint_meta.push(Some(CpuJoint::new(
            def.force_threshold,
            def.torque_threshold,
            def.user_data,
        )));
        mark_scene_dirty(w);
        JointId {
            index1: w.joints.len() as i32,
            world0: world.index1,
            generation: 1,
        }
    })
    .unwrap_or_else(b3_null_joint_id)
}

pub fn b3_create_parallel_joint(world: WorldId, def: &ParallelJointDef) -> JointId {
    with_world_mut(world, |w| {
        w.joints.push(JointGpu {
            a: gpu_index(def.body_a),
            b: gpu_index(def.body_b),
            kind: JOINT_PARALLEL,
            frame_a_rotation: def.local_rotation_a,
            frame_b_rotation: def.local_rotation_b,
            spring_hertz: def.hertz.max(0.0),
            spring_damping: def.damping.max(0.0),
            max_motor_force: def.max_torque.max(0.0),
            flags: u32::from(def.collide_connected) * JOINT_COLLIDE_CONNECTED,
            ..JointGpu::default()
        });
        w.joint_meta.push(Some(CpuJoint::new(
            def.force_threshold,
            def.torque_threshold,
            def.user_data,
        )));
        mark_scene_dirty(w);
        JointId {
            index1: w.joints.len() as i32,
            world0: world.index1,
            generation: 1,
        }
    })
    .unwrap_or_else(b3_null_joint_id)
}

pub fn b3_create_filter_joint(world: WorldId, def: &FilterJointDef) -> JointId {
    with_world_mut(world, |w| {
        w.joints.push(JointGpu {
            a: gpu_index(def.body_a),
            b: gpu_index(def.body_b),
            kind: JOINT_FILTER,
            flags: u32::from(def.collide_connected) * JOINT_COLLIDE_CONNECTED,
            ..JointGpu::default()
        });
        w.joint_meta.push(Some(CpuJoint::new(
            def.force_threshold,
            def.torque_threshold,
            def.user_data,
        )));
        mark_scene_dirty(w);
        JointId {
            index1: w.joints.len() as i32,
            world0: world.index1,
            generation: 1,
        }
    })
    .unwrap_or_else(b3_null_joint_id)
}

pub fn b3_create_motor_joint(world: WorldId, def: &MotorJointDef) -> JointId {
    with_world_mut(world, |w| {
        let anchor_a = stored_origin_anchor(def.local_anchor_a);
        let anchor_b = stored_origin_anchor(def.local_anchor_b);
        w.joints.push(JointGpu {
            a: gpu_index(def.body_a),
            b: gpu_index(def.body_b),
            kind: JOINT_MOTOR,
            anchor_a,
            hertz: def.linear_hertz,
            anchor_b,
            damping: def.linear_damping,
            axis: def.linear_velocity,
            frame_a_rotation: def.local_rotation_a,
            frame_b_rotation: def.local_rotation_b,
            flags: u32::from(def.collide_connected) * JOINT_COLLIDE_CONNECTED,
            spring_hertz: def.angular_hertz,
            spring_damping: def.angular_damping,
            target_translation: def.max_velocity_force,
            lower_translation: def.max_velocity_torque,
            upper_translation: def.max_spring_force,
            max_motor_force: def.max_spring_torque,
            motor_angular_velocity: def.angular_velocity,
            ..JointGpu::default()
        });
        w.joint_meta.push(Some(CpuJoint::new(
            def.force_threshold,
            def.torque_threshold,
            def.user_data,
        )));
        mark_scene_dirty(w);
        JointId {
            index1: w.joints.len() as i32,
            world0: world.index1,
            generation: 1,
        }
    })
    .unwrap_or_else(b3_null_joint_id)
}

pub fn b3_create_weld_joint(world: WorldId, def: &WeldJointDef) -> JointId {
    with_world_mut(world, |w| {
        let anchor_a = stored_origin_anchor(def.local_anchor_a);
        let anchor_b = stored_origin_anchor(def.local_anchor_b);
        w.joints.push(JointGpu {
            a: gpu_index(def.body_a),
            b: gpu_index(def.body_b),
            kind: JOINT_WELD,
            _pad0: 0,
            anchor_a,
            hertz: def.hertz,
            anchor_b,
            damping: def.damping,
            frame_a_rotation: def.local_rotation_a,
            frame_b_rotation: def.local_rotation_b,
            flags: u32::from(def.collide_connected) * JOINT_COLLIDE_CONNECTED,
            weld_linear_hertz: def.linear_hertz,
            weld_linear_damping: def.linear_damping_ratio,
            weld_angular_hertz: def.angular_hertz,
            weld_angular_damping: def.angular_damping_ratio,
            ..JointGpu::default()
        });
        w.joint_meta.push(Some(CpuJoint::new(
            def.force_threshold,
            def.torque_threshold,
            def.user_data,
        )));
        mark_scene_dirty(w);
        JointId {
            index1: w.joints.len() as i32,
            world0: world.index1,
            generation: 1,
        }
    })
    .unwrap_or_else(b3_null_joint_id)
}

pub fn b3_create_wheel_joint(world: WorldId, def: &WheelJointDef) -> JointId {
    with_world_mut(world, |w| {
        let anchor_a = stored_origin_anchor(def.local_anchor_a);
        let anchor_b = stored_origin_anchor(def.local_anchor_b);
        w.joints.push(JointGpu {
            a: gpu_index(def.body_a),
            b: gpu_index(def.body_b),
            kind: JOINT_WHEEL,
            anchor_a,
            hertz: def.hertz,
            anchor_b,
            damping: def.damping,
            frame_a_rotation: def.local_rotation_a,
            frame_b_rotation: def.local_rotation_b,
            flags: u32::from(def.enable_suspension_spring) * WHEEL_ENABLE_SUSPENSION_SPRING
                | u32::from(def.enable_suspension_limit) * WHEEL_ENABLE_SUSPENSION_LIMIT
                | u32::from(def.enable_spin_motor) * WHEEL_ENABLE_SPIN_MOTOR
                | u32::from(def.enable_steering) * WHEEL_ENABLE_STEERING
                | u32::from(def.enable_steering_limit) * WHEEL_ENABLE_STEERING_LIMIT
                | u32::from(def.collide_connected) * JOINT_COLLIDE_CONNECTED,
            spring_hertz: def.suspension_hertz,
            spring_damping: def.suspension_damping_ratio,
            lower_translation: def.lower_suspension_limit,
            upper_translation: def.upper_suspension_limit,
            max_motor_force: def.max_spin_torque,
            motor_speed: def.spin_speed,
            // Wheel-only aliases avoid growing the already aligned GPU record.
            weld_linear_hertz: def.steering_hertz,
            weld_linear_damping: def.steering_damping_ratio,
            target_rotation: [
                def.target_steering_angle,
                def.max_steering_torque,
                def.lower_steering_limit,
                def.upper_steering_limit,
            ],
            ..JointGpu::default()
        });
        w.joint_meta.push(Some(CpuJoint::new(
            def.force_threshold,
            def.torque_threshold,
            def.user_data,
        )));
        mark_scene_dirty(w);
        JointId {
            index1: w.joints.len() as i32,
            world0: world.index1,
            generation: 1,
        }
    })
    .unwrap_or_else(b3_null_joint_id)
}

pub fn b3_body_mark_hidden(body: BodyId) {
    let world = world_id_from_body(body);
    with_world_mut(world, |w| {
        if let Some(cpu) = body_mut(w, body) {
            cpu.gpu.flags |= FLAG_HIDDEN;
            w.bodies_dirty = true;
        }
    });
}

pub fn b3_body_get_position(body: BodyId) -> [f32; 3] {
    b3_body_get_transform(body).0
}

pub fn b3_body_get_rotation(body: BodyId) -> [f32; 4] {
    b3_body_get_transform(body).1
}

pub fn b3_body_get_transform(body: BodyId) -> ([f32; 3], [f32; 4]) {
    let world = world_id_from_body(body);
    with_world(world, |w| {
        body_ref(w, body)
            .map(|b| (body_origin(b), b.gpu.rot))
            .unwrap_or(([0.0; 3], [0.0, 0.0, 0.0, 1.0]))
    })
    .unwrap_or(([0.0; 3], [0.0, 0.0, 0.0, 1.0]))
}

/// Renderer-owned policy; ordinary C/Sokol consumers keep automatic snapshots.
pub(crate) fn b3_world_set_automatic_pose_snapshots(id:WorldId, enabled:bool) {
    with_world_mut_no_sync(id,|w| {
        w.automatic_pose_snapshots=enabled;
        #[cfg(not(target_arch = "wasm32"))]
        if let Some(sim)=w.sim.as_mut() {sim.set_automatic_pose_snapshots(enabled);}
    });
}

pub fn b3_world_prepare_pose_snapshot(id: WorldId) {
    with_world_mut_no_sync(id, |w| {
        #[cfg(not(target_arch = "wasm32"))]
        if w.automatic_pose_snapshots {ensure_pose_snapshot(w);}
        else {ensure_cpu_mirror_world(w,id);}
    });
}

pub fn b3_body_get_world_center(body: BodyId) -> [f32; 3] {
    let world = world_id_from_body(body);
    with_world(world, |w| {
        body_ref(w, body).map(|b| b.gpu.pos).unwrap_or([0.0; 3])
    })
    .unwrap_or([0.0; 3])
}

pub fn b3_world_ensure_gpu(id: WorldId) {
    with_world_mut(id, |w| {
        let (bodies, _, _) = pack_gpu_slots(w);
        if bodies.is_empty() {
            return;
        }
        let n = bodies.len() as u32;
        let h = FIXED_DT / DEFAULT_SUB_STEPS.max(1) as f32;
        ensure_sim(w, &bodies, n, h, FIXED_DT, true);
    });
}

pub fn b3_world_loading_needed(id: WorldId) -> bool {
    with_world_no_sync(id, |w| !w.bodies.is_empty() &&
        w.sim.as_ref().is_none_or(|sim| !sim.collision_pipeline_ready()))
        .unwrap_or(false)
}

/// Loading-only preparation. No physics step, callbacks, or pose advancement.
pub fn b3_world_prepare_collision(id: WorldId) {
    with_world_mut_no_sync(id, |w| {
        if let Some(sim) = w.sim.as_ref() { sim.prepare_collision_pipeline(); }
    });
}

// Pair keys in the pre-step GPU snapshot still refer to the prior shape order.
// Share immutable maps across steady-state steps; rebuild only on topology edits.
fn advance_contact_shape_history(w: &mut WorldInner, world0: u16) {
    w.previous_contact_shape_ids = w.contact_shape_ids.clone();
    w.previous_contact_child_ordinals = w.contact_child_ordinals.clone();
    if w.contact_shape_revision != w.query_topology {
        w.contact_shape_ids = std::sync::Arc::new(gpu_public_shapes(&w.shapes, &w.bodies)
            .into_iter().map(|(source, shape)| ShapeId {
                index1: source as i32 + 1, world0, generation: shape.generation,
            }).collect());
        w.contact_child_ordinals = Arc::new(w.shapes.iter().flatten()
            .filter(|shape| w.bodies.get(shape.body_index.saturating_sub(1) as usize).is_some_and(Option::is_some))
            .map(|shape| shape.compound_child_index).collect());
        w.contact_shape_revision = w.query_topology;
    }
}

// Resident submission currently covers event-free, non-jointed convex worlds.
// Mutators already synchronize, but dirty checks also protect direct scene edits.
fn can_submit_resident(w:&WorldInner)->bool {
    w.gpu_ccd_requested && w.gpu_resident_requested && w.def.enable_continuous && !w.post_ccd_pending
        && !w.scene_dirty && !w.bodies_dirty
        && !scene_capabilities(w).pending_forces
        && !w.joints.iter().any(|j|j.kind!=JOINT_NONE)
        && w.custom_filter_callback.is_none() && w.pre_solve_callback.is_none()
        && !world_needs_contact_readback(w)
        && w.sim.as_ref().is_some_and(|sim| sim.uses_convex_ccd() && !sim.has_step_forces())
}

fn step_gpu_inner(id: WorldId, dt: f32, sub_step_count: i32) {
    if dt == 0.0 {
        with_world_mut_no_sync(id, |w| w.reaction_inv_h = 0.0);
        return;
    }
    if !dt.is_finite() || dt<=0.0 {return;}
    with_world_mut_no_sync(id, |w| {
        let resident=can_submit_resident(w);
        #[cfg(not(target_arch = "wasm32"))]
        if !resident && (w.post_ccd_pending || w.gpu_mirror_stale) {
            let _ = sync_world_mirror_parts(w, id, false, false, false);
        }
        let forces = if resident { Vec::new() } else { take_accumulated_forces(w) };
        w.body_move_events.clear();
        w.contact_begin_events.clear();
        w.contact_end_events.clear();
        w.contact_hit_events.clear();
        w.sensor_begin_events.clear();
        w.sensor_end_events.clear();
        // These transitions were fully determined by host destruction/refilter.
        // Publish even when no remaining shape requests GPU contact readback.
        w.contact_end_events.append(&mut w.deferred_contact_end_events);
        w.sensor_end_events.append(&mut w.deferred_sensor_end_events);
        w.joint_events.clear();
        w.events_pending = false;
        let bodies=if resident {Vec::new()} else {pack_gpu_slots(w).0};
        if !resident {
            w.step_start_bodies.clone_from(&bodies);
            joint_reaction::prepare_frames(w);
        }
        w.continuous_sensor_hits.clear();
        if w.bodies.is_empty() { return; }
        let n = w.bodies.len() as u32;
        let sub = sub_step_count.max(1);
        let h = dt / sub as f32;
        w.last_substep_h = h;
        w.reaction_inv_h = sub as f32 * (1.0 / dt);
        if dt > 0.0 { advance_contact_shape_history(w, id.index1); }
        let refresh_ccd=w.scene_dirty || w.sim.is_none();
        ensure_sim(w, &bodies, n, h, dt, false);
        if let Some(sim) = w.sim.as_mut() { sim.set_step_forces(&forces); }
        configure_convex_ccd(w,id,refresh_ccd);
        apply_solver_topology(w);
        if w.physics_invalid {
            return;
        }
        // A zero-duration step submits no bound update, so retain host history.
        if dt > 0.0 {
            let edits: Vec<_> = std::mem::take(&mut w.pending_fat_transforms).into_iter()
                .filter(|((index, generation), _)| w.bodies.get(index.saturating_sub(1) as usize)
                    .and_then(Option::as_ref).is_some_and(|body| body.generation == *generation))
                .map(|((index, _), commands)| (index as u32 - 1, commands)).collect();
            if let Some(sim) = w.sim.as_mut() {
                if let Err(error) = sim.upload_fat_transforms(&edits) {
                    sim.mark_physics_invalid();
                    w.physics_invalid = true;
                    w.gpu_fail = std::ffi::CString::new(error).ok();
                    return;
                }
            }
        }
        if let Some(sim) = w.sim.as_mut() {
            sim.configure_idle_step(resident && w.gpu_idle_requested && w.def.enable_sleep,
                [w.query_topology,w.query_state],
                [w.query_topology,w.query_state.saturating_add(1+u64::from(sim.uses_convex_ccd()))]);
            #[cfg(not(target_arch = "wasm32"))]
            if w.custom_filter_callback.is_some() || w.pre_solve_callback.is_some() {
                let dense_shapes: Vec<(ShapeId, u32, u32)> = gpu_public_shapes(&w.shapes, &w.bodies)
                    .into_iter().map(|(source, shape)| (
                        ShapeId { index1: source as i32 + 1, world0: id.index1, generation: shape.generation },
                        shape.body_index.saturating_sub(1) as u32,
                        shape.event_flags,
                    )).collect();
                if let Some(callback) = w.custom_filter_callback {
                    sim.callback_candidates_submit();
                    let pairs = pollster::block_on(sim.read_callback_pairs());
                    let mut accepted = Vec::with_capacity(pairs.len());
                    let mut decisions = HashMap::new();
                    for key in pairs {
                        let dense_a = (key & 0xffff_ffff) as usize;
                        let dense_b = (key >> 32) as usize;
                        let (
                            Some(&(shape_id_a, body_a, flags_a)),
                            Some(&(shape_id_b, body_b, flags_b)),
                        ) = (dense_shapes.get(dense_a), dense_shapes.get(dense_b))
                        else {
                            continue;
                        };
                        let awake_dynamic = |body_index: u32| {
                            bodies.get(body_index as usize).is_some_and(|body| {
                                body.flags & (FLAG_STATIC | FLAG_KINEMATIC | FLAG_SLEEP) == 0
                            })
                        };
                        let accept = if (flags_a | flags_b) & SHAPE_ENABLE_CUSTOM_FILTERING == 0
                            || (!awake_dynamic(body_a) && !awake_dynamic(body_b)) {
                            true
                        } else {
                            let pair = keyed_contact(((shape_id_a.index1.max(shape_id_b.index1) as u64 - 1) << 32)
                                | (shape_id_a.index1.min(shape_id_b.index1) as u64 - 1), key, &w.contact_child_ordinals);
                            *decisions.entry(pair).or_insert_with(|| unsafe {
                                callback(shape_id_a, shape_id_b, w.custom_filter_context as *mut std::ffi::c_void)
                            })
                        };
                        if accept { accepted.push(key); }
                    }
                    sim.write_callback_pairs(&accepted);
                    sim.callback_narrowphase_submit();
                } else {
                    sim.callback_detect_submit();
                }
                if w.pre_solve_callback.is_some() {
                    let slots = pollster::block_on(sim.read_callback_slots());
                    let contacts = pollster::block_on(sim.read_contacts());
                    let mut disabled = Vec::new();
                    let mut decisions = HashMap::new();
                    for slot in slots {
                        let Some(contact) = contacts.get(slot as usize) else {
                            continue;
                        };
                        if contact.a == u32::MAX || contact.manifold_link[1] != 0 {
                            continue;
                        }
                        let key = contact.pair_key();
                        let dense_a = (key & 0xffff_ffff) as usize;
                        let dense_b = (key >> 32) as usize;
                        let (Some(&(mut shape_id_a, body_a, flags_a)), Some(&(mut shape_id_b, _, flags_b))) =
                            (dense_shapes.get(dense_a), dense_shapes.get(dense_b))
                        else {
                            continue;
                        };
                        if body_a != contact.a { std::mem::swap(&mut shape_id_a, &mut shape_id_b); }
                        let awake_dynamic = |body_index: u32| {
                            bodies.get(body_index as usize).is_some_and(|body| {
                                body.flags & (FLAG_STATIC | FLAG_KINEMATIC | FLAG_SLEEP) == 0
                            })
                        };
                        if !awake_dynamic(contact.a) && !awake_dynamic(contact.b) {
                            continue;
                        }
                        let pre_solve_opt_in = (flags_a | flags_b) & SHAPE_ENABLE_PRE_SOLVE_EVENTS != 0;
                        let sensor = (flags_a | flags_b) & SHAPE_IS_SENSOR != 0;
                        if pre_solve_opt_in && !sensor && contact.count > 0 {
                            if let Some(callback) = w.pre_solve_callback {
                                let Some(body_a) = bodies.get(contact.a as usize) else {
                                    continue;
                                };
                                let point = Vec3::from([
                                    body_a.pos[0] + contact.ra0[0],
                                    body_a.pos[1] + contact.ra0[1],
                                    body_a.pos[2] + contact.ra0[2],
                                ]);
                                let normal = Vec3::from([contact.nx, contact.ny, contact.nz]);
                                let pair = keyed_contact(((shape_id_a.index1.max(shape_id_b.index1) as u64 - 1) << 32)
                                | (shape_id_a.index1.min(shape_id_b.index1) as u64 - 1), key, &w.contact_child_ordinals);
                                let accept = *decisions.entry(pair).or_insert_with(|| unsafe {
                                    callback(shape_id_a, shape_id_b, point, normal, w.pre_solve_context as *mut std::ffi::c_void)
                                });
                                if !accept {
                                    disabled.push((slot, disabled_lifecycle(contact.lifecycle[1])));
                                }
                            }
                        }
                    }
                    if let Err(error) = sim.disable_callback_contacts(&disabled, &contacts) {
                        w.gpu_fail = std::ffi::CString::new(error).ok();
                        w.physics_invalid = true;
                        return;
                    }
                }
                #[cfg(not(target_arch = "wasm32"))]
                let encode_start = std::time::Instant::now();
                sim.set_contact_metrics_context(w.query_topology,w.query_state.saturating_add(1));
                sim.callback_finish_submit(sub);
                w.physics_step = sim.physics_step();
                if let Some(msg) = sim.sticky_fail_message() {
                    w.gpu_fail = std::ffi::CString::new(msg).ok();
                    w.physics_invalid = true;
                }
                if sim.physics_invalid() { w.physics_invalid = true; }
                #[cfg(not(target_arch = "wasm32"))]
                {
                    w.last_gpu_encode_ms = encode_start.elapsed().as_secs_f32() * 1e3;
                    w.gpu_mirror_stale = true;
                    w.post_ccd_pending = true;
                    w.events_pending = true;
                    w.query_state = w.query_state.saturating_add(1);
                }
                return;
            }
            #[cfg(not(target_arch = "wasm32"))]
            let encode_start = std::time::Instant::now();
            #[cfg(not(target_arch = "wasm32"))]
            sim.set_contact_metrics_context(w.query_topology,w.query_state.saturating_add(1));
            sim.step_submit(sub);
            w.physics_step = sim.physics_step();
            #[cfg(not(target_arch = "wasm32"))]
            {
                w.last_gpu_encode_ms = encode_start.elapsed().as_secs_f32() * 1e3;
                w.gpu_mirror_stale = true;
                w.post_ccd_pending = !sim.uses_convex_ccd();
                w.events_pending = true;
                // Contact metrics precede CCD; conservatively invalidate their state
                // identity when GPU corrections may have changed final poses.
                w.query_state = w.query_state.saturating_add(1 + u64::from(sim.uses_convex_ccd()));
            }
            #[cfg(not(target_arch = "wasm32"))]
            if let Some(msg) = sim.sticky_fail_message() {
                w.gpu_fail = std::ffi::CString::new(msg).ok();
                w.physics_invalid = true;
            }
            if sim.physics_invalid() {
                w.physics_invalid = true;
            }
            #[cfg(target_arch = "wasm32")]
            {
                w.gpu_mirror_stale = true;
                w.post_ccd_pending = !sim.uses_convex_ccd();
                w.events_pending = true;
                // Contact metrics precede CCD; conservatively invalidate their state
                // identity when GPU corrections may have changed final poses.
                w.query_state = w.query_state.saturating_add(1 + u64::from(sim.uses_convex_ccd()));
            }
        }
    });
}

fn disabled_lifecycle(flags: u32) -> u32 {
    if flags & CONTACT_START_TOUCHING != 0 {
        1
    } else if flags & CONTACT_TOUCHING != 0 {
        1 | CONTACT_STOP_TOUCHING
    } else {
        1
    }
}

fn take_accumulated_forces(w: &mut WorldInner) -> Vec<(u32, [f32; 3], [f32; 3])> {
    let mut loads = Vec::new();
    for (index, slot) in w.bodies.iter_mut().enumerate() {
        let Some(body) = slot else { continue; };
        let force = std::mem::take(&mut body.force);
        let torque = std::mem::take(&mut body.torque);
        if body.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC | FLAG_DISABLED | FLAG_SLEEP) == 0
            && (force != [0.0; 3] || torque != [0.0; 3]) {
            loads.push((index as u32, force, torque));
        }
    }
    if let Some(capabilities) = w.scene_capabilities.as_mut() { capabilities.pending_forces = false; }
    loads
}

fn wake_body(body: &mut CpuBody) {
    body.gpu.flags &= !FLAG_SLEEP;
    body.gpu.sleep_time = 0.0;
}

/// Submit one world step. Opt-in resident convex worlds retain GPU ownership.
/// Compatibility cohorts and dirty host mutations may finalize the prior step.
#[cfg(feature = "native-command-cache")]
pub(crate) fn b3_world_arm_render_copy(id: WorldId, copy: crate::native_async::RenderCopy) -> bool {
    with_world_mut_no_sync(id, |w| {
        if !can_submit_resident(w) { return false; }
        w.sim.as_mut().unwrap().render_copy = Some(copy);
        true
    }).unwrap_or(false)
}

#[cfg(feature = "native-command-cache")]
pub(crate) fn b3_world_cancel_render_copy(id: WorldId) {
    with_world_mut_no_sync(id, |w| { if let Some(sim) = &mut w.sim { sim.render_copy = None; } });
}

pub fn b3_world_step_gpu(id: WorldId, dt: f32, sub_step_count: i32) {
    step_gpu_inner(id, dt, sub_step_count);
}

/// Drain the GPU queue. Metrics and destroy use this when they need the queue empty.
pub fn b3_world_gpu_wait(id: WorldId) {
    with_world_mut_no_sync(id, |w| {
        if let Some(sim) = w.sim.as_mut() {
            sim.wait_completion();
            #[cfg(not(target_arch = "wasm32"))]
            {
                sim.harvest_gpu_timestamps(true);
                sim.finish_contact_status();
                if sim.physics_invalid() {
                    w.physics_invalid = true;
                }
                if let Some(msg) = sim.sticky_fail_message() {
                    w.gpu_fail = std::ffi::CString::new(msg).ok();
                    w.physics_invalid = true;
                }
            }
        }
        #[cfg(not(target_arch = "wasm32"))]
        copy_gpu_clocks(w);
    });
}

/// Wait for GPU completion and also refresh the CPU body mirror.
pub fn b3_world_gpu_wait_with_mirror(id: WorldId) {
    b3_world_gpu_wait(id);
    with_world_mut(id, |_| {});
}

#[cfg(not(target_arch = "wasm32"))]
pub fn b3_world_begin_timing(id: WorldId, steps: u32) {
    with_world_mut(id, |w| {
        if let Some(sim) = w.sim.as_mut() {
            sim.begin_timing_window(steps);
        }
    });
}

#[cfg(not(target_arch = "wasm32"))]
pub async fn b3_world_finish_timing(id: WorldId) -> Option<crate::sim::TimingWindow> {
    let mut out = None;
    {
        let mut worlds = lock_worlds();
        let Some(w) = slot_mut(&mut worlds, id) else {
            return None;
        };
        if let Some(sim) = w.sim.as_mut() {
            out = sim.finish_timing_window().await;
        }
    }
    out
}

pub fn b3_world_gpu_allocations(id: WorldId) -> Option<crate::sim::AllocationStats> {
    with_world(id, |w| w.sim.as_ref().map(|sim| sim.allocation_stats())).flatten()
}

pub fn b3_world_step(id: WorldId, dt: f32, sub_step_count: i32) {
    step_gpu_inner(id, dt, sub_step_count);
    #[cfg(not(target_arch = "wasm32"))]
    {
        let _ = pollster::block_on(b3_world_sync_from_gpu(id));
    }
}

#[cfg(not(target_arch = "wasm32"))]
fn body_transform(body: &BodyGpu, local_center: [f32; 3]) -> WorldTransform {
    let offset = quat_rotate(body.rot, local_center);
    WorldTransform {
        p: [
            body.pos[0] - offset[0],
            body.pos[1] - offset[1],
            body.pos[2] - offset[2],
        ],
        q: body.rot,
    }
}

#[cfg(not(target_arch = "wasm32"))]
fn sweep_bounds(
    shape: &HostShape,
    start: WorldTransform,
    end: WorldTransform,
) -> ([f32; 3], [f32; 3]) {
    let radius = shape
        .local_points
        .iter()
        .map(|point| {
            (point[0] * point[0] + point[1] * point[1] + point[2] * point[2]).sqrt() + shape.radius
        })
        .fold(shape.radius, f32::max);
    let mut lower = [0.0; 3];
    let mut upper = [0.0; 3];
    for axis in 0..3 {
        lower[axis] = start.p[axis].min(end.p[axis]) - radius;
        upper[axis] = start.p[axis].max(end.p[axis]) + radius;
    }
    (lower, upper)
}

#[cfg(not(target_arch = "wasm32"))]
fn bounds_overlap(a: ([f32; 3], [f32; 3]), b: ([f32; 3], [f32; 3])) -> bool {
    (0..3).all(|axis| a.0[axis] <= b.1[axis] && b.0[axis] <= a.1[axis])
}

fn shapes_collide(a: Filter, b: Filter) -> bool {
    if a.group_index != 0 && a.group_index == b.group_index {
        return a.group_index > 0;
    }
    (a.mask_bits & b.category_bits) != 0 && (a.category_bits & b.mask_bits) != 0
}

#[cfg(not(target_arch = "wasm32"))]
fn run_continuous_collision(w: &mut WorldInner, world0: u16, bodies: &mut [BodyGpu]) -> bool {
    if !w.def.enable_continuous || w.step_start_bodies.len() != bodies.len() {
        return false;
    }
    let mut source_to_dense = vec![u32::MAX; w.bodies.len()];
    let mut dense_to_source = Vec::with_capacity(bodies.len());
    for source in 0..w.bodies.len().min(bodies.len()) {
        source_to_dense[source] = source as u32;
        dense_to_source.push(source);
    }

    let body_pair_allowed = |dense_a: usize, dense_b: usize| {
        !w.joints.iter().any(|joint| {
            joint.kind != JOINT_NONE
                && joint.flags & JOINT_COLLIDE_CONNECTED == 0
                && ((joint.a as usize == dense_a && joint.b as usize == dense_b)
                    || (joint.a as usize == dense_b && joint.b as usize == dense_a))
        })
    };
    let quat_angle = |a: [f32; 4], b: [f32; 4]| {
        let dot = (a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3])
            .abs()
            .clamp(0.0, 1.0);
        2.0 * dot.acos()
    };
    let mut fast = Vec::new();
    for (dense, (&start, end)) in w.step_start_bodies.iter().zip(bodies.iter()).enumerate() {
        if end.flags & (FLAG_STATIC | FLAG_KINEMATIC | FLAG_SLEEP | FLAG_DISABLED) != 0
        {
            continue;
        }
        let source = dense_to_source[dense];
        let Some(cpu) = w.bodies[source].as_ref() else {
            continue;
        };
        let translation =
            glam::Vec3::from_array(end.pos).distance(glam::Vec3::from_array(start.pos));
        let rotation = quat_angle(start.rot, end.rot) * cpu.max_extent;
        if translation + rotation > 0.5 * cpu.min_extent {
            fast.push((end.flags & FLAG_BULLET != 0, dense, source));
        }
    }
    fast.sort_by_key(|&(bullet, dense, _)| (bullet, dense));
    if fast.is_empty() { return false; }

    // Preserve shape-slot order while avoiding a whole-world ownership and
    // target classification scan for every fast body. CCD changes poses and
    // wake state, but not ownership or static/kinematic/bullet classification.
    let mut fast_bodies = vec![false; w.bodies.len()];
    for &(_, _, source) in &fast { fast_bodies[source] = true; }
    let mut own_shapes = vec![Vec::new(); w.bodies.len()];
    let mut static_targets = Vec::new();
    let mut bullet_targets = Vec::new();
    let has_bullet = fast.iter().any(|&(bullet, _, _)| bullet);
    for (index, shape) in w.shapes.iter().enumerate() {
        let Some(shape) = shape else { continue; };
        if shape.public_kind == PUBLIC_KIND_COMPOUND { continue; }
        let source = shape.body_index.saturating_sub(1) as usize;
        if source >= source_to_dense.len() { continue; }
        if fast_bodies[source] { own_shapes[source].push(index); }
        let dense = source_to_dense[source] as usize;
        let Some(body) = bodies.get(dense) else { continue; };
        if body.flags & (FLAG_STATIC | FLAG_KINEMATIC) != 0 {
            static_targets.push(index);
        }
        if has_bullet && body.flags & FLAG_BULLET == 0 {
            bullet_targets.push(index);
        }
    }

    let mut corrected = false;
    let trace_body = std::env::var("GPU_PHYSICS_TRACE_BODY").ok().and_then(|s| s.parse::<usize>().ok());
    for (bullet, dense_fast, source_fast) in fast {
        let Some(cpu_fast) = w.bodies[source_fast].as_ref() else {
            continue;
        };
        let start_fast_body = w.step_start_bodies[dense_fast];
        let mut end_fast_body = bodies[dense_fast];
        let start_fast = body_transform(&start_fast_body, cpu_fast.local_center);
        let end_fast = body_transform(&end_fast_body, cpu_fast.local_center);
        let fast_shapes: Vec<(usize, HostShape, u32)> = own_shapes[source_fast]
            .iter().map(|&index| {
                let shape = w.shapes[index].as_ref().expect("live shape index");
                (index, host_shape(w, world0, index, shape).expect("valid convex shape"), shape.event_flags)
            }).collect();
        let mut solid_fraction = 1.0f32;
        let mut sensor_hits: Vec<(f32, ShapeId, ShapeId)> = Vec::new();

        for (_, fast_shape, fast_flags) in &fast_shapes {
            if fast_flags & SHAPE_IS_SENSOR != 0 {
                continue;
            }
            let fast_bounds = sweep_bounds(fast_shape, start_fast, end_fast);
            let targets = if bullet { &bullet_targets } else { &static_targets };
            for &target_index in targets {
                let target_cpu = w.shapes[target_index].as_ref().expect("live target index");
                let source_target = target_cpu.body_index.saturating_sub(1) as usize;
                if source_target == source_fast || source_target >= source_to_dense.len() {
                    continue;
                }
                let dense_target = source_to_dense[source_target] as usize;
                if dense_target >= bodies.len() {
                    continue;
                }
                let target_end_body = bodies[dense_target];
                let target_static = target_end_body.flags & (FLAG_STATIC | FLAG_KINEMATIC) != 0;
                if !target_static && !bullet {
                    continue;
                }
                if bullet && target_end_body.flags & FLAG_BULLET != 0 {
                    continue;
                }
                if !body_pair_allowed(dense_fast, dense_target)
                    || !shapes_collide(fast_shape.filter, target_cpu.filter)
                {
                    continue;
                }
                let Some(target_shape) = host_shape(w, world0, target_index, target_cpu) else {
                    continue;
                };
                if (target_cpu.event_flags | fast_flags) & SHAPE_ENABLE_CUSTOM_FILTERING != 0 {
                    if let Some(callback) = w.custom_filter_callback {
                        let accepted = unsafe {
                            callback(
                                target_shape.id,
                                fast_shape.id,
                                w.custom_filter_context as *mut std::ffi::c_void,
                            )
                        };
                        if !accepted {
                            continue;
                        }
                    }
                }
                let Some(cpu_target) = w.bodies[source_target].as_ref() else {
                    continue;
                };
                let target_start_body = w.step_start_bodies[dense_target];
                let start_target = body_transform(&target_start_body, cpu_target.local_center);
                let end_target = body_transform(&target_end_body, cpu_target.local_center);
                if !bounds_overlap(
                    fast_bounds,
                    sweep_bounds(&target_shape, start_target, end_target),
                ) {
                    continue;
                }
                let is_sensor = target_cpu.event_flags & SHAPE_IS_SENSOR != 0;
                if is_sensor
                    && (target_cpu.event_flags & SHAPE_ENABLE_SENSOR_EVENTS == 0
                        || fast_flags & SHAPE_ENABLE_SENSOR_EVENTS == 0)
                {
                    continue;
                }
    let hits = if target_static && target_shape.kind == KIND_MESH {
                    rigid_mesh_time_of_impacts(
                        &target_shape,
                        start_target,
                        fast_shape,
                        start_fast,
                        end_fast,
                        solid_fraction,
                    )
                    .into_iter()
                    .map(|candidate| candidate.hit)
                    .collect::<Vec<_>>()
                } else {
                    rigid_time_of_impact(
                        &target_shape,
                        start_target,
                        end_target,
                        fast_shape,
                        start_fast,
                        end_fast,
                        solid_fraction,
                    )
                    .into_iter()
                    .collect()
                };
                for hit in hits {
                    if is_sensor {
                        sensor_hits.push((hit.fraction, target_shape.id, fast_shape.id));
                        continue;
                    }
                    if (target_cpu.event_flags | fast_flags) & SHAPE_ENABLE_PRE_SOLVE_EVENTS != 0 {
                        if let Some(callback) = w.pre_solve_callback {
                            let accepted = unsafe {
                                callback(
                                    target_shape.id,
                                    fast_shape.id,
                                    Vec3::from(hit.point),
                                    Vec3::from(hit.normal),
                                    w.pre_solve_context as *mut std::ffi::c_void,
                                )
                            };
                            if !accepted {
                                continue;
                            }
                        }
                    }
                    solid_fraction = hit.fraction;
                    break;
                }
            }
        }

        if trace_body == Some(source_fast + 1) {
            eprintln!("gpu-ccd-trace step={} body={} start={:?} end={:?} fraction={}",
                w.physics_step, source_fast + 1, start_fast_body.pos, end_fast_body.pos, solid_fraction);
        }
        sensor_hits.sort_by(|a, b| {
            a.0.total_cmp(&b.0)
                .then_with(|| a.1.index1.cmp(&b.1.index1))
                .then_with(|| a.2.index1.cmp(&b.2.index1))
        });
        for (_, sensor, visitor) in sensor_hits
            .into_iter()
            .filter(|hit| hit.0 < solid_fraction)
            .take(8)
        {
            w.continuous_sensor_hits.push((sensor, visitor));
        }
        if solid_fraction < 1.0 {
            let center = glam::Vec3::from_array(start_fast_body.pos)
                .lerp(glam::Vec3::from_array(end_fast_body.pos), solid_fraction);
            let qa = glam::Quat::from_array(start_fast_body.rot).normalize();
            let mut qb = glam::Quat::from_array(end_fast_body.rot).normalize();
            if qa.dot(qb) < 0.0 {
                qb = -qb;
            }
            end_fast_body.pos = center.to_array();
            end_fast_body.rot = (qa * (1.0 - solid_fraction) + qb * solid_fraction)
                .normalize()
                .to_array();
            end_fast_body.flags &= !FLAG_SLEEP;
            end_fast_body.sleep_time = 0.0;
            bodies[dense_fast] = end_fast_body;
            corrected = true;
        }
    }
    w.continuous_sensor_hits
        .sort_by_key(|(sensor, visitor)| (sensor.index1, visitor.index1));
    w.continuous_sensor_hits.dedup();
    corrected
}

fn world_needs_contact_readback(w: &WorldInner) -> bool {
    if !w.live_contacts.is_empty() || !w.sensor_overlaps.is_empty() {
        return true;
    }
    if !w.scene_dirty && !w.bodies_dirty {
        if let Some(capabilities) = w.scene_capabilities {
            return capabilities.shape_events;
        }
    }
    scan_scene_capabilities(w).shape_events
}

#[cfg(not(target_arch = "wasm32"))]
fn sync_query_mirror(w: &mut WorldInner, id: WorldId) {
    let _ = sync_world_mirror_parts(w, id, false, false, false);
}

#[cfg(not(target_arch = "wasm32"))]
fn sync_world_mirror(w: &mut WorldInner, id: WorldId) -> Vec<BodyGpu> {
    sync_world_mirror_parts(w, id, true, true, false)
}

#[cfg(not(target_arch = "wasm32"))]
fn sync_world_mirror_parts(
    w: &mut WorldInner,
    id: WorldId,
    want_joints: bool,
    want_contacts: bool,
    want_data: bool,
) -> Vec<BodyGpu> {
    let need_data = want_data && w.contact_snapshot_key != Some(contact_api::snapshot_key(w));
    let need_bodies = w.gpu_mirror_stale;
    let need_events = want_contacts && w.events_pending;
    let need_joints = want_joints && !w.joints.is_empty() && (w.gpu_mirror_stale || w.events_pending);
    if !need_bodies && !w.post_ccd_pending && !need_events && !need_data {
        return w
            .bodies
            .iter()
            .filter_map(|b| b.as_ref().map(|c| c.gpu))
            .collect();
    }
    w.gpu_mirror_stale = false;
    if w.sim.is_none() {
        w.post_ccd_pending = false;
        w.events_pending = false;
        return w
            .bodies
            .iter()
            .filter_map(|b| b.as_ref().map(|c| c.gpu))
            .collect();
    }
    let wait_start = std::time::Instant::now();
    if let Some(sim) = w.sim.as_mut() {
        sim.wait_completion();
    }
    let wait_ms = wait_start.elapsed().as_secs_f32() * 1e3;
    if let Some(sim) = w.sim.as_mut() {
        sim.set_mirror_wait_ms(wait_ms);
    }
    let fetch_start = std::time::Instant::now();
    let old_sleep: Vec<bool> = w
        .bodies
        .iter()
        .map(|body| {
            body.as_ref()
                .is_some_and(|body| body.gpu.flags & FLAG_SLEEP != 0)
        })
        .collect();
    let copy_joints = need_joints;
    let copy_contacts = need_data || (need_events && world_needs_contact_readback(w));
    let (mut out, joints, contacts, previous_touching, start_flags) = {
        let sim = w.sim.as_mut().expect("checked above");
        if need_bodies || copy_joints || copy_contacts {
            sim.read_world_mirror(copy_joints, copy_contacts)
        } else {
            (
                w.bodies
                    .iter()
                    .filter_map(|b| b.as_ref().map(|c| c.gpu))
                    .collect(),
                Vec::new(),
                Vec::new(),
                Vec::new(),
                Vec::new(),
            )
        }
    };
    let (_wait, copy_ms, map_ms, bytes) = w
        .sim
        .as_ref()
        .map(|sim| sim.last_mirror_timings())
        .unwrap_or((0.0, 0.0, 0.0, 0));
    let ccd_start = std::time::Instant::now();
    let ccd_corrected = w.post_ccd_pending && run_continuous_collision(w, id.index1, &mut out);
    if ccd_corrected {
        w.query_state = w.query_state.saturating_add(1);
        w.sim
            .as_ref()
            .expect("checked above")
            .write_body_states(&out);
    }
    w.post_ccd_pending = false;
    let ccd_ms = ccd_start.elapsed().as_secs_f32() * 1e3;
    let apply_start = std::time::Instant::now();
    let epoch = snapshot_epoch(w);
    if need_bodies { joint_reaction::save_prepared_frames(w, &out); }
    if need_bodies || ccd_corrected {
        for (source, slot) in w.bodies.iter_mut().enumerate() {
            if let Some(cpu) = slot {
                if source < out.len() {
                    if cpu.host_epoch > epoch {
                        continue;
                    }
                    // After multiple resident steps the host mirror may be old.
                    // Event transitions must use the last GPU step's start flags.
                    let was_asleep = start_flags.get(source).map(|flags|flags & FLAG_SLEEP!=0)
                        .unwrap_or_else(||old_sleep.get(source).copied().unwrap_or(false));
                    cpu.gpu = out[source];
                    let flags = cpu.gpu.flags;
                    let is_static = flags & FLAG_STATIC != 0;
                    let is_asleep = flags & FLAG_SLEEP != 0;
                    if !is_static && (!is_asleep || !was_asleep) {
                        let center_offset = quat_rotate(cpu.gpu.rot, cpu.local_center);
                        w.body_move_events.push(BodyMoveEvent {
                            user_data: cpu.user_data,
                            transform: WorldTransform {
                                p: [
                                    cpu.gpu.pos[0] - center_offset[0],
                                    cpu.gpu.pos[1] - center_offset[1],
                                    cpu.gpu.pos[2] - center_offset[2],
                                ],
                                q: cpu.gpu.rot,
                            },
                            body_id: BodyId {
                                index1: source as i32 + 1,
                                world0: id.index1,
                                generation: cpu.generation,
                            },
                            fell_asleep: !was_asleep && is_asleep,
                        });
                    }
                }
            }
        }
    }
    if copy_joints && joints.len() == w.joints.len() {
        w.joints = joints;
    }
    if need_bodies || ccd_corrected {
        w.sim.as_mut().expect("checked above").accept_body_mirror_as_pose_snapshot();
    }
    let apply_ms = apply_start.elapsed().as_secs_f32() * 1e3;
    if copy_contacts {
        contact_api::update_registry(w, id.index1, &contacts);
    }
    if need_events {
        collect_joint_events(w, id.index1);
        if copy_contacts {
            update_sensor_events(w, id.index1, &contacts);
            update_contact_events(w, id.index1, &contacts, &previous_touching);
        }
        w.events_pending = false;
    }
    w.last_gpu_fetch_ms = fetch_start.elapsed().as_secs_f32() * 1e3;
    w.last_query_profile.wait_ms = wait_ms;
    w.last_query_profile.staging_ms = copy_ms;
    w.last_query_profile.map_ms = map_ms;
    w.last_query_profile.apply_ms = apply_ms;
    w.last_query_profile.ccd_ms = ccd_ms;
    w.last_query_profile.copied_bytes = bytes;
    // A full mirror consumes the pose snapshot, so drawing may bypass its
    // clock update. Publish available timestamps here without another wait.
    w.sim.as_mut().expect("checked above").harvest_gpu_timestamps(false);
    copy_gpu_clocks(w);
    out
}

#[cfg(not(target_arch = "wasm32"))]
fn ensure_cpu_mirror_world(w: &mut WorldInner, id: WorldId) {
    if w.gpu_mirror_stale || w.events_pending || w.post_ccd_pending {
        let _ = sync_world_mirror(w, id);
    }
}

#[cfg(target_arch = "wasm32")]
fn ensure_cpu_mirror_world(_w: &mut WorldInner, _id: WorldId) {}

#[cfg(not(target_arch = "wasm32"))]
pub async fn b3_world_sync_from_gpu(id: WorldId) -> Vec<BodyGpu> {
    with_world_mut_no_sync(id, |w| sync_world_mirror(w, id)).unwrap_or_default()
}

pub fn b3_world_last_encode_ms(id: WorldId) -> f32 {
    with_world_no_sync(id, |w| w.last_gpu_encode_ms).unwrap_or(0.0)
}

pub fn b3_world_last_fetch_ms(id: WorldId) -> f32 {
    with_world_no_sync(id, |w| w.last_gpu_fetch_ms).unwrap_or(0.0)
}

pub fn b3_world_last_collide_ms(id: WorldId) -> f32 {
    with_world_no_sync(id, |w| w.last_gpu_collide_ms).unwrap_or(0.0)
}

pub fn b3_world_last_solve_ms(id: WorldId) -> f32 {
    with_world_no_sync(id, |w| w.last_gpu_solve_ms).unwrap_or(0.0)
}

pub fn b3_world_last_integrate_ms(id: WorldId) -> f32 {
    with_world_no_sync(id, |w| w.last_gpu_integrate_ms).unwrap_or(0.0)
}

/// Internal benchmark diagnostics, available only after a completed GPU wait.
#[cfg(all(feature="native-command-cache",not(target_arch="wasm32")))]
pub(crate) fn b3_world_completed_broadphase_profile_ms(id:WorldId)->Option<Vec<f64>> {
    with_world_no_sync(id,|w|w.sim.as_ref()?.completed_broadphase_profile_ms()).flatten()
}

pub fn b3_world_last_broadphase_ms(id: WorldId) -> f32 {
    with_world_no_sync(id, |w| w.last_gpu_broadphase_ms).unwrap_or(0.0)
}

pub fn b3_world_last_narrowphase_ms(id: WorldId) -> f32 {
    with_world_no_sync(id, |w| w.last_gpu_narrowphase_ms).unwrap_or(0.0)
}

pub fn b3_world_last_graph_ms(id: WorldId) -> f32 {
    with_world_no_sync(id, |w| w.last_gpu_graph_ms).unwrap_or(0.0)
}

pub fn b3_world_last_prepare_ms(id: WorldId) -> f32 {
    with_world_no_sync(id, |w| w.last_gpu_prepare_ms).unwrap_or(0.0)
}

pub fn b3_world_last_device_ms(id: WorldId) -> f32 {
    with_world_no_sync(id, |w| w.last_gpu_device_ms).unwrap_or(0.0)
}

#[cfg(not(target_arch = "wasm32"))]
fn copy_gpu_clocks(w: &mut WorldInner) {
    let Some(sim) = w.sim.as_ref() else {
        return;
    };
    let (c, s, i) = sim.gpu_pass_ms();
    w.last_gpu_collide_ms = c;
    w.last_gpu_solve_ms = s;
    w.last_gpu_integrate_ms = i;
    let (bp, np, g) = sim.gpu_stage_ms();
    w.last_gpu_broadphase_ms = bp;
    w.last_gpu_narrowphase_ms = np;
    w.last_gpu_graph_ms = g;
    w.last_gpu_prepare_ms = sim.last_gpu_prepare_ms();
    w.last_gpu_device_ms = sim.last_gpu_device_ms();
    w.last_timestamp_step = sim.last_timestamp_step();
    w.physics_step = sim.physics_step();
}

pub fn b3_world_last_timestamp_step(id: WorldId) -> u64 {
    with_world_no_sync(id, |w| w.last_timestamp_step).unwrap_or(0)
}

pub fn b3_world_physics_step(id: WorldId) -> u64 {
    with_world_no_sync(id, |w| w.physics_step).unwrap_or(0)
}

/// Last physics step known to have completed on the GPU. `None` if unknown.
pub fn b3_world_completed_step(id: WorldId) -> Option<u64> {
    with_world_mut_no_sync(id, |w| {
        if let Some(sim) = w.sim.as_mut() {
            sim.poll_completion();
            sim.completed_physics_step()
        } else {
            Some(w.physics_step)
        }
    })
    .flatten()
}

pub fn b3_world_completed_known(id: WorldId) -> bool {
    b3_world_completed_step(id).is_some()
}

pub fn b3_world_pose_snapshot_step(id: WorldId) -> u64 {
    with_world_no_sync(id, |w| {
        w.sim.as_ref().map(|sim| sim.pose_snapshot_step()).unwrap_or(0)
    })
    .unwrap_or(0)
}

pub fn b3_world_last_query_profile(id: WorldId) -> super::QueryProfile {
    with_world_no_sync(id, |w| w.last_query_profile).unwrap_or_default()
}

pub fn b3_world_gpu_joint_lists(id: WorldId) -> (bool, Vec<(u32, Vec<u32>)>) {
    with_world_mut_no_sync(id, |w| {
        w.sim
            .as_mut()
            .map(|sim| sim.read_joint_lists())
            .unwrap_or((true, Vec::new()))
    })
    .unwrap_or((true, Vec::new()))
}

pub fn b3_world_last_solver_dispatches(id: WorldId) -> u32 {
    with_world_no_sync(id, |w| {
        w.sim.as_ref().map(|s| s.last_solver_dispatches()).unwrap_or(0)
    })
    .unwrap_or(0)
}

pub fn b3_world_last_joint_dispatches(id: WorldId) -> u32 {
    with_world_no_sync(id, |w| {
        w.sim.as_ref().map(|s| s.last_joint_dispatches()).unwrap_or(0)
    })
    .unwrap_or(0)
}

pub fn b3_world_physics_invalid(id: WorldId) -> bool {
    with_world_no_sync(id, |w| w.physics_invalid).unwrap_or(false)
}

pub fn b3_world_last_encode_commands(id: WorldId) -> u32 {
    with_world_no_sync(id, |w| {
        w.sim.as_ref().map(|s| s.last_encode_commands()).unwrap_or(0)
    })
    .unwrap_or(0)
}

pub fn b3_world_last_static_sort_dispatches(id: WorldId) -> u32 {
    with_world_no_sync(id, |w| {
        w.sim.as_ref().map(|s| s.last_static_sort_dispatches()).unwrap_or(0)
    })
    .unwrap_or(0)
}

#[cfg(test)]
pub(crate) fn set_component_tgs_requested_for_test(id:WorldId,requested:bool) {
    with_world_mut_no_sync(id,|w| {
        w.component_tgs_requested=requested;
        apply_solver_topology(w);
    });
}

pub fn b3_world_set_diagnostic_flags(id: WorldId, flags: u32) {
    with_world_mut_no_sync(id, |w| {
        w.diagnostic_flags_override = Some(flags);
        w.topology_dirty = true;
        if let Some(sim) = w.sim.as_mut() {
            sim.set_diagnostic_flags(flags);
        }
        apply_solver_topology(w);
    });
}

pub fn b3_world_clear_capacity_status(id: WorldId) {
    with_world_mut_no_sync(id, |w| {
        if let Some(sim) = w.sim.as_mut() {
            sim.clear_sticky_loss();
        }
        w.gpu_fail = None;
    });
}

pub fn b3_world_pose_snapshot_maps(id: WorldId) -> u64 {
    with_world_no_sync(id, |w| {
        w.sim.as_ref().map(|sim| sim.pose_snapshot_maps()).unwrap_or(0)
    })
    .unwrap_or(0)
}

pub fn b3_world_pose_snapshot_copies(id: WorldId) -> u64 {
    b3_world_pose_snapshot_maps(id)
}

pub fn b3_world_pose_snapshot_kicks(id: WorldId) -> u64 {
    with_world_no_sync(id, |w| {
        w.sim.as_ref().map(|sim| sim.pose_snapshot_kicks()).unwrap_or(0)
    })
    .unwrap_or(0)
}

pub fn b3_world_pose_snapshot_epoch(id: WorldId) -> u64 {
    with_world_no_sync(id, |w| snapshot_epoch(w)).unwrap_or(0)
}

/// Writable joint endpoints grouped by island root. Each writable body has at
/// most one owner component after contact/joint union.
pub fn b3_world_joint_writable_owners(id: WorldId) -> Vec<(u32, u32)> {
    with_world(id, |w| {
        let n = w.bodies.len();
        let mut parent: Vec<u32> = (0..n as u32).collect();
        let find = |parent: &mut [u32], mut x: u32| {
            while parent[x as usize] != x {
                let p = parent[x as usize];
                parent[x as usize] = parent[p as usize];
                x = p;
            }
            x
        };
        let writable = |body: &crate::types::BodyGpu| {
            body.flags & (FLAG_STATIC | FLAG_KINEMATIC | FLAG_DISABLED | FLAG_SLEEP) == 0
        };
        for joint in w.joints.iter() {
            if joint.kind == JOINT_NONE || joint.kind == JOINT_FILTER {
                continue;
            }
            let a = joint.a as usize;
            let b = joint.b as usize;
            let wa = w.bodies.get(a).and_then(|s| s.as_ref()).is_some_and(|s| writable(&s.gpu));
            let wb = w.bodies.get(b).and_then(|s| s.as_ref()).is_some_and(|s| writable(&s.gpu));
            if wa && wb {
                let ra = find(&mut parent, joint.a);
                let rb = find(&mut parent, joint.b);
                let lo = ra.min(rb);
                let hi = ra.max(rb);
                parent[hi as usize] = lo;
            }
        }
        let mut owners = Vec::new();
        for (i, slot) in w.bodies.iter().enumerate() {
            let Some(body) = slot else {
                continue;
            };
            if !writable(&body.gpu) {
                continue;
            }
            owners.push((i as u32, find(&mut parent, i as u32)));
        }
        owners
    })
    .unwrap_or_default()
}

pub fn b3_world_pose_export_live(id: WorldId) -> bool {
    with_world_no_sync(id, |w| w.pose_export_live).unwrap_or(false)
}

#[cfg(not(target_arch = "wasm32"))]
pub fn b3_world_pose_export_uuid(id: WorldId) -> Option<[u8; 16]> {
    with_world_no_sync(id, |w| w.sim.as_ref().and_then(|sim| sim.pose_export_uuid())).flatten()
}

#[cfg(not(target_arch = "wasm32"))]
pub fn b3_world_pose_export(id: WorldId) -> crate::pose_gl::PoseExportC {
    with_world_mut_no_sync(id, |w| {
        if let Some(sim) = w.sim.as_ref() {
            if let Some((fd, size, count)) = sim.pose_export_fd() {
                w.pose_export_live = true;
                return crate::pose_gl::PoseExportC {
                    mode: crate::pose_gl::POSE_EXPORT_FD,
                    fd,
                    size,
                    stride: std::mem::size_of::<crate::types::BodyStateGpu>() as u32,
                    count,
                    _pad: 0,
                };
            }
        }
        w.pose_export_live = false;
        crate::pose_gl::PoseExportC {
            mode: crate::pose_gl::POSE_EXPORT_CPU,
            fd: -1,
            size: 0,
            stride: std::mem::size_of::<crate::types::BodyStateGpu>() as u32,
            count: 0,
            _pad: 0,
        }
    })
    .unwrap_or(crate::pose_gl::PoseExportC {
        mode: crate::pose_gl::POSE_EXPORT_NONE,
        fd: -1,
        size: 0,
        stride: 0,
        count: 0,
        _pad: 0,
    })
}

#[cfg(not(target_arch = "wasm32"))]
fn ensure_pose_snapshot(w: &mut WorldInner) {
    {
        let Some(sim) = w.sim.as_mut() else {
            return;
        };
        if sim.pose_snapshot_consumed() {
            return;
        }
        sim.harvest_gpu_timestamps(false);
    }
    copy_gpu_clocks(w);
    let Some(sim) = w.sim.as_mut() else {
        return;
    };
    w.pose_export_live = sim.pose_export_fd().is_some();
    let fetch_start = std::time::Instant::now();
    let Some(poses) = sim.lock_pose_snapshot() else {
        return;
    };
    let epoch = sim.staged_pose_epoch();
    w.last_gpu_fetch_ms = fetch_start.elapsed().as_secs_f32() * 1e3;
    for (source, slot) in w.bodies.iter_mut().enumerate() {
        let Some(cpu) = slot else {
            continue;
        };
        let Some(pose) = poses.get(source) else {
            continue;
        };
        if cpu.host_epoch > epoch {
            continue;
        }
        cpu.gpu.pos = pose.pos;
        cpu.gpu.rot = pose.rot;
        cpu.gpu.vel = pose.vel;
        cpu.gpu.omega = pose.omega;
        cpu.gpu.flags = pose.flags;
        cpu.gpu.sleep_time = pose.sleep_time;
    }
}

pub fn b3_world_gpu_fail(id: WorldId) -> *const std::os::raw::c_char {
    with_world_no_sync(id, |w| {
        w.gpu_fail
            .as_ref()
            .map(|s| s.as_ptr())
            .unwrap_or(std::ptr::null())
    })
    .unwrap_or(std::ptr::null())
}

pub fn b3_world_set_gpu_fail(id: WorldId, message: &str) {
    with_world_mut_no_sync(id, |w| {
        eprintln!("\n*** GPU PHYSICS FAILED ***\n{message}\n");
        w.gpu_fail = std::ffi::CString::new(message).ok();
        w.physics_invalid = true;
    });
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct WorldHealthScan {
    pub dynamic_count: i32,
    pub nan_count: i32,
    pub worst_body: i32,
    pub min_y: f32,
    pub max_y: f32,
    pub max_speed: f32,
    pub exploded: i32,
}

impl Default for WorldHealthScan {
    fn default() -> Self {
        Self {
            dynamic_count: 0,
            nan_count: 0,
            worst_body: 0,
            min_y: 0.0,
            max_y: 0.0,
            max_speed: 0.0,
            exploded: 0,
        }
    }
}

/// Enumerate every live dynamic body, including unshaped slots.
pub fn b3_world_dynamic_body_ids(id: WorldId) -> Vec<BodyId> {
    with_world(id, |w| w.bodies.iter().enumerate().filter_map(|(index, slot)| {
        let body = slot.as_ref()?;
        if body.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC) != 0 { return None; }
        Some(BodyId { index1: index as i32 + 1, world0: id.index1, generation: body.generation })
    }).collect()).unwrap_or_default()
}

pub fn b3_world_joint_health(id: WorldId) -> Vec<(i32, bool, f32, bool, f32)> {
    with_world(id, |w| w.joints.iter().enumerate().filter_map(|(index, joint)| {
        if joint.kind == JOINT_NONE { return None; }
        let a = w.bodies.get(joint.a as usize)?.as_ref()?;
        let b = w.bodies.get(joint.b as usize)?.as_ref()?;
        let ra = joint_r(a, joint.anchor_a);
        let rb = joint_r(b, joint.anchor_b);
        let delta = glam::Vec3::from_array(std::array::from_fn(|i| b.gpu.pos[i] + rb[i] - a.gpu.pos[i] - ra[i]));
        let qa = (glam::Quat::from_array(a.gpu.rot) * glam::Quat::from_array(joint.frame_a_rotation)).normalize();
        let qb = (glam::Quat::from_array(b.gpu.rot) * glam::Quat::from_array(joint.frame_b_rotation)).normalize();
        let axis = qa * glam::Vec3::X;
        let error = if matches!(joint.kind, JOINT_WHEEL | JOINT_PRISMATIC) {
            (delta - axis * delta.dot(axis)).length()
        } else { delta.length() };
        let angular = match joint.kind {
            JOINT_REVOLUTE => (qa * glam::Vec3::Z).dot(qb * glam::Vec3::Z).clamp(-1.0, 1.0).acos(),
            JOINT_WHEEL => axis.dot(qb * glam::Vec3::Z).abs().min(1.0).asin(),
            JOINT_WELD | JOINT_PRISMATIC => 2.0 * qa.dot(qb).abs().min(1.0).acos(),
            _ => 0.0,
        };
        Some((index as i32 + 1,
            matches!(joint.kind, JOINT_REVOLUTE | JOINT_SPHERICAL | JOINT_WELD | JOINT_WHEEL | JOINT_PRISMATIC), error,
            matches!(joint.kind, JOINT_REVOLUTE | JOINT_WELD | JOINT_WHEEL | JOINT_PRISMATIC), angular))
    }).collect()).unwrap_or_default()
}

pub fn b3_world_health_scan(id: WorldId) -> WorldHealthScan {
    with_world(id, |w| {
        let mut acc = WorldHealthScan {
            min_y: 1.0e9,
            max_y: -1.0e9,
            ..WorldHealthScan::default()
        };
        for (index, slot) in w.bodies.iter().enumerate() {
            let Some(body) = slot else {
                continue;
            };
            if body.gpu.flags & FLAG_STATIC != 0 || body.gpu.flags & FLAG_KINEMATIC != 0 {
                continue;
            }
            // Match native b3Body_GetPosition: GPU storage is COM, not origin.
            let p = body_origin(body);
            let v = body.gpu.vel;
            let q = body.gpu.rot;
            let wvel = body.gpu.omega;
            let speed = (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]).sqrt();
            let finite = p.iter().chain(v.iter()).chain(q.iter()).chain(wvel.iter()).all(|x| x.is_finite())
                && speed.is_finite();
            acc.dynamic_count += 1;
            if !finite {
                acc.nan_count += 1;
                acc.worst_body = (index + 1) as i32;
            }
            if p[1] < acc.min_y {
                acc.min_y = p[1];
            }
            if p[1] > acc.max_y {
                acc.max_y = p[1];
            }
            if speed > acc.max_speed {
                acc.max_speed = speed;
                if speed > 120.0 {
                    acc.worst_body = (index + 1) as i32;
                }
            }
        }
        if acc.dynamic_count == 0 {
            acc.min_y = 0.0;
            acc.max_y = 0.0;
        }
        acc.exploded = i32::from(acc.nan_count > 0 || acc.max_speed > 120.0 || acc.max_y > 200.0);
        acc
    })
    .unwrap_or_default()
}

#[cfg(not(target_arch = "wasm32"))]
fn update_sensor_events(w: &mut WorldInner, world0: u16, contacts: &[crate::types::ContactGpu]) {
    let dense_shapes = gpu_public_shapes(&w.shapes, &w.bodies);
    let mut current: HashMap<i32, Vec<ShapeId>> = HashMap::new();

    for contact in contacts {
        if contact.a == u32::MAX
            || contact.count == 0
            || (contact.lifecycle[1] & CONTACT_TOUCHING) == 0
        {
            continue;
        }
        let key = contact.pair_key();
        let dense_a = (key & 0xffff_ffff) as usize;
        let dense_b = (key >> 32) as usize;
        let (Some(&(key_source_a, key_shape_a)), Some(&(key_source_b, key_shape_b))) =
            (dense_shapes.get(dense_a), dense_shapes.get(dense_b))
        else {
            continue;
        };
        // GPU body identity is a stable slot, including holes from deletion.
        let shape_a_body_slot = key_shape_a.body_index.saturating_sub(1) as u32;
        let (source_a, shape_a, source_b, shape_b) = if shape_a_body_slot == contact.a {
            (key_source_a, key_shape_a, key_source_b, key_shape_b)
        } else {
            (key_source_b, key_shape_b, key_source_a, key_shape_a)
        };
        let sensor_a = (shape_a.event_flags & SHAPE_IS_SENSOR) != 0;
        let sensor_b = (shape_b.event_flags & SHAPE_IS_SENSOR) != 0;
        if sensor_a == sensor_b {
            continue;
        }
        let (sensor_source, sensor, visitor_source, visitor) = if sensor_a {
            (source_a, shape_a, source_b, shape_b)
        } else {
            (source_b, shape_b, source_a, shape_a)
        };
        if (sensor.event_flags & SHAPE_ENABLE_SENSOR_EVENTS) == 0
            || (visitor.event_flags & SHAPE_ENABLE_SENSOR_EVENTS) == 0
        {
            continue;
        }
        current
            .entry(sensor_source as i32 + 1)
            .or_default()
            .push(ShapeId {
                index1: visitor_source as i32 + 1,
                world0,
                generation: visitor.generation,
            });
    }

    for &(sensor_shape_id, visitor_shape_id) in &w.continuous_sensor_hits {
        current
            .entry(sensor_shape_id.index1)
            .or_default()
            .push(visitor_shape_id);
    }

    for visitors in current.values_mut() {
        visitors.sort_by_key(|id| (id.index1, id.generation));
        visitors.dedup();
    }

    for (source, shape) in w.shapes.iter().enumerate() {
        let Some(shape) = shape.as_ref() else {
            continue;
        };
        if (shape.event_flags & SHAPE_IS_SENSOR) == 0 {
            continue;
        }
        let sensor_index1 = source as i32 + 1;
        let sensor_shape_id = ShapeId {
            index1: sensor_index1,
            world0,
            generation: shape.generation,
        };
        let previous = w
            .sensor_overlaps
            .get(&sensor_index1)
            .map(Vec::as_slice)
            .unwrap_or_default();
        let next = current
            .get(&sensor_index1)
            .map(Vec::as_slice)
            .unwrap_or_default();
        let mut old_index = 0;
        let mut new_index = 0;
        while old_index < previous.len() || new_index < next.len() {
            if new_index == next.len()
                || (old_index < previous.len()
                    && (previous[old_index].index1, previous[old_index].generation)
                        < (next[new_index].index1, next[new_index].generation))
            {
                w.sensor_end_events.push(SensorEndTouchEvent {
                    sensor_shape_id,
                    visitor_shape_id: previous[old_index],
                });
                old_index += 1;
            } else if old_index == previous.len()
                || (next[new_index].index1, next[new_index].generation)
                    < (previous[old_index].index1, previous[old_index].generation)
            {
                w.sensor_begin_events.push(SensorBeginTouchEvent {
                    sensor_shape_id,
                    visitor_shape_id: next[new_index],
                });
                new_index += 1;
            } else {
                old_index += 1;
                new_index += 1;
            }
        }
    }
    w.sensor_overlaps = current;
}

#[cfg(not(target_arch = "wasm32"))]
fn update_contact_events(w: &mut WorldInner, world0: u16, contacts: &[crate::types::ContactGpu], previous_touching: &[u64]) {
    let dense_shapes = gpu_public_shapes(&w.shapes, &w.bodies);
    // GPU previous-step union, independent of when the host last read events.
    let mut previous_pairs: HashMap<ContactKey, LiveContact> = HashMap::new();
    for &key in previous_touching {
        if key == u64::MAX { continue; }
        let (Some(&id_a), Some(&id_b)) = (
            w.previous_contact_shape_ids.get((key & 0xffff_ffff) as usize),
            w.previous_contact_shape_ids.get((key >> 32) as usize)
        ) else { continue; };
        let a = id_a.index1.saturating_sub(1) as usize;
        let b = id_b.index1.saturating_sub(1) as usize;
        let (Some(sa), Some(sb)) = (
            w.shapes.get(a).and_then(Option::as_ref), w.shapes.get(b).and_then(Option::as_ref)
        ) else { continue; };
        if sa.generation != id_a.generation || sb.generation != id_b.generation { continue; }
        if (sa.event_flags | sb.event_flags) & SHAPE_IS_SENSOR != 0 { continue; }
        let public_key = keyed_contact(((a.max(b) as u64) << 32) | a.min(b) as u64,
            key, &w.previous_contact_child_ordinals);
        if previous_pairs.contains_key(&public_key) { continue; }
        let previous = if let Some(live) = w.live_contacts.get(&public_key).or_else(|| w.contact_registry.get(&public_key).map(|e| &e.live)) {
            *live
        } else {
            // The begin may never have been read. Latest-step end still exists.
            let contact_id = contact_api::fresh_id(world0);
            LiveContact {
                shape_id_a: ShapeId { index1: a as i32 + 1, world0, generation: sa.generation },
                shape_id_b: ShapeId { index1: b as i32 + 1, world0, generation: sb.generation },
                contact_id,
                events_enabled: (sa.event_flags | sb.event_flags) & SHAPE_ENABLE_CONTACT_EVENTS != 0,
            }
        };
        previous_pairs.insert(public_key, previous);
    }
    let mut current: HashMap<ContactKey, LiveContact> = HashMap::new();
    let mut starts = Vec::new();
    let mut started_pairs = std::collections::HashSet::new();
    let mut hit_indices: HashMap<ContactKey, usize> = HashMap::new();

    for contact in contacts {
        let flags = contact.lifecycle[1];
        if contact.a == u32::MAX || contact.count == 0 || (flags & CONTACT_TOUCHING) == 0 {
            continue;
        }
        let key = contact.pair_key();
        let dense_a = (key & 0xffff_ffff) as usize;
        let dense_b = (key >> 32) as usize;
        let (Some(&(key_source_a, key_shape_a)), Some(&(key_source_b, key_shape_b))) =
            (dense_shapes.get(dense_a), dense_shapes.get(dense_b))
        else {
            continue;
        };
        // GPU body identity is a stable slot, including holes from deletion.
        let shape_a_body_slot = key_shape_a.body_index.saturating_sub(1) as u32;
        let (source_a, shape_a, source_b, shape_b) = if shape_a_body_slot == contact.a {
            (key_source_a, key_shape_a, key_source_b, key_shape_b)
        } else {
            (key_source_b, key_shape_b, key_source_a, key_shape_a)
        };
        if ((shape_a.event_flags | shape_b.event_flags) & SHAPE_IS_SENSOR) != 0 {
            continue;
        }
        let shape_id_a = ShapeId {
            index1: source_a as i32 + 1,
            world0,
            generation: shape_a.generation,
        };
        let shape_id_b = ShapeId {
            index1: source_b as i32 + 1,
            world0,
            generation: shape_b.generation,
        };
        let source_key = keyed_contact(((source_a.max(source_b) as u64) << 32) | source_a.min(source_b) as u64,
            key, &w.contact_child_ordinals);
        let events_enabled =
            ((shape_a.event_flags | shape_b.event_flags) & SHAPE_ENABLE_CONTACT_EVENTS) != 0;
        if !events_enabled && (shape_a.event_flags | shape_b.event_flags) & SHAPE_ENABLE_HIT_EVENTS == 0 {
            continue;
        }
        let live = if let Some(previous) = current.get(&source_key).or_else(|| w.contact_registry.get(&source_key).map(|e| &e.live)).or_else(|| previous_pairs.get(&source_key)).or_else(|| w.live_contacts.get(&source_key)).copied() {
            previous
        } else {
            let contact_id = contact_api::fresh_id(world0);
            let live = LiveContact {
                shape_id_a,
                shape_id_b,
                contact_id,
                events_enabled,
            };
            live
        };
        // Track actual GPU start transitions, including when a child slot was
        // visited before its root. A first late read must not invent a begin.
        if events_enabled && (flags & CONTACT_START_TOUCHING) != 0 && started_pairs.insert(source_key) {
            starts.push(source_key);
        }
        if ((shape_a.event_flags | shape_b.event_flags) & SHAPE_ENABLE_HIT_EVENTS) != 0 {
            let mut best_point = None;
            let mut approach_speed = w.def.hit_event_threshold;
            let body_a = w.bodies.get(contact.a as usize).and_then(Option::as_ref);
            let body_b = w.bodies.get(contact.b as usize).and_then(Option::as_ref);
            if let (Some(body_a), Some(body_b)) = (body_a, body_b) {
                let ras = [contact.ra0, contact.ra1, contact.ra2, contact.ra3];
                let rbs = [contact.rb0, contact.rb1, contact.rb2, contact.rb3];
                for point_index in 0..contact.count.min(4) as usize {
                    let speed = -f32::from_bits(contact._tail[point_index]);
                    if speed > approach_speed && contact.total_normal_impulse[point_index] > 0.0 {
                        approach_speed = speed;
                        best_point = Some([
                            0.5 * (body_a.gpu.pos[0]
                                + ras[point_index][0]
                                + body_b.gpu.pos[0]
                                + rbs[point_index][0]),
                            0.5 * (body_a.gpu.pos[1]
                                + ras[point_index][1]
                                + body_b.gpu.pos[1]
                                + rbs[point_index][1]),
                            0.5 * (body_a.gpu.pos[2]
                                + ras[point_index][2]
                                + body_b.gpu.pos[2]
                                + rbs[point_index][2]),
                        ]);
                    }
                }
            }
            if let Some(point) = best_point {
                let material_a = if shape_a.kind == KIND_MESH {
                    shape_a
                        .mesh_materials
                        .get(contact.material_index as usize)
                        .map_or(shape_a.user_material_id, |material| {
                            material.user_material_id
                        })
                } else {
                    shape_a.user_material_id
                };
                let material_b = if shape_b.kind == KIND_MESH {
                    shape_b
                        .mesh_materials
                        .get(contact.material_index as usize)
                        .map_or(shape_b.user_material_id, |material| {
                            material.user_material_id
                        })
                } else {
                    shape_b.user_material_id
                };
                let event = ContactHitEvent {
                    shape_id_a: live.shape_id_a,
                    shape_id_b: live.shape_id_b,
                    contact_id: live.contact_id,
                    point,
                    normal: if shape_id_a == live.shape_id_a { [contact.nx, contact.ny, contact.nz] }
                        else { [-contact.nx, -contact.ny, -contact.nz] },
                    approach_speed,
                    user_material_id_a: if shape_id_a == live.shape_id_a { material_a } else { material_b },
                    user_material_id_b: if shape_id_a == live.shape_id_a { material_b } else { material_a },
                };
                if let Some(&index) = hit_indices.get(&source_key) {
                    if event.approach_speed > w.contact_hit_events[index].approach_speed {
                        w.contact_hit_events[index] = event;
                    }
                } else {
                    hit_indices.insert(source_key, w.contact_hit_events.len());
                    w.contact_hit_events.push(event);
                }
            }
        }
        current.insert(source_key, live);
    }

    for key in starts {
        if !previous_pairs.contains_key(&key) {
            if let Some(live) = current.get(&key) {
                w.contact_begin_events.push(ContactBeginTouchEvent {
                    shape_id_a: live.shape_id_a, shape_id_b: live.shape_id_b, contact_id: live.contact_id,
                });
            }
        }
    }

    for (key, previous) in previous_pairs {
        if !current.contains_key(&key) && previous.events_enabled {
            w.contact_end_events.push(ContactEndTouchEvent {
                shape_id_a: previous.shape_id_a,
                shape_id_b: previous.shape_id_b,
                contact_id: previous.contact_id,
            });
        }
    }
    w.live_contacts = current;

    // End transitions above use the union of touching public pairs. A stopped
    // compound child must not end the pair while another child still touches.

}

#[cfg(not(target_arch = "wasm32"))]
pub async fn b3_world_sync_contacts(id: WorldId) -> Vec<crate::types::ContactGpu> {
    let mut worlds = lock_worlds();
    let Some(w) = slot_mut(&mut worlds, id) else {
        return Vec::new();
    };
    if let Some(sim) = w.sim.as_mut() {
        sim.read_contacts().await
    } else {
        Vec::new()
    }
}

#[cfg(not(target_arch = "wasm32"))]
pub async fn b3_world_sync_phase_words(id: WorldId) -> Vec<u32> {
    let mut worlds = lock_worlds();
    let Some(w) = slot_mut(&mut worlds, id) else {
        return Vec::new();
    };
    if let Some(sim) = w.sim.as_mut() {
        sim.read_phase_words().await
    } else {
        Vec::new()
    }
}

#[cfg(not(target_arch = "wasm32"))]
pub async fn b3_world_sync_mesh_candidate_words(id: WorldId) -> Vec<u32> {
    let mut worlds = lock_worlds();
    let Some(w) = slot_mut(&mut worlds, id) else {
        return Vec::new();
    };
    if let Some(sim) = w.sim.as_mut() {
        sim.read_mesh_candidate_words().await
    } else {
        Vec::new()
    }
}

#[cfg(not(target_arch = "wasm32"))]
#[cfg(not(target_arch = "wasm32"))]
pub async fn b3_world_live_step_stats(id: WorldId) -> Option<crate::sim::LiveStepStats> {
    let mut worlds = lock_worlds();
    let Some(w) = slot_mut(&mut worlds, id) else {
        return None;
    };
    if let Some(sim) = w.sim.as_mut() {
        let stats = sim.read_live_step_stats().await;
        if stats.capacity_loss() {
            w.gpu_fail = std::ffi::CString::new(format!(
                "capacity loss first_step={} {} contact_reasons={:#x}",
                stats.first_fail_step,
                stats.sticky.loss_detail(),
                stats.contact_drop_reasons
            ))
            .ok();
        }
        Some(stats)
    } else {
        None
    }
}

pub fn b3_world_body_count(id: WorldId) -> u32 {
    with_world(id, |w| {
        w.bodies.iter().filter(|b| b.is_some()).count() as u32
    })
    .unwrap_or(0)
}

pub fn b3_world_step_encoder(
    id: WorldId,
    dt: f32,
    sub_step_count: i32,
    _encoder: &mut wgpu::CommandEncoder,
) {
    // The external encoder was never used for submission. Share one step
    // owner so capture, CPU/GPU CCD selection and finalization cannot diverge.
    step_gpu_inner(id, dt, sub_step_count);
}

/// Returns immutable geometry only when topology changes. Never completes a
/// submission or downloads poses. A renderer must obtain current body buffers
/// independently and use body slots, including holes, for transform fetches.
pub fn b3_world_render_scene(
    id: WorldId, previous: Option<crate::render_scene::RenderSceneKey>,
) -> Option<crate::render_scene::RenderScene> {
    use crate::render_scene::{RenderGeometry, RenderScene, RenderSceneKey, RenderShape};
    with_world_no_sync(id, |w| {
        let key = RenderSceneKey { world: id, topology: w.query_topology };
        if previous == Some(key) { return None; }
        let mut shapes = Vec::new();
        for (index, slot) in w.shapes.iter().enumerate() {
            let Some(shape) = slot else { continue; };
            if shape.public_kind == PUBLIC_KIND_COMPOUND { continue; }
            let Some(body_slot) = shape.body_index.checked_sub(1).map(|i| i as usize) else { continue; };
            let Some(body) = w.bodies.get(body_slot).and_then(Option::as_ref) else { continue; };
            let geometry = match shape.kind {
                KIND_SPHERE => RenderGeometry::Sphere { center: shape.geometry_center, radius: shape.half[0] },
                KIND_CAPSULE => RenderGeometry::Capsule { center: shape.geometry_center, half_axis: shape.axis, radius: shape.half[0] },
                KIND_BOX => RenderGeometry::Box { center: shape.geometry_center, half: shape.half },
                KIND_CONVEX_HULL => RenderGeometry::Hull { points: Arc::new(shape.hull_points.clone()),
                    planes: Arc::new(shape.hull_planes.clone()), topology: Arc::new(shape.hull_topology.clone()) },
                KIND_MESH => {
                    let (position, rotation, scale) = shape.mesh_instance
                        .map(|m| (m.position, m.rotation, m.scale))
                        .unwrap_or(([0.0; 3], [0.0, 0.0, 0.0, 1.0], [1.0; 3]));
                    RenderGeometry::Mesh { vertices: shape.mesh_vertices.clone(), triangles: shape.mesh_triangles.clone(),
                        position, rotation, scale }
                }
                _ => panic!("unsupported render collider kind {}", shape.kind),
            };
            let collider = ShapeId { index1: index as i32 + 1, world0: id.index1, generation: shape.generation };
            let public_shape = if shape.compound_parent > 0 {
                let parent = w.shapes[(shape.compound_parent - 1) as usize].as_ref().expect("compound parent");
                ShapeId { index1: shape.compound_parent, world0: id.index1, generation: parent.generation }
            } else { collider };
            shapes.push(RenderShape { body_slot: body_slot as u32,
                body: BodyId { index1: body_slot as i32 + 1, world0: id.index1, generation: body.generation },
                collider, public_shape, child_index: shape.compound_child_index, geometry,
                custom_color: shape.custom_color, is_sensor: shape.event_flags & SHAPE_IS_SENSOR != 0, materials: shape.mesh_materials.clone() });
        }
        Some(RenderScene { key, shapes })
    }).flatten()
}

/// Finalize compatibility cohorts for drawing; resident convex worlds need
/// no CPU mirror. Synchronous public body getters retain their explicit wait.
pub fn b3_world_finalize_render_state(id: WorldId) {
    with_world_mut_no_sync(id, |w| {
        if !can_submit_resident(w) {ensure_cpu_mirror_world(w,id);}
    });
}

/// Host-side framing bounds for initial camera setup, without synchronization.
/// Excludes static supports so their large extents do not dwarf active objects.
pub fn b3_world_render_framing_bounds(id: WorldId) -> Option<([f32; 3], [f32; 3])> {
    with_world_no_sync(id, |w| {
        let mut lower = glam::Vec3::splat(f32::INFINITY);
        let mut upper = glam::Vec3::splat(f32::NEG_INFINITY);
        for body in w.bodies.iter().flatten() {
            if body.gpu.flags & (FLAG_STATIC | FLAG_DISABLED | FLAG_HIDDEN) != 0 { continue; }
            let p = glam::Vec3::from_array(body.gpu.pos);
            let extent = glam::Vec3::splat(body.max_extent.max(0.5));
            lower = lower.min(p - extent); upper = upper.max(p + extent);
        }
        (lower.is_finite() && upper.is_finite()).then_some((lower.to_array(), upper.to_array()))
    }).flatten()
}

/// Current GPU buffers plus their slot span. This metadata read never waits.
/// Callers needing CPU CCD finalization must request it explicitly first.
pub fn b3_world_render_buffers(id: WorldId) -> Option<(wgpu::Buffer, wgpu::Buffer, u32)> {
    with_world_no_sync(id, |w| w.sim.as_ref().map(|sim| (
        sim.current_bodies_buffer().clone(), sim.body_cold_buffer().clone(), sim.count,
    ))).flatten()
}

pub fn b3_world_current_buffers(id: WorldId) -> Option<(wgpu::Buffer, wgpu::Buffer)> {
    with_world_no_sync(id, |w| {
        w.sim.as_ref().map(|s| {
            (
                s.current_bodies_buffer().clone(),
                s.body_cold_buffer().clone(),
            )
        })
    })
    .flatten()
}

pub fn b3_world_gpu_report_name(id: WorldId) -> String {
    with_world_no_sync(id, |w| w.gpu.report.name.clone()).unwrap_or_default()
}

pub fn b3_world_contacts_enabled(id: WorldId) -> bool {
    with_world(id, |w| w.enable_contacts).unwrap_or(true)
}

pub fn b3_world_contact_epoch(id: WorldId) -> u64 {
    with_world(id, |w| w.sim.as_ref().map(|sim| sim.contact_epoch).unwrap_or(0)).unwrap_or(0)
}

fn disabled_body_gpu() -> BodyGpu {
    let mut body = BodyGpu::zeroed();
    body.flags = FLAG_DISABLED | FLAG_STATIC | FLAG_SLEEP;
    body
}

fn apply_solver_topology(w: &mut WorldInner) {
    let force = w
        .sim
        .as_ref()
        .is_some_and(|s| s.params().diagnostic_flags & crate::types::DIAG_RECOMPUTE_TOPOLOGY != 0);
    if w.topology_dirty || w.cached_topology.is_none() || force {
        w.cached_topology = Some(scan_topology_bounds(w));
        w.topology_dirty = false;
    }
    let bounds = w.cached_topology.unwrap_or_default();
    let jacobi = w.jacobi;
    if let Some(sim) = w.sim.as_mut() {
        sim.set_topology_bounds(bounds, u32::from(jacobi));
    }
}

fn scan_topology_bounds(w: &WorldInner) -> TopologyBounds {
    let mut shape_count = 0u32;
    let mut mesh_shapes = 0u32;
    let mut non_dynamic_shapes = 0u32;
    let mut writable_shapes: Vec<u32> = vec![0; w.bodies.len()];
    for shape in w.shapes.iter().flatten() {
        shape_count += 1;
        if shape.kind == KIND_MESH {
            mesh_shapes += 1;
        }
        let body_i = shape.body_index.saturating_sub(1) as usize;
        let Some(body) = w.bodies.get(body_i).and_then(|b| b.as_ref()) else {
            non_dynamic_shapes = non_dynamic_shapes.saturating_add(1);
            continue;
        };
        if gpu_is_non_dynamic(body.gpu.flags) {
            non_dynamic_shapes = non_dynamic_shapes.saturating_add(1);
        } else if let Some(slot) = writable_shapes.get_mut(body_i) {
            *slot = slot.saturating_add(1);
        }
    }
    let n = non_dynamic_shapes;
    let skip_general_static_sort = mesh_shapes == 0
        && writable_shapes.iter().all(|&d| match (d as u64).checked_mul(n as u64) {
            Some(prod) => prod <= 1,
            None => false,
        });
    let pair_bound = match (shape_count as u64).checked_mul(shape_count.saturating_sub(1) as u64) {
        Some(prod) => prod / 2,
        None => u64::MAX,
    };
    TopologyBounds {
        shape_count,
        mesh_shapes,
        // Shader joint indices address the sparse slot buffer. Destroyed joints
        // leave holes; a live count would exclude later slots after a re-grab.
        joints: w.joints.len() as u32,
        non_dynamic_shapes,
        skip_general_static_sort,
        static_degree_two_proof: mesh_shapes==0 && writable_shapes.iter().all(|&d| u64::from(d)*u64::from(n)<=2),
        one_group_pair_ok: pair_bound <= 64,
    }
}

fn pack_gpu_slots(w: &WorldInner) -> (Vec<BodyGpu>, Vec<[f32; 3]>, Vec<[f32; 3]>) {
    let mut bodies = Vec::with_capacity(w.bodies.len());
    let mut centers = Vec::with_capacity(w.bodies.len());
    let mut offdiag = Vec::with_capacity(w.bodies.len());
    for slot in &w.bodies {
        match slot {
            Some(cpu) => {
                let mut body = cpu.gpu;
                body.sleep_threshold = cpu.sleep_threshold;
                bodies.push(body);
                centers.push(cpu.local_center);
                let inverse = invert_symmetric(cpu.local_inertia);
                offdiag.push([inverse[3], inverse[4], inverse[5]]);
            }
            None => {
                bodies.push(disabled_body_gpu());
                centers.push([0.0; 3]);
                offdiag.push([0.0; 3]);
            }
        }
    }
    (bodies, centers, offdiag)
}

fn scan_scene_capabilities(w: &WorldInner) -> SceneCapabilities {
    const EVENT_MASK: u32 = SHAPE_ENABLE_CONTACT_EVENTS | SHAPE_ENABLE_SENSOR_EVENTS
        | SHAPE_ENABLE_HIT_EVENTS | SHAPE_IS_SENSOR;
    let mut result = SceneCapabilities {
        pending_forces: false, shape_events: false, ccd_shapes: true, component_bodies: true, has_bullet: false,
    };
    for shape in w.shapes.iter().flatten() {
        result.shape_events |= shape.event_flags & EVENT_MASK != 0;
        result.ccd_shapes &= shape.kind != KIND_MESH && shape.event_flags & SHAPE_IS_SENSOR == 0;
    }
    for body in w.bodies.iter().flatten() {
        result.pending_forces |= body.force != [0.0;3] || body.torque != [0.0;3];
        result.has_bullet |= body.gpu.flags & FLAG_BULLET != 0;
        result.component_bodies &= body.gpu.flags & FLAG_STATIC != 0
            || (body.gpu.flags & (FLAG_KINEMATIC | FLAG_DISABLED) == 0 && body.gpu.inv_mass > 0.0);
    }
    result
}

fn scene_capabilities(w: &WorldInner) -> SceneCapabilities {
    if !w.scene_dirty && !w.bodies_dirty {
        if let Some(value) = w.scene_capabilities { return value; }
    }
    scan_scene_capabilities(w)
}

// Opt-in integration while convex CCD is validated. Default remains CPU CCD.
fn configure_convex_ccd(w:&mut WorldInner,id:WorldId,refresh:bool) {
    let callbacks=w.custom_filter_callback.is_some() || w.pre_solve_callback.is_some();
    let capabilities=scene_capabilities(w);
    let has_bullet=capabilities.has_bullet;
    let key=(w.query_topology,w.def.enable_continuous,callbacks,has_bullet);
    if !refresh && w.gpu_ccd_cache_key==Some(key) { return; }
    w.gpu_ccd_cache_key=Some(key);
    if !w.gpu_ccd_requested { return; }
    let eligible=w.def.enable_continuous && !callbacks
        && !has_bullet
        && capabilities.ccd_shapes;
    if !eligible {
        if let Some(sim)=w.sim.as_mut() { sim.set_convex_ccd(None); }
        return;
    }
    use crate::ccd::{ConvexBody,ConvexShape,ConvexScene};
    let mut scene=ConvexScene{bodies:Vec::new(),shapes:Vec::new(),points:Vec::new(),
        indices:Vec::new(),targets_first:0,targets_count:0,joints_first:0,joints_count:0};
    let mut own=vec![Vec::new();w.bodies.len()];
    let mut targets=Vec::new();
    // Source-slot order is retained for both own shapes and target shapes.
    for (index,slot) in w.shapes.iter().enumerate() {
        let Some(shape)=slot.as_ref() else {continue;};
        let Some(host)=host_shape(w,id.index1,index,shape) else {continue;};
        let body=(shape.body_index-1) as usize;
        let first=scene.points.len() as u32;
        scene.points.extend(host.local_points.iter().map(|p|[p[0],p[1],p[2],0.0]));
        let sweep_radius=host.local_points.iter().map(|p|glam::Vec3::from_array(*p).length()+host.radius).fold(host.radius,f32::max);
        let number=scene.shapes.len() as u32;
        scene.shapes.push(ConvexShape{first,count:host.local_points.len() as u32,radius:host.radius,unused:0,
            body:body as u32,group:host.filter.group_index,
            category:[host.filter.category_bits as u32,(host.filter.category_bits>>32) as u32],
            mask:[host.filter.mask_bits as u32,(host.filter.mask_bits>>32) as u32],sweep_radius,unused2:0});
        own[body].push(number);
        if w.bodies[body].as_ref().unwrap().gpu.flags & (FLAG_STATIC|FLAG_KINEMATIC)!=0 {targets.push(number);}
    }
    for (i,slot) in w.bodies.iter().enumerate() {
        let (min_extent,max_extent,center)=slot.as_ref().map(|b|(b.min_extent,b.max_extent,b.local_center)).unwrap_or((0.0,0.0,[0.0;3]));
        scene.bodies.push(ConvexBody{first:scene.indices.len() as u32,count:own[i].len() as u32,
            min_extent,max_extent,center:[center[0],center[1],center[2],0.0]});
        scene.indices.extend_from_slice(&own[i]);
    }
    scene.targets_first=scene.indices.len() as u32;scene.targets_count=targets.len() as u32;
    scene.indices.extend(targets);
    scene.joints_first=scene.indices.len() as u32;
    for joint in &w.joints {
        if joint.kind!=JOINT_NONE && joint.flags & JOINT_COLLIDE_CONNECTED==0 {
            scene.indices.extend_from_slice(&[joint.a,joint.b]);scene.joints_count+=1;
        }
    }
    if let Some(sim)=w.sim.as_mut() {sim.set_convex_ccd(Some(&scene));}
}

fn world_capacity_hints(w: &WorldInner) -> (u32, u32) {
    (
        (w.def.capacity.static_body_count.max(0) as u32).saturating_add(w.def.capacity.dynamic_body_count.max(0) as u32),
        (w.def.capacity.static_shape_count.max(0) as u32).saturating_add(w.def.capacity.dynamic_shape_count.max(0) as u32),
    )
}

fn ensure_sim(w: &mut WorldInner, bodies: &[BodyGpu], n: u32, h: f32, step_dt: f32, upload_params: bool) {
    let need_scene = w.sim.is_none() || w.scene_dirty;
    // Refresh before uploads consume the dirty bits. Read paths use the scan
    // while dirty, so an edit cannot expose a previously cached eligibility.
    if need_scene || w.bodies_dirty || w.scene_capabilities.is_none() {
        w.scene_capabilities = Some(scan_scene_capabilities(w));
    }
    let capabilities = w.scene_capabilities.unwrap();
    if need_scene {
        w.gpu_ccd_cache_key=None;
        w.topology_dirty = true;
        let joints = w.joints.clone();
        let body_map: Vec<u32> = w
            .bodies
            .iter()
            .enumerate()
            .map(|(source, body)| {
                if body.is_some() {
                    source as u32
                } else {
                    u32::MAX
                }
            })
            .collect();
        let body_local_centers: Vec<[f32; 3]> = w
            .bodies
            .iter()
            .map(|body| {
                body.as_ref()
                    .map(|body| body.local_center)
                    .unwrap_or([0.0; 3])
            })
            .collect();
        let body_inv_inertia_offdiag: Vec<[f32; 3]> = w
            .bodies
            .iter()
            .map(|body| {
                body.as_ref()
                    .map(|body| {
                        let inverse = invert_symmetric(body.local_inertia);
                        [inverse[3], inverse[4], inverse[5]]
                    })
                    .unwrap_or([0.0; 3])
            })
            .collect();
        // Local bounds change only with scene topology, not with GPU poses.
        let mut compound_bounds: HashMap<i32, (glam::Vec3, glam::Vec3)> = HashMap::new();
        for child in w.shapes.iter().flatten().filter(|s| s.compound_parent != 0) {
            let center = glam::Vec3::from_array(child.geometry_center);
            let extent = match child.kind {
                KIND_SPHERE => glam::Vec3::splat(child.half[0]),
                KIND_CAPSULE => glam::Vec3::from_array(child.axis).abs() + glam::Vec3::splat(child.half[0]),
                _ => glam::Vec3::from_array(child.half),
            };
            let bounds = compound_bounds.entry(child.compound_parent)
                .or_insert((center - extent, center + extent));
            bounds.0 = bounds.0.min(center - extent);
            bounds.1 = bounds.1.max(center + extent);
        }
        let mut shapes = Vec::new();
        let mut hull_points = Vec::new();
        let mut hull_planes = Vec::new();
        let mut hull_edges = Vec::new();
        let mut hull_topology = Vec::new();
        let mut mesh_vertices = Vec::new();
        let mut mesh_triangles = Vec::new();
        let mut mesh_nodes = Vec::new();
        let mut surface_materials = Vec::new();
        let mut mesh_ranges: HashMap<(usize, usize, usize), (u32, u32, u32)> = HashMap::new();
        for (cpu_index, shape) in w.shapes.iter().enumerate() {
            let Some(shape) = shape.as_ref() else {
                continue;
            };
            let is_mesh = shape.kind == KIND_MESH;
            let geometry_key = (Arc::as_ptr(&shape.mesh_vertices) as usize,
                Arc::as_ptr(&shape.mesh_triangles) as usize, Arc::as_ptr(&shape.mesh_nodes) as usize);
            let shared_range = if is_mesh { mesh_ranges.get(&geometry_key).copied() } else { None };
            let hull_slot = if shape.hull_points.is_empty() {
                if is_mesh {
                    mesh_vertices.len() as u32
                } else {
                    u32::MAX
                }
            } else {
                hull_points.len() as u32
            };
            let plane_slot = if shape.hull_planes.is_empty() {
                if is_mesh {
                    mesh_triangles.len() as u32
                } else {
                    u32::MAX
                }
            } else {
                hull_planes.len() as u32
            };
            let edge_slot = if shape.hull_edges.is_empty() {
                if is_mesh {
                    // A node is packed as lower+upper vec4s; shaders address nodes.
                    (mesh_nodes.len() / 2) as u32
                } else {
                    u32::MAX
                }
            } else {
                hull_edges.len() as u32
            };
            let topology_slot = if shape.hull_topology.is_empty() {
                u32::MAX
            } else {
                hull_topology.len() as u32
            };
            let (hull_slot, plane_slot, edge_slot) = shared_range.unwrap_or((hull_slot, plane_slot, edge_slot));
            let material_slot = surface_materials.len() as u32;
            if let Some(mut gpu_shape) = shape.gpu(
                &body_map,
                hull_slot,
                plane_slot,
                edge_slot,
                topology_slot,
                material_slot,
                shapes.len() as u32,
                cpu_index as u32,
            ) {
                if shape.public_kind == PUBLIC_KIND_COMPOUND {
                    if let Some(&(lo, hi)) = compound_bounds.get(&(cpu_index as i32 + 1)) {
                        gpu_shape.local_center = ((lo + hi) * 0.5).to_array();
                        gpu_shape.half = ((hi - lo) * 0.5).to_array();
                    }
                }
                shapes.push(gpu_shape);
                surface_materials.extend(shape.mesh_materials.iter().map(|material| {
                    SurfaceMaterialGpu {
                        friction: material.friction,
                        restitution: material.restitution,
                        rolling_resistance: material.rolling_resistance,
                        _pad0: 0.0,
                        tangent_velocity: material.tangent_velocity,
                        custom_color: material.custom_color,
                        user_material_id_lo: material.user_material_id as u32,
                        user_material_id_hi: (material.user_material_id >> 32) as u32,
                        _pad1: [0; 2],
                    }
                }));
                hull_points.extend(
                    shape
                        .hull_points
                        .iter()
                        .map(|point| [point[0], point[1], point[2], 0.0]),
                );
                hull_planes.extend_from_slice(&shape.hull_planes);
                hull_edges.extend(
                    shape
                        .hull_edges
                        .iter()
                        .map(|edge| [edge[0], edge[1], edge[2], 0.0]),
                );
                hull_topology.extend_from_slice(&shape.hull_topology);
                if shared_range.is_none() {
                    if is_mesh { mesh_ranges.insert(geometry_key, (hull_slot, plane_slot, edge_slot)); }
                mesh_vertices.extend(
                    shape
                        .mesh_vertices
                        .iter()
                        .map(|point| [point[0], point[1], point[2], 0.0]),
                );
                mesh_triangles.extend_from_slice(&shape.mesh_triangles);
                mesh_nodes.extend(shape.mesh_nodes.iter().flat_map(|node| {
                    [
                        [
                            node.lower[0],
                            node.lower[1],
                            node.lower[2],
                            f32::from_bits(node.data),
                        ],
                        [
                            node.upper[0],
                            node.upper[1],
                            node.upper[2],
                            f32::from_bits(node.triangle_offset),
                        ],
                    ]
                }));
                }
            }
        }
        let mesh_node_live = (mesh_nodes.len() / 2) as u32;
        let live = GpuSceneCaps::live(
            n,
            shapes.len() as u32,
            hull_points.len() as u32,
            hull_planes.len() as u32,
            hull_edges.len() as u32,
            hull_topology.len() as u32,
            mesh_vertices.len() as u32,
            mesh_triangles.len() as u32,
            mesh_node_live,
            surface_materials.len() as u32,
            joints.len() as u32,
        );
        let mix_pairs = match build_mix_pairs(w, &shapes) {
            Ok(table) => table,
            Err(err) => {
                eprintln!("\n*** GPU PHYSICS FAILED ***\n{err}\n");
                w.gpu_fail = std::ffi::CString::new(err).ok();
                w.physics_invalid = true;
                if let Some(sim) = w.sim.as_mut() {
                    sim.mark_physics_invalid();
                }
                return;
            }
        };
        let proxy_indices: HashMap<i32, u32> = shapes.iter().enumerate()
            .filter(|(_, s)| s.event_flags & SHAPE_PUBLIC_PROXY != 0)
            .map(|(i, s)| (s._pad_filter[1] as i32 + 1, i as u32)).collect();
        for gpu_shape in &mut shapes {
            let cpu = w.shapes[gpu_shape._pad_filter[1] as usize].as_ref().unwrap();
            if cpu.compound_parent != 0 {
                gpu_shape._pad_filter[0] = proxy_indices[&cpu.compound_parent];
            }
        }
        let identities: Vec<_> = shapes.iter().map(|s| {
            let index = s._pad_filter[1];
            (index, w.shapes[index as usize].as_ref().unwrap().generation)
        }).collect();
        let (body_hint, shape_hint) = world_capacity_hints(w);
        let must_alloc = w.sim.as_ref().is_none_or(|sim| !sim.caps.fits(live));
        if must_alloc {
            let caps = w.sim.as_ref().map_or_else(
                || GpuSceneCaps::allocate(live, body_hint, shape_hint),
                |sim| sim.caps.grow_to_fit(live),
            );
            if let Err(err) = caps.validate_allocation(w.query_state as u32) {
                eprintln!("\n*** GPU PHYSICS FAILED ***\n{err}\n");
                w.gpu_fail = std::ffi::CString::new(err).ok();
                return;
            }
            let old = w.sim.take();
            let mut sim = match GpuSim::new(
                &w.gpu,
                bodies,
                &body_local_centers,
                &body_inv_inertia_offdiag,
                &shapes,
                &hull_points,
                &hull_planes,
                &hull_edges,
                &hull_topology,
                &mesh_vertices,
                &mesh_triangles,
                &mesh_nodes,
                &surface_materials,
                &mix_pairs,
                &joints,
                caps,
                w.enable_contacts,
                h,
                w.def.gravity,
            ) {
                Ok(sim) => {
                    if !w.physics_invalid {
                        w.gpu_fail = None;
                    }
                    sim
                }
                Err(err) => {
                    eprintln!("\n*** GPU PHYSICS FAILED ***\n{err}\n");
                    w.gpu_fail = std::ffi::CString::new(err).ok();
                    w.sim = old;
                    return;
                }
            };
            if let Some(flags)=w.diagnostic_flags_override {sim.set_diagnostic_flags(flags);}
            sim.shape_identities = identities.clone();
            sim.restore_physics_step(w.physics_step);
            if let Some(old) = old.as_ref() {
                sim.copy_contacts_from(old);
            }
            w.sim = Some(sim);
            if let Some(sim) = w.sim.as_mut() {
                sim.restore_physics_step(w.physics_step);
                if w.physics_invalid {
                    sim.mark_physics_invalid();
                }
            }
            apply_solver_topology(w);
        } else if let Some(sim) = w.sim.as_mut() {
            let bytes = match pack_scene_bytes(
                sim.caps,
                bodies,
                &body_local_centers,
                &body_inv_inertia_offdiag,
                &shapes,
                &hull_points,
                &hull_planes,
                &hull_edges,
                &hull_topology,
                &mesh_vertices,
                &mesh_triangles,
                &mesh_nodes,
                &surface_materials,
                &mix_pairs,
            ) {
                Ok(bytes) => bytes,
                Err(err) => {
                    eprintln!("\n*** GPU PHYSICS FAILED ***\n{err}\n");
                    w.gpu_fail = std::ffi::CString::new(err).ok();
                    return;
                }
            };
            sim.write_scene(&bytes, &shapes, bodies, &identities);
            sim.write_joints(&joints);
            sim.write_body_states(bodies);
            sim.set_geom_counts(
                shapes.len() as u32,
                joints.len() as u32,
                hull_points.len() as u32,
                hull_planes.len() as u32,
                hull_edges.len() as u32,
                hull_topology.len() as u32,
                mesh_vertices.len() as u32,
                mesh_triangles.len() as u32,
                mesh_node_live,
                surface_materials.len() as u32,
            );
        }
        w.scene_dirty = false;
        w.bodies_dirty = false;
        for cpu in w.bodies.iter_mut().flatten() {
            cpu.host_epoch = 0;
        }
    } else if w.bodies_dirty {
        if let Some(sim) = w.sim.as_ref() {
            for (index, slot) in w.bodies.iter().enumerate() {
                if let Some(cpu) = slot {
                    if cpu.host_epoch > 0 {
                        sim.write_body_state_at(index as u32, &cpu.gpu);
                    }
                }
            }
        }
        w.bodies_dirty = false;
        for cpu in w.bodies.iter_mut().flatten() {
            cpu.host_epoch = 0;
        }
    }
    if let Some(sim) = w.sim.as_mut() {
        #[cfg(not(target_arch = "wasm32"))]
        sim.set_automatic_pose_snapshots(w.automatic_pose_snapshots);
        sim.write_params(
            h,
            step_dt,
            w.def.gravity,
            n,
            w.enable_contacts,
            w.def.contact_hertz,
            w.def.contact_damping_ratio,
            w.def.contact_speed,
        );
        sim.set_maximum_linear_speed(w.def.maximum_linear_speed);
        sim.set_restitution_threshold(w.def.restitution_threshold);
        sim.set_solver_mode(w.jacobi);
        sim.set_sleep_enabled(w.def.enable_sleep);
        sim.set_continuous_enabled(w.def.enable_continuous);
        sim.set_contact_recycle_distance(w.contact_recycle_distance);
        // Component-local substeps cannot independently advance a shared
        // kinematic/zero-mass endpoint. Keep those worlds on global phases.
        sim.set_component_tgs(w.component_tgs_requested
            && !w.jacobi && w.custom_filter_callback.is_none() && w.pre_solve_callback.is_none()
            && !w.joints.iter().any(|j| j.kind != JOINT_NONE)
            && capabilities.component_bodies);

        // Stepping uploads the final table after assigning its step ID in
        // world_step / begin_callback_step. Queries and explicit setup need
        // the current parameters immediately, without a following submission.
        if upload_params { sim.flush_params(); }
    }
}

fn world_id_from_body(body: BodyId) -> WorldId {
    if body.index1 == 0 || body.world0 == 0 {
        return b3_null_world_id();
    }
    WorldId {
        index1: body.world0,
        generation: 1,
    }
}

fn push_shape(
    w: &mut WorldInner,
    body: BodyId,
    world: WorldId,
    kind: u32,
    half: [f32; 3],
    axis: [f32; 3],
    mass: f32,
    local_center: [f32; 3],
    geometry_center: [f32; 3],
    local_inertia: [f32; 6],
    inner_radius: f32,
    hull_points: Vec<[f32; 3]>,
    hull_planes: Vec<[f32; 4]>,
    hull_edges: Vec<[f32; 3]>,
    hull_topology: Vec<[u32; 4]>,
    def: &ShapeDef,
) -> ShapeId {
    if w.shapes.len() >= crate::types::MAX_SHAPE_SLOTS as usize {
        eprintln!("GPU physics: public shape index range exceeded; refusing shape {}", w.shapes.len());
        return b3_null_shape_id();
    }
    let (unit_mass, unit_inertia) = match kind {
        KIND_BOX => {
            let m = box_mass(half, 1.0);
            (m, box_central_inertia(half, 1.0))
        }
        KIND_SPHERE => {
            let m = sphere_mass(half[0], 1.0);
            let i = 0.4 * m * half[0] * half[0];
            (m, [i, i, i, 0.0, 0.0, 0.0])
        }
        KIND_CAPSULE => {
            let d = crate::types::compute_capsule_mass(axis.map(|v| -v), axis, half[0], 1.0);
            (d.mass, d.inertia)
        }
        _ if def.density > 0.0 => (mass / def.density, local_inertia.map(|v| v / def.density)),
        _ => (0.0, [0.0; 6]),
    };
    let shape = CpuShape {
        generation: 1,
        body_index: body.index1,
        kind,
        public_kind: kind,
        compound_parent: 0,
        compound_child_index: -1,
        next_compound_child_index: 0,
        compound_material_indices: [0; 4],
        half,
        axis,
        density: def.density,
        friction: def.friction,
        restitution: def.restitution,
        rolling: def.rolling_resistance,
        tangent_velocity: [0.0; 3],
        explosion_scale: def.explosion_scale,
        filter: def.filter,
        event_flags: u32::from(def.enable_contact_events) * SHAPE_ENABLE_CONTACT_EVENTS
            | u32::from(def.enable_sensor_events) * SHAPE_ENABLE_SENSOR_EVENTS
            | u32::from(def.enable_hit_events) * SHAPE_ENABLE_HIT_EVENTS
            | u32::from(def.enable_custom_filtering) * SHAPE_ENABLE_CUSTOM_FILTERING
            | u32::from(def.enable_pre_solve_events) * SHAPE_ENABLE_PRE_SOLVE_EVENTS
            | u32::from(!def.enable_speculative_contact) * SHAPE_DISABLE_SPECULATIVE
            | u32::from(def.is_sensor) * SHAPE_IS_SENSOR,
        user_material_id: def.user_material_id,
        custom_color: 0,
        mesh_materials: vec![SurfaceMaterial {
            friction: def.friction,
            restitution: def.restitution,
            rolling_resistance: def.rolling_resistance,
            tangent_velocity: [0.0; 3],
            user_material_id: def.user_material_id,
            custom_color: 0,
            padding: 0,
        }],
        user_data: def.user_data,
        inner_radius,
        hull_points,
        hull_planes,
        hull_edges,
        hull_topology,
        mesh_vertices: Arc::new(Vec::new()),
        mesh_triangles: Arc::new(Vec::new()),
        mesh_triangle_ids: Arc::new(Vec::new()),
        mesh_nodes: Arc::new(Vec::new()),
        mesh_scale: [1.0; 3],
        mesh_instance: None,
        mass,
        unit_mass,
        unit_inertia,
        geometry_center,
        local_center,
        local_inertia,
    };
    let index = w.shapes.len();
    w.shapes.push(Some(shape));
    if let Some(cpu) = body_mut(w, body) {
        cpu.shape_indices.push(index);
    }
    ShapeId {
        index1: w.shapes.len() as i32,
        world0: world.index1,
        generation: 1,
    }
}

fn body_mut(w: &mut WorldInner, id: BodyId) -> Option<&mut CpuBody> {
    let i = id.index1 as usize;
    if i == 0 {
        return None;
    }
    w.bodies
        .get_mut(i - 1)
        .and_then(|s| s.as_mut())
        .filter(|b| b.generation == id.generation)
}

fn body_ref(w: &WorldInner, id: BodyId) -> Option<&CpuBody> {
    let i = id.index1 as usize;
    if i == 0 {
        return None;
    }
    w.bodies
        .get(i - 1)
        .and_then(|s| s.as_ref())
        .filter(|b| b.generation == id.generation)
}

fn slot_mut(worlds: &mut [Option<WorldInner>], id: WorldId) -> Option<&mut WorldInner> {
    let i = id.index1 as usize;
    if i == 0 {
        return None;
    }
    worlds
        .get_mut(i - 1)
        .and_then(|s| s.as_mut())
        .filter(|w| w.generation == id.generation)
}

fn with_world_no_sync<T>(id: WorldId, f: impl FnOnce(&WorldInner) -> T) -> Option<T> {
    let worlds = lock_worlds();
    let i = id.index1 as usize;
    if i == 0 {
        return None;
    }
    worlds
        .get(i - 1)
        .and_then(|s| s.as_ref())
        .filter(|w| w.generation == id.generation)
        .map(f)
}

fn with_world<T>(id: WorldId, f: impl FnOnce(&WorldInner) -> T) -> Option<T> {
    let mut worlds = lock_worlds();
    slot_mut(&mut worlds, id).map(|w| {
        ensure_cpu_mirror_world(w, id);
        f(w)
    })
}

fn with_world_mut<T>(id: WorldId, f: impl FnOnce(&mut WorldInner) -> T) -> Option<T> {
    let mut worlds = lock_worlds();
    slot_mut(&mut worlds, id).map(|w| {
        ensure_cpu_mirror_world(w, id);
        if let Some(sim)=w.sim.as_ref() {sim.invalidate_idle_proof();}
        f(w)
    })
}

fn with_world_mut_no_sync<T>(id: WorldId, f: impl FnOnce(&mut WorldInner) -> T) -> Option<T> {
    let mut worlds = lock_worlds();
    slot_mut(&mut worlds, id).map(f)
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct DrawItemC {
    pub pos: [f32; 3],
    pub kind: u32,
    pub rot: [f32; 4],
    pub half: [f32; 3],
    pub axis: [f32; 3],
    pub flags: u32,
}

pub fn b3_world_draw_items(id: WorldId, out: &mut [DrawItemC]) -> i32 {
    with_world(id, |w| {
        let mut n = 0i32;
        for slot in w.bodies.iter() {
            let Some(cpu) = slot else { continue };
            if (cpu.gpu.flags & FLAG_HIDDEN) != 0 {
                continue;
            }
            if (n as usize) < out.len() {
                out[n as usize] = DrawItemC {
                    pos: cpu.gpu.pos,
                    kind: cpu.gpu.kind,
                    rot: cpu.gpu.rot,
                    half: cpu.gpu.half,
                    axis: cpu.axis,
                    flags: cpu.gpu.flags,
                };
            }
            n += 1;
        }
        n
    })
    .unwrap_or(0)
}

pub fn b3_world_aabb(id: WorldId) -> [f32; 6] {
    with_world(id, |w| {
        let mut lo = [f32::MAX; 3];
        let mut hi = [f32::MIN; 3];
        let mut any = false;
        for slot in w.bodies.iter() {
            let Some(cpu) = slot else { continue };
            any = true;
            for i in 0..3 {
                let r = cpu.gpu.half[i].abs().max(0.5);
                lo[i] = lo[i].min(cpu.gpu.pos[i] - r);
                hi[i] = hi[i].max(cpu.gpu.pos[i] + r);
            }
        }
        if !any {
            return [-1.0, -1.0, -1.0, 1.0, 1.0, 1.0];
        }
        [lo[0], lo[1], lo[2], hi[0], hi[1], hi[2]]
    })
    .unwrap_or([-1.0, -1.0, -1.0, 1.0, 1.0, 1.0])
}

/// Scheduling-phase diagnostics. `current` never means post-CCD collision state.
#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct WorldContactMetrics {
    pub snapshot_step: u64,
    pub submitted_step: u64,
    pub snapshot_topology: u64,
    pub current_topology: u64,
    pub snapshot_state: u64,
    pub current_state: u64,
    pub known: u32,
    pub current: u32,
    pub capacity_loss: u32,
    pub candidate_pairs: u32,
    pub allocated_roots: u32,
    pub allocated_manifold_slots: u32,
    pub touching_roots: u32,
    pub non_sensor_roots: u32,
}

#[cfg(not(target_arch = "wasm32"))]
pub fn b3_world_contact_metrics(id: WorldId, wait: bool) -> WorldContactMetrics {
    with_world_mut_no_sync(id, |w| {
        let mut result = WorldContactMetrics {
            submitted_step: w.physics_step, current_topology: w.query_topology,
            current_state: w.query_state, capacity_loss: u32::from(w.physics_invalid),
            ..WorldContactMetrics::default()
        };
        let Some(sim) = w.sim.as_mut() else { return result; };
        let Some(snapshot) = sim.contact_metrics(wait) else { return result; };
        result.snapshot_step = snapshot.step;
        result.snapshot_topology = snapshot.topology_revision;
        result.snapshot_state = snapshot.state_revision;
        result.known = 1;
        result.capacity_loss |= u32::from(snapshot.capacity_loss || sim.physics_invalid());
        result.current = u32::from(snapshot.step == w.physics_step
            && snapshot.topology_revision == w.query_topology && snapshot.state_revision == w.query_state
            && !w.scene_dirty && !w.bodies_dirty && result.capacity_loss == 0);
        result.candidate_pairs = snapshot.candidate_pairs;
        result.allocated_roots = snapshot.allocated_roots;
        result.allocated_manifold_slots = snapshot.allocated_manifold_slots;
        result.touching_roots = snapshot.touching_roots;
        result.non_sensor_roots = snapshot.non_sensor_roots;
        result
    }).unwrap_or_default()
}

pub fn b3_world_counts(id: WorldId) -> (i32, i32, i32) {
    // These are public topology, never a reason to wait, read body buffers or
    // harvest pending CCD/events. Compound collider children are private slots.
    with_world_no_sync(id, |w| {
        (
            w.bodies.iter().filter(|b| b.is_some()).count() as i32,
            w.shapes.iter().flatten().filter(|s| s.compound_parent == 0).count() as i32,
            w.joints.iter().filter(|j| j.kind != JOINT_NONE).count() as i32,
        )
    })
    .unwrap_or((0, 0, 0))
}

pub fn b3_live_world_count() -> i32 {
    let worlds = lock_worlds();
    worlds.iter().filter(|w| w.is_some()).count() as i32
}

pub fn b3_body_is_valid(id: BodyId) -> bool {
    let world = world_id_from_body(id);
    with_world_no_sync(world, |w| body_ref(w, id).is_some()).unwrap_or(false)
}

pub fn b3_shape_is_valid(id: ShapeId) -> bool {
    if id.index1 == 0 {
        return false;
    }
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_no_sync(world, |w| {
        w.shapes
            .get(id.index1 as usize - 1)
            .and_then(|s| s.as_ref())
            .is_some_and(|s| s.generation == id.generation)
    })
    .unwrap_or(false)
}

pub fn b3_body_get_shape_count(id: BodyId) -> i32 {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        if body_ref(w, id).is_none() {
            return 0;
        }
        w.shapes
            .iter()
            .flatten()
            .filter(|shape| shape.body_index == id.index1 && shape.compound_parent == 0)
            .count() as i32
    })
    .unwrap_or(0)
}

pub fn b3_body_get_shapes(id: BodyId, output: &mut [ShapeId]) -> usize {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        if body_ref(w, id).is_none() {
            return 0;
        }
        let mut count = 0;
        for (index, slot) in w.shapes.iter().enumerate() {
            let Some(shape) = slot.as_ref() else {
                continue;
            };
            if shape.body_index != id.index1 || shape.compound_parent != 0 {
                continue;
            }
            if count >= output.len() {
                break;
            }
            output[count] = ShapeId {
                index1: index as i32 + 1,
                world0: id.world0,
                generation: shape.generation,
            };
            count += 1;
        }
        count
    })
    .unwrap_or(0)
}

pub fn b3_joint_is_valid(id: JointId) -> bool {
    with_joint_metadata(id, |_, _| true).unwrap_or(false)
}

pub fn b3_destroy_joint(id: JointId, wake_attached: bool) {
    if id.index1 <= 0 {
        return;
    }
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        let Some(joint) = w.joints.get_mut(id.index1 as usize - 1) else {
            return;
        };
        if joint.kind == JOINT_NONE {
            return;
        }
        let (a, b) = (joint.a, joint.b);
        *joint = JointGpu::default();
        if let Some(meta) = w.joint_meta.get_mut(id.index1 as usize - 1) {
            *meta = None;
        }
        if wake_attached {
            for body_index in [a, b] {
                if let Some(Some(body)) = w.bodies.get_mut(body_index as usize) {
                    body.gpu.flags &= !FLAG_SLEEP;
                    body.gpu.sleep_time = 0.0;
                }
            }
        }
        mark_scene_dirty(w);
    });
}

fn with_joint_mut<R>(id: JointId, f: impl FnOnce(&mut JointGpu) -> R) -> Option<R> {
    if id.index1 <= 0 {
        return None;
    }
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        let (result, a, b) = {
            let joint = w.joints.get_mut(id.index1 as usize - 1)?;
            if joint.kind == JOINT_NONE {
                return None;
            }
            (f(joint), joint.a, joint.b)
        };
        for index in [a, b] {
            if let Some(Some(body)) = w.bodies.get_mut(index as usize) {
                body.gpu.flags &= !FLAG_SLEEP;
                body.gpu.sleep_time = 0.0;
            }
        }
        mark_scene_dirty(w);
        Some(result)
    })
    .flatten()
}

fn with_joint<R>(id: JointId, f: impl FnOnce(&JointGpu) -> R) -> Option<R> {
    if id.index1 <= 0 {
        return None;
    }
    with_world(
        WorldId {
            index1: id.world0,
            generation: 1,
        },
        |w| {
            let joint = w.joints.get(id.index1 as usize - 1)?;
            (joint.kind != JOINT_NONE).then(|| f(joint))
        },
    )
    .flatten()
}

pub fn b3_joint_set_constraint_tuning(id: JointId, hertz: f32, damping: f32) {
    with_joint_mut(id, |joint| {
        joint.hertz = hertz.max(0.0);
        joint.damping = damping.max(0.0);
    });
}

pub fn b3_joint_get_constraint_tuning(id: JointId) -> [f32; 2] {
    with_joint(id, |joint| [joint.hertz, joint.damping]).unwrap_or([0.0; 2])
}

fn with_joint_meta_mut<R>(id: JointId, f: impl FnOnce(&mut CpuJoint) -> R) -> Option<R> {
    if id.index1 <= 0 {
        return None;
    }
    with_world_mut(
        WorldId {
            index1: id.world0,
            generation: 1,
        },
        |w| {
            let index = id.index1 as usize - 1;
            if w.joints
                .get(index)
                .is_none_or(|joint| joint.kind == JOINT_NONE)
            {
                return None;
            }
            w.joint_meta.get_mut(index)?.as_mut().map(f)
        },
    )
    .flatten()
}

fn with_joint_meta<R>(id: JointId, f: impl FnOnce(&CpuJoint) -> R) -> Option<R> {
    if id.index1 <= 0 {
        return None;
    }
    with_world(
        WorldId {
            index1: id.world0,
            generation: 1,
        },
        |w| {
            let index = id.index1 as usize - 1;
            if w.joints
                .get(index)
                .is_none_or(|joint| joint.kind == JOINT_NONE)
            {
                return None;
            }
            w.joint_meta.get(index)?.as_ref().map(f)
        },
    )
    .flatten()
}

pub fn b3_joint_set_force_threshold(id: JointId, threshold: f32) {
    with_joint_meta_mut(id, |joint| {
        joint.force_threshold = threshold.clamp(0.0, f32::MAX);
    });
}

pub fn b3_joint_get_force_threshold(id: JointId) -> f32 {
    with_joint_meta(id, |joint| joint.force_threshold).unwrap_or(0.0)
}

pub fn b3_joint_set_torque_threshold(id: JointId, threshold: f32) {
    with_joint_meta_mut(id, |joint| {
        joint.torque_threshold = threshold.clamp(0.0, f32::MAX);
    });
}

pub fn b3_joint_get_torque_threshold(id: JointId) -> f32 {
    with_joint_meta(id, |joint| joint.torque_threshold).unwrap_or(0.0)
}

pub fn b3_joint_set_user_data(id: JointId, user_data: usize) {
    with_joint_meta_mut(id, |joint| joint.user_data = user_data);
}

pub fn b3_joint_get_user_data(id: JointId) -> usize {
    with_joint_meta(id, |joint| joint.user_data).unwrap_or(0)
}

fn with_revolute_joint_mut<R>(id: JointId, f: impl FnOnce(&mut JointGpu) -> R) -> Option<R> {
    with_joint_mut(id, |joint| (joint.kind == JOINT_REVOLUTE).then(|| f(joint))).flatten()
}

fn with_revolute_joint<R>(id: JointId, f: impl FnOnce(&JointGpu) -> R) -> Option<R> {
    with_joint(id, |joint| (joint.kind == JOINT_REVOLUTE).then(|| f(joint))).flatten()
}

fn with_weld_joint_mut<R>(id: JointId, f: impl FnOnce(&mut JointGpu) -> R) -> Option<R> {
    with_joint_mut(id, |joint| (joint.kind == JOINT_WELD).then(|| f(joint))).flatten()
}

fn with_weld_joint<R>(id: JointId, f: impl FnOnce(&JointGpu) -> R) -> Option<R> {
    with_joint(id, |joint| (joint.kind == JOINT_WELD).then(|| f(joint))).flatten()
}

pub fn b3_weld_joint_set_linear_hertz(id: JointId, hertz: f32) {
    with_weld_joint_mut(id, |joint| joint.weld_linear_hertz = hertz.max(0.0));
}

pub fn b3_weld_joint_get_linear_hertz(id: JointId) -> f32 {
    with_weld_joint(id, |joint| joint.weld_linear_hertz).unwrap_or(0.0)
}

pub fn b3_weld_joint_set_linear_damping_ratio(id: JointId, damping: f32) {
    with_weld_joint_mut(id, |joint| joint.weld_linear_damping = damping.max(0.0));
}

pub fn b3_weld_joint_get_linear_damping_ratio(id: JointId) -> f32 {
    with_weld_joint(id, |joint| joint.weld_linear_damping).unwrap_or(0.0)
}

pub fn b3_weld_joint_set_angular_hertz(id: JointId, hertz: f32) {
    with_weld_joint_mut(id, |joint| joint.weld_angular_hertz = hertz.max(0.0));
}

pub fn b3_weld_joint_get_angular_hertz(id: JointId) -> f32 {
    with_weld_joint(id, |joint| joint.weld_angular_hertz).unwrap_or(0.0)
}

pub fn b3_weld_joint_set_angular_damping_ratio(id: JointId, damping: f32) {
    with_weld_joint_mut(id, |joint| joint.weld_angular_damping = damping.max(0.0));
}

pub fn b3_weld_joint_get_angular_damping_ratio(id: JointId) -> f32 {
    with_weld_joint(id, |joint| joint.weld_angular_damping).unwrap_or(0.0)
}

pub fn b3_revolute_joint_enable_spring(id: JointId, enable: bool) {
    with_revolute_joint_mut(id, |joint| {
        let was_enabled = joint.flags & REVOLUTE_ENABLE_SPRING != 0;
        if was_enabled != enable {
            joint.spring_impulse = 0.0;
        }
        if enable {
            joint.flags |= REVOLUTE_ENABLE_SPRING;
        } else {
            joint.flags &= !REVOLUTE_ENABLE_SPRING;
        }
    });
}

pub fn b3_revolute_joint_is_spring_enabled(id: JointId) -> bool {
    with_revolute_joint(id, |joint| joint.flags & REVOLUTE_ENABLE_SPRING != 0).unwrap_or(false)
}

pub fn b3_revolute_joint_set_spring_hertz(id: JointId, hertz: f32) {
    with_revolute_joint_mut(id, |joint| joint.spring_hertz = hertz.max(0.0));
}

pub fn b3_revolute_joint_get_spring_hertz(id: JointId) -> f32 {
    with_revolute_joint(id, |joint| joint.spring_hertz).unwrap_or(0.0)
}

pub fn b3_revolute_joint_set_spring_damping(id: JointId, damping: f32) {
    with_revolute_joint_mut(id, |joint| joint.spring_damping = damping.max(0.0));
}

pub fn b3_revolute_joint_get_spring_damping(id: JointId) -> f32 {
    with_revolute_joint(id, |joint| joint.spring_damping).unwrap_or(0.0)
}

pub fn b3_revolute_joint_set_target_angle(id: JointId, angle: f32) {
    with_revolute_joint_mut(id, |joint| joint.target_translation = angle);
}

pub fn b3_revolute_joint_get_target_angle(id: JointId) -> f32 {
    with_revolute_joint(id, |joint| joint.target_translation).unwrap_or(0.0)
}

fn revolute_angle(joint: &JointGpu, w: &WorldInner) -> Option<f32> {
    let a = w.bodies.get(joint.a as usize)?.as_ref()?;
    let b = w.bodies.get(joint.b as usize)?.as_ref()?;
    let qa = quat_mul(a.gpu.rot, joint.frame_a_rotation);
    let mut qb = quat_mul(b.gpu.rot, joint.frame_b_rotation);
    let dot = qa[0] * qb[0] + qa[1] * qb[1] + qa[2] * qb[2] + qa[3] * qb[3];
    if dot < 0.0 {
        qb = [-qb[0], -qb[1], -qb[2], -qb[3]];
    }
    let rel = quat_mul([-qa[0], -qa[1], -qa[2], qa[3]], qb);
    let (z, s) = if rel[3] < 0.0 {
        (-rel[2], -rel[3])
    } else {
        (rel[2], rel[3])
    };
    Some(2.0 * z.atan2(s))
}

pub fn b3_revolute_joint_get_angle(id: JointId) -> f32 {
    if id.index1 <= 0 {
        return 0.0;
    }
    with_world(
        WorldId {
            index1: id.world0,
            generation: 1,
        },
        |w| {
            let joint = w.joints.get(id.index1 as usize - 1)?;
            (joint.kind == JOINT_REVOLUTE)
                .then(|| revolute_angle(joint, w))
                .flatten()
        },
    )
    .flatten()
    .unwrap_or(0.0)
}

pub fn b3_revolute_joint_enable_limit(id: JointId, enable: bool) {
    with_revolute_joint_mut(id, |joint| {
        let was_enabled = joint.flags & REVOLUTE_ENABLE_LIMIT != 0;
        if was_enabled != enable {
            joint.lower_impulse = 0.0;
            joint.upper_impulse = 0.0;
        }
        if enable {
            joint.flags |= REVOLUTE_ENABLE_LIMIT;
        } else {
            joint.flags &= !REVOLUTE_ENABLE_LIMIT;
        }
    });
}

pub fn b3_revolute_joint_is_limit_enabled(id: JointId) -> bool {
    with_revolute_joint(id, |joint| joint.flags & REVOLUTE_ENABLE_LIMIT != 0).unwrap_or(false)
}

pub fn b3_revolute_joint_get_lower_limit(id: JointId) -> f32 {
    with_revolute_joint(id, |joint| joint.lower_translation).unwrap_or(0.0)
}

pub fn b3_revolute_joint_get_upper_limit(id: JointId) -> f32 {
    with_revolute_joint(id, |joint| joint.upper_translation).unwrap_or(0.0)
}

pub fn b3_revolute_joint_set_limits(id: JointId, lower: f32, upper: f32) {
    with_revolute_joint_mut(id, |joint| {
        joint.lower_translation = lower
            .min(upper)
            .clamp(-0.99 * std::f32::consts::PI, 0.99 * std::f32::consts::PI);
        joint.upper_translation = upper
            .max(lower)
            .clamp(-0.99 * std::f32::consts::PI, 0.99 * std::f32::consts::PI);
    });
}

pub fn b3_revolute_joint_enable_motor(id: JointId, enable: bool) {
    with_revolute_joint_mut(id, |joint| {
        let was_enabled = joint.flags & REVOLUTE_ENABLE_MOTOR != 0;
        if was_enabled != enable {
            joint.motor_impulse = 0.0;
        }
        if enable {
            joint.flags |= REVOLUTE_ENABLE_MOTOR;
        } else {
            joint.flags &= !REVOLUTE_ENABLE_MOTOR;
        }
    });
}

pub fn b3_revolute_joint_is_motor_enabled(id: JointId) -> bool {
    with_revolute_joint(id, |joint| joint.flags & REVOLUTE_ENABLE_MOTOR != 0).unwrap_or(false)
}

pub fn b3_revolute_joint_set_motor_speed(id: JointId, speed: f32) {
    with_revolute_joint_mut(id, |joint| joint.motor_speed = speed);
}

pub fn b3_revolute_joint_get_motor_speed(id: JointId) -> f32 {
    with_revolute_joint(id, |joint| joint.motor_speed).unwrap_or(0.0)
}

pub fn b3_revolute_joint_set_max_motor_torque(id: JointId, torque: f32) {
    with_revolute_joint_mut(id, |joint| joint.max_motor_force = torque.max(0.0));
}

pub fn b3_revolute_joint_get_max_motor_torque(id: JointId) -> f32 {
    with_revolute_joint(id, |joint| joint.max_motor_force).unwrap_or(0.0)
}

pub fn b3_revolute_joint_get_motor_torque(id: JointId) -> f32 {
    if id.index1 <= 0 {
        return 0.0;
    }
    with_world(
        WorldId {
            index1: id.world0,
            generation: 1,
        },
        |w| {
            let joint = w.joints.get(id.index1 as usize - 1)?;
            (joint.kind == JOINT_REVOLUTE)
                .then(|| joint.motor_impulse / w.last_substep_h.max(f32::EPSILON))
        },
    )
    .flatten()
    .unwrap_or(0.0)
}

fn with_wheel_joint_mut<R>(id: JointId, f: impl FnOnce(&mut JointGpu) -> R) -> Option<R> {
    with_joint_mut(id, |joint| (joint.kind == JOINT_WHEEL).then(|| f(joint))).flatten()
}

fn with_wheel_joint<R>(id: JointId, f: impl FnOnce(&JointGpu) -> R) -> Option<R> {
    with_joint(id, |joint| (joint.kind == JOINT_WHEEL).then(|| f(joint))).flatten()
}

fn set_joint_flag(joint: &mut JointGpu, flag: u32, enable: bool) {
    if enable {
        joint.flags |= flag;
    } else {
        joint.flags &= !flag;
    }
}

pub fn b3_wheel_joint_enable_suspension(id: JointId, enable: bool) {
    with_wheel_joint_mut(id, |joint| {
        if (joint.flags & WHEEL_ENABLE_SUSPENSION_SPRING != 0) != enable {
            joint.spring_impulse = 0.0;
        }
        set_joint_flag(joint, WHEEL_ENABLE_SUSPENSION_SPRING, enable);
    });
}

pub fn b3_wheel_joint_is_suspension_enabled(id: JointId) -> bool {
    with_wheel_joint(id, |j| j.flags & WHEEL_ENABLE_SUSPENSION_SPRING != 0).unwrap_or(false)
}

pub fn b3_wheel_joint_set_suspension_hertz(id: JointId, value: f32) {
    with_wheel_joint_mut(id, |j| j.spring_hertz = value);
}

pub fn b3_wheel_joint_get_suspension_hertz(id: JointId) -> f32 {
    with_wheel_joint(id, |j| j.spring_hertz).unwrap_or(0.0)
}

pub fn b3_wheel_joint_set_suspension_damping_ratio(id: JointId, value: f32) {
    with_wheel_joint_mut(id, |j| j.spring_damping = value);
}

pub fn b3_wheel_joint_get_suspension_damping_ratio(id: JointId) -> f32 {
    with_wheel_joint(id, |j| j.spring_damping).unwrap_or(0.0)
}

pub fn b3_wheel_joint_enable_suspension_limit(id: JointId, enable: bool) {
    with_wheel_joint_mut(id, |joint| {
        if (joint.flags & WHEEL_ENABLE_SUSPENSION_LIMIT != 0) != enable {
            joint.lower_impulse = 0.0;
            joint.upper_impulse = 0.0;
        }
        set_joint_flag(joint, WHEEL_ENABLE_SUSPENSION_LIMIT, enable);
    });
}

pub fn b3_wheel_joint_is_suspension_limit_enabled(id: JointId) -> bool {
    with_wheel_joint(id, |j| j.flags & WHEEL_ENABLE_SUSPENSION_LIMIT != 0).unwrap_or(false)
}

pub fn b3_wheel_joint_get_lower_suspension_limit(id: JointId) -> f32 {
    with_wheel_joint(id, |j| j.lower_translation).unwrap_or(0.0)
}

pub fn b3_wheel_joint_get_upper_suspension_limit(id: JointId) -> f32 {
    with_wheel_joint(id, |j| j.upper_translation).unwrap_or(0.0)
}

pub fn b3_wheel_joint_set_suspension_limits(id: JointId, lower: f32, upper: f32) {
    with_wheel_joint_mut(id, |joint| {
        if lower != joint.lower_translation || upper != joint.upper_translation {
            joint.lower_translation = lower;
            joint.upper_translation = upper;
            joint.lower_impulse = 0.0;
            joint.upper_impulse = 0.0;
        }
    });
}

pub fn b3_wheel_joint_enable_spin_motor(id: JointId, enable: bool) {
    with_wheel_joint_mut(id, |joint| {
        if (joint.flags & WHEEL_ENABLE_SPIN_MOTOR != 0) != enable {
            joint.motor_impulse = 0.0;
        }
        set_joint_flag(joint, WHEEL_ENABLE_SPIN_MOTOR, enable);
    });
}

pub fn b3_wheel_joint_is_spin_motor_enabled(id: JointId) -> bool {
    with_wheel_joint(id, |j| j.flags & WHEEL_ENABLE_SPIN_MOTOR != 0).unwrap_or(false)
}

pub fn b3_wheel_joint_set_spin_motor_speed(id: JointId, value: f32) {
    with_wheel_joint_mut(id, |j| j.motor_speed = value);
}

pub fn b3_wheel_joint_get_spin_motor_speed(id: JointId) -> f32 {
    with_wheel_joint(id, |j| j.motor_speed).unwrap_or(0.0)
}

pub fn b3_wheel_joint_set_max_spin_torque(id: JointId, value: f32) {
    with_wheel_joint_mut(id, |j| j.max_motor_force = value);
}

pub fn b3_wheel_joint_get_max_spin_torque(id: JointId) -> f32 {
    with_wheel_joint(id, |j| j.max_motor_force).unwrap_or(0.0)
}

pub fn b3_wheel_joint_enable_steering(id: JointId, enable: bool) {
    with_wheel_joint_mut(id, |joint| {
        if (joint.flags & WHEEL_ENABLE_STEERING != 0) != enable {
            joint.angular_impulse = [0.0; 3];
        }
        set_joint_flag(joint, WHEEL_ENABLE_STEERING, enable);
    });
}

pub fn b3_wheel_joint_is_steering_enabled(id: JointId) -> bool {
    with_wheel_joint(id, |j| j.flags & WHEEL_ENABLE_STEERING != 0).unwrap_or(false)
}

pub fn b3_wheel_joint_set_steering_hertz(id: JointId, value: f32) {
    with_wheel_joint_mut(id, |j| j.weld_linear_hertz = value);
}

pub fn b3_wheel_joint_get_steering_hertz(id: JointId) -> f32 {
    with_wheel_joint(id, |j| j.weld_linear_hertz).unwrap_or(0.0)
}

pub fn b3_wheel_joint_set_steering_damping_ratio(id: JointId, value: f32) {
    with_wheel_joint_mut(id, |j| j.weld_linear_damping = value);
}

pub fn b3_wheel_joint_get_steering_damping_ratio(id: JointId) -> f32 {
    with_wheel_joint(id, |j| j.weld_linear_damping).unwrap_or(0.0)
}

pub fn b3_wheel_joint_set_max_steering_torque(id: JointId, value: f32) {
    with_wheel_joint_mut(id, |j| j.target_rotation[1] = value);
}

pub fn b3_wheel_joint_get_max_steering_torque(id: JointId) -> f32 {
    with_wheel_joint(id, |j| j.target_rotation[1]).unwrap_or(0.0)
}

pub fn b3_wheel_joint_enable_steering_limit(id: JointId, enable: bool) {
    with_wheel_joint_mut(id, |joint| {
        if (joint.flags & WHEEL_ENABLE_STEERING_LIMIT != 0) != enable {
            joint.weld_angular_impulse[1] = 0.0;
            joint.weld_angular_impulse[2] = 0.0;
        }
        set_joint_flag(joint, WHEEL_ENABLE_STEERING_LIMIT, enable);
    });
}

pub fn b3_wheel_joint_is_steering_limit_enabled(id: JointId) -> bool {
    with_wheel_joint(id, |j| j.flags & WHEEL_ENABLE_STEERING_LIMIT != 0).unwrap_or(false)
}

pub fn b3_wheel_joint_get_lower_steering_limit(id: JointId) -> f32 {
    with_wheel_joint(id, |j| j.target_rotation[2]).unwrap_or(0.0)
}

pub fn b3_wheel_joint_get_upper_steering_limit(id: JointId) -> f32 {
    with_wheel_joint(id, |j| j.target_rotation[3]).unwrap_or(0.0)
}

pub fn b3_wheel_joint_set_steering_limits(id: JointId, lower: f32, upper: f32) {
    with_wheel_joint_mut(id, |j| {
        j.target_rotation[2] = lower;
        j.target_rotation[3] = upper;
    });
}

pub fn b3_wheel_joint_set_target_steering_angle(id: JointId, value: f32) {
    with_wheel_joint_mut(id, |j| j.target_rotation[0] = value);
}

pub fn b3_wheel_joint_get_target_steering_angle(id: JointId) -> f32 {
    with_wheel_joint(id, |j| j.target_rotation[0]).unwrap_or(0.0)
}

fn wheel_live_value(id: JointId, f: impl FnOnce(&JointGpu, &WorldInner) -> Option<f32>) -> f32 {
    if id.index1 <= 0 {
        return 0.0;
    }
    with_world(
        WorldId {
            index1: id.world0,
            generation: 1,
        },
        |w| {
            let joint = w.joints.get(id.index1 as usize - 1)?;
            (joint.kind == JOINT_WHEEL).then(|| f(joint, w)).flatten()
        },
    )
    .flatten()
    .unwrap_or(0.0)
}

pub fn b3_wheel_joint_get_spin_speed(id: JointId) -> f32 {
    wheel_live_value(id, |joint, w| {
        let a = w.bodies.get(joint.a as usize)?.as_ref()?;
        let b = w.bodies.get(joint.b as usize)?.as_ref()?;
        let axis = quat_rotate(quat_mul(b.gpu.rot, joint.frame_b_rotation), [0.0, 0.0, 1.0]);
        Some(
            (b.gpu.omega[0] - a.gpu.omega[0]) * axis[0]
                + (b.gpu.omega[1] - a.gpu.omega[1]) * axis[1]
                + (b.gpu.omega[2] - a.gpu.omega[2]) * axis[2],
        )
    })
}

pub fn b3_wheel_joint_get_spin_torque(id: JointId) -> f32 {
    wheel_live_value(id, |joint, w| {
        Some(joint.motor_impulse / w.last_substep_h.max(f32::EPSILON))
    })
}

pub fn b3_wheel_joint_get_steering_angle(id: JointId) -> f32 {
    wheel_live_value(id, |joint, w| {
        let a = w.bodies.get(joint.a as usize)?.as_ref()?;
        let b = w.bodies.get(joint.b as usize)?.as_ref()?;
        let qa = quat_mul(a.gpu.rot, joint.frame_a_rotation);
        let qb = quat_mul(b.gpu.rot, joint.frame_b_rotation);
        let az = quat_rotate(qa, [0.0, 0.0, 1.0]);
        let ay = quat_rotate(qa, [0.0, 1.0, 0.0]);
        let bz = quat_rotate(qb, [0.0, 0.0, 1.0]);
        let cs = bz[0] * az[0] + bz[1] * az[1] + bz[2] * az[2];
        let ss = -(bz[0] * ay[0] + bz[1] * ay[1] + bz[2] * ay[2]);
        Some(ss.atan2(cs))
    })
}

pub fn b3_wheel_joint_get_steering_torque(id: JointId) -> f32 {
    wheel_live_value(id, |joint, w| {
        Some(joint.weld_angular_impulse[0] / w.last_substep_h.max(f32::EPSILON))
    })
}

fn with_spherical_joint_mut<R>(id: JointId, f: impl FnOnce(&mut JointGpu) -> R) -> Option<R> {
    with_joint_mut(id, |joint| {
        (joint.kind == JOINT_SPHERICAL).then(|| f(joint))
    })
    .flatten()
}

fn with_spherical_joint<R>(id: JointId, f: impl FnOnce(&JointGpu) -> R) -> Option<R> {
    with_joint(id, |joint| {
        (joint.kind == JOINT_SPHERICAL).then(|| f(joint))
    })
    .flatten()
}

fn spherical_relative_rotation(joint: &JointGpu, w: &WorldInner) -> Option<[f32; 4]> {
    let a = w.bodies.get(joint.a as usize)?.as_ref()?;
    let b = w.bodies.get(joint.b as usize)?.as_ref()?;
    let qa = quat_mul(a.gpu.rot, joint.frame_a_rotation);
    let mut qb = quat_mul(b.gpu.rot, joint.frame_b_rotation);
    let dot = qa[0] * qb[0] + qa[1] * qb[1] + qa[2] * qb[2] + qa[3] * qb[3];
    if dot < 0.0 {
        qb = [-qb[0], -qb[1], -qb[2], -qb[3]];
    }
    Some(quat_mul([-qa[0], -qa[1], -qa[2], qa[3]], qb))
}

fn spherical_live_angle(id: JointId, angle: impl FnOnce([f32; 4]) -> f32) -> f32 {
    if id.index1 <= 0 {
        return 0.0;
    }
    with_world(
        WorldId {
            index1: id.world0,
            generation: 1,
        },
        |w| {
            let joint = w.joints.get(id.index1 as usize - 1)?;
            (joint.kind == JOINT_SPHERICAL)
                .then(|| spherical_relative_rotation(joint, w).map(angle))
                .flatten()
        },
    )
    .flatten()
    .unwrap_or(0.0)
}

pub fn b3_spherical_joint_enable_cone_limit(id: JointId, enable: bool) {
    with_spherical_joint_mut(id, |joint| {
        if (joint.flags & SPHERICAL_ENABLE_CONE_LIMIT != 0) != enable {
            joint.swing_impulse = 0.0;
        }
        if enable {
            joint.flags |= SPHERICAL_ENABLE_CONE_LIMIT;
        } else {
            joint.flags &= !SPHERICAL_ENABLE_CONE_LIMIT;
        }
    });
}

pub fn b3_spherical_joint_is_cone_limit_enabled(id: JointId) -> bool {
    with_spherical_joint(id, |j| j.flags & SPHERICAL_ENABLE_CONE_LIMIT != 0).unwrap_or(false)
}

pub fn b3_spherical_joint_get_cone_limit(id: JointId) -> f32 {
    with_spherical_joint(id, |j| j.target_translation).unwrap_or(0.0)
}

pub fn b3_spherical_joint_set_cone_limit(id: JointId, angle: f32) {
    with_spherical_joint_mut(id, |j| {
        j.target_translation = angle.clamp(0.0, 0.5 * std::f32::consts::PI)
    });
}

pub fn b3_spherical_joint_get_cone_angle(id: JointId) -> f32 {
    spherical_live_angle(id, |q| {
        2.0 * (q[0] * q[0] + q[1] * q[1])
            .sqrt()
            .atan2((q[2] * q[2] + q[3] * q[3]).sqrt())
    })
}

pub fn b3_spherical_joint_enable_twist_limit(id: JointId, enable: bool) {
    with_spherical_joint_mut(id, |joint| {
        if (joint.flags & SPHERICAL_ENABLE_TWIST_LIMIT != 0) != enable {
            joint.lower_impulse = 0.0;
            joint.upper_impulse = 0.0;
        }
        if enable {
            joint.flags |= SPHERICAL_ENABLE_TWIST_LIMIT;
        } else {
            joint.flags &= !SPHERICAL_ENABLE_TWIST_LIMIT;
        }
    });
}

pub fn b3_spherical_joint_is_twist_limit_enabled(id: JointId) -> bool {
    with_spherical_joint(id, |j| j.flags & SPHERICAL_ENABLE_TWIST_LIMIT != 0).unwrap_or(false)
}

pub fn b3_spherical_joint_get_lower_twist_limit(id: JointId) -> f32 {
    with_spherical_joint(id, |j| j.lower_translation).unwrap_or(0.0)
}

pub fn b3_spherical_joint_get_upper_twist_limit(id: JointId) -> f32 {
    with_spherical_joint(id, |j| j.upper_translation).unwrap_or(0.0)
}

pub fn b3_spherical_joint_set_twist_limits(id: JointId, lower: f32, upper: f32) {
    with_spherical_joint_mut(id, |joint| {
        joint.lower_translation = lower
            .min(upper)
            .clamp(-0.99 * std::f32::consts::PI, 0.99 * std::f32::consts::PI);
        joint.upper_translation = upper
            .max(lower)
            .clamp(-0.99 * std::f32::consts::PI, 0.99 * std::f32::consts::PI);
    });
}

pub fn b3_spherical_joint_get_twist_angle(id: JointId) -> f32 {
    spherical_live_angle(id, |q| {
        if q[3] < 0.0 {
            2.0 * (-q[2]).atan2(-q[3])
        } else {
            2.0 * q[2].atan2(q[3])
        }
    })
}

pub fn b3_spherical_joint_enable_spring(id: JointId, enable: bool) {
    with_spherical_joint_mut(id, |joint| {
        if (joint.flags & SPHERICAL_ENABLE_SPRING != 0) != enable {
            joint.spring_angular_impulse = [0.0; 3];
        }
        if enable {
            joint.flags |= SPHERICAL_ENABLE_SPRING;
        } else {
            joint.flags &= !SPHERICAL_ENABLE_SPRING;
        }
    });
}

pub fn b3_spherical_joint_is_spring_enabled(id: JointId) -> bool {
    with_spherical_joint(id, |j| j.flags & SPHERICAL_ENABLE_SPRING != 0).unwrap_or(false)
}

pub fn b3_spherical_joint_set_spring_hertz(id: JointId, hertz: f32) {
    with_spherical_joint_mut(id, |j| j.spring_hertz = hertz.max(0.0));
}

pub fn b3_spherical_joint_get_spring_hertz(id: JointId) -> f32 {
    with_spherical_joint(id, |j| j.spring_hertz).unwrap_or(0.0)
}

pub fn b3_spherical_joint_set_spring_damping(id: JointId, damping: f32) {
    with_spherical_joint_mut(id, |j| j.spring_damping = damping.max(0.0));
}

pub fn b3_spherical_joint_get_spring_damping(id: JointId) -> f32 {
    with_spherical_joint(id, |j| j.spring_damping).unwrap_or(0.0)
}

pub fn b3_spherical_joint_set_target_rotation(id: JointId, rotation: [f32; 4]) {
    with_spherical_joint_mut(id, |j| j.target_rotation = rotation);
}

pub fn b3_spherical_joint_get_target_rotation(id: JointId) -> [f32; 4] {
    with_spherical_joint(id, |j| j.target_rotation).unwrap_or([0.0, 0.0, 0.0, 1.0])
}

pub fn b3_spherical_joint_enable_motor(id: JointId, enable: bool) {
    with_spherical_joint_mut(id, |joint| {
        if (joint.flags & SPHERICAL_ENABLE_MOTOR != 0) != enable {
            joint.motor_angular_impulse = [0.0; 3];
        }
        if enable {
            joint.flags |= SPHERICAL_ENABLE_MOTOR;
        } else {
            joint.flags &= !SPHERICAL_ENABLE_MOTOR;
        }
    });
}

pub fn b3_spherical_joint_is_motor_enabled(id: JointId) -> bool {
    with_spherical_joint(id, |j| j.flags & SPHERICAL_ENABLE_MOTOR != 0).unwrap_or(false)
}

pub fn b3_spherical_joint_set_motor_velocity(id: JointId, velocity: [f32; 3]) {
    with_spherical_joint_mut(id, |j| j.motor_angular_velocity = velocity);
}

pub fn b3_spherical_joint_get_motor_velocity(id: JointId) -> [f32; 3] {
    with_spherical_joint(id, |j| j.motor_angular_velocity).unwrap_or([0.0; 3])
}

pub fn b3_spherical_joint_set_max_motor_torque(id: JointId, torque: f32) {
    with_spherical_joint_mut(id, |j| j.max_motor_force = torque.max(0.0));
}

pub fn b3_spherical_joint_get_max_motor_torque(id: JointId) -> f32 {
    with_spherical_joint(id, |j| j.max_motor_force).unwrap_or(0.0)
}

pub fn b3_spherical_joint_get_motor_torque(id: JointId) -> [f32; 3] {
    if id.index1 <= 0 {
        return [0.0; 3];
    }
    with_world(
        WorldId {
            index1: id.world0,
            generation: 1,
        },
        |w| {
            let joint = w.joints.get(id.index1 as usize - 1)?;
            (joint.kind == JOINT_SPHERICAL).then(|| {
                let inv_h = 1.0 / w.last_substep_h.max(f32::EPSILON);
                [
                    inv_h * joint.motor_angular_impulse[0],
                    inv_h * joint.motor_angular_impulse[1],
                    inv_h * joint.motor_angular_impulse[2],
                ]
            })
        },
    )
    .flatten()
    .unwrap_or([0.0; 3])
}

fn with_prismatic_joint_mut<R>(id: JointId, f: impl FnOnce(&mut JointGpu) -> R) -> Option<R> {
    if id.index1 <= 0 {
        return None;
    }
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        let (result, a, b) = {
            let joint = w.joints.get_mut(id.index1 as usize - 1)?;
            if joint.kind != JOINT_PRISMATIC {
                return None;
            }
            (f(joint), joint.a, joint.b)
        };
        for index in [a, b] {
            if let Some(Some(body)) = w.bodies.get_mut(index as usize) {
                body.gpu.flags &= !FLAG_SLEEP;
                body.gpu.sleep_time = 0.0;
            }
        }
        mark_scene_dirty(w);
        Some(result)
    })
    .flatten()
}

fn with_prismatic_joint<R>(id: JointId, f: impl FnOnce(&JointGpu) -> R) -> Option<R> {
    if id.index1 <= 0 {
        return None;
    }
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        let joint = w.joints.get(id.index1 as usize - 1)?;
        (joint.kind == JOINT_PRISMATIC).then(|| f(joint))
    })
    .flatten()
}

fn with_distance_joint_mut<R>(id: JointId, f: impl FnOnce(&mut JointGpu) -> R) -> Option<R> {
    if id.index1 <= 0 {
        return None;
    }
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        let (result, a, b) = {
            let joint = w.joints.get_mut(id.index1 as usize - 1)?;
            if joint.kind != JOINT_DISTANCE {
                return None;
            }
            (f(joint), joint.a, joint.b)
        };
        for index in [a, b] {
            if let Some(Some(body)) = w.bodies.get_mut(index as usize) {
                body.gpu.flags &= !FLAG_SLEEP;
                body.gpu.sleep_time = 0.0;
            }
        }
        mark_scene_dirty(w);
        Some(result)
    })
    .flatten()
}

fn with_distance_joint<R>(id: JointId, f: impl FnOnce(&JointGpu) -> R) -> Option<R> {
    if id.index1 <= 0 {
        return None;
    }
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        let joint = w.joints.get(id.index1 as usize - 1)?;
        (joint.kind == JOINT_DISTANCE).then(|| f(joint))
    })
    .flatten()
}

pub fn b3_distance_joint_set_length(id: JointId, length: f32) {
    with_distance_joint_mut(id, |joint| {
        joint.target_translation = length.max(LINEAR_SLOP);
        joint.weld_linear_impulse = [0.0; 3];
        joint.impulse = 0.0;
        joint.spring_impulse = 0.0;
    });
}

pub fn b3_distance_joint_get_length(id: JointId) -> f32 {
    with_distance_joint(id, |joint| joint.target_translation).unwrap_or(0.0)
}

pub fn b3_distance_joint_enable_spring(id: JointId, enable: bool) {
    with_distance_joint_mut(id, |joint| {
        joint.flags = if enable {
            joint.flags | crate::types::DISTANCE_ENABLE_SPRING
        } else {
            joint.flags & !crate::types::DISTANCE_ENABLE_SPRING
        };
        joint.spring_impulse = 0.0;
    });
}

pub fn b3_distance_joint_is_spring_enabled(id: JointId) -> bool {
    with_distance_joint(id, |joint| {
        joint.flags & crate::types::DISTANCE_ENABLE_SPRING != 0
    })
    .unwrap_or(false)
}

pub fn b3_distance_joint_set_spring_force_range(id: JointId, lower: f32, upper: f32) {
    with_distance_joint_mut(id, |joint| {
        joint.perp_impulse = [lower.min(upper), upper.max(lower)];
        joint.spring_impulse = 0.0;
    });
}

pub fn b3_distance_joint_get_spring_force_range(id: JointId) -> [f32; 2] {
    with_distance_joint(id, |joint| joint.perp_impulse).unwrap_or([0.0; 2])
}

pub fn b3_distance_joint_set_spring_hertz(id: JointId, hertz: f32) {
    with_distance_joint_mut(id, |joint| joint.spring_hertz = hertz.max(0.0));
}

pub fn b3_distance_joint_get_spring_hertz(id: JointId) -> f32 {
    with_distance_joint(id, |joint| joint.spring_hertz).unwrap_or(0.0)
}

pub fn b3_distance_joint_set_spring_damping(id: JointId, damping: f32) {
    with_distance_joint_mut(id, |joint| joint.spring_damping = damping.max(0.0));
}

pub fn b3_distance_joint_get_spring_damping(id: JointId) -> f32 {
    with_distance_joint(id, |joint| joint.spring_damping).unwrap_or(0.0)
}

pub fn b3_distance_joint_enable_limit(id: JointId, enable: bool) {
    with_distance_joint_mut(id, |joint| {
        joint.flags = if enable {
            joint.flags | crate::types::DISTANCE_ENABLE_LIMIT
        } else {
            joint.flags & !crate::types::DISTANCE_ENABLE_LIMIT
        };
        joint.lower_impulse = 0.0;
        joint.upper_impulse = 0.0;
    });
}

pub fn b3_distance_joint_is_limit_enabled(id: JointId) -> bool {
    with_distance_joint(id, |joint| {
        joint.flags & crate::types::DISTANCE_ENABLE_LIMIT != 0
    })
    .unwrap_or(false)
}

pub fn b3_distance_joint_set_length_range(id: JointId, min_length: f32, max_length: f32) {
    with_distance_joint_mut(id, |joint| {
        joint.weld_linear_impulse = [0.0; 3];
        joint.lower_translation = min_length.max(LINEAR_SLOP);
        joint.upper_translation = max_length.max(joint.lower_translation);
        joint.lower_impulse = 0.0;
        joint.upper_impulse = 0.0;
    });
}

pub fn b3_distance_joint_get_min_length(id: JointId) -> f32 {
    with_distance_joint(id, |joint| joint.lower_translation).unwrap_or(0.0)
}

pub fn b3_distance_joint_get_max_length(id: JointId) -> f32 {
    with_distance_joint(id, |joint| joint.upper_translation).unwrap_or(0.0)
}

pub fn b3_distance_joint_get_current_length(id: JointId) -> f32 {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        let joint = w.joints.get(id.index1.checked_sub(1)? as usize)?;
        if joint.kind != JOINT_DISTANCE {
            return None;
        }
        let a = w.bodies.get(joint.a as usize)?.as_ref()?;
        let b = w.bodies.get(joint.b as usize)?.as_ref()?;
        let ra = joint_r(a, joint.anchor_a);
        let rb = joint_r(b, joint.anchor_b);
        let d = [
            b.gpu.pos[0] + rb[0] - a.gpu.pos[0] - ra[0],
            b.gpu.pos[1] + rb[1] - a.gpu.pos[1] - ra[1],
            b.gpu.pos[2] + rb[2] - a.gpu.pos[2] - ra[2],
        ];
        Some((d[0] * d[0] + d[1] * d[1] + d[2] * d[2]).sqrt())
    })
    .flatten()
    .unwrap_or(0.0)
}

pub fn b3_distance_joint_enable_motor(id: JointId, enable: bool) {
    with_distance_joint_mut(id, |joint| {
        if (joint.flags & crate::types::DISTANCE_ENABLE_MOTOR != 0) != enable {
            joint.weld_angular_impulse[0] = 0.0;
        }
        joint.flags = if enable {
            joint.flags | crate::types::DISTANCE_ENABLE_MOTOR
        } else {
            joint.flags & !crate::types::DISTANCE_ENABLE_MOTOR
        };
        joint.motor_impulse = 0.0;
    });
}

pub fn b3_distance_joint_is_motor_enabled(id: JointId) -> bool {
    with_distance_joint(id, |joint| {
        joint.flags & crate::types::DISTANCE_ENABLE_MOTOR != 0
    })
    .unwrap_or(false)
}

pub fn b3_distance_joint_set_motor_speed(id: JointId, speed: f32) {
    with_distance_joint_mut(id, |joint| joint.motor_speed = speed);
}

pub fn b3_distance_joint_get_motor_speed(id: JointId) -> f32 {
    with_distance_joint(id, |joint| joint.motor_speed).unwrap_or(0.0)
}

pub fn b3_distance_joint_set_max_motor_force(id: JointId, force: f32) {
    with_distance_joint_mut(id, |joint| joint.max_motor_force = force.max(0.0));
}

pub fn b3_distance_joint_get_max_motor_force(id: JointId) -> f32 {
    with_distance_joint(id, |joint| joint.max_motor_force).unwrap_or(0.0)
}

pub fn b3_distance_joint_get_motor_force(id: JointId) -> f32 {
    with_distance_joint(id, |joint| joint.motor_impulse / FIXED_DT).unwrap_or(0.0)
}

fn with_parallel_joint_mut<R>(id: JointId, f: impl FnOnce(&mut JointGpu) -> R) -> Option<R> {
    if id.index1 <= 0 {
        return None;
    }
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        let (result, a, b) = {
            let joint = w.joints.get_mut(id.index1 as usize - 1)?;
            if joint.kind != JOINT_PARALLEL {
                return None;
            }
            (f(joint), joint.a, joint.b)
        };
        for index in [a, b] {
            if let Some(Some(body)) = w.bodies.get_mut(index as usize) {
                body.gpu.flags &= !FLAG_SLEEP;
                body.gpu.sleep_time = 0.0;
            }
        }
        mark_scene_dirty(w);
        Some(result)
    })
    .flatten()
}

fn with_parallel_joint<R>(id: JointId, f: impl FnOnce(&JointGpu) -> R) -> Option<R> {
    if id.index1 <= 0 {
        return None;
    }
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        let joint = w.joints.get(id.index1 as usize - 1)?;
        (joint.kind == JOINT_PARALLEL).then(|| f(joint))
    })
    .flatten()
}

pub fn b3_parallel_joint_set_spring_hertz(id: JointId, hertz: f32) {
    with_parallel_joint_mut(id, |joint| joint.spring_hertz = hertz.max(0.0));
}

pub fn b3_parallel_joint_get_spring_hertz(id: JointId) -> f32 {
    with_parallel_joint(id, |joint| joint.spring_hertz).unwrap_or(0.0)
}

pub fn b3_parallel_joint_set_spring_damping(id: JointId, damping: f32) {
    with_parallel_joint_mut(id, |joint| joint.spring_damping = damping.max(0.0));
}

pub fn b3_parallel_joint_get_spring_damping(id: JointId) -> f32 {
    with_parallel_joint(id, |joint| joint.spring_damping).unwrap_or(0.0)
}

pub fn b3_parallel_joint_set_max_torque(id: JointId, torque: f32) {
    with_parallel_joint_mut(id, |joint| joint.max_motor_force = torque.max(0.0));
}

pub fn b3_parallel_joint_get_max_torque(id: JointId) -> f32 {
    with_parallel_joint(id, |joint| joint.max_motor_force).unwrap_or(0.0)
}

fn with_motor_joint_mut<R>(id: JointId, f: impl FnOnce(&mut JointGpu) -> R) -> Option<R> {
    if id.index1 <= 0 {
        return None;
    }
    with_world_mut(
        WorldId {
            index1: id.world0,
            generation: 1,
        },
        |world| {
            let joint = world.joints.get_mut((id.index1 - 1) as usize)?;
            (joint.kind == JOINT_MOTOR).then(|| f(joint))
        },
    )
    .flatten()
}

fn with_motor_joint<R>(id: JointId, f: impl FnOnce(&JointGpu) -> R) -> Option<R> {
    if id.index1 <= 0 {
        return None;
    }
    with_world(
        WorldId {
            index1: id.world0,
            generation: 1,
        },
        |world| {
            let joint = world.joints.get((id.index1 - 1) as usize)?;
            (joint.kind == JOINT_MOTOR).then(|| f(joint))
        },
    )
    .flatten()
}

pub fn b3_motor_joint_set_linear_velocity(id: JointId, velocity: [f32; 3]) {
    with_motor_joint_mut(id, |joint| joint.axis = velocity);
}

pub fn b3_motor_joint_get_linear_velocity(id: JointId) -> [f32; 3] {
    with_motor_joint(id, |joint| joint.axis).unwrap_or([0.0; 3])
}

pub fn b3_motor_joint_set_angular_velocity(id: JointId, velocity: [f32; 3]) {
    with_motor_joint_mut(id, |joint| joint.motor_angular_velocity = velocity);
}

pub fn b3_motor_joint_get_angular_velocity(id: JointId) -> [f32; 3] {
    with_motor_joint(id, |joint| joint.motor_angular_velocity).unwrap_or([0.0; 3])
}

pub fn b3_motor_joint_set_max_velocity_force(id: JointId, force: f32) {
    with_motor_joint_mut(id, |joint| joint.target_translation = force);
}

pub fn b3_motor_joint_get_max_velocity_force(id: JointId) -> f32 {
    with_motor_joint(id, |joint| joint.target_translation).unwrap_or(0.0)
}

pub fn b3_motor_joint_set_max_velocity_torque(id: JointId, torque: f32) {
    with_motor_joint_mut(id, |joint| joint.lower_translation = torque);
}

pub fn b3_motor_joint_get_max_velocity_torque(id: JointId) -> f32 {
    with_motor_joint(id, |joint| joint.lower_translation).unwrap_or(0.0)
}

pub fn b3_motor_joint_set_linear_hertz(id: JointId, hertz: f32) {
    with_motor_joint_mut(id, |joint| joint.hertz = hertz);
}

pub fn b3_motor_joint_get_linear_hertz(id: JointId) -> f32 {
    with_motor_joint(id, |joint| joint.hertz).unwrap_or(0.0)
}

pub fn b3_motor_joint_set_linear_damping(id: JointId, damping: f32) {
    with_motor_joint_mut(id, |joint| joint.damping = damping);
}

pub fn b3_motor_joint_get_linear_damping(id: JointId) -> f32 {
    with_motor_joint(id, |joint| joint.damping).unwrap_or(0.0)
}

pub fn b3_motor_joint_set_angular_hertz(id: JointId, hertz: f32) {
    with_motor_joint_mut(id, |joint| joint.spring_hertz = hertz);
}

pub fn b3_motor_joint_get_angular_hertz(id: JointId) -> f32 {
    with_motor_joint(id, |joint| joint.spring_hertz).unwrap_or(0.0)
}

pub fn b3_motor_joint_set_angular_damping(id: JointId, damping: f32) {
    with_motor_joint_mut(id, |joint| joint.spring_damping = damping);
}

pub fn b3_motor_joint_get_angular_damping(id: JointId) -> f32 {
    with_motor_joint(id, |joint| joint.spring_damping).unwrap_or(0.0)
}

pub fn b3_motor_joint_set_max_spring_force(id: JointId, force: f32) {
    with_motor_joint_mut(id, |joint| joint.upper_translation = force.max(0.0));
}

pub fn b3_motor_joint_get_max_spring_force(id: JointId) -> f32 {
    with_motor_joint(id, |joint| joint.upper_translation).unwrap_or(0.0)
}

pub fn b3_motor_joint_set_max_spring_torque(id: JointId, torque: f32) {
    with_motor_joint_mut(id, |joint| joint.max_motor_force = torque.max(0.0));
}

pub fn b3_motor_joint_get_max_spring_torque(id: JointId) -> f32 {
    with_motor_joint(id, |joint| joint.max_motor_force).unwrap_or(0.0)
}

pub fn b3_prismatic_joint_enable_spring(id: JointId, enable: bool) {
    with_prismatic_joint_mut(id, |joint| {
        joint.flags = if enable {
            joint.flags | crate::types::PRISMATIC_ENABLE_SPRING
        } else {
            joint.flags & !crate::types::PRISMATIC_ENABLE_SPRING
        };
        joint.spring_impulse = 0.0;
    });
}

pub fn b3_prismatic_joint_is_spring_enabled(id: JointId) -> bool {
    with_prismatic_joint(id, |joint| {
        joint.flags & crate::types::PRISMATIC_ENABLE_SPRING != 0
    })
    .unwrap_or(false)
}

pub fn b3_prismatic_joint_set_spring_hertz(id: JointId, hertz: f32) {
    with_prismatic_joint_mut(id, |joint| joint.spring_hertz = hertz.max(0.0));
}

pub fn b3_prismatic_joint_get_spring_hertz(id: JointId) -> f32 {
    with_prismatic_joint(id, |joint| joint.spring_hertz).unwrap_or(0.0)
}

pub fn b3_prismatic_joint_set_spring_damping(id: JointId, damping: f32) {
    with_prismatic_joint_mut(id, |joint| joint.spring_damping = damping.max(0.0));
}

pub fn b3_prismatic_joint_get_spring_damping(id: JointId) -> f32 {
    with_prismatic_joint(id, |joint| joint.spring_damping).unwrap_or(0.0)
}

pub fn b3_prismatic_joint_set_target_translation(id: JointId, translation: f32) {
    with_prismatic_joint_mut(id, |joint| {
        joint.target_translation = translation;
        joint.spring_impulse = 0.0;
    });
}

pub fn b3_prismatic_joint_get_target_translation(id: JointId) -> f32 {
    with_prismatic_joint(id, |joint| joint.target_translation).unwrap_or(0.0)
}

pub fn b3_prismatic_joint_enable_limit(id: JointId, enable: bool) {
    with_prismatic_joint_mut(id, |joint| {
        joint.flags = if enable {
            joint.flags | crate::types::PRISMATIC_ENABLE_LIMIT
        } else {
            joint.flags & !crate::types::PRISMATIC_ENABLE_LIMIT
        };
        joint.lower_impulse = 0.0;
        joint.upper_impulse = 0.0;
    });
}

pub fn b3_prismatic_joint_is_limit_enabled(id: JointId) -> bool {
    with_prismatic_joint(id, |joint| {
        joint.flags & crate::types::PRISMATIC_ENABLE_LIMIT != 0
    })
    .unwrap_or(false)
}

pub fn b3_prismatic_joint_get_lower_limit(id: JointId) -> f32 {
    with_prismatic_joint(id, |joint| joint.lower_translation).unwrap_or(0.0)
}

pub fn b3_prismatic_joint_get_upper_limit(id: JointId) -> f32 {
    with_prismatic_joint(id, |joint| joint.upper_translation).unwrap_or(0.0)
}

pub fn b3_prismatic_joint_set_limits(id: JointId, lower: f32, upper: f32) {
    with_prismatic_joint_mut(id, |joint| {
        joint.lower_translation = lower.min(upper);
        joint.upper_translation = upper.max(lower);
        joint.lower_impulse = 0.0;
        joint.upper_impulse = 0.0;
    });
}

pub fn b3_prismatic_joint_enable_motor(id: JointId, enable: bool) {
    with_prismatic_joint_mut(id, |joint| {
        joint.flags = if enable {
            joint.flags | crate::types::PRISMATIC_ENABLE_MOTOR
        } else {
            joint.flags & !crate::types::PRISMATIC_ENABLE_MOTOR
        };
        joint.motor_impulse = 0.0;
    });
}

pub fn b3_prismatic_joint_is_motor_enabled(id: JointId) -> bool {
    with_prismatic_joint(id, |joint| {
        joint.flags & crate::types::PRISMATIC_ENABLE_MOTOR != 0
    })
    .unwrap_or(false)
}

pub fn b3_prismatic_joint_set_motor_speed(id: JointId, speed: f32) {
    with_prismatic_joint_mut(id, |joint| joint.motor_speed = speed);
}

pub fn b3_prismatic_joint_get_motor_speed(id: JointId) -> f32 {
    with_prismatic_joint(id, |joint| joint.motor_speed).unwrap_or(0.0)
}

pub fn b3_prismatic_joint_set_max_motor_force(id: JointId, force: f32) {
    with_prismatic_joint_mut(id, |joint| joint.max_motor_force = force.max(0.0));
}

pub fn b3_prismatic_joint_get_max_motor_force(id: JointId) -> f32 {
    with_prismatic_joint(id, |joint| joint.max_motor_force).unwrap_or(0.0)
}

pub fn b3_prismatic_joint_get_motor_force(id: JointId) -> f32 {
    with_prismatic_joint(id, |joint| joint.motor_impulse / FIXED_DT).unwrap_or(0.0)
}

pub fn b3_prismatic_joint_get_translation(id: JointId) -> f32 {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        let joint = w.joints.get(id.index1.checked_sub(1)? as usize)?;
        if joint.kind != JOINT_PRISMATIC {
            return None;
        }
        let a = w.bodies.get(joint.a as usize)?.as_ref()?;
        let b = w.bodies.get(joint.b as usize)?.as_ref()?;
        let axis = quat_rotate(a.gpu.rot, joint.axis);
        let ra = joint_r(a, joint.anchor_a);
        let pa = [
            a.gpu.pos[0] + ra[0],
            a.gpu.pos[1] + ra[1],
            a.gpu.pos[2] + ra[2],
        ];
        let rb = joint_r(b, joint.anchor_b);
        let pb = [
            b.gpu.pos[0] + rb[0],
            b.gpu.pos[1] + rb[1],
            b.gpu.pos[2] + rb[2],
        ];
        Some((pb[0] - pa[0]) * axis[0] + (pb[1] - pa[1]) * axis[1] + (pb[2] - pa[2]) * axis[2])
    })
    .flatten()
    .unwrap_or(0.0)
}

pub fn b3_prismatic_joint_get_speed(id: JointId) -> f32 {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        let joint = w.joints.get(id.index1.checked_sub(1)? as usize)?;
        if joint.kind != JOINT_PRISMATIC {
            return None;
        }
        let a = w.bodies.get(joint.a as usize)?.as_ref()?;
        let b = w.bodies.get(joint.b as usize)?.as_ref()?;
        let axis = quat_rotate(a.gpu.rot, joint.axis);
        Some(
            (b.gpu.vel[0] - a.gpu.vel[0]) * axis[0]
                + (b.gpu.vel[1] - a.gpu.vel[1]) * axis[1]
                + (b.gpu.vel[2] - a.gpu.vel[2]) * axis[2],
        )
    })
    .flatten()
    .unwrap_or(0.0)
}

pub fn b3_shape_body(id: ShapeId) -> BodyId {
    if id.index1 == 0 {
        return b3_null_body_id();
    }
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_no_sync(world, |w| {
        w.shapes
            .get(id.index1 as usize - 1)
            .and_then(|s| s.as_ref())
            .filter(|s| s.generation == id.generation)
            .map(|s| live_body_id(w, id.world0, s.body_index))
            .unwrap_or_else(b3_null_body_id)
    })
    .unwrap_or_else(b3_null_body_id)
}

pub fn b3_shape_kind(id: ShapeId) -> i32 {
    if id.index1 == 0 {
        return -1;
    }
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        w.shapes
            .get(id.index1 as usize - 1)
            .and_then(|s| s.as_ref())
            .map(|s| s.public_kind as i32)
            .unwrap_or(-1)
    })
    .unwrap_or(-1)
}

pub fn b3_shape_get_filter(id: ShapeId) -> Filter {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        w.shapes
            .get(id.index1.saturating_sub(1) as usize)
            .and_then(|shape| shape.as_ref())
            .map(|shape| shape.filter)
    })
    .flatten()
    .unwrap_or_else(crate::api::b3_default_filter)
}

fn host_shape(w: &WorldInner, world0: u16, index: usize, shape: &CpuShape) -> Option<HostShape> {
    let body = w
        .bodies
        .get(shape.body_index.checked_sub(1)? as usize)?
        .as_ref()?;
    if (body.gpu.flags & FLAG_DISABLED) != 0 {
        return None;
    }
    host_shape_direct(w, world0, index, shape)
}

// Direct shape/body queries remain valid for disabled bodies. World broadphase
// queries use host_shape above to exclude them.
fn host_shape_direct(w: &WorldInner, world0: u16, index: usize, shape: &CpuShape) -> Option<HostShape> {
    if shape.public_kind == PUBLIC_KIND_COMPOUND {
        return None;
    }
    let body = w
        .bodies
        .get(shape.body_index.checked_sub(1)? as usize)?
        .as_ref()?;
    let local_points = match shape.kind {
        KIND_SPHERE => vec![shape.geometry_center],
        KIND_CAPSULE => vec![
            [
                shape.geometry_center[0] - shape.axis[0],
                shape.geometry_center[1] - shape.axis[1],
                shape.geometry_center[2] - shape.axis[2],
            ],
            [
                shape.geometry_center[0] + shape.axis[0],
                shape.geometry_center[1] + shape.axis[1],
                shape.geometry_center[2] + shape.axis[2],
            ],
        ],
        KIND_BOX => {
            let mut points = Vec::with_capacity(8);
            for x in [-shape.half[0], shape.half[0]] {
                for y in [-shape.half[1], shape.half[1]] {
                    for z in [-shape.half[2], shape.half[2]] {
                        points.push([
                            shape.geometry_center[0] + x,
                            shape.geometry_center[1] + y,
                            shape.geometry_center[2] + z,
                        ]);
                    }
                }
            }
            points
        }
        KIND_CONVEX_HULL => shape.hull_points.clone(),
        KIND_MESH => Vec::new(),
        _ => return None,
    };
    let (public_index, public_shape) = if shape.compound_parent != 0 {
        let parent_index = shape.compound_parent.checked_sub(1)? as usize;
        (parent_index, w.shapes.get(parent_index)?.as_ref()?)
    } else {
        (index, shape)
    };
    Some(HostShape {
        public_id: ShapeId {
            index1: public_index as i32 + 1,
            world0,
            generation: public_shape.generation,
        },
        child_index: if shape.compound_parent != 0 { shape.compound_child_index } else { -1 },
        compound_material_indices: shape.compound_material_indices,
        id: ShapeId {
            index1: index as i32 + 1,
            world0,
            generation: shape.generation,
        },
        kind: shape.kind,
        body_origin: body_origin(body),
        body_rotation: body.gpu.rot,
        local_points: if shape.kind == KIND_MESH { shape.mesh_vertices.clone() } else { Arc::new(local_points) },
        local_center: shape.local_center,
        radius: if matches!(shape.kind, KIND_SPHERE | KIND_CAPSULE) {
            shape.half[0]
        } else {
            0.0
        },
        half_extents: shape.half,
        hull_planes: Arc::new(if shape.kind == KIND_BOX {
            vec![
                [1.0, 0.0, 0.0, shape.geometry_center[0] + shape.half[0]],
                [-1.0, 0.0, 0.0, -shape.geometry_center[0] + shape.half[0]],
                [0.0, 1.0, 0.0, shape.geometry_center[1] + shape.half[1]],
                [0.0, -1.0, 0.0, -shape.geometry_center[1] + shape.half[1]],
                [0.0, 0.0, 1.0, shape.geometry_center[2] + shape.half[2]],
                [0.0, 0.0, -1.0, -shape.geometry_center[2] + shape.half[2]],
            ]
        } else {
            shape.hull_planes.clone()
        }),
        hull_topology: Arc::new(shape.hull_topology.clone()),
        mesh_triangles: shape.mesh_triangles.clone(),
        mesh_triangle_ids: shape.mesh_triangle_ids.clone(),
        mesh_nodes: shape.mesh_nodes.clone(),
        mesh_instance: shape.mesh_instance,
        filter: shape.filter,
        friction: shape.friction,
        restitution: shape.restitution,
        user_material_id: shape.user_material_id,
        user_data: shape.user_data,
    })
}

fn refresh_query_index(w: &mut WorldInner, world0: u16) {
    let topology = w.query_topology;
    let state = w.query_state;
    let stale = w
        .query_index
        .as_ref()
        .is_none_or(|index| index.topology != topology);
    if stale {
        let construct_start = std::time::Instant::now();
        let shapes: Vec<HostShape> = w
            .shapes
            .iter()
            .enumerate()
            .filter_map(|(index, shape)| host_shape(w, world0, index, shape.as_ref()?))
            .collect();
        let construct_ms = construct_start.elapsed().as_secs_f32() * 1e3;
        let index_start = std::time::Instant::now();
        let mut index = QueryIndex::rebuild(shapes);
        index.topology = topology;
        index.state = state;
        w.last_query_profile.construct_ms = construct_ms;
        w.last_query_profile.index_ms = index_start.elapsed().as_secs_f32() * 1e3;
        w.query_index = Some(index);
        return;
    }
    let Some(index) = w.query_index.as_mut() else {
        return;
    };
    if index.state == state {
        return;
    }
    let mut moved = false;
    for shape in &mut index.shapes {
        let slot = shape.id.index1.checked_sub(1).map(|i| i as usize);
        let Some(cpu) = slot.and_then(|i| w.shapes.get(i).and_then(|s| s.as_ref())) else {
            continue;
        };
        let Some(body) = w
            .bodies
            .get(cpu.body_index.checked_sub(1).unwrap_or(0) as usize)
            .and_then(|b| b.as_ref())
        else {
            continue;
        };
        let origin = body_origin(body);
        moved |= shape.body_origin != origin || shape.body_rotation != body.gpu.rot;
        shape.body_origin = origin;
        shape.body_rotation = body.gpu.rot;
        shape.filter = cpu.filter;
    }
    let index_start = std::time::Instant::now();
    if moved { index.refit(); }
    index.state = state;
    w.last_query_profile.index_ms = index_start.elapsed().as_secs_f32() * 1e3;
}

pub(super) fn with_query_index<T>(
    id: WorldId,
    f: impl FnOnce(&QueryIndex, &mut super::QueryProfile) -> T,
) -> Option<T> {
    with_world_mut_no_sync(id, |w| {
        #[cfg(not(target_arch = "wasm32"))]
        sync_query_mirror(w, id);
        refresh_query_index(w, id.index1);
        let mut profile = w.last_query_profile;
        let Some(index) = w.query_index.as_ref() else {
            return None;
        };
        let out = f(index, &mut profile);
        w.last_query_profile = profile;
        Some(out)
    })
    .flatten()
}

// Callback rays take an owned candidate snapshot before invoking user code.
// Publication revalidates WorldId (including generation) after callbacks return.
pub(super) fn query_ray_snapshot(
    id: WorldId, origin: [f32; 3], translation: [f32; 3],
) -> Option<(HostWorld, super::QueryProfile)> {
    with_world_mut_no_sync(id, |w| {
        w.last_query_profile = super::QueryProfile::default();
        #[cfg(not(target_arch = "wasm32"))]
        sync_query_mirror(w, id);
        refresh_query_index(w, id.index1);
        let index = w.query_index.as_ref()?;
        let start = std::time::Instant::now();
        let (snapshot, visits) = index.snapshot_ray(origin.into(), translation.into());
        let mut profile = w.last_query_profile;
        profile.broadphase_ms = start.elapsed().as_secs_f32() * 1e3;
        profile.visited_shapes = visits;
        Some((snapshot, profile))
    }).flatten()
}

pub(super) fn publish_query_profile(id: WorldId, profile: super::QueryProfile) {
    // The last completed query owns the profile, including after nested queries.
    with_world_mut_no_sync(id, |w| w.last_query_profile = profile);
}

pub(super) fn query_world_aabb(id: WorldId, bounds: super::Aabb) -> Option<HostWorld> {
    with_query_index(id, |index, _| index.snapshot_aabb(bounds))
}

pub(super) fn query_world(id: WorldId) -> Option<HostWorld> {
    with_query_index(id, |index, _| HostWorld {
        shapes: index.shapes.clone(),
        groups: index.groups.clone(),
    })
}

#[cfg(not(target_arch = "wasm32"))]
pub(super) fn gpu_cast_ray_closest(
    id: WorldId,
    origin: [f32; 3],
    translation: [f32; 3],
    filter: QueryFilter,
) -> RayResult {
    with_world_mut_no_sync(id, |w| {
        if w.sim.is_none() || w.scene_dirty {
            let (bodies, _, _) = pack_gpu_slots(w);
            if !bodies.is_empty() {
                let h = FIXED_DT / DEFAULT_SUB_STEPS.max(1) as f32;
                ensure_sim(w, &bodies, bodies.len() as u32, h, FIXED_DT, true);
            }
        }
        // Current-state pick waits for the same finalized physics as getters.
        // CCD runs at most once per completed step (here or via a getter/next step).
        let wait_start = std::time::Instant::now();
        ensure_cpu_mirror_world(w, id);
        let wait_ms = wait_start.elapsed().as_secs_f32() * 1e3;
        let mut copied = 0u64;
        if w.bodies_dirty {
            if let Some(sim) = w.sim.as_ref() {
                for (index, slot) in w.bodies.iter().enumerate() {
                    if let Some(cpu) = slot {
                        if cpu.host_epoch > 0 {
                            sim.write_body_state_at(index as u32, &cpu.gpu);
                            copied += core::mem::size_of::<crate::types::BodyStateGpu>() as u64;
                        }
                    }
                }
            }
        }
        w.last_query_profile.ccd_ms = 0.0;
        let Some(sim) = w.sim.as_mut() else {
            w.last_query_profile.wait_ms = wait_ms;
            return RayResult::default();
        };
        let gpu = sim.cast_ray_closest_gpu(
            origin,
            translation,
            filter.category_bits,
            filter.mask_bits,
            1.0,
        );
        w.last_query_profile.wait_ms = wait_ms;
        w.last_query_profile.staging_ms = 0.0;
        w.last_query_profile.apply_ms = 0.0;
        w.last_query_profile.construct_ms = 0.0;
        w.last_query_profile.index_ms = 0.0;
        w.last_query_profile.broadphase_ms = 0.0;
        w.last_query_profile.exact_ms = gpu.exclusive_ms;
        w.last_query_profile.map_ms = gpu.map_ms;
        w.last_query_profile.encode_ms = gpu.encode_ms;
        w.last_query_profile.copied_bytes = copied + 64;
        w.last_query_profile.overflow = gpu.overflow;
        if !gpu.hit {
            return RayResult::default();
        }
        let cpu_index = gpu.cpu_shape as usize;
        let Some(shape) = w.shapes.get(cpu_index).and_then(|s| s.as_ref()) else {
            return RayResult::default();
        };
        let (public_index, public_shape, child_index) = if shape.compound_parent != 0 {
            let parent_index = shape.compound_parent as usize - 1;
            let Some(parent) = w.shapes.get(parent_index).and_then(Option::as_ref) else {
                return RayResult::default();
            };
            (parent_index, parent, shape.compound_child_index)
        } else {
            (cpu_index, shape, -1)
        };
        RayResult {
            shape_id: ShapeId {
                index1: public_index as i32 + 1,
                world0: id.index1,
                generation: public_shape.generation,
            },
            point: gpu.point,
            normal: gpu.normal,
            user_material_id: gpu.material,
            fraction: gpu.fraction,
            triangle_index: gpu.triangle,
            child_index,
            node_visits: 0,
            leaf_visits: 1,
            hit: true,
        }
    })
    .unwrap_or_default()
}

pub fn b3_shape_get_density(id: ShapeId) -> f32 {
    // Density belongs to the public shape, including compound parents, and
    // never requires a pose/contact download.
    with_world_no_sync(
        WorldId {
            index1: id.world0,
            generation: 1,
        },
        |w| {
            let index = id.index1.checked_sub(1)? as usize;
            w.shapes
                .get(index)?
                .as_ref()
                .filter(|shape| shape.generation == id.generation)
                .map(|shape| shape.density)
        },
    )
    .flatten()
    .unwrap_or(0.0)
}

pub(super) fn query_shape(id: ShapeId) -> Option<HostShape> {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        let index = id.index1.checked_sub(1)? as usize;
        let shape = w.shapes.get(index)?.as_ref()?;
        if shape.generation != id.generation {
            return None;
        }
        host_shape_direct(w, id.world0, index, shape)
    })
    .flatten()
}

// Resolve a public compound after one synchronization. Geometry stays shared
// through HostShape's Arcs; do not rebuild the world query index for this lookup.
pub(super) fn query_shape_parts(id: ShapeId) -> Vec<HostShape> {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        let Some(index) = id.index1.checked_sub(1).map(|i| i as usize) else {
            return Vec::new();
        };
        let Some(shape) = w.shapes.get(index).and_then(Option::as_ref) else {
            return Vec::new();
        };
        if shape.generation != id.generation {
            return Vec::new();
        }
        if shape.public_kind != PUBLIC_KIND_COMPOUND {
            return host_shape_direct(w, id.world0, index, shape)
                .into_iter()
                .collect();
        }
        w.shapes
            .iter()
            .enumerate()
            .filter_map(|(index, slot)| {
                let child = slot.as_ref()?;
                (child.compound_parent == id.index1)
                    .then(|| host_shape_direct(w, id.world0, index, child))
                    .flatten()
            })
            .collect()
    })
    .unwrap_or_default()
}

pub(super) fn query_body(id: BodyId) -> Option<HostBody> {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        let body = body_ref(w, id)?;
        Some(HostBody {
            origin: body_origin(body),
            shapes: w
                .shapes
                .iter()
                .enumerate()
                .filter_map(|(index, shape)| {
                    let shape = shape.as_ref()?;
                    (shape.body_index == id.index1)
                        .then(|| host_shape_direct(w, id.world0, index, shape))
                        .flatten()
                })
                .collect(),
        })
    })
    .flatten()
}

pub fn b3_world_explode(id: WorldId, def: &ExplosionDef) {
    if !def.position.iter().all(|value| value.is_finite())
        || !def.radius.is_finite()
        || def.radius < 0.0
        || !def.falloff.is_finite()
        || def.falloff < 0.0
        || !def.impulse_per_area.is_finite()
    {
        return;
    }
    with_world_mut(id, |w| {
        struct Contribution {
            body_index: usize,
            impulse: [f32; 3],
            closest_local: [f32; 3],
        }

        let mut contributions = Vec::new();
        for (shape_index, slot) in w.shapes.iter().enumerate() {
            let Some(shape) = slot.as_ref() else {
                continue;
            };
            if shape.explosion_scale == 0.0
                || shape.filter.category_bits & def.mask_bits == 0
                || shape.body_index <= 0
            {
                continue;
            }
            let body_index = shape.body_index as usize - 1;
            let Some(body) = w.bodies.get(body_index).and_then(Option::as_ref) else {
                continue;
            };
            if body.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC) != 0 {
                continue;
            }
            let origin = body_origin(body);
            let inverse_rotation = [
                -body.gpu.rot[0],
                -body.gpu.rot[1],
                -body.gpu.rot[2],
                body.gpu.rot[3],
            ];
            let local_position = quat_rotate(
                inverse_rotation,
                [
                    def.position[0] - origin[0],
                    def.position[1] - origin[1],
                    def.position[2] - origin[2],
                ],
            );
            let Some(host) = host_shape(w, id.index1, shape_index, shape) else {
                continue;
            };
            let geometry = explosion_geometry(&host, local_position);
            if geometry.distance > def.radius + def.falloff {
                continue;
            }
            let attenuation = if geometry.distance > def.radius && def.falloff > 0.0 {
                ((def.radius + def.falloff - geometry.distance) / def.falloff).clamp(0.0, 1.0)
            } else {
                1.0
            };
            let magnitude =
                def.impulse_per_area * geometry.area * attenuation * shape.explosion_scale;
            let world_direction = quat_rotate(body.gpu.rot, geometry.direction);
            contributions.push(Contribution {
                body_index,
                impulse: [
                    magnitude * world_direction[0],
                    magnitude * world_direction[1],
                    magnitude * world_direction[2],
                ],
                closest_local: geometry.closest_point,
            });
        }

        if contributions.is_empty() {
            return;
        }
        for contribution in contributions {
            let Some(body) = w
                .bodies
                .get_mut(contribution.body_index)
                .and_then(Option::as_mut)
            else {
                continue;
            };
            for (velocity, impulse) in body.gpu.vel.iter_mut().zip(contribution.impulse) {
                *velocity += body.gpu.inv_mass * impulse;
            }
            let local_r = [
                contribution.closest_local[0] - body.local_center[0],
                contribution.closest_local[1] - body.local_center[1],
                contribution.closest_local[2] - body.local_center[2],
            ];
            let r = quat_rotate(body.gpu.rot, local_r);
            apply_world_angular_impulse(
                body,
                [
                    r[1] * contribution.impulse[2] - r[2] * contribution.impulse[1],
                    r[2] * contribution.impulse[0] - r[0] * contribution.impulse[2],
                    r[0] * contribution.impulse[1] - r[1] * contribution.impulse[0],
                ],
            );
            body.gpu.flags &= !FLAG_SLEEP;
            body.gpu.sleep_time = 0.0;
        }
        // Host-side velocity and wake changes replace GPU mutable state.
        w.bodies_dirty = true;
    });
}

pub fn b3_shape_set_explosion_scale(id: ShapeId, scale: f32) {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        if let Some(shape) = w
            .shapes
            .get_mut(id.index1.saturating_sub(1) as usize)
            .and_then(Option::as_mut)
            .filter(|shape| shape.generation == id.generation)
        {
            shape.explosion_scale = scale;
        }
    });
}

pub fn b3_shape_set_user_data(id: ShapeId, user_data: usize) {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        if let Some(shape) = w
            .shapes
            .get_mut(id.index1.saturating_sub(1) as usize)
            .and_then(Option::as_mut)
            .filter(|shape| shape.generation == id.generation)
        {
            shape.user_data = user_data;
        }
    });
}

pub fn b3_shape_set_filter(id: ShapeId, filter: Filter, invoke_contacts: bool) {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        let propagate;
        {
            let Some(shape) = w
                .shapes
                .get_mut(id.index1.saturating_sub(1) as usize)
                .and_then(|shape| shape.as_mut())
            else {
                return;
            };
            if shape.filter.category_bits == filter.category_bits
                && shape.filter.mask_bits == filter.mask_bits
                && shape.filter.group_index == filter.group_index
            {
                return;
            }
            shape.filter = filter;
            propagate = shape.public_kind == PUBLIC_KIND_COMPOUND;
        }
        mark_scene_dirty(w);
        if propagate {
            for slot in w.shapes.iter_mut().flatten() {
                if slot.compound_parent == id.index1 {
                    slot.filter = filter;
                }
            }
        }
        if !invoke_contacts {
            return;
        }

        let mut retained = HashMap::with_capacity(w.live_contacts.len());
        for (key, contact) in w.live_contacts.drain() {
            if contact.shape_id_a.index1 == id.index1 || contact.shape_id_b.index1 == id.index1 {
                if contact.events_enabled {
                    w.deferred_contact_end_events.push(ContactEndTouchEvent {
                        shape_id_a: contact.shape_id_a,
                        shape_id_b: contact.shape_id_b,
                        contact_id: contact.contact_id,
                    });
                }
            } else {
                retained.insert(key, contact);
            }
        }
        w.live_contacts = retained;
    });
}

pub fn b3_shape_enable_contact_events(id: ShapeId, enable: bool) {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        let Some(shape) = w
            .shapes
            .get_mut(id.index1.saturating_sub(1) as usize)
            .and_then(|shape| shape.as_mut())
        else {
            return;
        };
        if enable {
            shape.event_flags |= SHAPE_ENABLE_CONTACT_EVENTS;
        } else {
            shape.event_flags &= !SHAPE_ENABLE_CONTACT_EVENTS;
        }
        mark_scene_dirty(w);
    });
}

pub fn b3_shape_are_contact_events_enabled(id: ShapeId) -> bool {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        w.shapes
            .get(id.index1.saturating_sub(1) as usize)
            .and_then(|shape| shape.as_ref())
            .is_some_and(|shape| (shape.event_flags & SHAPE_ENABLE_CONTACT_EVENTS) != 0)
    })
    .unwrap_or(false)
}

pub fn b3_shape_enable_custom_filtering(id: ShapeId, enable: bool) {
    set_shape_event_flag(id, SHAPE_ENABLE_CUSTOM_FILTERING, enable);
}

pub fn b3_shape_is_custom_filtering_enabled(id: ShapeId) -> bool {
    shape_event_flag(id, SHAPE_ENABLE_CUSTOM_FILTERING)
}

pub fn b3_shape_enable_pre_solve_events(id: ShapeId, enable: bool) {
    set_shape_event_flag(id, SHAPE_ENABLE_PRE_SOLVE_EVENTS, enable);
}

pub fn b3_shape_enable_speculative_contact(id: ShapeId, enable: bool) {
    set_shape_event_flag(id, SHAPE_DISABLE_SPECULATIVE, !enable);
}

pub fn b3_shape_are_pre_solve_events_enabled(id: ShapeId) -> bool {
    shape_event_flag(id, SHAPE_ENABLE_PRE_SOLVE_EVENTS)
}

fn set_shape_event_flag(id: ShapeId, flag: u32, enable: bool) {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        let Some(shape) = w
            .shapes
            .get_mut(id.index1.saturating_sub(1) as usize)
            .and_then(Option::as_mut)
            .filter(|shape| shape.generation == id.generation)
        else {
            return;
        };
        if enable {
            shape.event_flags |= flag;
        } else {
            shape.event_flags &= !flag;
        }
        mark_scene_dirty(w);
    });
}

fn shape_event_flag(id: ShapeId, flag: u32) -> bool {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        w.shapes
            .get(id.index1.saturating_sub(1) as usize)
            .and_then(Option::as_ref)
            .filter(|shape| shape.generation == id.generation)
            .is_some_and(|shape| shape.event_flags & flag != 0)
    })
    .unwrap_or(false)
}

pub fn b3_shape_enable_hit_events(id: ShapeId, enable: bool) {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        let Some(shape) = w
            .shapes
            .get_mut(id.index1.saturating_sub(1) as usize)
            .and_then(Option::as_mut)
        else {
            return;
        };
        if enable {
            shape.event_flags |= SHAPE_ENABLE_HIT_EVENTS;
        } else {
            shape.event_flags &= !SHAPE_ENABLE_HIT_EVENTS;
        }
        mark_scene_dirty(w);
    });
}

pub fn b3_shape_are_hit_events_enabled(id: ShapeId) -> bool {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        w.shapes
            .get(id.index1.saturating_sub(1) as usize)
            .and_then(Option::as_ref)
            .is_some_and(|shape| shape.event_flags & SHAPE_ENABLE_HIT_EVENTS != 0)
    })
    .unwrap_or(false)
}

pub fn b3_shape_set_user_material_id(id: ShapeId, user_material_id: u64) {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        if let Some(shape) = w
            .shapes
            .get_mut(id.index1.saturating_sub(1) as usize)
            .and_then(Option::as_mut)
        {
            shape.user_material_id = user_material_id;
            if let Some(material) = shape.mesh_materials.first_mut() {
                material.user_material_id = user_material_id;
            }
            mark_scene_dirty(w);
        }
    });
}

pub fn b3_shape_set_surface_material(id: ShapeId, material: SurfaceMaterial) {
    if !material.friction.is_finite()
        || material.friction < 0.0
        || !material.restitution.is_finite()
        || material.restitution < 0.0
        || !material.rolling_resistance.is_finite()
        || material.rolling_resistance < 0.0
        || material
            .tangent_velocity
            .iter()
            .any(|value| !value.is_finite())
    {
        return;
    }
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        let Some(shape) = w
            .shapes
            .get_mut(id.index1.saturating_sub(1) as usize)
            .and_then(Option::as_mut)
        else {
            return;
        };
        shape.friction = material.friction;
        shape.restitution = material.restitution;
        shape.rolling = material.rolling_resistance;
        shape.tangent_velocity = material.tangent_velocity;
        shape.user_material_id = material.user_material_id;
        shape.custom_color = material.custom_color;
        if shape.mesh_materials.is_empty() {
            shape.mesh_materials.push(material);
        } else {
            shape.mesh_materials[0] = material;
        }
        mark_scene_dirty(w);
    });
}

pub fn b3_shape_get_surface_material(id: ShapeId) -> SurfaceMaterial {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        let shape = w
            .shapes
            .get(id.index1.saturating_sub(1) as usize)?
            .as_ref()?;
        Some(SurfaceMaterial {
            friction: shape.friction,
            restitution: shape.restitution,
            rolling_resistance: shape.rolling,
            tangent_velocity: shape.tangent_velocity,
            user_material_id: shape.user_material_id,
            custom_color: shape.custom_color,
            padding: 0,
        })
    })
    .flatten()
    .unwrap_or_else(crate::api::b3_default_surface_material)
}

pub fn b3_shape_set_friction(id: ShapeId, friction: f32) {
    let mut material = b3_shape_get_surface_material(id);
    material.friction = friction;
    b3_shape_set_surface_material(id, material);
}

pub fn b3_shape_set_restitution(id: ShapeId, restitution: f32) {
    let mut material = b3_shape_get_surface_material(id);
    material.restitution = restitution;
    b3_shape_set_surface_material(id, material);
}

fn add_wind_force(body: &mut CpuBody, force: [f32; 3], torque: [f32; 3], wake: bool) -> bool {
    if (body.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC | FLAG_DISABLED)) != 0
    {
        return false;
    }
    if wake {
        wake_body(body);
    } else if body.gpu.flags & FLAG_SLEEP != 0 {
        return false;
    }
    for (sum, value) in body.force.iter_mut().zip(force) {
        *sum += value;
    }
    for (sum, value) in body.torque.iter_mut().zip(torque) {
        *sum += value;
    }
    true
}

pub fn b3_shape_apply_wind(
    id: ShapeId,
    wind: [f32; 3],
    drag: f32,
    lift: f32,
    max_speed: f32,
    wake: bool,
) {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        let Some(shape) = w
            .shapes
            .get(id.index1.saturating_sub(1) as usize)
            .and_then(Option::as_ref)
            .cloned()
        else {
            return;
        };
        if shape.kind == KIND_MESH {
            return;
        }
        let body_id = live_body_id(w, id.world0, shape.body_index);
        let Some(body) = body_ref(w, body_id) else {
            return;
        };
        let rot = body.gpu.rot;
        let vel = body.gpu.vel;
        let omega = body.gpu.omega;
        let local_center = body.local_center;
        let air = 1.225;
        let mut force = [0.0; 3];
        let mut torque = [0.0; 3];
        let apply_triangle = |force: &mut [f32; 3],
                               torque: &mut [f32; 3],
                               p1: [f32; 3],
                               p2: [f32; 3],
                               p3: [f32; 3],
                               normal: [f32; 3]| {
            let local = [
                (p1[0] + p2[0] + p3[0]) / 3.0,
                (p1[1] + p2[1] + p3[1]) / 3.0,
                (p1[2] + p2[2] + p3[2]) / 3.0,
            ];
            let lever = quat_rotate(
                rot,
                [
                    local[0] - local_center[0],
                    local[1] - local_center[1],
                    local[2] - local_center[2],
                ],
            );
            let n = quat_rotate(rot, normal);
            let center_vel = [
                vel[0] + omega[1] * lever[2] - omega[2] * lever[1],
                vel[1] + omega[2] * lever[0] - omega[0] * lever[2],
                vel[2] + omega[0] * lever[1] - omega[1] * lever[0],
            ];
            let rel = [
                wind[0] - drag * center_vel[0],
                wind[1] - drag * center_vel[1],
                wind[2] - drag * center_vel[2],
            ];
            let speed = vec3_length(rel).min(max_speed.max(0.0));
            if speed <= 1.0e-8 {
                return;
            }
            let direction = [rel[0] / speed, rel[1] / speed, rel[2] / speed];
            if n[0] * direction[0] + n[1] * direction[1] + n[2] * direction[2] >= -1.0e-8 {
                return;
            }
            let e1 = [
                p2[0] - p1[0],
                p2[1] - p1[1],
                p2[2] - p1[2],
            ];
            let e2 = [
                p3[0] - p1[0],
                p3[1] - p1[1],
                p3[2] - p1[2],
            ];
            let we1 = quat_rotate(rot, e1);
            let we2 = quat_rotate(rot, e2);
            let cross = [
                we1[1] * we2[2] - we1[2] * we2[1],
                we1[2] * we2[0] - we1[0] * we2[2],
                we1[0] * we2[1] - we1[1] * we2[0],
            ];
            let area = -0.5
                * (cross[0] * direction[0] + cross[1] * direction[1] + cross[2] * direction[2]);
            if area <= 0.0 {
                return;
            }
            let nx = n[1] * direction[2] - n[2] * direction[1];
            let ny = n[2] * direction[0] - n[0] * direction[2];
            let nz = n[0] * direction[1] - n[1] * direction[0];
            let lift_dir = [
                ny * direction[2] - nz * direction[1],
                nz * direction[0] - nx * direction[2],
                nx * direction[1] - ny * direction[0],
            ];
            let mag = 0.5 * air * area * speed * speed;
            let delta = [
                mag * (direction[0] + lift * lift_dir[0]),
                mag * (direction[1] + lift * lift_dir[1]),
                mag * (direction[2] + lift * lift_dir[2]),
            ];
            for i in 0..3 {
                force[i] += delta[i];
            }
            torque[0] += lever[1] * delta[2] - lever[2] * delta[1];
            torque[1] += lever[2] * delta[0] - lever[0] * delta[2];
            torque[2] += lever[0] * delta[1] - lever[1] * delta[0];
        };
        match shape.kind {
            KIND_SPHERE => {
                let lever = quat_rotate(
                    rot,
                    [
                        shape.geometry_center[0] - local_center[0],
                        shape.geometry_center[1] - local_center[1],
                        shape.geometry_center[2] - local_center[2],
                    ],
                );
                let shape_vel = [
                    vel[0] + omega[1] * lever[2] - omega[2] * lever[1],
                    vel[1] + omega[2] * lever[0] - omega[0] * lever[2],
                    vel[2] + omega[0] * lever[1] - omega[1] * lever[0],
                ];
                let rel = [
                    wind[0] - drag * shape_vel[0],
                    wind[1] - drag * shape_vel[1],
                    wind[2] - drag * shape_vel[2],
                ];
                let speed = vec3_length(rel).min(max_speed.max(0.0));
                if speed > 1.0e-8 {
                    let direction = [rel[0] / speed, rel[1] / speed, rel[2] / speed];
                    let area = std::f32::consts::PI * shape.half[0] * shape.half[0];
                    let mag = 0.5 * air * area * speed * speed;
                    force = [mag * direction[0], mag * direction[1], mag * direction[2]];
                    torque = [
                        lever[1] * force[2] - lever[2] * force[1],
                        lever[2] * force[0] - lever[0] * force[2],
                        lever[0] * force[1] - lever[1] * force[0],
                    ];
                }
            }
            KIND_CAPSULE => {
                let lever = quat_rotate(
                    rot,
                    [
                        shape.geometry_center[0] - local_center[0],
                        shape.geometry_center[1] - local_center[1],
                        shape.geometry_center[2] - local_center[2],
                    ],
                );
                let shape_vel = [
                    vel[0] + omega[1] * lever[2] - omega[2] * lever[1],
                    vel[1] + omega[2] * lever[0] - omega[0] * lever[2],
                    vel[2] + omega[0] * lever[1] - omega[1] * lever[0],
                ];
                let rel = [
                    wind[0] - drag * shape_vel[0],
                    wind[1] - drag * shape_vel[1],
                    wind[2] - drag * shape_vel[2],
                ];
                let speed = vec3_length(rel).min(max_speed.max(0.0));
                if speed > 1.0e-8 {
                    let direction = [rel[0] / speed, rel[1] / speed, rel[2] / speed];
                    let d = quat_rotate(
                        rot,
                        [2.0 * shape.axis[0], 2.0 * shape.axis[1], 2.0 * shape.axis[2]],
                    );
                    let radius = shape.half[0];
                    let cross = [
                        d[1] * direction[2] - d[2] * direction[1],
                        d[2] * direction[0] - d[0] * direction[2],
                        d[0] * direction[1] - d[1] * direction[0],
                    ];
                    let area = std::f32::consts::PI * radius * radius + 2.0 * radius * vec3_length(cross);
                    let mag = 0.5 * air * area * speed * speed;
                    force = [mag * direction[0], mag * direction[1], mag * direction[2]];
                    torque = [
                        lever[1] * force[2] - lever[2] * force[1],
                        lever[2] * force[0] - lever[0] * force[2],
                        lever[0] * force[1] - lever[1] * force[0],
                    ];
                    let _ = lift;
                }
            }
            KIND_BOX => {
                let h = shape.half;
                let c = shape.geometry_center;
                let faces = [
                    ([1.0, 0.0, 0.0], h[1], h[2], h[0]),
                    ([-1.0, 0.0, 0.0], h[1], h[2], h[0]),
                    ([0.0, 1.0, 0.0], h[0], h[2], h[1]),
                    ([0.0, -1.0, 0.0], h[0], h[2], h[1]),
                    ([0.0, 0.0, 1.0], h[0], h[1], h[2]),
                    ([0.0, 0.0, -1.0], h[0], h[1], h[2]),
                ];
                for (normal, ua, ub, hc) in faces {
                    let center = [
                        c[0] + normal[0] * hc,
                        c[1] + normal[1] * hc,
                        c[2] + normal[2] * hc,
                    ];
                    let tangent = if normal[0].abs() < 0.5 {
                        [1.0, 0.0, 0.0]
                    } else {
                        [0.0, 1.0, 0.0]
                    };
                    let bitangent = [
                        normal[1] * tangent[2] - normal[2] * tangent[1],
                        normal[2] * tangent[0] - normal[0] * tangent[2],
                        normal[0] * tangent[1] - normal[1] * tangent[0],
                    ];
                    let tlen = vec3_length(bitangent).max(1.0e-8);
                    let b = [bitangent[0] / tlen, bitangent[1] / tlen, bitangent[2] / tlen];
                    let t = [
                        b[1] * normal[2] - b[2] * normal[1],
                        b[2] * normal[0] - b[0] * normal[2],
                        b[0] * normal[1] - b[1] * normal[0],
                    ];
                    let p1 = [
                        center[0] + t[0] * ua + b[0] * ub,
                        center[1] + t[1] * ua + b[1] * ub,
                        center[2] + t[2] * ua + b[2] * ub,
                    ];
                    let p2 = [
                        center[0] - t[0] * ua + b[0] * ub,
                        center[1] - t[1] * ua + b[1] * ub,
                        center[2] - t[2] * ua + b[2] * ub,
                    ];
                    let p3 = [
                        center[0] - t[0] * ua - b[0] * ub,
                        center[1] - t[1] * ua - b[1] * ub,
                        center[2] - t[2] * ua - b[2] * ub,
                    ];
                    apply_triangle(&mut force, &mut torque, p1, p2, p3, normal);
                }
            }
            KIND_CONVEX_HULL => {
                if shape.hull_topology.len() >= 3 && !shape.hull_points.is_empty() {
                    for face in 0..shape.hull_planes.len() {
                        let Some(start) = shape
                            .hull_topology
                            .iter()
                            .position(|edge| edge[3] as usize == face)
                        else {
                            continue;
                        };
                        let mut edge = start;
                        let origin = |idx: usize| shape.hull_topology.get(idx).map(|e| e[2] as usize);
                        let Some(i0) = origin(edge) else {
                            continue;
                        };
                        loop {
                            let next = shape.hull_topology.get(edge).map(|e| e[0] as usize);
                            let Some(n1) = next else { break };
                            let Some(i1) = origin(n1) else { break };
                            let n2 = shape.hull_topology.get(n1).map(|e| e[0] as usize);
                            let Some(i2) = n2.and_then(origin) else { break };
                            if i1 != i0 && i2 != i0 {
                                if let (Some(p1), Some(p2), Some(p3)) = (
                                    shape.hull_points.get(i0),
                                    shape.hull_points.get(i1),
                                    shape.hull_points.get(i2),
                                ) {
                                    let n = shape.hull_planes[face];
                                    apply_triangle(
                                        &mut force,
                                        &mut torque,
                                        *p1,
                                        *p2,
                                        *p3,
                                        [n[0], n[1], n[2]],
                                    );
                                }
                            }
                            edge = n1;
                            if edge == start {
                                break;
                            }
                        }
                    }
                }
            }
            _ => {}
        }
        if let Some(body) = body_mut(w, body_id) {
            add_wind_force(body, force, torque, wake);
            w.bodies_dirty = true;
        }
    });
}

pub fn b3_shape_set_mesh_material(id: ShapeId, index: usize, material: SurfaceMaterial) {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        let Some(shape) = w
            .shapes
            .get_mut(id.index1.saturating_sub(1) as usize)
            .and_then(Option::as_mut)
        else {
            return;
        };
        if shape.kind != KIND_MESH || index >= shape.mesh_materials.len() {
            return;
        }
        shape.mesh_materials[index] = material;
        if index == 0 {
            shape.friction = material.friction;
            shape.restitution = material.restitution;
            shape.rolling = material.rolling_resistance;
            shape.tangent_velocity = material.tangent_velocity;
            shape.user_material_id = material.user_material_id;
            shape.custom_color = material.custom_color;
        }
        mark_scene_dirty(w);
    });
}

pub fn b3_shape_get_mesh_material(id: ShapeId, index: usize) -> SurfaceMaterial {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        w.shapes
            .get(id.index1.saturating_sub(1) as usize)?
            .as_ref()?
            .mesh_materials
            .get(index)
            .copied()
    })
    .flatten()
    .unwrap_or_else(crate::api::b3_default_surface_material)
}

pub fn b3_shape_set_mesh_material_count(id: ShapeId, count: usize) {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        let Some(shape) = w
            .shapes
            .get_mut(id.index1.saturating_sub(1) as usize)
            .and_then(Option::as_mut)
        else {
            return;
        };
        if shape.kind != KIND_MESH || count == 0 || count > 256 {
            return;
        }
        let base = shape
            .mesh_materials
            .first()
            .copied()
            .unwrap_or_else(crate::api::b3_default_surface_material);
        shape.mesh_materials.resize(count, base);
        mark_scene_dirty(w);
    });
}

pub fn b3_shape_is_sensor(id: ShapeId) -> bool {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        w.shapes
            .get(id.index1.saturating_sub(1) as usize)
            .and_then(|shape| shape.as_ref())
            .is_some_and(|shape| (shape.event_flags & SHAPE_IS_SENSOR) != 0)
    })
    .unwrap_or(false)
}

pub fn b3_shape_set_sensor(id: ShapeId, is_sensor: bool) {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        let Some(shape) = w
            .shapes
            .get_mut(id.index1.saturating_sub(1) as usize)
            .and_then(|shape| shape.as_mut())
        else {
            return;
        };
        if is_sensor {
            shape.event_flags |= SHAPE_IS_SENSOR;
        } else {
            shape.event_flags &= !SHAPE_IS_SENSOR;
        }
        mark_scene_dirty(w);
    });
}

pub fn b3_shape_enable_sensor_events(id: ShapeId, enable: bool) {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| {
        let Some(shape) = w
            .shapes
            .get_mut(id.index1.saturating_sub(1) as usize)
            .and_then(|shape| shape.as_mut())
        else {
            return;
        };
        if enable {
            shape.event_flags |= SHAPE_ENABLE_SENSOR_EVENTS;
        } else {
            shape.event_flags &= !SHAPE_ENABLE_SENSOR_EVENTS;
        }
        mark_scene_dirty(w);
    });
}

pub fn b3_shape_are_sensor_events_enabled(id: ShapeId) -> bool {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        w.shapes
            .get(id.index1.saturating_sub(1) as usize)
            .and_then(|shape| shape.as_ref())
            .is_some_and(|shape| (shape.event_flags & SHAPE_ENABLE_SENSOR_EVENTS) != 0)
    })
    .unwrap_or(false)
}

pub fn b3_shape_get_sensor_capacity(id: ShapeId) -> i32 {
    if !b3_shape_is_sensor(id) {
        return 0;
    }
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        w.sensor_overlaps
            .get(&id.index1)
            .map_or(0, |overlaps| overlaps.len() as i32)
    })
    .unwrap_or(0)
}

pub fn b3_shape_get_sensor_data(id: ShapeId, output: &mut [ShapeId]) -> usize {
    if !b3_shape_is_sensor(id) {
        return 0;
    }
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world(world, |w| {
        let Some(overlaps) = w.sensor_overlaps.get(&id.index1) else {
            return 0;
        };
        let count = overlaps.len().min(output.len());
        output[..count].copy_from_slice(&overlaps[..count]);
        count
    })
    .unwrap_or(0)
}

pub fn b3_destroy_shape(id: ShapeId, update_body_mass: bool) {
    let world = WorldId {
        index1: id.world0,
        generation: 1,
    };
    with_world_mut(world, |w| destroy_shape_inner(w, id, update_body_mass));
}

fn destroy_shape_inner(w: &mut WorldInner, id: ShapeId, update_body_mass: bool) {
    let index = id.index1.saturating_sub(1) as usize;
    let Some(shape) = w.shapes.get_mut(index).and_then(Option::take) else {
        return;
    };
    let owner = live_body_id(w, id.world0, shape.body_index);
    if let Some(cpu) = body_mut(w, owner) {
        cpu.shape_indices.retain(|&slot| slot != index);
    }
    if shape.public_kind == PUBLIC_KIND_COMPOUND {
        let children: Vec<ShapeId> = w
            .shapes
            .iter()
            .enumerate()
            .filter_map(|(child_index, slot)| {
                let child = slot.as_ref()?;
                (child.compound_parent == id.index1).then_some(ShapeId {
                    index1: child_index as i32 + 1,
                    world0: id.world0,
                    generation: child.generation,
                })
            })
            .collect();
        for child in children {
            destroy_shape_inner(w, child, false);
        }
    }
    if (shape.event_flags & SHAPE_IS_SENSOR) != 0 {
        if let Some(overlaps) = w.sensor_overlaps.remove(&id.index1) {
            for visitor_shape_id in overlaps {
                w.deferred_sensor_end_events.push(SensorEndTouchEvent {
                    sensor_shape_id: id,
                    visitor_shape_id,
                });
            }
        }
    }
    let mut retained = HashMap::with_capacity(w.live_contacts.len());
    for (key, contact) in w.live_contacts.drain() {
        if contact.shape_id_a.index1 == id.index1 || contact.shape_id_b.index1 == id.index1 {
            if contact.events_enabled {
                w.deferred_contact_end_events.push(ContactEndTouchEvent {
                    shape_id_a: contact.shape_id_a,
                    shape_id_b: contact.shape_id_b,
                    contact_id: contact.contact_id,
                });
            }
        } else {
            retained.insert(key, contact);
        }
    }
    w.live_contacts = retained;
    mark_scene_dirty(w);
    let body = live_body_id(w, id.world0, shape.body_index);
    if update_body_mass {
        apply_body_mass_from_shapes_inner(w, body);
    } else {
        update_body_extents(w, body);
    }
}

pub fn b3_world_contact_event_ptrs(
    id: WorldId,
) -> (
    *const ContactBeginTouchEvent,
    i32,
    *const ContactEndTouchEvent,
    i32,
    *const ContactHitEvent,
    i32,
) {
    with_world(id, |w| {
        (
            w.contact_begin_events.as_ptr(),
            w.contact_begin_events.len() as i32,
            w.contact_end_events.as_ptr(),
            w.contact_end_events.len() as i32,
            w.contact_hit_events.as_ptr(),
            w.contact_hit_events.len() as i32,
        )
    })
    .unwrap_or((
        std::ptr::null(),
        0,
        std::ptr::null(),
        0,
        std::ptr::null(),
        0,
    ))
}

pub fn b3_world_body_event_ptrs(id: WorldId) -> (*const BodyMoveEvent, i32) {
    with_world(id, |w| {
        (w.body_move_events.as_ptr(), w.body_move_events.len() as i32)
    })
    .unwrap_or((std::ptr::null(), 0))
}

pub fn b3_world_joint_event_ptrs(id: WorldId) -> (*const JointEvent, i32) {
    with_world(id, |w| {
        (w.joint_events.as_ptr(), w.joint_events.len() as i32)
    })
    .unwrap_or((std::ptr::null(), 0))
}

pub fn b3_world_set_hit_event_threshold(id: WorldId, value: f32) {
    with_world_mut(id, |w| {
        w.def.hit_event_threshold = value.clamp(0.0, f32::MAX)
    });
}

pub fn b3_world_get_hit_event_threshold(id: WorldId) -> f32 {
    with_world(id, |w| w.def.hit_event_threshold).unwrap_or(0.0)
}

pub fn b3_world_sensor_event_ptrs(
    id: WorldId,
) -> (
    *const SensorBeginTouchEvent,
    i32,
    *const SensorEndTouchEvent,
    i32,
) {
    with_world(id, |w| {
        (
            w.sensor_begin_events.as_ptr(),
            w.sensor_begin_events.len() as i32,
            w.sensor_end_events.as_ptr(),
            w.sensor_end_events.len() as i32,
        )
    })
    .unwrap_or((std::ptr::null(), 0, std::ptr::null(), 0))
}

pub fn b3_body_is_static(id: BodyId) -> bool {
    let world = world_id_from_body(id);
    with_world_no_sync(world, |w| {
        body_ref(w, id).is_some_and(|b| (b.gpu.flags & FLAG_STATIC) != 0)
    })
    .unwrap_or(true)
}

pub fn b3_body_inv_mass(id: BodyId) -> f32 {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).map(|b| b.gpu.inv_mass).unwrap_or(0.0)
    })
    .unwrap_or(0.0)
}

pub fn b3_body_set_user_data(id: BodyId, user_data: usize) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        if let Some(body) = body_mut(w, id) {
            body.user_data = user_data;
        }
    });
}

pub fn b3_body_get_user_data(id: BodyId) -> usize {
    let world = world_id_from_body(id);
    with_world(world, |w| body_ref(w, id).map_or(0, |body| body.user_data)).unwrap_or(0)
}

pub fn b3_body_apply_mass_from_shapes(id: BodyId) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| apply_body_mass_from_shapes_inner(w, id));
}

pub fn b3_destroy_body(id: BodyId) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        if body_ref(w, id).is_none() { return; }
        if id.index1 > 0 {
            let destroyed_shapes: Vec<i32> = w
                .shapes
                .iter()
                .enumerate()
                .filter_map(|(index, shape)| {
                    shape
                        .as_ref()
                        .filter(|shape| shape.body_index == id.index1)
                        .map(|_| index as i32 + 1)
                })
                .collect();
            let mut retained = HashMap::with_capacity(w.live_contacts.len());
            for (key, contact) in w.live_contacts.drain() {
                if destroyed_shapes.contains(&contact.shape_id_a.index1)
                    || destroyed_shapes.contains(&contact.shape_id_b.index1)
                {
                    if contact.events_enabled {
                        w.deferred_contact_end_events.push(ContactEndTouchEvent {
                            shape_id_a: contact.shape_id_a,
                            shape_id_b: contact.shape_id_b,
                            contact_id: contact.contact_id,
                        });
                    }
                } else {
                    retained.insert(key, contact);
                }
            }
            w.live_contacts = retained;
            for shape_index1 in &destroyed_shapes {
                if let Some(overlaps) = w.sensor_overlaps.remove(shape_index1) {
                    let generation = w
                        .shapes
                        .get((*shape_index1 - 1) as usize)
                        .and_then(|shape| shape.as_ref())
                        .map_or(1, |shape| shape.generation);
                    let sensor_shape_id = ShapeId {
                        index1: *shape_index1,
                        world0: world.index1,
                        generation,
                    };
                    for visitor_shape_id in overlaps {
                        w.deferred_sensor_end_events.push(SensorEndTouchEvent {
                            sensor_shape_id,
                            visitor_shape_id,
                        });
                    }
                }
            }
            for shape in &mut w.shapes {
                if shape
                    .as_ref()
                    .is_some_and(|shape| shape.body_index == id.index1)
                {
                    *shape = None;
                }
            }
            // Native destroys attached joints with the body. Keeping them alive
            // would attach stale constraints to a later occupant of this slot.
            for (joint_index, joint) in w.joints.iter_mut().enumerate() {
                if joint.kind != JOINT_NONE && (joint.a == (id.index1 - 1) as u32 || joint.b == (id.index1 - 1) as u32) {
                    *joint = JointGpu::default();
                    if let Some(meta) = w.joint_meta.get_mut(joint_index) { *meta = None; }
                }
            }
            let index = id.index1 as usize - 1;
            if let Some(slot) = w.bodies.get_mut(index) {
                if slot.take().is_some() {
                    w.free_bodies.push(index);
                }
                mark_scene_dirty(w);
            }
        }
    });
}

pub fn b3_body_set_transform(id: BodyId, pos: [f32; 3], rot: [f32; 4]) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        let epoch = snapshot_epoch(w);
        if let Some(cpu) = body_mut(w, id) {
            let previous = body_origin(cpu);
            let previous_rotation = cpu.gpu.rot;
            let world_center = quat_rotate(rot, cpu.local_center);
            cpu.gpu.pos = [
                pos[0] + world_center[0],
                pos[1] + world_center[1],
                pos[2] + world_center[2],
            ];
            cpu.gpu.rot = rot;
            cpu.gpu.flags &= !FLAG_SLEEP;
            cpu.gpu.sleep_time = 0.0;
            cpu.host_epoch = epoch.saturating_add(1);
            let commands = w.pending_fat_transforms.entry((id.index1, id.generation)).or_default();
            if commands.try_reserve(2).is_err() {
                w.physics_invalid = true;
                w.gpu_fail = std::ffi::CString::new("transform history allocation failed").ok();
                return;
            }
            if commands.is_empty() {
                commands.push([previous[0], previous[1], previous[2], 0.0,
                    previous_rotation[0], previous_rotation[1], previous_rotation[2], previous_rotation[3]]);
            }
            commands.push([pos[0], pos[1], pos[2], 0.0, rot[0], rot[1], rot[2], rot[3]]);
            w.bodies_dirty = true;
            w.query_state = w.query_state.saturating_add(1);
        }
    });
}

pub fn b3_body_set_linear_velocity(id: BodyId, velocity: [f32; 3]) {
    if !velocity.iter().all(|v| v.is_finite()) { return; }
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        let epoch = snapshot_epoch(w);
        if let Some(cpu) = body_mut(w, id) {
            if cpu.gpu.flags & (FLAG_STATIC | FLAG_DISABLED) != 0 { return; }
            if velocity.iter().any(|v| *v != 0.0) { wake_body(cpu); }
            if cpu.gpu.flags & FLAG_SLEEP != 0 { return; }
            cpu.gpu.vel = velocity;
            cpu.host_epoch = epoch.saturating_add(1);
            w.bodies_dirty = true;
        }
    });
}

pub fn b3_body_get_linear_velocity(id: BodyId) -> [f32; 3] {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).map_or([0.0; 3], |body| body.gpu.vel)
    })
    .unwrap_or([0.0; 3])
}

pub fn b3_body_set_angular_velocity(id: BodyId, velocity: [f32; 3]) {
    if !velocity.iter().all(|v| v.is_finite()) { return; }
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        let epoch = snapshot_epoch(w);
        if let Some(cpu) = body_mut(w, id) {
            if cpu.gpu.flags & (FLAG_STATIC | FLAG_DISABLED) != 0 { return; }
            let velocity = std::array::from_fn(|i| {
                if cpu.gpu.flags & (1u32 << (11 + i)) != 0 { 0.0 } else { velocity[i] }
            });
            if velocity.iter().any(|v| *v != 0.0) { wake_body(cpu); }
            if cpu.gpu.flags & FLAG_SLEEP != 0 { return; }
            cpu.gpu.omega = velocity;
            cpu.host_epoch = epoch.saturating_add(1);
            w.bodies_dirty = true;
        }
    });
}

pub fn b3_body_get_angular_velocity(id: BodyId) -> [f32; 3] {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).map_or([0.0; 3], |body| body.gpu.omega)
    })
    .unwrap_or([0.0; 3])
}

/// Runtime contact recycling is host-owned metadata. Unchanged settings must
/// not turn per-frame UI setters into full-world pose/contact downloads.
pub fn b3_body_enable_contact_recycling(id: BodyId, enable: bool) {
    let world = world_id_from_body(id);
    with_world_mut_no_sync(world, |w| {
        let Some(body) = body_ref(w, id) else { return; };
        let current = body.gpu.flags & FLAG_DISABLE_CONTACT_RECYCLING == 0;
        if current == enable { return; }
        // Body uploads include mutable state: finalize a pending step before
        // editing its flag so the upload cannot restore an old pose/velocity.
        ensure_cpu_mirror_world(w, world);
        let epoch = snapshot_epoch(w);
        if let Some(body) = body_mut(w, id) {
            if enable {
                body.gpu.flags &= !FLAG_DISABLE_CONTACT_RECYCLING;
            } else {
                body.gpu.flags |= FLAG_DISABLE_CONTACT_RECYCLING;
            }
            body.host_epoch = epoch.saturating_add(1);
            w.bodies_dirty = true;
        }
    });
}

pub fn b3_body_is_contact_recycling_enabled(id: BodyId) -> bool {
    with_world_no_sync(world_id_from_body(id), |w| {
        body_ref(w, id).is_some_and(|body| body.gpu.flags & FLAG_DISABLE_CONTACT_RECYCLING == 0)
    }).unwrap_or(false)
}

pub fn b3_body_set_bullet(id: BodyId, bullet: bool) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        let epoch = snapshot_epoch(w);
        if let Some(body) = body_mut(w, id) {
            if bullet {
                body.gpu.flags |= FLAG_BULLET;
            } else {
                body.gpu.flags &= !FLAG_BULLET;
            }
            body.host_epoch = epoch.saturating_add(1);
            w.bodies_dirty = true;
        }
    });
}

pub fn b3_body_is_bullet(id: BodyId) -> bool {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).is_some_and(|body| body.gpu.flags & FLAG_BULLET != 0)
    })
    .unwrap_or(false)
}

pub fn b3_body_allow_fast_rotation(id: BodyId, allow: bool) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        let epoch = snapshot_epoch(w);
        if let Some(body) = body_mut(w, id) {
            if allow {
                body.gpu.flags |= FLAG_ALLOW_FAST_ROTATION;
            } else {
                body.gpu.flags &= !FLAG_ALLOW_FAST_ROTATION;
            }
            body.host_epoch = epoch.saturating_add(1);
            w.bodies_dirty = true;
        }
    });
}

pub fn b3_body_is_fast_rotation_allowed(id: BodyId) -> bool {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).is_some_and(|body| body.gpu.flags & FLAG_ALLOW_FAST_ROTATION != 0)
    })
    .unwrap_or(false)
}

fn mul_symmetric(m: [f32; 6], v: [f32; 3]) -> [f32; 3] {
    [
        m[0] * v[0] + m[3] * v[1] + m[4] * v[2],
        m[3] * v[0] + m[1] * v[1] + m[5] * v[2],
        m[4] * v[0] + m[5] * v[1] + m[2] * v[2],
    ]
}

fn apply_world_angular_impulse(body: &mut CpuBody, impulse: [f32; 3]) {
    let inverse_rotation = [
        -body.gpu.rot[0],
        -body.gpu.rot[1],
        -body.gpu.rot[2],
        body.gpu.rot[3],
    ];
    let local_impulse = quat_rotate(inverse_rotation, impulse);
    let local_delta = mul_symmetric(invert_symmetric(body.local_inertia), local_impulse);
    let delta = quat_rotate(body.gpu.rot, local_delta);
    for (omega, value) in body.gpu.omega.iter_mut().zip(delta) {
        *omega += value;
    }
}

pub fn b3_body_apply_force(id: BodyId, force: [f32; 3], point: [f32; 3], wake: bool) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        let epoch = snapshot_epoch(w);
        if let Some(body) = body_mut(w, id) {
            if (body.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC | FLAG_DISABLED)) != 0 {
                return;
            }
            if wake {
                wake_body(body);
            } else if body.gpu.flags & FLAG_SLEEP != 0 {
                return;
            }
            for (sum, value) in body.force.iter_mut().zip(force) {
                *sum += value;
            }
            let r = [
                point[0] - body.gpu.pos[0],
                point[1] - body.gpu.pos[1],
                point[2] - body.gpu.pos[2],
            ];
            body.torque[0] += r[1] * force[2] - r[2] * force[1];
            body.torque[1] += r[2] * force[0] - r[0] * force[2];
            body.torque[2] += r[0] * force[1] - r[1] * force[0];
            body.host_epoch = epoch.saturating_add(1);
            w.bodies_dirty = true;
        }
    });
}

pub fn b3_body_apply_force_to_center(id: BodyId, force: [f32; 3], wake: bool) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        let epoch = snapshot_epoch(w);
        if let Some(body) = body_mut(w, id) {
            if (body.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC | FLAG_DISABLED)) != 0 {
                return;
            }
            if wake {
                wake_body(body);
            } else if body.gpu.flags & FLAG_SLEEP != 0 {
                return;
            }
            for (sum, value) in body.force.iter_mut().zip(force) {
                *sum += value;
            }
            body.host_epoch = epoch.saturating_add(1);
            w.bodies_dirty = true;
        }
    });
}

pub fn b3_body_apply_torque(id: BodyId, torque: [f32; 3], wake: bool) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        let epoch = snapshot_epoch(w);
        if let Some(body) = body_mut(w, id) {
            if (body.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC | FLAG_DISABLED)) != 0 {
                return;
            }
            if wake {
                wake_body(body);
            } else if body.gpu.flags & FLAG_SLEEP != 0 {
                return;
            }
            for (sum, value) in body.torque.iter_mut().zip(torque) {
                *sum += value;
            }
            body.host_epoch = epoch.saturating_add(1);
            w.bodies_dirty = true;
        }
    });
}

pub fn b3_body_get_mass(id: BodyId) -> f32 {
    with_world_no_sync(world_id_from_body(id), |w| {
        body_ref(w, id).map(|body| body.mass).unwrap_or(0.0)
    }).unwrap_or(0.0)
}

pub fn b3_body_get_local_rotational_inertia(id: BodyId) -> [[f32; 3]; 3] {
    b3_body_get_mass_data(id).inertia
}

pub fn b3_body_get_world_inverse_rotational_inertia(id: BodyId) -> [[f32; 3]; 3] {
    with_world(world_id_from_body(id), |w| {
        let Some(body) = body_ref(w, id) else { return [[0.0; 3]; 3]; };
        let fixed = FLAG_LOCK_ANG_X | FLAG_LOCK_ANG_Y | FLAG_LOCK_ANG_Z;
        if body.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC) != 0 || body.gpu.flags & fixed == fixed {
            return [[0.0; 3]; 3];
        }
        let i = invert_symmetric(body.local_inertia);
        let local = glam::Mat3::from_cols_array_2d(&[
            [i[0],i[3],i[4]], [i[3],i[1],i[5]], [i[4],i[5],i[2]]]);
        let rotation = glam::Mat3::from_quat(glam::Quat::from_array(body.gpu.rot));
        (rotation * local * rotation.transpose()).to_cols_array_2d()
    }).unwrap_or([[0.0; 3]; 3])
}

pub fn b3_body_get_mass_data(id: BodyId) -> MassData {
    let world = world_id_from_body(id);
    // Mass properties change on host-side shape/mass edits, never integration.
    with_world_no_sync(world, |w| {
        body_ref(w, id).map(|body| {
            let fixed = FLAG_LOCK_ANG_X | FLAG_LOCK_ANG_Y | FLAG_LOCK_ANG_Z;
            let i = if body.gpu.flags & fixed == fixed || body.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC) != 0 { [0.0; 6] } else { body.local_inertia };
            MassData {
                mass: body.mass,
                center: body.local_center,
                inertia: [
                    [i[0], i[3], i[4]],
                    [i[3], i[1], i[5]],
                    [i[4], i[5], i[2]],
                ],
            }
        })
    })
    .flatten()
    .unwrap_or_default()
}

pub fn b3_body_set_mass_data(id: BodyId, data: MassData) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        let Some(cpu) = body_mut(w, id) else {
            return;
        };
        if (cpu.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC)) != 0 {
            return;
        }
        let old_center = cpu.gpu.pos;
        let origin = body_origin(cpu);
        cpu.mass = data.mass.max(0.0);
        cpu.local_center = data.center;
        cpu.local_inertia = [
            data.inertia[0][0],
            data.inertia[1][1],
            data.inertia[2][2],
            data.inertia[0][1],
            data.inertia[0][2],
            data.inertia[1][2],
        ];
        let world_center = quat_rotate(cpu.gpu.rot, data.center);
        cpu.gpu.pos = [
            origin[0] + world_center[0],
            origin[1] + world_center[1],
            origin[2] + world_center[2],
        ];
        let center_shift = [
            cpu.gpu.pos[0] - old_center[0],
            cpu.gpu.pos[1] - old_center[1],
            cpu.gpu.pos[2] - old_center[2],
        ];
        let angular = cpu.gpu.omega;
        cpu.gpu.vel[0] += angular[1] * center_shift[2] - angular[2] * center_shift[1];
        cpu.gpu.vel[1] += angular[2] * center_shift[0] - angular[0] * center_shift[2];
        cpu.gpu.vel[2] += angular[0] * center_shift[1] - angular[1] * center_shift[0];
        cpu.gpu.inv_mass = if data.mass > 0.0 { 1.0 / data.mass } else { 0.0 };
        // Native SetMassData treats translational and rotational mass independently.
        let inverse = invert_symmetric(cpu.local_inertia);
        cpu.gpu.inv_inertia = [inverse[0], inverse[1], inverse[2]];
        update_body_extents(w, id);
        mark_scene_dirty(w);
    });
}

pub fn b3_body_get_world_point(id: BodyId, local: [f32; 3]) -> [f32; 3] {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).map(|body| {
            let rotated = quat_rotate(body.gpu.rot, local);
            let origin = body_origin(body);
            [
                origin[0] + rotated[0],
                origin[1] + rotated[1],
                origin[2] + rotated[2],
            ]
        })
    })
    .flatten()
    .unwrap_or(local)
}

pub fn b3_body_get_local_point(id: BodyId, world_point: [f32; 3]) -> [f32; 3] {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).map(|body| {
            let origin = body_origin(body);
            let delta = [
                world_point[0] - origin[0],
                world_point[1] - origin[1],
                world_point[2] - origin[2],
            ];
            quat_rotate(
                [
                    -body.gpu.rot[0],
                    -body.gpu.rot[1],
                    -body.gpu.rot[2],
                    body.gpu.rot[3],
                ],
                delta,
            )
        })
    })
    .flatten()
    .unwrap_or(world_point)
}

pub fn b3_body_get_world_vector(id: BodyId, local: [f32; 3]) -> [f32; 3] {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).map(|body| quat_rotate(body.gpu.rot, local))
    })
    .flatten()
    .unwrap_or(local)
}

pub fn b3_body_get_local_vector(id: BodyId, world_vector: [f32; 3]) -> [f32; 3] {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).map(|body| {
            quat_rotate(
                [
                    -body.gpu.rot[0],
                    -body.gpu.rot[1],
                    -body.gpu.rot[2],
                    body.gpu.rot[3],
                ],
                world_vector,
            )
        })
    })
    .flatten()
    .unwrap_or(world_vector)
}

pub fn b3_body_get_world_point_velocity(id: BodyId, world_point: [f32; 3]) -> [f32; 3] {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).map(|body| {
            let r = [
                world_point[0] - body.gpu.pos[0],
                world_point[1] - body.gpu.pos[1],
                world_point[2] - body.gpu.pos[2],
            ];
            let w = body.gpu.omega;
            [
                body.gpu.vel[0] + w[1] * r[2] - w[2] * r[1],
                body.gpu.vel[1] + w[2] * r[0] - w[0] * r[2],
                body.gpu.vel[2] + w[0] * r[1] - w[1] * r[0],
            ]
        })
    })
    .flatten()
    .unwrap_or([0.0; 3])
}

pub fn b3_body_get_local_point_velocity(id: BodyId, local: [f32; 3]) -> [f32; 3] {
    let world_point = b3_body_get_world_point(id, local);
    b3_body_get_world_point_velocity(id, world_point)
}

pub fn b3_body_is_awake(id: BodyId) -> bool {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).is_some_and(|body| {
            (body.gpu.flags & (FLAG_SLEEP | FLAG_DISABLED)) == 0
                && (body.gpu.flags & FLAG_STATIC) == 0
        })
    })
    .unwrap_or(false)
}

pub fn b3_body_is_enabled(id: BodyId) -> bool {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).is_some_and(|body| (body.gpu.flags & FLAG_DISABLED) == 0)
    })
    .unwrap_or(false)
}

pub fn b3_body_set_enabled(id: BodyId, enable: bool) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        let epoch = snapshot_epoch(w);
        if let Some(body) = body_mut(w, id) {
            if enable {
                body.gpu.flags &= !FLAG_DISABLED;
                body.gpu.flags &= !FLAG_SLEEP;
                body.gpu.sleep_time = 0.0;
            } else {
                body.gpu.flags |= FLAG_DISABLED | FLAG_SLEEP;
                body.gpu.vel = [0.0; 3];
                body.gpu.omega = [0.0; 3];
                body.force = [0.0; 3];
                body.torque = [0.0; 3];
            }
            body.host_epoch = epoch.saturating_add(1);
            w.bodies_dirty = true;
            mark_scene_dirty(w);
        }
    });
}

const MOTION_LOCK_FLAGS: u32 = FLAG_LOCK_LIN_X
    | FLAG_LOCK_LIN_Y
    | FLAG_LOCK_LIN_Z
    | FLAG_LOCK_ANG_X
    | FLAG_LOCK_ANG_Y
    | FLAG_LOCK_ANG_Z;

pub fn b3_body_set_motion_locks(id: BodyId, locks: MotionLocks) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        let epoch = snapshot_epoch(w);
        if let Some(body) = body_mut(w, id) {
            body.gpu.flags = (body.gpu.flags & !MOTION_LOCK_FLAGS) | locks.to_flags();
            if locks.linear_x {
                body.gpu.vel[0] = 0.0;
            }
            if locks.linear_y {
                body.gpu.vel[1] = 0.0;
            }
            if locks.linear_z {
                body.gpu.vel[2] = 0.0;
            }
            if locks.angular_x {
                body.gpu.omega[0] = 0.0;
            }
            if locks.angular_y {
                body.gpu.omega[1] = 0.0;
            }
            if locks.angular_z {
                body.gpu.omega[2] = 0.0;
            }
            body.host_epoch = epoch.saturating_add(1);
            w.bodies_dirty = true;
        }
    });
}

pub fn b3_body_get_motion_locks(id: BodyId) -> MotionLocks {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).map_or(MotionLocks::default(), |body| {
            MotionLocks::from_flags(body.gpu.flags)
        })
    })
    .unwrap_or_default()
}

pub fn b3_body_get_local_center(id: BodyId) -> [f32; 3] {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).map_or([0.0; 3], |body| body.local_center)
    })
    .unwrap_or([0.0; 3])
}

pub fn b3_body_get_linear_damping(id: BodyId) -> f32 {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).map_or(0.0, |body| body.gpu.linear_damping)
    })
    .unwrap_or(0.0)
}

pub fn b3_body_get_angular_damping(id: BodyId) -> f32 {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).map_or(0.0, |body| body.gpu.angular_damping)
    })
    .unwrap_or(0.0)
}

pub fn b3_body_get_gravity_scale(id: BodyId) -> f32 {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id).map_or(0.0, |body| body.gpu.gravity_scale)
    })
    .unwrap_or(0.0)
}

pub fn b3_body_get_joint_count(id: BodyId) -> i32 {
    let world = world_id_from_body(id);
    let gpu = gpu_index(id);
    with_world(world, |w| {
        if body_ref(w, id).is_none() {
            return 0;
        }
        w.joints
            .iter()
            .filter(|joint| joint.kind != JOINT_NONE && (joint.a == gpu || joint.b == gpu))
            .count() as i32
    })
    .unwrap_or(0)
}

pub fn b3_body_get_joints(id: BodyId, output: &mut [JointId]) -> usize {
    let world = world_id_from_body(id);
    let gpu = gpu_index(id);
    with_world(world, |w| {
        if body_ref(w, id).is_none() {
            return 0;
        }
        let mut count = 0;
        for (index, joint) in w.joints.iter().enumerate() {
            if joint.kind == JOINT_NONE || (joint.a != gpu && joint.b != gpu) {
                continue;
            }
            if count >= output.len() {
                break;
            }
            output[count] = JointId {
                index1: index as i32 + 1,
                world0: id.world0,
                generation: 1,
            };
            count += 1;
        }
        count
    })
    .unwrap_or(0)
}

pub fn b3_body_apply_linear_impulse(id: BodyId, impulse: [f32; 3], point: [f32; 3], wake: bool) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        let epoch = snapshot_epoch(w);
        if let Some(body) = body_mut(w, id) {
            if (body.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC | FLAG_DISABLED)) != 0
                || (!wake && body.gpu.flags & FLAG_SLEEP != 0)
            {
                return;
            }
            for (velocity, value) in body.gpu.vel.iter_mut().zip(impulse) {
                *velocity += body.gpu.inv_mass * value;
            }
            let r = [
                point[0] - body.gpu.pos[0],
                point[1] - body.gpu.pos[1],
                point[2] - body.gpu.pos[2],
            ];
            apply_world_angular_impulse(
                body,
                [
                    r[1] * impulse[2] - r[2] * impulse[1],
                    r[2] * impulse[0] - r[0] * impulse[2],
                    r[0] * impulse[1] - r[1] * impulse[0],
                ],
            );
            body.gpu.flags &= !FLAG_SLEEP;
            body.gpu.sleep_time = 0.0;
            body.host_epoch = epoch.saturating_add(1);
            w.bodies_dirty = true;
        }
    });
}

pub fn b3_body_apply_linear_impulse_to_center(id: BodyId, impulse: [f32; 3], wake: bool) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        let epoch = snapshot_epoch(w);
        if let Some(body) = body_mut(w, id) {
            if (body.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC | FLAG_DISABLED)) != 0
                || (!wake && body.gpu.flags & FLAG_SLEEP != 0)
            {
                return;
            }
            for (velocity, value) in body.gpu.vel.iter_mut().zip(impulse) {
                *velocity += body.gpu.inv_mass * value;
            }
            body.gpu.flags &= !FLAG_SLEEP;
            body.gpu.sleep_time = 0.0;
            body.host_epoch = epoch.saturating_add(1);
            w.bodies_dirty = true;
        }
    });
}

pub fn b3_body_apply_angular_impulse(id: BodyId, impulse: [f32; 3], wake: bool) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        let epoch = snapshot_epoch(w);
        if let Some(body) = body_mut(w, id) {
            if (body.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC | FLAG_DISABLED)) != 0
                || (!wake && body.gpu.flags & FLAG_SLEEP != 0)
            {
                return;
            }
            apply_world_angular_impulse(body, impulse);
            body.gpu.flags &= !FLAG_SLEEP;
            body.gpu.sleep_time = 0.0;
            body.host_epoch = epoch.saturating_add(1);
            w.bodies_dirty = true;
        }
    });
}

pub fn b3_body_get_type(id: BodyId) -> BodyType {
    let world = world_id_from_body(id);
    with_world(world, |w| {
        body_ref(w, id)
            .map(|body| {
                if body.gpu.flags & FLAG_KINEMATIC != 0 {
                    BodyType::Kinematic
                } else if body.gpu.flags & FLAG_STATIC != 0 {
                    BodyType::Static
                } else {
                    BodyType::Dynamic
                }
            })
            .unwrap_or(BodyType::Static)
    })
    .unwrap_or(BodyType::Static)
}

pub fn b3_body_set_type(id: BodyId, body_type: BodyType) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        // Native compound and height-field shapes are static-only. Reject the
        // transition before changing solver ownership or recomputing mass.
        if body_ref(w, id).is_none() {
            return;
        }
        if body_type != BodyType::Static && w.shapes.iter().flatten().any(|shape| {
            shape.body_index == id.index1
                && matches!(shape.public_kind, PUBLIC_KIND_COMPOUND | PUBLIC_KIND_HEIGHT_FIELD)
        }) {
            return;
        }
        let mut make_dynamic = false;
        if let Some(body) = body_mut(w, id) {
            body.gpu.flags &= !(FLAG_STATIC | FLAG_KINEMATIC | FLAG_SLEEP);
            match body_type {
                BodyType::Static => {
                    body.gpu.flags |= FLAG_STATIC;
                    body.mass = 0.0;
                    body.gpu.inv_mass = 0.0;
                    body.gpu.inv_inertia = [0.0; 3];
                }
                BodyType::Kinematic => {
                    body.gpu.flags |= FLAG_KINEMATIC;
                    body.mass = 0.0;
                    body.gpu.inv_mass = 0.0;
                    body.gpu.inv_inertia = [0.0; 3];
                }
                BodyType::Dynamic => make_dynamic = true,
            }
            body.gpu.sleep_time = 0.0;
            mark_scene_dirty(w);
        }
        if make_dynamic {
            apply_body_mass_from_shapes_inner(w, id);
        }
    });
}

pub fn b3_body_set_target_transform(
    id: BodyId,
    position: [f32; 3],
    rotation: [f32; 4],
    time_step: f32,
    wake: bool,
) {
    if time_step <= 0.0 {
        return;
    }
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        let epoch = snapshot_epoch(w);
        if let Some(body) = body_mut(w, id) {
            let target_offset = quat_rotate(rotation, body.local_center);
            let target_center = [
                position[0] + target_offset[0],
                position[1] + target_offset[1],
                position[2] + target_offset[2],
            ];
            let inv_dt = 1.0 / time_step;
            body.gpu.vel = [
                (target_center[0] - body.gpu.pos[0]) * inv_dt,
                (target_center[1] - body.gpu.pos[1]) * inv_dt,
                (target_center[2] - body.gpu.pos[2]) * inv_dt,
            ];
            let inverse = [
                -body.gpu.rot[0],
                -body.gpu.rot[1],
                -body.gpu.rot[2],
                body.gpu.rot[3],
            ];
            let mut delta = quat_mul(rotation, inverse);
            if delta[3] < 0.0 {
                delta = delta.map(|value| -value);
            }
            let sin_half = (delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]).sqrt();
            body.gpu.omega = if sin_half > 1e-8 {
                let scale = 2.0 * sin_half.atan2(delta[3]) * inv_dt / sin_half;
                [delta[0] * scale, delta[1] * scale, delta[2] * scale]
            } else {
                [0.0; 3]
            };
            if wake {
                body.gpu.flags &= !FLAG_SLEEP;
                body.gpu.sleep_time = 0.0;
            }
            body.host_epoch = epoch.saturating_add(1);
            w.bodies_dirty = true;
        }
    });
}

pub fn b3_body_set_awake(id: BodyId, awake: bool) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        let epoch = snapshot_epoch(w);
        if let Some(body) = body_mut(w, id) {
            if awake {
                body.gpu.flags &= !FLAG_SLEEP;
                body.gpu.sleep_time = 0.0;
            } else if body.gpu.flags & (FLAG_STATIC | FLAG_KINEMATIC) == 0 {
                body.gpu.flags |= FLAG_SLEEP;
                body.gpu.vel = [0.0; 3];
                body.gpu.omega = [0.0; 3];
            }
            body.host_epoch = epoch.saturating_add(1);
            w.bodies_dirty = true;
        }
    });
}

pub fn b3_body_set_linear_damping(id: BodyId, damping: f32) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        if let Some(body) = body_mut(w, id) {
            body.gpu.linear_damping = damping.max(0.0);
            mark_scene_dirty(w);
        }
    });
}

pub fn b3_body_set_angular_damping(id: BodyId, damping: f32) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        if let Some(body) = body_mut(w, id) {
            body.gpu.angular_damping = damping.max(0.0);
            mark_scene_dirty(w);
        }
    });
}

pub fn b3_body_set_gravity_scale(id: BodyId, scale: f32) {
    let world = world_id_from_body(id);
    with_world_mut(world, |w| {
        if let Some(body) = body_mut(w, id) {
            body.gpu.gravity_scale = scale;
            mark_scene_dirty(w);
        }
    });
}

#[cfg(test)]
mod joint_event_tests {
    use super::*;

    fn close(actual: f32, expected: f32) {
        assert!((actual - expected).abs() < 1.0e-6, "{actual} != {expected}");
    }

    #[test]
    fn callback_veto_suppresses_new_events_and_ends_existing_touch() {
        assert_eq!(
            disabled_lifecycle(CONTACT_TOUCHING | CONTACT_START_TOUCHING),
            1
        );
        assert_eq!(
            disabled_lifecycle(CONTACT_TOUCHING),
            1 | CONTACT_STOP_TOUCHING
        );
        assert_eq!(disabled_lifecycle(1), 1);
    }

    #[test]
    fn reaction_impulses_use_solver_accumulators_for_every_joint_kind() {
        let no_bodies = [];

        let mut joint = JointGpu {
            kind: JOINT_PARALLEL,
            perp_impulse: [3.0, 4.0],
            ..JointGpu::default()
        };
        assert_eq!(joint_reaction_impulses(&no_bodies, &joint), (0.0, 5.0));

        joint = JointGpu {
            kind: JOINT_DISTANCE,
            impulse: 2.0,
            lower_impulse: 3.0,
            upper_impulse: 1.0,
            motor_impulse: 2.0,
            ..JointGpu::default()
        };
        assert_eq!(joint_reaction_impulses(&no_bodies, &joint), (6.0, 0.0));

        joint = JointGpu {
            kind: JOINT_MOTOR,
            impulse: 1.0,
            perp_impulse: [2.0, 3.0],
            spring_impulse: 4.0,
            lower_impulse: 5.0,
            upper_impulse: 6.0,
            angular_impulse: [1.0, 2.0, 3.0],
            motor_impulse: 4.0,
            _pad2: [5.0, 6.0],
            ..JointGpu::default()
        };
        let (force, torque) = joint_reaction_impulses(&no_bodies, &joint);
        close(force, (155.0f32).sqrt());
        close(torque, (155.0f32).sqrt());

        joint = JointGpu {
            kind: JOINT_PRISMATIC,
            motor_impulse: 2.0,
            lower_impulse: 3.0,
            upper_impulse: 1.0,
            perp_impulse: [4.0, 8.0],
            angular_impulse: [0.0, 3.0, 4.0],
            ..JointGpu::default()
        };
        let (force, torque) = joint_reaction_impulses(&no_bodies, &joint);
        close(force, (96.0f32).sqrt());
        close(torque, 5.0);

        joint = JointGpu {
            kind: JOINT_REVOLUTE,
            angular_impulse: [0.0, 3.0, 4.0],
            perp_impulse: [3.0, 4.0],
            motor_impulse: 2.0,
            lower_impulse: 3.0,
            upper_impulse: 1.0,
            ..JointGpu::default()
        };
        let (force, torque) = joint_reaction_impulses(&no_bodies, &joint);
        close(force, 5.0);
        close(torque, (41.0f32).sqrt());

        joint = JointGpu {
            kind: JOINT_SPHERICAL,
            angular_impulse: [0.0, 3.0, 4.0],
            spring_angular_impulse: [1.0, 2.0, 3.0],
            motor_angular_impulse: [4.0, 5.0, 6.0],
            lower_impulse: 4.0,
            upper_impulse: 1.0,
            swing_impulse: 9.0,
            ..JointGpu::default()
        };
        let (force, torque) = joint_reaction_impulses(&no_bodies, &joint);
        close(force, 5.0);
        close(torque, (218.0f32).sqrt());

        joint = JointGpu {
            kind: JOINT_WELD,
            weld_linear_impulse: [0.0, 3.0, 4.0],
            weld_angular_impulse: [0.0, 5.0, 12.0],
            ..JointGpu::default()
        };
        assert_eq!(joint_reaction_impulses(&no_bodies, &joint), (5.0, 13.0));

        joint = JointGpu {
            kind: JOINT_WHEEL,
            perp_impulse: [3.0, 4.0],
            spring_impulse: 5.0,
            lower_impulse: 2.0,
            upper_impulse: 1.0,
            motor_impulse: -7.0,
            ..JointGpu::default()
        };
        let (force, torque) = joint_reaction_impulses(&no_bodies, &joint);
        close(force, (61.0f32).sqrt());
        close(torque, 7.0);

        joint.kind = JOINT_FILTER;
        assert_eq!(joint_reaction_impulses(&no_bodies, &joint), (0.0, 0.0));
    }

    #[cfg(not(target_arch = "wasm32"))]
    #[test]
    fn spawning_growth_reuses_programs_without_aliasing_world_state() {
        let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
        let mut def = crate::api::b3_default_world_def();
        def.gravity = [0.0; 3];
        let first = b3_create_world(gpu.clone(), &def);
        let second = b3_create_world(gpu, &def);
        let mut bd = crate::api::b3_default_body_def();
        bd.body_type = BodyType::Dynamic;
        bd.position = [0.0, 5.0, 0.0];
        bd.linear_velocity = [2.0, 0.0, 0.0];
        let moving = b3_create_body(first, &bd);
        let shape = Sphere { center: [0.0; 3], radius: 0.25 };
        b3_create_sphere_shape(moving, &crate::api::b3_default_shape_def(), &shape);
        bd.position = [20.0, 5.0, 0.0];
        bd.linear_velocity = [0.0; 3];
        let stationary = b3_create_body(second, &bd);
        b3_create_sphere_shape(stationary, &crate::api::b3_default_shape_def(), &shape);
        b3_world_step(first, FIXED_DT, DEFAULT_SUB_STEPS);
        b3_world_step(second, FIXED_DT, DEFAULT_SUB_STEPS);
        let pipeline = with_world_no_sync(first, |w| w.sim.as_ref().unwrap().shared_pipeline_test()).unwrap();
        assert_eq!(pipeline, with_world_no_sync(second, |w| w.sim.as_ref().unwrap().shared_pipeline_test()).unwrap());
        let capacity = with_world_no_sync(first, |w| w.sim.as_ref().unwrap().caps.bodies).unwrap();
        for _ in 0..capacity { b3_create_body(first, &bd); }
        b3_world_step(first, FIXED_DT, DEFAULT_SUB_STEPS);
        assert!(with_world_no_sync(first, |w| w.sim.as_ref().unwrap().caps.bodies > capacity).unwrap());
        assert_eq!(pipeline, with_world_no_sync(first, |w| w.sim.as_ref().unwrap().shared_pipeline_test()).unwrap());
        assert!((b3_body_get_position(moving)[0] - 4.0 * FIXED_DT).abs() < 1e-5);
        b3_destroy_world(first);
        b3_world_step(second, FIXED_DT, DEFAULT_SUB_STEPS);
        assert_eq!(b3_body_get_position(stationary), [20.0, 5.0, 0.0]);
        b3_destroy_world(second);
    }

    #[cfg(not(target_arch = "wasm32"))]
    #[test]
    fn spawn_within_capacity_preserves_contact_buffers() {
        let gpu = match pollster::block_on(crate::sim::GpuDevice::new(None)) {
            Ok(gpu) => gpu,
            Err(err) => {
                eprintln!("skip spawn capacity test: {err}");
                return;
            }
        };
        let world = b3_create_world(gpu, &crate::api::b3_default_world_def());
        crate::scenes::create_ground(world, 20.0);
        let mut body_def = crate::api::b3_default_body_def();
        body_def.body_type = BodyType::Dynamic;
        body_def.position = [0.0, 1.0, 0.0];
        let box_a = b3_create_body(world, &body_def);
        let hull = crate::api::b3_make_box_hull(0.5, 0.5, 0.5);
        let shape_def = crate::api::b3_default_shape_def();
        crate::api::b3_create_hull_shape(box_a, &shape_def, &hull);
        b3_world_step(world, FIXED_DT, DEFAULT_SUB_STEPS);
        let epoch = b3_world_contact_epoch(world);
        assert_ne!(epoch, 0);
        body_def.position = [0.0, 3.0, 0.0];
        let box_b = b3_create_body(world, &body_def);
        crate::api::b3_create_hull_shape(box_b, &shape_def, &hull);
        b3_world_step(world, FIXED_DT, DEFAULT_SUB_STEPS);
        assert_eq!(
            b3_world_contact_epoch(world),
            epoch,
            "appending a body within capacity must not replace contact buffers"
        );
        b3_body_set_linear_velocity(box_b, [0.0, -1.0, 0.0]);
        b3_world_step(world, FIXED_DT, DEFAULT_SUB_STEPS);
        assert_eq!(b3_world_contact_epoch(world), epoch);
        b3_destroy_world(world);
    }
}

#[cfg(test)]
mod recycling_metadata_tests {
    use super::*;

    #[test]
    fn recycling_reads_and_unchanged_setters_do_not_harvest_gpu_state() {
        let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
        let world = b3_create_world(gpu, &super::super::b3_default_world_def());
        let mut bd = super::super::b3_default_body_def();
        bd.body_type = BodyType::Dynamic;
        bd.position = [0.0,5.0,0.0];
        let body = b3_create_body(world, &bd);
        super::super::b3_create_sphere_shape(body, &super::super::b3_default_shape_def(),
            &Sphere { center: [0.0;3], radius: 0.5 });
        b3_world_step_gpu(world, 1.0/60.0, 4);
        assert_eq!(with_world_no_sync(world, |w| w.gpu_mirror_stale), Some(true));
        for _ in 0..1000 {
            assert!(b3_body_is_contact_recycling_enabled(body));
            b3_body_enable_contact_recycling(body, true);
        }
        assert_eq!(with_world_no_sync(world, |w| w.gpu_mirror_stale), Some(true), "metadata API downloaded world");
        b3_body_enable_contact_recycling(body, false);
        let y = b3_body_get_position(body)[1];
        assert!(y < 5.0, "flag upload restored pre-step position");
        b3_world_step_gpu(world, 1.0/60.0, 4);
        b3_world_gpu_wait_with_mirror(world);
        assert!(b3_body_get_position(body)[1] < y);
        assert!(!b3_body_is_contact_recycling_enabled(body));
        b3_destroy_world(world);
        assert!(!b3_body_is_contact_recycling_enabled(body));
        b3_body_enable_contact_recycling(body, true);
    }
}


pub fn b3_world_set_contact_recycle_distance(id: WorldId, distance: f32) {
    with_world_mut_no_sync(id, |w| {
        w.contact_recycle_distance = distance.max(0.0).min(f32::MAX);
    });
}
pub fn b3_world_get_contact_recycle_distance(id: WorldId) -> f32 {
    with_world_no_sync(id, |w| w.contact_recycle_distance).unwrap_or(0.0)
}

#[cfg(test)]
mod mesh_shared_snapshot_tests {
    use super::*;

    #[test]
    fn mesh_snapshots_share_storage_and_survive_replacement_and_destruction() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        let world = b3_create_world(gpu, &super::super::b3_default_world_def());
        let body = b3_create_body(world, &super::super::b3_default_body_def());
        let points = [[-2.0,0.0,-2.0],[2.0,0.0,-2.0],[0.0,0.0,2.0]];
        let triangles = [[0,2,1]];
        let shape = b3_create_mesh_shape(body, &super::super::b3_default_shape_def(),
            &points, &triangles, &[], &[], &[], [1.0;3]);
        let snapshot = || with_world_no_sync(world, |w| {
            let index = shape.index1 as usize - 1;
            host_shape(w, shape.world0, index, w.shapes[index].as_ref().unwrap()).unwrap()
        }).unwrap();
        let old = snapshot();
        let same = snapshot();
        assert!(Arc::ptr_eq(&old.local_points, &same.local_points));
        assert!(Arc::ptr_eq(&old.mesh_triangles, &same.mesh_triangles));
        assert!(Arc::ptr_eq(&old.mesh_triangle_ids, &same.mesh_triangle_ids));
        assert!(Arc::ptr_eq(&old.mesh_nodes, &same.mesh_nodes));
        let raised = points.map(|p| [p[0], p[1]+2.0, p[2]]);
        assert!(b3_replace_mesh_shape(shape, &raised, &triangles, &[], &[], &[], [1.0;3]));
        let new = snapshot();
        assert!(!Arc::ptr_eq(&old.local_points, &new.local_points));
        assert_eq!(old.local_points.as_ref(), &points);
        assert_eq!(new.local_points.as_ref(), &raised);
        b3_destroy_world(world);
        assert_eq!(old.local_points.as_ref(), &points);
        assert_eq!(new.local_points.as_ref(), &raised);
        assert_eq!(old.mesh_triangle_ids.as_ref(), &[0]);
    }
}


pub fn b3_shape_set_density(id: ShapeId, density: f32, update_body_mass: bool) {
    if !density.is_finite() || density < 0.0 { return; }
    let world = WorldId { index1: id.world0, generation: 1 };
    with_world_mut(world, |w| {
        let Some(index) = id.index1.checked_sub(1).map(|v| v as usize) else { return; };
        let Some(shape) = w.shapes.get_mut(index).and_then(Option::as_mut) else { return; };
        if shape.generation != id.generation || shape.density == density { return; }
        shape.density = density;
        shape.mass = shape.unit_mass * density;
        shape.local_inertia = shape.unit_inertia.map(|v| v * density);
        let body_index = shape.body_index as usize - 1;
        let Some(body) = w.bodies.get(body_index).and_then(Option::as_ref) else { return; };
        let body = BodyId { index1: body_index as i32 + 1, world0: id.world0, generation: body.generation };
        w.query_topology = w.query_topology.saturating_add(1);
        if update_body_mass { apply_body_mass_from_shapes_inner(w, body); }
    });
}


pub fn b3_body_get_world(id: BodyId) -> WorldId {
    let world = world_id_from_body(id);
    with_world_no_sync(world, |w| body_ref(w, id).map(|_| world)).flatten().unwrap_or_default()
}

pub fn b3_shape_get_world(id: ShapeId) -> WorldId {
    let world = WorldId { index1: id.world0, generation: 1 };
    with_world_no_sync(world, |w| {
        let shape = w.shapes.get(id.index1.checked_sub(1)? as usize)?.as_ref()?;
        (shape.generation == id.generation).then_some(world)
    }).flatten().unwrap_or_default()
}

fn with_joint_metadata<T>(id: JointId, f: impl FnOnce(&WorldInner, &JointGpu) -> T) -> Option<T> {
    if id.generation != 1 { return None; }
    with_world_no_sync(WorldId { index1: id.world0, generation: 1 }, |w| {
        let joint = w.joints.get(id.index1.checked_sub(1)? as usize)?;
        (joint.kind != JOINT_NONE).then(|| f(w, joint))
    }).flatten()
}

pub fn b3_joint_get_kind(id: JointId) -> u32 {
    with_joint_metadata(id, |_, joint| joint.kind).unwrap_or(JOINT_NONE)
}

pub fn b3_joint_get_body(id: JointId, second: bool) -> BodyId {
    with_joint_metadata(id, |w, joint| {
        let index = if second { joint.b } else { joint.a };
        w.bodies.get(index as usize).and_then(Option::as_ref).map(|body| BodyId {
            index1: index as i32 + 1, world0: id.world0, generation: body.generation,
        }).unwrap_or_default()
    }).unwrap_or_default()
}

pub fn b3_joint_get_world(id: JointId) -> WorldId {
    with_joint_metadata(id, |_, _| WorldId { index1: id.world0, generation: 1 }).unwrap_or_default()
}

#[cfg(test)]
mod joint_metadata_tests {
    use super::*;
    #[test]
    fn identity_getters_do_not_harvest_a_pending_step() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        let world = b3_create_world(gpu, &super::super::b3_default_world_def());
        let ground = b3_create_body(world, &super::super::b3_default_body_def());
        let mut bd = super::super::b3_default_body_def();bd.body_type = BodyType::Dynamic;
        let body = b3_create_body(world, &bd);
        let shape = b3_create_hull_shape(body, &super::super::b3_default_shape_def(), &super::super::b3_make_box_hull(0.5,0.5,0.5));
        let mut jd = super::super::b3_default_revolute_joint_def();jd.body_a = ground;jd.body_b = body;
        let joint = b3_create_revolute_joint(world, &jd);
        b3_world_step_gpu(world, 1.0/60.0, 4);
        assert_eq!(with_world_no_sync(world, |w| w.gpu_mirror_stale), Some(true));
        assert_eq!(b3_world_get_maximum_linear_speed(world), 400.0);
        b3_world_set_maximum_linear_speed(world, 17.0);
        assert_eq!(b3_world_get_restitution_threshold(world), 1.0);
        b3_world_set_restitution_threshold(world, 2.5);
        for invalid in [0.0, -1.0, f32::NAN, f32::INFINITY] {
            b3_world_set_maximum_linear_speed(world, invalid);
        }
        for _ in 0..1000 {
            assert_eq!(b3_world_get_maximum_linear_speed(world), 17.0);
            assert_eq!(b3_world_get_restitution_threshold(world), 2.5);
            assert_eq!(b3_body_get_world(body), world);
            assert_eq!(b3_shape_get_world(shape), world);
            assert_eq!(b3_joint_get_world(joint), world);
            assert_eq!(b3_joint_get_kind(joint), JOINT_REVOLUTE);
            assert!(!b3_joint_get_collide_connected(joint));
            b3_joint_set_collide_connected(joint, false);
            assert_eq!(b3_joint_get_body(joint, false), ground);
            assert_eq!(b3_joint_get_body(joint, true), body);
            assert_eq!(b3_shape_body(shape), body);
            assert!(b3_body_is_valid(body));
            assert!(b3_shape_is_valid(shape));
            assert!(b3_joint_is_valid(joint));
        }
        assert_eq!(with_world_no_sync(world, |w| w.gpu_mirror_stale), Some(true));
        b3_destroy_body(body);
        let replacement = b3_create_body(world, &bd);
        assert_eq!(replacement.index1, body.index1);
        assert_ne!(replacement.generation, body.generation);
        b3_destroy_body(body); // stale body ID must not destroy the replacement
        assert!(b3_body_is_valid(replacement));
        assert_eq!(b3_joint_get_body(joint, true), BodyId::default());
        b3_destroy_world(world);
    }
}


fn live_body_id(w: &WorldInner, world0: u16, index1: i32) -> BodyId {
    index1.checked_sub(1).and_then(|i| w.bodies.get(i as usize))
        .and_then(Option::as_ref).map(|body| BodyId { index1, world0, generation: body.generation })
        .unwrap_or_default()
}

/// Configuration only: no completed-state readback is needed. Applied on the next submit.
pub fn b3_world_set_maximum_linear_speed(id: WorldId, speed: f32) {
    if !speed.is_finite() || speed <= 0.0 { return; }
    with_world_mut_no_sync(id, |w| w.def.maximum_linear_speed = speed);
}

pub fn b3_world_get_maximum_linear_speed(id: WorldId) -> f32 {
    with_world_no_sync(id, |w| w.def.maximum_linear_speed).unwrap_or(0.0)
}

pub fn b3_joint_get_collide_connected(id: JointId) -> bool {
    with_joint_metadata(id, |_, j| j.flags & JOINT_COLLIDE_CONNECTED != 0).unwrap_or(false)
}

pub fn b3_joint_set_collide_connected(id: JointId, enable: bool) {
    if id.index1 <= 0 || id.generation != 1 { return; }
    let world = WorldId { index1: id.world0, generation: 1 };
    // Harvest pending contact events before retiring their pair, so an unread
    // begin is preserved and the end is published on the next native step.
    with_world_mut_no_sync(world, |w| {
        let Some(joint) = w.joints.get(id.index1 as usize - 1) else { return; };
        if joint.kind == JOINT_NONE || (joint.flags & JOINT_COLLIDE_CONNECTED != 0) == enable { return; }
        #[cfg(not(target_arch = "wasm32"))]
        let _ = sync_world_mirror(w, world);
        let joint = &mut w.joints[id.index1 as usize - 1];
        let (a, b) = (joint.a, joint.b);
        if enable { joint.flags |= JOINT_COLLIDE_CONNECTED; }
        else { joint.flags &= !JOINT_COLLIDE_CONNECTED; }
        if !enable {
            if let Some(sim) = w.sim.as_mut() { sim.retire_body_pair_contacts(a, b); }
            let shapes = &w.shapes;
            w.live_contacts.retain(|_, contact| {
                let owner = |id: ShapeId| shapes.get(id.index1.saturating_sub(1) as usize)
                    .and_then(Option::as_ref).map(|s| s.body_index - 1);
                let pair = (owner(contact.shape_id_a), owner(contact.shape_id_b));
                let remove = pair == (Some(a as i32), Some(b as i32)) || pair == (Some(b as i32), Some(a as i32));
                if remove && contact.events_enabled {
                    w.deferred_contact_end_events.push(ContactEndTouchEvent {
                        shape_id_a: contact.shape_id_a, shape_id_b: contact.shape_id_b,
                        contact_id: contact.contact_id,
                    });
                }
                !remove
            });
            contact_api::retire_body_pair(w, a as i32 + 1, b as i32 + 1);
        }
        // Rebuild the pair veto table and broadphase inputs, without waking bodies.
        mark_scene_dirty(w);
    });
}

pub fn b3_world_set_restitution_threshold(id: WorldId, threshold: f32) {
    if threshold.is_nan() { return; }
    with_world_mut_no_sync(id, |w| w.def.restitution_threshold = threshold.clamp(0.0, f32::MAX));
}
pub fn b3_world_get_restitution_threshold(id: WorldId) -> f32 {
    with_world_no_sync(id, |w| w.def.restitution_threshold).unwrap_or(0.0)
}

#[cfg(test)]
mod mesh_instance_tests {
    use super::*;
    use crate::api::*;

    #[test]
    fn raw_mesh_instances_share_gpu_ranges_and_preserve_affine_support() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        let rotation = glam::Quat::from_rotation_x(0.3);
        let normal = rotation * glam::Vec3::Y;
        let mut wd = b3_default_world_def();
        wd.gravity = (-10.0 * normal).to_array();
        wd.enable_sleep = false;
        let world = b3_create_world(gpu, &wd);
        let mut bd = b3_default_body_def();
        bd.position = [-100.0, 0.0, 0.0];
        let source_body = b3_create_body(world, &bd);
        let points = [[-2.0, 0.0, -2.0], [2.0, 0.0, -2.0], [0.0, 0.0, 2.0]];
        let triangles = [[0, 2, 1]];
        let source = b3_create_mesh_shape(
            source_body,
            &b3_default_shape_def(),
            &points,
            &triangles,
            &[0x77],
            &[0],
            &[],
            [1.0; 3],
        );
        let body = b3_create_body(world, &b3_default_body_def());
        for (q, scale) in [([f32::MAX; 4], [1.0; 3]),
                           ([0.0, 0.0, 0.0, 1.0], [f32::MAX; 3])] {
            let rejected = b3_create_mesh_instance(body, &b3_default_shape_def(), source,
                WorldTransform { p: [0.0; 3], q }, scale);
            assert_eq!(rejected.index1, 0, "overflowing affine transform must be refused");
        }
        let parent = b3_create_compound_parent(body, &b3_default_shape_def());
        let mut instances = Vec::new();
        for bits in 0..8 {
            let scale: [f32; 3] = [
                if bits & 1 == 0 { 1.5 } else { -1.5 },
                if bits & 2 == 0 { 0.7 } else { -0.7 },
                if bits & 4 == 0 { 0.8 } else { -0.8 },
            ];
            let center = glam::Vec3::new(bits as f32 * 8.0, 2.0, 0.0);
            let outward = normal * scale[1].signum();
            let mut sd = b3_default_shape_def();
            sd.user_material_id = 100 + bits;
            let child = b3_create_mesh_instance(
                body,
                &sd,
                source,
                WorldTransform {
                    p: center.to_array(),
                    q: rotation.to_array(),
                },
                scale,
            );
            assert_ne!(child.index1, 0);
            assert!(b3_shape_attach_compound_child(parent, child));
            let origin = (center + outward * 2.0).to_array();
            let translation = (-4.0 * outward).to_array();
            let host = b3_shape_ray_cast(child, origin, translation);
            assert!(host.hit, "host instance {bits}");
            assert!((host.fraction - 0.5).abs() < 1e-5);
            let hit = unsafe {
                b3_world_cast_ray_closest(world, origin, translation, b3_default_query_filter())
            };
            assert!(hit.hit, "GPU instance {bits}");
            assert_eq!(hit.shape_id, parent);
            assert_eq!(hit.child_index, bits as i32);
            assert_eq!(hit.user_material_id, 100 + bits);
            assert!((hit.fraction - 0.5).abs() < 1e-5);
            assert!((glam::Vec3::from_array(hit.normal) - outward).length() < 1e-5);
            let reverse = b3_shape_ray_cast(
                child,
                (center - outward * 2.0).to_array(),
                (outward * 4.0).to_array(),
            );
            assert!(!reverse.hit);
            instances.push((child, center, outward));
        }
        let counts = || {
            with_world_no_sync(world, |w| {
                let p = w.sim.as_ref().unwrap().params();
                (
                    p.mesh_vertex_count,
                    p.mesh_triangle_count,
                    p.mesh_node_count,
                )
            })
            .unwrap()
        };
        assert_eq!(
            counts(),
            (3, 1, 1),
            "eight instances must not duplicate packed geometry"
        );
        let retained = query_shape(source).unwrap();
        with_world_no_sync(world, |w| {
            let raw = w.shapes[source.index1 as usize - 1].as_ref().unwrap();
            for (id, _, _) in &instances {
                let clone = w.shapes[id.index1 as usize - 1].as_ref().unwrap();
                assert!(Arc::ptr_eq(&raw.mesh_vertices, &clone.mesh_vertices));
                assert!(Arc::ptr_eq(&raw.mesh_triangles, &clone.mesh_triangles));
                assert!(Arc::ptr_eq(&raw.mesh_nodes, &clone.mesh_nodes));
            }
        });
        let raised = points.map(|p| [p[0], p[1] + 3.0, p[2]]);
        assert!(b3_replace_mesh_shape(
            source,
            &raised,
            &triangles,
            &[0x77],
            &[0],
            &[],
            [1.0; 3]
        ));
        b3_world_ensure_gpu(world);
        assert_eq!(
            counts(),
            (6, 2, 2),
            "source replacement must leave clones on old geometry"
        );
        b3_destroy_shape(source, false);
        b3_world_ensure_gpu(world);
        assert_eq!(counts(), (3, 1, 1));
        assert_eq!(retained.local_points.as_ref(), &points);
        let mut balls = Vec::new();
        for (_, center, outward) in &instances {
            let mut bd = b3_default_body_def();
            bd.body_type = BodyType::Dynamic;
            bd.position = (*center + *outward * 0.45).to_array();
            bd.gravity_scale = if outward.dot(normal) > 0.0 { 1.0 } else { -1.0 };
            bd.enable_sleep = false;
            let ball = b3_create_body(world, &bd);
            b3_create_sphere_shape(
                ball,
                &b3_default_shape_def(),
                &Sphere {
                    center: [0.0; 3],
                    radius: 0.25,
                },
            );
            balls.push((ball, *center, *outward));
        }
        for _ in 0..120 {
            b3_world_step(world, 1.0 / 60.0, 4);
        }
        assert!(!b3_world_physics_invalid(world));
        for (ball, center, outward) in balls {
            let position = glam::Vec3::from_array(b3_body_get_position(ball));
            let support = (position - center).dot(outward);
            assert!((0.24..0.27).contains(&support), "lost support: {support}");
        }
        b3_destroy_world(world);
        assert_eq!(retained.local_points.as_ref(), &points);
    }
}

#[cfg(test)]
mod world_counter_tests {
    use super::*;
    use crate::api::*;

    #[test]
    fn topology_counts_do_not_download_or_harvest_submitted_physics() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        let world = b3_create_world(gpu, &b3_default_world_def());
        let mut bd = b3_default_body_def(); bd.body_type = BodyType::Dynamic;
        bd.position = [0.0,5.0,0.0];
        let body = b3_create_body(world,&bd);
        b3_create_sphere_shape(body,&b3_default_shape_def(),&Sphere {center:[0.0;3],radius:0.5});
        b3_world_step_gpu(world,1.0/60.0,4);
        let state = || with_world_no_sync(world, |w| (w.gpu_mirror_stale,w.events_pending,w.post_ccd_pending,
            w.last_query_profile.copied_bytes,w.sim.as_ref().unwrap().completed_physics_step()));
        let before = state();
        assert!(before.unwrap().0);
        for _ in 0..1000 { assert_eq!(b3_world_counts(world),(1,1,0)); }
        assert_eq!(state(),before,"topology counters must not download poses, complete physics or harvest events/CCD");
        b3_world_gpu_wait_with_mirror(world);
        assert!(b3_body_get_position(body)[1]<5.0,"physics still completes through its explicit path");
        b3_destroy_world(world);
        assert_eq!(b3_world_counts(world),(0,0,0));
    }

    #[test]
    fn topology_counts_track_public_shapes_and_live_joint_holes() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        let world = b3_create_world(gpu,&b3_default_world_def());
        let ground = b3_create_body(world,&b3_default_body_def());
        let sd = b3_default_shape_def();
        let parent = b3_create_compound_parent(ground,&sd);
        for x in [0.0,4.0] {
            let child = b3_create_sphere_shape(ground,&sd,&Sphere {center:[x,0.0,0.0],radius:0.5});
            assert!(attach_compound_child(parent,child,None,[0;4]));
        }
        let mut bd = b3_default_body_def();bd.body_type=BodyType::Dynamic;
        let body = b3_create_body(world,&bd);
        let shape = b3_create_sphere_shape(body,&sd,&Sphere {center:[0.0;3],radius:0.5});
        let mut jd = b3_default_revolute_joint_def();jd.body_a=ground;jd.body_b=body;
        let first = b3_create_revolute_joint(world,&jd);
        let second = b3_create_revolute_joint(world,&jd);
        assert_eq!(b3_world_counts(world),(2,2,2));
        b3_destroy_joint(first,false);
        assert_eq!(b3_world_counts(world),(2,2,1));
        b3_destroy_joint(second,false);
        b3_destroy_shape(shape,true);
        assert_eq!(b3_world_counts(world),(2,1,0));
        b3_destroy_shape(parent,false);
        assert_eq!(b3_world_counts(world),(2,0,0));
        b3_destroy_world(world);
    }
}

#[cfg(test)]
mod contact_metrics_tests {
    use super::*;
    use crate::api::*;
    unsafe extern "C" fn allow(_:ShapeId,_:ShapeId,_:*mut std::ffi::c_void)->bool { true }

    #[test]
    fn ccd_correction_invalidates_the_scheduling_snapshot_state() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        for gpu_ccd in [false,true] {
        let world=b3_create_world(gpu.clone(),&b3_default_world_def());
        // Deferred host CCD changes the revision during harvest. GPU CCD is
        // already part of the submitted revision and must not change it twice.
        with_world_mut_no_sync(world,|w|w.gpu_ccd_requested=gpu_ccd);
        crate::scenes::create_ground(world,10.0);
        let mut bd=b3_default_body_def();bd.body_type=BodyType::Dynamic;
        bd.position=[0.0,2.0,0.0];bd.linear_velocity=[0.0,-240.0,0.0];
        let body=b3_create_body(world,&bd);
        b3_create_sphere_shape(body,&b3_default_shape_def(),&Sphere {center:[0.0;3],radius:0.5});
        b3_world_step_gpu(world,1.0/60.0,4);
        with_world_no_sync(world,|w|assert_eq!(w.sim.as_ref().unwrap().uses_convex_ccd(),gpu_ccd));
        let scheduled=b3_world_contact_metrics(world,true);
        assert_eq!(scheduled.current,u32::from(!gpu_ccd));
        assert_eq!(scheduled.current_state,scheduled.snapshot_state+u64::from(gpu_ccd));
        b3_world_gpu_wait_with_mirror(world);
        let corrected=b3_world_contact_metrics(world,false);
        assert_eq!(corrected.snapshot_step,scheduled.snapshot_step);
        if gpu_ccd {
            assert_eq!(corrected.current_state,scheduled.current_state,"GPU CCD must not invalidate twice: {corrected:?}");
            assert_eq!(corrected.current,0);
        } else {
            assert!(corrected.current_state>corrected.snapshot_state,"{corrected:?}");
            assert_eq!(corrected.current,0);
        }
        assert!(b3_body_get_position(body)[1]>0.49);
        b3_destroy_world(world);
        }
    }

    #[test]
    fn world_contact_metrics_track_mutations_callbacks_and_world_reuse() {
        assert_eq!(std::mem::size_of::<WorldContactMetrics>(),80);
        assert_eq!(std::mem::offset_of!(WorldContactMetrics,known),48);
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        for callbacks in [false,true] {
            let world=b3_create_world(gpu.clone(),&b3_default_world_def());
            assert_eq!(b3_world_contact_metrics(world,false).known,0);
            b3_world_enable_continuous(world,false);
            crate::scenes::create_ground(world,10.0);
            let mut bd=b3_default_body_def();bd.body_type=BodyType::Dynamic;bd.position=[0.0,0.5,0.0];
            let body=b3_create_body(world,&bd);
            let mut sd=b3_default_shape_def();sd.enable_custom_filtering=callbacks;
            b3_create_sphere_shape(body,&sd,&Sphere {center:[0.0;3],radius:0.5});
            if callbacks { b3_world_set_custom_filter_callback(world,Some(allow),std::ptr::null_mut()); }
            b3_world_step_gpu(world,1.0/60.0,4);
            let pending=with_world_no_sync(world,|w|(w.gpu_mirror_stale,w.events_pending,w.post_ccd_pending));
            let first=b3_world_contact_metrics(world,true);
            assert_eq!((first.known,first.current,first.snapshot_step),(1,1,1),"{first:?}");
            assert!(first.allocated_roots>=1 && first.touching_roots>=1,"{first:?}");
            assert_eq!(with_world_no_sync(world,|w|(w.gpu_mirror_stale,w.events_pending,w.post_ccd_pending)),pending,
                       "metrics must not harvest CCD/events/body state");
            b3_body_set_transform(body,[0.0,5.0,0.0],[0.0,0.0,0.0,1.0]);
            let teleported=b3_world_contact_metrics(world,false);
            assert_eq!((teleported.known,teleported.current),(1,0));
            assert!(teleported.current_state>teleported.snapshot_state);
            let extra=b3_create_sphere_shape(body,&sd,&Sphere {center:[2.0,0.0,0.0],radius:0.5});
            let changed=b3_world_contact_metrics(world,false);
            assert_eq!(changed.current,0);
            assert!(changed.current_topology>changed.snapshot_topology);
            b3_world_step_gpu(world,1.0/60.0,4);
            let newer=b3_world_contact_metrics(world,true);
            assert_eq!((newer.known,newer.current,newer.snapshot_step),(1,1,2),"{newer:?}");
            assert_eq!(newer.touching_roots,0);
            b3_destroy_shape(extra,true);
            assert_eq!(b3_world_contact_metrics(world,false).current,0);
            b3_destroy_world(world);
            assert_eq!(b3_world_contact_metrics(world,false).known,0);
            let replacement=b3_create_world(gpu.clone(),&b3_default_world_def());
            assert_eq!(b3_world_contact_metrics(world,false).known,0);
            assert_eq!(b3_world_contact_metrics(replacement,false).known,0);
            b3_destroy_world(replacement);
        }
    }
}

#[cfg(test)]
mod appended_extent_tests {
    use super::*;
    use crate::api::*;

    #[test]
    fn body_shape_index_preserves_order_and_drops_deleted_owners() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        let world = b3_create_world(gpu, &b3_default_world_def());
        let mut bodies = Vec::new();
        let sd = b3_default_shape_def();
        let sphere = Sphere { center: [1.0, 2.0, 3.0], radius: 0.5 };
        for _ in 0..32 {
            let body = b3_create_body(world, &BodyDef { body_type: BodyType::Dynamic, ..b3_default_body_def() });
            b3_create_sphere_shape(body, &sd, &sphere);
            bodies.push(body);
        }
        let check = |body: BodyId| with_world_mut_no_sync(world, |w| {
            let expected: Vec<usize> = w.shapes.iter().enumerate()
                .filter_map(|(i,s)| s.as_ref().filter(|s| s.body_index == body.index1).map(|_| i)).collect();
            assert_eq!(body_ref(w, body).unwrap().shape_indices, expected);
            let before = (body_ref(w, body).unwrap().min_extent, body_ref(w, body).unwrap().max_extent);
            update_body_extents_range(w, body, 0..w.shapes.len(), true);
            let after = body_ref(w, body).unwrap();
            assert_eq!(before, (after.min_extent, after.max_extent));
        });
        for &body in &bodies {
            let child = b3_create_sphere_shape(body, &sd, &Sphere { center: [-4.0, 0.0, 0.0], ..sphere });
            check(body);
            b3_destroy_shape(child, true);
            check(body);
        }
        let old = bodies[0];
        b3_destroy_body(old);
        let replacement = b3_create_body(world, &b3_default_body_def());
        assert_eq!(old.index1, replacement.index1);
        assert_ne!(old.generation, replacement.generation);
        b3_create_sphere_shape(replacement, &sd, &sphere);
        check(replacement);
        b3_destroy_world(world);
    }

    #[test]
    fn appended_extents_match_full_scan_after_com_changes_and_deletions() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        let world = b3_create_world(gpu, &b3_default_world_def());
        for body_type in [BodyType::Static, BodyType::Dynamic] {
            let mut bd = b3_default_body_def(); bd.body_type = body_type;
            let body = b3_create_body(world, &bd);
            let check = || with_world_mut_no_sync(world, |w| {
                let b = body_ref(w, body).unwrap();
                let actual = (b.min_extent, b.max_extent);
                update_body_extents_range(w, body, 0..w.shapes.len(), true);
                let b = body_ref(w, body).unwrap();
                assert_eq!(actual, (b.min_extent, b.max_extent));
            });
            let mut shapes = Vec::new();
            for i in 0..128 {
                let mut sd = b3_default_shape_def();
                sd.update_body_mass = i % 7 == 3;
                let center = [i as f32 * 0.3, (i % 5) as f32, -0.2];
                let shape = if i % 2 == 0 {
                    b3_create_sphere_shape(body, &sd, &Sphere { center, radius: 0.1 + (i % 3) as f32 })
                } else {
                    b3_create_capsule_shape(body, &sd, &Capsule {
                        center1: center, center2: [center[0], center[1] + 1.0, center[2]], radius: 0.3,
                    })
                };
                shapes.push(shape);
                check();
                if i % 19 == 4 {
                    let mut data = b3_body_get_mass_data(body);
                    data.center = [i as f32 * 0.1, -2.0, 3.0];
                    b3_body_set_mass_data(body, data);
                    check();
                }
            }
            for shape in shapes.into_iter().rev() {
                b3_destroy_shape(shape, false);
                check();
            }
            b3_create_sphere_shape(body, &ShapeDef { update_body_mass: false, ..b3_default_shape_def() },
                &Sphere { center: [20.0, 0.0, 0.0], radius: 0.2 });
            check();
        }
        let body = b3_create_body(world, &b3_default_body_def());
        let sd = ShapeDef { update_body_mass: false, ..b3_default_shape_def() };
        let parent = b3_create_compound_parent(body, &sd);
        let child = b3_create_sphere_shape(body, &sd, &Sphere { center: [2.0,0.0,0.0], radius: 0.5 });
        assert!(b3_shape_attach_compound_child(parent, child));
        let check = || with_world_mut_no_sync(world, |w| {
            let b = body_ref(w, body).unwrap();
            let actual = (b.min_extent, b.max_extent);
            update_body_extents_range(w, body, 0..w.shapes.len(), true);
            let b = body_ref(w, body).unwrap();
            assert_eq!(actual, (b.min_extent, b.max_extent));
        });
        check();
        let vertices = [[30.0,0.0,0.0],[31.0,0.0,0.0],[30.0,1.0,0.0]];
        let mesh = b3_create_mesh_shape(body, &sd, &vertices, &[[0,1,2]], &[], &[], &[], [1.0;3]);
        assert!(mesh.index1 > 0);
        check();
        let instance = b3_create_mesh_instance(body, &sd, mesh,
            WorldTransform { p:[4.0,0.0,0.0], q:[0.0,0.0,0.0,1.0] }, [2.0;3]);
        assert!(instance.index1 > 0);
        check();
        b3_destroy_shape(instance, false);
        let small = [[0.0,0.0,0.0],[1.0,0.0,0.0],[0.0,1.0,0.0]];
        assert!(b3_replace_mesh_shape(mesh, &small, &[[0,1,2]], &[], &[], &[], [1.0;3]));
        check();
        b3_destroy_world(world);
    }
}

#[cfg(test)]
mod color_tail_tests {
    use super::*;
    use crate::api::{b3_default_world_def, b3_default_body_def, b3_default_shape_def,
        b3_default_revolute_joint_def, b3_make_box_hull};

    #[test]
    fn color_tail_preserves_full_schedule_for_wide_and_small_worlds() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        // 8: host-proven shortcut; 100: GPU-only shortcut (tail must skip);
        // 600: wide colors, including multiple batches when prefix is zero.
        for (count, jointed) in [(8, false), (100, false), (600, false), (600, true)] {
            let run = |prefix| {
                let world = b3_create_world(gpu.clone(), &b3_default_world_def());
                // Exercise the global color-wave encoder even when the launcher
                // requests component TGS. Dispatch-count checks below require it.
                with_world_mut_no_sync(world, |w| w.component_tgs_requested = false);
                b3_world_enable_sleeping(world, false);
                crate::scenes::create_mixed_stacks(world, count);
                if jointed {
                    let mut bd = b3_default_body_def();
                    bd.position = [100.0, 10.0, 0.0];
                    let anchor = b3_create_body(world, &bd);
                    bd.body_type = BodyType::Dynamic;
                    bd.position = [100.0, 8.0, 0.0];
                    let body = b3_create_body(world, &bd);
                    b3_create_sphere_shape(body, &b3_default_shape_def(), &Sphere { center: [0.0;3], radius: 0.5 });
                    let mut jd = b3_default_revolute_joint_def();
                    jd.body_a = anchor;
                    jd.body_b = body;
                    jd.local_anchor_b = [0.0, 2.0, 0.0];
                    b3_create_revolute_joint(world, &jd);
                    b3_body_set_linear_velocity(body, [1.0, 0.0, 0.0]);
                    // More than three static contacts on one writable body
                    // force overflow while the main stacks keep wide colors.
                    bd = b3_default_body_def();
                    bd.position = [100.0, 0.0, 0.0];
                    let floor = b3_create_body(world, &bd);
                    b3_create_hull_shape(floor, &b3_default_shape_def(), &b3_make_box_hull(2.0, 0.5, 2.0));
                    bd.body_type = BodyType::Dynamic;
                    bd.position = [100.0, 1.0, 0.0];
                    let compound = b3_create_body(world, &bd);
                    for _ in 0..26 {
                        b3_create_sphere_shape(compound, &b3_default_shape_def(), &Sphere { center: [0.0;3], radius: 0.5 });
                    }
                }
                b3_world_ensure_gpu(world);
                with_world_mut_no_sync(world, |w| w.sim.as_mut().unwrap().set_color_wave_prefix(prefix));
                for step in 0..120 {
                    if jointed && step == 60 {
                        let mut bd = b3_default_body_def();
                        bd.position = [200.0, 0.0, 0.0];
                        let extra = b3_create_body(world, &bd);
                        b3_create_sphere_shape(extra, &b3_default_shape_def(), &Sphere { center: [0.0;3], radius: 0.5 });
                        b3_world_ensure_gpu(world);
                        if prefix == u32::MAX {
                            with_world_mut_no_sync(world, |w| assert_eq!(w.sim.as_ref().unwrap().color_wave_prefix(), 23,
                                "topology change must reset the performance hint"));
                        } else {
                            // Growth can replace GpuSim. Reapply the test-only
                            // override just as the production env is reread.
                            with_world_mut_no_sync(world, |w| w.sim.as_mut().unwrap().set_color_wave_prefix(prefix));
                        }
                    }
                    b3_world_step_gpu(world, 1.0 / 60.0, 4);
                }
                b3_world_gpu_wait(world);
                let bodies = pollster::block_on(b3_world_sync_from_gpu(world));
                let stats = pollster::block_on(b3_world_live_step_stats(world)).unwrap();
                assert!(!stats.capacity_loss());
                let commands = b3_world_last_solver_dispatches(world);
                if jointed {
                    let contacts = pollster::block_on(b3_world_sync_contacts(world));
                    assert!(contacts.iter().any(|c| c.a != u32::MAX && c.count > 0 && c.color == crate::types::OVERFLOW_COLOR),
                        "fixture must exercise overflow-first ordering");
                }
                b3_destroy_world(world);
                (bodies, commands)
            };
            let (reference, _) = run(23);
            for prefix in [0, 8, u32::MAX] {
                let (actual, commands) = run(prefix);
                assert_eq!(reference.len(), actual.len());
                for (a, b) in reference.iter().zip(&actual) {
                    for (x, y) in a.pos.iter().chain(&a.rot).chain(&a.vel).chain(&a.omega)
                        .zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)) {
                        assert!(x.is_finite() && y.is_finite() && (x-y).abs() < 1e-5,
                            "count={count} prefix={prefix}: {x} != {y}");
                    }
                }
                if count == 600 {
                    if prefix == u32::MAX {
                        assert!(commands < 325, "GPU color hint must select a shorter schedule");
                    } else {
                        // Jointed worlds split the twelve warm/solve/relax
                        // waves around anchored joints; restitution stays whole.
                        let partition_dispatches = if jointed { 4 * 3 } else { 0 };
                        assert_eq!(commands, 13 * (prefix + 6) + partition_dispatches,
                            "wide world must exercise batched tail and anchored-joint boundary");
                    }
                    if !jointed { assert!(crate::dump::mixed_stacks_quality_error(&actual, 120).is_none()); }
                }
            }
        }
    }
}

#[cfg(all(test, not(target_arch = "wasm32")))]
mod render_scene_tests {
    use super::*;
    use crate::api::*;
    use crate::render_scene::RenderGeometry;

    #[test]
    fn render_scene_preserves_slots_children_and_does_not_harvest_physics() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        for resident in [false,true] {
        let world = b3_create_world(gpu.clone(), &b3_default_world_def());
        with_world_mut_no_sync(world,|w| {w.gpu_ccd_requested=resident;w.gpu_resident_requested=resident;});
        let mut def = b3_default_body_def();
        def.body_type = BodyType::Dynamic;
        let first = b3_create_body(world, &b3_default_body_def());
        let hole = b3_create_body(world, &def);
        let last = b3_create_body(world, &def);
        let sd = b3_default_shape_def();
        let parent = b3_create_compound_parent(first, &sd);
        for x in [-2.0, 3.0] {
            let child = b3_create_sphere_shape(first, &sd, &Sphere { center: [x, 0.0, 0.0], radius: 0.5 });
            assert!(b3_shape_attach_compound_child(parent, child));
        }
        b3_create_hull_shape(last, &sd, &b3_make_cube_hull(0.5));
        b3_destroy_body(hole);
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
        let state = || with_world_no_sync(world, |w| (w.gpu_mirror_stale, w.post_ccd_pending,
            w.sim.as_ref().unwrap().pose_snapshot_maps(), w.sim.as_ref().unwrap().completed_physics_step()));
        let before = state();
        let scene = b3_world_render_scene(world, None).unwrap();
        assert_eq!(b3_world_render_buffers(world).unwrap().2, 3, "GPU slot span includes the deleted slot");
        assert_eq!(scene.shapes.len(), 3);
        assert_eq!(scene.shapes.iter().map(|s| s.body_slot).collect::<Vec<_>>(), vec![0, 0, 2]);
        assert_eq!(scene.shapes[0].public_shape, parent);
        assert_eq!(scene.shapes[1].public_shape, parent);
        assert_ne!(scene.shapes[0].collider, scene.shapes[1].collider);
        match &scene.shapes[0].geometry {
            RenderGeometry::Sphere { center, radius } => { assert_eq!(*center, [-2.0, 0.0, 0.0]); assert_eq!(*radius, 0.5); }
            _ => panic!("sphere geometry lost"),
        }
        for _ in 0..100 { assert!(b3_world_render_scene(world, Some(scene.key)).is_none()); }
        assert_eq!(state(), before, "render metadata must not wait or mirror physics");
        assert_eq!(with_world_no_sync(world,can_submit_resident),Some(resident));
        b3_world_finalize_render_state(world);
        if resident {
            assert_eq!(state(),before,"resident finalization must not download or wait");
        } else {
            assert!(!state().unwrap().0, "explicit compatibility finalization consumes the pending body state");
        }
        b3_destroy_body(last);
        let reused = b3_create_body(world, &def);
        b3_create_sphere_shape(reused, &sd, &Sphere { center: [0.0; 3], radius: 0.25 });
        let updated = b3_world_render_scene(world, Some(scene.key)).unwrap();
        assert_ne!(updated.key, scene.key);
        assert!(updated.shapes.iter().all(|s| s.body != last));
        assert_eq!(scene.shapes.len(), 3, "owned old geometry remains usable until its GPU work completes");
        b3_destroy_world(world);
        assert!(b3_world_render_scene(world, Some(scene.key)).is_none());
        }
    }
}

#[cfg(test)]
mod convex_ccd_integration_tests {
    use super::*;
    use crate::api::*;

    #[test]
    fn accumulated_forces_survive_uploads_and_zero_steps() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        let mut wd=b3_default_world_def(); wd.gravity=[0.0;3];
        let world=b3_create_world(gpu,&wd);
        with_world_mut_no_sync(world,|w| {w.gpu_ccd_requested=true;w.gpu_resident_requested=true;});
        let mut bd=b3_default_body_def(); bd.body_type=BodyType::Dynamic;
        bd.enable_sleep=false;bd.linear_damping=0.0;bd.angular_damping=0.0;
        let body=b3_create_body(world,&bd);
        let shape=b3_create_sphere_shape(body,&b3_default_shape_def(),&Sphere{center:[0.0;3],radius:0.5});
        b3_world_step_gpu(world,1.0/60.0,4);
        for mode in 0..4 {
            b3_body_set_transform(body,[0.0;3],[0.0,0.0,0.0,1.0]);
            b3_body_set_linear_velocity(body,[0.0;3]);
            b3_body_set_angular_velocity(body,[0.0;3]);
            match mode {
                0 => b3_body_apply_force_to_center(body,[3.0,0.0,0.0],true),
                1 => b3_body_apply_force(body,[3.0,0.0,0.0],[0.0,0.5,0.0],true),
                2 => b3_body_apply_torque(body,[0.0,0.0,2.0],true),
                _ => b3_shape_apply_wind(shape,[3.0,0.0,0.0],1.0,0.0,10.0,true),
            }
            let (force,torque,inv_mass)=with_world_no_sync(world,|w| {
                let b=body_ref(w,body).unwrap();(b.force,b.torque,b.gpu.inv_mass)
            }).unwrap();
            assert!(force!=[0.0;3] || torque!=[0.0;3]);
            b3_world_ensure_gpu(world);
            b3_world_step_gpu(world,0.0,4);
            with_world_no_sync(world,|w| {
                assert!(!w.bodies_dirty && !can_submit_resident(w));
                let b=body_ref(w,body).unwrap();assert_eq!(b.force,force);assert_eq!(b.torque,torque);
            });
            b3_world_step_gpu(world,1.0/60.0,4);
            let v=b3_body_get_linear_velocity(body);let omega=b3_body_get_angular_velocity(body);
            for i in 0..3 {
                assert!((v[i]-force[i]*inv_mass/60.0).abs()<1e-5,"mode {mode}: {v:?}");
                assert!((omega[i]-torque[i]*inv_mass*10.0/60.0).abs()<1e-5,"mode {mode}: {omega:?}");
            }
            with_world_no_sync(world,|w|assert!(w.bodies.iter().flatten().all(|b|b.force==[0.0;3] && b.torque==[0.0;3])));
            b3_world_step_gpu(world,1.0/60.0,4);
            assert_eq!(b3_body_get_linear_velocity(body),v,"force applied twice");
        }
        b3_body_apply_force_to_center(body,[1.0,0.0,0.0],true);
        b3_destroy_body(body);
        b3_world_step_gpu(world,1.0/60.0,4);
        with_world_no_sync(world,|w|assert!(w.bodies.iter().flatten().all(|b|b.force==[0.0;3] && b.torque==[0.0;3])));
        let replacement=b3_create_body(world,&bd);
        b3_create_sphere_shape(replacement,&b3_default_shape_def(),&Sphere{center:[0.0;3],radius:0.5});
        b3_world_step_gpu(world,1.0/60.0,4);
        assert_eq!(b3_body_get_linear_velocity(replacement),[0.0;3]);
        b3_destroy_world(world);
    }

    #[test]
    fn scene_capability_cache_tracks_public_mutations() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        let world=b3_create_world(gpu,&b3_default_world_def());
        let mut bd=b3_default_body_def(); bd.body_type=BodyType::Dynamic;
        let body=b3_create_body(world,&bd);
        let shape=b3_create_sphere_shape(body,&b3_default_shape_def(),&Sphere{center:[0.0;3],radius:0.5});
        let check=|expected:SceneCapabilities| {
            with_world_no_sync(world,|w| {
                assert_eq!(scene_capabilities(w),expected);
                assert_eq!(scene_capabilities(w),scan_scene_capabilities(w));
                assert_eq!(world_needs_contact_readback(w),expected.shape_events);
            });
            b3_world_ensure_gpu(world);
            with_world_no_sync(world,|w| {
                assert!(!w.scene_dirty && !w.bodies_dirty);
                assert_eq!(w.scene_capabilities,Some(expected));
                assert_eq!(scene_capabilities(w),scan_scene_capabilities(w));
                assert_eq!(world_needs_contact_readback(w),expected.shape_events);
            });
        };
        let plain=SceneCapabilities{pending_forces:false,shape_events:false,ccd_shapes:true,component_bodies:true,has_bullet:false};
        check(plain);
        for flag in [SHAPE_ENABLE_CONTACT_EVENTS,SHAPE_ENABLE_SENSOR_EVENTS,SHAPE_ENABLE_HIT_EVENTS] {
            set_shape_event_flag(shape,flag,true); check(SceneCapabilities{shape_events:true,..plain});
            set_shape_event_flag(shape,flag,false); check(plain);
        }
        b3_body_set_bullet(body,true); check(SceneCapabilities{has_bullet:true,..plain});
        b3_body_set_bullet(body,false); check(plain);
        b3_body_set_type(body,BodyType::Kinematic); check(SceneCapabilities{component_bodies:false,..plain});
        b3_body_set_type(body,BodyType::Static); check(plain);
        b3_body_set_type(body,BodyType::Dynamic); check(plain);
        b3_body_set_enabled(body,false); check(SceneCapabilities{component_bodies:false,..plain});
        b3_body_set_enabled(body,true); check(plain);
        b3_destroy_shape(shape,true); check(SceneCapabilities{component_bodies:false,..plain});
        let mut sd=b3_default_shape_def(); sd.is_sensor=true;
        let sensor=b3_create_sphere_shape(body,&sd,&Sphere{center:[0.0;3],radius:0.5});
        check(SceneCapabilities{shape_events:true,ccd_shapes:false,..plain});
        b3_destroy_shape(sensor,true);
        b3_destroy_body(body); check(plain);
        let replacement=b3_create_body(world,&bd);
        b3_create_sphere_shape(replacement,&b3_default_shape_def(),&Sphere{center:[0.0;3],radius:0.5});
        check(plain);
        b3_destroy_world(world);
    }

    #[test]
    fn resident_steps_and_render_metadata_do_not_download_body_state() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        let world=b3_create_world(gpu,&b3_default_world_def());
        with_world_mut_no_sync(world,|w|w.gpu_ccd_requested=true);
        crate::scenes::create_ground(world,10.0);
        let mut bd=b3_default_body_def();bd.body_type=BodyType::Dynamic;
        bd.position=[0.0,2.0,0.0];bd.linear_velocity=[0.0,-240.0,0.0];
        let body=b3_create_body(world,&bd);
        b3_create_sphere_shape(body,&b3_default_shape_def(),&Sphere{center:[0.0;3],radius:0.5});
        for _ in 0..240 {
            b3_world_step_gpu(world,1.0/60.0,4);
            b3_world_finalize_render_state(world);
            assert_eq!(b3_world_counts(world).0,2);
            assert!(!b3_world_gpu_report_name(world).is_empty());
        }
        with_world_no_sync(world,|w|{
            assert!(can_submit_resident(w) && w.gpu_mirror_stale);
            assert_eq!(body_ref(w,body).unwrap().gpu.pos[1],2.0,"host must remain untouched until explicitly queried");
            let sim=w.sim.as_ref().unwrap();
            assert_eq!(sim.last_mirror_timings().3,0,"no body mirror copy");
            assert_eq!(sim.pose_snapshot_maps(),0,"no pose mapping");
        });
        b3_world_step_gpu(world,0.0,4);
        assert_eq!(b3_world_physics_step(world),240);
        // Synchronous query still waits and sees the corrected latest state.
        let position=b3_body_get_position(body);
        assert!((position[1]-0.5).abs()<0.01,"{position:?}");
        with_world_no_sync(world,|w|assert!(body_ref(w,body).unwrap().gpu.flags & FLAG_SLEEP!=0));
        assert_eq!(b3_world_body_event_ptrs(world).1,0,
            "a body asleep before the last step must not report a new sleep event from stale host flags");

        b3_body_set_transform(body,[0.0,2.0,0.0],[0.0,0.0,0.0,1.0]);
        b3_body_set_linear_velocity(body,[0.0,-240.0,0.0]);
        b3_body_apply_force_to_center(body,[1000.0,0.0,0.0],true);
        b3_world_step_gpu(world,1.0/60.0,4);
        for _ in 0..8 {b3_world_step_gpu(world,1.0/60.0,4);b3_world_finalize_render_state(world);}
        assert!(b3_body_get_position(body)[1]>0.49);
        assert!(b3_body_get_position(body)[0]>0.0,"sparse host force must reach GPU state");
        b3_destroy_body(body);
        let replacement=b3_create_body(world,&bd);
        b3_create_sphere_shape(replacement,&b3_default_shape_def(),&Sphere{center:[0.0;3],radius:0.5});
        bd.position=[-3.0,3.0,0.0];
        let extra=b3_create_body(world,&bd);
        b3_create_sphere_shape(extra,&b3_default_shape_def(),&Sphere{center:[0.0;3],radius:0.5});
        b3_world_ensure_gpu(world); // Topology upload before stepping must clear old CCD spans.
        for _ in 0..8 {b3_world_step_gpu(world,1.0/60.0,4);b3_world_finalize_render_state(world);}
        assert!(b3_body_get_position(replacement)[1]>0.49);
        assert!(b3_body_get_position(extra)[1]>0.49);
        b3_destroy_world(world);
    }

    #[test]
    fn convex_gpu_ccd_matches_large_scene_cpu_ccd_checkpoints() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        for (scene,count) in [(crate::types::DemoScene::MixedStacks,600),(crate::types::DemoScene::Dominoes,30)] {
            let config=crate::types::DemoConfig{scene,body_count:count,body_count_explicit:true,contacts:true,jacobi:false};
            let mut baseline=Vec::new();
            for use_gpu in [false,true] {
                let world=crate::scenes::build_demo_world(gpu.clone(),&config);
                with_world_mut_no_sync(world,|w|w.gpu_ccd_requested=use_gpu);
                for step in 1..=120 {
                    if use_gpu {
                        b3_world_step_gpu(world,1.0/60.0,4);
                        b3_world_finalize_render_state(world);
                    } else {b3_world_step(world,1.0/60.0,4);}
                    if [1,30,60,120].contains(&step) {
                        if use_gpu {b3_world_gpu_wait_with_mirror(world);}
                        let state=with_world_no_sync(world,|w|{
                            assert_eq!(w.sim.as_ref().unwrap().uses_convex_ccd(),use_gpu);
                            w.bodies.iter().flatten().map(|b|b.gpu).collect::<Vec<_>>()
                        }).unwrap();
                        if use_gpu {
                            let index=[1,30,60,120].iter().position(|&x|x==step).unwrap();
                            let expected:&Vec<BodyGpu>=&baseline[index];
                            assert_eq!(state.len(),expected.len());
                            for (i,(a,b)) in state.iter().zip(expected).enumerate() {
                                let dp=glam::Vec3::from_array(a.pos).distance(glam::Vec3::from_array(b.pos));
                                assert!(dp<0.002,"{scene:?} step {step} body {i}: position error {dp}");
                                let rotation=glam::Quat::from_array(a.rot).dot(glam::Quat::from_array(b.rot)).abs();
                                assert!(rotation>0.999999,"{scene:?} step {step} body {i}: rotation dot {rotation}");
                                let dv=glam::Vec3::from_array(a.vel).distance(glam::Vec3::from_array(b.vel));
                                let dw=glam::Vec3::from_array(a.omega).distance(glam::Vec3::from_array(b.omega));
                                assert!(dv<0.02 && dw<0.02,"{scene:?} step {step} body {i}: velocity errors {dv}, {dw}");
                                assert!(a.pos.iter().chain(a.vel.iter()).chain(a.rot.iter()).all(|x|x.is_finite()));
                            }
                        } else {baseline.push(state);}
                    }
                }
                b3_destroy_world(world);
            }
        }
    }

    #[test]
    fn convex_gpu_ccd_preserves_kinematic_filters_and_joint_exclusions() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        for case in 0..5 {
            let mut baseline=0.0;
            for use_gpu in [false,true] {
                let world=b3_create_world(gpu.clone(),&b3_default_world_def());
                with_world_mut_no_sync(world,|w|w.gpu_ccd_requested=use_gpu);
                b3_world_set_contacts(world,false); // Exercise CCD, not speculative response.
                let mut bd=b3_default_body_def();bd.body_type=BodyType::Kinematic;
                bd.position=[0.0,-1.0,0.0];bd.linear_velocity=[0.5,0.0,0.0];
                let ground=b3_create_body(world,&bd);
                let mut sd=b3_default_shape_def();sd.filter.category_bits=1u64<<40;sd.filter.mask_bits=1u64<<41;
                if case==2 {sd.filter.group_index=-7;}
                if case==3 {sd.filter.group_index=7;sd.filter.mask_bits=0;}
                b3_create_hull_shape(ground,&sd,&b3_make_box_hull(10.0,1.0,10.0));
                bd.body_type=BodyType::Dynamic;bd.position=[0.0,2.0,0.0];bd.linear_velocity=[0.0,-240.0,0.0];
                let body=b3_create_body(world,&bd);
                sd.filter.category_bits=1u64<<41;sd.filter.mask_bits=if case==1 || case==3 {0}else{1u64<<40};
                b3_create_sphere_shape(body,&sd,&Sphere{center:[0.0;3],radius:0.5});
                if case==4 {
                    let mut jd=b3_default_filter_joint_def();jd.body_a=ground;jd.body_b=body;
                    b3_create_filter_joint(world,&jd);
                }
                b3_world_step(world,1.0/60.0,4);
                assert_eq!(with_world_no_sync(world,|w|w.sim.as_ref().unwrap().uses_convex_ccd()),Some(use_gpu));
                let y=b3_body_get_position(body)[1];
                if use_gpu {assert!((y-baseline).abs()<2e-4,"case {case}: GPU {y} CPU {baseline}");}
                else {baseline=y;}
                assert_eq!(y>0.49,case==0 || case==3,"case {case}: {y}");
                b3_destroy_world(world);
            }
        }
    }

    #[test]
    fn convex_gpu_ccd_corrects_live_steps_and_rebuilds_after_mutation() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
        let mut reference=Vec::new();
        for use_gpu in [false,true] {
            let world=b3_create_world(gpu.clone(),&b3_default_world_def());
            with_world_mut_no_sync(world,|w|w.gpu_ccd_requested=use_gpu);
            crate::scenes::create_ground(world,10.0);
            let mut bd=b3_default_body_def();bd.body_type=BodyType::Dynamic;
            let hole=b3_create_body(world,&bd);
            let mut bodies=Vec::new();
            for i in 0..3 {
                bd.position=[i as f32*3.0-3.0,2.0,0.0];bd.linear_velocity=[0.0,-240.0,0.0];
                let body=b3_create_body(world,&bd);
                match i {
                    0=>{b3_create_sphere_shape(body,&b3_default_shape_def(),&Sphere{center:[1.0,0.0,0.0],radius:0.5});}
                    1=>{b3_create_capsule_shape(body,&b3_default_shape_def(),&Capsule{center1:[-0.3,0.0,0.0],center2:[0.3,0.0,0.0],radius:0.3});}
                    _=>{b3_create_hull_shape(body,&b3_default_shape_def(),&b3_make_box_hull(0.4,0.4,0.4));}
                }
                bodies.push(body);
            }
            b3_destroy_body(hole);
            for step in 0..3 {
                b3_world_step(world,1.0/60.0,4);
                let error=pollster::block_on(gpu.device.pop_error_scope());
                if let Some(error)=error { panic!("CCD integration validation: {error}"); }
                gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
                assert_eq!(with_world_no_sync(world,|w|w.sim.as_ref().unwrap().uses_convex_ccd()),Some(use_gpu));
                for (i,&body) in bodies.iter().enumerate() {
                    let position=b3_body_get_position(body);
                    assert!(position[1]>0.25,"body {i} fell through: {position:?}");
                    if use_gpu {
                        let expected:[f32;3]=reference[step*3+i];
                        assert!(glam::Vec3::from_array(position).distance(glam::Vec3::from_array(expected))<0.002,
                            "step {step} body {i}: GPU {position:?}, CPU {expected:?}");
                    } else {reference.push(position);}
                }
            }
            // A body-flag mutation must invalidate the eligible cohort; do not
            // silently skip bullet CCD with the ordinary-body kernel.
            b3_body_set_bullet(bodies[0],true);
            b3_world_step(world,1.0/60.0,4);
            assert_eq!(with_world_no_sync(world,|w|w.sim.as_ref().unwrap().uses_convex_ccd()),Some(false));
            b3_body_set_bullet(bodies[0],false);
            b3_body_set_transform(bodies[0],[-3.0,2.0,0.0],[0.0,0.0,0.0,1.0]);
            b3_body_set_linear_velocity(bodies[0],[0.0,-240.0,0.0]);
            b3_world_step(world,1.0/60.0,4);
            assert_eq!(with_world_no_sync(world,|w|w.sim.as_ref().unwrap().uses_convex_ccd()),Some(use_gpu));
            assert!(b3_body_get_position(bodies[0])[1]>0.49);
            b3_destroy_world(world);
        }
        assert!(pollster::block_on(gpu.device.pop_error_scope()).is_none(),"CCD integration shader validation");
    }
}


#[cfg(all(test, not(target_arch = "wasm32")))]
mod complete_component_tests {
    use super::*;
    use crate::api::{b3_default_world_def,b3_default_body_def,b3_default_shape_def,b3_make_box_hull};
    #[test]
    fn contradictory_degree_two_proof_invalidates_submission() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        let world=b3_create_world(gpu,&b3_default_world_def());
        b3_world_set_diagnostic_flags(world,crate::types::DIAG_BOUNDED_STATIC_SORT);
        let mut bd=b3_default_body_def();bd.position=[0.0,-0.5,0.0];
        for _ in 0..3 {
            let body=b3_create_body(world,&bd);
            b3_create_hull_shape(body,&b3_default_shape_def(),&b3_make_box_hull(2.0,0.5,2.0));
        }
        bd.body_type=BodyType::Dynamic;bd.position=[0.0,0.5,0.0];
        let body=b3_create_body(world,&bd);
        b3_create_hull_shape(body,&b3_default_shape_def(),&b3_make_box_hull(0.5,0.5,0.5));
        b3_world_ensure_gpu(world);
        with_world_mut_no_sync(world,|w| {
            let bounds=w.cached_topology.as_mut().unwrap();
            assert!(!bounds.static_degree_two_proof);
            bounds.static_degree_two_proof=true; // deliberately contradict live geometry
        });
        b3_world_step_gpu(world,1.0/60.0,4);
        assert!(pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
        b3_destroy_world(world);
    }

    #[test]
    fn complete_components_merge_and_split_match_global_phases() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
        for sleep in [false,true] {
        let run=|hybrid| {
            let mut wd=b3_default_world_def();wd.enable_sleep=sleep;
            let world=b3_create_world(gpu.clone(),&wd);
            with_world_mut_no_sync(world,|w|{w.component_tgs_requested=hybrid;});
            let mut bd=b3_default_body_def();bd.body_type=BodyType::Static;bd.position=[0.0,-0.5,0.0];
            let ground=b3_create_body(world,&bd);
            b3_create_hull_shape(ground,&b3_default_shape_def(),&b3_make_box_hull(60.0,0.5,5.0));
            let mut bodies=Vec::new();
            bd.body_type=BodyType::Dynamic;
            // Twelve touching bodies make one large component; removing the
            // middle body splits it into components below the eight-body limit.
            for i in 0..12 {
                bd.position=[i as f32,0.5,0.0];
                let body=b3_create_body(world,&bd);
                b3_create_hull_shape(body,&b3_default_shape_def(),&b3_make_box_hull(0.5,0.5,0.5));
                bodies.push(body);
            }
            bd.position=[40.0,3.0,0.0]; // independent small component, shared ground
            let small=b3_create_body(world,&bd);
            b3_create_hull_shape(small,&b3_default_shape_def(),&b3_make_box_hull(0.5,0.5,0.5));
            let mut checkpoints=Vec::new();
            for frame in 0..90 {
                if frame==20 || frame==50 {
                    b3_body_set_transform(bodies[6],if frame==20 {[25.0,0.5,0.0]}else{[6.0,0.5,0.0]},[0.0,0.0,0.0,1.0]);
                    b3_body_set_linear_velocity(bodies[6],[0.0;3]);
                    b3_body_set_angular_velocity(bodies[6],[0.0;3]);
                }
                b3_world_step_gpu(world,1.0/60.0,4);
                if hybrid && [19,21,51].contains(&frame) {
                    let count=with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().test_large_component_count()).unwrap();
                    assert_eq!(count,if frame==21 {0}else{1},"large ownership at frame {frame}");
                }
                if sleep && frame==49 {
                    let states=pollster::block_on(b3_world_sync_from_gpu(world));
                    assert!(states.iter().any(|b|b.inv_mass>0.0 && b.flags & FLAG_SLEEP != 0),
                        "sleep/wake comparison must exercise sleeping components");
                }
                if sleep && frame==89 {
                    let states=pollster::block_on(b3_world_sync_from_gpu(world));
                    assert!(states[1..13].iter().all(|b|b.flags & FLAG_SLEEP != 0),
                        "reconnected large component must return to sleep");
                }
                if [0,19,20,21,49,50,51,89].contains(&frame) {
                    checkpoints.push(pollster::block_on(b3_world_sync_from_gpu(world)));
                }
            }
            assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
            b3_destroy_world(world);checkpoints
        };
        let expected=run(false);let actual=run(true);
        for (checkpoint,(a,b)) in actual.iter().zip(&expected).enumerate() {
            assert_eq!(a.len(),b.len());
            for (index,(a,b)) in a.iter().zip(b).enumerate() {
                for (x,y) in a.pos.iter().chain(&a.rot).chain(&a.vel).chain(&a.omega)
                    .zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)) {
                    assert!(x.is_finite() && y.is_finite() && (x-y).abs()<1e-5,
                        "checkpoint {checkpoint} body {index}: {x} != {y}");
                }
            }
        }
        }
        if let Some(error)=pollster::block_on(gpu.device.pop_error_scope()) {panic!("{error}");}
    }

    #[test]
    fn complete_components_match_global_color_phases() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
        for (scene,count) in [(crate::types::DemoScene::MixedStacks,600),(crate::types::DemoScene::Dominoes,30)] {
            let run=|enabled| {
                let cfg=crate::types::DemoConfig{scene,body_count:count,body_count_explicit:true,contacts:true,jacobi:false};
                let world=crate::scenes::build_demo_world(gpu.clone(),&cfg);
                with_world_mut_no_sync(world,|w| {
                    w.component_tgs_requested=enabled;
                    // This fixture asserts dispatch counts even after bodies sleep.
                    w.gpu_idle_requested=false;
                });
                b3_world_set_diagnostic_flags(world,if enabled {crate::types::DIAG_BOUNDED_STATIC_SORT}else{crate::types::DIAG_FORCE_GENERAL_STATIC_SORT});
                for _ in 0..120 { b3_world_step_gpu(world,1.0/60.0,4); }
                if enabled {assert_eq!(b3_world_last_solver_dispatches(world),2,"prototype must actually run");}
                let states=pollster::block_on(b3_world_sync_from_gpu(world));
                let stats=pollster::block_on(b3_world_live_step_stats(world)).unwrap();
                assert!(!stats.capacity_loss());
                b3_destroy_world(world);
                states
            };
            let expected=run(false);let actual=run(true);
            assert_eq!(actual.len(),expected.len());
            for (index,(a,b)) in actual.iter().zip(&expected).enumerate() {
                for (x,y) in a.pos.iter().chain(&a.rot).chain(&a.vel).chain(&a.omega)
                    .zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)) {
                    assert!(x.is_finite() && y.is_finite() && (x-y).abs()<1e-5,"{scene:?} body {index}: {x} != {y}");
                }
            }
        }
        if let Some(error)=pollster::block_on(gpu.device.pop_error_scope()) {panic!("{error}");}
    }

    #[test]
    fn falling_cubes_exceed_old_spatial_insertion_limit() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        let cfg = crate::types::DemoConfig {
            scene: crate::types::DemoScene::FallingCubes,
            body_count: 30000, body_count_explicit: true,
            contacts: true, jacobi: false,
        };
        let world = crate::scenes::build_demo_world(gpu, &cfg);
        b3_world_enable_sleeping(world, false);
        b3_world_ensure_gpu(world);
        b3_world_begin_timing(world, 1);
        b3_world_step_gpu(world, 1.0 / 60.0, 4);
        b3_world_gpu_wait(world);
        let timing = pollster::block_on(b3_world_finish_timing(world)).unwrap();
        let inserts = timing.workload.cell_inserts_peak;
        assert!(inserts > 65536, "fixture must exceed old insertion budget: {inserts}");
        let stats = pollster::block_on(b3_world_live_step_stats(world)).unwrap();
        assert!(!stats.capacity_loss(), "{stats:?}");
        eprintln!("30,000 falling cubes: {} spatial entries, no capacity loss", inserts);
        b3_destroy_world(world);
    }

    #[test]
    fn falling_cube_solver_schedules_match_through_impact() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
        // Exercise the measured dense workload beyond the 8,168-body shared
        // graph limit, including the complete benchmark collision window.
        let checkpoints = [1, 65, 78, 79, 90, 150, 330];
        let run = |component, batched| {
            let cfg = crate::types::DemoConfig {
                scene: crate::types::DemoScene::FallingCubes,
                body_count: 15000,
                body_count_explicit: true,
                contacts: true,
                jacobi: false,
            };
            let world = crate::scenes::build_demo_world(gpu.clone(), &cfg);
            with_world_mut_no_sync(world, |w| w.component_tgs_requested = component);
            b3_world_enable_sleeping(world, false);
            b3_world_ensure_gpu(world);
            with_world_mut_no_sync(world, |w| {
                w.sim.as_mut().unwrap().set_batched_graph_test(batched)
            });
            if !component {
                with_world_mut_no_sync(world, |w| w.sim.as_mut().unwrap().set_color_wave_prefix(20));
            }
            let mut snapshots = Vec::new();
            for step in 1..=330 {
                b3_world_step_gpu(world, 1.0 / 60.0, 4);
                if checkpoints.contains(&step) {
                    let bodies = pollster::block_on(b3_world_sync_from_gpu(world));
                    let contacts = with_world_mut_no_sync(world, |w| {
                        pollster::block_on(w.sim.as_mut().unwrap().read_contacts())
                    })
                    .unwrap();
                    let root = |mut index: usize| loop {
                        let parent = bodies[index].island_id as usize;
                        if parent == index {
                            break index;
                        }
                        assert!(parent < index, "invalid island parent at step {step}");
                        index = parent;
                    };
                    for contact in contacts.iter().filter(|c| c.count > 0 && c.a != u32::MAX) {
                        let a = contact.a as usize;
                        let b = contact.b as usize;
                        if bodies[a].inv_mass > 0.0 && bodies[b].inv_mass > 0.0 {
                            assert_eq!(
                                root(a),
                                root(b),
                                "contact {a}-{b} crosses islands: component={component}, step={step}"
                            );
                        }
                    }
                    snapshots.push(bodies);
                    let stats = pollster::block_on(b3_world_live_step_stats(world)).unwrap();
                    assert!(!stats.capacity_loss(), "component={component}, step={step}");
                }
            }
            if component {
                assert_eq!(b3_world_last_solver_dispatches(world), 2);
            } else {
                assert!(b3_world_last_solver_dispatches(world) > 2);
            }
            b3_destroy_world(world);
            snapshots
        };
        let global = run(false, false);
        let repeat = run(false, false);
        for (step, (a, b)) in global.iter().zip(&repeat).enumerate() {
            let err = a
                .iter()
                .zip(b)
                .flat_map(|(a, b)| {
                    a.pos
                        .iter()
                        .chain(&a.rot)
                        .chain(&a.vel)
                        .chain(&a.omega)
                        .zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega))
                })
                .map(|(x, y)| (x - y).abs())
                .fold(0.0f32, f32::max);
            assert_eq!(err, 0.0, "global repeat step {}", checkpoints[step]);
        }

        for (variant, component) in [
            ("component", run(true, false)),
            ("batched-graph", run(false, true)),
        ] {
            let mut maximum_error = 0.0f32;
            for (checkpoint, (actual, expected)) in component.iter().zip(&global).enumerate() {
                assert_eq!(actual.len(), 15001);
                assert_eq!(actual.len(), expected.len());
                let mut channel_errors = [0.0f32; 4];
                for (a, b) in actual.iter().zip(expected) {
                    for (channel, (left, right)) in [
                        (&a.pos[..], &b.pos[..]),
                        (&a.rot[..], &b.rot[..]),
                        (&a.vel[..], &b.vel[..]),
                        (&a.omega[..], &b.omega[..]),
                    ]
                    .into_iter()
                    .enumerate()
                    {
                        for (x, y) in left.iter().zip(right) {
                            assert!(x.is_finite() && y.is_finite());
                            channel_errors[channel] = channel_errors[channel].max((x - y).abs());
                        }
                    }
                }
                eprintln!(
                    "step {}: max position/rotation/velocity/omega errors {channel_errors:?}",
                    checkpoints[checkpoint]
                );
                maximum_error = maximum_error.max(channel_errors.into_iter().fold(0.0f32, f32::max));
            }
            assert!(
                maximum_error < 1e-5,
                "{variant} disagreement: {maximum_error}"
            );
        }
        if let Some(error) = pollster::block_on(gpu.device.pop_error_scope()) {
            panic!("{error}");
        }
    }

    #[test]
    fn batched_graph_preserves_dense_overflow_and_partial_batches() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
        for count in [2, 23, 24, 40] {
            let run = |batched| {
                let mut wd = b3_default_world_def();
                wd.gravity = [0.0; 3];
                wd.enable_sleep = false;
                let world = b3_create_world(gpu.clone(), &wd);
                with_world_mut_no_sync(world, |w| w.component_tgs_requested = false);
                let hull = crate::api::b3_make_cube_hull(0.5);
                let mut bd = b3_default_body_def();
                bd.body_type = BodyType::Dynamic;
                for i in 0..count {
                    bd.position = [0.001 * (i % 4) as f32, 0.002 * (i / 4) as f32, 0.0];
                    b3_create_hull_shape(b3_create_body(world, &bd), &b3_default_shape_def(), &hull);
                }
                b3_world_ensure_gpu(world);
                with_world_mut_no_sync(world, |w| {
                    let sim = w.sim.as_mut().unwrap();
                    sim.set_shared_graph_test(false);
                    sim.set_batched_graph_test(batched);
                    sim.set_color_wave_prefix(20);
                });
                b3_world_step_gpu(world, 1.0 / 60.0, 4);
                let bodies = pollster::block_on(b3_world_sync_from_gpu(world));
                let contacts = with_world_mut_no_sync(world, |w| {
                    pollster::block_on(w.sim.as_mut().unwrap().read_contacts())
                })
                .unwrap();
                let schedule = contacts
                    .iter()
                    .enumerate()
                    .filter(|(_, c)| c.count > 0)
                    .map(|(slot, c)| (slot, c.a, c.b, c.color, c.lifecycle[2]))
                    .collect::<Vec<_>>();
                assert_eq!(schedule.len(), (count * (count - 1) / 2) as usize);
                if count > 20 {
                    assert!(schedule
                        .iter()
                        .any(|(_, _, _, color, _)| *color == crate::types::OVERFLOW_COLOR));
                }
                assert!(!pollster::block_on(b3_world_live_step_stats(world))
                    .unwrap()
                    .capacity_loss());
                b3_destroy_world(world);
                (bodies, schedule)
            };
            let (expected, expected_schedule) = run(false);
            let (actual, actual_schedule) = run(true);
            assert_eq!(
                actual_schedule, expected_schedule,
                "count={count}: exact slot/color/order"
            );
            for (a, b) in actual.iter().zip(&expected) {
                for (x, y) in a
                    .pos
                    .iter()
                    .chain(&a.rot)
                    .chain(&a.vel)
                    .chain(&a.omega)
                    .zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega))
                {
                    assert!(x.is_finite() && y.is_finite() && (x - y).abs() < 1e-5);
                }
            }
        }
        if let Some(error) = pollster::block_on(gpu.device.pop_error_scope()) {
            panic!("{error}");
        }
    }

    #[test]
    fn completed_wait_refreshes_stale_capacity_status() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        for component in [false, true] {
            let cfg = crate::types::DemoConfig {
                scene: crate::types::DemoScene::FallingCubes,
                body_count: 1, body_count_explicit: true, contacts: true, jacobi: false,
            };
            let world = crate::scenes::build_demo_world(gpu.clone(), &cfg);
            b3_world_enable_sleeping(world, false);
            with_world_mut_no_sync(world, |w| w.component_tgs_requested = component);
            b3_world_ensure_gpu(world);
            // Occupy the single status slot with a clean step while later GPU
            // work records a loss. This reproduces the lag deterministically.
            with_world_mut_no_sync(world, |w| w.sim.as_mut().unwrap().hold_idle_status_test(true));
            b3_world_step_gpu(world, 1.0 / 60.0, 4);
            b3_world_set_diagnostic_flags(world, crate::types::DIAG_FORCE_CAPACITY_LOSS);
            b3_world_step_gpu(world, 1.0 / 60.0, 4);
            b3_world_set_diagnostic_flags(world, 0);
            b3_world_step_gpu(world, 1.0 / 60.0, 4);
            assert_eq!(b3_world_physics_step(world), 3);
            with_world_mut_no_sync(world, |w| w.sim.as_mut().unwrap().hold_idle_status_test(false));
            b3_world_gpu_wait(world);
            assert!(!b3_world_gpu_fail(world).is_null());
            let stats = pollster::block_on(b3_world_live_step_stats(world)).unwrap();
            assert!(stats.capacity_loss());
            assert_eq!(stats.first_fail_step, 2);
            b3_world_step_gpu(world, 1.0 / 60.0, 4);
            assert_eq!(b3_world_physics_step(world), 3, "no stepping after loss");
            b3_destroy_world(world);
        }
    }

    #[test]
    fn small_component_workgroup_boundaries_preserve_every_body() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
        for count in [0u32,1,7,8,9,15,16,17,31,32,33,63,64,65] {
            let run=|size| {
                let world=b3_create_world(gpu.clone(),&b3_default_world_def());
                with_world_mut_no_sync(world,|w|w.component_tgs_requested=true);
                b3_world_enable_sleeping(world,false);
                let mut bd=b3_default_body_def();bd.body_type=BodyType::Dynamic;
                for i in 0..count {
                    bd.position=[3.0*i as f32,10.0,0.0];
                    let body=b3_create_body(world,&bd);
                    b3_create_hull_shape(body,&b3_default_shape_def(),&b3_make_box_hull(0.5,0.5,0.5));
                }
                b3_world_ensure_gpu(world);
                with_world_mut_no_sync(world,|w|{if let Some(sim)=w.sim.as_mut() {sim.set_small_component_group_test(size);}});
                for _ in 0..3 {b3_world_step_gpu(world,1.0/60.0,4);}
                let states=pollster::block_on(b3_world_sync_from_gpu(world));
                assert_eq!(states.len(),count as usize);
                assert!(states.iter().all(|b|b.pos[1]<9.999 && b.vel[1]<0.0),"every body must advance, including partial groups");
                b3_destroy_world(world);states
            };
            let expected=run(64);
            for size in [8,16,32] {
                let actual=run(size);
                for (a,b) in actual.iter().zip(&expected) {
                    for (x,y) in a.pos.iter().chain(&a.rot).chain(&a.vel).chain(&a.omega).zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)) {
                        assert!(x.is_finite() && (x-y).abs()<1e-5,"count {count}, group {size}: {x} != {y}");
                    }
                }
            }
        }
        if let Some(error)=pollster::block_on(gpu.device.pop_error_scope()) {panic!("{error}");}
    }

    #[test]
    fn complete_components_preserve_nonempty_overflow_order() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
        let run=|component| {
            let world=b3_create_world(gpu.clone(),&b3_default_world_def());
            b3_world_enable_sleeping(world,false);
            with_world_mut_no_sync(world,|w|w.component_tgs_requested=component);
            let sd=b3_default_shape_def();let mut bd=b3_default_body_def();
            bd.position=[0.0,-0.5,0.0];
            // One writable body with twenty-eight overlapping supports forces
            // ranks beyond the 22 available static colors into overflow.
            for _ in 0..28 {
                let ground=b3_create_body(world,&bd);
                b3_create_hull_shape(ground,&sd,&b3_make_box_hull(20.0,0.5,20.0));
            }
            bd.body_type=BodyType::Dynamic;bd.position=[0.0,0.49,0.0];bd.linear_velocity=[2.0,-0.1,0.5];
            let body=b3_create_body(world,&bd);
            b3_create_hull_shape(body,&sd,&b3_make_box_hull(0.5,0.5,0.5));
            b3_world_ensure_gpu(world);
            let mut states=Vec::new();let mut seen_overflow=false;
            for _ in 0..120 {
                b3_world_step_gpu(world,1.0/60.0,4);
                if component {assert_eq!(b3_world_last_solver_dispatches(world),2);}
                let contacts=with_world_mut_no_sync(world,|w|pollster::block_on(w.sim.as_mut().unwrap().read_contacts())).unwrap();
                let overflow=contacts.iter().filter(|c|c.count>0 && c.color==crate::types::OVERFLOW_COLOR).count();
                seen_overflow|=overflow>=2;
                states.push(pollster::block_on(b3_world_sync_from_gpu(world)));
            }
            assert!(seen_overflow,"must exercise order-sensitive overflow contacts");
            assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
            b3_destroy_world(world);states
        };
        let expected=run(false);let actual=run(true);
        for (frame,(a,b)) in actual.iter().zip(&expected).enumerate() {
            assert_eq!(a.len(),b.len());
            for (a,b) in a.iter().zip(b) {
                for (x,y) in a.pos.iter().chain(&a.rot).chain(&a.vel).chain(&a.omega)
                    .zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)) {
                    assert!(x.is_finite() && y.is_finite() && (x-y).abs()<1e-5,"frame {frame}: {x} != {y}");
                }
            }
        }
        if let Some(error)=pollster::block_on(gpu.device.pop_error_scope()) {panic!("{error}");}
    }

    #[test]
    fn large_components_preserve_nonempty_overflow_order() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
        let run=|component| {
            let world=b3_create_world(gpu.clone(),&b3_default_world_def());
            b3_world_enable_sleeping(world,false);
            with_world_mut_no_sync(world,|w|w.component_tgs_requested=component);
            let sd=b3_default_shape_def();let mut bd=b3_default_body_def();
            bd.position=[0.0,-0.5,0.0];
            // One writable body with thirty-six overlapping supports forces
            // ranks beyond the 22 available static colors into overflow.
            // More than 32 roots selects the cooperative large-component path.
            for _ in 0..36 {
                let ground=b3_create_body(world,&bd);
                b3_create_hull_shape(ground,&sd,&b3_make_box_hull(20.0,0.5,20.0));
            }
            bd.body_type=BodyType::Dynamic;bd.position=[0.0,0.49,0.0];bd.linear_velocity=[2.0,-0.1,0.5];
            let body=b3_create_body(world,&bd);
            b3_create_hull_shape(body,&sd,&b3_make_box_hull(0.5,0.5,0.5));
            b3_world_ensure_gpu(world);
            let mut states=Vec::new();let mut seen_overflow=false;
            for _ in 0..120 {
                b3_world_step_gpu(world,1.0/60.0,4);
                if component {assert_eq!(b3_world_last_solver_dispatches(world),2);}
                let contacts=with_world_mut_no_sync(world,|w|pollster::block_on(w.sim.as_mut().unwrap().read_contacts())).unwrap();
                let overflow=contacts.iter().filter(|c|c.count>0 && c.color==crate::types::OVERFLOW_COLOR).count();
                seen_overflow|=overflow>=2;
                states.push(pollster::block_on(b3_world_sync_from_gpu(world)));
            }
            assert!(seen_overflow,"must exercise order-sensitive overflow contacts");
            assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
            b3_destroy_world(world);states
        };
        let expected=run(false);let actual=run(true);
        for (frame,(a,b)) in actual.iter().zip(&expected).enumerate() {
            assert_eq!(a.len(),b.len());
            for (a,b) in a.iter().zip(b) {
                for (x,y) in a.pos.iter().chain(&a.rot).chain(&a.vel).chain(&a.omega)
                    .zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)) {
                    assert!(x.is_finite() && y.is_finite() && (x-y).abs()<1e-5,"frame {frame}: {x} != {y}");
                }
            }
        }
        if let Some(error)=pollster::block_on(gpu.device.pop_error_scope()) {panic!("{error}");}
    }

    #[test]
    fn small_component_workgroups_preserve_physics() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
        for (scene,count) in [(crate::types::DemoScene::MixedStacks,600),(crate::types::DemoScene::Dominoes,30)] {
            let run=|size| {
                let cfg=crate::types::DemoConfig{scene,body_count:count,body_count_explicit:true,contacts:true,jacobi:false};
                let world=crate::scenes::build_demo_world(gpu.clone(),&cfg);
                with_world_mut_no_sync(world,|w|w.component_tgs_requested=true);
                b3_world_set_diagnostic_flags(world,crate::types::DIAG_BOUNDED_STATIC_SORT);
                b3_world_enable_sleeping(world,false);
                b3_world_ensure_gpu(world);
                with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().set_small_component_group_test(size));
                for _ in 0..120 { b3_world_step_gpu(world,1.0/60.0,4); }
                assert_eq!(b3_world_last_solver_dispatches(world),2,"component path must actually run");
                let states=pollster::block_on(b3_world_sync_from_gpu(world));
                let stats=pollster::block_on(b3_world_live_step_stats(world)).unwrap();
                assert!(!stats.capacity_loss());
                b3_destroy_world(world);
                states
            };
            let expected=run(64);for size in [8,16,32] { let actual=run(size);
            assert_eq!(actual.len(),expected.len());
            for (index,(a,b)) in actual.iter().zip(&expected).enumerate() {
                for (x,y) in a.pos.iter().chain(&a.rot).chain(&a.vel).chain(&a.omega)
                    .zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)) {
                    assert!(x.is_finite() && y.is_finite() && (x-y).abs()<1e-5,"{scene:?} body {index}: {x} != {y}");
                }
            }
        }
        }
        if let Some(error)=pollster::block_on(gpu.device.pop_error_scope()) {panic!("{error}");}
    }

    #[test]
    fn pair_matrix_threshold_and_capacity_are_bounded() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        let run_candidates=|world| {
            b3_world_ensure_gpu(world);
            with_world_mut_no_sync(world,|w| {
                let sim=w.sim.as_mut().unwrap();sim.set_pair_matrix_test(true);
                sim.callback_candidates_submit();
                let pairs=pollster::block_on(sim.read_callback_pairs());
                let stats=pollster::block_on(sim.read_live_step_stats());
                (sim.pair_matrix_used_test(),sim.params().shape_count,pairs,stats.capacity_loss())
            }).unwrap()
        };
        let sd=b3_default_shape_def();let hull=b3_make_box_hull(0.1,0.1,0.1);
        let mut bd=b3_default_body_def();bd.body_type=BodyType::Dynamic;
        let world=b3_create_world(gpu.clone(),&b3_default_world_def());
        for i in 0..704 {bd.position=[i as f32*4.0,0.0,0.0];let b=b3_create_body(world,&bd);b3_create_hull_shape(b,&sd,&hull);}
        let (used,n,pairs,loss)=run_candidates(world);assert!(used && n==704 && pairs.is_empty() && !loss);
        bd.position=[4000.0,0.0,0.0];let extra=b3_create_body(world,&bd);b3_create_hull_shape(extra,&sd,&hull);
        let (used,n,pairs,loss)=run_candidates(world);assert!(!used && n==705 && pairs.is_empty() && !loss);
        b3_destroy_body(extra);
        let (used,n,pairs,loss)=run_candidates(world);assert_eq!(used,n<=704);assert!(pairs.is_empty() && !loss);
        b3_destroy_world(world);
        let world=b3_create_world(gpu,&b3_default_world_def());bd.position=[0.0;3];
        for _ in 0..704 {let b=b3_create_body(world,&bd);b3_create_hull_shape(b,&sd,&hull);}
        // Stop after candidate generation: do not solve a deliberately overflowing dense pile.
        let (used,n,pairs,loss)=run_candidates(world);assert!(used && n==704 && loss);
        assert_eq!(pairs.len(),crate::types::PAIR_CAP as usize);
        assert!(pairs.windows(2).all(|p|p[0]<p[1]));
        assert_eq!(pairs[0],1u64<<32);
        b3_destroy_world(world);
    }

    #[test]
    fn canonical_grid_matches_matrix_across_cells_and_hash_collisions() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        for reservation in [0,8193] {
        let mut wd=b3_default_world_def();
        wd.capacity.dynamic_body_count=reservation;
        wd.capacity.dynamic_shape_count=reservation;
        let world=b3_create_world(gpu.clone(),&wd);
        let sd=b3_default_shape_def();
        let mut bd=b3_default_body_def();
        // Translated clusters share hash buckets exactly but must not interact.
        let period=4.0*crate::types::HASH_BUCKETS as f32*crate::types::DEFAULT_CELL_SIZE;
        for cluster in 0..2 {
            let x=cluster as f32*period;
            bd.body_type=BodyType::Static;bd.position=[x,-1.0,0.0];
            let ground=b3_create_body(world,&bd);
            b3_create_hull_shape(ground,&sd,&b3_make_box_hull(16.0,0.5,16.0));
            for i in 0..96 {
                bd.body_type=if i%13==0 {BodyType::Static} else {BodyType::Dynamic};
                bd.position=[x+(i%8) as f32*1.5-5.0, (i/32) as f32*1.25-0.25, ((i/8)%4) as f32*1.5-2.5];
                let b=b3_create_body(world,&bd);
                let half=if i%11==0 {2.6} else {0.65};
                b3_create_hull_shape(b,&sd,&b3_make_box_hull(half,0.7,half));
            }
        }
        b3_world_ensure_gpu(world);
        let mut results=Vec::new();
        for matrix in [false,true,false] {
            let pairs=with_world_mut_no_sync(world,|w| {
                let sim=w.sim.as_mut().unwrap();sim.set_pair_matrix_test(matrix);
                sim.callback_candidates_submit();
                assert_eq!(sim.pair_matrix_used_test(),matrix);
                let pairs=pollster::block_on(sim.read_callback_pairs());
                if !matrix {
                    let counts=pollster::block_on(sim.read_scratch_prefix(3));
                    assert_eq!(counts[0] as usize,pairs.len(),"each pair must have exactly one emitting cell");
                }
                assert!(!pollster::block_on(sim.read_live_step_stats()).capacity_loss());
                pairs
            }).unwrap();
            assert!(!pairs.is_empty());results.push(pairs);
        }
        assert_eq!(results[0],results[1],"canonical grid must match independent all-pairs matrix");
        assert_eq!(results[1],results[2],"grid reuse after matrix must preserve every pair");
        b3_destroy_world(world);
        }
    }

    #[test]
    fn pair_matrix_mutations_match_grid() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
        let run=|enabled| {
            let world=b3_create_world(gpu.clone(),&b3_default_world_def());
            with_world_mut_no_sync(world,|w|w.component_tgs_requested=true);
            b3_world_enable_sleeping(world,false);
            b3_world_set_diagnostic_flags(world,crate::types::DIAG_BOUNDED_STATIC_SORT);
            let sd=b3_default_shape_def();let hull=b3_make_box_hull(0.5,0.5,0.5);
            let mut bd=b3_default_body_def();bd.position=[0.0,-0.5,0.0];
            let ground=b3_create_body(world,&bd);
            b3_create_hull_shape(ground,&sd,&b3_make_box_hull(20.0,0.5,20.0));
            bd.body_type=BodyType::Dynamic;
            let mut bodies=Vec::new();let mut shapes=Vec::new();
            for i in 0..3 {
                bd.position=[0.0,0.5+i as f32,0.0];
                let b=b3_create_body(world,&bd);bodies.push(b);shapes.push(b3_create_hull_shape(b,&sd,&hull));
            }
            let mut states=Vec::new();let mut schedules=Vec::new();let mut pairs=Vec::new();let mut max_hits=0;
            for frame in 0..80 {
                match frame {
                    10=>{let mut filter=sd.filter;filter.mask_bits=0;b3_shape_set_filter(shapes[2],filter,true);},
                    15=>b3_shape_set_filter(shapes[2],sd.filter,true),
                    20=>b3_body_set_transform(bodies[2],[3.0,0.5,0.0],[0.0,0.0,0.0,1.0]),
                    25=>b3_body_set_transform(bodies[2],[0.0,2.5,0.0],[0.0,0.0,0.0,1.0]),
                    30=>{b3_destroy_shape(shapes[2],true);shapes[2]=b3_create_hull_shape(bodies[2],&sd,&hull);},
                    35=>b3_body_set_type(bodies[1],BodyType::Static),
                    40=>b3_body_set_type(bodies[1],BodyType::Dynamic),
                    45=>b3_body_set_awake(bodies[0],false),
                    50=>b3_body_set_awake(bodies[0],true),
                    55=>{for i in 0..260 {bd.position=[4.0+(i%16) as f32,0.5+(i/16) as f32,4.0];let b=b3_create_body(world,&bd);b3_create_hull_shape(b,&sd,&hull);}},
                    _=>{},
                }
                b3_world_ensure_gpu(world);
                if frame==54 {assert_eq!(with_world_no_sync(world,|w|w.sim.as_ref().unwrap().caps.bodies).unwrap(),256);}
                if frame==55 {assert!(with_world_no_sync(world,|w|w.sim.as_ref().unwrap().caps.bodies).unwrap()>256,"must exercise real buffer growth");}
                with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().set_pair_matrix_test(enabled));
                b3_world_step_gpu(world,1.0/60.0,4);
                assert_eq!(with_world_no_sync(world,|w|w.sim.as_ref().unwrap().pair_matrix_used_test()).unwrap(),enabled);
                pairs.push(with_world_mut_no_sync(world,|w| {
                    let sim=w.sim.as_mut().unwrap();
                    let pairs=pollster::block_on(sim.read_callback_pairs());
                    if !enabled {
                        let counts=pollster::block_on(sim.read_scratch_prefix(3));
                        assert_eq!(counts[0] as usize,pairs.len(),"grid emitted duplicate pairs at frame {frame}");
                    }
                    pairs
                }).unwrap());
                states.push(pollster::block_on(b3_world_sync_from_gpu(world)));
                let contacts=with_world_mut_no_sync(world,|w|pollster::block_on(w.sim.as_mut().unwrap().read_contacts())).unwrap();
                schedules.push(contacts.iter().enumerate().filter(|(_,c)|c.count>0 && c.lifecycle[1]&2!=0 && (c.color<20 || c.color==23)).map(|(slot,c)|(slot,c.a,c.b,c.color,c.lifecycle[2])).collect::<Vec<_>>());
                if let Some(hits)=with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().test_graph_memo_hits()).unwrap() {max_hits=max_hits.max(hits);}
            }
            if enabled && std::env::var("GPU_PHYSICS_GRAPH_MEMO").as_deref()==Ok("1") {assert!(max_hits>0);}
            assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
            b3_destroy_world(world);(states,schedules,pairs)
        };
        let (expected,es,ep)=run(false);let (actual,asched,ap)=run(true);assert_eq!(ap,ep,"exact ordered candidate pairs per step");for (frame,(a,b)) in asched.iter().zip(&es).enumerate() {assert_eq!(a,b,"schedule frame {frame}");}
        for (frame,(actual,expected)) in actual.iter().zip(&expected).enumerate() {
            assert_eq!(actual.len(),expected.len());
            for (index,(a,b)) in actual.iter().zip(expected).enumerate() {
                for (x,y) in a.pos.iter().chain(&a.rot).chain(&a.vel).chain(&a.omega).zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)) {
                    assert!(x.is_finite() && y.is_finite() && (x-y).abs()<1e-5,"frame {frame} body {index}: {x} != {y}");
                }
            }
        }
        if let Some(error)=pollster::block_on(gpu.device.pop_error_scope()) {panic!("{error}");}
    }

    #[test]
    fn shared_graph_mutations_match_canonical() { graph_mutations_match_canonical(false); }

    #[test]
    fn batched_graph_mutations_match_canonical() { graph_mutations_match_canonical(true); }

    fn graph_mutations_match_canonical(batched: bool) {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
        let run=|enabled| {
            let world=b3_create_world(gpu.clone(),&b3_default_world_def());
            with_world_mut_no_sync(world,|w|w.component_tgs_requested=!batched);
            b3_world_enable_sleeping(world,false);
            b3_world_set_diagnostic_flags(world,crate::types::DIAG_BOUNDED_STATIC_SORT);
            let sd=b3_default_shape_def();let hull=b3_make_box_hull(0.5,0.5,0.5);
            let mut bd=b3_default_body_def();bd.position=[0.0,-0.5,0.0];
            let ground=b3_create_body(world,&bd);
            b3_create_hull_shape(ground,&sd,&b3_make_box_hull(20.0,0.5,20.0));
            bd.body_type=BodyType::Dynamic;
            let mut bodies=Vec::new();let mut shapes=Vec::new();
            for i in 0..3 {
                bd.position=[0.0,0.5+i as f32,0.0];
                let b=b3_create_body(world,&bd);bodies.push(b);shapes.push(b3_create_hull_shape(b,&sd,&hull));
            }
            let mut states=Vec::new();let mut schedules=Vec::new();let mut max_hits=0;
            for frame in 0..80 {
                match frame {
                    10=>{let mut filter=sd.filter;filter.mask_bits=0;b3_shape_set_filter(shapes[2],filter,true);},
                    15=>b3_shape_set_filter(shapes[2],sd.filter,true),
                    20=>b3_body_set_transform(bodies[2],[3.0,0.5,0.0],[0.0,0.0,0.0,1.0]),
                    25=>b3_body_set_transform(bodies[2],[0.0,2.5,0.0],[0.0,0.0,0.0,1.0]),
                    30=>{b3_destroy_shape(shapes[2],true);shapes[2]=b3_create_hull_shape(bodies[2],&sd,&hull);},
                    32 if batched=>{
                        // Destroy and reuse a body slot while cached edge inputs exist.
                        b3_destroy_body(bodies[2]);bd.position=[0.0,2.5,0.0];
                        bodies[2]=b3_create_body(world,&bd);
                        shapes[2]=b3_create_hull_shape(bodies[2],&sd,&hull);
                    },
                    35=>b3_body_set_type(bodies[1],BodyType::Static),
                    40=>b3_body_set_type(bodies[1],BodyType::Dynamic),
                    45=>b3_body_set_awake(bodies[0],false),
                    50=>b3_body_set_awake(bodies[0],true),
                    55=>{for i in 0..260 {bd.position=[4.0+(i%16) as f32,0.5+(i/16) as f32,4.0];let b=b3_create_body(world,&bd);b3_create_hull_shape(b,&sd,&hull);}},
                    _=>{},
                }
                b3_world_ensure_gpu(world);
                if frame==54 {assert_eq!(with_world_no_sync(world,|w|w.sim.as_ref().unwrap().caps.bodies).unwrap(),256);}
                if frame==55 {assert!(with_world_no_sync(world,|w|w.sim.as_ref().unwrap().caps.bodies).unwrap()>256,"must exercise real buffer growth");}
                with_world_mut_no_sync(world,|w|{
                    let sim=w.sim.as_mut().unwrap();
                    sim.set_shared_graph_test(enabled && !batched);
                    sim.set_batched_graph_test(enabled && batched);
                });
                b3_world_step_gpu(world,1.0/60.0,4);
                states.push(pollster::block_on(b3_world_sync_from_gpu(world)));
                let contacts=with_world_mut_no_sync(world,|w|pollster::block_on(w.sim.as_mut().unwrap().read_contacts())).unwrap();
                schedules.push(contacts.iter().enumerate().filter(|(_,c)|c.count>0 && c.lifecycle[1]&2!=0 && (c.color<20 || c.color==23)).map(|(slot,c)|(slot,c.a,c.b,c.color,c.lifecycle[2])).collect::<Vec<_>>());
                if let Some(hits)=with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().test_graph_memo_hits()).unwrap() {max_hits=max_hits.max(hits);}
            }
            if enabled && std::env::var("GPU_PHYSICS_GRAPH_MEMO").as_deref()==Ok("1") {assert!(max_hits>0);}
            assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
            b3_destroy_world(world);(states,schedules)
        };
        let (expected,es)=run(false);let (actual,asched)=run(true);assert_eq!(asched,es);
        for (frame,(actual,expected)) in actual.iter().zip(&expected).enumerate() {
            assert_eq!(actual.len(),expected.len());
            for (index,(a,b)) in actual.iter().zip(expected).enumerate() {
                for (x,y) in a.pos.iter().chain(&a.rot).chain(&a.vel).chain(&a.omega).zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)) {
                    assert!(x.is_finite() && y.is_finite() && (x-y).abs()<1e-5,"frame {frame} body {index}: {x} != {y}");
                }
            }
        }
        if let Some(error)=pollster::block_on(gpu.device.pop_error_scope()) {panic!("{error}");}
    }

    #[test]
    fn shared_graph_matches_global_color_phases() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
        for (scene,count) in [(crate::types::DemoScene::MixedStacks,600),(crate::types::DemoScene::Dominoes,30)] {
            let run=|enabled| {
                let cfg=crate::types::DemoConfig{scene,body_count:count,body_count_explicit:true,contacts:true,jacobi:false};
                let world=crate::scenes::build_demo_world(gpu.clone(),&cfg);
                with_world_mut_no_sync(world,|w|w.component_tgs_requested=true);
                b3_world_set_diagnostic_flags(world,crate::types::DIAG_BOUNDED_STATIC_SORT);
                b3_world_ensure_gpu(world);
                with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().set_shared_graph_test(enabled));
                b3_world_enable_sleeping(world,false);
                let mut schedules=Vec::new();
                for frame in 0..120 {
                    b3_world_step_gpu(world,1.0/60.0,4);
                    if frame%10==0 {
                        let contacts=with_world_mut_no_sync(world,|w|pollster::block_on(w.sim.as_mut().unwrap().read_contacts())).unwrap();
                        schedules.push(contacts.iter().enumerate().filter(|(_,c)|c.count>0 && c.lifecycle[1]&2!=0 && (c.color<20 || c.color==23)).map(|(slot,c)|(slot,c.a,c.b,c.color,c.lifecycle[2])).collect::<Vec<_>>());
                    }
                }
                if enabled && scene==crate::types::DemoScene::MixedStacks {
                    if let Some(hits)=with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().test_graph_memo_hits()).unwrap() {
                        assert!(hits>0,"memoization must actually reuse a repeated active graph");
                    }
                }
                assert_eq!(with_world_no_sync(world,|w|w.sim.as_ref().unwrap().shared_graph_initialized()).unwrap(),enabled);
                let states=pollster::block_on(b3_world_sync_from_gpu(world));
                let stats=pollster::block_on(b3_world_live_step_stats(world)).unwrap();
                assert!(!stats.capacity_loss());
                b3_destroy_world(world);
                (states,schedules)
            };
            let (expected,expected_schedule)=run(false);let (actual,actual_schedule)=run(true);
            assert_eq!(actual_schedule,expected_schedule,"exact slot/color/local assignments");
            assert_eq!(actual.len(),expected.len());
            for (index,(a,b)) in actual.iter().zip(&expected).enumerate() {
                for (x,y) in a.pos.iter().chain(&a.rot).chain(&a.vel).chain(&a.omega)
                    .zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)) {
                    assert!(x.is_finite() && y.is_finite() && (x-y).abs()<1e-5,"{scene:?} body {index}: {x} != {y}");
                }
            }
        }
        if let Some(error)=pollster::block_on(gpu.device.pop_error_scope()) {panic!("{error}");}
    }
}


#[cfg(all(test,not(target_arch="wasm32")))]
mod idle_resident_tests {
    use super::*;
    use crate::api::{b3_default_world_def,b3_default_body_def,b3_default_shape_def,b3_make_box_hull};
    fn fixture(gpu:GpuDevice)->(WorldId,BodyId) {
        let world=b3_create_world(gpu,&b3_default_world_def());
        with_world_mut_no_sync(world,|w|{w.gpu_ccd_requested=true;w.gpu_resident_requested=true;w.gpu_idle_requested=true;});
        let mut bd=b3_default_body_def();bd.position=[0.0,-0.5,0.0];
        let ground=b3_create_body(world,&bd);
        b3_create_hull_shape(ground,&b3_default_shape_def(),&b3_make_box_hull(20.0,0.5,20.0));
        bd.body_type=BodyType::Dynamic;bd.position=[0.0,0.5,0.0];
        let body=b3_create_body(world,&bd);
        b3_create_hull_shape(body,&b3_default_shape_def(),&b3_make_box_hull(0.5,0.5,0.5));
        (world,body)
    }
    fn idle(world:WorldId)->bool {
        with_world_no_sync(world,|w|w.sim.as_ref().unwrap().last_step_was_idle()).unwrap()
    }
    fn step(world:WorldId) {b3_world_step_gpu(world,1.0/60.0,4);}
    fn settle(world:WorldId) {
        for _ in 0..180 {step(world);b3_world_gpu_wait(world);if idle(world) {return;}}
        panic!("world never entered verified idle stepping");
    }
    #[test]
    fn idle_resident_steps_preserve_state_and_submission_identity() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
        let (world,body)=fixture(gpu.clone());settle(world);
        let before=b3_body_get_position(body);
        let start=b3_world_physics_step(world);
        for _ in 0..120 {step(world);assert!(idle(world));assert_eq!(b3_world_last_solver_dispatches(world),0);}
        assert_eq!(b3_world_physics_step(world),start+120);
        b3_world_gpu_wait(world);
        assert_eq!(b3_body_get_position(body),before);
        assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
        b3_world_step_gpu(world,0.0,4);assert_eq!(b3_world_physics_step(world),start+120);
        b3_destroy_world(world);
        if let Some(error)=pollster::block_on(gpu.device.pop_error_scope()) {panic!("{error}");}
    }
    #[test]
    fn idle_resident_wake_matches_ordinary_contact_history() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        let run=|enabled| {
            let (world,body)=fixture(gpu.clone());
            with_world_mut_no_sync(world,|w|w.gpu_idle_requested=enabled);
            let mut bd=b3_default_body_def();bd.body_type=BodyType::Dynamic;bd.position=[0.0,1.5,0.0];
            let top=b3_create_body(world,&bd);
            b3_create_hull_shape(top,&b3_default_shape_def(),&b3_make_box_hull(0.5,0.5,0.5));
            for _ in 0..120 {step(world);b3_world_gpu_wait(world);}
            assert_eq!(idle(world),enabled);
            b3_body_apply_linear_impulse_to_center(body,[250.0,1000.0,0.0],true);
            let mut frames=Vec::new();
            for _ in 0..60 {step(world);frames.push(pollster::block_on(b3_world_sync_from_gpu(world)));}
            b3_destroy_world(world);frames
        };
        let expected=run(false);let actual=run(true);
        for (frame,(a,b)) in actual.iter().zip(&expected).enumerate() {
            for (slot,(a,b)) in a.iter().zip(b).enumerate() {
                for (x,y) in a.pos.iter().chain(&a.rot).chain(&a.vel).chain(&a.omega)
                    .zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)) {
                    assert!(x.is_finite() && y.is_finite() && (x-y).abs()<1e-5,"wake frame{frame} slot{slot}: {x} != {y}");
                }
            }
        }
    }

    #[test]
    fn idle_resident_delayed_proof_requires_unbroken_chain() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        for delay in [1,2,5] {
            for mutate in 0..5 {
                let (world,body)=fixture(gpu.clone());settle(world);
                // Drain old status, then force a fresh all-asleep submission.
                b3_world_gpu_wait(world);
                b3_body_set_awake(body,false);
                b3_world_ensure_gpu(world);
                b3_world_gpu_wait(world);
                with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().hold_idle_status_test(true));
                let start=b3_world_physics_step(world);
                for _ in 0..=delay {step(world);b3_world_gpu_wait(world);assert!(!idle(world));}
                match mutate {
                    1 => b3_body_apply_linear_impulse_to_center(body,[0.0,1000.0,0.0],true),
                    2 => b3_world_set_gravity(world,[0.0,-20.0,0.0]),
                    3 => {with_world_mut_no_sync(world,|w|w.query_state+=1);},
                    4 => {
                        with_world_mut_no_sync(world,|w|w.gpu_idle_requested=false);
                        step(world);b3_world_gpu_wait(world);
                        with_world_mut_no_sync(world,|w|w.gpu_idle_requested=true);
                    },
                    _ => {},
                }
                let before_zero=b3_world_physics_step(world);
                b3_world_step_gpu(world,0.0,4);
                assert_eq!(b3_world_physics_step(world),before_zero);
                with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().hold_idle_status_test(false));
                step(world);assert_eq!(idle(world),mutate==0,"delay{delay} mutate{mutate}");
                assert_eq!(b3_world_physics_step(world),start+delay+2+u64::from(mutate==4));
                b3_world_gpu_wait(world);
                let p=b3_body_get_position(body);
                if mutate==1 {assert!(p[1]>0.5);} else {assert!((p[1]-0.5).abs()<0.01);}
                assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
                b3_destroy_world(world);
            }
        }
    }

    #[test]
    fn idle_resident_growth_discards_old_proof() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        let (world,_)=fixture(gpu);settle(world);
        let capacity=with_world_no_sync(world,|w|w.sim.as_ref().unwrap().caps.bodies).unwrap();
        for _ in 0..capacity {b3_create_body(world,&b3_default_body_def());}
        step(world);assert!(!idle(world));
        assert!(with_world_no_sync(world,|w|w.sim.as_ref().unwrap().caps.bodies>capacity).unwrap());
        settle(world);
        assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
        b3_destroy_world(world);
    }

    #[test]
    fn idle_resident_mutations_restore_normal_physics() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
        let (world,body)=fixture(gpu.clone());settle(world);
        b3_body_set_linear_velocity(body,[0.0,3.0,0.0]);
        let hit=unsafe {crate::api::b3_world_cast_ray_closest(world,[0.0,5.0,0.0],[0.0,-10.0,0.0],crate::api::b3_default_query_filter())};
        assert!(hit.hit);
        step(world);
        assert!(!idle(world));assert!(b3_body_get_position(body)[1]>0.5);
        settle(world);
        b3_body_set_transform(body,[0.0,3.0,0.0],[0.0,0.0,0.0,1.0]);step(world);
        assert!(!idle(world));assert!(b3_body_get_position(body)[1]<3.0);
        settle(world);
        b3_body_apply_force_to_center(body,[0.0,120000.0,0.0],true); // default density:1000kg/m³
        unsafe {crate::api::b3_world_cast_ray_closest(world,[0.0,5.0,0.0],[0.0,-10.0,0.0],crate::api::b3_default_query_filter());}
        step(world);assert!(!idle(world));assert!(b3_body_get_linear_velocity(body)[1]>0.1);
        settle(world);
        b3_body_set_awake(body,false);step(world);assert!(!idle(world));
        // The zero-active status above may still be pending when this wakes.
        b3_body_apply_linear_impulse_to_center(body,[0.0,1000.0,0.0],true);
        step(world);assert!(!idle(world));assert!(b3_body_get_linear_velocity(body)[1]>0.1);
        settle(world);
        let mut bd=b3_default_body_def();bd.body_type=BodyType::Dynamic;bd.position=[3.0,4.0,0.0];
        let added=b3_create_body(world,&bd);
        b3_create_hull_shape(added,&b3_default_shape_def(),&b3_make_box_hull(0.5,0.5,0.5));
        step(world);assert!(!idle(world));assert!(b3_body_get_position(added)[1]<4.0);
        b3_destroy_body(added);step(world);assert!(!idle(world));settle(world);
        b3_world_enable_sleeping(world,false);step(world);assert!(!idle(world));
        b3_destroy_world(world);
        if let Some(error)=pollster::block_on(gpu.device.pop_error_scope()) {panic!("{error}");}
    }
}

#[cfg(all(test, not(target_arch="wasm32")))]
mod demand_pose_staging_tests {
    use super::*;
    use crate::api::{b3_default_world_def,b3_default_body_def,b3_default_shape_def,b3_make_box_hull};

    fn fixture(gpu:GpuDevice, automatic:bool)->(WorldId,BodyId,BodyId) {
        let mut wd=b3_default_world_def();wd.enable_sleep=false;
        let world=b3_create_world(gpu,&wd);
        with_world_mut_no_sync(world,|w|{w.gpu_ccd_requested=true;w.gpu_resident_requested=true;w.component_tgs_requested=true;});
        b3_world_set_automatic_pose_snapshots(world,automatic);
        crate::scenes::create_ground(world,20.0);
        let mut bd=b3_default_body_def();bd.body_type=BodyType::Dynamic;bd.position=[-2.0,8.0,0.0];
        let first=b3_create_body(world,&bd);
        b3_create_sphere_shape(first,&b3_default_shape_def(),&Sphere{center:[0.25,0.4,0.0],radius:0.5});
        let hole=b3_create_body(world,&bd);
        bd.position=[2.0,10.0,0.0];let last=b3_create_body(world,&bd);
        b3_create_hull_shape(last,&b3_default_shape_def(),&b3_make_box_hull(0.5,0.5,0.5));
        b3_destroy_body(hole);
        (world,first,last)
    }
    fn step(w:WorldId) {b3_world_step_gpu(w,1.0/60.0,4);}
    fn position(w:WorldId,b:BodyId)->([f32;3],[f32;4]) {
        let result=b3_body_get_transform(b);
        assert_eq!(b3_world_pose_snapshot_step(w),b3_world_physics_step(w));
        result
    }
    #[test]
    fn demand_pose_staging_preserves_getters_mutations_and_growth() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        let run=|automatic| {
            let (w,a,b)=fixture(gpu.clone(),automatic);
            for _ in 0..600 {step(w);}
            assert_eq!(b3_world_pose_snapshot_kicks(w),if automatic {600}else{0});
            assert_eq!(b3_world_pose_snapshot_maps(w),0);
            assert_eq!(b3_world_pose_snapshot_epoch(w),600);
            let mut out=vec![position(w,a),position(w,b)];
            assert!(out[0].0[1].abs()<0.2 && (out[1].0[1]-0.5).abs()<0.02);
            let state=position(w,a);assert_eq!(state,position(w,a));
            assert!(!with_world_no_sync(w,|x|x.gpu_mirror_stale).unwrap());
            // No submitted step or logical epoch advance on pause/dt=0.
            b3_world_step_gpu(w,0.0,4);assert_eq!(b3_world_pose_snapshot_epoch(w),600);
            b3_body_set_transform(a,[3.0,6.0,1.0],[0.0,0.0,0.0,1.0]);
            b3_world_prepare_pose_snapshot(w);
            assert_eq!(b3_body_get_position(a),[3.0,6.0,1.0]);
            step(w);b3_world_prepare_pose_snapshot(w);out.push(position(w,a));
            assert!(out.last().unwrap().0[1]<6.0);
            let mut bd=b3_default_body_def();
            for i in 0..270 {bd.position=[1000.0+i as f32*4.0,0.0,0.0];let body=b3_create_body(w,&bd);b3_create_hull_shape(body,&b3_default_shape_def(),&b3_make_box_hull(0.1,0.1,0.1));}
            step(w);out.push(position(w,a));out.push(position(w,b));
            assert!(with_world_no_sync(w,|x|x.sim.as_ref().unwrap().caps.bodies).unwrap()>256);
            b3_destroy_body(b);bd.body_type=BodyType::Dynamic;bd.position=[-4.0,4.0,0.0];
            let reused=b3_create_body(w,&bd);b3_create_hull_shape(reused,&b3_default_shape_def(),&b3_make_box_hull(0.5,0.5,0.5));
            step(w);out.push(position(w,reused));out.push(position(w,a));
            if !automatic {assert_eq!(b3_world_pose_snapshot_kicks(w),0);}
            assert!(!pollster::block_on(b3_world_live_step_stats(w)).unwrap().capacity_loss());
            b3_destroy_world(w);out
        };
        let expected=run(true);let actual=run(false);
        for (a,b) in actual.iter().zip(&expected) {
            for (x,y) in a.0.iter().chain(&a.1).zip(b.0.iter().chain(&b.1)) {assert!(x.is_finite() && (x-y).abs()<1e-5,"{a:?} != {b:?}");}
        }
    }
    #[test]
    fn demand_pose_staging_policy_changes_do_not_replay_old_copies() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        let (w,a,_)=fixture(gpu.clone(),true);step(w);
        assert_eq!(b3_world_pose_snapshot_kicks(w),1);
        b3_world_set_automatic_pose_snapshots(w,false);
        for _ in 0..20 {step(w);}
        assert_eq!(b3_world_pose_snapshot_epoch(w),21);
        assert_eq!(b3_world_pose_snapshot_kicks(w),1);
        let current=position(w,a);
        b3_world_set_automatic_pose_snapshots(w,true);
        b3_world_prepare_pose_snapshot(w);assert_eq!(b3_body_get_transform(a),current);
        step(w);assert_eq!(b3_world_pose_snapshot_epoch(w),22);assert_eq!(b3_world_pose_snapshot_kicks(w),2);
        b3_world_set_automatic_pose_snapshots(w,false);
        b3_body_set_transform(a,[0.0,5.0,0.0],[0.0,0.0,0.0,1.0]);
        b3_world_prepare_pose_snapshot(w);assert_eq!(b3_body_get_position(a),[0.0,5.0,0.0]);
        b3_destroy_world(w);
        let (fresh,a,_)=fixture(gpu,false);step(fresh);let p=position(fresh,a);
        assert!(p.0[1]>7.9);assert_eq!(b3_world_pose_snapshot_kicks(fresh),0);
        b3_destroy_world(fresh);
    }

    #[test]
    fn demand_pose_staging_preserves_completed_ccd_mirrors() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        for automatic in [true,false] {
            let (w,a,_)=fixture(gpu.clone(),automatic);
            b3_body_set_bullet(a,true);
            b3_body_set_transform(a,[0.0,0.4,0.0],[0.0,0.0,0.0,1.0]);
            b3_body_set_linear_velocity(a,[0.0,-80.0,0.0]);
            step(w);
            assert!(with_world_no_sync(w,|x|x.post_ccd_pending).unwrap());
            b3_world_gpu_wait_with_mirror(w);
            let corrected=position(w,a);
            assert!(corrected.0[1]>0.08,"CCD lost support: {corrected:?}");
            let maps=b3_world_pose_snapshot_maps(w);
            for _ in 0..4 {b3_world_prepare_pose_snapshot(w);assert_eq!(position(w,a),corrected);}
            assert_eq!(b3_world_pose_snapshot_maps(w),maps);
            if !automatic {assert_eq!(b3_world_pose_snapshot_kicks(w),0);}
            b3_destroy_world(w);
        }
    }

    #[test]
    fn demand_pose_staging_preserves_named_scene_states() {
        let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
        for (scene,count) in [(crate::types::DemoScene::MixedStacks,600),(crate::types::DemoScene::Dominoes,30)] {
            let run=|automatic| {
                let cfg=crate::types::DemoConfig{scene,body_count:count,body_count_explicit:true,contacts:true,jacobi:false};
                let w=crate::scenes::build_demo_world(gpu.clone(),&cfg);
                b3_world_set_automatic_pose_snapshots(w,automatic);
                b3_world_enable_sleeping(w,false);
                with_world_mut_no_sync(w,|x|{x.gpu_ccd_requested=true;x.gpu_resident_requested=true;x.component_tgs_requested=true;});
                for _ in 0..120 {step(w);}
                assert_eq!(b3_world_pose_snapshot_kicks(w),if automatic {120}else{0});
                let states=pollster::block_on(b3_world_sync_from_gpu(w));
                assert!(!pollster::block_on(b3_world_live_step_stats(w)).unwrap().capacity_loss());
                b3_destroy_world(w);states
            };
            let expected=run(true);let actual=run(false);assert_eq!(actual.len(),expected.len());
            for (i,(a,b)) in actual.iter().zip(&expected).enumerate() {
                assert_eq!(a.flags,b.flags);
                for (x,y) in a.pos.iter().chain(&a.rot).chain(&a.vel).chain(&a.omega).zip(b.pos.iter().chain(&b.rot).chain(&b.vel).chain(&b.omega)) {
                    assert!(x.is_finite() && (x-y).abs()<1e-5,"{scene:?} body {i}: {x} != {y}");
                }
            }
        }
    }
}


pub fn b3_world_set_contact_tuning(id: WorldId, hertz: f32, damping: f32, speed: f32) {
    if [hertz, damping, speed].iter().any(|v| v.is_nan()) { return; }
    with_world_mut_no_sync(id, |w| {
        w.def.contact_hertz = hertz.clamp(0.0, f32::MAX);
        w.def.contact_damping_ratio = damping.clamp(0.0, f32::MAX);
        w.def.contact_speed = speed.clamp(0.0, f32::MAX);
    });
}
pub fn b3_world_set_user_data(id: WorldId, value: usize) {
    with_world_mut_no_sync(id, |w| w.user_data = value);
}
pub fn b3_world_get_user_data(id: WorldId) -> usize {
    with_world_no_sync(id, |w| w.user_data).unwrap_or(0)
}
pub fn b3_world_get_awake_body_count(id: WorldId) -> i32 {
    with_world(id, |w| w.bodies.iter().flatten().filter(|b|
        b.gpu.flags & (FLAG_STATIC | FLAG_DISABLED | FLAG_SLEEP) == 0).count() as i32).unwrap_or(0)
}
pub fn b3_body_set_name(id: BodyId, name: Option<&std::ffi::CStr>) {
    with_world_mut_no_sync(world_id_from_body(id), |w| {
        if let Some(b) = body_mut(w, id) { b.name = name.map(ToOwned::to_owned); }
    });
}
pub fn b3_body_get_name(id: BodyId) -> *const std::ffi::c_char {
    with_world_no_sync(world_id_from_body(id), |w| body_ref(w, id)
        .and_then(|b| b.name.as_ref()).map_or(std::ptr::null(), |n| n.as_ptr()))
        .unwrap_or(std::ptr::null())
}
pub fn b3_body_enable_sleep(id: BodyId, enable: bool) {
    with_world_mut(world_id_from_body(id), |w| {
        let epoch = snapshot_epoch(w);
        if let Some(b) = body_mut(w, id) {
            if enable { b.gpu.flags |= FLAG_SLEEP_ENABLED; }
            else { b.gpu.flags &= !(FLAG_SLEEP_ENABLED | FLAG_SLEEP); b.gpu.sleep_time = 0.0; }
            b.host_epoch = epoch.saturating_add(1);
            mark_scene_dirty(w);
        }
    });
}
pub fn b3_body_is_sleep_enabled(id: BodyId) -> bool {
    with_world_no_sync(world_id_from_body(id), |w| body_ref(w, id)
        .is_some_and(|b| b.gpu.flags & FLAG_SLEEP_ENABLED != 0)).unwrap_or(false)
}
pub fn b3_body_set_sleep_threshold(id: BodyId, value: f32) {
    if !value.is_finite() || value < 0.0 { return; }
    with_world_mut(world_id_from_body(id), |w| {
        if let Some(b) = body_mut(w, id) { b.sleep_threshold = value; mark_scene_dirty(w); }
    });
}
pub fn b3_body_get_sleep_threshold(id: BodyId) -> f32 {
    with_world_no_sync(world_id_from_body(id), |w| body_ref(w, id)
        .map_or(0.0, |b| b.sleep_threshold)).unwrap_or(0.0)
}
pub fn b3_body_enable_hit_events(id: BodyId, enable: bool) {
    with_world_mut(world_id_from_body(id), |w| {
        if body_ref(w, id).is_none() { return; }
        for shape in w.shapes.iter_mut().flatten().filter(|s| s.body_index == id.index1) {
            if enable { shape.event_flags |= SHAPE_ENABLE_HIT_EVENTS; }
            else { shape.event_flags &= !SHAPE_ENABLE_HIT_EVENTS; }
        }
        mark_scene_dirty(w);
    });
}

#[cfg(test)]
mod api_completion_tests {
    use super::*;
    use crate::api::*;

    #[test]
    fn body_settings_affect_own_gpu_state_and_survive_rebuilds() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        let mut wd = b3_default_world_def();
        wd.gravity = [0.0; 3];
        let world = b3_create_world(gpu, &wd);
        b3_world_set_user_data(world, 12345);
        assert_eq!(b3_world_get_user_data(world), 12345);
        b3_world_set_contact_tuning(world, 12.0, 0.75, 2.0);
        with_world_no_sync(world, |w| {
            assert_eq!((w.def.contact_hertz, w.def.contact_damping_ratio, w.def.contact_speed), (12.0, 0.75, 2.0));
        });
        let mut bd = b3_default_body_def();
        bd.body_type = BodyType::Dynamic;
        bd.linear_velocity = [0.02, 0.0, 0.0];
        let a = b3_create_body(world, &bd);
        bd.position = [10.0, 0.0, 0.0];
        let b = b3_create_body(world, &bd);
        let sa = b3_create_sphere_shape(a, &b3_default_shape_def(), &Sphere { center: [0.0; 3], radius: 0.5 });
        let sb = b3_create_sphere_shape(b, &b3_default_shape_def(), &Sphere { center: [0.0; 3], radius: 0.5 });
        b3_body_set_sleep_threshold(a, 0.0);
        b3_body_set_sleep_threshold(b, 0.05);
        b3_body_enable_hit_events(a, true);
        assert!(b3_shape_are_hit_events_enabled(sa));
        assert!(!b3_shape_are_hit_events_enabled(sb));
        let name = std::ffi::CString::new("owned name").unwrap();
        b3_body_set_name(a, Some(&name));
        drop(name);
        assert_eq!(unsafe { std::ffi::CStr::from_ptr(b3_body_get_name(a)) }.to_bytes(), b"owned name");
        for _ in 0..45 { b3_world_step(world, 1.0/60.0, 4); }
        pollster::block_on(b3_world_sync_from_gpu(world));
        assert!(b3_body_is_awake(a), "zero-threshold moving body must stay awake");
        assert!(!b3_body_is_awake(b), "quiet body must sleep using its own threshold");
        assert_eq!(b3_world_get_awake_body_count(world), 1);
        b3_body_enable_sleep(b, false);
        assert!(!b3_body_is_sleep_enabled(b));
        assert!(b3_body_is_awake(b));
        for _ in 0..45 { b3_world_step(world, 1.0/60.0, 4); }
        pollster::block_on(b3_world_sync_from_gpu(world));
        assert!(b3_body_is_awake(b), "sleep-disabled body must remain awake");
        assert_eq!(b3_body_get_sleep_threshold(a), 0.0);
        b3_destroy_body(a);
        let reused = b3_create_body(world, &bd);
        assert!(b3_body_get_name(reused).is_null());
        assert_eq!(b3_body_get_sleep_threshold(reused), 0.05);
        b3_destroy_world(world);
    }
}

/// Local frames are relative to body origins, never the current centers of mass.
pub fn b3_joint_set_local_frame(id: JointId, second: bool, position: [f32; 3], rotation: [f32; 4]) {
    if position.iter().chain(rotation.iter()).any(|v| !v.is_finite())
        || (rotation.iter().map(|v| v*v).sum::<f32>() - 1.0).abs() > 0.001 { return; }
    with_world_mut(WorldId { index1: id.world0, generation: 1 }, |w| {
        let Some(index) = id.index1.checked_sub(1).map(|v| v as usize) else { return; };
        let Some(meta) = w.joint_meta.get(index).and_then(Option::as_ref) else { return; };
        if meta.generation != id.generation { return; }
        let Some(joint) = w.joints.get_mut(index) else { return; };
        if second { joint.anchor_b = position; joint.frame_b_rotation = rotation; }
        else {
            joint.anchor_a = position; joint.frame_a_rotation = rotation;
            if joint.kind == JOINT_PRISMATIC { joint.axis = quat_rotate(rotation, [1.0, 0.0, 0.0]); }
        }
        mark_scene_dirty(w);
    });
}
pub fn b3_joint_get_local_frame(id: JointId, second: bool) -> ([f32; 3], [f32; 4]) {
    with_joint_metadata(id, |_, j| if second { (j.anchor_b, j.frame_b_rotation) }
        else { (j.anchor_a, j.frame_a_rotation) }).unwrap_or(([0.0; 3], [0.0, 0.0, 0.0, 1.0]))
}
pub fn b3_joint_wake_bodies(id: JointId) {
    let a = b3_joint_get_body(id, false);
    let b = b3_joint_get_body(id, true);
    if a.index1 > 0 { b3_body_set_awake(a, true); }
    if b.index1 > 0 { b3_body_set_awake(b, true); }
}

pub fn b3_shape_compute_mass_data(id: ShapeId) -> MassData {
    with_world_no_sync(WorldId { index1: id.world0, generation: 1 }, |w| {
        let shape = w.shapes.get(id.index1.checked_sub(1)? as usize)?.as_ref()?;
        if shape.generation != id.generation || !matches!(shape.public_kind, KIND_SPHERE | KIND_CAPSULE | KIND_BOX | KIND_CONVEX_HULL) {
            return None;
        }
        let i = shape.local_inertia;
        Some(MassData { mass: shape.mass, center: shape.local_center,
            inertia: [[i[0],i[3],i[4]], [i[3],i[1],i[5]], [i[4],i[5],i[2]]] })
    }).flatten().unwrap_or_default()
}

#[cfg(all(feature = "replay-diagnostics", not(target_arch = "wasm32")))]
pub(crate) fn seed_drag_snapshot(
    id: WorldId,
    s: &crate::drag_replay::Snapshot,
) -> Result<(), String> {
    b3_world_gpu_wait_with_mirror(id);
    let mut worlds = lock_worlds();
    let w = slot_mut(&mut worlds, id).ok_or("world missing")?;
    let sim = w.sim.as_mut().ok_or("sim missing")?;
    let mut bodies = pollster::block_on(sim.read_bodies());
    let mut joints = pollster::block_on(sim.read_joints());
    if w.bodies.iter().flatten().count() != s.bodies.len() {
        return Err("body count mismatch".into());
    }
    let mut seen = std::collections::HashSet::new();
    for b in &s.bodies {
        if !seen.insert(b.id) {
            return Err("duplicate body".into());
        }
        let old = bodies.get_mut(b.id).ok_or("body slot missing")?;
        let host = w
            .bodies
            .get(b.id)
            .and_then(Option::as_ref)
            .ok_or("host body slot missing")?;
        if host.local_center != [0.0; 3]
            || old.inv_mass != b.inv_mass
            || old.inv_inertia != b.inv_inertia
        {
            return Err(format!(
                "mass/COM mismatch at {}: {:?} vs {:?}",
                b.id, old.inv_inertia, b.inv_inertia
            ));
        }
        old.pos = b.p;
        old.rot = b.q;
        old.vel = b.v;
        old.omega = b.w;
        old.dp = [0.0; 3];
        old.dq = [0.0, 0.0, 0.0, 1.0];
        if b.awake != 0 {
            old.flags &= !FLAG_SLEEP;
        } else {
            old.flags |= FLAG_SLEEP;
        }
    }
    if let Some(r) = &s.joint {
        let j = joints.get_mut(r.slot).ok_or("joint slot missing")?;
        if j.kind != crate::types::JOINT_MOTOR || j.a != r.a || j.b != r.b {
            return Err("motor identity mismatch".into());
        }
        j.anchor_a = r.anchor_a;
        j.anchor_b = r.anchor_b;
        j.frame_a_rotation = r.frame_a;
        j.frame_b_rotation = r.frame_b;
        j.axis = r.linear_velocity;
        j.motor_angular_velocity = r.angular_velocity;
        j.hertz = r.tuning[0];
        j.damping = r.tuning[1];
        j.upper_translation = r.tuning[2];
        j.target_translation = r.tuning[3];
        j.spring_hertz = r.tuning[4];
        j.spring_damping = r.tuning[5];
        j.max_motor_force = r.tuning[6];
        j.lower_translation = r.tuning[7];
        j.impulse = r.lv[0];
        j.perp_impulse = [r.lv[1], r.lv[2]];
        j.spring_impulse = r.ls[0];
        j.lower_impulse = r.ls[1];
        j.upper_impulse = r.ls[2];
        j.angular_impulse = r.av;
        j.motor_impulse = r.angular_spring[0];
        j._pad2 = [r.angular_spring[1], r.angular_spring[2]];
    } else if joints.iter().any(|j| j.kind != 0) {
        return Err("snapshot has no joint but GPU does".into());
    }
    let mut writable_contacts = std::collections::HashSet::new();
    for contact in &s.contacts {
        let a = bodies.get(contact.a as usize).ok_or("contact A missing")?;
        let b = bodies.get(contact.b as usize).ok_or("contact B missing")?;
        if !seen.contains(&(contact.a as usize))
            || !seen.contains(&(contact.b as usize))
            || a.inv_mass != 0.0
            || b.inv_mass <= 0.0
            || !writable_contacts.insert(contact.b)
        {
            return Err("replay requires independent static/dynamic contacts".into());
        }
    }
    sim.seed_drag_contacts(&s.contacts)?;
    sim.write_body_states(&bodies);
    sim.write_joints(&joints);
    let read = pollster::block_on(sim.read_bodies());
    for b in &s.bodies {
        let actual = &read[b.id];
        let expected = &bodies[b.id];
        if actual.pos != expected.pos
            || actual.rot != expected.rot
            || actual.vel != expected.vel
            || actual.omega != expected.omega
            || actual.flags != expected.flags
        {
            return Err("body readback mismatch".into());
        }
    }
    let read = pollster::block_on(sim.read_joints());
    if bytemuck::cast_slice::<_, u8>(&read) != bytemuck::cast_slice::<_, u8>(&joints) {
        return Err("joint readback mismatch".into());
    }
    eprintln!("drag-snapshot body and joint readback verified");
    for b in &s.bodies {
        w.bodies[b.id].as_mut().unwrap().gpu = bodies[b.id];
    }
    w.joints = joints;
    eprintln!(
        "drag-snapshot seeded {} bodies, {} cached contacts, motor {:?}",
        s.bodies.len(),
        s.contacts.len(),
        s.joint.as_ref().map(|j| j.slot)
    );
    Ok(())
}

#[cfg(test)]
mod host_rotation_rounding_tests {
    use super::quat_rotate;

    #[test]
    fn rounded_unit_quaternion_preserves_its_rotation_axis() {
        // This is an ordinary float32 unit quaternion, not an intentionally
        // scaled quaternion. The homogeneous formula incorrectly scales v by
        // the rounded squared norm; Box3D's cross-product form preserves it.
        let h = std::f32::consts::FRAC_1_SQRT_2;
        for axis in 0..3 {
            let mut q = [0.0, 0.0, 0.0, h];
            q[axis] = h;
            let mut v = [0.0; 3];
            v[axis] = 2.0;
            assert_eq!(quat_rotate(q, v), v);
            q[axis] = -h;
            assert_eq!(quat_rotate(q, v), v);
        }
    }
}

#[cfg(all(test,not(target_arch="wasm32")))]
mod resident_force_cache_probe {
use super::*;use crate::api::*;
fn check(world:WorldId) {
 with_world_no_sync(world,|w| {
  assert_eq!(scene_capabilities(w).pending_forces,w.bodies.iter().flatten().any(|b|b.force!=[0.0;3] || b.torque!=[0.0;3]));
  let old=w.gpu_ccd_requested && w.gpu_resident_requested && w.def.enable_continuous && !w.post_ccd_pending
   && !w.scene_dirty && !w.bodies_dirty && w.bodies.iter().flatten().all(|b|b.force==[0.0;3] && b.torque==[0.0;3])
   && !w.joints.iter().any(|j|j.kind!=JOINT_NONE) && w.custom_filter_callback.is_none() && w.pre_solve_callback.is_none()
   && !world_needs_contact_readback(w) && w.sim.as_ref().is_some_and(|sim| sim.uses_convex_ccd() && !sim.has_step_forces());
  assert_eq!(can_submit_resident(w),old);
 });
}
#[test]
fn resident_force_cache_tracks_consumption_and_flushes() {
 let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
 let mut wd=b3_default_world_def();wd.gravity=[0.0;3];let world=b3_create_world(gpu.clone(),&wd);
 with_world_mut_no_sync(world,|w|{w.gpu_ccd_requested=true;w.gpu_resident_requested=true;});
 let mut bd=b3_default_body_def();bd.body_type=BodyType::Dynamic;bd.position=[0.0,5.0,0.0];
 let body=b3_create_body(world,&bd);let shape=b3_create_hull_shape(body,&b3_default_shape_def(),&b3_make_box_hull(0.5,0.5,0.5));
 b3_world_step_gpu(world,1.0/60.0,4);check(world);assert!(with_world_no_sync(world,can_submit_resident).unwrap());
 for kind in 0..5 {
  match kind {
   0=>b3_body_apply_force_to_center(body,[3.0,0.0,0.0],true),
   1=>b3_body_apply_force(body,[0.0,3.0,0.0],[1.0,5.0,0.0],true),
   2=>b3_body_apply_torque(body,[0.0,0.0,3.0],true),
   3=>b3_shape_apply_wind(shape,[10.0,0.0,0.0],1.0,0.0,100.0,true),
   _=>{b3_body_apply_force_to_center(body,[3.0,0.0,0.0],true);b3_body_set_type(body,BodyType::Static);},
  }
  check(world);assert!(with_world_no_sync(world,|w|scene_capabilities(w).pending_forces).unwrap());
  b3_world_ensure_gpu(world);check(world);assert!(!with_world_no_sync(world,can_submit_resident).unwrap());
  b3_world_step_gpu(world,0.0,4);check(world);assert!(with_world_no_sync(world,|w|scene_capabilities(w).pending_forces).unwrap());
  b3_world_step_gpu(world,1.0/60.0,4);check(world);
  assert!(!with_world_no_sync(world,|w|scene_capabilities(w).pending_forces).unwrap());
  b3_world_finalize_render_state(world);check(world);
  if with_world_no_sync(world,|w|w.sim.as_ref().unwrap().has_step_forces()).unwrap() {
   assert!(!with_world_no_sync(world,can_submit_resident).unwrap(), "resident submission must not replay the previous load");
   b3_world_step_gpu(world,0.0,4);
   assert!(with_world_no_sync(world,|w|w.sim.as_ref().unwrap().has_step_forces()).unwrap());
   b3_world_step_gpu(world,1.0/60.0,4);check(world);
   assert!(!with_world_no_sync(world,|w|w.sim.as_ref().unwrap().has_step_forces()).unwrap());
   assert!(with_world_no_sync(world,can_submit_resident).unwrap(), "cleared load must permit resident reentry");
  }
 }
 b3_destroy_body(body);check(world);b3_world_ensure_gpu(world);check(world);
 b3_destroy_world(world);if let Some(error)=pollster::block_on(gpu.device.pop_error_scope()) {panic!("{error}");}
}
}

#[cfg(all(test, feature = "native-command-cache", not(target_arch = "wasm32")))]
mod full_replay_long_probe {
use super::*;
use crate::api::*;
#[test]
fn full_physics_replay_long_state_and_reentry() { run_long_reentry(true, crate::types::DemoScene::MixedStacks, 600); }
#[test]
fn global_physics_replay_long_state_and_reentry() { run_long_reentry(false, crate::types::DemoScene::MixedStacks, 600); }
#[test]
fn global_physics_replay_connected_pile_state_and_reentry() {
 run_long_reentry(false, crate::types::DemoScene::FallingCubes, 1000);
}
fn run_long_reentry(component: bool, scene: crate::types::DemoScene, count: u32) {
 let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
 gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
 let run=|enabled| {
  let cfg=crate::types::DemoConfig{scene,body_count:count,body_count_explicit:true,contacts:true,jacobi:false};
  let world=crate::scenes::build_demo_world(gpu.clone(),&cfg);
  with_world_mut_no_sync(world,|w|{w.component_tgs_requested=component;w.gpu_idle_requested=false;});
  b3_world_set_diagnostic_flags(world,crate::types::DIAG_BOUNDED_STATIC_SORT);
  b3_world_enable_sleeping(world,false);
  let ids=b3_world_dynamic_body_ids(world);assert_eq!(ids.len(),count as usize);
  let chosen=ids[count as usize-1];let initial=b3_body_get_position(chosen);
  let mut states=Vec::new();let mut hits=Vec::new();
  for step in 0..1000 {
   match step {
    250=>b3_body_set_transform(chosen,[initial[0],initial[1]+0.015,initial[2]],[0.0,0.0,0.0,1.0]),
    350=>b3_body_set_type(chosen,BodyType::Kinematic),
    400=>b3_body_set_type(chosen,BodyType::Dynamic),
    450=>b3_world_begin_timing(world,5),
    550 | 560 if !component => {
      with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().set_color_wave_prefix(if step==550 {0} else {20}));
    },
    700=>b3_world_enable_sleeping(world,true),
    750=>b3_body_set_awake(chosen,true),
    800=>b3_world_enable_sleeping(world,false),
    900=>{
        let before=with_world_no_sync(world,|w|w.sim.as_ref().unwrap().caps.bodies).unwrap();
        let mut bd=b3_default_body_def();bd.body_type=BodyType::Dynamic;
        let sd=b3_default_shape_def();let hull=b3_make_box_hull(0.5,0.5,0.5);
        for i in 0..500 {bd.position=[100.0+(i%50) as f32*1.5,4.0,100.0+(i/50) as f32*1.5];let body=b3_create_body(world,&bd);b3_create_hull_shape(body,&sd,&hull);}
        b3_world_ensure_gpu(world);
        assert!(with_world_no_sync(world,|w|w.sim.as_ref().unwrap().caps.bodies).unwrap()>before,"real capacity growth");
    },
    _=>{}
   }
   b3_world_ensure_gpu(world);
   with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().set_full_replay_test(enabled));
   b3_world_step_gpu(world,1.0/60.0,if (500..510).contains(&step){2}else{4});
   if (step+1)%25==0 {states.push(pollster::block_on(b3_world_sync_from_gpu(world)));}
   if (step+1)%100==0 {
    let h=with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().physics_replay_hits()).unwrap();
    hits.push(h);
   }
  }
  eprintln!("FULL_REPLAY_LONG enabled={enabled} hits={hits:?}");
  if enabled {assert!(hits[1]>=190,"must repeatedly reuse before mutation");assert!(hits[5]>=hits[3]+150,"must resume sustained reuse after type changes");assert!(hits[9]>80,"must reuse after capacity growth");}
  assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
  b3_destroy_world(world);states
 };
 let expected=run(false);let actual=run(true);assert_eq!(expected.len(),actual.len());
 let mut worst=0.0f32;
 for (checkpoint,(a,b)) in actual.iter().zip(&expected).enumerate(){
  assert_eq!(a.len(),b.len());
  for (i,(x,y)) in a.iter().zip(b).enumerate(){
   assert_eq!(x.flags,y.flags,"checkpoint {checkpoint} body {i} flags");
   for(v,w) in x.pos.iter().chain(&x.rot).chain(&x.vel).chain(&x.omega).zip(y.pos.iter().chain(&y.rot).chain(&y.vel).chain(&y.omega)){
    assert!(v.is_finite()&&w.is_finite());worst=worst.max((v-w).abs());
    assert!((v-w).abs()<1e-5,"checkpoint {checkpoint} body {i} {v} != {w}");
   }
  }
 }
 eprintln!("FULL_REPLAY_LONG checkpoints={} worst={worst}",actual.len());
 if let Some(e)=pollster::block_on(gpu.device.pop_error_scope()){panic!("{e}");}
}
}

#[cfg(all(test, feature = "native-command-cache", not(target_arch = "wasm32")))]
mod full_replay_dominoes_probe {
 use super::*;
 #[test]
 fn full_physics_replay_dominoes_full_state() {
  let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();
  gpu.device.push_error_scope(wgpu::ErrorFilter::Validation);
  let run=|enabled| {
   let cfg=crate::types::DemoConfig{scene:crate::types::DemoScene::Dominoes,body_count:30,body_count_explicit:true,contacts:true,jacobi:false};
   let world=crate::scenes::build_demo_world(gpu.clone(),&cfg);
   with_world_mut_no_sync(world,|w|{w.component_tgs_requested=true;w.gpu_idle_requested=false;});
   b3_world_set_diagnostic_flags(world,crate::types::DIAG_BOUNDED_STATIC_SORT);
   b3_world_enable_sleeping(world,false);
   b3_world_set_automatic_pose_snapshots(world,false);
   b3_world_ensure_gpu(world);
   with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().set_full_replay_test(enabled));
   let mut states=Vec::new();
   for step in 1..=1200 {
    b3_world_step_gpu(world,1.0/60.0,4);
    if step%60==0 {let bodies=pollster::block_on(b3_world_sync_from_gpu(world));assert_eq!(bodies.len(),5431);states.push(bodies);}
   }
   let hits=with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().physics_replay_hits()).unwrap();
   eprintln!("DOMINOES_REPLAY enabled={enabled} hits={hits}");
   if enabled {assert!(hits>=1100,"replay must run during the target window");} else {assert_eq!(hits,0);}
   assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
   b3_destroy_world(world);states
  };
  let reference=run(false);let replay=run(true);assert_eq!(reference.len(),20);assert_eq!(replay.len(),20);
  let mut worst=0.0f32;
  for (frame,(a,b)) in replay.iter().zip(&reference).enumerate() {
   for (body,(x,y)) in a.iter().zip(b).enumerate() {
    assert_eq!(x.flags,y.flags,"frame {frame} body {body} flags");
    for (v,w) in x.pos.iter().chain(&x.rot).chain(&x.vel).chain(&x.omega).zip(y.pos.iter().chain(&y.rot).chain(&y.vel).chain(&y.omega)) {
     assert!(v.is_finite()&&w.is_finite());worst=worst.max((v-w).abs());
     assert!((v-w).abs()<1e-5,"frame {frame} body {body}: {v} != {w}");
    }
   }
  }
  eprintln!("DOMINOES_REPLAY checkpoints=20 worst={worst}");
  if let Some(e)=pollster::block_on(gpu.device.pop_error_scope()){panic!("{e}");}
 }
}
