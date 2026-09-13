use bytemuck::{Pod, Zeroable};

pub const WORKGROUP_SIZE: u32 = 64;
pub const DEFAULT_BODY_COUNT: u32 = 256;
/// Spare hull/mesh words when the world def does not hint a peak.
pub const DEFAULT_GEOM_SLACK: u32 = 64;
pub const DEFAULT_JOINT_CAPACITY: u32 = 16;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct GpuSceneCaps {
    pub bodies: u32,
    pub shapes: u32,
    pub hull_points: u32,
    pub hull_planes: u32,
    pub hull_edges: u32,
    pub hull_topology: u32,
    pub mesh_vertices: u32,
    pub mesh_triangles: u32,
    pub mesh_nodes: u32,
    pub materials: u32,
    pub joints: u32,
}

impl GpuSceneCaps {
    pub fn live(
        bodies: u32,
        shapes: u32,
        hull_points: u32,
        hull_planes: u32,
        hull_edges: u32,
        hull_topology: u32,
        mesh_vertices: u32,
        mesh_triangles: u32,
        mesh_nodes: u32,
        materials: u32,
        joints: u32,
    ) -> Self {
        Self {
            bodies,
            shapes,
            hull_points,
            hull_planes,
            hull_edges,
            hull_topology,
            mesh_vertices,
            mesh_triangles,
            mesh_nodes,
            materials,
            joints,
        }
    }

    pub fn allocate(
        live: Self,
        body_hint: u32,
        shape_hint: u32,
    ) -> Self {
        Self {
            bodies: live.bodies.max(body_hint).max(DEFAULT_BODY_COUNT),
            shapes: live.shapes.max(shape_hint).max(DEFAULT_BODY_COUNT),
            hull_points: live.hull_points.max(DEFAULT_GEOM_SLACK),
            hull_planes: live.hull_planes.max(DEFAULT_GEOM_SLACK),
            hull_edges: live.hull_edges.max(DEFAULT_GEOM_SLACK),
            hull_topology: live.hull_topology.max(DEFAULT_GEOM_SLACK),
            mesh_vertices: live.mesh_vertices.max(DEFAULT_GEOM_SLACK),
            mesh_triangles: live.mesh_triangles.max(DEFAULT_GEOM_SLACK),
            mesh_nodes: live.mesh_nodes.max(DEFAULT_GEOM_SLACK),
            materials: live.materials.max(DEFAULT_GEOM_SLACK),
            joints: live.joints.max(DEFAULT_JOINT_CAPACITY),
        }
    }

    pub fn fits(self, live: Self) -> bool {
        self.bodies >= live.bodies
            && self.shapes >= live.shapes
            && self.hull_points >= live.hull_points
            && self.hull_planes >= live.hull_planes
            && self.hull_edges >= live.hull_edges
            && self.hull_topology >= live.hull_topology
            && self.mesh_vertices >= live.mesh_vertices
            && self.mesh_triangles >= live.mesh_triangles
            && self.mesh_nodes >= live.mesh_nodes
            && self.materials >= live.materials
            && self.joints >= live.joints
    }

    pub fn grow_to_fit(self, live: Self) -> Self {
        Self {
            bodies: grow_cap(self.bodies, live.bodies),
            shapes: grow_cap(self.shapes, live.shapes),
            hull_points: grow_cap(self.hull_points, live.hull_points),
            hull_planes: grow_cap(self.hull_planes, live.hull_planes),
            hull_edges: grow_cap(self.hull_edges, live.hull_edges),
            hull_topology: grow_cap(self.hull_topology, live.hull_topology),
            mesh_vertices: grow_cap(self.mesh_vertices, live.mesh_vertices),
            mesh_triangles: grow_cap(self.mesh_triangles, live.mesh_triangles),
            mesh_nodes: grow_cap(self.mesh_nodes, live.mesh_nodes),
            materials: grow_cap(self.materials, live.materials),
            joints: grow_cap(self.joints, live.joints),
        }
    }

    /// Host scene heap size after padding to these caps. `None` on overflow.
    pub fn scene_heap_bytes(self) -> Option<u64> {
        let bodies = u64::from(self.bodies.max(1));
        let add = |total: u64, count: u64, stride: u64| total.checked_add(count.checked_mul(stride)?);
        let mut n = add(0, bodies, core::mem::size_of::<BodyColdGpu>() as u64)?;
        n = add(n, bodies, 32)?;
        n = add(n, u64::from(self.shapes), core::mem::size_of::<ShapeGpu>() as u64)?;
        n = add(n, u64::from(self.hull_points), 16)?;
        n = add(n, u64::from(self.hull_planes), 16)?;
        n = add(n, u64::from(self.hull_edges), 16)?;
        n = add(n, u64::from(self.hull_topology), 16)?;
        n = add(n, u64::from(self.mesh_vertices), 16)?;
        n = add(n, u64::from(self.mesh_triangles), 16)?;
        n = add(n, u64::from(self.mesh_nodes).checked_mul(2)?, 16)?;
        n = add(n, u64::from(self.materials), core::mem::size_of::<SurfaceMaterialGpu>() as u64)?;
        add(n, u64::from(MIX_PAIR_CAP), core::mem::size_of::<MixPairGpu>() as u64)
    }

    pub fn validate_allocation(self, generation: u32) -> Result<u64, String> {
        let heap = self.scene_heap_bytes().ok_or_else(|| {
            format!(
                "scene heap size overflow generation={generation} bodies={} shapes={} materials={} joints={}",
                self.bodies, self.shapes, self.materials, self.joints
            )
        })?;
        eprintln!(
            "gpu-alloc scene generation={generation} bodies={} shapes={} hull_points={} hull_planes={} hull_edges={} hull_topology={} mesh_vertices={} mesh_triangles={} mesh_nodes={} materials={} joints={} scene_heap_bytes={heap} max={}",
            self.bodies,
            self.shapes,
            self.hull_points,
            self.hull_planes,
            self.hull_edges,
            self.hull_topology,
            self.mesh_vertices,
            self.mesh_triangles,
            self.mesh_nodes,
            self.materials,
            self.joints,
            MAX_SCENE_HEAP_BYTES
        );
        if heap > MAX_SCENE_HEAP_BYTES {
            return Err(format!(
                "refusing scene heap of {heap} bytes (limit {MAX_SCENE_HEAP_BYTES}); generation={generation} bodies={} shapes={} materials={}",
                self.bodies, self.shapes, self.materials
            ));
        }
        Ok(heap)
    }
}

pub fn grow_cap(old: u32, needed: u32) -> u32 {
    if needed <= old {
        old
    } else {
        needed.max(old.saturating_mul(2).max(1))
    }
}
pub const FIXED_DT: f32 = 1.0 / 60.0;
pub const DEFAULT_SUB_STEPS: i32 = 4;
pub const GRAVITY_Y: f32 = -10.0;
pub const GROUND_Y: f32 = 0.0;
pub const SPHERE_RADIUS: f32 = 0.18;
pub const DUMP_CHECKPOINTS: &[u32] = &[0, 1, 50, 100, 200, 300];
pub const KIND_SPHERE: u32 = 0;
pub const KIND_BOX: u32 = 1;
pub const KIND_CAPSULE: u32 = 2;
pub const KIND_CONVEX_HULL: u32 = 3;
pub const KIND_MESH: u32 = 4;
pub const FLAG_STATIC: u32 = 1;
pub const FLAG_HIDDEN: u32 = 2;
pub const FLAG_SLEEP: u32 = 4;
pub const FLAG_SLEEP_ENABLED: u32 = 8;
pub const FLAG_KINEMATIC: u32 = 16;
pub const FLAG_BULLET: u32 = 1 << 5;
pub const FLAG_ALLOW_FAST_ROTATION: u32 = 1 << 6;
pub const FLAG_DISABLED: u32 = 1 << 7;
pub const FLAG_DISABLE_CONTACT_RECYCLING: u32 = 1 << 14;
/// Previous completed step exceeded the continuous-motion threshold.
pub const FLAG_FAST: u32 = 1 << 15;

/// Matches WGSL `is_non_dynamic`: body type and disabled state, not mass.
pub fn gpu_is_non_dynamic(flags: u32) -> bool {
    flags & (FLAG_STATIC | FLAG_KINEMATIC | FLAG_DISABLED) != 0
}

#[derive(Clone, Copy, Debug, Default)]
pub struct TopologyBounds {
    pub shape_count: u32,
    pub mesh_shapes: u32,
    pub joints: u32,
    pub non_dynamic_shapes: u32,
    pub skip_general_static_sort: bool,
    pub static_degree_two_proof: bool,
    pub one_group_pair_ok: bool,
}
pub const FLAG_LOCK_LIN_X: u32 = 1 << 8;
pub const FLAG_LOCK_LIN_Y: u32 = 1 << 9;
pub const FLAG_LOCK_LIN_Z: u32 = 1 << 10;
pub const FLAG_LOCK_ANG_X: u32 = 1 << 11;
pub const FLAG_LOCK_ANG_Y: u32 = 1 << 12;
pub const FLAG_LOCK_ANG_Z: u32 = 1 << 13;
pub const SHAPE_ENABLE_CONTACT_EVENTS: u32 = 1;
pub const SHAPE_ENABLE_SENSOR_EVENTS: u32 = 1 << 1;
pub const SHAPE_IS_SENSOR: u32 = 1 << 2;
pub const SHAPE_ENABLE_HIT_EVENTS: u32 = 1 << 3;
pub const SHAPE_ENABLE_CUSTOM_FILTERING: u32 = 1 << 4;
pub const SHAPE_ENABLE_PRE_SOLVE_EVENTS: u32 = 1 << 5;
pub const SHAPE_DISABLE_SPECULATIVE: u32 = 1 << 6;
// GPU-only metadata: parent bounds are not collision/query geometry.
pub const SHAPE_PUBLIC_PROXY: u32 = 1 << 7;
pub const SHAPE_COMPOUND_CHILD: u32 = 1 << 8;
/// Per-point provenance in the existing lifecycle flags; no layout growth.
pub const CONTACT_PERSISTED_SHIFT: u32 = 16;
pub const CONTACT_PERSISTED_MASK: u32 = 0x000f0000;
pub const CONTACT_TOUCHING: u32 = 2;
pub const CONTACT_START_TOUCHING: u32 = 8;
pub const CONTACT_STOP_TOUCHING: u32 = 16;
pub const MAX_CONTACTS_PER_BODY: u32 = 64;
/// Box3D `B3_GRAPH_COLOR_COUNT` (last index is overflow).
pub const PARALLEL_COLORS: u32 = 23;
pub const OVERFLOW_COLOR: u32 = 23;
pub const MAX_COLORS: u32 = 24;
/// Box3D `B3_DYNAMIC_COLOR_COUNT` (static–dyn colors sit above this).
#[allow(dead_code)]
pub const DYNAMIC_COLOR_COUNT: u32 = 20;
pub const HASH_BUCKETS: u32 = 16384;
pub const MAX_INSERTS: u32 = 65536;
pub const PAIR_CAP: u32 = 65536;
/// Host/device scene heap ceiling. Requests above this are rejected, not clamped.
pub const MAX_SCENE_HEAP_BYTES: u64 = 512 * 1024 * 1024;
/// Packed graph keys store body IDs in 16 bits alongside unique indices.
pub const MAX_BODY_SLOTS: u32 = 65536;
/// Keep pair-key lookup below 50% load even when the dense contact pool is full.
pub const CONTACT_HASH_CAP: u32 = PAIR_CAP * 2;
pub const RADIX_GROUP_SIZE: u32 = 256;
pub const RADIX_BUCKETS: u32 = 256;
pub const RADIX_GROUPS: u32 = PAIR_CAP / RADIX_GROUP_SIZE;
/// Canonical body-pair table for `joint_disables_collision`. Power of two.
pub const JOINT_FILTER_CAP: u32 = 4096;
/// Bounded host-callback mix table. Not a cartesian Village-scale material grid.
pub const MIX_PAIR_CAP: u32 = 4096;
pub const GPU_MAX_COMPOUND_CHILDREN: u32 = 4096;
pub const JOINT_FILTER_PROBE: u32 = 32;
pub const SCRATCH_U32: u32 = 64
    + HASH_BUCKETS
    + MAX_INSERTS * 3
    + PAIR_CAP
    + 6 * PAIR_CAP
    + RADIX_GROUPS * RADIX_BUCKETS
    + RADIX_BUCKETS;

pub const SCR_UNIQUE_N: u32 = 2;
pub const SCR_POW2: u32 = 4;
pub const SCR_OCCUPIED_N: u32 = 5;
pub const SCR_COLOR: u32 = 16;
pub const SCR_HASH: u32 = 64;
pub const SCR_PAIRS: u32 = SCR_HASH + HASH_BUCKETS + MAX_INSERTS * 3;
pub const SCR_OCCUPIED_CONTACT: u32 = SCR_PAIRS + 3 * PAIR_CAP;
pub const SCR_NEXT_OCCUPIED: u32 = SCR_OCCUPIED_CONTACT + PAIR_CAP;
pub const SCR_PREVIOUS_TOUCHING: u32 = SCR_NEXT_OCCUPIED + PAIR_CAP;
pub const SCR_RADIX_OUT: u32 = SCR_PREVIOUS_TOUCHING + PAIR_CAP;
pub const SCR_RADIX_HIST: u32 = SCR_RADIX_OUT + PAIR_CAP;
pub const SCR_RADIX_BASE: u32 = SCR_RADIX_HIST + RADIX_GROUPS * RADIX_BUCKETS;
pub const ATOM_PAIR_N: u32 = 0;
pub const ATOM_PAIR_DROPPED: u32 = 2;
pub const ATOM_INSERT_DROPPED: u32 = 3;
pub const ATOM_CONTACT_DROPPED: u32 = 4;
pub const ATOM_HASH_HOP_DROPPED: u32 = 5;
pub const ATOM_STATIC_N: u32 = 7;
pub const ATOM_STICKY_PAIR_DROPPED: u32 = 8;
pub const ATOM_STICKY_INSERT_DROPPED: u32 = 9;
pub const ATOM_STICKY_CONTACT_DROPPED: u32 = 10;
pub const ATOM_STICKY_HASH_HOP_DROPPED: u32 = 11;
pub const ATOM_STICKY_FIRST_STEP: u32 = 12;
pub const ATOM_GRAPH_PROOF_FAIL: u32 = 13;
pub const ATOM_JOINT_LIST_DROPPED: u32 = 14;
pub const ATOM_MANIFOLD_ALLOC: u32 = 15;

pub fn pair_hash_mix(key: u32) -> u32 {
    let mut x = key;
    x ^= x >> 16;
    x = x.wrapping_mul(0x7feb_352d);
    x ^= x >> 15;
    x = x.wrapping_mul(0x846c_a68b);
    x ^ (x >> 16)
}
/// Statics that occupy more cells than this stay on the compact fat list
/// instead of filling the spatial hash (grounds, large walls).
#[allow(dead_code)]
pub const MAX_STATIC_HASH_CELLS: u32 = 8;

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct BroadphaseStats {
    pub pair_candidates_dropped: u32,
    pub cell_inserts_dropped: u32,
    pub contact_pairs_dropped: u32,
    pub hash_traversals_dropped: u32,
}

impl BroadphaseStats {
    pub fn has_loss(&self) -> bool {
        self.pair_candidates_dropped
            | self.cell_inserts_dropped
            | self.contact_pairs_dropped
            | self.hash_traversals_dropped
            != 0
    }

    pub fn loss_detail(&self) -> String {
        format!(
            "pairs={} inserts={} contacts={} hash={}",
            self.pair_candidates_dropped,
            self.cell_inserts_dropped,
            self.contact_pairs_dropped,
            self.hash_traversals_dropped
        )
    }

    pub fn from_atom_words(words: &[u32]) -> Self {
        Self {
            pair_candidates_dropped: words.get(ATOM_PAIR_DROPPED as usize).copied().unwrap_or(0),
            cell_inserts_dropped: words
                .get(ATOM_INSERT_DROPPED as usize)
                .copied()
                .unwrap_or(0),
            contact_pairs_dropped: words
                .get(ATOM_CONTACT_DROPPED as usize)
                .copied()
                .unwrap_or(0),
            hash_traversals_dropped: words
                .get(ATOM_HASH_HOP_DROPPED as usize)
                .copied()
                .unwrap_or(0),
        }
    }

    pub fn sticky_from_atom_words(words: &[u32]) -> Self {
        Self {
            pair_candidates_dropped: words
                .get(ATOM_STICKY_PAIR_DROPPED as usize)
                .copied()
                .unwrap_or(0),
            cell_inserts_dropped: words
                .get(ATOM_STICKY_INSERT_DROPPED as usize)
                .copied()
                .unwrap_or(0),
            contact_pairs_dropped: words
                .get(ATOM_STICKY_CONTACT_DROPPED as usize)
                .copied()
                .unwrap_or(0),
            hash_traversals_dropped: words
                .get(ATOM_STICKY_HASH_HOP_DROPPED as usize)
                .copied()
                .unwrap_or(0),
        }
    }
}

/// Compact joint lists: heads, unique roots, offsets, counts, then joint ids.
pub fn joint_list_words(body_capacity: u32, joint_capacity: u32) -> u32 {
    let bodies = body_capacity.max(64);
    4 * bodies + joint_capacity.max(bodies)
}

pub fn scratch_u32_count(body_capacity: u32) -> usize {
    scratch_u32_count_with_joints(body_capacity, body_capacity)
}

pub fn scratch_u32_count_with_joints(body_capacity: u32, joint_capacity: u32) -> usize {
    let capacity = body_capacity.max(64);
    // Six island work arrays, occupancy, fused flags, eight fused slots per
    // body, 24 phase summaries, four diagnostic words per body for each of the
    // 24 captured phases, and compact per-color contact indices.
    (SCRATCH_U32
        + 16 * capacity
        + 24 * 8
        + 24 * 4 * capacity
        + MAX_COLORS * contact_capacity(body_capacity)
        + JOINT_FILTER_CAP * 2
        + joint_list_words(body_capacity, joint_capacity)) as usize
}

pub fn joint_filter_word_offset(body_capacity: u32) -> u32 {
    joint_filter_word_offset_with_joints(body_capacity, body_capacity)
}

pub fn joint_filter_word_offset_with_joints(body_capacity: u32, joint_capacity: u32) -> u32 {
    scratch_u32_count_with_joints(body_capacity, joint_capacity) as u32
        - JOINT_FILTER_CAP * 2
        - joint_list_words(body_capacity, joint_capacity)
}

/// Compact joint component lists live after the filter table: heads, unique
/// roots, per-component offsets/counts, then joint indices in original order.
pub fn joint_head_word(body_capacity: u32) -> u32 {
    joint_filter_word_offset(body_capacity) + JOINT_FILTER_CAP * 2
}

/// Shader `joint_head_base()` for a live `params.body_count`.
pub fn joint_head_live(body_count: u32, contact_capacity: u32) -> u32 {
    SCRATCH_U32
        + 16 * body_count
        + 24 * 8
        + 24 * 4 * body_count
        + MAX_COLORS * contact_capacity
        + JOINT_FILTER_CAP * 2
}

/// Exact lookup table for joints that disable collision. Overflow means the
/// shader must keep the linear scan as a correct fallback.
pub fn build_joint_filter_table(joints: &[JointGpu]) -> (Vec<u32>, bool) {
    let cap = JOINT_FILTER_CAP as usize;
    let mut table = vec![u32::MAX; cap * 2];
    let mut overflow = false;
    for joint in joints {
        if joint.kind == JOINT_NONE {
            continue;
        }
        let a = joint.a.min(joint.b);
        let b = joint.a.max(joint.b);
        let packed = (a << 16) | (b & 0xffff);
        let disable = u32::from(
            joint.flags & JOINT_COLLIDE_CONNECTED == 0,
        );
        let mut h = pair_hash_mix(pair_hash_mix(a) ^ b);
        let mut placed = false;
        for _ in 0..JOINT_FILTER_PROBE {
            let idx = (h as usize) & (cap - 1);
            let stored = table[idx * 2];
            if stored == u32::MAX {
                table[idx * 2] = packed;
                table[idx * 2 + 1] = disable;
                placed = true;
                break;
            }
            if stored == packed {
                table[idx * 2 + 1] |= disable;
                placed = true;
                break;
            }
            h = h.wrapping_add(1);
        }
        overflow |= !placed;
    }
    (table, overflow)
}

pub fn contact_capacity(body_capacity: u32) -> u32 {
    body_capacity
        .max(1)
        .saturating_mul(16)
        .max(256)
        .next_power_of_two()
        .min(PAIR_CAP)
}

/// Pass LUT rows: relax=0, bias=1, warm=2, restitution=3.
pub const PASS_BIAS_ROWS: u32 = 4;
pub const DEFAULT_CELL_SIZE: f32 = 2.5;
/// Box3D `B3_LINEAR_SLOP` at 1 m units.
pub const LINEAR_SLOP: f32 = 0.005;
/// Box3D `B3_SPECULATIVE_DISTANCE`.
pub const SPECULATIVE_DISTANCE: f32 = 0.02;
pub const DIAG_DISABLE_RECYCLING: u32 = 1 << 0;
pub const DIAG_DISABLE_SAT_CACHE: u32 = 1 << 1;
pub const DIAG_DISABLE_ROLLING: u32 = 1 << 2;
pub const DIAG_REBUILD_GRAPH: u32 = 1 << 3;
pub const DIAG_DISABLE_SLEEP: u32 = 1 << 4;
pub const DIAG_PHASE_CAPTURE: u32 = 1 << 5;
pub const DIAG_FORCE_CAPACITY_LOSS: u32 = 1 << 6;
pub const DIAG_GENERAL_SOLVER: u32 = 1 << 7;
pub const DIAG_FORCE_GENERAL_STATIC_SORT: u32 = 1 << 8;
pub const DIAG_RECOMPUTE_TOPOLOGY: u32 = 1 << 9;
pub const DIAG_STATIC_DEGREE_ONE_PROOF: u32 = 1 << 10;
pub const DIAG_JOINT_FILTER_SCAN: u32 = 1 << 11;
pub const DIAG_JOINT_FILTER_OVERFLOW: u32 = 1 << 12;
pub const DIAG_SERIAL_JOINTS: u32 = 1 << 13;
pub const DIAG_PARALLEL_JOINTS: u32 = 1 << 14;
pub const DIAG_MESH_CANDIDATES: u32 = 1 << 15;
pub const DIAG_STATIC_DEGREE_TWO_PROOF: u32 = 1 << 16;
pub const DIAG_BOUNDED_STATIC_SORT: u32 = 1 << 17;
pub const MESH_TRACE_WORDS: u32 = 1 + 2048 * 16;
/// Experimental static-only fused TGS. Off until differential tests pass.
/// Last attempt: local pass mode was ignored because `solve_manifold` read
/// uniform `params.use_bias`, so relax re-applied bias and skipped friction.
pub const ENABLE_FUSED_ISLANDS: bool = false;
pub const JOINT_NONE: u32 = 0;
pub const JOINT_REVOLUTE: u32 = 1;
pub const JOINT_WELD: u32 = 2;
pub const JOINT_SPHERICAL: u32 = 3;
pub const JOINT_PRISMATIC: u32 = 4;
pub const JOINT_FILTER: u32 = 5;
pub const JOINT_DISTANCE: u32 = 6;
pub const JOINT_PARALLEL: u32 = 7;
pub const JOINT_MOTOR: u32 = 8;
pub const JOINT_WHEEL: u32 = 9;
pub const REVOLUTE_ENABLE_SPRING: u32 = 1 << 0;
pub const REVOLUTE_ENABLE_LIMIT: u32 = 1 << 1;
pub const REVOLUTE_ENABLE_MOTOR: u32 = 1 << 2;
pub const SPHERICAL_ENABLE_SPRING: u32 = 1 << 0;
pub const SPHERICAL_ENABLE_CONE_LIMIT: u32 = 1 << 1;
pub const SPHERICAL_ENABLE_TWIST_LIMIT: u32 = 1 << 2;
pub const SPHERICAL_ENABLE_MOTOR: u32 = 1 << 3;
pub const PRISMATIC_ENABLE_SPRING: u32 = 1 << 0;
pub const PRISMATIC_ENABLE_LIMIT: u32 = 1 << 1;
pub const PRISMATIC_ENABLE_MOTOR: u32 = 1 << 2;
pub const DISTANCE_ENABLE_SPRING: u32 = 1 << 0;
pub const DISTANCE_ENABLE_LIMIT: u32 = 1 << 1;
pub const DISTANCE_ENABLE_MOTOR: u32 = 1 << 2;
pub const WHEEL_ENABLE_SUSPENSION_SPRING: u32 = 1 << 0;
pub const WHEEL_ENABLE_SUSPENSION_LIMIT: u32 = 1 << 1;
pub const WHEEL_ENABLE_SPIN_MOTOR: u32 = 1 << 2;
pub const WHEEL_ENABLE_STEERING: u32 = 1 << 3;
pub const WHEEL_ENABLE_STEERING_LIMIT: u32 = 1 << 4;
pub const JOINT_COLLIDE_CONNECTED: u32 = 1 << 31;

#[repr(C)]
#[derive(Clone, Copy, Debug, Pod, Zeroable)]
pub struct BodyGpu {
    pub pos: [f32; 3],
    pub inv_mass: f32,
    pub vel: [f32; 3],
    pub kind: u32,
    pub half: [f32; 3],
    pub flags: u32,
    pub rot: [f32; 4],
    pub omega: [f32; 3],
    pub restitution: f32,
    pub inv_inertia: [f32; 3],
    pub friction: f32,
    pub gravity_scale: f32,
    pub linear_damping: f32,
    pub angular_damping: f32,
    pub rolling: f32,
    /// TGS position delta for this world step (Box3D `deltaPosition`).
    pub dp: [f32; 3],
    /// Time continuously below the sleep threshold.
    pub sleep_time: f32,
    /// TGS rotation delta (`deltaRotation`), identity at step start.
    pub dq: [f32; 4],
    /// Stable minimum-body label of the current contact/joint island.
    pub island_id: u32,
    /// Box3D-style farthest-point sleep velocity diagnostic.
    pub sleep_velocity: f32,
    pub _pad_island: [u32; 2],
}

/// Mutable body state used by the simulation hot path.
///
/// Shape, mass-property, and material data lives in `BodyColdGpu`, so solver
/// passes update 96 bytes in place instead of ping-ponging the 160-byte export
/// record.
#[repr(C)]
#[derive(Clone, Copy, Debug, Pod, Zeroable)]
pub struct BodyStateGpu {
    pub pos: [f32; 3],
    pub inv_mass: f32,
    pub vel: [f32; 3],
    pub flags: u32,
    pub rot: [f32; 4],
    pub omega: [f32; 3],
    pub sleep_velocity: f32,
    pub dp: [f32; 3],
    pub sleep_time: f32,
    pub dq: [f32; 4],
}

/// Immutable body data, uploaded once when the simulation is rebuilt.
#[repr(C)]
#[derive(Clone, Copy, Debug, Pod, Zeroable)]
pub struct BodyColdGpu {
    pub half: [f32; 3],
    pub kind: u32,
    pub inv_inertia: [f32; 3],
    pub restitution: f32,
    pub gravity_scale: f32,
    pub linear_damping: f32,
    pub angular_damping: f32,
    pub friction: f32,
    pub rolling: f32,
    pub local_center: [f32; 3],
}

impl BodyStateGpu {
    pub fn from_body(body: &BodyGpu) -> Self {
        Self {
            pos: body.pos,
            inv_mass: body.inv_mass,
            vel: body.vel,
            flags: body.flags,
            rot: body.rot,
            omega: body.omega,
            sleep_velocity: body.sleep_velocity,
            dp: body.dp,
            sleep_time: body.sleep_time,
            dq: body.dq,
        }
    }
}

impl BodyColdGpu {
    pub fn from_body(body: &BodyGpu, local_center: [f32; 3]) -> Self {
        Self {
            half: body.half,
            kind: body.kind,
            inv_inertia: body.inv_inertia,
            restitution: body.restitution,
            gravity_scale: body.gravity_scale,
            linear_damping: body.linear_damping,
            angular_damping: body.angular_damping,
            friction: body.friction,
            rolling: body.rolling,
            local_center,
        }
    }
}

/// Immutable collider metadata. Hull topology is stored in the tail of the
/// same GPU scene buffer as `BodyColdGpu` to stay within WebGPU's guaranteed
/// eight-storage-buffer limit.
#[repr(C)]
#[derive(Clone, Copy, Debug, Pod, Zeroable)]
pub struct ShapeGpu {
    pub body_index: u32,
    pub plane_slot: u32,
    pub kind: u32,
    /// Packed hull point/plane/edge/half-edge counts.
    pub topology_counts: u32,
    pub local_center: [f32; 3],
    pub topology_slot: u32,
    pub half: [f32; 3],
    pub rolling: f32,
    pub axis: [f32; 3],
    pub friction: f32,
    pub restitution: f32,
    pub inner_radius: f32,
    pub hull_slot: u32,
    pub edge_slot: u32,
    pub category_bits_lo: u32,
    pub category_bits_hi: u32,
    pub mask_bits_lo: u32,
    pub mask_bits_hi: u32,
    pub group_index: i32,
    pub event_flags: u32,
    pub material_slot: u32,
    pub material_count: u32,
    pub tangent_velocity: [f32; 3],
    pub custom_color: u32,
    pub user_material_id_lo: u32,
    pub user_material_id_hi: u32,
    pub _pad_filter: [u32; 2],
    pub instance_position: [f32; 3],
    pub instance_flags: u32,
    pub instance_rotation: [f32; 4],
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Pod, Zeroable)]
pub struct SurfaceMaterialGpu {
    pub friction: f32,
    pub restitution: f32,
    pub rolling_resistance: f32,
    pub _pad0: f32,
    pub tangent_velocity: [f32; 3],
    pub custom_color: u32,
    pub user_material_id_lo: u32,
    pub user_material_id_hi: u32,
    pub _pad1: [u32; 2],
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Pod, Zeroable)]
pub struct MixPairGpu {
    pub key: u32,
    pub friction: f32,
    pub restitution: f32,
    pub occupied: u32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Pod, Zeroable)]
pub struct ContactGpu {
    pub a: u32,
    pub b: u32,
    pub color: u32,
    pub count: u32,
    pub nx: f32,
    pub ny: f32,
    pub nz: f32,
    pub friction: f32,
    /// World-space rA at collide (`p - posA`) + `baseSeparation` in w.
    pub ra0: [f32; 4],
    pub ra1: [f32; 4],
    pub ra2: [f32; 4],
    pub ra3: [f32; 4],
    /// World-space rB at collide + accumulated normal impulse in w.
    pub rb0: [f32; 4],
    pub rb1: [f32; 4],
    pub rb2: [f32; 4],
    pub rb3: [f32; 4],
    pub friction_impulse: [f32; 2],
    pub twist_impulse: f32,
    pub rolling: f32,
    pub center_a: [f32; 3],
    pub _pad_ca: f32,
    pub center_b: [f32; 3],
    pub _pad_cb: f32,
    pub rolling_impulse: [f32; 3],
    pub _pad_end: f32,
    pub tangent_velocity: [f32; 3],
    pub material_index: u32,
    pub _tail: [u32; 8],
    /// Relative center displacement and validity marker used by contact recycling.
    pub cached_relative: [f32; 4],
    pub cached_rotation_a: [f32; 4],
    pub cached_rotation_b: [f32; 4],
    /// Generation, lifecycle flags, persistent per-color local index, packed pair key.
    pub lifecycle: [u32; 4],
    pub prepared_normal_mass: [f32; 4],
    pub prepared_lever_arm: [f32; 4],
    pub total_normal_impulse: [f32; 4],
    /// Inverse central-friction 2x2 matrix: m11, m12, m21, m22.
    pub prepared_tangent_inv: [f32; 4],
    /// Bias rate, mass scale, impulse scale, static-pair marker.
    pub prepared_softness: [f32; 4],
    pub persistent_ra0: [f32; 4],
    pub persistent_ra1: [f32; 4],
    pub persistent_ra2: [f32; 4],
    pub persistent_ra3: [f32; 4],
    pub persistent_rb0: [f32; 4],
    pub persistent_rb1: [f32; 4],
    pub persistent_rb2: [f32; 4],
    pub persistent_rb3: [f32; 4],
    /// Next slot + 1, owning root slot + 1 (zero for roots), root chain length, representative mesh triangle + 1.
    /// Zeroed metadata is the existing one-manifold representation.
    pub manifold_link: [u32; 4],
    /// Per-point mesh triangle index + 1; zero denotes a non-mesh witness.
    pub point_triangles: [u32; 4],
}

impl ContactGpu {
    pub fn point_persisted(&self, point: usize) -> bool {
        point < self.count.min(4) as usize
            && self.lifecycle[1] & (1u32 << (CONTACT_PERSISTED_SHIFT + point as u32)) != 0
    }
}

/// Constraint data touched by every solver pass.
#[repr(C)]
#[derive(Clone, Copy, Debug, Pod, Zeroable)]
pub struct ContactHotGpu {
    pub a: u32,
    pub b: u32,
    pub color: u32,
    pub count: u32,
    pub n: [f32; 3],
    pub friction: f32,
    pub ra: [[f32; 4]; 4],
    pub rb: [[f32; 4]; 4],
    pub friction_impulse: [f32; 2],
    pub twist_impulse: f32,
    pub rolling: f32,
    pub center_a: [f32; 3],
    pub _pad_ca: f32,
    pub center_b: [f32; 3],
    pub _pad_cb: f32,
    pub rolling_impulse: [f32; 3],
    pub restitution: f32,
    pub tangent_velocity: [f32; 3],
    pub material_index: u32,
    /// Next slot + 1, owning root slot + 1 (zero for roots), root chain length, representative mesh triangle + 1.
    /// Zeroed metadata is the existing one-manifold representation.
    pub manifold_link: [u32; 4],
}

/// Values prepared once per world step and consumed by solver passes.
#[repr(C)]
#[derive(Clone, Copy, Debug, Pod, Zeroable)]
pub struct ContactPreparedGpu {
    pub relative_velocity: [u32; 4],
    pub normal_mass: [f32; 4],
    pub lever_arm: [f32; 4],
    pub total_normal_impulse: [f32; 4],
    pub tangent_inv: [f32; 4],
    pub softness: [f32; 4],
}

/// Narrowphase recycling, feature matching, and persistent graph identity.
#[repr(C)]
#[derive(Clone, Copy, Debug, Pod, Zeroable)]
pub struct ContactPersistentGpu {
    pub feature_ids: [u32; 4],
    pub cached_relative: [f32; 4],
    pub cached_rotation_a: [f32; 4],
    pub cached_rotation_b: [f32; 4],
    pub lifecycle: [u32; 4],
    pub persistent_ra: [[f32; 4]; 4],
    pub persistent_rb: [[f32; 4]; 4],
    pub point_triangles: [u32; 4],
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, Pod, Zeroable)]
pub struct JointGpu {
    pub a: u32,
    pub b: u32,
    pub kind: u32,
    pub _pad0: u32,
    pub anchor_a: [f32; 3],
    pub hertz: f32,
    pub anchor_b: [f32; 3],
    pub damping: f32,
    pub axis: [f32; 3],
    pub impulse: f32,
    pub frame_a_rotation: [f32; 4],
    pub frame_b_rotation: [f32; 4],
    pub perp_impulse: [f32; 2],
    pub flags: u32,
    pub _pad1: u32,
    pub angular_impulse: [f32; 3],
    pub spring_impulse: f32,
    pub motor_impulse: f32,
    pub lower_impulse: f32,
    pub upper_impulse: f32,
    pub spring_hertz: f32,
    pub spring_damping: f32,
    pub target_translation: f32,
    pub lower_translation: f32,
    pub upper_translation: f32,
    pub max_motor_force: f32,
    pub motor_speed: f32,
    pub _pad2: [f32; 2],
    pub motor_angular_velocity: [f32; 3],
    pub _pad3: f32,
    pub target_rotation: [f32; 4],
    pub spring_angular_impulse: [f32; 3],
    pub swing_impulse: f32,
    pub motor_angular_impulse: [f32; 3],
    pub _pad4: f32,
    pub weld_linear_hertz: f32,
    pub weld_linear_damping: f32,
    pub weld_angular_hertz: f32,
    pub weld_angular_damping: f32,
    pub weld_linear_impulse: [f32; 3],
    pub _pad_weld_linear: f32,
    pub weld_angular_impulse: [f32; 3],
    pub _pad_weld_angular: f32,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum DemoScene {
    Spheres,
    Stack,
    SingleBox,
    BoxStack,
    SphereStack,
    CapsuleStack,
    Revolute,
    Weld,
    AnchoredMechanisms,
    JointChain,
    Pyramid,
    Bounce,
    Mixed,
    Spinner,
    Ramp,
    Dominoes,
    HighResistance,
    MixedStacks,
}

impl DemoScene {
    pub const ALL: [DemoScene; 17] = [
        DemoScene::SingleBox,
        DemoScene::BoxStack,
        DemoScene::SphereStack,
        DemoScene::CapsuleStack,
        DemoScene::HighResistance,
        DemoScene::MixedStacks,
        DemoScene::Revolute,
        DemoScene::Weld,
        DemoScene::AnchoredMechanisms,
        DemoScene::JointChain,
        DemoScene::Stack,
        DemoScene::Pyramid,
        DemoScene::Bounce,
        DemoScene::Mixed,
        DemoScene::Spinner,
        DemoScene::Ramp,
        DemoScene::Spheres,
    ];

    pub fn slug(self) -> &'static str {
        match self {
            DemoScene::Spheres => "spheres",
            DemoScene::Stack => "stack",
            DemoScene::SingleBox => "single-box",
            DemoScene::BoxStack => "box-stack",
            DemoScene::SphereStack => "sphere-stack",
            DemoScene::CapsuleStack => "capsule-stack",
            DemoScene::Revolute => "revolute",
            DemoScene::Weld => "weld",
            DemoScene::AnchoredMechanisms => "anchored-mechanisms",
            DemoScene::JointChain => "joint-chain",
            DemoScene::Pyramid => "pyramid",
            DemoScene::Bounce => "bounce",
            DemoScene::Mixed => "mixed",
            DemoScene::Spinner => "spinner",
            DemoScene::Ramp => "ramp",
            DemoScene::Dominoes => "dominoes",
            DemoScene::HighResistance => "high-resistance",
            DemoScene::MixedStacks => "mixed-stacks",
        }
    }

    fn index(self) -> usize {
        Self::ALL.iter().position(|s| *s == self).unwrap_or(0)
    }

    pub fn next(self) -> DemoScene {
        let i = self.index();
        Self::ALL[(i + 1) % Self::ALL.len()]
    }

    pub fn prev(self) -> DemoScene {
        let i = self.index();
        Self::ALL[(i + Self::ALL.len() - 1) % Self::ALL.len()]
    }
}

#[derive(Clone, Copy, Debug)]
pub struct DemoConfig {
    pub body_count: u32,
    pub body_count_explicit: bool,
    pub contacts: bool,
    pub scene: DemoScene,
    pub jacobi: bool,
}

pub fn scene_scale_count(scene: DemoScene, body_count: u32, explicit: bool) -> u32 {
    if explicit {
        return match scene {
            DemoScene::MixedStacks => body_count.max(2),
            _ => body_count.max(1),
        };
    }
    match scene {
        DemoScene::MixedStacks => 600,
        DemoScene::Dominoes => 30,
        _ => body_count.max(1),
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Pod, Zeroable)]
pub struct SimParams {
    pub dt: f32,
    pub gravity_x: f32,
    pub gravity_y: f32,
    pub gravity_z: f32,
    pub color_select: u32,
    pub enable_contacts: u32,
    pub max_contacts: u32,
    pub body_count: u32,
    pub cell_size: f32,
    pub bias_rate: f32,
    pub mass_scale: f32,
    pub impulse_scale: f32,
    pub contact_speed: f32,
    pub sleep_threshold: f32,
    pub use_bias: u32,
    pub joint_count: u32,
    pub solver_mode: u32,
    pub enable_sleep: u32,
    pub step_dt: f32,
    pub diagnostic_flags: u32,
    pub contact_hertz: f32,
    pub contact_damping: f32,
    pub contact_capacity: u32,
    pub shape_count: u32,
    pub shape_base_u32: u32,
    pub hull_point_count: u32,
    pub hull_base_u32: u32,
    pub hull_plane_count: u32,
    pub hull_plane_base_u32: u32,
    pub hull_edge_count: u32,
    pub hull_edge_base_u32: u32,
    pub hull_topology_count: u32,
    pub hull_topology_base_u32: u32,
    pub mesh_vertex_count: u32,
    pub mesh_vertex_base_u32: u32,
    pub mesh_triangle_count: u32,
    pub mesh_triangle_base_u32: u32,
    pub mesh_node_count: u32,
    pub mesh_node_base_u32: u32,
    pub surface_material_count: u32,
    pub surface_material_base_u32: u32,
    pub sub_step_count: u32,
    pub physics_step: u32,
    pub mix_pair_count: u32,
    pub mix_pair_base_u32: u32,
    pub enable_continuous: u32,
    pub contact_recycle_distance: f32,
    pub fat_bounds_base: u32,
    pub fat_bounds_epoch: u32,
    pub fat_commands_base: u32,
    pub fat_commands_epoch: u32,
    pub fat_commands_count: u32,
    pub remap_old_count: u32,
    pub remap_capture: u32,
    pub remap_history_step: u32,
    pub maximum_linear_speed: f32,
    pub restitution_threshold: f32,
    pub _pad_block: [u32; 7],
}

impl SimParams {
    pub fn new(count: u32, enable_contacts: bool, dt: f32, gravity: [f32; 3]) -> Self {
        let (bias_rate, mass_scale, impulse_scale) = make_soft(30.0, 10.0, dt);
        Self {
            dt,
            gravity_x: gravity[0],
            gravity_y: gravity[1],
            gravity_z: gravity[2],
            color_select: 0,
            enable_contacts: u32::from(enable_contacts),
            max_contacts: MAX_CONTACTS_PER_BODY,
            body_count: count,
            cell_size: DEFAULT_CELL_SIZE,
            bias_rate,
            mass_scale,
            impulse_scale,
            contact_speed: 3.0,
            sleep_threshold: 0.05,
            use_bias: 1,
            joint_count: 0,
            solver_mode: 0,
            enable_sleep: 1,
            step_dt: dt,
            diagnostic_flags: 0,
            contact_hertz: 30.0,
            contact_damping: 10.0,
            contact_capacity: contact_capacity(count),
            shape_count: 0,
            shape_base_u32: 0,
            hull_point_count: 0,
            hull_base_u32: 0,
            hull_plane_count: 0,
            hull_plane_base_u32: 0,
            hull_edge_count: 0,
            hull_edge_base_u32: 0,
            hull_topology_count: 0,
            hull_topology_base_u32: 0,
            mesh_vertex_count: 0,
            mesh_vertex_base_u32: 0,
            mesh_triangle_count: 0,
            mesh_triangle_base_u32: 0,
            mesh_node_count: 0,
            mesh_node_base_u32: 0,
            surface_material_count: 0,
            surface_material_base_u32: 0,
            sub_step_count: 4,
            physics_step: 0,
            mix_pair_count: 0,
            mix_pair_base_u32: 0,
            enable_continuous: 1,
            contact_recycle_distance: 0.05,
            fat_bounds_base: 0,
            fat_bounds_epoch: 1,
            fat_commands_base: 0,
            fat_commands_epoch: 0,
            fat_commands_count: 0,
            remap_old_count: 0,
            remap_capture: 0,
            remap_history_step: 0,
            maximum_linear_speed: 400.0,
            restitution_threshold: 1.0,
            _pad_block: [0; 7],
        }
    }
}

/// Box3D `b3MakeSoft` (contact Hertz / zeta / h).
pub fn make_soft(hertz: f32, zeta: f32, h: f32) -> (f32, f32, f32) {
    if hertz == 0.0 {
        return (0.0, 0.0, 0.0);
    }
    let omega = 2.0 * std::f32::consts::PI * hertz;
    let a1 = 2.0 * zeta + h * omega;
    let a2 = h * omega * a1;
    let a3 = 1.0 / (1.0 + a2);
    let bias_rate = omega / a1;
    let mass_scale = a2 * a3;
    let impulse_scale = a3;
    (bias_rate, mass_scale, impulse_scale)
}

const _: () = assert!(core::mem::size_of::<SimParams>() == 256);
const _: () = assert!(core::mem::offset_of!(SimParams, maximum_linear_speed) == 220);
const _: () = assert!(core::mem::offset_of!(SimParams, restitution_threshold) == 224);
const _: () = assert!(core::mem::size_of::<BodyGpu>() == 160);
const _: () = assert!(core::mem::size_of::<BodyStateGpu>() == 96);
const _: () = assert!(core::mem::size_of::<BodyColdGpu>() == 64);
const _: () = assert!(core::mem::offset_of!(ShapeGpu, instance_position) == 144);
const _: () = assert!(core::mem::offset_of!(ShapeGpu, instance_flags) == 156);
const _: () = assert!(core::mem::offset_of!(ShapeGpu, instance_rotation) == 160);
const _: () = assert!(core::mem::size_of::<ShapeGpu>() == 176);
const _: () = assert!(core::mem::offset_of!(ShapeGpu, category_bits_lo) == 80);
const _: () = assert!(core::mem::offset_of!(ShapeGpu, mask_bits_lo) == 88);
const _: () = assert!(core::mem::offset_of!(ShapeGpu, group_index) == 96);
const _: () = assert!(core::mem::offset_of!(ShapeGpu, event_flags) == 100);
const _: () = assert!(core::mem::size_of::<ContactGpu>() == 576);
const _: () = assert!(core::mem::size_of::<ContactHotGpu>() == 256);
const _: () = assert!(core::mem::size_of::<ContactPreparedGpu>() == 96);
const _: () = assert!(core::mem::size_of::<ContactPersistentGpu>() == 224);
const _: () = assert!(core::mem::size_of::<JointGpu>() == 288);

pub fn sphere_mass(radius: f32, density: f32) -> f32 {
    (4.0 / 3.0) * std::f32::consts::PI * radius * radius * radius * density
}

pub fn box_mass(half: [f32; 3], density: f32) -> f32 {
    8.0 * half[0] * half[1] * half[2] * density
}

/// Central mass properties for a capsule, matching `b3ComputeCapsuleMass`.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct CapsuleMass {
    pub mass: f32,
    pub center: [f32; 3],
    /// Symmetric tensor `xx, yy, zz, xy, xz, yz` about the midpoint.
    pub inertia: [f32; 6],
}

/// Capsule mass and central inertia for arbitrary endpoints.
///
/// `h = |c2-c1|`. A zero-length capsule is the sphere limit and does not
/// normalize a zero axis. Density and radius follow the caller; invalid
/// non-finite inputs yield zero mass.
pub fn compute_capsule_mass(
    center1: [f32; 3],
    center2: [f32; 3],
    radius: f32,
    density: f32,
) -> CapsuleMass {
    let dx = center2[0] - center1[0];
    let dy = center2[1] - center1[1];
    let dz = center2[2] - center1[2];
    let h2 = dx * dx + dy * dy + dz * dz;
    let h = h2.sqrt();
    let r = radius;
    let pi = std::f32::consts::PI;
    let mc = density * pi * r * r * h;
    let ms = density * (4.0 / 3.0) * pi * r * r * r;
    let mass = mc + ms;
    let i_par = 0.5 * mc * r * r + 0.4 * ms * r * r;
    let i_perp = mc * (3.0 * r * r + h * h) / 12.0
        + 0.4 * ms * r * r
        + 0.125 * ms * (3.0 * r + 2.0 * h) * h;
    let center = [
        0.5 * (center1[0] + center2[0]),
        0.5 * (center1[1] + center2[1]),
        0.5 * (center1[2] + center2[2]),
    ];
    let inertia = if h2 > 1000.0 * f32::MIN_POSITIVE {
        let inv = 1.0 / h;
        let u = [dx * inv, dy * inv, dz * inv];
        let d = i_par - i_perp;
        [
            i_perp + d * u[0] * u[0],
            i_perp + d * u[1] * u[1],
            i_perp + d * u[2] * u[2],
            d * u[0] * u[1],
            d * u[0] * u[2],
            d * u[1] * u[2],
        ]
    } else {
        [i_par, i_par, i_par, 0.0, 0.0, 0.0]
    };
    CapsuleMass {
        mass,
        center,
        inertia,
    }
}

pub fn capsule_mass(radius: f32, half_len: f32, density: f32) -> f32 {
    compute_capsule_mass(
        [0.0, -half_len, 0.0],
        [0.0, half_len, 0.0],
        radius,
        density,
    )
    .mass
}

pub fn box_inv_inertia(mass: f32, half: [f32; 3]) -> [f32; 3] {
    let hx2 = half[0] * half[0];
    let hy2 = half[1] * half[1];
    let hz2 = half[2] * half[2];
    let ixx = mass / 3.0 * (hy2 + hz2);
    let iyy = mass / 3.0 * (hx2 + hz2);
    let izz = mass / 3.0 * (hx2 + hy2);
    [
        if ixx > 1e-8 { 1.0 / ixx } else { 0.0 },
        if iyy > 1e-8 { 1.0 / iyy } else { 0.0 },
        if izz > 1e-8 { 1.0 / izz } else { 0.0 },
    ]
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn broadphase_stats_decode_explicit_drop_counters() {
        let mut words = [0u32; 7];
        words[ATOM_PAIR_DROPPED as usize] = 11;
        words[ATOM_INSERT_DROPPED as usize] = 12;
        words[ATOM_CONTACT_DROPPED as usize] = 13;
        words[ATOM_HASH_HOP_DROPPED as usize] = 14;

        assert_eq!(
            BroadphaseStats::from_atom_words(&words),
            BroadphaseStats {
                pair_candidates_dropped: 11,
                cell_inserts_dropped: 12,
                contact_pairs_dropped: 13,
                hash_traversals_dropped: 14,
            }
        );
    }

    #[test]
    fn radix_layout_covers_full_pair_capacity() {
        assert_eq!(RADIX_GROUPS * RADIX_GROUP_SIZE, PAIR_CAP);
        assert_eq!(RADIX_BUCKETS, 1 << 8);
        assert!(CONTACT_HASH_CAP.is_power_of_two());
        assert!(CONTACT_HASH_CAP >= 2 * PAIR_CAP);
    }

    #[test]
    fn contact_capacity_scales_with_world_demand() {
        assert_eq!(contact_capacity(1), 256);
        assert_eq!(contact_capacity(64), 1024);
        assert_eq!(contact_capacity(256), 4096);
        assert_eq!(contact_capacity(10_000), PAIR_CAP);
    }

    #[test]
    fn contact_split_preserves_export_size_without_duplication() {
        assert_eq!(core::mem::size_of::<ContactHotGpu>(), 256);
        assert_eq!(core::mem::size_of::<ContactPreparedGpu>(), 96);
        assert_eq!(core::mem::size_of::<ContactPersistentGpu>(), 224);
        assert_eq!(
            core::mem::size_of::<ContactHotGpu>()
                + core::mem::size_of::<ContactPreparedGpu>()
                + core::mem::size_of::<ContactPersistentGpu>(),
            core::mem::size_of::<ContactGpu>()
        );
    }

    #[test]
    fn scene_caps_grow_geometrically_and_fit_live_counts() {
        assert_eq!(grow_cap(256, 40), 256);
        assert_eq!(grow_cap(256, 300), 512);
        let live = GpuSceneCaps::live(40, 40, 10, 10, 10, 10, 0, 0, 0, 4, 0);
        let caps = GpuSceneCaps::allocate(live, 0, 0);
        assert!(caps.fits(live));
        assert_eq!(caps.bodies, DEFAULT_BODY_COUNT);
        let grown = caps.grow_to_fit(GpuSceneCaps::live(
            300, 40, 10, 10, 10, 10, 0, 0, 0, 4, 0,
        ));
        assert_eq!(grown.bodies, 512);
        assert!(grown.fits(GpuSceneCaps::live(
            300, 40, 10, 10, 10, 10, 0, 0, 0, 4, 0,
        )));
    }

    #[test]
    fn village_scale_cartesian_materials_are_the_132gb_abort() {
        let shapes = 52_500u64;
        let cartesian = shapes
            .checked_mul(shapes)
            .unwrap()
            .checked_mul(core::mem::size_of::<SurfaceMaterialGpu>() as u64)
            .unwrap();
        assert_eq!(cartesian, 132_300_000_000);
        let live = GpuSceneCaps::live(8, 52_500, 64, 64, 64, 64, 64, 64, 64, 52_500, 0);
        let linear = live.scene_heap_bytes().expect("linear heap");
        assert!(linear < MAX_SCENE_HEAP_BYTES, "{linear}");
        assert_eq!(crate::types::GPU_MAX_COMPOUND_CHILDREN, 4096);
        let mut huge = live;
        huge.materials = u32::try_from(shapes * shapes).unwrap_or(u32::MAX);
        assert!(huge.validate_allocation(1).is_err());
    }

    #[test]
    fn capsule_mass_matches_box3d_y_axis_unit_density() {
        let m = compute_capsule_mass([0.0, -1.0, 0.0], [0.0, 1.0, 0.0], 0.5, 1.0);
        assert!((m.mass - 2.0943951).abs() < 1e-6);
        assert!(
            (m.inertia[1] - 0.24870942).abs() < 1e-6,
            "axial {}",
            m.inertia[1]
        );
        assert!(
            (m.inertia[0] - 1.39408174).abs() < 1e-5,
            "transverse {}",
            m.inertia[0]
        );
        assert!((m.inertia[2] - 1.39408174).abs() < 1e-5);
        assert_eq!(m.center, [0.0, 0.0, 0.0]);
        assert!(m.inertia[3].abs() < 1e-7);
        assert!(m.inertia[4].abs() < 1e-7);
        assert!(m.inertia[5].abs() < 1e-7);
    }

    #[test]
    fn capsule_mass_x_axis_and_sphere_limit() {
        let x = compute_capsule_mass([-1.0, 0.0, 0.0], [1.0, 0.0, 0.0], 0.5, 1.0);
        assert!((x.inertia[0] - 0.24870942).abs() < 1e-6);
        assert!((x.inertia[1] - 1.39408174).abs() < 1e-5);
        let sph = compute_capsule_mass([1.0, 2.0, 3.0], [1.0, 2.0, 3.0], 0.5, 2.0);
        let i = 0.4 * sph.mass * 0.25;
        assert!((sph.inertia[0] - i).abs() < 1e-6);
        assert!((sph.inertia[1] - i).abs() < 1e-6);
        assert_eq!(sph.center, [1.0, 2.0, 3.0]);
    }

    #[test]
    fn capsule_mass_oblique_axis_is_symmetric() {
        let m = compute_capsule_mass([0.0, 0.0, 0.0], [1.0, 1.0, 0.0], 0.25, 3.0);
        assert!(m.mass > 0.0);
        assert!((m.center[0] - 0.5).abs() < 1e-6);
        assert!((m.center[1] - 0.5).abs() < 1e-6);
    }
}
