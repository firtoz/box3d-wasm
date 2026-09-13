use std::cell::Cell;
use std::mem;
use std::sync::atomic::{AtomicU64, Ordering};
#[cfg(not(target_arch = "wasm32"))]
use std::time::Instant;

use bytemuck::Zeroable;
use wgpu::{
    util::DeviceExt, Adapter, Backends, BindGroup, BindGroupLayout, Buffer, ComputePipeline,
    Device, Instance, InstanceDescriptor, Queue, ShaderModule, Surface,
};

use crate::adapter::{pick_adapter, AdapterReport};
#[cfg(not(target_arch = "wasm32"))]
use crate::types::BroadphaseStats;
use crate::types::{
    BodyColdGpu, BodyGpu, BodyStateGpu, ContactGpu, ContactHotGpu, ContactPersistentGpu,
    ContactPreparedGpu, JointGpu, MixPairGpu, ShapeGpu, SimParams, SurfaceMaterialGpu, DIAG_DISABLE_RECYCLING,
    DIAG_DISABLE_ROLLING, DIAG_DISABLE_SAT_CACHE, DIAG_DISABLE_SLEEP, DIAG_FORCE_CAPACITY_LOSS,
    DIAG_FORCE_GENERAL_STATIC_SORT, DIAG_GENERAL_SOLVER, DIAG_PHASE_CAPTURE, DIAG_REBUILD_GRAPH,
    DIAG_STATIC_DEGREE_ONE_PROOF, GpuSceneCaps, MAX_COLORS,
    PASS_BIAS_ROWS, WORKGROUP_SIZE,
};

#[cfg(not(target_arch = "wasm32"))]
fn diagnostic_flags_from_env() -> u32 {
    std::env::var("GPU_PHYSICS_AB")
        .unwrap_or_default()
        .split(',')
        .fold(0, |flags, name| {
            flags
                | match name.trim() {
                    "no-recycle" => DIAG_DISABLE_RECYCLING,
                    "no-sat-cache" => DIAG_DISABLE_SAT_CACHE,
                    "no-rolling" => DIAG_DISABLE_ROLLING,
                    "rebuild-graph" => DIAG_REBUILD_GRAPH,
                    "no-sleep" => DIAG_DISABLE_SLEEP,
                    "phase-capture" => DIAG_PHASE_CAPTURE,
                    "force-drop" => DIAG_FORCE_CAPACITY_LOSS,
                    "general-solver" => DIAG_GENERAL_SOLVER,
                    "general-static-sort" => DIAG_FORCE_GENERAL_STATIC_SORT,
                    "bounded-static-sort" => crate::types::DIAG_BOUNDED_STATIC_SORT,
                    "joint-filter-scan" => crate::types::DIAG_JOINT_FILTER_SCAN,
                    "serial-joints" => crate::types::DIAG_SERIAL_JOINTS,
                    "parallel-joints" => crate::types::DIAG_PARALLEL_JOINTS,
                    "mesh-candidates" => crate::types::DIAG_MESH_CANDIDATES,
                    _ => 0,
                }
        })
}

#[cfg(target_arch = "wasm32")]
fn diagnostic_flags_from_env() -> u32 {
    0
}

/// One WGSL module, split on disk under `shaders/physics/`.
const PHYSICS_WGSL: &str = concat!(
    include_str!("../shaders/physics/types.wgsl"),
    include_str!("../shaders/physics/math.wgsl"),
    include_str!("../shaders/physics/hull.wgsl"),
    include_str!("../shaders/physics/collide.wgsl"),
    include_str!("../shaders/physics/broadphase.wgsl"),
    include_str!("../shaders/physics/solve.wgsl"),
    include_str!("../shaders/physics/integrate.wgsl"),
    include_str!("../shaders/physics/query.wgsl"),
);

pub struct GpuDevice {
    pub instance: Instance,
    pub adapter: Adapter,
    pub device: Device,
    pub queue: Queue,
    pub report: AdapterReport,
    pub timestamp_queries: bool,
    pub subgroups: bool,
    #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
    pub(crate) secondary_queue: Option<std::sync::Arc<crate::native_async::SecondaryQueue>>,
}

impl GpuDevice {
    pub fn instance_new() -> Instance {
        Instance::new(&InstanceDescriptor {
            backends: native_or_web_backends(),
            #[cfg(feature = "native-command-cache")]
            flags: if std::env::var_os("GPU_PHYSICS_NATIVE_VALIDATE").is_some() {
                wgpu::InstanceFlags::debugging()
            } else { wgpu::InstanceFlags::default() },
            ..Default::default()
        })
    }

    pub async fn new(surface: Option<&Surface<'_>>) -> Result<Self, String> {
        Self::from_instance(Self::instance_new(), surface).await
    }

    pub async fn from_instance(
        instance: Instance,
        surface: Option<&Surface<'_>>,
    ) -> Result<Self, String> {
        Self::from_instance_mode(instance, surface, false).await
    }

    /// Experimental device setup only: stepping still uses the primary queue.
    #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
    pub async fn new_with_secondary_queue(surface: Option<&Surface<'_>>) -> Result<Self, String> {
        Self::from_instance_mode(Self::instance_new(), surface, true).await
    }

    pub(crate) async fn from_instance_mode(instance: Instance, surface: Option<&Surface<'_>>, secondary: bool) -> Result<Self, String> {
        let (adapter, report) = pick_adapter(&instance, surface).await?;
        log::info!("GPU adapter: {}", report.summary_line());
        eprintln!("GPU adapter: {}", report.summary_line());

        let limits = if cfg!(target_arch = "wasm32") {
            wgpu::Limits::downlevel_defaults().using_resolution(adapter.limits())
        } else {
            let supported = adapter.limits();
            wgpu::Limits {
                max_storage_buffer_binding_size: supported.max_storage_buffer_binding_size,
                max_buffer_size: supported.max_buffer_size,
                max_storage_buffers_per_shader_stage: supported
                    .max_storage_buffers_per_shader_stage
                    .max(10),
                max_compute_workgroup_storage_size: supported.max_compute_workgroup_storage_size.min(32768),
                max_compute_workgroup_size_x: supported.max_compute_workgroup_size_x,
                max_compute_invocations_per_workgroup: supported
                    .max_compute_invocations_per_workgroup,
                ..wgpu::Limits::default()
            }
        };
        let mut features = wgpu::Features::empty();
        let af = adapter.features();
        if af.contains(wgpu::Features::TIMESTAMP_QUERY) {
            features |= wgpu::Features::TIMESTAMP_QUERY;
        }
        if af.contains(wgpu::Features::TIMESTAMP_QUERY_INSIDE_ENCODERS) {
            features |= wgpu::Features::TIMESTAMP_QUERY_INSIDE_ENCODERS;
        }
        if af.contains(wgpu::Features::SUBGROUP) {
            features |= wgpu::Features::SUBGROUP;
        }
        #[cfg(not(target_arch = "wasm32"))]
        if std::env::var_os("GPU_PHYSICS_PIPELINE_CACHE_DIR").is_some() && af.contains(wgpu::Features::PIPELINE_CACHE) {
            features |= wgpu::Features::PIPELINE_CACHE;
        }
        let descriptor = wgpu::DeviceDescriptor {
            label: Some("gpu-physics-device"), required_features: features,
            required_limits: limits, memory_hints: wgpu::MemoryHints::MemoryUsage,
            trace: wgpu::Trace::Off,
        };
        #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
        let (device, queue, secondary_queue) = if secondary {
            let (device, queue, extra) = crate::native_async::open_device(&adapter, &descriptor)?;
            (device, queue, Some(extra))
        } else {
            let (device, queue) = adapter.request_device(&descriptor).await
                .map_err(|e| format!("request_device failed: {e}"))?;
            (device, queue, None)
        };
        #[cfg(not(all(feature = "native-command-cache", not(target_arch = "wasm32"))))]
        let (device, queue) = {
            debug_assert!(!secondary);
            adapter.request_device(&descriptor).await
                .map_err(|e| format!("request_device failed: {e}"))?
        };
        if features.contains(wgpu::Features::SUBGROUP) {
            eprintln!("GPU features: subgroups");
        }
        eprintln!(
            "GPU limits: storage bind {:.1} MiB, buffer {:.1} MiB",
            f64::from(device.limits().max_storage_buffer_binding_size) / (1024.0 * 1024.0),
            device.limits().max_buffer_size as f64 / (1024.0 * 1024.0),
        );

        device.on_uncaptured_error(Box::new(|err| {
            log::error!("uncaptured WebGPU error: {err:?}");
            eprintln!("uncaptured WebGPU error: {err}");
        }));

        Ok(Self {
            instance,
            adapter,
            device,
            queue,
            report,
            timestamp_queries: features.contains(wgpu::Features::TIMESTAMP_QUERY)
                && features.contains(wgpu::Features::TIMESTAMP_QUERY_INSIDE_ENCODERS),
            subgroups: features.contains(wgpu::Features::SUBGROUP),
            #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
            secondary_queue,
        })
    }
}

impl Clone for GpuDevice {
    fn clone(&self) -> Self {
        Self {
            instance: self.instance.clone(),
            adapter: self.adapter.clone(),
            device: self.device.clone(),
            queue: self.queue.clone(),
            report: self.report.clone(),
            timestamp_queries: self.timestamp_queries,
            subgroups: self.subgroups,
            #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
            secondary_queue: self.secondary_queue.clone(),
        }
    }
}

/// wgpu `PollType::Wait` uses a 60s fence timeout. A long unwaited queue (metrics
/// `spheres`) can exceed that; retry until the wait actually finishes.
pub fn poll_until_idle(device: &Device) {
    loop {
        match device.poll(wgpu::PollType::wait()) {
            Ok(status) if status.wait_finished() => return,
            Ok(_) | Err(wgpu::PollError::Timeout) => {}
        }
    }
}

fn native_or_web_backends() -> Backends {
    #[cfg(target_arch = "wasm32")]
    {
        Backends::BROWSER_WEBGPU
    }
    #[cfg(not(target_arch = "wasm32"))]
    {
        // The optional raw-command backend is Vulkan-specific. Do not select
        // Metal/DX12 and discover that only after submitting a cached pass.
        if cfg!(feature = "native-command-cache") {
            Backends::VULKAN
        } else {
            Backends::VULKAN | Backends::METAL | Backends::DX12
        }
    }
}

static CONTACT_EPOCH: AtomicU64 = AtomicU64::new(1);

fn pad_pod<T: Copy + Zeroable>(src: &[T], cap: usize) -> Result<Vec<T>, String> {
    if cap > src.len() {
        let extra = cap - src.len();
        let stride = core::mem::size_of::<T>();
        extra
            .checked_mul(stride)
            .ok_or_else(|| format!("pad overflow {} * {stride}", extra))?;
    }
    let mut v = src.to_vec();
    v.resize(cap.max(src.len()), T::zeroed());
    Ok(v)
}

pub fn pack_scene_bytes(
    caps: GpuSceneCaps,
    bodies: &[BodyGpu],
    body_local_centers: &[[f32; 3]],
    body_inv_inertia_offdiag: &[[f32; 3]],
    shapes: &[ShapeGpu],
    hull_points: &[[f32; 4]],
    hull_planes: &[[f32; 4]],
    hull_edges: &[[f32; 4]],
    hull_topology: &[[u32; 4]],
    mesh_vertices: &[[f32; 4]],
    mesh_triangles: &[[u32; 4]],
    mesh_nodes: &[[f32; 4]],
    surface_materials: &[SurfaceMaterialGpu],
    mix_pairs: &[MixPairGpu],
) -> Result<Vec<u8>, String> {
    let heap = caps.validate_allocation(0)?;
    let capacity = caps.bodies.max(bodies.len() as u32).max(1) as usize;
    let mut cold: Vec<_> = bodies
        .iter()
        .enumerate()
        .map(|(i, body)| {
            BodyColdGpu::from_body(body, body_local_centers.get(i).copied().unwrap_or([0.0; 3]))
        })
        .collect();
    cold.resize(capacity, BodyColdGpu::zeroed());
    let mut body_extra: Vec<[f32; 8]> = body_inv_inertia_offdiag
        .iter()
        .map(|v| [v[0], v[1], v[2], 0.0, 0.0, 0.0, 0.0, 0.0])
        .collect();
    body_extra.resize(capacity, [0.0; 8]);
    // Immutable per-body motion bounds, rebuilt only with the scene heap.
    // Inertia xyz, minimum extent, maximum extent xyz, padding. No new binding.
    for extra in &mut body_extra { extra[3] = f32::MAX; }
    let mut extents = vec![[0.0f32; 3]; capacity];
    for shape in shapes {
        let index = shape.body_index as usize;
        if index >= bodies.len() { continue; }
        let minimum = match shape.kind {
            crate::types::KIND_SPHERE | crate::types::KIND_CAPSULE => shape.half[0],
            crate::types::KIND_BOX => shape.half.into_iter().fold(f32::MAX, f32::min),
            crate::types::KIND_CONVEX_HULL => shape.inner_radius,
            _ => f32::MAX,
        };
        body_extra[index][3] = body_extra[index][3].min(minimum.max(crate::types::LINEAR_SLOP));
        let center = cold[index].local_center;
        let mut extent = [0.0f32; 3];
        if shape.kind == crate::types::KIND_CONVEX_HULL {
            let start = shape.hull_slot as usize;
            let end = start + (shape.topology_counts & 0xff) as usize;
            for point in hull_points.get(start..end).unwrap_or(&[]) {
                for axis in 0..3 { extent[axis] = extent[axis].max((point[axis] - center[axis]).abs()); }
            }
        } else {
            for axis in 0..3 {
                let radius = match shape.kind {
                    crate::types::KIND_SPHERE => shape.half[0],
                    crate::types::KIND_CAPSULE => shape.half[0] + shape.axis[axis].abs(),
                    _ => shape.half[axis],
                };
                extent[axis] = (shape.local_center[axis] - center[axis]).abs() + radius;
            }
        }
        for axis in 0..3 { extents[index][axis] = extents[index][axis].max(extent[axis]); }
    }
    // Keep motion bounds separate from cold.half: readback/rendering still uses it.
    for (extra, extent) in body_extra.iter_mut().zip(extents) { extra[4..7].copy_from_slice(&extent); }
    let padded_shapes = pad_pod(shapes, caps.shapes as usize)?;
    let padded_hull_points = pad_pod(hull_points, caps.hull_points as usize)?;
    let padded_hull_planes = pad_pod(hull_planes, caps.hull_planes as usize)?;
    let padded_hull_edges = pad_pod(hull_edges, caps.hull_edges as usize)?;
    let padded_hull_topology = pad_pod(hull_topology, caps.hull_topology as usize)?;
    let padded_mesh_vertices = pad_pod(mesh_vertices, caps.mesh_vertices as usize)?;
    let padded_mesh_triangles = pad_pod(mesh_triangles, caps.mesh_triangles as usize)?;
    let padded_mesh_nodes = pad_pod(mesh_nodes, (caps.mesh_nodes as usize).saturating_mul(2))?;
    let padded_materials = pad_pod(surface_materials, caps.materials as usize)?;
    let padded_mix = pad_pod(mix_pairs, crate::types::MIX_PAIR_CAP as usize)?;
    let mut scene_bytes = Vec::new();
    scene_bytes.try_reserve(heap as usize).map_err(|_| {
        format!("host reservation of {heap} scene bytes failed")
    })?;
    scene_bytes.extend_from_slice(bytemuck::cast_slice(&cold));
    scene_bytes.extend_from_slice(bytemuck::cast_slice(&body_extra));
    scene_bytes.extend_from_slice(bytemuck::cast_slice(&padded_shapes));
    scene_bytes.extend_from_slice(bytemuck::cast_slice(&padded_hull_points));
    scene_bytes.extend_from_slice(bytemuck::cast_slice(&padded_hull_planes));
    scene_bytes.extend_from_slice(bytemuck::cast_slice(&padded_hull_edges));
    scene_bytes.extend_from_slice(bytemuck::cast_slice(&padded_hull_topology));
    scene_bytes.extend_from_slice(bytemuck::cast_slice(&padded_mesh_vertices));
    scene_bytes.extend_from_slice(bytemuck::cast_slice(&padded_mesh_triangles));
    scene_bytes.extend_from_slice(bytemuck::cast_slice(&padded_mesh_nodes));
    scene_bytes.extend_from_slice(bytemuck::cast_slice(&padded_materials));
    scene_bytes.extend_from_slice(bytemuck::cast_slice(&padded_mix));
    Ok(scene_bytes)
}

fn decode_body_gpu(
    states: &[BodyStateGpu],
    cold: &[BodyColdGpu],
    islands: &[u32],
) -> Vec<BodyGpu> {
    states
        .iter()
        .zip(cold)
        .zip(islands)
        .map(|((state, cold), island_id)| BodyGpu {
            pos: state.pos,
            inv_mass: state.inv_mass,
            vel: state.vel,
            kind: cold.kind,
            half: cold.half,
            flags: state.flags,
            rot: state.rot,
            omega: state.omega,
            restitution: cold.restitution,
            inv_inertia: cold.inv_inertia,
            friction: cold.friction,
            gravity_scale: cold.gravity_scale,
            linear_damping: cold.linear_damping,
            angular_damping: cold.angular_damping,
            rolling: cold.rolling,
            dp: state.dp,
            sleep_time: state.sleep_time,
            dq: state.dq,
            island_id: *island_id,
            sleep_velocity: state.sleep_velocity,
            _pad_island: [0; 2],
        })
        .collect()
}

fn decode_contacts(
    hot: &[ContactHotGpu],
    persistent: &[ContactPersistentGpu],
    prepared: &[ContactPreparedGpu],
) -> Vec<ContactGpu> {
    hot.iter()
        .zip(persistent)
        .zip(prepared)
        .map(|((hot, persistent), prepared)| ContactGpu {
            a: hot.a,
            b: hot.b,
            color: hot.color,
            count: hot.count,
            nx: hot.n[0],
            ny: hot.n[1],
            nz: hot.n[2],
            friction: hot.friction,
            ra0: hot.ra[0],
            ra1: hot.ra[1],
            ra2: hot.ra[2],
            ra3: hot.ra[3],
            rb0: hot.rb[0],
            rb1: hot.rb[1],
            rb2: hot.rb[2],
            rb3: hot.rb[3],
            friction_impulse: hot.friction_impulse,
            twist_impulse: hot.twist_impulse,
            rolling: hot.rolling,
            center_a: hot.center_a,
            _pad_ca: hot._pad_ca,
            center_b: hot.center_b,
            _pad_cb: hot._pad_cb,
            rolling_impulse: hot.rolling_impulse,
            _pad_end: hot.restitution,
            tangent_velocity: hot.tangent_velocity,
            material_index: hot.material_index,
            manifold_link: hot.manifold_link,
            point_triangles: persistent.point_triangles,
            _tail: [
                prepared.relative_velocity[0],
                prepared.relative_velocity[1],
                prepared.relative_velocity[2],
                prepared.relative_velocity[3],
                persistent.feature_ids[0],
                persistent.feature_ids[1],
                persistent.feature_ids[2],
                persistent.feature_ids[3],
            ],
            cached_relative: persistent.cached_relative,
            cached_rotation_a: persistent.cached_rotation_a,
            cached_rotation_b: persistent.cached_rotation_b,
            lifecycle: persistent.lifecycle,
            prepared_normal_mass: prepared.normal_mass,
            prepared_lever_arm: prepared.lever_arm,
            total_normal_impulse: prepared.total_normal_impulse,
            prepared_tangent_inv: prepared.tangent_inv,
            prepared_softness: prepared.softness,
            persistent_ra0: persistent.persistent_ra[0],
            persistent_ra1: persistent.persistent_ra[1],
            persistent_ra2: persistent.persistent_ra[2],
            persistent_ra3: persistent.persistent_ra[3],
            persistent_rb0: persistent.persistent_rb[0],
            persistent_rb1: persistent.persistent_rb[1],
            persistent_rb2: persistent.persistent_rb[2],
            persistent_rb3: persistent.persistent_rb[3],
        })
        .collect()
}

// Only bound-affecting inputs invalidate a cached fat proxy. Material/filter
// uploads must not destroy its containment history. Full values avoid hash collisions.
#[derive(Clone, PartialEq)]
struct FatGeometryKey {
    source: u32,
    body: u32,
    body_type: u32,
    kind: u32,
    proxy_flags: u32,
    center: [f32; 3],
    half: [f32; 3],
    axis: [f32; 3],
    inner_radius: f32,
}
impl FatGeometryKey {
    fn new(shape: &ShapeGpu, bodies: &[BodyGpu]) -> Self {
        Self {
            source: shape._pad_filter[1], body: shape.body_index,
            body_type: bodies.get(shape.body_index as usize).map_or(0, |b| b.flags
                & (crate::types::FLAG_STATIC | crate::types::FLAG_KINEMATIC | crate::types::FLAG_DISABLED)),
            kind: shape.kind,
            proxy_flags: shape.event_flags & (crate::types::SHAPE_PUBLIC_PROXY | crate::types::SHAPE_COMPOUND_CHILD),
            center: shape.local_center, half: shape.half, axis: shape.axis, inner_radius: shape.inner_radius,
        }
    }
}

pub struct GpuSim {
    #[cfg(feature = "native-command-cache")]
    pub(crate) render_copy: Option<crate::native_async::RenderCopy>,
    startup_cache: crate::pipeline_cache::StartupCache,
    pub device: Device,
    pub queue: Queue,
    pub report: AdapterReport,
    pub count: u32,
    pub caps: GpuSceneCaps,
    pub contact_epoch: u64,
    params: SimParams,
    physics_step: u64,
    last_submit: Option<wgpu::SubmissionIndex>,
    completed_step: u64,
    completed_known: bool,
    pose_step: u64,
    last_mirror_wait_ms: f32,
    last_mirror_copy_ms: f32,
    last_mirror_map_ms: f32,
    last_mirror_bytes: u64,
    one_group_wave_only: bool,
    color_wave_prefix: u32,
    color_wave_prefix_override: Option<u32>,
    color_hint_min_step: u64,
    one_group_pair_ok: bool,
    skip_general_static_sort: bool,
    solver_dispatches: Cell<u32>,
    joint_dispatches: Cell<u32>,
    static_sort_dispatches: Cell<u32>,
    encode_commands: Cell<u32>,
    physics_invalid: bool,
    callback_open: bool,
    pass_lut: Buffer,
    bodies: Buffer,
    body_cold: Buffer,
    convex_ccd: Option<crate::ccd::ConvexCcd>,
    contacts: Buffer,
    contact_persistent: Buffer,
    contact_prepared: Buffer,
    #[allow(dead_code)]
    joints: Buffer,
    #[allow(dead_code)]
    scratch: Buffer,
    #[allow(dead_code)]
    atom: Buffer,
    query: Buffer,
    indirect: Buffer,
    #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
    radix_cache: Option<crate::native_command_cache::RadixCache>,
    #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
    contact_cache: Option<crate::native_command_cache::RadixCache>,
    #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
    graph_cache: Option<crate::native_command_cache::RadixCache>,
    #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
    tail_cache: [Option<crate::native_command_cache::RadixCache>;3],
    bind_group: BindGroup,
    bind_group_layout: BindGroupLayout,
    fat_geometry: Vec<FatGeometryKey>,
    pub shape_identities: Vec<(u32, u16)>,
    clear_remapped_contact_hash: ComputePipeline,
    remap_contact_shapes: ComputePipeline,
    publish_remapped_contacts: ComputePipeline,
    contact_slots: u32,
    shape_count: u32,
    #[cfg(not(target_arch = "wasm32"))]
    staging: Option<Buffer>,
    reset_deltas: ComputePipeline,
    island_init: ComputePipeline,
    island_union_edges: ComputePipeline,
    island_accumulate_wake: ComputePipeline,
    island_apply_wake: ComputePipeline,
    island_reset_ready: ComputePipeline,
    island_accumulate_ready: ComputePipeline,
    island_apply_sleep: ComputePipeline,
    apply_deltas: ComputePipeline,
    integrate_vel: ComputePipeline,
    clear_broadphase: ComputePipeline,
    pair_matrix: std::sync::OnceLock<[ComputePipeline;5]>,
    pair_matrix_flat: bool,
    pair_matrix_requested: bool,
    pair_matrix_used: bool,
    hash_insert: ComputePipeline,
    write_insert_indirect: ComputePipeline,
    write_radix_indirect: ComputePipeline,
    emit_hash_pairs: ComputePipeline,
    emit_static_pairs: ComputePipeline,
    collect_fat_statics: ComputePipeline,
    finish_fat_statics: ComputePipeline,
    write_occupied_indirect: ComputePipeline,
    emit_prev_pairs: ComputePipeline,
    radix_histogram: [ComputePipeline; 4],
    radix_bucket_bases: ComputePipeline,
    radix_group_prefix: ComputePipeline,
    radix_scatter: [ComputePipeline; 4],
    compact_unique_histogram: ComputePipeline,
    compact_unique_bases: ComputePipeline,
    compact_unique_scatter: ComputePipeline,
    compact_unique_gather: ComputePipeline,
    find_existing_contact_slots: ComputePipeline,
    alloc_free_histogram: ComputePipeline,
    alloc_free_bases: ComputePipeline,
    alloc_free_scatter: ComputePipeline,
    alloc_missing_histogram: ComputePipeline,
    alloc_missing_bases: ComputePipeline,
    alloc_missing_scatter: ComputePipeline,
    alloc_bind_slots: ComputePipeline,
    collide_pairs_no_mesh: std::sync::OnceLock<ComputePipeline>,
    collide_pairs_mesh: std::sync::OnceLock<ComputePipeline>,
    collision_shader: ShaderModule,
    collision_layout: wgpu::PipelineLayout,
    retire_stale_contacts: ComputePipeline,
    retire_body_pair_contacts: ComputePipeline,
    begin_occupied_contacts: ComputePipeline,
    collect_occupied_contacts: ComputePipeline,
    finish_occupied_contacts: ComputePipeline,
    write_unique_indirect: ComputePipeline,
    write_alloc_indirects: ComputePipeline,
    graph_clear_meta: ComputePipeline,
    graph_reset_colors: ComputePipeline,
    graph_classify: ComputePipeline,
    graph_mark_edges: ComputePipeline,
    graph_compact_paired: std::sync::OnceLock<[ComputePipeline;3]>,
    graph_count_static_degree: ComputePipeline,
    graph_finish_static_degree: ComputePipeline,
    graph_pack_static_keys: ComputePipeline,
    graph_radix_histogram: [ComputePipeline; 4],
    graph_radix_scatter: [ComputePipeline; 4],
    graph_mark_static_starts: ComputePipeline,
    graph_encode_static_colors: ComputePipeline,
    graph_mark_color_starts: ComputePipeline,
    graph_assign_static: ComputePipeline,
    graph_assign_dynamic: ComputePipeline,
    graph_shared_requested: bool,
    graph_memo_base: Option<u32>,
    graph_assign_memo: std::sync::OnceLock<ComputePipeline>,
    graph_assign_shared: std::sync::OnceLock<ComputePipeline>,
    color_and_compact: ComputePipeline,
    prepare_contacts: ComputePipeline,
    #[allow(dead_code)]
    warm_start: ComputePipeline,
    #[allow(dead_code)]
    solve_contacts: ComputePipeline,
    component_reset: ComputePipeline,
    component_count: ComputePipeline,
    component_offsets: ComputePipeline,
    component_color_offsets: ComputePipeline,
    component_scatter: ComputePipeline,
    solve_large_components: ComputePipeline,
    solve_complete_components: ComputePipeline,
    small_component_workgroup_size:u32,
    component_tgs: bool,
    count_active_bodies: ComputePipeline,
    idle_eligible: bool,
    idle_input_context: [u64;2],
    idle_output_context: [u64;2],
    idle_proof: Option<(u64,[u64;2],u64)>,
    // Contiguous eligible submissions: first step, last step, output context, epoch.
    // An all-sleeping world is invariant under these unmutated closed-world steps.
    idle_chain: Option<(u64,u64,[u64;2],u64)>,
    #[cfg(test)]
    hold_idle_status: bool,
    idle_count_valid_step: Option<u64>,
    sticky_pending_idle_context: Option<([u64;2],u64)>,
    last_step_idle: bool,
    idle_epoch: Cell<u64>,
    solve_color: ComputePipeline,
    solve_color_wave_one_group: ComputePipeline,
    solve_color_tail_one_group: ComputePipeline,
    #[allow(dead_code)]
    solve_overflow: ComputePipeline,
    solve_joints: ComputePipeline,
    compact_joint_heads: ComputePipeline,
    compact_joint_components: ComputePipeline,
    ray_closest: ComputePipeline,
    ray_closest_pick: ComputePipeline,
    ray_closest_commit: ComputePipeline,
    #[allow(dead_code)]
    solve_tiny_islands: ComputePipeline,
    integrate_pos: ComputePipeline,
    jacobi_clear: ComputePipeline,
    solve_jacobi: ComputePipeline,
    apply_jacobi: ComputePipeline,
    capture_phase: ComputePipeline,
    island_workgroup_size: u32,
    timestamp_queries: bool,
    metrics: Option<MetricsRes>,
    #[cfg(not(target_arch = "wasm32"))]
    step_timestamp: Option<TimestampRes>,
    #[cfg(not(target_arch = "wasm32"))]
    ts_ring: Option<TimestampRing>,
    #[cfg(not(target_arch = "wasm32"))]
    last_gpu_collide_ms: f32,
    #[cfg(not(target_arch = "wasm32"))]
    last_gpu_broadphase_ms: f32,
    #[cfg(not(target_arch = "wasm32"))]
    last_gpu_narrowphase_ms: f32,
    #[cfg(not(target_arch = "wasm32"))]
    last_gpu_graph_ms: f32,
    #[cfg(not(target_arch = "wasm32"))]
    last_gpu_solve_ms: f32,
    #[cfg(not(target_arch = "wasm32"))]
    last_gpu_integrate_ms: f32,
    #[cfg(not(target_arch = "wasm32"))]
    last_gpu_prepare_ms: f32,
    #[cfg(not(target_arch = "wasm32"))]
    last_gpu_device_ms: f32,
    #[cfg(not(target_arch = "wasm32"))]
    pose_readback: Option<PoseReadback>,
    #[cfg(not(target_arch = "wasm32"))]
    automatic_pose_snapshots: bool,
    #[cfg(not(target_arch = "wasm32"))]
    pose_epoch: u64,
    #[cfg(not(target_arch = "wasm32"))]
    pose_export: Option<crate::pose_gl::PoseExportBuf>,
    #[cfg(not(target_arch = "wasm32"))]
    sticky_staging: Buffer,
    #[cfg(not(target_arch = "wasm32"))]
    sticky_pending: Option<std::sync::mpsc::Receiver<Result<(), wgpu::BufferAsyncError>>>,
    #[cfg(not(target_arch = "wasm32"))]
    sticky_loss: bool,
    #[cfg(not(target_arch = "wasm32"))]
    sticky_pending_step: u64,
    #[cfg(not(target_arch = "wasm32"))]
    metrics_context: [u64; 2],
    #[cfg(not(target_arch = "wasm32"))]
    sticky_pending_context: [u64; 2],
    #[cfg(not(target_arch = "wasm32"))]
    contact_metrics: Option<ContactMetrics>,
    #[cfg(not(target_arch = "wasm32"))]
    sticky_first_step: u32,
    #[cfg(not(target_arch = "wasm32"))]
    sticky_causes: [u32; 5],
    #[cfg(not(target_arch = "wasm32"))]
    sticky_contact_reasons: u32,
}

struct MetricsRes {
    steps: u32,
    submitted: u32,
    timestamp: Option<TimestampRes>,
    workload: Buffer,
    #[cfg(not(target_arch = "wasm32"))]
    workload_staging: Buffer,
    #[cfg(not(target_arch = "wasm32"))]
    cpu_encode_ms: Vec<f64>,
    #[cfg(not(target_arch = "wasm32"))]
    cpu_submit_ms: Vec<f64>,
}

struct TimestampRes {
    queries: wgpu::QuerySet,
    resolve: Buffer,
    #[cfg(not(target_arch = "wasm32"))]
    staging: Buffer,
}

#[cfg(not(target_arch = "wasm32"))]
struct TimestampRing {
    staging: [Buffer; 2],
    pending: [Option<std::sync::mpsc::Receiver<Result<(), wgpu::BufferAsyncError>>>; 2],
    step: [u64; 2],
    last_step: u64,
}

#[cfg(not(target_arch = "wasm32"))]
struct PoseReadback {
    staging: [Buffer; 2],
    pending: Option<usize>,
    /// Last pending slot copied into CPU body state. Mapping again while this
    /// matches `pending` would recopy the whole snapshot for every getter.
    consumed: Option<usize>,
    epoch: u64,
    maps: u64,
    kicks: u64,
    step: [u64; 2],
}

const TIMESTAMP_STEP_STRIDE: u64 = wgpu::QUERY_RESOLVE_BUFFER_ALIGNMENT;
const TIMESTAMP_QUERY_COUNT: u32 = 8;
const WORKLOAD_STEP_WORDS: u64 = 11;
const WORKLOAD_STEP_STRIDE: u64 = WORKLOAD_STEP_WORDS * mem::size_of::<u32>() as u64;

#[derive(Clone, Copy, Debug, Default)]
pub struct GpuRayHit {
    pub hit: bool,
    pub gpu_shape: u32,
    pub cpu_shape: u32,
    pub body: u32,
    pub point: [f32; 3],
    pub normal: [f32; 3],
    pub fraction: f32,
    pub material: u64,
    pub triangle: i32,
    pub overflow: u32,
    pub encode_ms: f32,
    pub map_ms: f32,
    pub exclusive_ms: f32,
    pub inclusive_ms: f32,
}

/// Completed GPU snapshot from contact scheduling, not native public contact IDs.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct ContactMetrics {
    pub step: u64,
    pub topology_revision: u64,
    pub state_revision: u64,
    pub capacity_loss: bool,
    pub candidate_pairs: u32,
    pub allocated_roots: u32,
    pub allocated_manifold_slots: u32,
    pub touching_roots: u32,
    pub non_sensor_roots: u32,
}

#[derive(Clone, Copy, Debug, Default)]
pub struct LiveStepStats {
    pub drops: crate::types::BroadphaseStats,
    pub sticky: crate::types::BroadphaseStats,
    pub first_fail_step: u32,
    pub unique_pairs: u32,
    pub narrowphase_pairs: u32,
    pub graph_proof_fail: u32,
    pub contact_drop_reasons: u32,
}

impl LiveStepStats {
    pub fn capacity_loss(&self) -> bool {
        self.sticky.has_loss() || self.drops.has_loss() || self.graph_proof_fail != 0
    }
}

#[derive(Clone, Copy, Debug, Default)]
pub struct AllocationStats {
    pub body_bytes: u64,
    pub shape_bytes: u64,
    pub contact_bytes: u64,
    pub joint_bytes: u64,
    pub scratch_bytes: u64,
    pub atom_bytes: u64,
    pub total_bytes: u64,
}

#[cfg(not(target_arch = "wasm32"))]
#[derive(Clone, Copy, Debug)]
pub struct CallbackContact {
    pub slot: u32,
    pub key: u32,
    pub body_a: u32,
    pub body_b: u32,
    pub count: u32,
    pub normal: [f32; 3],
    pub point_offset_a: [f32; 3],
    pub lifecycle_flags: u32,
}

#[derive(Clone, Copy, Debug, Default)]
pub struct WorkloadStats {
    pub candidate_pairs_avg: f64,
    pub candidate_pairs_peak: u32,
    pub cell_inserts_avg: f64,
    pub cell_inserts_peak: u32,
    pub unique_pairs_avg: f64,
    pub unique_pairs_peak: u32,
    pub narrowphase_pairs_avg: f64,
    pub narrowphase_pairs_peak: u32,
    pub occupied_contact_slots_avg: f64,
    pub occupied_contact_slots_peak: u32,
    pub capacity_drops_avg: f64,
    pub capacity_drops_peak: u32,
}

#[derive(Clone, Debug, Default)]
pub struct TimingWindow {
    pub steps: u32,
    pub gpu_collide_ms: Option<f64>,
    pub gpu_broadphase_ms: Option<f64>,
    pub gpu_narrowphase_ms: Option<f64>,
    pub gpu_graph_ms: Option<f64>,
    pub gpu_prepare_ms: Option<f64>,
    pub gpu_device_ms: Option<f64>,
    pub gpu_solve_ms: Option<f64>,
    pub gpu_integrate_ms: Option<f64>,
    pub cpu_encode_ms: f64,
    pub cpu_submit_ms: f64,
    pub workload: WorkloadStats,
}

impl GpuSim {
    pub fn new(
        gpu: &GpuDevice,
        bodies: &[BodyGpu],
        body_local_centers: &[[f32; 3]],
        body_inv_inertia_offdiag: &[[f32; 3]],
        shapes: &[ShapeGpu],
        hull_points: &[[f32; 4]],
        hull_planes: &[[f32; 4]],
        hull_edges: &[[f32; 4]],
        hull_topology: &[[u32; 4]],
        mesh_vertices: &[[f32; 4]],
        mesh_triangles: &[[u32; 4]],
        mesh_nodes: &[[f32; 4]],
        surface_materials: &[SurfaceMaterialGpu],
        mix_pairs: &[MixPairGpu],
        joints: &[JointGpu],
        caps: GpuSceneCaps,
        enable_contacts: bool,
        dt: f32,
        gravity: [f32; 3],
    ) -> Result<Self, String> {
        let device = gpu.device.clone();
        let queue = gpu.queue.clone();
        let pipeline_cache = crate::pipeline_cache::StartupCache::new(gpu);
        let make_compute = |device: &Device, layout: &wgpu::PipelineLayout, shader: &ShaderModule, entry: &str| {
            make_compute_cached(device, layout, shader, entry, pipeline_cache.cache.as_ref())
        };
        let make_compute_with_constant = |device: &Device, layout: &wgpu::PipelineLayout, shader: &ShaderModule, entry: &str, name: &str, value: f64| {
            make_compute_constant_cached(device, layout, shader, entry, name, value, pipeline_cache.cache.as_ref())
        };
        let count = bodies.len() as u32;
        let capacity = caps.bodies.max(count).max(1);
        let mut params = SimParams::new(count, enable_contacts, dt, gravity);
        params.joint_count = joints.len() as u32;
        params.shape_count = shapes.len() as u32;
        params.shape_base_u32 =
            capacity * ((mem::size_of::<BodyColdGpu>() + 32) / mem::size_of::<u32>()) as u32;
        params.hull_point_count = hull_points.len() as u32;
        params.hull_base_u32 = params.shape_base_u32
            + caps.shapes * (mem::size_of::<ShapeGpu>() / mem::size_of::<u32>()) as u32;
        params.hull_plane_count = hull_planes.len() as u32;
        params.hull_plane_base_u32 = params.hull_base_u32 + 4 * caps.hull_points;
        params.hull_edge_count = hull_edges.len() as u32;
        params.hull_edge_base_u32 = params.hull_plane_base_u32 + 4 * caps.hull_planes;
        params.hull_topology_count = hull_topology.len() as u32;
        params.hull_topology_base_u32 = params.hull_edge_base_u32 + 4 * caps.hull_edges;
        params.mesh_vertex_count =
            u32::try_from(mesh_vertices.len()).expect("mesh vertex capacity exceeds u32");
        params.mesh_vertex_base_u32 =
            params.hull_topology_base_u32 + 4 * caps.hull_topology;
        params.mesh_triangle_count =
            u32::try_from(mesh_triangles.len()).expect("mesh triangle capacity exceeds u32");
        params.mesh_triangle_base_u32 = params.mesh_vertex_base_u32 + 4 * caps.mesh_vertices;
        assert_eq!(
            mesh_nodes.len() % 2,
            0,
            "mesh node storage must use two vec4 rows"
        );
        params.mesh_node_count =
            u32::try_from(mesh_nodes.len() / 2).expect("mesh BVH node capacity exceeds u32");
        params.mesh_node_base_u32 = params.mesh_triangle_base_u32 + 4 * caps.mesh_triangles;
        params.surface_material_count = surface_materials.len() as u32;
        params.surface_material_base_u32 = params.mesh_node_base_u32 + 8 * caps.mesh_nodes;
        params.mix_pair_base_u32 = params.surface_material_base_u32
            + caps.materials * (mem::size_of::<SurfaceMaterialGpu>() / mem::size_of::<u32>()) as u32;
        params.mix_pair_count = u32::from(mix_pairs.iter().any(|pair| pair.occupied != 0));
        params.diagnostic_flags = diagnostic_flags_from_env();
        if params.diagnostic_flags & DIAG_DISABLE_SLEEP != 0 {
            params.enable_sleep = 0;
        }
        let small_component_workgroup_size=match std::env::var("GPU_PHYSICS_SMALL_COMPONENT_WG").ok().and_then(|v|v.parse::<u32>().ok()) {
            Some(n @ (8|16|32|64))=>n,
            _=>16,
        };
        let fat_geometry = shapes.iter().map(|s| FatGeometryKey::new(s, bodies)).collect();
        let body_state_bytes = mem::size_of::<BodyStateGpu>() * capacity as usize;
        let contact_slots = crate::types::contact_capacity(capacity);
        params.contact_capacity = contact_slots;
        let joint_bytes = mem::size_of::<JointGpu>() * joints.len().max(1);
        params.fat_bounds_base = crate::types::scratch_u32_count_with_joints(
            capacity, caps.joints.max(capacity)) as u32;
        let scratch_bytes = (params.fat_bounds_base as usize + 8 * caps.shapes as usize) * 4;
        let atom_contact_start = 16 + crate::types::HASH_BUCKETS + crate::types::PAIR_CAP;
        let atom_contact_value_start = atom_contact_start + crate::types::CONTACT_HASH_CAP;
        let atom_count = atom_contact_value_start + crate::types::CONTACT_HASH_CAP + 9 * capacity + 25;

        let pass_lut = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("pass-lut"),
            size: (PASS_BIAS_ROWS as u64 * MAX_COLORS as u64) * mem::size_of::<SimParams>() as u64,
            usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let mut states: Vec<_> = bodies.iter().map(BodyStateGpu::from_body).collect();
        states.resize(capacity as usize, BodyStateGpu::zeroed());
        let scene_bytes = pack_scene_bytes(
            caps,
            bodies,
            body_local_centers,
            body_inv_inertia_offdiag,
            shapes,
            hull_points,
            hull_planes,
            hull_edges,
            hull_topology,
            mesh_vertices,
            mesh_triangles,
            mesh_nodes,
            surface_materials,
            mix_pairs,
        )?;
        let max_bind = u64::from(device.limits().max_storage_buffer_binding_size);
        let max_buffer = device.limits().max_buffer_size;
        let scene_len = scene_bytes.len() as u64;
        if scene_len > max_bind || scene_len > max_buffer {
            return Err(format!(
                "GPU scene heap ({scene_len} bytes) exceeds storage binding ({max_bind}) or buffer ({max_buffer}) limit"
            ));
        }

        let bodies = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("body-state"),
            contents: bytemuck::cast_slice(&states),
            usage: wgpu::BufferUsages::STORAGE
                | wgpu::BufferUsages::VERTEX
                | wgpu::BufferUsages::COPY_SRC
                | wgpu::BufferUsages::COPY_DST,
        });
        let body_cold = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("body-cold"),
            contents: &scene_bytes,
            usage: wgpu::BufferUsages::STORAGE
                | wgpu::BufferUsages::VERTEX
                | wgpu::BufferUsages::COPY_SRC
                | wgpu::BufferUsages::COPY_DST,
        });
        let _ = body_state_bytes;
        let empty_contacts = vec![ContactHotGpu::empty(); contact_slots.max(1) as usize];
        let contacts = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("contact-hot"),
            contents: bytemuck::cast_slice(&empty_contacts),
            usage: wgpu::BufferUsages::STORAGE
                | wgpu::BufferUsages::COPY_DST
                | wgpu::BufferUsages::COPY_SRC,
        });
        let contact_persistent = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("contact-persistent"),
            contents: bytemuck::cast_slice(&vec![
                ContactPersistentGpu::zeroed();
                contact_slots.max(1) as usize
            ]),
            usage: wgpu::BufferUsages::STORAGE
                | wgpu::BufferUsages::COPY_DST
                | wgpu::BufferUsages::COPY_SRC,
        });
        let contact_prepared = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("contact-prepared"),
            contents: bytemuck::cast_slice(&vec![
                ContactPreparedGpu::zeroed();
                contact_slots.max(1) as usize
            ]),
            usage: wgpu::BufferUsages::STORAGE
                | wgpu::BufferUsages::COPY_DST
                | wgpu::BufferUsages::COPY_SRC,
        });
        let mut joint_pad = joints.to_vec();
        joint_pad.resize(caps.joints.max(1) as usize, JointGpu::zeroed());
        let joints_buf = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("joints"),
            contents: bytemuck::cast_slice(&joint_pad),
            usage: wgpu::BufferUsages::STORAGE
                | wgpu::BufferUsages::COPY_DST
                | wgpu::BufferUsages::COPY_SRC,
        });
        let scratch = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("scratch"),
            size: scratch_bytes as u64,
            usage: wgpu::BufferUsages::STORAGE
                | wgpu::BufferUsages::COPY_DST
                | wgpu::BufferUsages::COPY_SRC,
            mapped_at_creation: false,
        });
        let trace_words = if params.diagnostic_flags & crate::types::DIAG_MESH_CANDIDATES != 0 { crate::types::MESH_TRACE_WORDS } else { 0 };
        let mut atom_init = vec![0u32; (atom_count + trace_words) as usize];
        atom_init[atom_contact_start as usize
            ..(atom_contact_start + crate::types::CONTACT_HASH_CAP) as usize]
            .fill(u32::MAX);
        atom_init[atom_contact_value_start as usize
            ..(atom_contact_value_start + crate::types::CONTACT_HASH_CAP) as usize]
            .fill(u32::MAX);
        let atom = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("atom"),
            contents: bytemuck::cast_slice(&atom_init),
            usage: wgpu::BufferUsages::STORAGE
                | wgpu::BufferUsages::COPY_DST
                | wgpu::BufferUsages::COPY_SRC,
        });
        let indirect = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("indirect"),
            size: (6 + crate::types::MAX_COLORS as u64) * 16,
            usage: wgpu::BufferUsages::INDIRECT | wgpu::BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });
        #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
        {
            // Initialize native-copy resources through wgpu once, before any
            // state is uploaded. Native commands do not update its init tracker.
            // Apply to both controls in this experimental build.
            let mut init=device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label:Some("native-broadphase-init"),
            });
            init.clear_buffer(&scratch,0,None);
            init.clear_buffer(&indirect,0,None);
            queue.submit(Some(init.finish()));
        }
        // Cache storage begins after the entire capacity-sized component region,
        // never after the current live count. New sims start with a zero valid word.
        let memo_base = 261u64 + 54 * u64::from(capacity) + 2 * u64::from(contact_slots);
        let memo_words = memo_base + 52 + 2 * u64::from(capacity) + 6 * u64::from(contact_slots);
        let graph_memo_base = (std::env::var("GPU_PHYSICS_GRAPH_MEMO").as_deref()==Ok("1")
            && capacity<=8160 && device.limits().max_compute_workgroup_storage_size>=32768
            && memo_words*4<=max_bind.min(max_buffer))
            .then_some(memo_base as u32);
        let query = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("query"),
            size: if graph_memo_base.is_some() {memo_words*4} else {256*4},
            usage: wgpu::BufferUsages::STORAGE
                | wgpu::BufferUsages::COPY_DST
                | wgpu::BufferUsages::COPY_SRC,
            mapped_at_creation: false,
        });
        #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
        {
            let mut init=device.create_command_encoder(&wgpu::CommandEncoderDescriptor{label:Some("native-query-init")});
            init.clear_buffer(&query,0,None);
            queue.submit(Some(init.finish()));
        }
        let _ = joint_bytes;
        #[cfg(not(target_arch = "wasm32"))]
        let staging = None;

        let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
            label: Some("physics"),
            source: wgpu::ShaderSource::Wgsl(PHYSICS_WGSL.into()),
        });

        let bgl = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
            label: Some("physics-bgl"),
            entries: &[
                storage_entry(0, wgpu::BufferBindingType::Uniform, true),
                storage_entry(
                    1,
                    wgpu::BufferBindingType::Storage { read_only: false },
                    false,
                ),
                storage_entry(
                    2,
                    wgpu::BufferBindingType::Storage { read_only: true },
                    false,
                ),
                storage_entry(
                    3,
                    wgpu::BufferBindingType::Storage { read_only: false },
                    false,
                ),
                storage_entry(
                    4,
                    wgpu::BufferBindingType::Storage { read_only: false },
                    false,
                ),
                storage_entry(
                    5,
                    wgpu::BufferBindingType::Storage { read_only: false },
                    false,
                ),
                storage_entry(
                    6,
                    wgpu::BufferBindingType::Storage { read_only: false },
                    false,
                ),
                storage_entry(
                    7,
                    wgpu::BufferBindingType::Storage { read_only: false },
                    false,
                ),
                storage_entry(
                    8,
                    wgpu::BufferBindingType::Storage { read_only: false },
                    false,
                ),
                storage_entry(
                    9,
                    wgpu::BufferBindingType::Storage { read_only: false },
                    false,
                ),
            ],
        });

        let pipeline_layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
            label: Some("physics-pl"),
            bind_group_layouts: &[&bgl],
            push_constant_ranges: &[],
        });

        let bind_group = physics_bind_group(
            &device,
            &bgl,
            &pass_lut,
            &bodies,
            &body_cold,
            &contacts,
            &contact_persistent,
            &contact_prepared,
            &joints_buf,
            &scratch,
            &atom,
            &query,
        );

        let radix_histogram = [
            "radix_histogram_0",
            "radix_histogram_8",
            "radix_histogram_16",
            "radix_histogram_24",
        ]
        .map(|entry| make_compute(&device, &pipeline_layout, &shader, entry));
        let radix_scatter = [
            "radix_scatter_0",
            "radix_scatter_8",
            "radix_scatter_16",
            "radix_scatter_24",
        ]
        .map(|entry| make_compute(&device, &pipeline_layout, &shader, entry));
        let graph_radix_histogram = [
            "graph_radix_histogram_0",
            "graph_radix_histogram_8",
            "graph_radix_histogram_16",
            "graph_radix_histogram_24",
        ]
        .map(|entry| make_compute(&device, &pipeline_layout, &shader, entry));
        let graph_radix_scatter = [
            "graph_radix_scatter_0",
            "graph_radix_scatter_8",
            "graph_radix_scatter_16",
            "graph_radix_scatter_24",
        ]
        .map(|entry| make_compute(&device, &pipeline_layout, &shader, entry));
        let limits = device.limits();
        let island_workgroup_size = if limits.max_compute_invocations_per_workgroup >= 128
            && limits.max_compute_workgroup_size_x >= 128
        {
            128
        } else {
            64
        };
        let island_compute = |entry| {
            make_compute_with_constant(
                &device,
                &pipeline_layout,
                &shader,
                entry,
                "ISLAND_WORKGROUP_SIZE",
                island_workgroup_size as f64,
            )
        };

        let color_wave_prefix_override = match std::env::var("GPU_PHYSICS_COLOR_PREFIX") {
            Ok(s) if s == "auto" => None,
            Ok(s) => {
                let n = s.parse::<u32>().expect("GPU_PHYSICS_COLOR_PREFIX must be auto or 0..23");
                assert!(n <= crate::types::OVERFLOW_COLOR);
                Some(n)
            }
            Err(_) => None,
        };
        let mut sim = Self {
            #[cfg(feature = "native-command-cache")]
            render_copy: None,
            device: device.clone(),
            queue,
            report: gpu.report.clone(),
            count,
            caps,
            contact_epoch: CONTACT_EPOCH.fetch_add(1, Ordering::Relaxed),
            params,
            physics_step: 0,
            last_submit: None,
            completed_step: 0,
            completed_known: true,
            pose_step: 0,
            last_mirror_wait_ms: 0.0,
            last_mirror_copy_ms: 0.0,
            last_mirror_map_ms: 0.0,
            last_mirror_bytes: 0,
            one_group_wave_only: false,
            color_wave_prefix: color_wave_prefix_override.unwrap_or(crate::types::OVERFLOW_COLOR),
            color_wave_prefix_override,
            color_hint_min_step: 0,
            one_group_pair_ok: false,
            skip_general_static_sort: false,
            solver_dispatches: Cell::new(0),
            joint_dispatches: Cell::new(0),
            static_sort_dispatches: Cell::new(0),
            encode_commands: Cell::new(0),
            physics_invalid: false,
            callback_open: false,
            pass_lut,
            bodies,
            body_cold,
            convex_ccd: None,
            contacts,
            contact_persistent,
            contact_prepared,
            joints: joints_buf,
            scratch,
            atom,
            query,
            indirect,
            #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
            radix_cache: None,
            #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
            contact_cache: None,
            #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
            graph_cache: None,
            #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
            tail_cache: std::array::from_fn(|_|None),
            bind_group,
            bind_group_layout: bgl,
            fat_geometry,
            shape_identities: shapes.iter().enumerate().map(|(i, _)| (i as u32, 0)).collect(),
            clear_remapped_contact_hash: make_compute(&device, &pipeline_layout, &shader, "clear_remapped_contact_hash"),
            remap_contact_shapes: make_compute(&device, &pipeline_layout, &shader, "remap_contact_shapes"),
            publish_remapped_contacts: make_compute(&device, &pipeline_layout, &shader, "publish_remapped_contacts"),
            contact_slots,
            shape_count: shapes.len() as u32,
            #[cfg(not(target_arch = "wasm32"))]
            staging,
            reset_deltas: make_compute(&device, &pipeline_layout, &shader, "reset_deltas"),
            island_init: island_compute("island_init"),
            island_union_edges: island_compute("island_union_edges"),
            island_accumulate_wake: island_compute("island_accumulate_wake"),
            island_apply_wake: island_compute("island_apply_wake"),
            island_reset_ready: island_compute("island_reset_ready"),
            island_accumulate_ready: island_compute("island_accumulate_ready"),
            island_apply_sleep: island_compute("island_apply_sleep"),
            apply_deltas: make_compute(&device, &pipeline_layout, &shader, "apply_deltas"),
            integrate_vel: make_compute(&device, &pipeline_layout, &shader, "integrate_vel"),
            clear_broadphase: make_compute(&device, &pipeline_layout, &shader, "clear_broadphase"),
            pair_matrix: std::sync::OnceLock::new(),
            pair_matrix_requested: std::env::var("GPU_PHYSICS_PAIR_MATRIX").as_deref()==Ok("1"),
            pair_matrix_used: false,
            pair_matrix_flat: shapes.iter().all(|s|s.kind!=crate::types::KIND_MESH && s.event_flags & (crate::types::SHAPE_PUBLIC_PROXY|crate::types::SHAPE_COMPOUND_CHILD)==0),
            hash_insert: make_compute(&device, &pipeline_layout, &shader, "hash_insert"),
            write_insert_indirect: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "write_insert_indirect",
            ),
            write_radix_indirect: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "write_radix_indirect",
            ),
            emit_hash_pairs: make_compute(&device, &pipeline_layout, &shader, "emit_hash_pairs"),
            emit_static_pairs: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "emit_static_pairs",
            ),
            collect_fat_statics: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "collect_fat_statics",
            ),
            finish_fat_statics: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "finish_fat_statics",
            ),
            write_occupied_indirect: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "write_occupied_indirect",
            ),
            emit_prev_pairs: make_compute(&device, &pipeline_layout, &shader, "emit_prev_pairs"),
            radix_histogram,
            radix_bucket_bases: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "radix_bucket_bases",
            ),
            radix_group_prefix: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "radix_group_prefix",
            ),
            radix_scatter,
            compact_unique_histogram: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "compact_unique_histogram",
            ),
            compact_unique_bases: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "compact_unique_bases",
            ),
            compact_unique_scatter: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "compact_unique_scatter",
            ),
            compact_unique_gather: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "compact_unique_gather",
            ),
            find_existing_contact_slots: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "find_existing_contact_slots",
            ),
            alloc_free_histogram: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "alloc_free_histogram",
            ),
            alloc_free_bases: make_compute(&device, &pipeline_layout, &shader, "alloc_free_bases"),
            alloc_free_scatter: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "alloc_free_scatter",
            ),
            alloc_missing_histogram: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "alloc_missing_histogram",
            ),
            alloc_missing_bases: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "alloc_missing_bases",
            ),
            alloc_missing_scatter: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "alloc_missing_scatter",
            ),
            alloc_bind_slots: make_compute(&device, &pipeline_layout, &shader, "alloc_bind_slots"),
            collide_pairs_no_mesh: std::sync::OnceLock::new(),
            collide_pairs_mesh: std::sync::OnceLock::new(),
            collision_shader: shader.clone(),
            collision_layout: pipeline_layout.clone(),
            retire_body_pair_contacts: make_compute(&device, &pipeline_layout, &shader, "retire_body_pair_contacts"),
            retire_stale_contacts: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "retire_stale_contacts",
            ),
            begin_occupied_contacts: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "begin_occupied_contacts",
            ),
            collect_occupied_contacts: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "collect_occupied_contacts",
            ),
            finish_occupied_contacts: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "finish_occupied_contacts",
            ),
            write_unique_indirect: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "write_unique_indirect",
            ),
            write_alloc_indirects: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "write_alloc_indirects",
            ),
            graph_clear_meta: make_compute(&device, &pipeline_layout, &shader, "graph_clear_meta"),
            graph_reset_colors: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "graph_reset_colors",
            ),
            graph_classify: make_compute(&device, &pipeline_layout, &shader, "graph_classify"),
            graph_mark_edges: make_compute(&device, &pipeline_layout, &shader, "graph_mark_edges"),
            graph_compact_paired: std::sync::OnceLock::new(),
            graph_count_static_degree: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "graph_count_static_degree",
            ),
            graph_finish_static_degree: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "graph_finish_static_degree",
            ),
            graph_pack_static_keys: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "graph_pack_static_keys",
            ),
            graph_radix_histogram,
            graph_radix_scatter,
            graph_mark_static_starts: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "graph_mark_static_starts",
            ),
            graph_encode_static_colors: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "graph_encode_static_colors",
            ),
            graph_mark_color_starts: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "graph_mark_color_starts",
            ),
            graph_assign_static: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "graph_assign_static",
            ),
            graph_memo_base,
            graph_assign_memo: std::sync::OnceLock::new(),
            graph_shared_requested: std::env::var("GPU_PHYSICS_GRAPH_SHARED").as_deref()==Ok("1"),
            graph_assign_shared: std::sync::OnceLock::new(),
            graph_assign_dynamic: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "graph_assign_dynamic",
            ),
            color_and_compact: island_compute("color_and_compact"),
            prepare_contacts: make_compute(&device, &pipeline_layout, &shader, "prepare_contacts"),
            warm_start: make_compute(&device, &pipeline_layout, &shader, "warm_start"),
            solve_contacts: make_compute(&device, &pipeline_layout, &shader, "solve_contacts"),
            component_reset: make_compute(&device, &pipeline_layout, &shader, "component_reset"),
            component_count: make_compute(&device, &pipeline_layout, &shader, "component_count"),
            component_offsets: make_compute(&device, &pipeline_layout, &shader, "component_offsets"),
            component_color_offsets: make_compute(&device, &pipeline_layout, &shader, "component_color_offsets"),
            component_scatter: make_compute(&device, &pipeline_layout, &shader, "component_scatter"),
            solve_large_components: make_compute(&device, &pipeline_layout, &shader, "solve_large_components"),
            small_component_workgroup_size,
            solve_complete_components: make_compute_with_constant(&device,&pipeline_layout,&shader,"solve_complete_components",
                "SMALL_COMPONENT_WORKGROUP_SIZE",f64::from(small_component_workgroup_size)),
            component_tgs: false,
            count_active_bodies: make_compute(&device,&pipeline_layout,&shader,"count_active_bodies"),
            idle_eligible:false, idle_input_context:[0;2], idle_output_context:[0;2],
            idle_proof:None, idle_chain:None, #[cfg(test)] hold_idle_status:false, idle_count_valid_step:None, sticky_pending_idle_context:None,
            last_step_idle:false, idle_epoch:Cell::new(0),
            solve_color: make_compute(&device, &pipeline_layout, &shader, "solve_color"),
            solve_color_wave_one_group: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "solve_color_wave_one_group",
            ),
            solve_overflow: make_compute(&device, &pipeline_layout, &shader, "solve_overflow"),
            solve_color_tail_one_group: make_compute(&device, &pipeline_layout, &shader, "solve_color_tail_one_group"),
            solve_joints: make_compute(&device, &pipeline_layout, &shader, "solve_joints"),
            compact_joint_heads: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "compact_joint_heads",
            ),
            compact_joint_components: make_compute(
                &device,
                &pipeline_layout,
                &shader,
                "compact_joint_components",
            ),
            ray_closest: make_compute(&device, &pipeline_layout, &shader, "ray_closest"),
            ray_closest_pick: make_compute(&device, &pipeline_layout, &shader, "ray_closest_pick"),
            ray_closest_commit: make_compute(&device, &pipeline_layout, &shader, "ray_closest_commit"),
            solve_tiny_islands: make_compute(&device, &pipeline_layout, &shader, "solve_tiny_islands"),
            integrate_pos: make_compute(&device, &pipeline_layout, &shader, "integrate_pos"),
            jacobi_clear: make_compute(&device, &pipeline_layout, &shader, "jacobi_clear"),
            solve_jacobi: make_compute(&device, &pipeline_layout, &shader, "solve_jacobi"),
            apply_jacobi: make_compute(&device, &pipeline_layout, &shader, "apply_jacobi"),
            capture_phase: make_compute(&device, &pipeline_layout, &shader, "capture_phase"),
            island_workgroup_size,
            timestamp_queries: gpu.timestamp_queries,
            metrics: None,
            #[cfg(not(target_arch = "wasm32"))]
            step_timestamp: gpu.timestamp_queries.then(|| Self::make_timestamp(&device, TIMESTAMP_QUERY_COUNT, "physics-step-ts")),
            #[cfg(not(target_arch = "wasm32"))]
            ts_ring: gpu.timestamp_queries.then(|| Self::make_timestamp_ring(&device)),
            #[cfg(not(target_arch = "wasm32"))]
            last_gpu_collide_ms: 0.0,
            #[cfg(not(target_arch = "wasm32"))]
            last_gpu_broadphase_ms: 0.0,
            #[cfg(not(target_arch = "wasm32"))]
            last_gpu_narrowphase_ms: 0.0,
            #[cfg(not(target_arch = "wasm32"))]
            last_gpu_graph_ms: 0.0,
            #[cfg(not(target_arch = "wasm32"))]
            last_gpu_solve_ms: 0.0,
            #[cfg(not(target_arch = "wasm32"))]
            last_gpu_integrate_ms: 0.0,
            #[cfg(not(target_arch = "wasm32"))]
            last_gpu_prepare_ms: 0.0,
            #[cfg(not(target_arch = "wasm32"))]
            last_gpu_device_ms: 0.0,
            #[cfg(not(target_arch = "wasm32"))]
            pose_readback: None,
            #[cfg(not(target_arch = "wasm32"))]
            automatic_pose_snapshots: true,
            #[cfg(not(target_arch = "wasm32"))]
            pose_epoch: 0,
            #[cfg(not(target_arch = "wasm32"))]
            pose_export: crate::pose_gl::try_create_export_buffer(&device, body_state_bytes as u64),
            #[cfg(not(target_arch = "wasm32"))]
            sticky_staging: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("physics-sticky-status"),
                size: 60,
                usage: wgpu::BufferUsages::MAP_READ | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            }),
            #[cfg(not(target_arch = "wasm32"))]
            sticky_pending: None,
            #[cfg(not(target_arch = "wasm32"))]
            sticky_loss: false,
            #[cfg(not(target_arch = "wasm32"))]
            sticky_pending_step: 0,
            #[cfg(not(target_arch = "wasm32"))]
            metrics_context: [0;2],
            #[cfg(not(target_arch = "wasm32"))]
            sticky_pending_context: [0;2],
            #[cfg(not(target_arch = "wasm32"))]
            contact_metrics: None,
            #[cfg(not(target_arch = "wasm32"))]
            sticky_first_step: 0,
            #[cfg(not(target_arch = "wasm32"))]
            sticky_causes: [0; 5],
            #[cfg(not(target_arch = "wasm32"))]
            sticky_contact_reasons: 0,
            startup_cache: pipeline_cache,
        };
        sim.startup_cache.save();
        sim.upload_pass_lut();
        sim.upload_joint_filter(&joint_pad);
        Ok(sim)
    }

    #[cfg(not(target_arch = "wasm32"))]
    fn make_timestamp(device: &Device, query_count: u32, label: &str) -> TimestampRes {
        let size = TIMESTAMP_STEP_STRIDE * u64::from((query_count + TIMESTAMP_QUERY_COUNT - 1) / TIMESTAMP_QUERY_COUNT);
        TimestampRes {
            queries: device.create_query_set(&wgpu::QuerySetDescriptor {
                label: Some(label),
                ty: wgpu::QueryType::Timestamp,
                count: query_count,
            }),
            resolve: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("physics-ts-resolve"),
                size,
                usage: wgpu::BufferUsages::QUERY_RESOLVE | wgpu::BufferUsages::COPY_SRC,
                mapped_at_creation: false,
            }),
            staging: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("physics-ts-staging"),
                size,
                usage: wgpu::BufferUsages::MAP_READ | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            }),
        }
    }

    #[cfg(not(target_arch = "wasm32"))]
    fn make_timestamp_ring(device: &Device) -> TimestampRing {
        let make = |label: &str| {
            device.create_buffer(&wgpu::BufferDescriptor {
                label: Some(label),
                size: TIMESTAMP_STEP_STRIDE,
                usage: wgpu::BufferUsages::MAP_READ | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            })
        };
        TimestampRing {
            staging: [make("physics-ts-ring-0"), make("physics-ts-ring-1")],
            pending: [None, None],
            step: [0, 0],
            last_step: 0,
        }
    }

    pub fn current_bodies_buffer(&self) -> &Buffer {
        &self.bodies
    }

    pub fn body_cold_buffer(&self) -> &Buffer {
        &self.body_cold
    }

    pub fn params(&self) -> SimParams {
        self.params
    }

    pub fn enable_contacts(&self) -> bool {
        self.params.enable_contacts == 1
    }

    fn live_bg(&self) -> &BindGroup {
        &self.bind_group
    }

    fn ping_bg(&self) -> &BindGroup {
        &self.bind_group
    }

    fn dispatch_live(
        &self,
        encoder: &mut wgpu::CommandEncoder,
        pipeline: &ComputePipeline,
        groups: u32,
        color: u32,
        use_bias: u32,
        timestamps: Option<wgpu::ComputePassTimestampWrites<'_>>,
    ) {
        let off = Self::pass_lut_offset(color, use_bias) as u32;
        {
            let mut pass = encoder.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("physics-live"),
                timestamp_writes: timestamps,
            });
            pass.set_pipeline(pipeline);
            pass.set_bind_group(0, self.live_bg(), &[off]);
            pass.dispatch_workgroups(groups.max(1), 1, 1);
        }
    }

    fn emit_wave<'a>(&'a self, pass: &mut wgpu::ComputePass<'a>, use_bias: u32) {
        let prefix = if self.params.diagnostic_flags & DIAG_GENERAL_SOLVER != 0 {
            crate::types::OVERFLOW_COLOR
        } else { self.color_wave_prefix };
        pass.set_bind_group(
            0,
            self.live_bg(),
            &[Self::pass_lut_offset(0, use_bias) as u32],
        );
        pass.set_pipeline(&self.solve_color_wave_one_group);
        pass.dispatch_workgroups_indirect(&self.indirect, 0);
        self.solver_dispatches
            .set(self.solver_dispatches.get().saturating_add(1));
        self.encode_commands
            .set(self.encode_commands.get().saturating_add(1));
        if self.one_group_wave_only {
            return;
        }
        pass.set_pipeline(&self.solve_color);
        for col in
            std::iter::once(crate::types::OVERFLOW_COLOR).chain(0..prefix)
        {
            pass.set_bind_group(
                0,
                self.live_bg(),
                &[Self::pass_lut_offset(col, use_bias) as u32],
            );
            pass.dispatch_workgroups_indirect(&self.indirect, 16 + u64::from(col) * 16);
            self.solver_dispatches
                .set(self.solver_dispatches.get().saturating_add(1));
            self.encode_commands
                .set(self.encode_commands.get().saturating_add(1));
        }
        if prefix < crate::types::DYNAMIC_COLOR_COUNT {
            pass.set_pipeline(&self.solve_color_tail_one_group);
            pass.set_bind_group(0, self.live_bg(), &[Self::pass_lut_offset(prefix, use_bias) as u32]);
            pass.dispatch_workgroups(1, 1, 1);
            self.solver_dispatches.set(self.solver_dispatches.get().saturating_add(1));
            self.encode_commands.set(self.encode_commands.get().saturating_add(1));
        }
        // Static-contact colors follow all dynamic colors in the reference
        // order. Keep their often-wide ground-contact waves fully parallel.
        if prefix < crate::types::OVERFLOW_COLOR {
            pass.set_pipeline(&self.solve_color);
            for col in prefix.max(crate::types::DYNAMIC_COLOR_COUNT)..crate::types::OVERFLOW_COLOR {
                pass.set_bind_group(0, self.live_bg(), &[Self::pass_lut_offset(col, use_bias) as u32]);
                pass.dispatch_workgroups_indirect(&self.indirect, 16 + u64::from(col) * 16);
                self.solver_dispatches.set(self.solver_dispatches.get().saturating_add(1));
                self.encode_commands.set(self.encode_commands.get().saturating_add(1));
            }
        }
    }

    #[cfg(test)]
    pub(crate) fn set_color_wave_prefix(&mut self, prefix: u32) {
        assert!(prefix <= crate::types::OVERFLOW_COLOR || prefix == u32::MAX);
        self.color_wave_prefix_override = (prefix != u32::MAX).then_some(prefix);
        self.color_wave_prefix = self.color_wave_prefix_override.unwrap_or(crate::types::OVERFLOW_COLOR);
    }

    #[cfg(test)]
    pub(crate) fn color_wave_prefix(&self) -> u32 { self.color_wave_prefix }

    fn emit_n<'a>(
        &'a self,
        pass: &mut wgpu::ComputePass<'a>,
        pipeline: &'a ComputePipeline,
        groups: u32,
        color: u32,
        use_bias: u32,
    ) {
        pass.set_pipeline(pipeline);
        pass.set_bind_group(
            0,
            self.ping_bg(),
            &[Self::pass_lut_offset(color, use_bias) as u32],
        );
        pass.dispatch_workgroups(groups.max(1), 1, 1);
        self.encode_commands
            .set(self.encode_commands.get().saturating_add(1));
    }

    fn joint_serial(&self) -> bool {
        self.params.diagnostic_flags & crate::types::DIAG_SERIAL_JOINTS != 0
    }

    fn joint_solve_groups(&self) -> u32 {
        if self.joint_serial() {
            1
        } else {
            self.params.joint_count.div_ceil(WORKGROUP_SIZE).max(1)
        }
    }

    fn prepare_joint_components(&self, encoder: &mut wgpu::CommandEncoder) {
        if self.params.joint_count == 0 || self.joint_serial() {
            return;
        }
        self.dispatch_n(
            encoder,
            &self.compact_joint_heads.clone(),
            self.body_groups(),
            0,
            1,
        );
        self.dispatch_n(
            encoder,
            &self.compact_joint_components.clone(),
            1,
            0,
            1,
        );
    }

    fn dispatch_wave(&self, encoder: &mut wgpu::CommandEncoder, use_bias: u32) {
        let jacobi = self.params.solver_mode == 1;
        if jacobi {
            let off = Self::pass_lut_offset(0, use_bias) as u32;
            let mut pass = encoder.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("physics-jacobi-wave"),
                timestamp_writes: None,
            });
            pass.set_bind_group(0, self.live_bg(), &[off]);
            pass.set_pipeline(&self.jacobi_clear);
            pass.dispatch_workgroups(self.body_groups().max(1), 1, 1);
            pass.set_pipeline(&self.solve_jacobi);
            pass.dispatch_workgroups_indirect(&self.indirect, 0);
            pass.set_pipeline(&self.apply_jacobi);
            pass.dispatch_workgroups(self.body_groups().max(1), 1, 1);
            return;
        }

        // The first indirect command is enabled only when every non-overflow
        // color fits one workgroup. Ordered per-color commands use compacted
        // graph counts. Keeping all commands in one compute pass removes pass
        // setup while preserving Gauss-Seidel color order.
        let mut pass = encoder.begin_compute_pass(&wgpu::ComputePassDescriptor {
            label: Some("physics-colored-wave"),
            timestamp_writes: None,
        });
        self.emit_wave(&mut pass, use_bias);
    }

    fn dispatch_n(
        &self,
        encoder: &mut wgpu::CommandEncoder,
        pipeline: &ComputePipeline,
        groups: u32,
        color: u32,
        use_bias: u32,
    ) {
        let bg = self.ping_bg();
        let off = Self::pass_lut_offset(color, use_bias) as u32;
        {
            let mut pass = encoder.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("physics-pass"),
                timestamp_writes: None,
            });
            pass.set_pipeline(pipeline);
            pass.set_bind_group(0, bg, &[off]);
            pass.dispatch_workgroups(groups.max(1), 1, 1);
        }
        self.encode_commands
            .set(self.encode_commands.get().saturating_add(1));
    }

    fn body_groups(&self) -> u32 {
        (self.count + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE.max(1)
    }

    fn shape_groups(&self) -> u32 {
        (self.shape_count + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE.max(1)
    }

    fn island_groups(&self, count: u32) -> u32 {
        count.div_ceil(self.island_workgroup_size)
    }

    fn pass_lut_offset(color: u32, use_bias: u32) -> u64 {
        (use_bias.min(PASS_BIAS_ROWS - 1) * MAX_COLORS + color.min(MAX_COLORS - 1)) as u64
            * mem::size_of::<SimParams>() as u64
    }

    fn indirect_prepare_offset() -> u64 {
        (1 + crate::types::MAX_COLORS as u64) * 16
    }

    fn indirect_island_offset() -> u64 {
        (2 + crate::types::MAX_COLORS as u64) * 16
    }

    fn indirect_radix_offset() -> u64 {
        (3 + crate::types::MAX_COLORS as u64) * 16
    }

    fn indirect_static_offset() -> u64 {
        (4 + crate::types::MAX_COLORS as u64) * 16
    }

    fn dispatch_n_indirect(
        &self,
        encoder: &mut wgpu::CommandEncoder,
        pipeline: &ComputePipeline,
        indirect_offset: u64,
        color: u32,
        use_bias: u32,
    ) {
        let bg = self.ping_bg();
        let off = Self::pass_lut_offset(color, use_bias) as u32;
        {
            let mut pass = encoder.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("physics-pass-indirect"),
                timestamp_writes: None,
            });
            pass.set_pipeline(pipeline);
            pass.set_bind_group(0, bg, &[off]);
            pass.dispatch_workgroups_indirect(&self.indirect, indirect_offset);
        }
        self.encode_commands
            .set(self.encode_commands.get().saturating_add(1));
    }

    fn copy_solver_indirects(&self, enc: &mut wgpu::CommandEncoder) {
        enc.copy_buffer_to_buffer(
            &self.scratch,
            40 * 4,
            &self.indirect,
            Self::indirect_prepare_offset(),
            16,
        );
        enc.copy_buffer_to_buffer(
            &self.scratch,
            44 * 4,
            &self.indirect,
            Self::indirect_island_offset(),
            16,
        );
    }

    fn try_native_tail(&mut self, enc:&mut wgpu::CommandEncoder, stage:usize)->bool {
        #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
        {
            if std::env::var("GPU_PHYSICS_NATIVE_TAIL_CACHE").as_deref()!=Ok("1")
                || self.params.diagnostic_flags & crate::types::DIAG_PHASE_CAPTURE!=0 {return false;}
            use crate::native_command_cache::Command::{Dispatch,Indirect,CopyBuffer};
            let bg=self.body_groups().max(1);
            let ig=self.island_groups(self.count).max(1);
            // Both indirect producers use these counts, large roots are capped at 256.
            if u64::from(crate::types::PAIR_CAP.max(self.params.joint_count)).div_ceil(u64::from(self.island_workgroup_size))
                >u64::from(self.device.limits().max_compute_workgroups_per_dimension) {return false;}
            let key=[6+stage as u32 | (self.small_component_workgroup_size<<8) | (self.island_workgroup_size<<16),self.count,self.contact_slots];
            if self.tail_cache[stage].as_ref().is_none_or(|c| c.group!=self.bind_group || c.key!=key) {
                let large_offset=(5+crate::types::MAX_COLORS as u64)*16;
                let commands=match stage {
                    0=>vec![Dispatch(&self.island_init,ig),Indirect(&self.island_union_edges,Self::indirect_island_offset()),
                        Dispatch(&self.island_accumulate_wake,ig),Dispatch(&self.island_apply_wake,ig),
                        Indirect(&self.prepare_contacts,Self::indirect_prepare_offset())],
                    1=>vec![Dispatch(&self.component_reset,bg),Dispatch(&self.component_count,bg.max(self.contact_groups())),
                        Dispatch(&self.component_offsets,1),Dispatch(&self.component_color_offsets,bg),
                        Dispatch(&self.component_scatter,bg.max(self.contact_groups())),
                        CopyBuffer{buffer:&self.query,source:257*4,destination:large_offset,bytes:12},
                        Dispatch(&self.solve_complete_components,self.count.div_ceil(self.small_component_workgroup_size).max(1)),
                        Indirect(&self.solve_large_components,large_offset)],
                    2=>vec![Dispatch(&self.apply_deltas,bg),Dispatch(&self.island_reset_ready,ig),
                        Dispatch(&self.island_accumulate_ready,ig),Dispatch(&self.island_apply_sleep,ig)],
                    _=>unreachable!(),
                };
                self.tail_cache[stage]=Some(crate::native_command_cache::RadixCache::record(
                    &self.device,&self.bind_group,&self.collision_layout,&self.indirect,Some(&self.scratch),
                    Self::pass_lut_offset(0,1) as u32,key,&commands));
            }
            let buffers=[&self.bodies,&self.contacts,&self.contact_persistent,&self.contact_prepared,
                &self.joints,&self.scratch,&self.atom,&self.query];
            enc.transition_resources(buffers.into_iter().map(|buffer|wgpu::BufferTransition{
                buffer,state:wgpu::BufferUses::STORAGE_READ_WRITE}).chain([
                    wgpu::BufferTransition{buffer:&self.body_cold,state:wgpu::BufferUses::STORAGE_READ_ONLY},
                    wgpu::BufferTransition{buffer:&self.pass_lut,state:wgpu::BufferUses::UNIFORM},
                    wgpu::BufferTransition{buffer:&self.indirect,state:wgpu::BufferUses::INDIRECT},
                ]),std::iter::empty());
            self.tail_cache[stage].as_ref().unwrap().encode(enc);
            self.encode_commands.set(self.encode_commands.get().saturating_add([5,7,4][stage]));
            return true;
        }
        #[cfg(not(all(feature = "native-command-cache", not(target_arch = "wasm32"))))]
        {let _=(enc,stage);false}
    }

    fn dispatch_islands_and_prepare(&mut self, enc: &mut wgpu::CommandEncoder) {
        if self.try_native_tail(enc,0) {return;}
        let island_body_groups = self.island_groups(self.count);
        self.dispatch_n(enc, &self.island_init.clone(), island_body_groups, 0, 1);
        self.dispatch_n_indirect(
            enc,
            &self.island_union_edges.clone(),
            Self::indirect_island_offset(),
            0,
            1,
        );
        self.dispatch_n(
            enc,
            &self.island_accumulate_wake.clone(),
            island_body_groups,
            0,
            1,
        );
        self.dispatch_n(
            enc,
            &self.island_apply_wake.clone(),
            island_body_groups,
            0,
            1,
        );
        self.dispatch_n_indirect(
            enc,
            &self.prepare_contacts.clone(),
            Self::indirect_prepare_offset(),
            0,
            1,
        );
    }

    fn upload_pass_lut(&self) {
        let mut bytes =
            vec![0u8; PASS_BIAS_ROWS as usize * MAX_COLORS as usize * mem::size_of::<SimParams>()];
        for bias in 0..PASS_BIAS_ROWS {
            for color in 0..MAX_COLORS {
                let mut p = self.params;
                p.color_select = color;
                p.use_bias = bias;
                let off = Self::pass_lut_offset(color, bias) as usize;
                bytes[off..off + mem::size_of::<SimParams>()]
                    .copy_from_slice(bytemuck::bytes_of(&p));
            }
        }
        self.queue.write_buffer(&self.pass_lut, 0, &bytes);
    }

    pub fn flush_params(&self) {
        self.upload_pass_lut();
    }

    fn ping_dispatch(
        &mut self,
        encoder: &mut wgpu::CommandEncoder,
        pipeline: &ComputePipeline,
        groups: u32,
        color: u32,
        use_bias: u32,
    ) {
        self.dispatch_n(encoder, pipeline, groups, color, use_bias);
    }

    fn copy_then(
        &mut self,
        encoder: &mut wgpu::CommandEncoder,
        pipeline: &ComputePipeline,
        groups: u32,
        color: u32,
        use_bias: u32,
    ) {
        let off = Self::pass_lut_offset(color, use_bias) as u32;
        {
            let mut pass = encoder.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("physics-in-place"),
                timestamp_writes: None,
            });
            pass.set_bind_group(0, self.ping_bg(), &[off]);
            pass.set_pipeline(pipeline);
            pass.dispatch_workgroups(groups.max(1), 1, 1);
        }
        self.joint_dispatches
            .set(self.joint_dispatches.get().saturating_add(1));
        self.encode_commands
            .set(self.encode_commands.get().saturating_add(1));
    }

    fn contact_groups(&self) -> u32 {
        (self.contact_slots + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE.max(1)
    }

    fn pair_groups(&self) -> u32 {
        (crate::types::PAIR_CAP + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE.max(1)
    }

    fn hash_groups(&self) -> u32 {
        (crate::types::HASH_BUCKETS + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE.max(1)
    }

    #[cfg(test)]
    pub(crate) fn set_pair_matrix_test(&mut self, enabled:bool) { self.pair_matrix_requested=enabled; }
    #[cfg(test)]
    pub(crate) fn pair_matrix_used_test(&self)->bool { self.pair_matrix_used }
    fn broadphase_candidates_pass(&mut self, enc: &mut wgpu::CommandEncoder) {
        self.pair_matrix_used=false;
        let shape_groups = self.shape_groups();
        let cg = self
            .contact_groups()
            .max(self.hash_groups())
            .max(self.pair_groups());
        let off = Self::pass_lut_offset(0, 1) as u32;
        if self.pair_matrix_requested && self.pair_matrix_flat
            && self.shape_count<=704 && self.params.mesh_triangle_count==0 && self.params.enable_contacts!=0 {
            self.pair_matrix_used=true;
            assert!(self.shape_count*self.shape_count.div_ceil(32)<=crate::types::HASH_BUCKETS);
            let pipelines=self.pair_matrix.get_or_init(|| ["pair_matrix_build","pair_matrix_previous","pair_matrix_count","pair_matrix_bases","pair_matrix_scatter"]
                .map(|name|self.make_runtime_compute(&self.device,&self.collision_layout,&self.collision_shader,name)));
            #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
            if std::env::var("GPU_PHYSICS_NATIVE_PAIR_CACHE").as_deref()==Ok("1") {
                use crate::native_command_cache::Command::Dispatch;
                // Shape span determines matrix/row dispatches. Contact capacity may
                // change independently of shape span, so retain it in the key too.
                let key=[5,cg.max(1),self.shape_count | (self.contact_groups().max(1)<<10)];
                if self.radix_cache.as_ref().is_none_or(|c| c.group!=self.bind_group || c.key!=key) {
                    let commands=[Dispatch(&self.clear_broadphase,cg.max(1)),
                        Dispatch(&pipelines[0],(self.shape_count*self.shape_count.div_ceil(32)).div_ceil(64).max(1)),
                        Dispatch(&pipelines[1],self.contact_groups().max(1)),
                        Dispatch(&pipelines[2],shape_groups.max(1)),
                        Dispatch(&pipelines[3],1),Dispatch(&pipelines[4],shape_groups.max(1))];
                    self.radix_cache=Some(crate::native_command_cache::RadixCache::record(
                        &self.device,&self.bind_group,&self.collision_layout,&self.indirect,
                        Some(&self.scratch),off,key,&commands));
                }
                let buffers=[&self.bodies,&self.contacts,&self.contact_persistent,&self.contact_prepared,
                    &self.joints,&self.scratch,&self.atom,&self.query];
                enc.transition_resources(buffers.into_iter().map(|buffer|wgpu::BufferTransition{
                    buffer,state:wgpu::BufferUses::STORAGE_READ_WRITE}).chain([
                        wgpu::BufferTransition{buffer:&self.body_cold,state:wgpu::BufferUses::STORAGE_READ_ONLY},
                        wgpu::BufferTransition{buffer:&self.pass_lut,state:wgpu::BufferUses::UNIFORM},
                        wgpu::BufferTransition{buffer:&self.indirect,state:wgpu::BufferUses::INDIRECT},
                    ]),std::iter::empty());
                self.radix_cache.as_ref().unwrap().encode(enc);
                return;
            }
            {
                let mut pass=enc.begin_compute_pass(&wgpu::ComputePassDescriptor{label:Some("pair-matrix-build"),timestamp_writes:None});
                pass.set_bind_group(0,self.ping_bg(),&[off]);
                pass.set_pipeline(&self.clear_broadphase);pass.dispatch_workgroups(cg.max(1),1,1);
                pass.set_pipeline(&pipelines[0]);pass.dispatch_workgroups((self.shape_count*self.shape_count.div_ceil(32)).div_ceil(64).max(1),1,1);
            }
            {
                let mut pass=enc.begin_compute_pass(&wgpu::ComputePassDescriptor{label:Some("pair-matrix-compact"),timestamp_writes:None});
                pass.set_bind_group(0,self.ping_bg(),&[off]);
                // Bounded capacity dispatch avoids sharing native cached indirect arguments.
                // previous_pair_key rejects lanes beyond the GPU occupied count.
                pass.set_pipeline(&pipelines[1]);pass.dispatch_workgroups(self.contact_groups().max(1),1,1);
                pass.set_pipeline(&pipelines[2]);pass.dispatch_workgroups(shape_groups.max(1),1,1);
                pass.set_pipeline(&pipelines[3]);pass.dispatch_workgroups(1,1,1);
                pass.set_pipeline(&pipelines[4]);pass.dispatch_workgroups(shape_groups.max(1),1,1);
            }
            return;
        }
        #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
        if std::env::var("GPU_PHYSICS_NATIVE_RADIX_CACHE").as_deref() == Ok("2") {
            use crate::native_command_cache::Command::{Dispatch,Indirect,CopyArgs};
            let key=[2,shape_groups.max(1),cg.max(1)];
            if self.radix_cache.as_ref().is_none_or(|c| c.group!=self.bind_group || c.key!=key) {
                // All three private indirect producers clamp before writing:
                // inserts <= MAX_INSERTS, occupied <= contact_capacity <= PAIR_CAP,
                // radix <= PAIR_CAP. No user-provided dispatch arguments enter here.
                assert!(self.params.contact_capacity<=crate::types::PAIR_CAP);
                assert!(crate::types::MAX_INSERTS.div_ceil(64)<=self.device.limits().max_compute_workgroups_per_dimension);
                assert!(crate::types::PAIR_CAP.div_ceil(64)<=self.device.limits().max_compute_workgroups_per_dimension);
                let mut commands=vec![
                    Dispatch(&self.clear_broadphase,cg.max(1)),
                    Dispatch(&self.collect_fat_statics,shape_groups.max(1)),
                    Dispatch(&self.finish_fat_statics,1),
                    Dispatch(&self.emit_static_pairs,shape_groups.max(1)),
                    Dispatch(&self.hash_insert,shape_groups.max(1)),
                    Dispatch(&self.write_insert_indirect,1),
                    Dispatch(&self.write_occupied_indirect,1),
                    CopyArgs{source:8*4,destination:0,bytes:32},
                    Indirect(&self.emit_hash_pairs,0),Indirect(&self.emit_prev_pairs,16),
                    Dispatch(&self.write_radix_indirect,1),
                    CopyArgs{source:48*4,destination:Self::indirect_radix_offset(),bytes:16},
                ];
                for digit in 0..4 {
                    commands.extend([
                        Indirect(&self.radix_histogram[digit],Self::indirect_radix_offset()),
                        Dispatch(&self.radix_bucket_bases,1),Dispatch(&self.radix_group_prefix,1),
                        Indirect(&self.radix_scatter[digit],Self::indirect_radix_offset()),
                    ]);
                }
                commands.extend([
                    Indirect(&self.compact_unique_histogram,Self::indirect_radix_offset()),
                    Dispatch(&self.compact_unique_bases,1),
                    Indirect(&self.compact_unique_scatter,Self::indirect_radix_offset()),
                    Indirect(&self.compact_unique_gather,Self::indirect_radix_offset()),
                ]);
                self.radix_cache=Some(crate::native_command_cache::RadixCache::record(
                    &self.device,&self.bind_group,&self.collision_layout,&self.indirect,
                    Some(&self.scratch),off,key,&commands));
            }
            let buffers=[&self.bodies,&self.contacts,&self.contact_persistent,&self.contact_prepared,
                &self.joints,&self.scratch,&self.atom,&self.query];
            enc.transition_resources(buffers.into_iter().map(|buffer|wgpu::BufferTransition{
                buffer,state:wgpu::BufferUses::STORAGE_READ_WRITE}).chain([
                    wgpu::BufferTransition{buffer:&self.body_cold,state:wgpu::BufferUses::STORAGE_READ_ONLY},
                    wgpu::BufferTransition{buffer:&self.pass_lut,state:wgpu::BufferUses::UNIFORM},
                    wgpu::BufferTransition{buffer:&self.indirect,state:wgpu::BufferUses::INDIRECT},
                ]),std::iter::empty());
            self.radix_cache.as_ref().unwrap().encode(enc);
            return;
        }
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("physics-broadphase-build"),
                timestamp_writes: None,
            });
            pass.set_bind_group(0, self.ping_bg(), &[off]);
            pass.set_pipeline(&self.clear_broadphase);
            pass.dispatch_workgroups(cg.max(1), 1, 1);
            pass.set_pipeline(&self.collect_fat_statics);
            pass.dispatch_workgroups(shape_groups.max(1), 1, 1);
            pass.set_pipeline(&self.finish_fat_statics);
            pass.dispatch_workgroups(1, 1, 1);
            pass.set_pipeline(&self.emit_static_pairs);
            pass.dispatch_workgroups(shape_groups.max(1), 1, 1);
            pass.set_pipeline(&self.hash_insert);
            pass.dispatch_workgroups(shape_groups.max(1), 1, 1);
            pass.set_pipeline(&self.write_insert_indirect);
            pass.dispatch_workgroups(1, 1, 1);
            pass.set_pipeline(&self.write_occupied_indirect);
            pass.dispatch_workgroups(1, 1, 1);
        }
        enc.copy_buffer_to_buffer(&self.scratch, 8 * 4, &self.indirect, 0, 32);
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("physics-broadphase-pairs"),
                timestamp_writes: None,
            });
            pass.set_bind_group(0, self.ping_bg(), &[off]);
            pass.set_pipeline(&self.emit_hash_pairs);
            pass.dispatch_workgroups_indirect(&self.indirect, 0);
            pass.set_pipeline(&self.emit_prev_pairs);
            pass.dispatch_workgroups_indirect(&self.indirect, 16);
            pass.set_pipeline(&self.write_radix_indirect);
            pass.dispatch_workgroups(1, 1, 1);
        }
        enc.copy_buffer_to_buffer(
            &self.scratch,
            48 * 4,
            &self.indirect,
            Self::indirect_radix_offset(),
            16,
        );
        #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
        if std::env::var("GPU_PHYSICS_NATIVE_RADIX_CACHE").as_deref() == Ok("1") {
            if self.radix_cache.as_ref().is_none_or(|c|c.group != self.bind_group || c.key != [1,0,0]) {
                let mut commands=Vec::new();
                for digit in 0..4 {
                    commands.extend([(&self.radix_histogram[digit],true),(&self.radix_bucket_bases,false),
                        (&self.radix_group_prefix,false),(&self.radix_scatter[digit],true)]);
                }
                commands.extend([(&self.compact_unique_histogram,true),(&self.compact_unique_bases,false),
                    (&self.compact_unique_scatter,true),(&self.compact_unique_gather,true)]);
                self.radix_cache=Some(crate::native_command_cache::RadixCache::new(&self.device,&self.bind_group,
                    &self.collision_layout,&self.indirect,off,Self::indirect_radix_offset(),&commands));
            }
            let buffers=[&self.bodies,&self.contacts,&self.contact_persistent,&self.contact_prepared,
                &self.joints,&self.scratch,&self.atom,&self.query];
            enc.transition_resources(buffers.into_iter().map(|buffer|wgpu::BufferTransition{
                buffer,state:wgpu::BufferUses::STORAGE_READ_WRITE}).chain([
                    wgpu::BufferTransition{buffer:&self.body_cold,state:wgpu::BufferUses::STORAGE_READ_ONLY},
                    wgpu::BufferTransition{buffer:&self.pass_lut,state:wgpu::BufferUses::UNIFORM},
                    wgpu::BufferTransition{buffer:&self.indirect,state:wgpu::BufferUses::INDIRECT},
                ]),std::iter::empty());
            self.radix_cache.as_ref().unwrap().encode(enc);
            return;
        }
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("physics-broadphase-radix"),
                timestamp_writes: None,
            });
            pass.set_bind_group(0, self.ping_bg(), &[off]);
            for digit in 0..4 {
                pass.set_pipeline(&self.radix_histogram[digit]);
                pass.dispatch_workgroups_indirect(&self.indirect, Self::indirect_radix_offset());
                pass.set_pipeline(&self.radix_bucket_bases);
                pass.dispatch_workgroups(1, 1, 1);
                pass.set_pipeline(&self.radix_group_prefix);
                pass.dispatch_workgroups(1, 1, 1);
                pass.set_pipeline(&self.radix_scatter[digit]);
                pass.dispatch_workgroups_indirect(&self.indirect, Self::indirect_radix_offset());
            }
            pass.set_pipeline(&self.compact_unique_histogram);
            pass.dispatch_workgroups_indirect(&self.indirect, Self::indirect_radix_offset());
            pass.set_pipeline(&self.compact_unique_bases);
            pass.dispatch_workgroups(1, 1, 1);
            pass.set_pipeline(&self.compact_unique_scatter);
            pass.dispatch_workgroups_indirect(&self.indirect, Self::indirect_radix_offset());
            pass.set_pipeline(&self.compact_unique_gather);
            pass.dispatch_workgroups_indirect(&self.indirect, Self::indirect_radix_offset());
        }
    }

    fn radix256_groups(count: u32) -> u32 {
        count.div_ceil(crate::types::RADIX_GROUP_SIZE).max(1)
    }

    #[allow(dead_code)]
    fn unique_pair_groups(&self) -> u32 {
        Self::radix256_groups(crate::types::PAIR_CAP)
    }

    fn copy_radix_indirect(&self, enc: &mut wgpu::CommandEncoder) {
        enc.copy_buffer_to_buffer(
            &self.scratch,
            48 * 4,
            &self.indirect,
            Self::indirect_radix_offset(),
            16,
        );
    }

    fn copy_prepare_indirect(&self, enc: &mut wgpu::CommandEncoder) {
        enc.copy_buffer_to_buffer(
            &self.scratch,
            40 * 4,
            &self.indirect,
            Self::indirect_prepare_offset(),
            16,
        );
    }

    fn narrowphase_detect_pass(&mut self, enc: &mut wgpu::CommandEncoder) {
        let off = Self::pass_lut_offset(0, 1) as u32;
        #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
        if std::env::var("GPU_PHYSICS_NATIVE_CONTACT_CACHE").as_deref() == Ok("1") {
            use crate::native_command_cache::Command::{Dispatch,Indirect,CopyArgs};
            let key=[3,u32::from(self.shape_count>=2),u32::from(self.params.mesh_triangle_count!=0)];
            if self.contact_cache.as_ref().is_none_or(|c|c.group!=self.bind_group || c.key!=key) {
                // Private writers: unique/missing <= PAIR_CAP, free slots <= contact_capacity.
                // Same writer kernels and ordered allocation as the wgpu path below.
                assert!(self.params.contact_capacity<=crate::types::PAIR_CAP);
                assert!(crate::types::PAIR_CAP.div_ceil(64)<=self.device.limits().max_compute_workgroups_per_dimension);
                let radix=Self::indirect_radix_offset();
                let prepare=Self::indirect_prepare_offset();
                let mut commands=vec![
                    CopyArgs{source:40*4,destination:prepare,bytes:16},
                    Indirect(&self.find_existing_contact_slots,prepare),
                    Dispatch(&self.write_unique_indirect,1),
                    CopyArgs{source:48*4,destination:radix,bytes:16},
                    Indirect(&self.alloc_missing_histogram,radix),
                    Dispatch(&self.alloc_missing_bases,1),
                    Indirect(&self.alloc_missing_scatter,radix),
                    Dispatch(&self.write_alloc_indirects,1),
                    CopyArgs{source:48*4,destination:radix,bytes:16},
                    CopyArgs{source:40*4,destination:prepare,bytes:16},
                    Indirect(&self.alloc_free_histogram,radix),
                    Dispatch(&self.alloc_free_bases,1),
                    Indirect(&self.alloc_free_scatter,radix),
                    Dispatch(&self.write_unique_indirect,1),
                    Indirect(&self.alloc_bind_slots,prepare),
                    CopyArgs{source:8*4,destination:0,bytes:16},
                ];
                if self.shape_count>=2 {commands.push(Indirect(self.collision_pipeline(),0));}
                self.contact_cache=Some(crate::native_command_cache::RadixCache::record(
                    &self.device,&self.bind_group,&self.collision_layout,&self.indirect,
                    Some(&self.scratch),off,key,&commands));
            }
            let buffers=[&self.bodies,&self.contacts,&self.contact_persistent,&self.contact_prepared,
                &self.joints,&self.scratch,&self.atom,&self.query];
            enc.transition_resources(buffers.into_iter().map(|buffer|wgpu::BufferTransition{
                buffer,state:wgpu::BufferUses::STORAGE_READ_WRITE}).chain([
                    wgpu::BufferTransition{buffer:&self.body_cold,state:wgpu::BufferUses::STORAGE_READ_ONLY},
                    wgpu::BufferTransition{buffer:&self.pass_lut,state:wgpu::BufferUses::UNIFORM},
                    wgpu::BufferTransition{buffer:&self.indirect,state:wgpu::BufferUses::INDIRECT},
                ]),std::iter::empty());
            self.contact_cache.as_ref().unwrap().encode(enc);
            return;
        }
        self.copy_prepare_indirect(enc);
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("physics-contact-lookup"),
                timestamp_writes: None,
            });
            pass.set_bind_group(0, self.ping_bg(), &[off]);
            pass.set_pipeline(&self.find_existing_contact_slots);
            pass.dispatch_workgroups_indirect(&self.indirect, Self::indirect_prepare_offset());
            pass.set_pipeline(&self.write_unique_indirect);
            pass.dispatch_workgroups(1, 1, 1);
        }
        self.copy_radix_indirect(enc);
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("physics-contact-missing"),
                timestamp_writes: None,
            });
            pass.set_bind_group(0, self.ping_bg(), &[off]);
            pass.set_pipeline(&self.alloc_missing_histogram);
            pass.dispatch_workgroups_indirect(&self.indirect, Self::indirect_radix_offset());
            pass.set_pipeline(&self.alloc_missing_bases);
            pass.dispatch_workgroups(1, 1, 1);
            // Persist the ordered missing-pair list before free-slot scans reuse bases.
            pass.set_pipeline(&self.alloc_missing_scatter);
            pass.dispatch_workgroups_indirect(&self.indirect, Self::indirect_radix_offset());
            pass.set_pipeline(&self.write_alloc_indirects);
            pass.dispatch_workgroups(1, 1, 1);
        }
        self.copy_radix_indirect(enc);
        self.copy_prepare_indirect(enc);
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("physics-contact-free-slots"),
                timestamp_writes: None,
            });
            pass.set_bind_group(0, self.ping_bg(), &[off]);
            pass.set_pipeline(&self.alloc_free_histogram);
            pass.dispatch_workgroups_indirect(&self.indirect, Self::indirect_radix_offset());
            pass.set_pipeline(&self.alloc_free_bases);
            pass.dispatch_workgroups(1, 1, 1);
            pass.set_pipeline(&self.alloc_free_scatter);
            pass.dispatch_workgroups_indirect(&self.indirect, Self::indirect_radix_offset());
            // Graph compaction consumes this scratch indirect count after narrowphase,
            // even though allocation no longer needs to copy it a second time.
            pass.set_pipeline(&self.write_unique_indirect);
            pass.dispatch_workgroups(1, 1, 1);
        }
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("physics-contact-bind"),
                timestamp_writes: None,
            });
            pass.set_bind_group(0, self.ping_bg(), &[off]);
            pass.set_pipeline(&self.alloc_bind_slots);
            pass.dispatch_workgroups_indirect(&self.indirect, Self::indirect_prepare_offset());
        }
        enc.copy_buffer_to_buffer(&self.scratch, 8 * 4, &self.indirect, 0, 16);
        // No pair can exist with fewer than two live collider slots. Allocation
        // and retirement still run, so removing the last shape clears history.
        if self.shape_count >= 2 {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("physics-narrowphase"),
                timestamp_writes: None,
            });
            pass.set_bind_group(0, self.ping_bg(), &[off]);
            pass.set_pipeline(self.collision_pipeline());
            pass.dispatch_workgroups_indirect(&self.indirect, 0);
        }
    }

    fn make_runtime_compute(&self, device: &Device, layout: &wgpu::PipelineLayout, shader: &ShaderModule, entry: &str) -> ComputePipeline {
        let p = make_compute_cached(device, layout, shader, entry, self.startup_cache.cache.as_ref());
        self.startup_cache.save();
        p
    }
    fn make_runtime_compute_with_constant(&self, device: &Device, layout: &wgpu::PipelineLayout, shader: &ShaderModule, entry: &str, name: &str, value: f64) -> ComputePipeline {
        let p = make_compute_constant_cached(device, layout, shader, entry, name, value, self.startup_cache.cache.as_ref());
        self.startup_cache.save();
        p
    }

    pub(crate) fn collision_pipeline_ready(&self) -> bool {
        if self.params.mesh_triangle_count != 0 { self.collide_pairs_mesh.get().is_some() }
        else { self.collide_pairs_no_mesh.get().is_some() }
    }

    /// Prepare the lazy collision variant without dispatching or advancing time.
    pub(crate) fn prepare_collision_pipeline(&self) { let _ = self.collision_pipeline(); }

    fn collision_pipeline(&self) -> &ComputePipeline {
        // Counts describe live packed geometry, not reserved buffer capacity.
        // A later mesh upload must select the general variant even when buffers
        // did not grow; each variant is compiled at most once per simulation.
        let mesh = self.params.mesh_triangle_count != 0;
        let cache = if mesh { &self.collide_pairs_mesh } else { &self.collide_pairs_no_mesh };
        cache.get_or_init(|| self.make_runtime_compute_with_constant(
            &self.device, &self.collision_layout, &self.collision_shader,
            "collide_pairs", "COLLISION_MESH_ENABLED", if mesh { 1.0 } else { 0.0 },
        ))
    }

    fn collide_detect_pass(&mut self, enc: &mut wgpu::CommandEncoder) {
        self.broadphase_candidates_pass(enc);
        self.narrowphase_detect_pass(enc);
    }

    #[cfg(test)]
    pub(crate) fn set_small_component_group_test(&mut self,size:u32) {
        assert!(matches!(size,8|16|32|64));
        self.solve_complete_components=self.make_runtime_compute_with_constant(&self.device,&self.collision_layout,
            &self.collision_shader,"solve_complete_components","SMALL_COMPONENT_WORKGROUP_SIZE",f64::from(size));
        self.small_component_workgroup_size=size;
    }

    #[cfg(test)]
    pub(crate) fn set_shared_graph_test(&mut self, enabled:bool) { self.graph_shared_requested=enabled; }
    #[cfg(test)]
    pub(crate) fn shared_graph_initialized(&self) -> bool { self.graph_assign_shared.get().is_some() || self.graph_assign_memo.get().is_some() }

    fn shared_graph_eligible(&self) -> bool {
        self.graph_shared_requested && self.params.body_count <= 8168
            && self.device.limits().max_compute_workgroup_storage_size >= 32768
    }

    fn dynamic_graph_pipeline(&self) -> &ComputePipeline {
        if !self.shared_graph_eligible() { return &self.graph_assign_dynamic; }
        if let Some(base)=self.graph_memo_base.filter(|_|self.params.body_count<=8160 && self.params.joint_count==0 && self.params.mesh_triangle_count==0) {
            return self.graph_assign_memo.get_or_init(|| {
                let source=format!("{}\n{}",PHYSICS_WGSL,include_str!("../shaders/physics/graph_memo.wgsl"));
                let shader=self.device.create_shader_module(wgpu::ShaderModuleDescriptor {
                    label:Some("physics-memo-graph"),source:wgpu::ShaderSource::Wgsl(source.into()),
                });
                self.make_runtime_compute_with_constant(&self.device,&self.collision_layout,&shader,
                    "graph_assign_dynamic_memo","GRAPH_MEMO_BASE",f64::from(base))
            });
        }
        self.graph_assign_shared.get_or_init(|| {
            // The extra entry is absent on unsupported devices; its only
            // workgroup allocation is 8168 occupancy masks + 24 color counters (32 KiB).
            let source=format!("{}\n{}",PHYSICS_WGSL,include_str!("../shaders/physics/graph_shared.wgsl"));
            let shader=self.device.create_shader_module(wgpu::ShaderModuleDescriptor {
                label:Some("physics-shared-graph"),source:wgpu::ShaderSource::Wgsl(source.into()),
            });
            self.make_runtime_compute(&self.device,&self.collision_layout,&shader,"graph_assign_dynamic_shared")
        })
    }

    fn paired_graph_pipelines(&self)->&[ComputePipeline;3] {
        self.graph_compact_paired.get_or_init(|| {
            assert!(2*crate::types::RADIX_GROUPS<=crate::types::MAX_INSERTS);
            ["graph_compact_paired_histogram","graph_compact_paired_bases","graph_compact_paired_scatter"]
                .map(|entry|self.make_runtime_compute(&self.device,&self.collision_layout,&self.collision_shader,entry))
        })
    }

    fn collide_finalize_pass(&mut self, enc: &mut wgpu::CommandEncoder) {
        let off = Self::pass_lut_offset(0, 1) as u32;
        let rebuild = self.params.diagnostic_flags & crate::types::DIAG_REBUILD_GRAPH != 0;
        #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
        if std::env::var("GPU_PHYSICS_NATIVE_GRAPH_CACHE").as_deref()==Ok("1") {
            use crate::native_command_cache::Command::{Dispatch,Indirect,CopyArgs};
            let shared=self.shared_graph_eligible();
            let memo=shared && self.graph_memo_base.is_some() && self.params.body_count<=8160
                && self.params.joint_count==0 && self.params.mesh_triangle_count==0;
            // Memo base is immutable within a GpuSim. Buffer replacement is keyed by bind group.
            let flags=4 | (u32::from(rebuild)<<4) | (u32::from(self.skip_general_static_sort)<<5)
                | (u32::from(shared)<<6) | (u32::from(memo)<<7)
                | (u32::from(self.pair_matrix_used)<<8)
                | (if self.pair_matrix_used {self.contact_groups().max(1)<<10} else {0});
            let key=[flags,self.body_groups().max(1),self.params.joint_count.div_ceil(WORKGROUP_SIZE)];
            if self.graph_cache.as_ref().is_none_or(|c|c.group!=self.bind_group || c.key!=key) {
                assert!(self.params.contact_capacity<=crate::types::PAIR_CAP);
                assert!(crate::types::PAIR_CAP.div_ceil(64)<=self.device.limits().max_compute_workgroups_per_dimension);
                let radix=Self::indirect_radix_offset();let statics=Self::indirect_static_offset();
                let mut commands=Vec::new();
                if rebuild {commands.push(Dispatch(&self.color_and_compact,1));}
                else {
                    commands.extend([Dispatch(&self.graph_reset_colors,1),Dispatch(&self.graph_clear_meta,key[1]),Indirect(&self.graph_classify,0)]);
                    if key[2]>0 {commands.push(Dispatch(&self.graph_mark_edges,key[2]));}
                }
                commands.push(CopyArgs{source:48*4,destination:radix,bytes:16});
                if !rebuild {
                    let paired=self.paired_graph_pipelines();
                    commands.extend([Indirect(&paired[0],radix),Dispatch(&paired[1],1),Indirect(&paired[2],radix),
                        Indirect(&self.graph_count_static_degree,0),Dispatch(&self.graph_finish_static_degree,1),
                        CopyArgs{source:48*4,destination:radix,bytes:16},
                        CopyArgs{source:59*4,destination:statics,bytes:16},Indirect(&self.graph_pack_static_keys,statics)]);
                    if !self.skip_general_static_sort {
                        for digit in 0..4 {commands.extend([
                            Indirect(&self.graph_radix_histogram[digit],radix),Dispatch(&self.radix_bucket_bases,1),
                            Dispatch(&self.radix_group_prefix,1),Indirect(&self.graph_radix_scatter[digit],radix)]);}
                        commands.extend([Indirect(&self.graph_mark_static_starts,statics),Indirect(&self.graph_encode_static_colors,statics)]);
                        for digit in 0..4 {commands.extend([
                            Indirect(&self.graph_radix_histogram[digit],radix),Dispatch(&self.radix_bucket_bases,1),
                            Dispatch(&self.radix_group_prefix,1),Indirect(&self.graph_radix_scatter[digit],radix)]);}
                        commands.push(Indirect(&self.graph_mark_color_starts,statics));
                    }
                    commands.extend([Indirect(&self.graph_assign_static,statics),Dispatch(self.dynamic_graph_pipeline(),1)]);
                }
                commands.extend([Dispatch(&self.begin_occupied_contacts,1),
                    if self.pair_matrix_used { Dispatch(&self.retire_stale_contacts,self.contact_groups().max(1)) }
                    else {Indirect(&self.retire_stale_contacts,16)},
                    Indirect(&self.collect_occupied_contacts,0),Dispatch(&self.finish_occupied_contacts,1),
                    CopyArgs{source:8*4,destination:0,bytes:16},
                    CopyArgs{source:64*4,destination:16,bytes:u64::from(crate::types::MAX_COLORS)*16},
                    CopyArgs{source:40*4,destination:Self::indirect_prepare_offset(),bytes:16},
                    CopyArgs{source:44*4,destination:Self::indirect_island_offset(),bytes:16}]);
                self.graph_cache=Some(crate::native_command_cache::RadixCache::record(
                    &self.device,&self.bind_group,&self.collision_layout,&self.indirect,Some(&self.scratch),
                    off,key,&commands));
            }
            if !rebuild && !self.skip_general_static_sort {
                self.static_sort_dispatches.set(self.static_sort_dispatches.get().saturating_add(32));
            }
            let buffers=[&self.bodies,&self.contacts,&self.contact_persistent,&self.contact_prepared,
                &self.joints,&self.scratch,&self.atom,&self.query];
            enc.transition_resources(buffers.into_iter().map(|buffer|wgpu::BufferTransition{
                buffer,state:wgpu::BufferUses::STORAGE_READ_WRITE}).chain([
                    wgpu::BufferTransition{buffer:&self.body_cold,state:wgpu::BufferUses::STORAGE_READ_ONLY},
                    wgpu::BufferTransition{buffer:&self.pass_lut,state:wgpu::BufferUses::UNIFORM},
                    wgpu::BufferTransition{buffer:&self.indirect,state:wgpu::BufferUses::INDIRECT},
                ]),std::iter::empty());
            self.graph_cache.as_ref().unwrap().encode(enc);
            return;
        }
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("physics-contact-color"),
                timestamp_writes: None,
            });
            pass.set_bind_group(0, self.ping_bg(), &[off]);
            if rebuild {
                pass.set_pipeline(&self.color_and_compact);
                pass.dispatch_workgroups(1, 1, 1);
            } else {
                pass.set_pipeline(&self.graph_reset_colors);
                pass.dispatch_workgroups(1, 1, 1);
                pass.set_pipeline(&self.graph_clear_meta);
                pass.dispatch_workgroups(self.body_groups().max(1), 1, 1);
                pass.set_pipeline(&self.graph_classify);
                pass.dispatch_workgroups_indirect(&self.indirect, 0);
                if self.params.joint_count > 0 {
                    pass.set_pipeline(&self.graph_mark_edges);
                    pass.dispatch_workgroups(
                        self.params.joint_count.div_ceil(WORKGROUP_SIZE),
                        1,
                        1,
                    );
                }
            }
        }
        self.copy_radix_indirect(enc);
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("physics-contact-color-compact"),
                timestamp_writes: None,
            });
            pass.set_bind_group(0, self.ping_bg(), &[off]);
            if !rebuild {
                let pipelines=self.paired_graph_pipelines();
                pass.set_pipeline(&pipelines[0]);
                pass.dispatch_workgroups_indirect(&self.indirect,Self::indirect_radix_offset());
                pass.set_pipeline(&pipelines[1]);pass.dispatch_workgroups(1,1,1);
                pass.set_pipeline(&pipelines[2]);
                pass.dispatch_workgroups_indirect(&self.indirect,Self::indirect_radix_offset());
                pass.set_pipeline(&self.graph_count_static_degree);
                pass.dispatch_workgroups_indirect(&self.indirect, 0);
                pass.set_pipeline(&self.graph_finish_static_degree);
                pass.dispatch_workgroups(1, 1, 1);
            }
        }
        if !rebuild {
            self.copy_radix_indirect(enc);
            enc.copy_buffer_to_buffer(
                &self.scratch,
                59 * 4,
                &self.indirect,
                Self::indirect_static_offset(),
                16,
            );
            {
                let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                    label: Some("physics-contact-color-static"),
                    timestamp_writes: None,
                });
                pass.set_bind_group(0, self.ping_bg(), &[off]);
                pass.set_pipeline(&self.graph_pack_static_keys);
                pass.dispatch_workgroups_indirect(&self.indirect, Self::indirect_static_offset());
                if !self.skip_general_static_sort {
                    self.static_sort_dispatches.set(
                        self.static_sort_dispatches
                            .get()
                            .saturating_add(32),
                    );
                    for digit in 0..4 {
                        pass.set_pipeline(&self.graph_radix_histogram[digit]);
                        pass.dispatch_workgroups_indirect(
                            &self.indirect,
                            Self::indirect_radix_offset(),
                        );
                        pass.set_pipeline(&self.radix_bucket_bases);
                        pass.dispatch_workgroups(1, 1, 1);
                        pass.set_pipeline(&self.radix_group_prefix);
                        pass.dispatch_workgroups(1, 1, 1);
                        pass.set_pipeline(&self.graph_radix_scatter[digit]);
                        pass.dispatch_workgroups_indirect(
                            &self.indirect,
                            Self::indirect_radix_offset(),
                        );
                    }
                    pass.set_pipeline(&self.graph_mark_static_starts);
                    pass.dispatch_workgroups_indirect(
                        &self.indirect,
                        Self::indirect_static_offset(),
                    );
                    pass.set_pipeline(&self.graph_encode_static_colors);
                    pass.dispatch_workgroups_indirect(
                        &self.indirect,
                        Self::indirect_static_offset(),
                    );
                    for digit in 0..4 {
                        pass.set_pipeline(&self.graph_radix_histogram[digit]);
                        pass.dispatch_workgroups_indirect(
                            &self.indirect,
                            Self::indirect_radix_offset(),
                        );
                        pass.set_pipeline(&self.radix_bucket_bases);
                        pass.dispatch_workgroups(1, 1, 1);
                        pass.set_pipeline(&self.radix_group_prefix);
                        pass.dispatch_workgroups(1, 1, 1);
                        pass.set_pipeline(&self.graph_radix_scatter[digit]);
                        pass.dispatch_workgroups_indirect(
                            &self.indirect,
                            Self::indirect_radix_offset(),
                        );
                    }
                    pass.set_pipeline(&self.graph_mark_color_starts);
                    pass.dispatch_workgroups_indirect(
                        &self.indirect,
                        Self::indirect_static_offset(),
                    );
                }
                pass.set_pipeline(&self.graph_assign_static);
                pass.dispatch_workgroups_indirect(&self.indirect, Self::indirect_static_offset());
                pass.set_pipeline(self.dynamic_graph_pipeline());
                pass.dispatch_workgroups(1, 1, 1);
            }
        }
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("physics-contact-color-occupied"),
                timestamp_writes: None,
            });
            pass.set_bind_group(0, self.ping_bg(), &[off]);
            pass.set_pipeline(&self.begin_occupied_contacts);
            pass.dispatch_workgroups(1, 1, 1);
            pass.set_pipeline(&self.retire_stale_contacts);
            if self.pair_matrix_used {pass.dispatch_workgroups(self.contact_groups().max(1),1,1);}
            else {pass.dispatch_workgroups_indirect(&self.indirect,16);}
            pass.set_pipeline(&self.collect_occupied_contacts);
            pass.dispatch_workgroups_indirect(&self.indirect, 0);
            pass.set_pipeline(&self.finish_occupied_contacts);
            pass.dispatch_workgroups(1, 1, 1);
        }
        enc.copy_buffer_to_buffer(&self.scratch, 8 * 4, &self.indirect, 0, 16);
        enc.copy_buffer_to_buffer(
            &self.scratch,
            64 * 4,
            &self.indirect,
            16,
            u64::from(crate::types::MAX_COLORS) * 16,
        );
        self.copy_solver_indirects(enc);
    }

    fn dispatch_colored_solve(&self, encoder: &mut wgpu::CommandEncoder, use_bias: u32) {
        self.dispatch_wave(encoder, use_bias);
    }

    fn capture_phase(&self, encoder: &mut wgpu::CommandEncoder, phase: u32) {
        if self.params.diagnostic_flags & DIAG_PHASE_CAPTURE != 0 {
            self.dispatch_live(
                encoder,
                &self.capture_phase,
                self.body_groups(),
                phase,
                0,
                None,
            );
        }
    }

    /// One Box3D world step: collide once, then TGS substeps, then apply deltas.
    pub fn world_step(&mut self, sub_steps: i32) {
        #[cfg(not(target_arch = "wasm32"))]
        self.harvest_gpu_timestamps(false);
        if self.physics_invalid {
            return;
        }
        if self.params.step_dt <= 0.0 {
            return;
        }
        let eligible = self.idle_eligible && self.params.enable_sleep!=0
            && self.params.diagnostic_flags & (DIAG_DISABLE_SLEEP|DIAG_PHASE_CAPTURE|DIAG_REBUILD_GRAPH)==0;
        let epoch = self.idle_epoch.get();
        if !eligible || self.idle_chain.is_some_and(|(_,last,context,e)|
            last!=self.physics_step || context!=self.idle_input_context || e!=epoch) {
            self.idle_chain=None;
        }
        let idle = eligible && self.idle_proof.is_some_and(|(step,context,e)| {
            e==epoch && (step==self.physics_step && context==self.idle_input_context
                || self.idle_chain.is_some_and(|(first,last,output,chain_epoch)|
                    step>=first && step<=last && last==self.physics_step
                    && output==self.idle_input_context && context[0]==output[0] && chain_epoch==e))
        });
        self.last_step_idle=idle;
        self.idle_count_valid_step=None;
        if !idle {self.idle_proof=None;}
        self.physics_step = self.physics_step.saturating_add(1);
        self.params.physics_step = self.physics_step as u32;
        self.solver_dispatches.set(0);
        self.joint_dispatches.set(0);
        self.static_sort_dispatches.set(0);
        self.encode_commands.set(0);
        #[cfg(not(target_arch = "wasm32"))]
        let encode_start = Instant::now();
        let metric_step = self
            .metrics
            .as_ref()
            .filter(|metrics| metrics.submitted < metrics.steps)
            .map(|metrics| metrics.submitted);
        let mut enc = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("world-step"),
            });
        if idle {
            // Real submission/timestamps/pose copies still follow below. Only
            // physics work is absent: the confirmed state has not changed.
            for index in 0..TIMESTAMP_QUERY_COUNT-1 {self.write_pass_timestamp(&mut enc,metric_step,index);}
            self.idle_proof=Some((self.physics_step,self.idle_output_context,self.idle_epoch.get()));
            self.idle_count_valid_step=Some(self.physics_step);
        } else {
        if let Some(ccd)=&self.convex_ccd { ccd.capture(&mut enc,&self.bodies); }
        self.params.sub_step_count = sub_steps.max(1) as u32;
        self.upload_pass_lut();
        self.ping_dispatch(
            &mut enc,
            &self.reset_deltas.clone(),
            self.body_groups(),
            0,
            1,
        );
        self.write_pass_timestamp(&mut enc, metric_step, 0);
        self.broadphase_candidates_pass(&mut enc);
        self.write_pass_timestamp(&mut enc, metric_step, 1);
        self.narrowphase_detect_pass(&mut enc);
        self.write_pass_timestamp(&mut enc, metric_step, 2);
        self.collide_finalize_pass(&mut enc);
        self.write_pass_timestamp(&mut enc, metric_step, 3);
        self.capture_phase(&mut enc, 0);
        let body_groups = self.body_groups();
        let island_body_groups = self.island_groups(self.count);
        self.dispatch_islands_and_prepare(&mut enc);
        self.prepare_joint_components(&mut enc);
        self.capture_phase(&mut enc, 1);
        if crate::types::ENABLE_FUSED_ISLANDS {
            self.dispatch_n(
                &mut enc,
                &self.solve_tiny_islands.clone(),
                body_groups,
                0,
                1,
            );
        }

        self.write_pass_timestamp(&mut enc, metric_step, 4);
        let nsub = sub_steps.max(1);
        let bg = body_groups;
        let has_joints = self.params.joint_count > 0;
        if self.component_tgs {
            if !self.try_native_tail(&mut enc,1) {
            self.dispatch_n(&mut enc, &self.component_reset, bg, 0, 1);
            self.dispatch_n(&mut enc, &self.component_count, bg.max(self.contact_groups()), 0, 1);
            self.dispatch_n(&mut enc, &self.component_offsets, 1, 0, 1);
            self.dispatch_n(&mut enc, &self.component_color_offsets, bg, 0, 1);
            self.dispatch_n(&mut enc, &self.component_scatter, bg.max(self.contact_groups()), 0, 1);
            let large_offset=(5 + crate::types::MAX_COLORS as u64)*16;
            enc.copy_buffer_to_buffer(&self.query,257*4,&self.indirect,large_offset,12);
            self.dispatch_n(&mut enc, &self.solve_complete_components.clone(), self.count.div_ceil(self.small_component_workgroup_size), 0, 1);
            {
                let mut pass=enc.begin_compute_pass(&wgpu::ComputePassDescriptor{label:Some("large-component-tgs"),timestamp_writes:None});
                pass.set_pipeline(&self.solve_large_components);
                pass.set_bind_group(0,self.live_bg(),&[Self::pass_lut_offset(0,1) as u32]);
                pass.dispatch_workgroups_indirect(&self.indirect,large_offset);
            }
            self.encode_commands.set(self.encode_commands.get().saturating_add(1));
            }
            self.solver_dispatches.set(2);
        } else if self.params.solver_mode == 1 {
            let joints = self.solve_joints.clone();
            let int_vel = self.integrate_vel.clone();
            let int_pos = self.integrate_pos.clone();
            for _sub_step in 0..nsub {
                self.ping_dispatch(&mut enc, &int_vel, bg, 0, 1);
                self.dispatch_colored_solve(&mut enc, 2);
                if has_joints {
                    self.copy_then(&mut enc, &joints, self.joint_solve_groups(), 0, 2);
                }
                self.dispatch_colored_solve(&mut enc, 1);
                if has_joints {
                    self.copy_then(&mut enc, &joints, self.joint_solve_groups(), 0, 1);
                }
                self.ping_dispatch(&mut enc, &int_pos, bg, 0, 1);
                self.dispatch_colored_solve(&mut enc, 0);
                if has_joints {
                    self.copy_then(&mut enc, &joints, self.joint_solve_groups(), 0, 0);
                }
            }
            self.dispatch_colored_solve(&mut enc, 3);
        } else {
            {
                let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                    label: Some("physics-tgs"),
                    timestamp_writes: None,
                });
                for _ in 0..nsub {
                    self.emit_n(&mut pass, &self.integrate_vel, bg, 0, 1);
                    self.emit_wave(&mut pass, 2);
                    if has_joints {
                        self.emit_n(&mut pass, &self.solve_joints, self.joint_solve_groups(), 0, 2);
                        self.joint_dispatches
                            .set(self.joint_dispatches.get().saturating_add(1));
                    }
                    self.emit_wave(&mut pass, 1);
                    if has_joints {
                        self.emit_n(&mut pass, &self.solve_joints, self.joint_solve_groups(), 0, 1);
                        self.joint_dispatches
                            .set(self.joint_dispatches.get().saturating_add(1));
                    }
                    self.emit_n(&mut pass, &self.integrate_pos, bg, 0, 1);
                    self.emit_wave(&mut pass, 0);
                    if has_joints {
                        self.emit_n(&mut pass, &self.solve_joints, self.joint_solve_groups(), 0, 0);
                        self.joint_dispatches
                            .set(self.joint_dispatches.get().saturating_add(1));
                    }
                }
                self.emit_wave(&mut pass, 3);
            }
        }
        self.capture_phase(&mut enc, 22);
        self.write_pass_timestamp(&mut enc, metric_step, 5);
        if !self.try_native_tail(&mut enc,2) {
        self.ping_dispatch(&mut enc, &self.apply_deltas.clone(), bg, 0, 1);
        // A body may only remain asleep when its complete contact/joint island
        // is ready. Preserve quiet-body timers until the whole island is ready.
        self.dispatch_n(
            &mut enc,
            &self.island_reset_ready.clone(),
            island_body_groups,
            0,
            1,
        );
        self.dispatch_n(
            &mut enc,
            &self.island_accumulate_ready.clone(),
            island_body_groups,
            0,
            1,
        );
        self.dispatch_n(
            &mut enc,
            &self.island_apply_sleep.clone(),
            island_body_groups,
            0,
            1,
        );
        }
        if let Some(ccd)=&self.convex_ccd { ccd.correct(&mut enc); }
        self.capture_phase(&mut enc, 23);
        self.write_pass_timestamp(&mut enc, metric_step, 6);

            if self.idle_eligible {
                enc.clear_buffer(&self.query,72*4,Some(4));
                self.dispatch_n(&mut enc,&self.count_active_bodies,self.body_groups(),0,1);
                self.idle_count_valid_step=Some(self.physics_step);
            }
        }
        if eligible && self.idle_count_valid_step==Some(self.physics_step) {
            let first=self.idle_chain.map_or(self.physics_step,|(first,_,_,_)|first);
            self.idle_chain=Some((first,self.physics_step,self.idle_output_context,epoch));
        } else {self.idle_chain=None;}
        self.write_pass_timestamp(&mut enc,metric_step,7);
        if let Some(step) = metric_step {
            if let Some(metrics) = &self.metrics {
                if let Some(ts) = &metrics.timestamp {
                    let first = step * TIMESTAMP_QUERY_COUNT;
                    enc.resolve_query_set(
                        &ts.queries,
                        first..first + TIMESTAMP_QUERY_COUNT,
                        &ts.resolve,
                        u64::from(step) * TIMESTAMP_STEP_STRIDE,
                    );
                }
                let workload_offset = u64::from(step) * WORKLOAD_STEP_STRIDE;
                enc.copy_buffer_to_buffer(&self.scratch, 0, &metrics.workload, workload_offset, 16);
                enc.copy_buffer_to_buffer(
                    &self.atom,
                    0,
                    &metrics.workload,
                    workload_offset + 16,
                    28,
                );
            }
        }
        #[cfg(not(target_arch = "wasm32"))]
        let mut ts_ring_slot: Option<usize> = None;
        #[cfg(not(target_arch = "wasm32"))]
        if let (Some(ts), Some(ring)) = (&self.step_timestamp, self.ts_ring.as_mut()) {
            if let Some(slot) = (0..2).find(|&i| ring.pending[i].is_none()) {
                enc.resolve_query_set(&ts.queries, 0..TIMESTAMP_QUERY_COUNT, &ts.resolve, 0);
                enc.copy_buffer_to_buffer(
                    &ts.resolve,
                    0,
                    &ring.staging[slot],
                    0,
                    TIMESTAMP_STEP_STRIDE,
                );
                ring.step[slot] = self.physics_step;
                ts_ring_slot = Some(slot);
            }
        }
        #[cfg(not(target_arch = "wasm32"))]
        if let Some(export) = &self.pose_export {
            let copy = export.size.min(self.bodies.size());
            if copy > 0 {
                enc.copy_buffer_to_buffer(&self.bodies, 0, &export.buffer, 0, copy);
            }
        }
        #[cfg(not(target_arch = "wasm32"))]
        self.encode_contact_status(&mut enc);
        #[cfg(feature = "native-command-cache")]
        if let Some(copy) = self.render_copy.take() {
            copy.encode(&mut enc, &self.queue, &self.bodies, &self.body_cold);
        }
        let command = enc.finish();
        #[cfg(not(target_arch = "wasm32"))]
        let encode_ms = encode_start.elapsed().as_secs_f64() * 1000.0;
        #[cfg(not(target_arch = "wasm32"))]
        let submit_start = Instant::now();
        self.record_submit(self.queue.submit(Some(command)));
        self.completed_known = false;
        #[cfg(not(target_arch = "wasm32"))]
        let submit_ms = submit_start.elapsed().as_secs_f64() * 1000.0;
        #[cfg(not(target_arch = "wasm32"))]
        if let Some(slot) = ts_ring_slot {
            if let Some(ring) = self.ts_ring.as_mut() {
                let slice = ring.staging[slot].slice(..TIMESTAMP_STEP_STRIDE);
                let (tx, rx) = oneshot();
                slice.map_async(wgpu::MapMode::Read, move |r| {
                    let _ = tx.send(r);
                });
                ring.pending[slot] = Some(rx);
            }
        }
        #[cfg(not(target_arch = "wasm32"))]
        self.map_contact_status();
        if metric_step.is_some() {
            if let Some(metrics) = self.metrics.as_mut() {
                metrics.submitted += 1;
                #[cfg(not(target_arch = "wasm32"))]
                {
                    metrics.cpu_encode_ms.push(encode_ms);
                    metrics.cpu_submit_ms.push(submit_ms);
                }
            }
        }
        #[cfg(not(target_arch = "wasm32"))]
        self.kick_pose_readback();
    }

    fn write_pass_timestamp(
        &self,
        enc: &mut wgpu::CommandEncoder,
        metric_step: Option<u32>,
        index: u32,
    ) {
        #[cfg(not(target_arch = "wasm32"))]
        if let Some(ts) = &self.step_timestamp {
            enc.write_timestamp(&ts.queries, index);
        }
        if let (Some(step), Some(ts)) = (
            metric_step,
            self.metrics
                .as_ref()
                .and_then(|metrics| metrics.timestamp.as_ref()),
        ) {
            enc.write_timestamp(&ts.queries, step * TIMESTAMP_QUERY_COUNT + index);
        }
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn gpu_pass_ms(&self) -> (f32, f32, f32) {
        (
            self.last_gpu_collide_ms,
            self.last_gpu_solve_ms,
            self.last_gpu_integrate_ms,
        )
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn gpu_stage_ms(&self) -> (f32, f32, f32) {
        (
            self.last_gpu_broadphase_ms,
            self.last_gpu_narrowphase_ms,
            self.last_gpu_graph_ms,
        )
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn last_gpu_prepare_ms(&self) -> f32 {
        self.last_gpu_prepare_ms
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn last_gpu_device_ms(&self) -> f32 {
        self.last_gpu_device_ms
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn last_timestamp_step(&self) -> u64 {
        self.ts_ring.as_ref().map(|ring| ring.last_step).unwrap_or(0)
    }

    pub fn physics_step(&self) -> u64 {
        self.physics_step
    }

    pub fn restore_physics_step(&mut self, step: u64) {
        self.physics_step = step;
        self.params.physics_step = step as u32;
    }

    fn record_submit(&mut self, index: wgpu::SubmissionIndex) {
        self.last_submit = Some(index);
    }

    pub fn wait_completion(&mut self) {
        if let Some(index) = self.last_submit.clone() {
            loop {
                match self.device.poll(wgpu::PollType::wait_for(index.clone())) {
                    Ok(status) if status.wait_finished() => break,
                    Ok(_) | Err(wgpu::PollError::Timeout) => {}
                }
            }
        } else {
            poll_until_idle(&self.device);
        }
        self.completed_step = self.physics_step;
        self.completed_known = true;
    }

    pub fn poll_completion(&mut self) {
        match self.device.poll(wgpu::PollType::Poll) {
            Ok(status) if status.wait_finished() => {
                self.completed_step = self.physics_step;
                self.completed_known = true;
            }
            _ => {}
        }
    }

    pub fn completed_physics_step(&self) -> Option<u64> {
        self.completed_known.then_some(self.completed_step)
    }

    /// Ordered mutation of cached contact roots; does not submit a physics step.
    pub fn retire_body_pair_contacts(&mut self, a: u32, b: u32) {
        // Words 64/65 are reserved for mutation commands, beyond ray state 0..31.
        self.queue.write_buffer(&self.query, 64 * 4, bytemuck::cast_slice(&[a, b]));
        let mut enc = self.device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
            label: Some("retire-joint-body-pair"),
        });
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("retire-joint-body-pair"), timestamp_writes: None,
            });
            pass.set_pipeline(&self.retire_body_pair_contacts);
            pass.set_bind_group(0, self.live_bg(), &[0]);
            pass.dispatch_workgroups(self.pair_groups(), 1, 1);
        }
        self.record_submit(self.queue.submit(Some(enc.finish())));
    }

    pub fn pose_snapshot_step(&self) -> u64 {
        self.pose_step
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn cast_ray_closest_gpu(
        &mut self,
        origin: [f32; 3],
        translation: [f32; 3],
        category_bits: u64,
        mask_bits: u64,
        max_fraction: f32,
    ) -> GpuRayHit {
        let encode_start = Instant::now();
        let mut words = [0u32; 32];
        words[0] = origin[0].to_bits();
        words[1] = origin[1].to_bits();
        words[2] = origin[2].to_bits();
        words[3] = translation[0].to_bits();
        words[4] = translation[1].to_bits();
        words[5] = translation[2].to_bits();
        words[6] = category_bits as u32;
        words[7] = (category_bits >> 32) as u32;
        words[8] = mask_bits as u32;
        words[9] = (mask_bits >> 32) as u32;
        words[10] = max_fraction.to_bits();
        words[12] = f32::MAX.to_bits();
        words[13] = u32::MAX;
        words[29] = 0xFFFF;
        self.queue
            .write_buffer(&self.query, 0, bytemuck::cast_slice(&words));
        let groups = self.shape_groups().max(1);
        let mut enc = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("ray-closest"),
            });
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("ray-closest-reduce"),
                timestamp_writes: None,
            });
            pass.set_pipeline(&self.ray_closest);
            pass.set_bind_group(0, self.live_bg(), &[0]);
            pass.dispatch_workgroups(groups, 1, 1);
        }
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("ray-closest-pick"),
                timestamp_writes: None,
            });
            pass.set_pipeline(&self.ray_closest_pick);
            pass.set_bind_group(0, self.live_bg(), &[0]);
            pass.dispatch_workgroups(groups, 1, 1);
        }
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("ray-closest-commit"),
                timestamp_writes: None,
            });
            pass.set_pipeline(&self.ray_closest_commit);
            pass.set_bind_group(0, self.live_bg(), &[0]);
            pass.dispatch_workgroups(groups, 1, 1);
        }
        let result_off = 16 * 4;
        let result_bytes = 16 * 4;
        self.ensure_staging(result_bytes.max(256));
        let staging = self.staging.as_ref().expect("staging buffer").clone();
        enc.copy_buffer_to_buffer(
            &self.query,
            result_off,
            &staging,
            0,
            result_bytes as u64,
        );
        let encode_ms = encode_start.elapsed().as_secs_f32() * 1e3;
        let inclusive_start = Instant::now();
        self.record_submit(self.queue.submit(Some(enc.finish())));
        poll_until_idle(&self.device);
        let exclusive_ms = inclusive_start.elapsed().as_secs_f32() * 1e3;
        let slice = staging.slice(..result_bytes as u64);
        let (tx, rx) = oneshot();
        slice.map_async(wgpu::MapMode::Read, move |r| {
            let _ = tx.send(r);
        });
        let map_start = Instant::now();
        poll_until_idle(&self.device);
        rx.recv().expect("map_async dropped").expect("map failed");
        let map_ms = map_start.elapsed().as_secs_f32() * 1e3;
        let inclusive_ms = inclusive_start.elapsed().as_secs_f32() * 1e3;
        let data = slice.get_mapped_range();
        let w: &[u32] = bytemuck::cast_slice(&data);
        let tri = w.get(13).copied().unwrap_or(0xFFFF);
        let hit = GpuRayHit {
            hit: w.first().copied().unwrap_or(0) == 1,
            gpu_shape: w.get(1).copied().unwrap_or(0),
            point: [
                f32::from_bits(w.get(2).copied().unwrap_or(0)),
                f32::from_bits(w.get(3).copied().unwrap_or(0)),
                f32::from_bits(w.get(4).copied().unwrap_or(0)),
            ],
            normal: [
                f32::from_bits(w.get(5).copied().unwrap_or(0)),
                f32::from_bits(w.get(6).copied().unwrap_or(0)),
                f32::from_bits(w.get(7).copied().unwrap_or(0)),
            ],
            fraction: f32::from_bits(w.get(8).copied().unwrap_or(0)),
            body: w.get(9).copied().unwrap_or(0),
            material: w.get(10).copied().unwrap_or(0) as u64
                | ((w.get(11).copied().unwrap_or(0) as u64) << 32),
            cpu_shape: w.get(12).copied().unwrap_or(0),
            triangle: if tri == 0xFFFF { -1 } else { tri as i32 },
            overflow: w.get(14).copied().unwrap_or(0),
            encode_ms,
            map_ms,
            exclusive_ms,
            inclusive_ms,
        };
        drop(data);
        staging.unmap();
        self.completed_step = self.physics_step;
        self.completed_known = true;
        hit
    }

    pub fn last_mirror_timings(&self) -> (f32, f32, f32, u64) {
        (
            self.last_mirror_wait_ms,
            self.last_mirror_copy_ms,
            self.last_mirror_map_ms,
            self.last_mirror_bytes,
        )
    }

    pub fn set_diagnostic_flags(&mut self, flags: u32) {
        self.invalidate_idle_proof();
        assert_eq!(flags & crate::types::DIAG_MESH_CANDIDATES,
            self.params.diagnostic_flags & crate::types::DIAG_MESH_CANDIDATES,
            "mesh-candidates allocation must be selected at world creation");
        self.params.diagnostic_flags = flags;
        if flags & DIAG_DISABLE_SLEEP != 0 {
            self.params.enable_sleep = 0;
        }
        self.one_group_wave_only = flags & DIAG_GENERAL_SOLVER == 0
            && self.params.solver_mode == 0
            && self.one_group_pair_ok;
        if flags & DIAG_FORCE_GENERAL_STATIC_SORT != 0 {
            self.skip_general_static_sort = false;
            self.params.diagnostic_flags &= !DIAG_STATIC_DEGREE_ONE_PROOF;
        }
        self.upload_pass_lut();
    }

    pub fn set_topology_bounds(
        &mut self,
        bounds: crate::types::TopologyBounds,
        solver_mode: u32,
    ) {
        self.params.joint_count = bounds.joints;
        let general = self.params.diagnostic_flags & DIAG_GENERAL_SOLVER != 0
            || solver_mode != 0
            || self.params.solver_mode != 0;
        self.one_group_pair_ok = bounds.mesh_shapes == 0 && bounds.one_group_pair_ok;
        self.one_group_wave_only = !general && self.one_group_pair_ok;
        let force_sort = self.params.diagnostic_flags & DIAG_FORCE_GENERAL_STATIC_SORT != 0;
        self.skip_general_static_sort =
            !force_sort && bounds.mesh_shapes == 0 && (bounds.skip_general_static_sort || (bounds.static_degree_two_proof
                && self.params.diagnostic_flags & crate::types::DIAG_BOUNDED_STATIC_SORT != 0));
        if self.skip_general_static_sort && bounds.skip_general_static_sort {
            self.params.diagnostic_flags |= DIAG_STATIC_DEGREE_ONE_PROOF;
        } else {
            self.params.diagnostic_flags &= !DIAG_STATIC_DEGREE_ONE_PROOF;
        }
        if self.skip_general_static_sort && !bounds.skip_general_static_sort && bounds.static_degree_two_proof {
            self.params.diagnostic_flags |= crate::types::DIAG_STATIC_DEGREE_TWO_PROOF;
        } else {
            self.params.diagnostic_flags &= !crate::types::DIAG_STATIC_DEGREE_TWO_PROOF;
        }
    }

    pub fn last_solver_dispatches(&self) -> u32 {
        self.solver_dispatches.get()
    }

    pub fn last_joint_dispatches(&self) -> u32 {
        self.joint_dispatches.get()
    }

    pub fn last_static_sort_dispatches(&self) -> u32 {
        self.static_sort_dispatches.get()
    }

    pub fn last_encode_commands(&self) -> u32 {
        self.encode_commands.get()
    }

    pub fn physics_invalid(&self) -> bool {
        self.physics_invalid
    }

    pub fn mark_physics_invalid(&mut self) {
        self.physics_invalid = true;
    }

    pub fn clear_sticky_loss(&mut self) {
        self.queue.write_buffer(&self.query, 66 * 4, bytemuck::bytes_of(&0u32));
        let zeros = [0u32; 6];
        self.queue.write_buffer(
            &self.atom,
            u64::from(crate::types::ATOM_STICKY_PAIR_DROPPED) * 4,
            bytemuck::bytes_of(&zeros),
        );
        #[cfg(not(target_arch = "wasm32"))]
        {
            self.sticky_loss = false;
            self.sticky_first_step = 0;
            self.sticky_causes = [0; 5];
            self.sticky_contact_reasons = 0;
        }
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn sticky_fail_message(&self) -> Option<String> {
        if self.sticky_loss {
            Some(format!(
                "capacity loss first_step={} pairs={} inserts={} contacts={} hash={} proof={} contact_reasons={:#x}",
                self.sticky_first_step, self.sticky_causes[0],self.sticky_causes[1],
                self.sticky_causes[2],self.sticky_causes[3],self.sticky_causes[4],self.sticky_contact_reasons
            ))
        } else {
            None
        }
    }

    #[cfg(not(target_arch = "wasm32"))]
    fn encode_contact_status(&self, enc: &mut wgpu::CommandEncoder) {
        if self.sticky_pending.is_none() {
            enc.copy_buffer_to_buffer(
                &self.atom,
                u64::from(crate::types::ATOM_STICKY_PAIR_DROPPED) * 4,
                &self.sticky_staging,
                0,
                24,
            );
            enc.copy_buffer_to_buffer(&self.query, 66 * 4, &self.sticky_staging, 24, 4);
            enc.copy_buffer_to_buffer(&self.scratch, 2 * 4, &self.sticky_staging, 28, 4);
            enc.copy_buffer_to_buffer(&self.scratch, 5 * 4, &self.sticky_staging, 32, 4);
            enc.copy_buffer_to_buffer(&self.query, 67 * 4, &self.sticky_staging, 36, 24);
        }
    }

    #[cfg(not(target_arch = "wasm32"))]
    fn map_contact_status(&mut self) {
        if self.sticky_pending.is_none() {
            let slice = self.sticky_staging.slice(..60);
            let (tx, rx) = oneshot();
            slice.map_async(wgpu::MapMode::Read, move |r| {
                let _ = tx.send(r);
            });
            self.sticky_pending_step = self.physics_step;
            self.sticky_pending_context = self.metrics_context;
            self.sticky_pending_idle_context=(self.idle_count_valid_step==Some(self.physics_step)).then_some((self.idle_output_context,self.idle_epoch.get()));
            self.sticky_pending = Some(rx);
        }
    }

    #[cfg(not(target_arch = "wasm32"))]
    fn harvest_sticky_status(&mut self, wait: bool) {
        #[cfg(test)]
        if self.hold_idle_status {return;}

        let Some(rx) = self.sticky_pending.take() else {
            return;
        };
        if wait {
            poll_until_idle(&self.device);
        } else {
            let _ = self.device.poll(wgpu::PollType::Poll);
        }
        let ready = if wait {
            rx.recv().ok().and_then(Result::ok).is_some()
        } else {
            match rx.try_recv() {
                Ok(Ok(())) => true,
                Ok(Err(_)) => false,
                Err(std::sync::mpsc::TryRecvError::Empty) => {
                    self.sticky_pending = Some(rx);
                    return;
                }
                Err(_) => false,
            }
        };
        if !ready {
            return;
        }
        let slice = self.sticky_staging.slice(..60);
        let data = slice.get_mapped_range();
        let words: &[u32] = bytemuck::cast_slice(&data);
        let pair = words.first().copied().unwrap_or(0);
        let insert = words.get(1).copied().unwrap_or(0);
        let contact = words.get(2).copied().unwrap_or(0);
        let hop = words.get(3).copied().unwrap_or(0);
        let first = words.get(4).copied().unwrap_or(0);
        let proof = words.get(5).copied().unwrap_or(0);
        self.sticky_contact_reasons |= words.get(6).copied().unwrap_or(0);
        if self.color_wave_prefix_override.is_none() && self.sticky_pending_step >= self.color_hint_min_step {
            // This is a scheduling hint, not a capacity proof. If a newer graph
            // grows a color, the tail still visits all of it in bounded batches.
            self.color_wave_prefix = words[13].min(crate::types::DYNAMIC_COLOR_COUNT);
        }
        if let Some((context,epoch))=self.sticky_pending_idle_context {
            if epoch==self.idle_epoch.get() && self.idle_proof.is_none_or(|(step,_,_)|self.sticky_pending_step>=step) {
                self.idle_proof=if words[14]==0 && !self.physics_invalid && pair|insert|contact|hop|proof==0 {
                    Some((self.sticky_pending_step,context,epoch))
                } else {None};
            }
        }
        self.contact_metrics = Some(ContactMetrics {
            step: self.sticky_pending_step,
            topology_revision: self.sticky_pending_context[0],
            state_revision: self.sticky_pending_context[1],
            capacity_loss: self.physics_invalid || pair | insert | contact | hop | proof != 0,
            candidate_pairs: words[7], allocated_roots: words[8],
            allocated_manifold_slots: words[9], touching_roots: words[10], non_sensor_roots: words[11],
        });
        drop(data);
        self.sticky_staging.unmap();
        if pair | insert | contact | hop | proof != 0 {
            for (saved, value) in self.sticky_causes.iter_mut().zip([pair,insert,contact,hop,proof]) {
                *saved = (*saved).max(value);
            }
            self.sticky_loss = true;
            self.physics_invalid = true;
            if self.sticky_first_step == 0 {
                self.sticky_first_step = first;
            }
        }
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn set_contact_metrics_context(&mut self, topology: u64, state: u64) {
        self.metrics_context = [topology,state];
    }

    /// May return an older completed step if the status slot is still in flight.
    /// No pose/contact-buffer download or CCD/event harvest is performed.
    #[cfg(not(target_arch = "wasm32"))]
    pub fn contact_metrics(&mut self, wait: bool) -> Option<ContactMetrics> {
        self.harvest_sticky_status(wait);
        self.contact_metrics
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn pose_export_fd(&self) -> Option<(i32, u64, u32)> {
        self.pose_export
            .as_ref()
            .map(|e| (e.fd, e.size, self.count))
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn harvest_gpu_timestamps(&mut self, wait: bool) {
        self.harvest_sticky_status(wait);
        if self.ts_ring.as_ref().is_none_or(|ring| ring.pending.iter().all(|p| p.is_none())) {
            return;
        }
        if wait {
            poll_until_idle(&self.device);
        } else {
            let _ = self.device.poll(wgpu::PollType::Poll);
        }
        let period = self.queue.get_timestamp_period() as f32 / 1e6;
        let fallback = [
            self.last_gpu_collide_ms,
            self.last_gpu_solve_ms,
            self.last_gpu_integrate_ms,
            self.last_gpu_broadphase_ms,
            self.last_gpu_narrowphase_ms,
            self.last_gpu_graph_ms,
            self.last_gpu_prepare_ms,
            self.last_gpu_device_ms,
        ];
        let mut best: Option<(u64, [f32; 8])> = None;
        if let Some(ring) = self.ts_ring.as_mut() {
            for slot in 0..2 {
                let Some(rx) = ring.pending[slot].take() else {
                    continue;
                };
                let ready = if wait {
                    rx.recv().ok().and_then(Result::ok).is_some()
                } else {
                    match rx.try_recv() {
                        Ok(Ok(())) => true,
                        Ok(Err(_)) => false,
                        Err(std::sync::mpsc::TryRecvError::Empty) => {
                            ring.pending[slot] = Some(rx);
                            continue;
                        }
                        Err(_) => false,
                    }
                };
                if !ready {
                    continue;
                }
                let slice = ring.staging[slot].slice(..TIMESTAMP_STEP_STRIDE);
                let data = slice.get_mapped_range();
                let timestamps: &[u64] = bytemuck::cast_slice(&data);
                let values = if timestamps.len() >= 7 {
                    [
                        timestamps[3].wrapping_sub(timestamps[0]) as f32 * period,
                        timestamps[5].wrapping_sub(timestamps[4]) as f32 * period,
                        timestamps[6].wrapping_sub(timestamps[5]) as f32 * period,
                        timestamps[1].wrapping_sub(timestamps[0]) as f32 * period,
                        timestamps[2].wrapping_sub(timestamps[1]) as f32 * period,
                        timestamps[3].wrapping_sub(timestamps[2]) as f32 * period,
                        timestamps[4].wrapping_sub(timestamps[3]) as f32 * period,
                        timestamps[6].wrapping_sub(timestamps[0]) as f32 * period,
                    ]
                } else {
                    fallback
                };
                drop(data);
                ring.staging[slot].unmap();
                let step = ring.step[slot];
                if best.map(|(s, _)| step >= s).unwrap_or(true) {
                    best = Some((step, values));
                }
            }
        }
        if let Some((step, values)) = best {
            let apply = self
                .ts_ring
                .as_mut()
                .is_some_and(|ring| {
                    if step >= ring.last_step {
                        ring.last_step = step;
                        true
                    } else {
                        false
                    }
                });
            if apply {
                self.last_gpu_collide_ms = values[0];
                self.last_gpu_solve_ms = values[1];
                self.last_gpu_integrate_ms = values[2];
                self.last_gpu_broadphase_ms = values[3];
                self.last_gpu_narrowphase_ms = values[4];
                self.last_gpu_graph_ms = values[5];
                self.last_gpu_prepare_ms = values[6];
                self.last_gpu_device_ms = values[7];
            }
        }
    }

    #[cfg(not(target_arch = "wasm32"))]
    fn pose_readback_bytes(count: u32) -> u64 {
        (mem::size_of::<BodyStateGpu>() as u64 * u64::from(count.max(1))).max(256)
    }

    #[cfg(not(target_arch = "wasm32"))]
    fn ensure_pose_readback(&mut self) {
        let size = Self::pose_readback_bytes(self.count);
        if self
            .pose_readback
            .as_ref()
            .is_some_and(|rb| rb.staging[0].size() >= size)
        {
            return;
        }
        if self.pose_readback.is_some() {
            poll_until_idle(&self.device);
        }
        let maps = self.pose_readback.as_ref().map(|rb| rb.maps).unwrap_or(0);
        let kicks = self.pose_readback.as_ref().map(|rb| rb.kicks).unwrap_or(0);
        self.pose_readback = None;
        let make = |label: &str| {
            self.device.create_buffer(&wgpu::BufferDescriptor {
                label: Some(label),
                size,
                usage: wgpu::BufferUsages::MAP_READ | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            })
        };
        self.pose_readback = Some(PoseReadback {
            staging: [make("pose-rb-0"), make("pose-rb-1")],
            pending: None,
            consumed: None,
            epoch: 0,
            maps,
            kicks,
            step: [0, 0],
        });
    }

    #[cfg(not(target_arch = "wasm32"))]
    fn kick_pose_readback(&mut self) {
        // Host-edit precedence advances with submitted state, not physical copies.
        self.pose_epoch=self.pose_epoch.saturating_add(1);
        if !self.automatic_pose_snapshots {return;}
        self.ensure_pose_readback();
        let size = Self::pose_readback_bytes(self.count);
        let index = {
            let Some(rb) = self.pose_readback.as_mut() else {
                return;
            };
            let slot = match rb.pending {
                Some(0) => 1,
                _ => 0,
            };
            let mut enc = self
                .device
                .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                    label: Some("pose-readback"),
                });
            enc.copy_buffer_to_buffer(&self.bodies, 0, &rb.staging[slot], 0, size);
            let index = self.queue.submit(Some(enc.finish()));
            rb.pending = Some(slot);
            rb.consumed = None;
            rb.kicks += 1;
            rb.epoch = self.pose_epoch;
            rb.step[slot] = self.physics_step;
            index
        };
        self.last_submit = Some(index);
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub(crate) fn set_automatic_pose_snapshots(&mut self, enabled:bool) {
        if self.automatic_pose_snapshots!=enabled {
            // Never expose an old pending staging copy after a policy change.
            // Buffers remain retained until in-flight writes complete in queue order.
            if let Some(rb)=self.pose_readback.as_mut() {rb.pending=None;rb.consumed=None;}
            self.automatic_pose_snapshots=enabled;
        }
    }

    #[cfg(not(target_arch = "wasm32"))]
    // Latest submitted pose generation; advancing does not imply a CPU copy.
    pub fn pose_snapshot_epoch(&self) -> u64 {
        self.pose_epoch
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub(crate) fn staged_pose_epoch(&self)->u64 {
        self.pose_readback.as_ref().map(|rb|rb.epoch).unwrap_or(0)
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn pose_snapshot_maps(&self) -> u64 {
        self.pose_readback.as_ref().map(|rb| rb.maps).unwrap_or(0)
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn pose_snapshot_kicks(&self) -> u64 {
        self.pose_readback.as_ref().map(|rb| rb.kicks).unwrap_or(0)
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn pose_snapshot_consumed(&self) -> bool {
        self.pose_readback.as_ref().is_some_and(|rb| {
            rb.pending.is_some() && rb.consumed == rb.pending
        })
    }

    /// A completed full-state mirror supersedes the staged pose copy, which
    /// may predate CCD corrections. Never apply that older copy afterward.
    #[cfg(not(target_arch = "wasm32"))]
    pub fn accept_body_mirror_as_pose_snapshot(&mut self) {
        if let Some(rb) = self.pose_readback.as_mut() {
            rb.consumed = rb.pending;
        }
        self.pose_step = self.physics_step;
    }

    /// Previous completed pose copy. Does not wait on the current GPU step.
    #[cfg(not(target_arch = "wasm32"))]
    pub fn lock_pose_snapshot(&mut self) -> Option<Vec<BodyStateGpu>> {
        {
            let rb = self.pose_readback.as_ref()?;
            let slot = rb.pending?;
            if rb.consumed == Some(slot) {
                return None;
            }
        }
        self.harvest_gpu_timestamps(false);
        let needed = mem::size_of::<BodyStateGpu>() * self.count.max(1) as usize;
        let rb = self.pose_readback.as_mut()?;
        let slot = rb.pending?;
        if rb.consumed == Some(slot) {
            return None;
        }
        if needed as u64 > rb.staging[slot].size() {
            return None;
        }
        match self.device.poll(wgpu::PollType::Poll) {
            Ok(status) if status.wait_finished() => {}
            _ => return None,
        }
        let slice = rb.staging[slot].slice(..needed as u64);
        let (tx, rx) = oneshot();
        slice.map_async(wgpu::MapMode::Read, move |r| {
            let _ = tx.send(r);
        });
        poll_until_idle(&self.device);
        rx.recv().ok()?.ok()?;
        let data = slice.get_mapped_range();
        let poses: Vec<BodyStateGpu> = bytemuck::cast_slice(&data[..needed]).to_vec();
        drop(data);
        rb.staging[slot].unmap();
        rb.consumed = Some(slot);
        rb.maps += 1;
        let pose_step = rb.step[slot];
        self.pose_step = pose_step;
        Some(poses)
    }

    pub fn step_frame(&mut self, _encoder: &mut wgpu::CommandEncoder, sub_steps: i32) {
        self.world_step(sub_steps);
    }

    pub(crate) fn set_convex_ccd(&mut self, scene:Option<&crate::ccd::ConvexScene>) {
        self.convex_ccd=scene.map(|scene|crate::ccd::ConvexCcd::new(&self.device,&self.bodies,scene));
    }

    pub(crate) fn uses_convex_ccd(&self)->bool { self.convex_ccd.is_some() }

    pub fn step_submit(&mut self, sub_steps: i32) {
        self.world_step(sub_steps);
    }

    /// Submit broadphase and narrowphase, stopping before graph coloring and
    /// constraint preparation so host callbacks can veto current-step pairs.
    #[cfg(not(target_arch = "wasm32"))]
    fn begin_callback_step(&mut self) -> bool {
        self.harvest_gpu_timestamps(false);
        if self.physics_invalid || self.params.step_dt <= 0.0 {
            return false;
        }
        if !self.callback_open {
            self.physics_step = self.physics_step.saturating_add(1);
            self.params.physics_step = self.physics_step as u32;
            self.callback_open = true;
        }
        self.upload_pass_lut();
        true
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn callback_detect_submit(&mut self) {
        if !self.begin_callback_step() {
            return;
        }
        let mut enc = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("world-step-callback-detect"),
            });
        self.ping_dispatch(
            &mut enc,
            &self.reset_deltas.clone(),
            self.body_groups(),
            0,
            1,
        );
        self.collide_detect_pass(&mut enc);
        self.queue.submit(Some(enc.finish()));
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn callback_candidates_submit(&mut self) {
        if !self.begin_callback_step() {
            return;
        }
        let mut enc = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("world-step-callback-candidates"),
            });
        self.ping_dispatch(
            &mut enc,
            &self.reset_deltas.clone(),
            self.body_groups(),
            0,
            1,
        );
        self.broadphase_candidates_pass(&mut enc);
        self.queue.submit(Some(enc.finish()));
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn callback_narrowphase_submit(&mut self) {
        let mut enc = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("world-step-callback-narrowphase"),
            });
        self.narrowphase_detect_pass(&mut enc);
        self.queue.submit(Some(enc.finish()));
    }

    /// Continue a callback-staged step after host veto writes have been queued.
    #[cfg(not(target_arch = "wasm32"))]
    pub fn callback_finish_submit(&mut self, sub_steps: i32) {
        if self.physics_invalid || self.params.step_dt <= 0.0 {
            self.callback_open = false;
            return;
        }
        self.callback_open = false;
        self.solver_dispatches.set(0);
        self.joint_dispatches.set(0);
        let mut enc = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("world-step-callback-finish"),
            });
        self.collide_finalize_pass(&mut enc);
        let body_groups = self.body_groups();
        let island_body_groups = self.island_groups(self.count);
        self.dispatch_islands_and_prepare(&mut enc);
        self.prepare_joint_components(&mut enc);
        let joints = self.solve_joints.clone();
        let int_vel = self.integrate_vel.clone();
        let int_pos = self.integrate_pos.clone();
        let apply = self.apply_deltas.clone();
        let has_joints = self.params.joint_count > 0;
        for _ in 0..sub_steps.max(1) {
            self.ping_dispatch(&mut enc, &int_vel, body_groups, 0, 1);
            self.dispatch_colored_solve(&mut enc, 2);
            if has_joints {
                self.copy_then(&mut enc, &joints, self.joint_solve_groups(), 0, 2);
            }
            self.dispatch_colored_solve(&mut enc, 1);
            if has_joints {
                self.copy_then(&mut enc, &joints, self.joint_solve_groups(), 0, 1);
            }
            self.ping_dispatch(&mut enc, &int_pos, body_groups, 0, 1);
            self.dispatch_colored_solve(&mut enc, 0);
            if has_joints {
                self.copy_then(&mut enc, &joints, self.joint_solve_groups(), 0, 0);
            }
        }
        self.dispatch_colored_solve(&mut enc, 3);
        self.ping_dispatch(&mut enc, &apply, body_groups, 0, 1);
        self.dispatch_n(
            &mut enc,
            &self.island_reset_ready.clone(),
            island_body_groups,
            0,
            1,
        );
        self.dispatch_n(
            &mut enc,
            &self.island_accumulate_ready.clone(),
            island_body_groups,
            0,
            1,
        );
        self.dispatch_n(
            &mut enc,
            &self.island_apply_sleep.clone(),
            island_body_groups,
            0,
            1,
        );
        self.encode_contact_status(&mut enc);
        self.record_submit(self.queue.submit(Some(enc.finish())));
        self.completed_known = false;
        self.map_contact_status();
    }

    pub fn set_restitution_threshold(&mut self, threshold: f32) {
        self.params.restitution_threshold = threshold;
    }

    pub fn set_maximum_linear_speed(&mut self, speed: f32) {
        self.params.maximum_linear_speed = speed;
    }

    pub fn write_params(
        &mut self,
        dt: f32,
        step_dt: f32,
        gravity: [f32; 3],
        count: u32,
        enable_contacts: bool,
        contact_hertz: f32,
        contact_damping: f32,
        contact_speed: f32,
    ) {
        let (bias_rate, mass_scale, impulse_scale) =
            crate::types::make_soft(contact_hertz, contact_damping, dt);
        // The normal parameter upload also disables the previous host batch.
        self.params.fat_commands_count = 0;
        self.params.dt = dt;
        self.params.gravity_x = gravity[0];
        self.params.gravity_y = gravity[1];
        self.params.gravity_z = gravity[2];
        self.params.body_count = count;
        self.params.enable_contacts = u32::from(enable_contacts);
        self.params.max_contacts = crate::types::MAX_CONTACTS_PER_BODY;
        self.params.bias_rate = bias_rate;
        self.params.mass_scale = mass_scale;
        self.params.impulse_scale = impulse_scale;
        self.params.contact_speed = contact_speed;
        self.params.step_dt = step_dt;
        self.params.contact_hertz = contact_hertz;
        self.params.contact_damping = contact_damping;
        self.count = count;
    }

    pub(crate) fn invalidate_idle_proof(&self) {
        self.idle_epoch.set(self.idle_epoch.get().checked_add(1).expect("idle mutation epoch exhausted"));
    }

    pub(crate) fn configure_idle_step(&mut self,eligible:bool,input:[u64;2],output:[u64;2]) {
        self.idle_eligible=eligible;
        self.idle_input_context=input;
        self.idle_output_context=output;
    }
    #[cfg(test)]
    pub(crate) fn hold_idle_status_test(&mut self, hold:bool) {self.hold_idle_status=hold;}
    #[cfg(test)]
    pub(crate) fn last_step_was_idle(&self)->bool {self.last_step_idle}

    pub(crate) fn set_component_tgs(&mut self, enabled: bool) {
        if enabled {
            let capacity = self.caps.bodies.max(self.count).max(1);
            let size = (261 + 54 * u64::from(capacity) + 2 * u64::from(self.contact_slots)) * 4;
            if size > self.query.size() {
                let grown = self.device.create_buffer(&wgpu::BufferDescriptor {
                    label: Some("query-component-lists"), size,
                    usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_SRC | wgpu::BufferUsages::COPY_DST,
                    mapped_at_creation: false,
                });
                let mut encoder = self.device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
                    label: Some("grow-query-component-lists"),
                });
                #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
                encoder.clear_buffer(&grown,0,None);
                // Preserve pending query/status words; same-queue ordering needs no host wait.
                encoder.copy_buffer_to_buffer(&self.query, 0, &grown, 0, self.query.size());
                self.queue.submit(Some(encoder.finish()));
                self.bind_group = physics_bind_group(&self.device, &self.bind_group_layout,
                    &self.pass_lut, &self.bodies, &self.body_cold, &self.contacts, &self.contact_persistent,
                    &self.contact_prepared, &self.joints, &self.scratch, &self.atom, &grown);
                self.query = grown;
            }
        }
        self.component_tgs = enabled;
    }

    pub fn set_solver_mode(&mut self, jacobi: bool) {
        self.params.solver_mode = u32::from(jacobi);
    }

    pub fn set_contact_recycle_distance(&mut self, distance: f32) {
        self.params.contact_recycle_distance = distance;
    }

    pub fn set_continuous_enabled(&mut self, enabled: bool) {
        self.params.enable_continuous = u32::from(enabled);
    }

    pub fn set_sleep_enabled(&mut self, enabled: bool) {
        self.params.enable_sleep =
            u32::from(enabled && self.params.diagnostic_flags & DIAG_DISABLE_SLEEP == 0);
    }

    /// Ordered host teleports: upload once per physics submission. Existing
    /// clear lanes consume each body's list exactly once, including callbacks.
    pub fn upload_fat_transforms(&mut self, transforms: &[(u32, Vec<[f32; 8]>)]) -> Result<(), String> {
        let count = transforms.iter().try_fold(0usize, |n, (_, v)| n.checked_add(v.len()))
            .ok_or("transform history count overflow")?;
        if count == 0 {
            if self.params.fat_commands_count != 0 {
                self.params.fat_commands_count = 0;
                self.flush_params();
            }
            return Ok(());
        }
        self.invalidate_idle_proof();
        let heads = 2usize * self.caps.bodies.max(self.count).max(1) as usize;
        let words = count.checked_mul(8).and_then(|n| n.checked_add(heads))
            .ok_or("transform history size overflow")?;
        let base = self.params.fat_bounds_base as usize + 8 * self.caps.shapes as usize;
        let bytes = base.checked_add(words).and_then(|n| n.checked_mul(4))
            .ok_or("transform history allocation overflow")? as u64;
        let limit = u64::from(self.device.limits().max_storage_buffer_binding_size)
            .min(self.device.limits().max_buffer_size);
        if bytes > limit { return Err(format!("transform history needs {bytes} bytes, device permits {limit}")); }
        if bytes > self.scratch.size() {
            let size = bytes.checked_next_power_of_two().unwrap_or(bytes).min(limit);
            let grown = self.device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("scratch-host-transforms"), size,
                usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_SRC | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            });
            let mut encoder = self.device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("grow-scratch-host-transforms"),
            });
            #[cfg(all(feature = "native-command-cache", not(target_arch = "wasm32")))]
            encoder.clear_buffer(&grown,0,None);
            encoder.copy_buffer_to_buffer(&self.scratch, 0, &grown, 0, self.scratch.size());
            self.queue.submit(Some(encoder.finish()));
            self.bind_group = physics_bind_group(&self.device, &self.bind_group_layout,
                &self.pass_lut, &self.bodies, &self.body_cold, &self.contacts, &self.contact_persistent,
                &self.contact_prepared, &self.joints, &grown, &self.atom, &self.query);
            self.scratch = grown;
        }
        let mut data = Vec::new();
        data.try_reserve_exact(words).map_err(|_| "transform history host allocation failed")?;
        data.resize(heads, 0u32);
        for (body, entries) in transforms {
            if *body >= self.count { return Err("transform history references missing body".into()); }
            data[2 * *body as usize] = data.len() as u32;
            data[2 * *body as usize + 1] = entries.len() as u32;
            data.extend_from_slice(bytemuck::cast_slice(entries));
        }
        self.params.fat_commands_epoch = self.params.fat_commands_epoch.checked_add(1)
            .ok_or("transform history epoch exhausted")?;
        self.params.fat_commands_base = base as u32;
        self.params.fat_commands_count = count as u32;
        self.queue.write_buffer(&self.scratch, (base * 4) as u64, bytemuck::cast_slice(&data));
        self.flush_params();
        Ok(())
    }

    fn remap_physical_contacts(&mut self, old: &[(u32, u16)]) {
        // Appending shapes cannot change an existing physical pair key.
        if old.iter().enumerate().all(|(i, id)| self.shape_identities.get(i) == Some(id)) { return; }
        assert!(old.len() <= crate::types::PAIR_CAP as usize);
        let new: std::collections::HashMap<_, _> = self.shape_identities.iter()
            .enumerate().map(|(i, &id)| (id, i as u32)).collect();
        let mapping: Vec<u32> = old.iter().map(|id| new.get(id).copied().unwrap_or(u32::MAX)).collect();
        if !mapping.is_empty() {
            self.queue.write_buffer(&self.scratch, u64::from(crate::types::SCR_RADIX_OUT) * 4,
                bytemuck::cast_slice(&mapping));
        }
        let next_step = self.physics_step.wrapping_add(1) as u32;
        self.params.remap_capture = u32::from(self.params.remap_history_step != next_step);
        self.params.remap_history_step = next_step;
        self.params.remap_old_count = old.len() as u32;
        self.flush_params();
        let mut encoder = self.device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
            label: Some("remap-contact-shapes"),
        });
        self.dispatch_n(&mut encoder, &self.clear_remapped_contact_hash,
            crate::types::CONTACT_HASH_CAP.div_ceil(WORKGROUP_SIZE), 0, 1);
        self.dispatch_n(&mut encoder, &self.remap_contact_shapes, self.contact_groups(), 0, 1);
        self.dispatch_n(&mut encoder, &self.publish_remapped_contacts, self.contact_groups(), 0, 1);
        self.queue.submit(Some(encoder.finish()));
    }

    pub fn write_scene(&mut self, bytes: &[u8], shapes: &[ShapeGpu], bodies: &[BodyGpu], identities: &[(u32, u16)]) {
        self.pair_matrix_flat=shapes.iter().all(|s|s.kind!=crate::types::KIND_MESH && s.event_flags & (crate::types::SHAPE_PUBLIC_PROXY|crate::types::SHAPE_COMPOUND_CHILD)==0);
        self.invalidate_idle_proof();
        // Immutable CCD spans/targets must never outlive the uploaded topology.
        self.convex_ccd=None;
        if self.color_wave_prefix_override.is_none() {
            self.color_wave_prefix = crate::types::OVERFLOW_COLOR;
            self.color_hint_min_step = self.physics_step.saturating_add(1);
        }
        let geometry: Vec<_> = shapes.iter().map(|s| FatGeometryKey::new(s, bodies)).collect();
        let moved_or_removed = self.shape_identities.iter().enumerate()
            .any(|(i, id)| identities.get(i) != Some(id));
        if moved_or_removed {
            // Stage only on layout changes: source/destination records may overlap.
            let old_slots: std::collections::HashMap<_, _> = self.shape_identities.iter()
                .enumerate().map(|(i, &id)| (id, i)).collect();
            let temporary = self.device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("remapped-fat-bounds"), size: (geometry.len().max(1) * 32) as u64,
                usage: wgpu::BufferUsages::COPY_SRC | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            });
            let mut encoder = self.device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("remap-fat-bounds"),
            });
            let mut spans: Vec<(usize, usize, usize)> = Vec::new();
            for (dst, id) in identities.iter().enumerate() {
                if let Some(&src) = old_slots.get(id).filter(|&&src| self.fat_geometry[src] == geometry[dst]) {
                    if let Some((start_src, start_dst, count)) = spans.last_mut()
                        .filter(|(start_src, start_dst, count)| src == *start_src + *count && dst == *start_dst + *count) {
                        let _ = (start_src, start_dst);
                        *count += 1;
                    } else {
                        spans.push((src, dst, 1));
                    }
                }
            }
            for (src, dst, count) in spans {
                encoder.copy_buffer_to_buffer(&self.scratch,
                    u64::from(self.params.fat_bounds_base) * 4 + src as u64 * 32,
                    &temporary, dst as u64 * 32, count as u64 * 32);
            }
            if !geometry.is_empty() {
                encoder.copy_buffer_to_buffer(&temporary, 0, &self.scratch,
                    u64::from(self.params.fat_bounds_base) * 4, geometry.len() as u64 * 32);
            }
            self.queue.submit(Some(encoder.finish()));
            let old = std::mem::replace(&mut self.shape_identities, identities.to_vec());
            self.remap_physical_contacts(&old);
        } else {
            for (index, key) in geometry.iter().enumerate() {
                if self.fat_geometry.get(index) != Some(key) {
                    let word = self.params.fat_bounds_base as u64 + 8 * index as u64 + 3;
                    self.queue.write_buffer(&self.scratch, word * 4, bytemuck::bytes_of(&0u32));
                }
            }
        }
        self.shape_identities = identities.to_vec();
        self.fat_geometry = geometry;
        assert!(bytes.len() as u64 <= self.body_cold.size(), "scene blob exceeds allocated heap");
        if !bytes.is_empty() { self.queue.write_buffer(&self.body_cold, 0, bytes); }
    }

    pub fn write_joints(&mut self, joints: &[JointGpu]) {
        self.invalidate_idle_proof();
        let mut pad = joints.to_vec();
        pad.resize(self.caps.joints.max(1) as usize, JointGpu::zeroed());
        self.queue
            .write_buffer(&self.joints, 0, bytemuck::cast_slice(&pad));
        self.upload_joint_filter(&pad);
    }

    fn upload_joint_filter(&mut self, joints: &[JointGpu]) {
        let (table, overflow) = crate::types::build_joint_filter_table(joints);
        if overflow {
            self.params.diagnostic_flags |= crate::types::DIAG_JOINT_FILTER_OVERFLOW;
            self.upload_pass_lut();
        } else {
            self.params.diagnostic_flags &= !crate::types::DIAG_JOINT_FILTER_OVERFLOW;
        }
        let bodies = self.caps.bodies.max(self.count).max(1);
        let offset = u64::from(crate::types::joint_filter_word_offset_with_joints(
            bodies,
            self.caps.joints.max(bodies),
        )) * 4;
        self.queue
            .write_buffer(&self.scratch, offset, bytemuck::cast_slice(&table));
    }

    pub fn set_geom_counts(
        &mut self,
        shapes: u32,
        joints: u32,
        hull_points: u32,
        hull_planes: u32,
        hull_edges: u32,
        hull_topology: u32,
        mesh_vertices: u32,
        mesh_triangles: u32,
        mesh_nodes: u32,
        materials: u32,
    ) {
        self.shape_count = shapes;
        self.params.shape_count = shapes;
        self.params.joint_count = joints;
        self.params.hull_point_count = hull_points;
        self.params.hull_plane_count = hull_planes;
        self.params.hull_edge_count = hull_edges;
        self.params.hull_topology_count = hull_topology;
        self.params.mesh_vertex_count = mesh_vertices;
        self.params.mesh_triangle_count = mesh_triangles;
        self.params.mesh_node_count = mesh_nodes;
        self.params.surface_material_count = materials;
    }

    pub fn copy_contacts_from(&mut self, old: &GpuSim) {
        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("copy-contacts"),
            });
        let copy = |encoder: &mut wgpu::CommandEncoder, src: &Buffer, dst: &Buffer| {
            let size = src.size().min(dst.size());
            if size > 0 {
                encoder.copy_buffer_to_buffer(src, 0, dst, 0, size);
            }
        };
        copy(&mut encoder, &old.contacts, &self.contacts);
        copy(&mut encoder, &old.contact_persistent, &self.contact_persistent);
        copy(&mut encoder, &old.contact_prepared, &self.contact_prepared);
        let core_atom_size = |sim: &GpuSim| sim.atom.size() -
            if sim.params.diagnostic_flags & crate::types::DIAG_MESH_CANDIDATES != 0 {
                u64::from(crate::types::MESH_TRACE_WORDS) * 4
            } else { 0 };
        encoder.copy_buffer_to_buffer(&old.atom, 0, &self.atom, 0,
            core_atom_size(old).min(core_atom_size(self)));
        // Roots absent from the new candidate list still need retention/retirement.
        // The count and list have fixed offsets; body-dependent graph scratch does not.
        for (word, words) in [
            (crate::types::SCR_OCCUPIED_N, 1),
            (crate::types::SCR_OCCUPIED_CONTACT, old.contact_slots.min(self.contact_slots)),
        ] {
            encoder.copy_buffer_to_buffer(
                &old.scratch, u64::from(word) * 4,
                &self.scratch, u64::from(word) * 4, u64::from(words) * 4,
            );
        }
        // Preserve bound hysteresis without a host download. Coalesce adjacent
        // matching records so unchanged large scenes require one cache copy.
        let sources: std::collections::HashMap<_, _> = old.shape_identities.iter()
            .enumerate().map(|(i, &id)| (id, i)).collect();
        let mut span: Option<(usize, usize, usize)> = None;
        let copy_span = |encoder: &mut wgpu::CommandEncoder, (src, dst, count): (usize, usize, usize)| {
            encoder.copy_buffer_to_buffer(
                &old.scratch, u64::from(old.params.fat_bounds_base) * 4 + src as u64 * 32,
                &self.scratch, u64::from(self.params.fat_bounds_base) * 4 + dst as u64 * 32,
                count as u64 * 32,
            );
        };
        for (dst, key) in self.fat_geometry.iter().enumerate() {
            let src = sources.get(&self.shape_identities[dst]).copied()
                .filter(|&i| old.fat_geometry[i] == *key);
            match (span, src) {
                (Some((start_src, start_dst, count)), Some(src))
                    if src == start_src + count && dst == start_dst + count => {
                        span = Some((start_src, start_dst, count + 1));
                    }
                (previous, src) => {
                    if let Some(previous) = previous { copy_span(&mut encoder, previous); }
                    span = src.map(|src| (src, dst, 1));
                }
            }
        }
        if let Some(span) = span { copy_span(&mut encoder, span); }
        let previous = u64::from(crate::types::SCR_PREVIOUS_TOUCHING) * 4;
        if self.contact_slots > old.contact_slots {
            let empty = vec![u32::MAX; (self.contact_slots - old.contact_slots) as usize];
            self.queue.write_buffer(&self.scratch, previous + u64::from(old.contact_slots) * 4,
                bytemuck::cast_slice(&empty));
        }
        encoder.copy_buffer_to_buffer(&old.scratch, previous, &self.scratch, previous,
            u64::from(old.contact_slots.min(self.contact_slots)) * 4);
        self.params.remap_history_step = old.params.remap_history_step;
        self.params.fat_bounds_epoch = old.params.fat_bounds_epoch;
        self.params.fat_commands_epoch = old.params.fat_commands_epoch;
        self.queue.submit(Some(encoder.finish()));
        self.contact_epoch = old.contact_epoch;
        self.remap_physical_contacts(&old.shape_identities);
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn begin_timing_window(&mut self, steps: u32) {
        let steps = steps.max(1);
        let timestamp = self.timestamp_queries.then(|| {
            let query_count = steps * TIMESTAMP_QUERY_COUNT;
            let size = u64::from(steps) * TIMESTAMP_STEP_STRIDE;
            TimestampRes {
                queries: self.device.create_query_set(&wgpu::QuerySetDescriptor {
                    label: Some("physics-metrics-ts"),
                    ty: wgpu::QueryType::Timestamp,
                    count: query_count,
                }),
                resolve: self.device.create_buffer(&wgpu::BufferDescriptor {
                    label: Some("physics-metrics-ts-resolve"),
                    size,
                    usage: wgpu::BufferUsages::QUERY_RESOLVE | wgpu::BufferUsages::COPY_SRC,
                    mapped_at_creation: false,
                }),
                staging: self.device.create_buffer(&wgpu::BufferDescriptor {
                    label: Some("physics-metrics-ts-staging"),
                    size,
                    usage: wgpu::BufferUsages::MAP_READ | wgpu::BufferUsages::COPY_DST,
                    mapped_at_creation: false,
                }),
            }
        });
        let workload_size = u64::from(steps) * WORKLOAD_STEP_STRIDE;
        self.metrics = Some(MetricsRes {
            steps,
            submitted: 0,
            timestamp,
            workload: self.device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("physics-metrics-workload"),
                size: workload_size,
                usage: wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::COPY_SRC,
                mapped_at_creation: false,
            }),
            workload_staging: self.device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("physics-metrics-workload-staging"),
                size: workload_size,
                usage: wgpu::BufferUsages::MAP_READ | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            }),
            cpu_encode_ms: Vec::with_capacity(steps as usize),
            cpu_submit_ms: Vec::with_capacity(steps as usize),
        });
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub async fn finish_timing_window(&mut self) -> Option<TimingWindow> {
        let metrics = self.metrics.take()?;
        let steps = metrics.submitted;
        if steps == 0 {
            return None;
        }
        poll_until_idle(&self.device);
        let workload_size = u64::from(steps) * WORKLOAD_STEP_STRIDE;
        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("physics-metrics-readback"),
            });
        encoder.copy_buffer_to_buffer(
            &metrics.workload,
            0,
            &metrics.workload_staging,
            0,
            workload_size,
        );
        if let Some(ts) = &metrics.timestamp {
            encoder.copy_buffer_to_buffer(
                &ts.resolve,
                0,
                &ts.staging,
                0,
                u64::from(steps) * TIMESTAMP_STEP_STRIDE,
            );
        }
        self.queue.submit(Some(encoder.finish()));
        poll_until_idle(&self.device);

        let workload_slice = metrics.workload_staging.slice(..workload_size);
        let (tx, rx) = oneshot();
        workload_slice.map_async(wgpu::MapMode::Read, move |r| {
            let _ = tx.send(r);
        });
        poll_until_idle(&self.device);
        rx.recv().ok()?.ok()?;
        let workload_data = workload_slice.get_mapped_range();
        let words: &[u32] = bytemuck::cast_slice(&workload_data);
        let mut workload = WorkloadStats::default();
        for values in words.chunks_exact(WORKLOAD_STEP_WORDS as usize) {
            accumulate_counter(
                &mut workload.candidate_pairs_avg,
                &mut workload.candidate_pairs_peak,
                values[4],
            );
            accumulate_counter(
                &mut workload.cell_inserts_avg,
                &mut workload.cell_inserts_peak,
                values[5],
            );
            accumulate_counter(
                &mut workload.unique_pairs_avg,
                &mut workload.unique_pairs_peak,
                values[2],
            );
            accumulate_counter(
                &mut workload.narrowphase_pairs_avg,
                &mut workload.narrowphase_pairs_peak,
                values[3],
            );
            accumulate_counter(
                &mut workload.occupied_contact_slots_avg,
                &mut workload.occupied_contact_slots_peak,
                values[10],
            );
            let capacity_drops = values[6]
                .saturating_add(values[7])
                .saturating_add(values[8])
                .saturating_add(values[9]);
            accumulate_counter(
                &mut workload.capacity_drops_avg,
                &mut workload.capacity_drops_peak,
                capacity_drops,
            );
        }
        let divisor = f64::from(steps);
        workload.candidate_pairs_avg /= divisor;
        workload.cell_inserts_avg /= divisor;
        workload.unique_pairs_avg /= divisor;
        workload.narrowphase_pairs_avg /= divisor;
        workload.occupied_contact_slots_avg /= divisor;
        workload.capacity_drops_avg /= divisor;
        drop(workload_data);
        metrics.workload_staging.unmap();

        let mut result = TimingWindow {
            steps,
            cpu_encode_ms: mean(&metrics.cpu_encode_ms),
            cpu_submit_ms: mean(&metrics.cpu_submit_ms),
            workload,
            ..Default::default()
        };
        if let Some(ts) = &metrics.timestamp {
            let size = u64::from(steps) * TIMESTAMP_STEP_STRIDE;
            let slice = ts.staging.slice(..size);
            let (tx, rx) = oneshot();
            slice.map_async(wgpu::MapMode::Read, move |r| {
                let _ = tx.send(r);
            });
            poll_until_idle(&self.device);
            rx.recv().ok()?.ok()?;
            let data = slice.get_mapped_range();
            let timestamps: &[u64] = bytemuck::cast_slice(&data);
            let period = self.queue.get_timestamp_period() as f64 / 1e6;
            let mut collide = 0.0;
            let mut solve = 0.0;
            let mut integrate = 0.0;
            let mut broadphase = 0.0;
            let mut narrowphase = 0.0;
            let mut graph = 0.0;
            let mut prepare = 0.0;
            let mut device = 0.0;
            for step_values in timestamps.chunks_exact((TIMESTAMP_STEP_STRIDE / 8) as usize) {
                let t = &step_values[..TIMESTAMP_QUERY_COUNT as usize];
                collide += t[3].wrapping_sub(t[0]) as f64 * period;
                solve += t[5].wrapping_sub(t[4]) as f64 * period;
                integrate += t[6].wrapping_sub(t[5]) as f64 * period;
                broadphase += t[1].wrapping_sub(t[0]) as f64 * period;
                narrowphase += t[2].wrapping_sub(t[1]) as f64 * period;
                graph += t[3].wrapping_sub(t[2]) as f64 * period;
                prepare += t[4].wrapping_sub(t[3]) as f64 * period;
                device += t[6].wrapping_sub(t[0]) as f64 * period;
            }
            result.gpu_collide_ms = Some(collide / divisor);
            result.gpu_solve_ms = Some(solve / divisor);
            result.gpu_integrate_ms = Some(integrate / divisor);
            result.gpu_broadphase_ms = Some(broadphase / divisor);
            result.gpu_narrowphase_ms = Some(narrowphase / divisor);
            result.gpu_graph_ms = Some(graph / divisor);
            result.gpu_prepare_ms = Some(prepare / divisor);
            result.gpu_device_ms = Some(device / divisor);
            drop(data);
            ts.staging.unmap();
        }
        Some(result)
    }

    pub fn allocation_stats(&self) -> AllocationStats {
        let body_bytes = (mem::size_of::<BodyStateGpu>() + mem::size_of::<BodyColdGpu>() + 32)
            as u64
            * u64::from(self.caps.bodies.max(1));
        let shape_bytes = mem::size_of::<ShapeGpu>() as u64 * u64::from(self.caps.shapes)
            + 16 * u64::from(
                self.params.hull_point_count
                    + self.params.hull_plane_count
                    + self.params.hull_edge_count,
            );
        let contact_bytes = (mem::size_of::<ContactHotGpu>()
            + mem::size_of::<ContactPersistentGpu>()
            + mem::size_of::<ContactPreparedGpu>()) as u64
            * u64::from(self.contact_slots.max(1));
        let joint_bytes =
            mem::size_of::<JointGpu>() as u64 * u64::from(self.params.joint_count.max(1));
        let scratch_bytes = self.scratch.size();
        let atom_bytes = self.atom.size();
        let fixed_bytes = self.pass_lut.size() + self.indirect.size() + self.query.size();
        AllocationStats {
            body_bytes,
            shape_bytes,
            contact_bytes,
            joint_bytes,
            scratch_bytes,
            atom_bytes,
            total_bytes: body_bytes
                + shape_bytes
                + contact_bytes
                + joint_bytes
                + scratch_bytes
                + atom_bytes
                + fixed_bytes,
        }
    }

    #[cfg(not(target_arch = "wasm32"))]
    fn ensure_staging(&mut self, size: u64) {
        let required = size.max(256);
        if self
            .staging
            .as_ref()
            .is_some_and(|buffer| buffer.size() >= required)
        {
            return;
        }
        self.staging = Some(self.device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("readback-staging"),
            size: required.next_power_of_two(),
            usage: wgpu::BufferUsages::MAP_READ | wgpu::BufferUsages::COPY_DST,
            mapped_at_creation: false,
        }));
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub async fn read_bodies(&mut self) -> Vec<BodyGpu> {
        let state_size = (mem::size_of::<BodyStateGpu>() * self.count as usize) as u64;
        let cold_size = (mem::size_of::<BodyColdGpu>() * self.count as usize) as u64;
        let island_size = mem::size_of::<u32>() as u64 * u64::from(self.count);
        let size = state_size + cold_size + island_size;
        self.ensure_staging(size);
        let staging = self.staging.as_ref().expect("staging buffer");
        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("readback"),
            });
        encoder.copy_buffer_to_buffer(&self.bodies, 0, staging, 0, state_size);
        encoder.copy_buffer_to_buffer(&self.body_cold, 0, staging, state_size, cold_size);
        let island_word = 16
            + crate::types::HASH_BUCKETS
            + crate::types::PAIR_CAP
            + 2 * crate::types::CONTACT_HASH_CAP
            + 6 * self.count;
        encoder.copy_buffer_to_buffer(
            &self.atom,
            u64::from(island_word) * mem::size_of::<u32>() as u64,
            staging,
            state_size + cold_size,
            island_size,
        );
        self.queue.submit(Some(encoder.finish()));

        let slice = staging.slice(..size);
        let (tx, rx) = oneshot();
        slice.map_async(wgpu::MapMode::Read, move |r| {
            let _ = tx.send(r);
        });
        poll_until_idle(&self.device);
        rx.recv().expect("map_async dropped").expect("map failed");
        let data = slice.get_mapped_range();
        let state_end = state_size as usize;
        let cold_end = state_end + cold_size as usize;
        let states: &[BodyStateGpu] = bytemuck::cast_slice(&data[..state_end]);
        let cold: &[BodyColdGpu] = bytemuck::cast_slice(&data[state_end..cold_end]);
        let islands: &[u32] = bytemuck::cast_slice(&data[cold_end..]);
        let bodies = decode_body_gpu(states, cold, islands);
        drop(data);
        staging.unmap();
        bodies
    }

    pub fn write_body_states(&self, bodies: &[BodyGpu]) {
        self.invalidate_idle_proof();
        let states: Vec<BodyStateGpu> = bodies.iter().map(BodyStateGpu::from_body).collect();
        if !states.is_empty() {
            self.queue
                .write_buffer(&self.bodies, 0, bytemuck::cast_slice(&states));
        }
    }

    pub fn write_body_state_at(&self, slot: u32, body: &BodyGpu) {
        self.invalidate_idle_proof();
        let state = BodyStateGpu::from_body(body);
        let off = u64::from(slot) * mem::size_of::<BodyStateGpu>() as u64;
        self.queue
            .write_buffer(&self.bodies, off, bytemuck::bytes_of(&state));
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub async fn read_joints(&mut self) -> Vec<JointGpu> {
        let count = self.params.joint_count as usize;
        if count == 0 {
            return Vec::new();
        }
        let size = (mem::size_of::<JointGpu>() * count) as u64;
        self.ensure_staging(size);
        let staging = self.staging.as_ref().expect("staging buffer");
        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("joint-readback"),
            });
        encoder.copy_buffer_to_buffer(&self.joints, 0, staging, 0, size);
        self.queue.submit(Some(encoder.finish()));

        let slice = staging.slice(..size);
        let (tx, rx) = oneshot();
        slice.map_async(wgpu::MapMode::Read, move |result| {
            let _ = tx.send(result);
        });
        poll_until_idle(&self.device);
        rx.recv().expect("map_async dropped").expect("map failed");
        let data = slice.get_mapped_range();
        let joints = bytemuck::cast_slice::<u8, JointGpu>(&data).to_vec();
        drop(data);
        staging.unmap();
        joints
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub async fn read_contacts(&mut self) -> Vec<ContactGpu> {
        let slots = self.contact_slots.max(1) as usize;
        let hot_size = (mem::size_of::<ContactHotGpu>() * slots) as u64;
        let persistent_size = (mem::size_of::<ContactPersistentGpu>() * slots) as u64;
        let prepared_size = (mem::size_of::<ContactPreparedGpu>() * slots) as u64;
        let size = hot_size + persistent_size + prepared_size;
        self.ensure_staging(size);
        let staging = self.staging.as_ref().expect("staging buffer");
        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("contact-readback"),
            });
        encoder.copy_buffer_to_buffer(&self.contacts, 0, staging, 0, hot_size);
        encoder.copy_buffer_to_buffer(
            &self.contact_persistent,
            0,
            staging,
            hot_size,
            persistent_size,
        );
        encoder.copy_buffer_to_buffer(
            &self.contact_prepared,
            0,
            staging,
            hot_size + persistent_size,
            prepared_size,
        );
        self.queue.submit(Some(encoder.finish()));

        let slice = staging.slice(..size);
        let (tx, rx) = oneshot();
        slice.map_async(wgpu::MapMode::Read, move |r| {
            let _ = tx.send(r);
        });
        poll_until_idle(&self.device);
        rx.recv().expect("map_async dropped").expect("map failed");
        let data = slice.get_mapped_range();
        let hot_end = hot_size as usize;
        let persistent_end = hot_end + persistent_size as usize;
        let hot: &[ContactHotGpu] = bytemuck::cast_slice(&data[..hot_end]);
        let persistent: &[ContactPersistentGpu] =
            bytemuck::cast_slice(&data[hot_end..persistent_end]);
        let prepared: &[ContactPreparedGpu] = bytemuck::cast_slice(&data[persistent_end..]);
        let contacts = decode_contacts(hot, persistent, prepared);
        drop(data);
        staging.unmap();
        contacts
    }

    /// Bodies always. Joints/contacts only when the host mirror needs them.
    /// Typical sample HUD time was dominated by copying every contact slot
    /// (~0.5 KB × capacity, often ~2 MB) even when no events were enabled.
    #[cfg(not(target_arch = "wasm32"))]
    pub fn read_world_mirror(
        &mut self,
        copy_joints: bool,
        copy_contacts: bool,
    ) -> (Vec<BodyGpu>, Vec<JointGpu>, Vec<ContactGpu>, Vec<u32>, Vec<u32>) {
        let count = self.count as usize;
        let state_size = (mem::size_of::<BodyStateGpu>() * count) as u64;
        let cold_size = (mem::size_of::<BodyColdGpu>() * count) as u64;
        let island_size = mem::size_of::<u32>() as u64 * u64::from(self.count);
        let joint_count = if copy_joints {
            self.params.joint_count as usize
        } else {
            0
        };
        let joint_size = (mem::size_of::<JointGpu>() * joint_count) as u64;
        let slots = if copy_contacts {
            self.contact_slots.max(1) as usize
        } else {
            0
        };
        let hot_size = (mem::size_of::<ContactHotGpu>() * slots) as u64;
        let persistent_size = (mem::size_of::<ContactPersistentGpu>() * slots) as u64;
        let prepared_size = (mem::size_of::<ContactPreparedGpu>() * slots) as u64;
        let mut off = 0u64;
        let body_off = off;
        off += state_size;
        let cold_off = off;
        off += cold_size;
        let island_off = off;
        off += island_size;
        let joint_off = off;
        off += joint_size;
        let hot_off = off;
        off += hot_size;
        let persistent_off = off;
        off += persistent_size;
        let prepared_off = off;
        off += prepared_size;
        let previous_off = off;
        let previous_size = (slots * mem::size_of::<u32>()) as u64;
        off += previous_size;
        let start_off=off;
        let start_size=if self.convex_ccd.is_some() {state_size} else {0};
        off+=start_size;
        let size = off.max(256);
        self.ensure_staging(size);
        let staging = self.staging.as_ref().expect("staging buffer").clone();
        let copy_start = Instant::now();
        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("world-mirror-readback"),
            });
        encoder.copy_buffer_to_buffer(&self.bodies, 0, &staging, body_off, state_size);
        encoder.copy_buffer_to_buffer(&self.body_cold, 0, &staging, cold_off, cold_size);
        let island_word = 16
            + crate::types::HASH_BUCKETS
            + crate::types::PAIR_CAP
            + 2 * crate::types::CONTACT_HASH_CAP
            + 6 * self.count;
        encoder.copy_buffer_to_buffer(
            &self.atom,
            u64::from(island_word) * mem::size_of::<u32>() as u64,
            &staging,
            island_off,
            island_size,
        );
        if joint_size > 0 {
            encoder.copy_buffer_to_buffer(&self.joints, 0, &staging, joint_off, joint_size);
        }
        if copy_contacts {
            encoder.copy_buffer_to_buffer(
                &self.scratch,
                u64::from(crate::types::SCR_PREVIOUS_TOUCHING) * 4,
                &staging, previous_off, previous_size,
            );
            encoder.copy_buffer_to_buffer(&self.contacts, 0, &staging, hot_off, hot_size);
            encoder.copy_buffer_to_buffer(
                &self.contact_persistent,
                0,
                &staging,
                persistent_off,
                persistent_size,
            );
            encoder.copy_buffer_to_buffer(
                &self.contact_prepared,
                0,
                &staging,
                prepared_off,
                prepared_size,
            );
        }
        if let Some(ccd)=&self.convex_ccd {
            encoder.copy_buffer_to_buffer(&ccd.start,0,&staging,start_off,start_size);
        }
        let copy_ms = copy_start.elapsed().as_secs_f32() * 1e3;
        self.record_submit(self.queue.submit(Some(encoder.finish())));

        let slice = staging.slice(..size);
        let (tx, rx) = oneshot();
        slice.map_async(wgpu::MapMode::Read, move |r| {
            let _ = tx.send(r);
        });
        let map_start = Instant::now();
        poll_until_idle(&self.device);
        rx.recv().expect("map_async dropped").expect("map failed");
        let map_ms = map_start.elapsed().as_secs_f32() * 1e3;
        self.last_mirror_copy_ms = copy_ms;
        self.last_mirror_map_ms = map_ms;
        self.last_mirror_bytes = size;
        self.completed_step = self.physics_step;
        self.completed_known = true;
        let data = slice.get_mapped_range();
        let states: &[BodyStateGpu] =
            bytemuck::cast_slice(&data[body_off as usize..cold_off as usize]);
        let cold: &[BodyColdGpu] =
            bytemuck::cast_slice(&data[cold_off as usize..island_off as usize]);
        let islands: &[u32] = bytemuck::cast_slice(&data[island_off as usize..joint_off as usize]);
        let bodies = decode_body_gpu(states, cold, islands);
        let joints = if joint_size == 0 {
            Vec::new()
        } else {
            bytemuck::cast_slice::<u8, JointGpu>(&data[joint_off as usize..hot_off as usize])
                .to_vec()
        };
        let contacts = if !copy_contacts {
            Vec::new()
        } else {
            let hot: &[ContactHotGpu] =
                bytemuck::cast_slice(&data[hot_off as usize..persistent_off as usize]);
            let persistent: &[ContactPersistentGpu] =
                bytemuck::cast_slice(&data[persistent_off as usize..prepared_off as usize]);
            let prepared: &[ContactPreparedGpu] = bytemuck::cast_slice(&data[prepared_off as usize..previous_off as usize]);
            decode_contacts(hot, persistent, prepared)
        };
        let previous_touching = if copy_contacts {
            bytemuck::cast_slice::<u8, u32>(&data[previous_off as usize..start_off as usize]).to_vec()
        } else { Vec::new() };
        let start_flags=if start_size>0 {
            bytemuck::cast_slice::<u8,BodyStateGpu>(&data[start_off as usize..off as usize])
                .iter().map(|body|body.flags).collect()
        } else {Vec::new()};
        drop(data);
        staging.unmap();
        (bodies, joints, contacts, previous_touching, start_flags)
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn set_mirror_wait_ms(&mut self, wait_ms: f32) {
        self.last_mirror_wait_ms = wait_ms;
    }

    /// GPU compact joint lists from the last prepared step.
    #[cfg(not(target_arch = "wasm32"))]
    pub fn read_joint_lists(&mut self) -> (bool, Vec<(u32, Vec<u32>)>) {
        let n = self.count;
        let joints = self.params.joint_count;
        if joints == 0 {
            return (true, Vec::new());
        }
        let head = crate::types::joint_head_live(n, self.params.contact_capacity);
        let unique = head + n;
        let offb = unique + n;
        let cntb = offb + n;
        let listb = cntb + n;
        let words = listb + n.max(joints) + 1;
        let byte_off = u64::from(head) * 4;
        let size = u64::from(words.saturating_sub(head)) * 4;
        let total = size + 8;
        self.ensure_staging(total.max(256));
        let staging = self.staging.as_ref().expect("staging buffer").clone();
        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("joint-list-readback"),
            });
        encoder.copy_buffer_to_buffer(&self.scratch, byte_off, &staging, 0, size);
        encoder.copy_buffer_to_buffer(&self.scratch, 62 * 4, &staging, size, 8);
        self.record_submit(self.queue.submit(Some(encoder.finish())));
        let slice = staging.slice(..total);
        let (tx, rx) = oneshot();
        slice.map_async(wgpu::MapMode::Read, move |r| {
            let _ = tx.send(r);
        });
        poll_until_idle(&self.device);
        rx.recv().expect("map_async dropped").expect("map failed");
        let data = slice.get_mapped_range();
        let meta: [u32; 2] = bytemuck::cast_slice(&data[size as usize..])
            .try_into()
            .unwrap_or([0, 0]);
        let list_ok = meta[0] == 1;
        let ncomp = meta[1];
        let body = bytemuck::cast_slice::<u8, u32>(&data[..size as usize]);
        let mut out = Vec::new();
        if list_ok {
            for c in 0..ncomp as usize {
                let root = *body.get((unique - head + c as u32) as usize).unwrap_or(&u32::MAX);
                let off = *body.get((offb - head + c as u32) as usize).unwrap_or(&0);
                let count = *body.get((cntb - head + c as u32) as usize).unwrap_or(&0);
                let mut ids = Vec::with_capacity(count as usize);
                for k in 0..count {
                    let idx = (listb - head + off + k) as usize;
                    if let Some(&j) = body.get(idx) {
                        ids.push(j);
                    }
                }
                out.push((root, ids));
            }
        }
        drop(data);
        staging.unmap();
        (list_ok, out)
    }

    /// Contact slots for the compact, pair-key-sorted candidate list produced
    /// by the current callback detection stage.
    #[cfg(not(target_arch = "wasm32"))]
    pub async fn read_callback_pairs(&mut self) -> Vec<u32> {
        let pairs_size = u64::from(crate::types::PAIR_CAP) * 4;
        let size = 4 + pairs_size;
        self.ensure_staging(size);
        let staging = self.staging.as_ref().expect("staging buffer");
        let pairs_word = 64 + crate::types::HASH_BUCKETS + 3 * crate::types::MAX_INSERTS;
        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("callback-pairs-readback"),
            });
        encoder.copy_buffer_to_buffer(&self.scratch, 2 * 4, staging, 0, 4);
        encoder.copy_buffer_to_buffer(
            &self.scratch,
            u64::from(pairs_word) * 4,
            staging,
            4,
            pairs_size,
        );
        self.queue.submit(Some(encoder.finish()));
        let slice = staging.slice(..size);
        let (tx, rx) = oneshot();
        slice.map_async(wgpu::MapMode::Read, move |result| {
            let _ = tx.send(result);
        });
        poll_until_idle(&self.device);
        rx.recv().expect("map_async dropped").expect("map failed");
        let data = slice.get_mapped_range();
        let count = u32::from_ne_bytes(data[..4].try_into().unwrap()).min(crate::types::PAIR_CAP);
        let pairs = bytemuck::cast_slice::<u8, u32>(&data[4..])[..count as usize].to_vec();
        drop(data);
        staging.unmap();
        pairs
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub fn write_callback_pairs(&self, pairs: &[u32]) {
        let count = (pairs.len() as u32).min(crate::types::PAIR_CAP);
        let pairs_word = 64 + crate::types::HASH_BUCKETS + 3 * crate::types::MAX_INSERTS;
        if count > 0 {
            self.queue.write_buffer(
                &self.scratch,
                u64::from(pairs_word) * 4,
                bytemuck::cast_slice(&pairs[..count as usize]),
            );
        }
        self.queue
            .write_buffer(&self.scratch, 2 * 4, bytemuck::bytes_of(&[count, count]));
        let indirect = [count.div_ceil(WORKGROUP_SIZE), 1, 1, 0];
        self.queue
            .write_buffer(&self.scratch, 8 * 4, bytemuck::bytes_of(&indirect));
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub async fn read_callback_slots(&mut self) -> Vec<u32> {
        const COUNT_SIZE: u64 = 4;
        let slots_size = u64::from(crate::types::PAIR_CAP) * 4;
        let size = COUNT_SIZE + slots_size;
        self.ensure_staging(size);
        let staging = self.staging.as_ref().expect("staging buffer");
        let active_word = 64
            + crate::types::HASH_BUCKETS
            + 3 * crate::types::MAX_INSERTS
            + 2 * crate::types::PAIR_CAP;
        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("callback-slots-readback"),
            });
        encoder.copy_buffer_to_buffer(&self.scratch, 2 * 4, staging, 0, COUNT_SIZE);
        encoder.copy_buffer_to_buffer(
            &self.scratch,
            u64::from(active_word) * 4,
            staging,
            COUNT_SIZE,
            slots_size,
        );
        self.queue.submit(Some(encoder.finish()));
        let slice = staging.slice(..size);
        let (tx, rx) = oneshot();
        slice.map_async(wgpu::MapMode::Read, move |result| {
            let _ = tx.send(result);
        });
        poll_until_idle(&self.device);
        rx.recv().expect("map_async dropped").expect("map failed");
        let data = slice.get_mapped_range();
        let count = u32::from_ne_bytes(data[..4].try_into().unwrap()).min(crate::types::PAIR_CAP);
        let slots = bytemuck::cast_slice::<u8, u32>(&data[4..])[..count as usize].to_vec();
        drop(data);
        staging.unmap();
        slots
    }

    /// Disable complete manifolds before coloring/prepare. Queue writes are
    /// ordered before the callback-finish submission.
    #[cfg(not(target_arch = "wasm32"))]
    pub fn disable_callback_contacts(&mut self, roots: &[(u32, u32)], snapshot: &[ContactGpu]) -> Result<(), &'static str> {
        let contacts = match callback_disabled_members(roots, snapshot) {
            Ok(members) => members,
            Err(error) => { self.mark_physics_invalid(); return Err(error); }
        };
        let hot_stride = mem::size_of::<ContactHotGpu>() as u64;
        let persistent_stride = mem::size_of::<ContactPersistentGpu>() as u64;
        let lifecycle_offset = mem::offset_of!(ContactPersistentGpu, lifecycle) as u64;
        for &(slot, lifecycle_flags) in &contacts {
            self.queue.write_buffer(
                &self.contacts,
                u64::from(slot) * hot_stride + 12,
                &0u32.to_ne_bytes(),
            );
            // A veto is not a cached geometric miss. Rebuild next time so a
            // stationary contact can be accepted again without moving first.
            self.queue.write_buffer(
                &self.contact_persistent,
                u64::from(slot) * persistent_stride + mem::offset_of!(ContactPersistentGpu, cached_relative) as u64 + 12,
                &0f32.to_ne_bytes(),
            );
            self.queue.write_buffer(
                &self.contact_persistent,
                u64::from(slot) * persistent_stride + lifecycle_offset + 4,
                &lifecycle_flags.to_ne_bytes(),
            );
        }
        Ok(())
    }

    /// Broadphase capacity losses from the most recently submitted world step.
    #[cfg(not(target_arch = "wasm32"))]
    pub async fn read_broadphase_stats(&mut self) -> BroadphaseStats {
        const SIZE: u64 = 7 * 4;
        self.ensure_staging(SIZE);
        let staging = self.staging.as_ref().expect("staging buffer");
        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("broadphase-stats-readback"),
            });
        encoder.copy_buffer_to_buffer(&self.atom, 0, staging, 0, SIZE);
        self.queue.submit(Some(encoder.finish()));

        let slice = staging.slice(..SIZE);
        let (tx, rx) = oneshot();
        slice.map_async(wgpu::MapMode::Read, move |result| {
            let _ = tx.send(result);
        });
        poll_until_idle(&self.device);
        rx.recv().expect("map_async dropped").expect("map failed");
        let data = slice.get_mapped_range();
        let stats = BroadphaseStats::from_atom_words(bytemuck::cast_slice(&data));
        drop(data);
        staging.unmap();
        stats
    }

    /// Unique-pair / contact counts plus capacity-loss counters from the last step.
    #[cfg(not(target_arch = "wasm32"))]
    pub async fn read_live_step_stats(&mut self) -> LiveStepStats {
        const ATOM_SIZE: u64 = 16 * 4;
        const SCRATCH_SIZE: u64 = 16;
        const SIZE: u64 = ATOM_SIZE + SCRATCH_SIZE + 4;
        self.ensure_staging(SIZE);
        let staging = self.staging.as_ref().expect("staging buffer");
        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("live-step-stats-readback"),
            });
        encoder.copy_buffer_to_buffer(&self.atom, 0, staging, 0, ATOM_SIZE);
        encoder.copy_buffer_to_buffer(&self.scratch, 0, staging, ATOM_SIZE, SCRATCH_SIZE);
        encoder.copy_buffer_to_buffer(&self.query, 66 * 4, staging, ATOM_SIZE + SCRATCH_SIZE, 4);
        self.queue.submit(Some(encoder.finish()));

        let slice = staging.slice(..SIZE);
        let (tx, rx) = oneshot();
        slice.map_async(wgpu::MapMode::Read, move |result| {
            let _ = tx.send(result);
        });
        poll_until_idle(&self.device);
        rx.recv().expect("map_async dropped").expect("map failed");
        let data = slice.get_mapped_range();
        let words: &[u32] = bytemuck::cast_slice(&data);
        let drops = BroadphaseStats::from_atom_words(words);
        let sticky = BroadphaseStats::sticky_from_atom_words(words);
        let first_fail_step = words
            .get(crate::types::ATOM_STICKY_FIRST_STEP as usize)
            .copied()
            .unwrap_or(0);
        let unique_pairs = words.get(16 + 2).copied().unwrap_or(0);
        let narrowphase_pairs = words.get(16 + 3).copied().unwrap_or(0);
        let contact_drop_reasons = words.get(20).copied().unwrap_or(0);
        let graph_proof_fail = words
            .get(crate::types::ATOM_GRAPH_PROOF_FAIL as usize)
            .copied()
            .unwrap_or(0);
        drop(data);
        staging.unmap();
        if graph_proof_fail != 0 {
            self.physics_invalid = true;
        }
        LiveStepStats {
            drops,
            sticky,
            first_fail_step,
            unique_pairs,
            narrowphase_pairs,
            graph_proof_fail,
            contact_drop_reasons,
        }
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub async fn read_phase_words(&mut self) -> Vec<u32> {
        let start_words = crate::types::SCRATCH_U32 + 16 * self.count;
        let word_count = 24 * 8 + 24 * 4 * self.count;
        let offset = u64::from(start_words) * 4;
        let size = u64::from(word_count) * 4;
        self.ensure_staging(size);
        let staging = self.staging.as_ref().expect("staging buffer");
        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("phase-readback"),
            });
        encoder.copy_buffer_to_buffer(&self.scratch, offset, staging, 0, size);
        self.queue.submit(Some(encoder.finish()));

        let slice = staging.slice(..size);
        let (tx, rx) = oneshot();
        slice.map_async(wgpu::MapMode::Read, move |result| {
            let _ = tx.send(result);
        });
        poll_until_idle(&self.device);
        rx.recv().expect("map_async dropped").expect("map failed");
        let data = slice.get_mapped_range();
        let words = bytemuck::cast_slice(&data).to_vec();
        drop(data);
        staging.unmap();
        words
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub async fn read_mesh_candidate_words(&mut self) -> Vec<u32> {
        if self.params.diagnostic_flags & crate::types::DIAG_MESH_CANDIDATES == 0 { return Vec::new(); }
        let word_count = crate::types::MESH_TRACE_WORDS;
        let offset = self.atom.size() - u64::from(word_count) * 4;
        let size = u64::from(word_count) * 4;
        self.ensure_staging(size);
        let staging = self.staging.as_ref().expect("staging buffer");
        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("mesh-candidate-readback"),
            });
        encoder.copy_buffer_to_buffer(&self.atom, offset, staging, 0, size);
        self.queue.submit(Some(encoder.finish()));

        let slice = staging.slice(..size);
        let (tx, rx) = oneshot();
        slice.map_async(wgpu::MapMode::Read, move |result| {
            let _ = tx.send(result);
        });
        poll_until_idle(&self.device);
        rx.recv().expect("map_async dropped").expect("map failed");
        let data = slice.get_mapped_range();
        let words = bytemuck::cast_slice(&data).to_vec();
        drop(data);
        staging.unmap();
        words
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub async fn debug_compact_unique(
        &mut self,
        pairs: &[u32],
        radix_garbage: u32,
    ) -> (u32, u32, Vec<u32>) {
        let n = (pairs.len() as u32).min(crate::types::PAIR_CAP);
        let garbage = vec![radix_garbage; crate::types::RADIX_BUCKETS as usize];
        self.queue.write_buffer(
            &self.scratch,
            u64::from(crate::types::SCR_RADIX_BASE) * 4,
            bytemuck::cast_slice(&garbage),
        );
        if n > 0 {
            self.queue.write_buffer(
                &self.scratch,
                u64::from(crate::types::SCR_PAIRS) * 4,
                bytemuck::cast_slice(&pairs[..n as usize]),
            );
        }
        self.queue
            .write_buffer(&self.atom, 0, bytemuck::bytes_of(&n));
        let groups = n.div_ceil(crate::types::RADIX_GROUP_SIZE).max(1);
        let off = Self::pass_lut_offset(0, 1) as u32;
        let mut enc = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("debug-compact-unique"),
            });
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("debug-compact-unique"),
                timestamp_writes: None,
            });
            pass.set_bind_group(0, self.ping_bg(), &[off]);
            pass.set_pipeline(&self.compact_unique_histogram);
            pass.dispatch_workgroups(groups, 1, 1);
            pass.set_pipeline(&self.compact_unique_bases);
            pass.dispatch_workgroups(1, 1, 1);
            pass.set_pipeline(&self.compact_unique_scatter);
            pass.dispatch_workgroups(groups, 1, 1);
            pass.set_pipeline(&self.compact_unique_gather);
            pass.dispatch_workgroups(groups, 1, 1);
        }
        let header_bytes = 32u64;
        let size = header_bytes + u64::from(n) * 4;
        self.ensure_staging(size.max(32));
        let staging = self.staging.as_ref().expect("staging buffer");
        enc.copy_buffer_to_buffer(&self.scratch, 0, staging, 0, header_bytes);
        if n > 0 {
            enc.copy_buffer_to_buffer(
                &self.scratch,
                u64::from(crate::types::SCR_PAIRS) * 4,
                staging,
                header_bytes,
                u64::from(n) * 4,
            );
        }
        self.queue.submit(Some(enc.finish()));
        let slice = staging.slice(..size);
        let (tx, rx) = oneshot();
        slice.map_async(wgpu::MapMode::Read, move |result| {
            let _ = tx.send(result);
        });
        poll_until_idle(&self.device);
        rx.recv().expect("map_async dropped").expect("map failed");
        let data = slice.get_mapped_range();
        let header: &[u32] = bytemuck::cast_slice(&data[..header_bytes as usize]);
        let unique = header[crate::types::SCR_UNIQUE_N as usize];
        let raw = header[crate::types::SCR_POW2 as usize];
        let compacted = if n == 0 {
            Vec::new()
        } else {
            bytemuck::cast_slice(&data[header_bytes as usize..]).to_vec()
        };
        drop(data);
        staging.unmap();
        (unique, raw, compacted)
    }

    #[cfg(all(test,not(target_arch="wasm32")))]
    pub(crate) fn test_graph_memo_hits(&mut self)->Option<u32> {
        let base=self.graph_memo_base?;
        self.ensure_staging(4);
        let staging=self.staging.as_ref().unwrap();
        let mut encoder=self.device.create_command_encoder(&wgpu::CommandEncoderDescriptor{label:Some("test-memo-hits")});
        encoder.copy_buffer_to_buffer(&self.query,u64::from(base+3)*4,staging,0,4);
        self.queue.submit(Some(encoder.finish()));
        let slice=staging.slice(..4);let (tx,rx)=oneshot();
        slice.map_async(wgpu::MapMode::Read,move |r|{let _=tx.send(r);});
        poll_until_idle(&self.device);rx.recv().unwrap().unwrap();
        let data=slice.get_mapped_range();let result=bytemuck::cast_slice::<u8,u32>(&data)[0];
        drop(data);staging.unmap();Some(result)
    }

    #[cfg(all(test,not(target_arch="wasm32")))]
    pub(crate) fn test_large_component_count(&mut self)->u32 {
        self.ensure_staging(4);
        let staging=self.staging.as_ref().unwrap();
        let mut encoder=self.device.create_command_encoder(&wgpu::CommandEncoderDescriptor{label:Some("test-component-count")});
        encoder.copy_buffer_to_buffer(&self.query,256*4,staging,0,4);
        self.queue.submit(Some(encoder.finish()));
        let slice=staging.slice(..4);let (tx,rx)=oneshot();
        slice.map_async(wgpu::MapMode::Read,move |r|{let _=tx.send(r);});
        poll_until_idle(&self.device);rx.recv().unwrap().unwrap();
        let data=slice.get_mapped_range();let result=bytemuck::cast_slice::<u8,u32>(&data)[0];
        drop(data);staging.unmap();result
    }

    #[cfg(not(target_arch = "wasm32"))]
    pub async fn read_scratch_prefix(&mut self, words: u32) -> Vec<u32> {
        let size = u64::from(words) * 4;
        self.ensure_staging(size);
        let staging = self.staging.as_ref().expect("staging buffer");
        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("scratch-prefix-readback"),
            });
        encoder.copy_buffer_to_buffer(&self.scratch, 0, staging, 0, size);
        self.queue.submit(Some(encoder.finish()));
        let slice = staging.slice(..size);
        let (tx, rx) = oneshot();
        slice.map_async(wgpu::MapMode::Read, move |result| {
            let _ = tx.send(result);
        });
        poll_until_idle(&self.device);
        rx.recv().expect("map_async dropped").expect("map failed");
        let data = slice.get_mapped_range();
        let out = bytemuck::cast_slice(&data).to_vec();
        drop(data);
        staging.unmap();
        out
    }
}

fn storage_entry(
    binding: u32,
    ty: wgpu::BufferBindingType,
    dynamic: bool,
) -> wgpu::BindGroupLayoutEntry {
    wgpu::BindGroupLayoutEntry {
        binding,
        visibility: wgpu::ShaderStages::COMPUTE,
        ty: wgpu::BindingType::Buffer {
            ty,
            has_dynamic_offset: dynamic,
            min_binding_size: if dynamic {
                std::num::NonZeroU64::new(mem::size_of::<SimParams>() as u64)
            } else {
                None
            },
        },
        count: None,
    }
}

fn physics_bind_group(
    device: &Device,
    layout: &BindGroupLayout,
    params: &Buffer,
    src: &Buffer,
    dst: &Buffer,
    contacts: &Buffer,
    contact_persistent: &Buffer,
    contact_prepared: &Buffer,
    joints: &Buffer,
    scratch: &Buffer,
    atom: &Buffer,
    query: &Buffer,
) -> BindGroup {
    device.create_bind_group(&wgpu::BindGroupDescriptor {
        label: Some("physics-bg"),
        layout,
        entries: &[
            wgpu::BindGroupEntry {
                binding: 0,
                resource: wgpu::BindingResource::Buffer(wgpu::BufferBinding {
                    buffer: params,
                    offset: 0,
                    size: Some(
                        std::num::NonZeroU64::new(mem::size_of::<SimParams>() as u64).unwrap(),
                    ),
                }),
            },
            wgpu::BindGroupEntry {
                binding: 1,
                resource: src.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 2,
                resource: dst.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 3,
                resource: contacts.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 4,
                resource: joints.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 5,
                resource: scratch.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 6,
                resource: atom.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 7,
                resource: contact_persistent.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 8,
                resource: contact_prepared.as_entire_binding(),
            },
            wgpu::BindGroupEntry {
                binding: 9,
                resource: query.as_entire_binding(),
            },
        ],
    })
}

fn make_compute_cached(
    device: &Device,
    layout: &wgpu::PipelineLayout,
    shader: &ShaderModule,
    entry: &str,
    cache: Option<&wgpu::PipelineCache>,
) -> ComputePipeline {
    crate::loading::set(&format!("Compiling shader: {entry}"));
    let start = std::env::var_os("GPU_PHYSICS_TRACE_PIPELINES").map(|_| {
        eprintln!("gpu-pipeline begin {entry}");
        std::time::Instant::now()
    });
    let pipeline = device.create_compute_pipeline(&wgpu::ComputePipelineDescriptor {
        label: Some(entry),
        layout: Some(layout),
        module: shader,
        entry_point: Some(entry),
        compilation_options: Default::default(),
        cache,
    });
    if let Some(start) = start {
        eprintln!("gpu-pipeline end {entry} {:.3} ms", start.elapsed().as_secs_f64() * 1000.0);
    }
    pipeline
}

fn make_compute_constant_cached(
    device: &Device,
    layout: &wgpu::PipelineLayout,
    shader: &ShaderModule,
    entry: &str,
    name: &str,
    value: f64,
    cache: Option<&wgpu::PipelineCache>,
) -> ComputePipeline {
    crate::loading::set(&format!("Compiling shader: {entry}"));
    let start = std::env::var_os("GPU_PHYSICS_TRACE_PIPELINES").map(|_| {
        eprintln!("gpu-pipeline begin {entry}");
        std::time::Instant::now()
    });
    let pipeline = device.create_compute_pipeline(&wgpu::ComputePipelineDescriptor {
        label: Some(entry),
        layout: Some(layout),
        module: shader,
        entry_point: Some(entry),
        compilation_options: wgpu::PipelineCompilationOptions {
            constants: &[(name, value)],
            ..Default::default()
        },
        cache,
    });
    if let Some(start) = start {
        eprintln!("gpu-pipeline end {entry} {:.3} ms", start.elapsed().as_secs_f64() * 1000.0);
    }
    pipeline
}

impl BodyGpu {
    pub fn zeroed() -> Self {
        Self {
            pos: [0.0; 3],
            inv_mass: 0.0,
            vel: [0.0; 3],
            kind: 0,
            half: [0.0; 3],
            flags: 1,
            rot: [0.0, 0.0, 0.0, 1.0],
            omega: [0.0; 3],
            restitution: 0.0,
            inv_inertia: [0.0; 3],
            friction: 0.6,
            gravity_scale: 1.0,
            linear_damping: 0.0,
            angular_damping: 0.0,
            rolling: 0.0,
            dp: [0.0; 3],
            sleep_time: 0.0,
            dq: [0.0, 0.0, 0.0, 1.0],
            island_id: u32::MAX,
            sleep_velocity: 0.0,
            _pad_island: [0; 2],
        }
    }
}

impl ContactGpu {
    pub fn empty() -> Self {
        let mut c: Self = bytemuck::Zeroable::zeroed();
        c.a = u32::MAX;
        c.b = u32::MAX;
        c
    }
}

impl ContactHotGpu {
    fn empty() -> Self {
        let mut contact = Self::zeroed();
        contact.a = u32::MAX;
        contact.b = u32::MAX;
        contact
    }
}

impl JointGpu {
    pub fn zeroed() -> Self {
        Self {
            a: 0,
            b: 0,
            kind: 0,
            _pad0: 0,
            anchor_a: [0.0; 3],
            hertz: 60.0,
            anchor_b: [0.0; 3],
            damping: 2.0,
            axis: [1.0, 0.0, 0.0],
            impulse: 0.0,
            ..Self::default()
        }
    }
}

#[cfg(not(target_arch = "wasm32"))]
fn oneshot<T: Send + 'static>() -> (std::sync::mpsc::Sender<T>, std::sync::mpsc::Receiver<T>) {
    std::sync::mpsc::channel()
}

#[cfg(not(target_arch = "wasm32"))]
fn accumulate_counter(sum: &mut f64, peak: &mut u32, value: u32) {
    *sum += f64::from(value);
    *peak = (*peak).max(value);
}

#[cfg(not(target_arch = "wasm32"))]
fn mean(values: &[f64]) -> f64 {
    if values.is_empty() {
        0.0
    } else {
        values.iter().sum::<f64>() / values.len() as f64
    }
}

#[cfg(all(test, not(target_arch = "wasm32")))]
#[path = "manifold_chain_tests.rs"]
mod manifold_chain_tests;

// Validate every chain before issuing any write; never partly apply a veto.
#[cfg(not(target_arch = "wasm32"))]
fn callback_disabled_members(roots: &[(u32, u32)], snapshot: &[ContactGpu]) -> Result<Vec<(u32, u32)>, &'static str> {
    let mut members = Vec::new();
    for &(root, flags) in roots {
        let first = snapshot.get(root as usize).ok_or("pre-solve root out of bounds")?;
        let count = first.manifold_link[2].max(1) as usize;
        if first.manifold_link[1] != 0 || count > snapshot.len() { return Err("invalid pre-solve chain root"); }
        let mut slot = root;
        for i in 0..count {
            let c = snapshot.get(slot as usize).ok_or("pre-solve child out of bounds")?;
            let parent = if i == 0 { 0 } else { root + 1 };
            if c.a == u32::MAX || c.a == c.b || c.count == 0 || c.count > 4
                || c.a != first.a || c.b != first.b || c.manifold_link[1] != parent {
                return Err("invalid pre-solve child ownership");
            }
            members.push((slot, if i == 0 { flags } else { 1 }));
            let next = c.manifold_link[0];
            if i + 1 == count {
                if next != 0 { return Err("unterminated pre-solve chain"); }
            } else { slot = next.checked_sub(1).ok_or("short pre-solve chain")?; }
        }
    }
    Ok(members)
}
