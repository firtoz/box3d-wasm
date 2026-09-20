// Bind group 0 layouts. Concatenated with the rest of shaders/physics/ in sim.rs.

struct Body {
    pos: vec3<f32>,
    inv_mass: f32,
    vel: vec3<f32>,
    kind: u32,
    half: vec3<f32>,
    flags: u32,
    rot: vec4<f32>,
    omega: vec3<f32>,
    restitution: f32,
    inv_inertia: vec3<f32>,
    inv_inertia_offdiag: vec3<f32>,
    friction: f32,
    gravity_scale: f32,
    linear_damping: f32,
    angular_damping: f32,
    rolling: f32,
    dp: vec3<f32>,
    sleep_time: f32,
    dq: vec4<f32>,
    island_id: u32,
    sleep_velocity: f32,
    _pad_island: vec2<u32>,
}

struct BodyState {
    pos: vec3<f32>,
    inv_mass: f32,
    vel: vec3<f32>,
    flags: u32,
    rot: vec4<f32>,
    omega: vec3<f32>,
    sleep_velocity: f32,
    dp: vec3<f32>,
    sleep_time: f32,
    dq: vec4<f32>,
}

struct BodyCold {
    half: vec3<f32>,
    kind: u32,
    inv_inertia: vec3<f32>,
    restitution: f32,
    gravity_scale: f32,
    linear_damping: f32,
    angular_damping: f32,
    friction: f32,
    rolling: f32,
    local_center: vec3<f32>,
}

struct Shape {
    body_index: u32,
    plane_slot: u32,
    kind: u32,
    topology_counts: u32,
    local_center: vec3<f32>,
    topology_slot: u32,
    half: vec3<f32>,
    rolling: f32,
    axis: vec3<f32>,
    friction: f32,
    restitution: f32,
    inner_radius: f32,
    hull_slot: u32,
    edge_slot: u32,
    category_bits_lo: u32,
    category_bits_hi: u32,
    mask_bits_lo: u32,
    mask_bits_hi: u32,
    group_index: i32,
    event_flags: u32,
    material_slot: u32,
    material_count: u32,
    tangent_velocity: vec3<f32>,
    custom_color: u32,
    user_material_id: vec2<u32>,
    _pad_filter: vec2<u32>,
    instance_position: vec3<f32>,
    instance_flags: u32,
    instance_rotation: vec4<f32>,
}

struct SurfaceMaterial {
    friction: f32,
    restitution: f32,
    rolling_resistance: f32,
    _pad0: f32,
    tangent_velocity: vec3<f32>,
    custom_color: u32,
    user_material_id: vec2<u32>,
    _pad1: vec2<u32>,
}

struct ContactHot {
    a: u32,
    b: u32,
    color: u32,
    count: u32,
    n: vec3<f32>,
    friction: f32,
    ra0: vec4<f32>,
    ra1: vec4<f32>,
    ra2: vec4<f32>,
    ra3: vec4<f32>,
    rb0: vec4<f32>,
    rb1: vec4<f32>,
    rb2: vec4<f32>,
    rb3: vec4<f32>,
    friction_impulse: vec2<f32>,
    twist_impulse: f32,
    rolling: f32,
    center_a: vec3<f32>,
    _pad_ca: f32,
    center_b: vec3<f32>,
    _pad_cb: f32,
    rolling_impulse: vec3<f32>,
    restitution: f32,
    tangent_velocity: vec3<f32>,
    material_index: u32,
    manifold_link: vec4<u32>,
}

struct ContactPrepared {
    relative_velocity: vec4<u32>,
    normal_mass: vec4<f32>,
    lever_arm: vec4<f32>,
    total_normal_impulse: vec4<f32>,
    tangent_inv: vec4<f32>,
    softness: vec4<f32>,
}

struct ContactPersistent {
    feature_ids: vec4<u32>,
    cached_relative: vec4<f32>,
    cached_rotation_a: vec4<f32>,
    cached_rotation_b: vec4<f32>,
    lifecycle: vec4<u32>,
    persistent_ra0: vec4<f32>,
    persistent_ra1: vec4<f32>,
    persistent_ra2: vec4<f32>,
    persistent_ra3: vec4<f32>,
    persistent_rb0: vec4<f32>,
    persistent_rb1: vec4<f32>,
    persistent_rb2: vec4<f32>,
    persistent_rb3: vec4<f32>,
    point_triangles: vec4<u32>,
}

struct Contact {
    a: u32,
    b: u32,
    color: u32,
    count: u32,
    n: vec3<f32>,
    friction: f32,
    ra0: vec4<f32>,
    ra1: vec4<f32>,
    ra2: vec4<f32>,
    ra3: vec4<f32>,
    rb0: vec4<f32>,
    rb1: vec4<f32>,
    rb2: vec4<f32>,
    rb3: vec4<f32>,
    friction_impulse: vec2<f32>,
    twist_impulse: f32,
    rolling: f32,
    center_a: vec3<f32>,
    _pad_ca: f32,
    center_b: vec3<f32>,
    _pad_cb: f32,
    rolling_impulse: vec3<f32>,
    _pad_end: f32,
    tangent_velocity: vec3<f32>,
    material_index: u32,
    _tail0: vec4<u32>,
    _tail1: vec4<u32>,
    cached_relative: vec4<f32>,
    cached_rotation_a: vec4<f32>,
    cached_rotation_b: vec4<f32>,
    lifecycle: vec4<u32>,
    prepared_normal_mass: vec4<f32>,
    prepared_lever_arm: vec4<f32>,
    total_normal_impulse: vec4<f32>,
    prepared_tangent_inv: vec4<f32>,
    prepared_softness: vec4<f32>,
    persistent_ra0: vec4<f32>,
    persistent_ra1: vec4<f32>,
    persistent_ra2: vec4<f32>,
    persistent_ra3: vec4<f32>,
    persistent_rb0: vec4<f32>,
    persistent_rb1: vec4<f32>,
    persistent_rb2: vec4<f32>,
    persistent_rb3: vec4<f32>,
    manifold_link: vec4<u32>,
    point_triangles: vec4<u32>,
}

struct Joint {
    a: u32,
    b: u32,
    kind: u32,
    _pad0: u32,
    anchor_a: vec3<f32>,
    hertz: f32,
    anchor_b: vec3<f32>,
    damping: f32,
    axis: vec3<f32>,
    impulse: f32,
    frame_a_rotation: vec4<f32>,
    frame_b_rotation: vec4<f32>,
    perp_impulse: vec2<f32>,
    flags: u32,
    _pad1: u32,
    angular_impulse: vec3<f32>,
    spring_impulse: f32,
    motor_impulse: f32,
    lower_impulse: f32,
    upper_impulse: f32,
    spring_hertz: f32,
    spring_damping: f32,
    target_translation: f32,
    lower_translation: f32,
    upper_translation: f32,
    max_motor_force: f32,
    motor_speed: f32,
    _pad2: vec2<f32>,
    motor_angular_velocity: vec3<f32>,
    _pad3: f32,
    target_rotation: vec4<f32>,
    spring_angular_impulse: vec3<f32>,
    swing_impulse: f32,
    motor_angular_impulse: vec3<f32>,
    _pad4: f32,
    weld_linear_hertz: f32,
    weld_linear_damping: f32,
    weld_angular_hertz: f32,
    weld_angular_damping: f32,
    weld_linear_impulse: vec3<f32>,
    _pad_weld_linear: f32,
    weld_angular_impulse: vec3<f32>,
    _pad_weld_angular: f32,
}

struct SimParams {
    dt: f32,
    gravity_x: f32,
    gravity_y: f32,
    gravity_z: f32,
    color_select: u32,
    enable_contacts: u32,
    max_contacts: u32,
    body_count: u32,
    cell_size: f32,
    bias_rate: f32,
    mass_scale: f32,
    impulse_scale: f32,
    contact_speed: f32,
    sleep_threshold: f32,
    use_bias: u32,
    joint_count: u32,
    solver_mode: u32,
    enable_sleep: u32,
    step_dt: f32,
    diagnostic_flags: u32,
    contact_hertz: f32,
    contact_damping: f32,
    contact_capacity: u32,
    shape_count: u32,
    shape_base_u32: u32,
    hull_point_count: u32,
    hull_base_u32: u32,
    hull_plane_count: u32,
    hull_plane_base_u32: u32,
    hull_edge_count: u32,
    hull_edge_base_u32: u32,
    hull_topology_count: u32,
    hull_topology_base_u32: u32,
    mesh_vertex_count: u32,
    mesh_vertex_base_u32: u32,
    mesh_triangle_count: u32,
    mesh_triangle_base_u32: u32,
    mesh_node_count: u32,
    mesh_node_base_u32: u32,
    surface_material_count: u32,
    surface_material_base_u32: u32,
    sub_step_count: u32,
    physics_step: u32,
    mix_pair_count: u32,
    mix_pair_base_u32: u32,
    enable_continuous: u32,
    contact_recycle_distance: f32,
    fat_bounds_base: u32,
    fat_bounds_epoch: u32,
    fat_commands_base: u32,
    fat_commands_epoch: u32,
    fat_commands_count: u32,
    remap_old_count: u32,
    remap_capture: u32,
    remap_history_step: u32,
    maximum_linear_speed: f32,
    restitution_threshold: f32,
}

const KIND_SPHERE: u32 = 0u;
const KIND_BOX: u32 = 1u;
const KIND_CAPSULE: u32 = 2u;
const KIND_CONVEX_HULL: u32 = 3u;
const KIND_MESH: u32 = 4u;
const FLAG_STATIC: u32 = 1u;
const FLAG_SLEEP: u32 = 4u;
const FLAG_SLEEP_ENABLED: u32 = 8u;
const FLAG_LOCK_ANG_X: u32 = 2048u;
const FLAG_LOCK_ANG_Y: u32 = 4096u;
const FLAG_LOCK_ANG_Z: u32 = 8192u;
const FLAG_FIXED_ROTATION: u32 = FLAG_LOCK_ANG_X | FLAG_LOCK_ANG_Y | FLAG_LOCK_ANG_Z;
const FLAG_KINEMATIC: u32 = 16u;
const FLAG_BULLET: u32 = 32u;
const FLAG_ALLOW_FAST_ROTATION: u32 = 64u;
const FLAG_DISABLED: u32 = 128u;
const FLAG_DISABLE_CONTACT_RECYCLING: u32 = 16384u;
const FLAG_FAST: u32 = 32768u;
const JOINT_NONE: u32 = 0u;
const JOINT_REVOLUTE: u32 = 1u;
const JOINT_WELD: u32 = 2u;
const JOINT_SPHERICAL: u32 = 3u;
const JOINT_PRISMATIC: u32 = 4u;
const JOINT_FILTER: u32 = 5u;
const JOINT_DISTANCE: u32 = 6u;
const JOINT_PARALLEL: u32 = 7u;
const JOINT_MOTOR: u32 = 8u;
const JOINT_WHEEL: u32 = 9u;
const TIME_TO_SLEEP: f32 = 0.5;
const EMPTY: u32 = 0xffffffffu;
const TOMBSTONE: u32 = 0xfffffffeu;
const OVERFLOW_COLOR: u32 = 23u;
const DYNAMIC_COLOR_COUNT: u32 = 20u;
const REVOLUTE_ENABLE_SPRING: u32 = 1u;
const REVOLUTE_ENABLE_LIMIT: u32 = 2u;
const REVOLUTE_ENABLE_MOTOR: u32 = 4u;
const SPHERICAL_ENABLE_SPRING: u32 = 1u;
const SPHERICAL_ENABLE_CONE_LIMIT: u32 = 2u;
const SPHERICAL_ENABLE_TWIST_LIMIT: u32 = 4u;
const SPHERICAL_ENABLE_MOTOR: u32 = 8u;
const PRISMATIC_ENABLE_SPRING: u32 = 1u;
const PRISMATIC_ENABLE_LIMIT: u32 = 2u;
const PRISMATIC_ENABLE_MOTOR: u32 = 4u;
const DISTANCE_ENABLE_SPRING: u32 = 1u;
const DISTANCE_ENABLE_LIMIT: u32 = 2u;
const DISTANCE_ENABLE_MOTOR: u32 = 4u;
const WHEEL_ENABLE_SUSPENSION_SPRING: u32 = 1u;
const WHEEL_ENABLE_SUSPENSION_LIMIT: u32 = 2u;
const WHEEL_ENABLE_SPIN_MOTOR: u32 = 4u;
const WHEEL_ENABLE_STEERING: u32 = 8u;
const WHEEL_ENABLE_STEERING_LIMIT: u32 = 16u;
const JOINT_COLLIDE_CONNECTED: u32 = 0x80000000u;
const SPECULATIVE: f32 = 0.02;
const LINEAR_SLOP: f32 = 0.005;
const CONTACT_RECYCLE: f32 = 0.05;
const CONTACT_RECYCLE_ANG: f32 = 0.99240388;
const CONTACT_ALIVE: u32 = 1u;
// Bits 16..19 record point matching independently of impulse magnitude.
const CONTACT_PERSISTED_SHIFT: u32 = 16u;
const CONTACT_PERSISTED_MASK: u32 = 0x000f0000u;
const CONTACT_TOUCHING: u32 = 2u;
const CONTACT_RECYCLED: u32 = 4u;
const CONTACT_START_TOUCHING: u32 = 8u;
const CONTACT_STOP_TOUCHING: u32 = 16u;
const SHAPE_ENABLE_CONTACT_EVENTS: u32 = 1u;
const SHAPE_ENABLE_SENSOR_EVENTS: u32 = 2u;
const SHAPE_IS_SENSOR: u32 = 4u;
const SHAPE_ENABLE_HIT_EVENTS: u32 = 8u;
const SHAPE_DISABLE_SPECULATIVE: u32 = 64u;
const SHAPE_PUBLIC_PROXY: u32 = 128u;
const SHAPE_COMPOUND_CHILD: u32 = 256u;
const DIAG_DISABLE_RECYCLING: u32 = 1u;
const DIAG_DISABLE_SAT_CACHE: u32 = 2u;
const DIAG_DISABLE_ROLLING: u32 = 4u;
const DIAG_REBUILD_GRAPH: u32 = 8u;
const DIAG_DISABLE_SLEEP: u32 = 16u;
const DIAG_PHASE_CAPTURE: u32 = 32u;
const DIAG_FORCE_CAPACITY_LOSS: u32 = 64u;
const DIAG_GENERAL_SOLVER: u32 = 128u;
const DIAG_FORCE_GENERAL_STATIC_SORT: u32 = 256u;
const DIAG_RECOMPUTE_TOPOLOGY: u32 = 512u;
const DIAG_STATIC_DEGREE_ONE_PROOF: u32 = 1024u;
const DIAG_JOINT_FILTER_SCAN: u32 = 2048u;
const DIAG_JOINT_FILTER_OVERFLOW: u32 = 4096u;
const DIAG_SERIAL_JOINTS: u32 = 8192u;
const DIAG_PARALLEL_JOINTS: u32 = 16384u;
const DIAG_MESH_CANDIDATES: u32 = 32768u;
const MESH_TRACE_WORDS: u32 = 32769u;
const JOINT_FILTER_CAP: u32 = 4096u;
const JOINT_FILTER_PROBE: u32 = 32u;
const HASH_BUCKETS: u32 = 16384u;
const MAX_INSERTS: u32 = 65536u;
const PAIR_CAP: u32 = 65536u;
const CONTACT_HASH_CAP: u32 = 131072u;
const SCR_PAIR_N: u32 = 0u;
const SCR_INSERT_N: u32 = 1u;
const SCR_UNIQUE_N: u32 = 2u;
const SCR_NCONTACTS: u32 = 3u;
const SCR_POW2: u32 = 4u;
const SCR_OCCUPIED_N: u32 = 5u;
const SCR_BSTRIDE: u32 = 6u;
const SCR_STATIC_N: u32 = 7u;
const SCR_INDIRECT_COLLIDE: u32 = 8u;
const SCR_INDIRECT_OCCUPIED: u32 = 12u;
const SCR_COLOR: u32 = 16u;
const SCR_INDIRECT_PREPARE: u32 = 40u;
const SCR_INDIRECT_ISLAND: u32 = 44u;
const SCR_INDIRECT_RADIX: u32 = 48u;
const SCR_RADIX_GROUP_N: u32 = 52u;
const SCR_FREE_N: u32 = 53u;
const SCR_MISSING_N: u32 = 54u;
const SCR_FUSED_N: u32 = 55u;
const SCR_DYN_DYN_N: u32 = 56u;
const SCR_GRAPH_STATIC_N: u32 = 57u;
const SCR_GRAPH_STATIC_MAX: u32 = 58u;
const SCR_INDIRECT_STATIC: u32 = 59u;
const SCR_JOINT_LIST_OK: u32 = 62u;
const SCR_JOINT_COMP_N: u32 = 63u;
const SCR_INDIRECT_COLOR: u32 = 64u;
const SCR_HASH: u32 = 64u;
const SCR_INS_BODY: u32 = SCR_HASH + HASH_BUCKETS;
const SCR_INS_NEXT: u32 = SCR_INS_BODY + MAX_INSERTS;
const SCR_INS_CELL: u32 = SCR_INS_NEXT + MAX_INSERTS;
const SCR_PAIRS: u32 = SCR_INS_CELL + MAX_INSERTS;
const SCR_COLOR_LIST: u32 = SCR_PAIRS + PAIR_CAP;
const SCR_CONTACT_MARK: u32 = SCR_COLOR_LIST;
const SCR_ACTIVE_CONTACT: u32 = SCR_CONTACT_MARK + PAIR_CAP;
const SCR_OCCUPIED_CONTACT: u32 = SCR_ACTIVE_CONTACT + PAIR_CAP;
// Graph dynamic-pair workspace becomes next occupied roots after graph assignment.
// The previous occupied roots stay immutable through parallel retirement.
const SCR_NEXT_OCCUPIED: u32 = SCR_OCCUPIED_CONTACT + PAIR_CAP;
const SCR_PREVIOUS_TOUCHING: u32 = SCR_NEXT_OCCUPIED + PAIR_CAP;
const SCR_RADIX_OUT: u32 = SCR_PREVIOUS_TOUCHING + PAIR_CAP;
const RADIX_GROUP_SIZE: u32 = 256u;
const RADIX_GROUPS: u32 = PAIR_CAP / RADIX_GROUP_SIZE;
const RADIX_BUCKETS: u32 = 256u;
const SCR_RADIX_HIST: u32 = SCR_RADIX_OUT + PAIR_CAP;
const SCR_RADIX_BASE: u32 = SCR_RADIX_HIST + RADIX_GROUPS * RADIX_BUCKETS;
const SCR_GRAPH: u32 = SCR_RADIX_BASE + RADIX_BUCKETS;
fn color_body_base() -> u32 {
    return SCR_GRAPH + 6u * params.body_count;
}
fn fused_flag_base() -> u32 {
    return color_body_base() + params.body_count;
}
fn fused_slots_base() -> u32 {
    return fused_flag_base() + params.body_count;
}
fn phase_summary_base() -> u32 {
    return fused_slots_base() + 8u * params.body_count;
}
fn atom_graph_color() -> u32 {
    return ATOM_JACOBI + 9u * params.body_count;
}
fn atom_dyn_dyn() -> u32 {
    return atom_graph_color() + 24u;
}
fn phase_body_base() -> u32 {
    return phase_summary_base() + 24u * 8u;
}
fn color_contact_base() -> u32 {
    return phase_body_base() + 24u * 8u * params.body_count;
}
fn joint_filter_base() -> u32 {
    return color_contact_base() + 24u * params.contact_capacity;
}
fn joint_head_base() -> u32 {
    return joint_filter_base() + JOINT_FILTER_CAP * 2u;
}
fn joint_comp_base() -> u32 {
    return joint_head_base() + params.body_count;
}
fn joint_list_off_base() -> u32 {
    return joint_comp_base() + params.body_count;
}
fn joint_list_count_base() -> u32 {
    return joint_list_off_base() + params.body_count;
}
fn joint_list_base() -> u32 {
    return joint_list_count_base() + params.body_count;
}
const SOLVER_TGS: u32 = 0u;
const SOLVER_JACOBI: u32 = 1u;
const MIN_FRICTION_WEIGHT: f32 = 1e-10;
const SAT_FACE_A: u32 = 1u;
const SAT_FACE_B: u32 = 2u;
const SAT_EDGE: u32 = 3u;
override ISLAND_WORKGROUP_SIZE: u32 = 64u;

const ATOM_PAIR_N: u32 = 0u;
const ATOM_INSERT_N: u32 = 1u;
const ATOM_PAIR_DROPPED: u32 = 2u;
const ATOM_INSERT_DROPPED: u32 = 3u;
const ATOM_CONTACT_DROPPED: u32 = 4u;
const ATOM_HASH_HOP_DROPPED: u32 = 5u;
const ATOM_OCCUPIED_N: u32 = 6u;
const ATOM_STATIC_N: u32 = 7u;
const ATOM_STICKY_PAIR_DROPPED: u32 = 8u;
const ATOM_STICKY_INSERT_DROPPED: u32 = 9u;
const ATOM_STICKY_CONTACT_DROPPED: u32 = 10u;
const ATOM_STICKY_HASH_HOP_DROPPED: u32 = 11u;
const ATOM_STICKY_FIRST_STEP: u32 = 12u;
const ATOM_GRAPH_PROOF_FAIL: u32 = 13u;
const ATOM_JOINT_LIST_DROPPED: u32 = 14u;
const ATOM_MANIFOLD_ALLOC: u32 = 15u;
const ATOM_HASH: u32 = 16u;
const ATOM_PAIR_SET: u32 = ATOM_HASH + HASH_BUCKETS;
const ATOM_CONTACT_KEY: u32 = ATOM_PAIR_SET + PAIR_CAP;
const ATOM_CONTACT_VALUE: u32 = ATOM_CONTACT_KEY + CONTACT_HASH_CAP;
const ATOM_JACOBI: u32 = ATOM_CONTACT_VALUE + CONTACT_HASH_CAP;
fn atom_island_label() -> u32 {
    return ATOM_JACOBI + 6u * params.body_count;
}
fn atom_island_wake() -> u32 {
    return atom_island_label() + params.body_count;
}
fn atom_island_ready() -> u32 {
    return atom_island_wake() + params.body_count;
}

@group(0) @binding(0) var<uniform> params: SimParams;
@group(0) @binding(1) var<storage, read_write> body_states: array<BodyState>;
@group(0) @binding(2) var<storage, read> scene_words: array<u32>;
@group(0) @binding(3) var<storage, read_write> contacts: array<ContactHot>;
@group(0) @binding(4) var<storage, read_write> joints: array<Joint>;
@group(0) @binding(5) var<storage, read_write> scratch: array<u32>;
@group(0) @binding(6) var<storage, read_write> atom: array<atomic<u32>>;
@group(0) @binding(7) var<storage, read_write> contact_persistent: array<ContactPersistent>;
@group(0) @binding(8) var<storage, read_write> contact_prepared: array<ContactPrepared>;
@group(0) @binding(9) var<storage, read_write> query: array<atomic<u32>>;

fn island_root(body: u32) -> u32 {
    let base = atom_island_label();
    var root = body;
    loop {
        let parent = atomicLoad(&atom[base + root]);
        if (parent == root || parent == EMPTY) {
            return parent;
        }
        root = parent;
    }
    return root;
}

fn island_union(a: u32, b: u32) {
    let base = atom_island_label();
    var ra = island_root(a);
    var rb = island_root(b);
    loop {
        if (ra == EMPTY || rb == EMPTY || ra == rb) {
            return;
        }
        let lo = min(ra, rb);
        let hi = max(ra, rb);
        atomicMin(&atom[base + hi], lo);
        ra = island_root(lo);
        rb = island_root(hi);
    }
}

fn record_capacity_drop_n(per_step: u32, sticky: u32, n: u32) {
    if (n == 0u) {
        return;
    }
    atomicAdd(&atom[per_step], n);
    atomicAdd(&atom[sticky], n);
    let sid = max(params.physics_step, 1u);
    loop {
        let r = atomicCompareExchangeWeak(&atom[ATOM_STICKY_FIRST_STEP], 0u, sid);
        if (r.exchanged || r.old_value != 0u) {
            break;
        }
    }
}

fn record_capacity_drop(per_step: u32, sticky: u32) {
    record_capacity_drop_n(per_step, sticky, 1u);
}

fn write_count_indirect(base: u32, count: u32, workgroup_size: u32) {
    let wg = max(workgroup_size, 1u);
    scratch[base] = max((count + wg - 1u) / wg, 1u);
    scratch[base + 1u] = 1u;
    scratch[base + 2u] = 1u;
    scratch[base + 3u] = 0u;
}

fn scene_f32(word: u32) -> f32 {
    return bitcast<f32>(scene_words[word]);
}

fn load_body_cold(i: u32) -> BodyCold {
    let w = 16u * i;
    return BodyCold(
        vec3<f32>(scene_f32(w), scene_f32(w + 1u), scene_f32(w + 2u)),
        scene_words[w + 3u],
        vec3<f32>(scene_f32(w + 4u), scene_f32(w + 5u), scene_f32(w + 6u)),
        scene_f32(w + 7u),
        scene_f32(w + 8u),
        scene_f32(w + 9u),
        scene_f32(w + 10u),
        scene_f32(w + 11u),
        scene_f32(w + 12u),
        vec3<f32>(scene_f32(w + 13u), scene_f32(w + 14u), scene_f32(w + 15u)),
    );
}

fn load_shape(i: u32) -> Shape {
    let w = params.shape_base_u32 + 44u * i;
    return Shape(
        scene_words[w],
        scene_words[w + 1u],
        scene_words[w + 2u],
        scene_words[w + 3u],
        vec3<f32>(scene_f32(w + 4u), scene_f32(w + 5u), scene_f32(w + 6u)),
        scene_words[w + 7u],
        vec3<f32>(scene_f32(w + 8u), scene_f32(w + 9u), scene_f32(w + 10u)),
        scene_f32(w + 11u),
        vec3<f32>(scene_f32(w + 12u), scene_f32(w + 13u), scene_f32(w + 14u)),
        scene_f32(w + 15u),
        scene_f32(w + 16u),
        scene_f32(w + 17u),
        scene_words[w + 18u],
        scene_words[w + 19u],
        scene_words[w + 20u],
        scene_words[w + 21u],
        scene_words[w + 22u],
        scene_words[w + 23u],
        bitcast<i32>(scene_words[w + 24u]),
        scene_words[w + 25u],
        scene_words[w + 26u],
        scene_words[w + 27u],
        vec3<f32>(scene_f32(w + 28u), scene_f32(w + 29u), scene_f32(w + 30u)),
        scene_words[w + 31u],
        vec2<u32>(scene_words[w + 32u], scene_words[w + 33u]),
        vec2<u32>(scene_words[w + 34u], scene_words[w + 35u]),
        vec3<f32>(scene_f32(w + 36u), scene_f32(w + 37u), scene_f32(w + 38u)),
        scene_words[w + 39u],
        vec4<f32>(scene_f32(w + 40u), scene_f32(w + 41u), scene_f32(w + 42u), scene_f32(w + 43u)),
    );
}

fn load_surface_material(shape: Shape, index: u32) -> SurfaceMaterial {
    let count = max(shape.material_count, 1u);
    let material = min(
        shape.material_slot + min(index, count - 1u),
        params.surface_material_count - 1u,
    );
    let w = params.surface_material_base_u32 + 12u * material;
    return SurfaceMaterial(
        scene_f32(w), scene_f32(w + 1u), scene_f32(w + 2u), scene_f32(w + 3u),
        vec3<f32>(scene_f32(w + 4u), scene_f32(w + 5u), scene_f32(w + 6u)),
        scene_words[w + 7u],
        vec2<u32>(scene_words[w + 8u], scene_words[w + 9u]),
        vec2<u32>(scene_words[w + 10u], scene_words[w + 11u]),
    );
}

fn load_hull_point(shape: Shape, i: u32) -> vec3<f32> {
    if (shape.kind == KIND_BOX) { return BOX_PT[i] * shape.half + shape.local_center; }
    let point = min(shape.hull_slot + i, params.hull_point_count - 1u);
    let w = params.hull_base_u32 + 4u * point;
    return vec3<f32>(scene_f32(w), scene_f32(w + 1u), scene_f32(w + 2u));
}

fn hull_point_count(shape: Shape) -> u32 {
    if (shape.kind == KIND_BOX) { return 8u; }
    return shape.topology_counts & 0xffu;
}

fn hull_plane_count(shape: Shape) -> u32 {
    if (shape.kind == KIND_BOX) { return 6u; }
    return (shape.topology_counts >> 8u) & 0xffu;
}

fn hull_edge_count(shape: Shape) -> u32 {
    return (shape.topology_counts >> 16u) & 0xffu;
}

fn hull_topology_count(shape: Shape) -> u32 {
    if (shape.kind == KIND_BOX) { return 24u; }
    return (shape.topology_counts >> 24u) & 0xffu;
}

fn load_hull_plane(shape: Shape, i: u32) -> vec4<f32> {
    if (shape.kind == KIND_BOX) { let n = BOX_FACE_N[i]; return vec4<f32>(n, dot(abs(n), shape.half) + dot(n, shape.local_center)); }
    let plane = min(shape.plane_slot + i, params.hull_plane_count - 1u);
    let w = params.hull_plane_base_u32 + 4u * plane;
    return vec4<f32>(
        scene_f32(w),
        scene_f32(w + 1u),
        scene_f32(w + 2u),
        scene_f32(w + 3u),
    );
}

fn load_hull_edge(shape: Shape, i: u32) -> vec3<f32> {
    let edge = min(shape.edge_slot + i, params.hull_edge_count - 1u);
    let w = params.hull_edge_base_u32 + 4u * edge;
    return vec3<f32>(scene_f32(w), scene_f32(w + 1u), scene_f32(w + 2u));
}

fn load_hull_topology(shape: Shape, i: u32) -> vec4<u32> {
    if (shape.kind == KIND_BOX) { return vec4<u32>(BOX_EDGE_NEXT[i], BOX_EDGE_TWIN[i], BOX_EDGE_ORIGIN[i], BOX_EDGE_FACE[i]); }
    let edge = min(shape.topology_slot + i, params.hull_topology_count - 1u);
    let w = params.hull_topology_base_u32 + 4u * edge;
    return vec4<u32>(scene_words[w], scene_words[w + 1u], scene_words[w + 2u], scene_words[w + 3u]);
}

fn mesh_to_body_point(shape: Shape, raw: vec3<f32>) -> vec3<f32> {
    if (shape.instance_flags == 0u) { return raw; }
    return shape.instance_position + quat_rotate(shape.instance_rotation, shape.axis * raw);
}
fn mesh_to_raw_vector(shape: Shape, body_vector: vec3<f32>) -> vec3<f32> {
    if (shape.instance_flags == 0u) { return body_vector; }
    return quat_inv_rotate(shape.instance_rotation, body_vector) / shape.axis;
}
fn mesh_to_raw_point(shape: Shape, body_point: vec3<f32>) -> vec3<f32> {
    if (shape.instance_flags == 0u) { return body_point; }
    return mesh_to_raw_vector(shape, body_point - shape.instance_position);
}

fn load_mesh_vertex(shape: Shape, i: u32) -> vec3<f32> {
    let vertex = min(shape.hull_slot + i, params.mesh_vertex_count - 1u);
    let w = params.mesh_vertex_base_u32 + 4u * vertex;
    return mesh_to_body_point(shape, vec3<f32>(scene_f32(w), scene_f32(w + 1u), scene_f32(w + 2u)));
}

fn load_mesh_triangle(shape: Shape, i: u32) -> vec4<u32> {
    let triangle = min(shape.plane_slot + i, params.mesh_triangle_count - 1u);
    let w = params.mesh_triangle_base_u32 + 4u * triangle;
    let tri = vec4<u32>(scene_words[w], scene_words[w + 1u], scene_words[w + 2u], scene_words[w + 3u]);
    if (shape.instance_flags != 0u && shape.axis.x * shape.axis.y * shape.axis.z < 0.0) {
        return vec4<u32>(tri.x, tri.z, tri.y, (tri.w & 0xffffff00u) | ((tri.w >> 4u) & 7u));
    }
    return tri;
}

fn load_mesh_node_lower(shape: Shape, i: u32) -> vec4<f32> {
    let node = min(shape.edge_slot + i, params.mesh_node_count - 1u);
    let w = params.mesh_node_base_u32 + 8u * node;
    return vec4<f32>(scene_f32(w), scene_f32(w + 1u), scene_f32(w + 2u), scene_f32(w + 3u));
}

fn load_mesh_node_upper(shape: Shape, i: u32) -> vec4<f32> {
    let node = min(shape.edge_slot + i, params.mesh_node_count - 1u);
    let w = params.mesh_node_base_u32 + 8u * node + 4u;
    return vec4<f32>(scene_f32(w), scene_f32(w + 1u), scene_f32(w + 2u), scene_f32(w + 3u));
}

// BodyCold (16 words) and body-extra (16 words) are capacity-sized arrays.
// shape_base_u32 follows both arrays; body_count is only the live slot span.
fn body_extra_offset(i: u32) -> u32 {
    let body_capacity = params.shape_base_u32 / 32u;
    return 16u * body_capacity + 16u * i;
}

fn load_body(i: u32) -> Body {
    let s = body_states[i];
    let c = load_body_cold(i);
    let extra = body_extra_offset(i);
    return Body(
        s.pos,
        s.inv_mass,
        s.vel,
        c.kind,
        c.half,
        s.flags,
        s.rot,
        s.omega,
        c.restitution,
        c.inv_inertia,
        vec3<f32>(
            scene_f32(extra),
            scene_f32(extra + 1u),
            scene_f32(extra + 2u),
        ),
        c.friction,
        c.gravity_scale,
        c.linear_damping,
        c.angular_damping,
        c.rolling,
        s.dp,
        s.sleep_time,
        s.dq,
        EMPTY,
        s.sleep_velocity,
        vec2<u32>(i),
    );
}

fn store_body(i: u32, b: Body) {
    body_states[i] = BodyState(
        b.pos,
        b.inv_mass,
        b.vel,
        b.flags,
        b.rot,
        b.omega,
        b.sleep_velocity,
        b.dp,
        b.sleep_time,
        b.dq,
    );
}

fn assemble_contact(h: ContactHot, p: ContactPersistent, f: ContactPrepared) -> Contact {
    return Contact(
        h.a, h.b, h.color, h.count,
        h.n, h.friction,
        h.ra0, h.ra1, h.ra2, h.ra3,
        h.rb0, h.rb1, h.rb2, h.rb3,
        h.friction_impulse, h.twist_impulse, h.rolling,
        h.center_a, h._pad_ca,
        h.center_b, h._pad_cb,
        h.rolling_impulse, h.restitution, h.tangent_velocity, h.material_index,
        f.relative_velocity, p.feature_ids,
        p.cached_relative, p.cached_rotation_a, p.cached_rotation_b, p.lifecycle,
        f.normal_mass, f.lever_arm, f.total_normal_impulse, f.tangent_inv, f.softness,
        p.persistent_ra0, p.persistent_ra1, p.persistent_ra2, p.persistent_ra3,
        p.persistent_rb0, p.persistent_rb1, p.persistent_rb2, p.persistent_rb3, h.manifold_link, p.point_triangles,
    );
}

fn load_contact(slot: u32) -> Contact {
    return assemble_contact(
        contacts[slot],
        contact_persistent[slot],
        contact_prepared[slot],
    );
}

fn load_solve_contact(slot: u32) -> Contact {
    let h = contacts[slot];
    let f = contact_prepared[slot];
    var p: ContactPersistent;
    p.feature_ids = vec4<u32>(0u);
    p.cached_relative = vec4<f32>(0.0);
    p.cached_rotation_a = vec4<f32>(0.0);
    p.cached_rotation_b = vec4<f32>(0.0);
    p.lifecycle = vec4<u32>(0u);
    p.persistent_ra0 = vec4<f32>(0.0);
    p.persistent_ra1 = vec4<f32>(0.0);
    p.persistent_ra2 = vec4<f32>(0.0);
    p.persistent_ra3 = vec4<f32>(0.0);
    p.persistent_rb0 = vec4<f32>(0.0);
    p.persistent_rb1 = vec4<f32>(0.0);
    p.persistent_rb2 = vec4<f32>(0.0);
    p.persistent_rb3 = vec4<f32>(0.0);
    return assemble_contact(h, p, f);
}

fn store_hot(slot: u32, c: Contact) {
    contacts[slot] = ContactHot(
        c.a, c.b, c.color, c.count,
        c.n, c.friction,
        c.ra0, c.ra1, c.ra2, c.ra3,
        c.rb0, c.rb1, c.rb2, c.rb3,
        c.friction_impulse, c.twist_impulse, c.rolling,
        c.center_a, c._pad_ca, c.center_b, c._pad_cb,
        c.rolling_impulse, c._pad_end, c.tangent_velocity, c.material_index, c.manifold_link,
    );
}

fn store_prepared(slot: u32, c: Contact) {
    contact_prepared[slot] = ContactPrepared(
        c._tail0,
        c.prepared_normal_mass,
        c.prepared_lever_arm,
        c.total_normal_impulse,
        c.prepared_tangent_inv,
        c.prepared_softness,
    );
}

fn store_graph_meta(slot: u32, color: u32, local: u32) {
    contacts[slot].color = color;
    contact_persistent[slot].lifecycle.z = local;
}

fn store_contact(slot: u32, c: Contact) {
    store_hot(slot, c);
    store_prepared(slot, c);
    contact_persistent[slot] = ContactPersistent(
        c._tail1,
        c.cached_relative,
        c.cached_rotation_a,
        c.cached_rotation_b,
        c.lifecycle,
        c.persistent_ra0, c.persistent_ra1, c.persistent_ra2, c.persistent_ra3,
        c.persistent_rb0, c.persistent_rb1, c.persistent_rb2, c.persistent_rb3, c.point_triangles,
    );
}

fn store_solve_contact(slot: u32, c: Contact) {
    store_hot(slot, c);
    store_prepared(slot, c);
}

const DIAG_STATIC_DEGREE_TWO_PROOF:u32=65536u;

// Solve phases mutate impulses only; preparation still stores the full record.
fn store_contact_impulses(slot: u32, c: Contact) {
    contacts[slot].rb0.w = c.rb0.w;
    contacts[slot].rb1.w = c.rb1.w;
    contacts[slot].rb2.w = c.rb2.w;
    contacts[slot].rb3.w = c.rb3.w;
    contacts[slot].friction_impulse = c.friction_impulse;
    contacts[slot].twist_impulse = c.twist_impulse;
    contacts[slot].rolling_impulse = c.rolling_impulse;
    contact_prepared[slot].total_normal_impulse = c.total_normal_impulse;
}
