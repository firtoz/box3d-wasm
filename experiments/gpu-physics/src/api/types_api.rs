//! World / body / shape defs with Box3D field names (subset used by this GPU crate).

use std::ffi::c_void;

use super::ids::ShapeId;
use super::ids::{b3_null_body_id, BodyId, JointId};
use crate::types::{
    FLAG_LOCK_ANG_X, FLAG_LOCK_ANG_Y, FLAG_LOCK_ANG_Z, FLAG_LOCK_LIN_X, FLAG_LOCK_LIN_Y,
    FLAG_LOCK_LIN_Z,
};

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum BodyType {
    Static = 0,
    Kinematic = 1,
    Dynamic = 2,
}

/// Optional world capacities. Same fields as Box3D `b3Capacity`.
/// Zero means “use the GPU default floor”. Set these to the peak live count
/// so spawn/despawn stays a slot pool instead of growing buffers.
#[derive(Clone, Copy, Debug, Default)]
pub struct WorldCapacity {
    pub static_shape_count: i32,
    pub dynamic_shape_count: i32,
    pub static_body_count: i32,
    pub dynamic_body_count: i32,
    pub contact_count: i32,
}

#[derive(Clone, Copy, Debug)]
pub struct WorldDef {
    pub gravity: [f32; 3],
    pub hit_event_threshold: f32,
    pub contact_hertz: f32,
    pub contact_damping_ratio: f32,
    pub contact_speed: f32,
    pub maximum_linear_speed: f32,
    pub restitution_threshold: f32,
    pub enable_sleep: bool,
    pub enable_continuous: bool,
    pub capacity: WorldCapacity,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct MassData {
    pub mass: f32,
    pub center: [f32; 3],
    pub inertia: [[f32; 3]; 3],
}

#[derive(Clone, Copy, Debug, Default)]
pub struct MotionLocks {
    pub linear_x: bool,
    pub linear_y: bool,
    pub linear_z: bool,
    pub angular_x: bool,
    pub angular_y: bool,
    pub angular_z: bool,
}

impl MotionLocks {
    pub fn to_flags(self) -> u32 {
        let mut f = 0u32;
        if self.linear_x {
            f |= FLAG_LOCK_LIN_X;
        }
        if self.linear_y {
            f |= FLAG_LOCK_LIN_Y;
        }
        if self.linear_z {
            f |= FLAG_LOCK_LIN_Z;
        }
        if self.angular_x {
            f |= FLAG_LOCK_ANG_X;
        }
        if self.angular_y {
            f |= FLAG_LOCK_ANG_Y;
        }
        if self.angular_z {
            f |= FLAG_LOCK_ANG_Z;
        }
        f
    }

    pub fn from_flags(flags: u32) -> Self {
        Self {
            linear_x: flags & FLAG_LOCK_LIN_X != 0,
            linear_y: flags & FLAG_LOCK_LIN_Y != 0,
            linear_z: flags & FLAG_LOCK_LIN_Z != 0,
            angular_x: flags & FLAG_LOCK_ANG_X != 0,
            angular_y: flags & FLAG_LOCK_ANG_Y != 0,
            angular_z: flags & FLAG_LOCK_ANG_Z != 0,
        }
    }
}

#[derive(Clone, Copy, Debug)]
pub struct BodyDef {
    pub body_type: BodyType,
    pub position: [f32; 3],
    pub rotation: [f32; 4],
    pub linear_velocity: [f32; 3],
    pub angular_velocity: [f32; 3],
    pub linear_damping: f32,
    pub angular_damping: f32,
    pub gravity_scale: f32,
    pub is_awake: bool,
    pub enable_sleep: bool,
    pub enable_contact_recycling: bool,
    pub is_bullet: bool,
    pub allow_fast_rotation: bool,
    pub motion_locks: MotionLocks,
    pub user_data: usize,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct Filter {
    pub category_bits: u64,
    pub mask_bits: u64,
    pub group_index: i32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct ContactId {
    pub index1: i32,
    pub world0: u16,
    pub padding: i16,
    pub generation: u32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ContactBeginTouchEvent {
    pub shape_id_a: ShapeId,
    pub shape_id_b: ShapeId,
    pub contact_id: ContactId,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ContactEndTouchEvent {
    pub shape_id_a: ShapeId,
    pub shape_id_b: ShapeId,
    pub contact_id: ContactId,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ContactHitEvent {
    pub shape_id_a: ShapeId,
    pub shape_id_b: ShapeId,
    pub contact_id: ContactId,
    pub point: [f32; 3],
    pub normal: [f32; 3],
    pub approach_speed: f32,
    pub user_material_id_a: u64,
    pub user_material_id_b: u64,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct BodyMoveEvent {
    /// Stored as an integer so world storage remains `Send`; this has pointer
    /// size/alignment and is exposed as `void*` by the C ABI.
    pub user_data: usize,
    pub transform: WorldTransform,
    pub body_id: BodyId,
    pub fell_asleep: bool,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct JointEvent {
    pub joint_id: JointId,
    /// Stored as an integer so world storage remains `Send`; this has pointer
    /// size/alignment and is exposed as `void*` by the C ABI.
    pub user_data: usize,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct WorldTransform {
    pub p: [f32; 3],
    pub q: [f32; 4],
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct SensorBeginTouchEvent {
    pub sensor_shape_id: ShapeId,
    pub visitor_shape_id: ShapeId,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct SensorEndTouchEvent {
    pub sensor_shape_id: ShapeId,
    pub visitor_shape_id: ShapeId,
}

const _: () = assert!(core::mem::size_of::<ContactId>() == 12);
const _: () = assert!(core::mem::size_of::<ContactBeginTouchEvent>() == 28);
const _: () = assert!(core::mem::size_of::<ContactEndTouchEvent>() == 28);
const _: () = assert!(core::mem::size_of::<ContactHitEvent>() == 72);
const _: () = assert!(core::mem::size_of::<BodyMoveEvent>() == 48);
const _: () = assert!(core::mem::size_of::<JointEvent>() == 16);
const _: () = assert!(core::mem::size_of::<SensorBeginTouchEvent>() == 16);
const _: () = assert!(core::mem::size_of::<SensorEndTouchEvent>() == 16);

#[derive(Clone, Copy, Debug)]
pub struct ShapeDef {
    pub density: f32,
    pub friction: f32,
    pub restitution: f32,
    pub rolling_resistance: f32,
    pub explosion_scale: f32,
    pub filter: Filter,
    pub is_sensor: bool,
    pub enable_sensor_events: bool,
    pub enable_contact_events: bool,
    pub enable_hit_events: bool,
    pub enable_custom_filtering: bool,
    pub enable_pre_solve_events: bool,
    pub enable_speculative_contact: bool,
    pub user_material_id: u64,
    pub user_data: usize,
    pub update_body_mass: bool,
}

/// Box3D surface properties. Meshes and height fields may select one entry per
/// triangle/cell; convex shapes use a single entry.
#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct SurfaceMaterial {
    pub friction: f32,
    pub restitution: f32,
    pub rolling_resistance: f32,
    pub tangent_velocity: [f32; 3],
    pub user_material_id: u64,
    pub custom_color: u32,
    pub padding: u32,
}

pub type FrictionCallback = unsafe extern "C" fn(f32, u64, f32, u64) -> f32;
pub type RestitutionCallback = unsafe extern "C" fn(f32, u64, f32, u64) -> f32;

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ExplosionDef {
    pub mask_bits: u64,
    pub position: [f32; 3],
    pub radius: f32,
    pub falloff: f32,
    pub impulse_per_area: f32,
}

const _: () = assert!(core::mem::size_of::<ExplosionDef>() == 32);

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct Sphere {
    pub center: [f32; 3],
    pub radius: f32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct Capsule {
    pub center1: [f32; 3],
    pub center2: [f32; 3],
    pub radius: f32,
}

#[derive(Clone, Copy, Debug)]
pub struct BoxHull {
    pub half_extents: [f32; 3],
    pub center: [f32; 3],
}

#[derive(Clone, Debug)]
pub struct ConvexHull {
    pub points: Vec<[f32; 3]>,
    /// Local face planes as normal.xyz and offset.
    pub planes: Vec<[f32; 4]>,
    /// Unique unoriented local edge directions.
    pub edge_directions: Vec<[f32; 3]>,
    /// Half edges as next, twin, origin, face indices.
    pub half_edges: Vec<[u32; 4]>,
    pub half_extents: [f32; 3],
    pub aabb_center: [f32; 3],
    pub center: [f32; 3],
    pub inner_radius: f32,
    pub volume: f32,
    /// Symmetric central inertia tensor per unit density: xx, yy, zz, xy, xz, yz.
    pub central_inertia: [f32; 6],
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct Aabb {
    pub lower_bound: [f32; 3],
    pub upper_bound: [f32; 3],
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct Vec3 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}

impl From<[f32; 3]> for Vec3 {
    fn from(value: [f32; 3]) -> Self {
        Self {
            x: value[0],
            y: value[1],
            z: value[2],
        }
    }
}

impl From<Vec3> for [f32; 3] {
    fn from(value: Vec3) -> Self {
        [value.x, value.y, value.z]
    }
}

/// Box3D-compatible custom pair filter. The callback runs while the world is
/// locked; like Box3D, it must not mutate or query that world.
pub type CustomFilterCallback = unsafe extern "C" fn(ShapeId, ShapeId, *mut c_void) -> bool;

/// Box3D-compatible pre-solve callback. Returning false disables the complete
/// manifold for the current step only.
pub type PreSolveCallback = unsafe extern "C" fn(ShapeId, ShapeId, Vec3, Vec3, *mut c_void) -> bool;

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct QueryFilter {
    pub category_bits: u64,
    pub mask_bits: u64,
    pub id: u64,
    pub name: *const core::ffi::c_char,
}

impl Default for QueryFilter {
    fn default() -> Self {
        Self {
            category_bits: u64::MAX,
            mask_bits: u64::MAX,
            id: 0,
            name: core::ptr::null(),
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ShapeProxy {
    pub points: *const [f32; 3],
    pub count: i32,
    pub radius: f32,
}

impl Default for ShapeProxy {
    fn default() -> Self {
        Self {
            points: core::ptr::null(),
            count: 0,
            radius: 0.0,
        }
    }
}

impl ShapeProxy {
    /// # Safety
    /// `points` must reference at least `count` valid vectors for the duration
    /// of the returned slice.
    pub unsafe fn as_slice(&self) -> &[[f32; 3]] {
        if self.points.is_null() || self.count <= 0 {
            &[]
        } else {
            core::slice::from_raw_parts(self.points, self.count as usize)
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct TreeStats {
    pub node_visits: i32,
    pub leaf_visits: i32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct CastOutput {
    pub normal: [f32; 3],
    pub point: [f32; 3],
    pub fraction: f32,
    pub iterations: i32,
    pub triangle_index: i32,
    pub child_index: i32,
    pub material_index: i32,
    pub hit: bool,
}

pub type WorldCastOutput = CastOutput;

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct BodyCastResult {
    pub shape_id: ShapeId,
    pub point: [f32; 3],
    pub normal: [f32; 3],
    pub fraction: f32,
    pub triangle_index: i32,
    pub user_material_id: u64,
    pub iterations: i32,
    pub hit: bool,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct RayResult {
    pub shape_id: ShapeId,
    pub point: [f32; 3],
    pub normal: [f32; 3],
    pub user_material_id: u64,
    pub fraction: f32,
    pub triangle_index: i32,
    pub child_index: i32,
    pub node_visits: i32,
    pub leaf_visits: i32,
    pub hit: bool,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct QueryProfile {
    pub wait_ms: f32,
    pub staging_ms: f32,
    pub map_ms: f32,
    pub apply_ms: f32,
    pub ccd_ms: f32,
    pub construct_ms: f32,
    pub index_ms: f32,
    pub broadphase_ms: f32,
    pub exact_ms: f32,
    pub callback_ms: f32,
    pub encode_ms: f32,
    pub copied_bytes: u64,
    pub visited_shapes: u32,
    pub rejected_shapes: u32,
    pub exact_casts: u32,
    pub overflow: u32,
}

pub type OverlapResultFcn =
    unsafe extern "C" fn(shape_id: ShapeId, context: *mut core::ffi::c_void) -> bool;
pub type CastResultFcn = unsafe extern "C" fn(
    shape_id: ShapeId,
    point: Vec3,
    normal: Vec3,
    fraction: f32,
    user_material_id: u64,
    triangle_index: i32,
    child_index: i32,
    context: *mut core::ffi::c_void,
) -> f32;

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct Plane {
    pub normal: Vec3,
    pub offset: f32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct PlaneResult {
    pub plane: Plane,
    pub point: Vec3,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct CollisionPlane {
    pub plane: Plane,
    pub push_limit: f32,
    pub push: f32,
    pub clip_velocity: bool,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct PlaneSolverResult {
    pub delta: Vec3,
    pub iteration_count: i32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct BodyPlaneResult {
    pub shape_id: ShapeId,
    pub result: PlaneResult,
}

pub type PlaneResultFcn = unsafe extern "C" fn(
    shape_id: ShapeId,
    planes: *const PlaneResult,
    plane_count: i32,
    context: *mut core::ffi::c_void,
) -> bool;
pub type MoverFilterFcn =
    unsafe extern "C" fn(shape_id: ShapeId, context: *mut core::ffi::c_void) -> bool;

const _: () = assert!(core::mem::size_of::<Aabb>() == 24);
const _: () = assert!(core::mem::size_of::<QueryFilter>() == 32);
const _: () = assert!(core::mem::size_of::<ShapeProxy>() == 16);
const _: () = assert!(core::mem::size_of::<TreeStats>() == 8);
const _: () = assert!(core::mem::size_of::<CastOutput>() == 48);
const _: () = assert!(core::mem::size_of::<BodyCastResult>() == 56);
const _: () = assert!(core::mem::size_of::<RayResult>() == 64);
const _: () = assert!(core::mem::size_of::<Plane>() == 16);
const _: () = assert!(core::mem::size_of::<PlaneResult>() == 28);
const _: () = assert!(core::mem::size_of::<CollisionPlane>() == 28);
const _: () = assert!(core::mem::size_of::<PlaneSolverResult>() == 16);
const _: () = assert!(core::mem::size_of::<BodyPlaneResult>() == 36);

#[derive(Clone, Copy, Debug)]
pub struct RevoluteJointDef {
    pub body_a: BodyId,
    pub body_b: BodyId,
    pub local_anchor_a: [f32; 3],
    pub local_anchor_b: [f32; 3],
    pub local_rotation_a: [f32; 4],
    pub local_rotation_b: [f32; 4],
    pub hertz: f32,
    pub damping: f32,
    pub force_threshold: f32,
    pub torque_threshold: f32,
    pub user_data: usize,
    pub collide_connected: bool,
    pub target_angle: f32,
    pub enable_spring: bool,
    pub spring_hertz: f32,
    pub spring_damping: f32,
    pub enable_limit: bool,
    pub lower_angle: f32,
    pub upper_angle: f32,
    pub enable_motor: bool,
    pub max_motor_torque: f32,
    pub motor_speed: f32,
}

#[derive(Clone, Copy, Debug)]
pub struct SphericalJointDef {
    pub body_a: BodyId,
    pub body_b: BodyId,
    pub local_anchor_a: [f32; 3],
    pub local_anchor_b: [f32; 3],
    pub local_rotation_a: [f32; 4],
    pub local_rotation_b: [f32; 4],
    pub hertz: f32,
    pub damping: f32,
    pub force_threshold: f32,
    pub torque_threshold: f32,
    pub user_data: usize,
    pub collide_connected: bool,
    pub enable_spring: bool,
    pub spring_hertz: f32,
    pub spring_damping: f32,
    pub target_rotation: [f32; 4],
    pub enable_cone_limit: bool,
    pub cone_angle: f32,
    pub enable_twist_limit: bool,
    pub lower_twist_angle: f32,
    pub upper_twist_angle: f32,
    pub enable_motor: bool,
    pub max_motor_torque: f32,
    pub motor_velocity: [f32; 3],
}

#[derive(Clone, Copy, Debug)]
pub struct PrismaticJointDef {
    pub body_a: BodyId,
    pub body_b: BodyId,
    pub local_anchor_a: [f32; 3],
    pub local_anchor_b: [f32; 3],
    pub local_axis_a: [f32; 3],
    pub local_rotation_a: [f32; 4],
    pub local_rotation_b: [f32; 4],
    pub hertz: f32,
    pub damping: f32,
    pub force_threshold: f32,
    pub torque_threshold: f32,
    pub user_data: usize,
    pub collide_connected: bool,
    pub enable_spring: bool,
    pub spring_hertz: f32,
    pub spring_damping: f32,
    pub target_translation: f32,
    pub enable_limit: bool,
    pub lower_translation: f32,
    pub upper_translation: f32,
    pub enable_motor: bool,
    pub max_motor_force: f32,
    pub motor_speed: f32,
}

#[derive(Clone, Copy, Debug)]
pub struct DistanceJointDef {
    pub body_a: BodyId,
    pub body_b: BodyId,
    pub local_anchor_a: [f32; 3],
    pub local_anchor_b: [f32; 3],
    pub hertz: f32,
    pub damping: f32,
    pub force_threshold: f32,
    pub torque_threshold: f32,
    pub user_data: usize,
    pub collide_connected: bool,
    pub length: f32,
    pub enable_spring: bool,
    pub lower_spring_force: f32,
    pub upper_spring_force: f32,
    pub spring_hertz: f32,
    pub spring_damping: f32,
    pub enable_limit: bool,
    pub min_length: f32,
    pub max_length: f32,
    pub enable_motor: bool,
    pub max_motor_force: f32,
    pub motor_speed: f32,
}

#[derive(Clone, Copy, Debug)]
pub struct ParallelJointDef {
    pub body_a: BodyId,
    pub body_b: BodyId,
    pub local_rotation_a: [f32; 4],
    pub local_rotation_b: [f32; 4],
    pub hertz: f32,
    pub damping: f32,
    pub force_threshold: f32,
    pub torque_threshold: f32,
    pub user_data: usize,
    pub collide_connected: bool,
    pub max_torque: f32,
}

#[derive(Clone, Copy, Debug)]
pub struct FilterJointDef {
    pub body_a: BodyId,
    pub body_b: BodyId,
    pub force_threshold: f32,
    pub torque_threshold: f32,
    pub user_data: usize,
    pub collide_connected: bool,
}

#[derive(Clone, Copy, Debug)]
pub struct MotorJointDef {
    pub body_a: BodyId,
    pub body_b: BodyId,
    pub local_anchor_a: [f32; 3],
    pub local_anchor_b: [f32; 3],
    pub local_rotation_a: [f32; 4],
    pub local_rotation_b: [f32; 4],
    pub linear_velocity: [f32; 3],
    pub max_velocity_force: f32,
    pub angular_velocity: [f32; 3],
    pub max_velocity_torque: f32,
    pub linear_hertz: f32,
    pub linear_damping: f32,
    pub max_spring_force: f32,
    pub angular_hertz: f32,
    pub angular_damping: f32,
    pub max_spring_torque: f32,
    pub force_threshold: f32,
    pub torque_threshold: f32,
    pub user_data: usize,
    pub collide_connected: bool,
}

#[derive(Clone, Copy, Debug)]
pub struct WeldJointDef {
    pub body_a: BodyId,
    pub body_b: BodyId,
    pub local_anchor_a: [f32; 3],
    pub local_anchor_b: [f32; 3],
    pub local_rotation_a: [f32; 4],
    pub local_rotation_b: [f32; 4],
    pub hertz: f32,
    pub damping: f32,
    pub force_threshold: f32,
    pub torque_threshold: f32,
    pub user_data: usize,
    pub collide_connected: bool,
    pub linear_hertz: f32,
    pub linear_damping_ratio: f32,
    pub angular_hertz: f32,
    pub angular_damping_ratio: f32,
}

#[derive(Clone, Copy, Debug)]
pub struct WheelJointDef {
    pub body_a: BodyId,
    pub body_b: BodyId,
    pub local_anchor_a: [f32; 3],
    pub local_anchor_b: [f32; 3],
    pub local_rotation_a: [f32; 4],
    pub local_rotation_b: [f32; 4],
    pub hertz: f32,
    pub damping: f32,
    pub force_threshold: f32,
    pub torque_threshold: f32,
    pub user_data: usize,
    pub collide_connected: bool,
    pub enable_suspension_spring: bool,
    pub suspension_hertz: f32,
    pub suspension_damping_ratio: f32,
    pub enable_suspension_limit: bool,
    pub lower_suspension_limit: f32,
    pub upper_suspension_limit: f32,
    pub enable_spin_motor: bool,
    pub max_spin_torque: f32,
    pub spin_speed: f32,
    pub enable_steering: bool,
    pub steering_hertz: f32,
    pub steering_damping_ratio: f32,
    pub target_steering_angle: f32,
    pub max_steering_torque: f32,
    pub enable_steering_limit: bool,
    pub lower_steering_limit: f32,
    pub upper_steering_limit: f32,
}

pub const B3_SECRET_COOKIE: i32 = 1_152_023;

pub fn b3_default_world_def() -> WorldDef {
    WorldDef {
        gravity: [0.0, -10.0, 0.0],
        hit_event_threshold: 1.0,
        contact_hertz: 30.0,
        contact_damping_ratio: 10.0,
        contact_speed: 3.0,
        maximum_linear_speed: 400.0,
        restitution_threshold: 1.0,
        enable_sleep: true,
        enable_continuous: true,
        capacity: WorldCapacity::default(),
    }
}

pub fn b3_default_body_def() -> BodyDef {
    BodyDef {
        body_type: BodyType::Static,
        position: [0.0, 0.0, 0.0],
        rotation: [0.0, 0.0, 0.0, 1.0],
        linear_velocity: [0.0, 0.0, 0.0],
        angular_velocity: [0.0, 0.0, 0.0],
        linear_damping: 0.0,
        angular_damping: 0.0,
        gravity_scale: 1.0,
        is_awake: true,
        enable_sleep: true,
        enable_contact_recycling: true,
        is_bullet: false,
        allow_fast_rotation: false,
        motion_locks: MotionLocks::default(),
        user_data: 0,
    }
}

pub fn b3_default_shape_def() -> ShapeDef {
    ShapeDef {
        density: 1000.0,
        friction: 0.6,
        restitution: 0.0,
        rolling_resistance: 0.0,
        explosion_scale: 1.0,
        filter: b3_default_filter(),
        is_sensor: false,
        enable_sensor_events: false,
        enable_contact_events: false,
        enable_hit_events: false,
        enable_custom_filtering: false,
        enable_pre_solve_events: false,
        enable_speculative_contact: true,
        user_material_id: 0,
        user_data: 0,
        update_body_mass: true,
    }
}

pub fn b3_default_surface_material() -> SurfaceMaterial {
    SurfaceMaterial {
        friction: 0.6,
        ..SurfaceMaterial::default()
    }
}

pub fn b3_default_explosion_def() -> ExplosionDef {
    ExplosionDef {
        mask_bits: u64::MAX,
        position: [0.0; 3],
        radius: 0.0,
        falloff: 0.0,
        impulse_per_area: 0.0,
    }
}

pub fn b3_default_query_filter() -> QueryFilter {
    QueryFilter::default()
}

pub fn b3_default_filter() -> Filter {
    Filter {
        category_bits: 1,
        mask_bits: u64::MAX,
        group_index: 0,
    }
}

pub fn b3_should_shapes_collide(a: Filter, b: Filter) -> bool {
    if a.group_index == b.group_index && a.group_index != 0 {
        return a.group_index > 0;
    }
    (a.mask_bits & b.category_bits) != 0 && (a.category_bits & b.mask_bits) != 0
}

pub const B3_PI: f32 = 3.141_592_653_59;
pub const B3_DEG_TO_RAD: f32 = 0.017_453_292_51;

pub fn b3_make_box_hull(hx: f32, hy: f32, hz: f32) -> BoxHull {
    BoxHull {
        half_extents: [hx, hy, hz],
        center: [0.0; 3],
    }
}

pub fn b3_make_cube_hull(half: f32) -> BoxHull {
    b3_make_box_hull(half, half, half)
}

/// Box3D `b3UnwindAngle`: IEEE remainder into `[-pi, pi]`.
fn b3_unwind_angle(radians: f32) -> f32 {
    let two_pi = 2.0 * B3_PI;
    radians - two_pi * (radians / two_pi).round()
}

/// Box3D `b3ComputeCosSin` (Bhaskara, then renormalize).
pub fn b3_compute_cos_sin(radians: f32) -> (f32, f32) {
    let x = b3_unwind_angle(radians);
    let pi2 = B3_PI * B3_PI;
    let c = if x < -0.5 * B3_PI {
        let y = x + B3_PI;
        let y2 = y * y;
        -(pi2 - 4.0 * y2) / (pi2 + y2)
    } else if x > 0.5 * B3_PI {
        let y = x - B3_PI;
        let y2 = y * y;
        -(pi2 - 4.0 * y2) / (pi2 + y2)
    } else {
        let y2 = x * x;
        (pi2 - 4.0 * y2) / (pi2 + y2)
    };
    let s = if x < 0.0 {
        let y = x + B3_PI;
        -16.0 * y * (B3_PI - y) / (5.0 * pi2 - 4.0 * y * (B3_PI - y))
    } else {
        16.0 * x * (B3_PI - x) / (5.0 * pi2 - 4.0 * x * (B3_PI - x))
    };
    let mag = (s * s + c * c).sqrt();
    let inv = if mag > 0.0 { 1.0 / mag } else { 0.0 };
    (c * inv, s * inv)
}

/// Box3D `b3MakeQuatFromAxisAngle`. Axis must be unit. Returns `[x, y, z, w]`.
pub fn b3_make_quat_from_axis_angle(axis: [f32; 3], radians: f32) -> [f32; 4] {
    let (c, s) = b3_compute_cos_sin(0.5 * radians);
    [axis[0] * s, axis[1] * s, axis[2] * s, c]
}

pub fn b3_default_revolute_joint_def() -> RevoluteJointDef {
    RevoluteJointDef {
        body_a: b3_null_body_id(),
        body_b: b3_null_body_id(),
        local_anchor_a: [0.0; 3],
        local_anchor_b: [0.0; 3],
        local_rotation_a: [0.0, 0.0, 0.0, 1.0],
        local_rotation_b: [0.0, 0.0, 0.0, 1.0],
        hertz: 60.0,
        damping: 2.0,
        force_threshold: f32::MAX,
        torque_threshold: f32::MAX,
        user_data: 0,
        collide_connected: false,
        target_angle: 0.0,
        enable_spring: false,
        spring_hertz: 0.0,
        spring_damping: 0.0,
        enable_limit: false,
        lower_angle: 0.0,
        upper_angle: 0.0,
        enable_motor: false,
        max_motor_torque: 0.0,
        motor_speed: 0.0,
    }
}

pub fn b3_default_spherical_joint_def() -> SphericalJointDef {
    SphericalJointDef {
        body_a: b3_null_body_id(),
        body_b: b3_null_body_id(),
        local_anchor_a: [0.0; 3],
        local_anchor_b: [0.0; 3],
        local_rotation_a: [0.0, 0.0, 0.0, 1.0],
        local_rotation_b: [0.0, 0.0, 0.0, 1.0],
        hertz: 60.0,
        damping: 2.0,
        force_threshold: f32::MAX,
        torque_threshold: f32::MAX,
        user_data: 0,
        collide_connected: false,
        enable_spring: false,
        spring_hertz: 0.0,
        spring_damping: 0.0,
        target_rotation: [0.0, 0.0, 0.0, 1.0],
        enable_cone_limit: false,
        cone_angle: 0.0,
        enable_twist_limit: false,
        lower_twist_angle: 0.0,
        upper_twist_angle: 0.0,
        enable_motor: false,
        max_motor_torque: 0.0,
        motor_velocity: [0.0; 3],
    }
}

pub fn b3_default_prismatic_joint_def() -> PrismaticJointDef {
    PrismaticJointDef {
        body_a: b3_null_body_id(),
        body_b: b3_null_body_id(),
        local_anchor_a: [0.0; 3],
        local_anchor_b: [0.0; 3],
        local_axis_a: [1.0, 0.0, 0.0],
        local_rotation_a: [0.0, 0.0, 0.0, 1.0],
        local_rotation_b: [0.0, 0.0, 0.0, 1.0],
        hertz: 60.0,
        damping: 2.0,
        force_threshold: f32::MAX,
        torque_threshold: f32::MAX,
        user_data: 0,
        collide_connected: false,
        enable_spring: false,
        spring_hertz: 0.0,
        spring_damping: 0.0,
        target_translation: 0.0,
        enable_limit: false,
        lower_translation: 0.0,
        upper_translation: 0.0,
        enable_motor: false,
        max_motor_force: 0.0,
        motor_speed: 0.0,
    }
}

pub fn b3_default_distance_joint_def() -> DistanceJointDef {
    DistanceJointDef {
        body_a: b3_null_body_id(),
        body_b: b3_null_body_id(),
        local_anchor_a: [0.0; 3],
        local_anchor_b: [0.0; 3],
        hertz: 60.0,
        damping: 2.0,
        force_threshold: f32::MAX,
        torque_threshold: f32::MAX,
        user_data: 0,
        collide_connected: false,
        length: 1.0,
        enable_spring: false,
        lower_spring_force: -f32::MAX,
        upper_spring_force: f32::MAX,
        spring_hertz: 0.0,
        spring_damping: 0.0,
        enable_limit: false,
        min_length: 0.0,
        max_length: 1.0e5,
        enable_motor: false,
        max_motor_force: 0.0,
        motor_speed: 0.0,
    }
}

pub fn b3_default_parallel_joint_def() -> ParallelJointDef {
    ParallelJointDef {
        body_a: b3_null_body_id(),
        body_b: b3_null_body_id(),
        local_rotation_a: [0.0, 0.0, 0.0, 1.0],
        local_rotation_b: [0.0, 0.0, 0.0, 1.0],
        hertz: 1.0,
        damping: 1.0,
        force_threshold: f32::MAX,
        torque_threshold: f32::MAX,
        user_data: 0,
        collide_connected: false,
        max_torque: f32::MAX,
    }
}

pub fn b3_default_filter_joint_def() -> FilterJointDef {
    FilterJointDef {
        body_a: b3_null_body_id(),
        body_b: b3_null_body_id(),
        force_threshold: f32::MAX,
        torque_threshold: f32::MAX,
        user_data: 0,
        collide_connected: false,
    }
}

pub fn b3_default_motor_joint_def() -> MotorJointDef {
    MotorJointDef {
        body_a: b3_null_body_id(),
        body_b: b3_null_body_id(),
        local_anchor_a: [0.0; 3],
        local_anchor_b: [0.0; 3],
        local_rotation_a: [0.0, 0.0, 0.0, 1.0],
        local_rotation_b: [0.0, 0.0, 0.0, 1.0],
        linear_velocity: [0.0; 3],
        max_velocity_force: 0.0,
        angular_velocity: [0.0; 3],
        max_velocity_torque: 0.0,
        linear_hertz: 0.0,
        linear_damping: 0.0,
        max_spring_force: 0.0,
        angular_hertz: 0.0,
        angular_damping: 0.0,
        max_spring_torque: 0.0,
        force_threshold: f32::MAX,
        torque_threshold: f32::MAX,
        user_data: 0,
        collide_connected: false,
    }
}

pub fn b3_default_weld_joint_def() -> WeldJointDef {
    WeldJointDef {
        body_a: b3_null_body_id(),
        body_b: b3_null_body_id(),
        local_anchor_a: [0.0; 3],
        local_anchor_b: [0.0; 3],
        local_rotation_a: [0.0, 0.0, 0.0, 1.0],
        local_rotation_b: [0.0, 0.0, 0.0, 1.0],
        hertz: 60.0,
        damping: 2.0,
        force_threshold: f32::MAX,
        torque_threshold: f32::MAX,
        user_data: 0,
        collide_connected: false,
        linear_hertz: 0.0,
        linear_damping_ratio: 0.0,
        angular_hertz: 0.0,
        angular_damping_ratio: 0.0,
    }
}

pub fn b3_default_wheel_joint_def() -> WheelJointDef {
    WheelJointDef {
        body_a: b3_null_body_id(),
        body_b: b3_null_body_id(),
        local_anchor_a: [0.0; 3],
        local_anchor_b: [0.0; 3],
        local_rotation_a: [0.0, 0.0, 0.0, 1.0],
        local_rotation_b: [0.0, 0.0, 0.0, 1.0],
        hertz: 60.0,
        damping: 2.0,
        force_threshold: f32::MAX,
        torque_threshold: f32::MAX,
        user_data: 0,
        collide_connected: false,
        enable_suspension_spring: true,
        suspension_hertz: 1.0,
        suspension_damping_ratio: 0.7,
        enable_suspension_limit: false,
        lower_suspension_limit: 0.0,
        upper_suspension_limit: 0.0,
        enable_spin_motor: false,
        max_spin_torque: 0.0,
        spin_speed: 0.0,
        enable_steering: false,
        steering_hertz: 1.0,
        steering_damping_ratio: 0.7,
        target_steering_angle: 0.0,
        max_steering_torque: 0.0,
        enable_steering_limit: false,
        lower_steering_limit: 0.0,
        upper_steering_limit: 0.0,
    }
}
