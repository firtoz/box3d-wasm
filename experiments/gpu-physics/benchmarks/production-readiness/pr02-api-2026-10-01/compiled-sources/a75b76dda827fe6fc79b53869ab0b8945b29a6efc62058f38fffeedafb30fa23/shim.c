// Thin C wrappers so a Box3D sample can include box3d.h and link this crate instead of libbox3d.

#include "box3d/box3d.h"
#include "box3d/collision.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "native_clock.h"
#include "growable_slots.h"

/* Separate per-compound resource guard, not a world shape-index limit.
   Shared mesh instances avoid duplicating geometry for each compound child. */
#define GPU_MAX_COMPOUND_CHILDREN 65535

static int gpu_compound_child_limit(void)
{
    return GPU_MAX_COMPOUND_CHILDREN;
}

static int gpu_compound_count(int capsules, int hulls, int meshes, int spheres)
{
    if (capsules < 0 || hulls < 0 || meshes < 0 || spheres < 0) return -1;
    int64_t count = (int64_t)capsules + hulls + meshes + spheres;
    return count > GPU_MAX_COMPOUND_CHILDREN ? GPU_MAX_COMPOUND_CHILDREN + 1 : (int)count;
}

extern b3CompoundData* __real_b3CreateCompound(const b3CompoundDef* def);
extern void gpu_b3_world_set_fail(b3WorldId id, const char* message);

static void refuse_compound(int children)
{
    // Geometry cooking has no owning world. The rejected version marker makes
    // a later shape import fail its actual world, without poisoning another one.
    fprintf(stderr, "\n*** GPU PHYSICS FAILED ***\nrefusing compound with %d children (limit %d)\n",
            children, gpu_compound_child_limit());
}

B3_API b3CompoundData* b3CreateCompound(const b3CompoundDef* def)
{
	int children = 0;
	if (def != NULL)
	{
		children = gpu_compound_count(def->capsuleCount, def->hullCount, def->meshCount, def->sphereCount);
	}
	if (children < 0 || children > gpu_compound_child_limit())
	{
		refuse_compound(children);
		/* Preserve a destroyable allocation, but prevent both backends from
           importing the rejected data as a successful empty compound. */
		b3CompoundDef empty = {0};
		b3CompoundData* rejected = __real_b3CreateCompound(&empty);
		if (rejected != NULL) rejected->version = 0;
		return rejected;
	}
	return __real_b3CreateCompound(def);
}

static float g_gpu_step_ms;

typedef struct GpuHullMirror
{
	b3HullData* data;
	int32_t index1;
	uint16_t world0;
	int32_t parent;
} GpuHullMirror;


typedef struct GpuMeshMirror
{
	const b3MeshData* data;
	b3Vec3 scale;
	b3SurfaceMaterial* materials;
	int materialCount;
	int32_t index1;
	uint16_t world0;
	int32_t parent;
} GpuMeshMirror;


typedef struct GpuHeightFieldMirror
{
	const b3HeightFieldData* data;
	b3SurfaceMaterial* materials;
	int materialCount;
	int32_t index1;
	uint16_t world0;
} GpuHeightFieldMirror;


typedef struct GpuGeometryMirror
{
    GpuHullMirror hull;
    GpuMeshMirror mesh;
    GpuHeightFieldMirror height;
} GpuGeometryMirror;
static GpuSlots g_geometry[GPU_METADATA_WORLDS];

static GpuGeometryMirror* geometry_slot(b3ShapeId id, bool create)
{
    if (id.world0 == 0 || id.world0 >= GPU_METADATA_WORLDS) return NULL;
    return (GpuGeometryMirror*)gpu_slots_get(&g_geometry[id.world0], id.index1, sizeof(GpuGeometryMirror), create);
}

static GpuHeightFieldMirror* find_height_field_mirror(b3ShapeId id, bool create)
{
    GpuGeometryMirror* slot = geometry_slot(id, create);
    if (!slot || (!create && slot->height.index1 == 0)) return NULL;
    return &slot->height;
}

static GpuMeshMirror* find_mesh_mirror(b3ShapeId id, bool create)
{
    GpuGeometryMirror* slot = geometry_slot(id, create);
    if (!slot || (!create && slot->mesh.index1 == 0)) return NULL;
    return &slot->mesh;
}

static GpuHullMirror* find_hull_mirror(b3ShapeId id, bool create)
{
    GpuGeometryMirror* slot = geometry_slot(id, create);
    if (!slot || (!create && slot->hull.index1 == 0)) return NULL;
    return &slot->hull;
}


void gpu_shape_mirror_parent(b3ShapeId child, b3ShapeId parent)
{
    GpuHullMirror* hull = find_hull_mirror(child, false);
    if (hull) { hull->parent = parent.index1; }
    GpuMeshMirror* mesh = find_mesh_mirror(child, false);
    if (mesh) { mesh->parent = parent.index1; }
}

static b3HullData* clone_hull_blob(const b3HullData* hull)
{
	if (hull == NULL)
	{
		return NULL;
	}
	// byteCount includes face data and explicit identity padding.
	size_t size = (size_t)hull->byteCount;
	b3HullData* copy = (b3HullData*)malloc(size);
	if (copy != NULL)
	{
		memcpy(copy, hull, size);
	}
	return copy;
}

void gpu_shape_mirror_hull(b3ShapeId id, const b3HullData* hull)
{
	if (id.index1 <= 0)
	{
		return;
	}
	GpuHullMirror* mirror = find_hull_mirror(id, true);
	if (mirror == NULL)
	{
		return;
	}
	b3HullData* copy = clone_hull_blob(hull);
	free(mirror->data);
	mirror->data = copy;
	mirror->index1 = id.index1;
	mirror->world0 = id.world0;
}

static double monotonic_milliseconds(void)
{
	return (double)gpu_monotonic_ns() * 1.0e-6;
}






typedef struct b3WorldId GpuWorldId;
typedef struct b3BodyId GpuBodyId;
typedef struct b3ShapeId GpuShapeId;
typedef struct b3JointId GpuJointId;

_Static_assert(sizeof(b3QueryFilter) == 32, "Rust/C b3QueryFilter ABI mismatch");
_Static_assert(sizeof(b3ShapeProxy) == 16, "Rust/C b3ShapeProxy ABI mismatch");
_Static_assert(sizeof(b3TreeStats) == 8, "Rust/C b3TreeStats ABI mismatch");
_Static_assert(sizeof(b3WorldCastOutput) == 48, "Rust/C b3WorldCastOutput ABI mismatch");
_Static_assert(sizeof(b3BodyCastResult) == 56, "Rust/C b3BodyCastResult ABI mismatch");
_Static_assert(sizeof(b3RayResult) == 64, "Rust/C b3RayResult ABI mismatch");
_Static_assert(sizeof(b3ExplosionDef) == 32, "Rust/C b3ExplosionDef ABI mismatch");

extern GpuWorldId gpu_b3_create_world_with_capacity(float gx, float gy, float gz,
    int static_bodies, int dynamic_bodies, int static_shapes, int dynamic_shapes);
extern void gpu_b3_world_set_restitution_threshold(GpuWorldId, float);
extern float gpu_b3_world_get_restitution_threshold(GpuWorldId);
extern void gpu_b3_world_set_maximum_linear_speed(GpuWorldId, float);
extern float gpu_b3_world_get_maximum_linear_speed(GpuWorldId);
extern void gpu_b3_destroy_world(GpuWorldId id);
extern bool gpu_b3_world_is_valid(GpuWorldId id);
extern void gpu_b3_world_step(GpuWorldId id, float dt, int sub_step_count);
extern void gpu_b3_world_wait(GpuWorldId id);
extern float gpu_b3_world_last_encode_ms(GpuWorldId id);
extern float gpu_b3_world_last_fetch_ms(GpuWorldId id);
extern void gpu_b3_world_set_custom_filter_callback(GpuWorldId id, b3CustomFilterFcn* callback, void* context);
extern void gpu_b3_world_set_pre_solve_callback(GpuWorldId id, b3PreSolveFcn* callback, void* context);
extern void gpu_b3_world_explode(GpuWorldId id, uint64_t mask_bits, float x, float y, float z, float radius,
								 float falloff, float impulse_per_area);
extern void gpu_b3_world_get_contact_events(GpuWorldId id, const b3ContactBeginTouchEvent** begin_events,
											int* begin_count, const b3ContactEndTouchEvent** end_events,
											int* end_count, const b3ContactHitEvent** hit_events, int* hit_count);
extern void gpu_b3_world_get_body_events(GpuWorldId id, const b3BodyMoveEvent** events, int* count);
extern void gpu_b3_world_get_joint_events(GpuWorldId id, const b3JointEvent** events, int* count);
extern void gpu_b3_world_set_hit_event_threshold(GpuWorldId id, float value);
extern float gpu_b3_world_get_hit_event_threshold(GpuWorldId id);
extern void gpu_b3_world_get_sensor_events(GpuWorldId id, const b3SensorBeginTouchEvent** begin_events,
										   int* begin_count, const b3SensorEndTouchEvent** end_events,
										   int* end_count);
extern void gpu_b3_world_enable_sleeping(GpuWorldId id, bool enable);
extern bool gpu_b3_world_is_sleeping_enabled(GpuWorldId id);
extern void gpu_b3_world_set_gravity(GpuWorldId id, float x, float y, float z);
extern b3Vec3 gpu_b3_world_get_gravity(GpuWorldId id);
extern void gpu_b3_world_enable_continuous(GpuWorldId id, bool enable);
extern bool gpu_b3_world_is_continuous_enabled(GpuWorldId id);
extern void gpu_b3_world_set_friction_callback(GpuWorldId id, b3FrictionCallback* callback);
extern void gpu_b3_world_set_restitution_callback(GpuWorldId id, b3RestitutionCallback* callback);
extern GpuBodyId gpu_b3_create_body(GpuWorldId world, int body_type, float px, float py, float pz, float rx,
								   float ry, float rz, float rw, float vx, float vy, float vz, float wx, float wy,
								   float wz, float gravity_scale, uint32_t locks);
extern void gpu_b3_body_set_bullet(GpuBodyId body, bool bullet);
extern bool gpu_b3_body_is_bullet(GpuBodyId body);
extern void gpu_b3_body_allow_fast_rotation(GpuBodyId body, bool allow);
extern bool gpu_b3_body_is_fast_rotation_allowed(GpuBodyId body);
extern GpuShapeId gpu_b3_create_sphere(GpuBodyId body, float center_x, float center_y, float center_z, float radius,
									  float density, float friction, float restitution, float rolling,
									  bool update_body_mass);
extern GpuShapeId gpu_b3_create_hull(GpuBodyId body, float hx, float hy, float hz, float ox, float oy, float oz,
									float density, float friction, float restitution, float rolling, bool update_body_mass);
extern GpuShapeId gpu_b3_create_convex_hull(
	GpuBodyId body, const float* points, int point_count, const float* planes, int plane_count, const uint8_t* edges,
	int half_edge_count, float hx, float hy, float hz, float ax, float ay, float az, float cx, float cy, float cz,
	float inner_radius, float volume, float ixx, float iyy, float izz, float ixy, float ixz, float iyz, float density,
	float friction, float restitution, float rolling, bool update_body_mass);
extern GpuShapeId gpu_b3_create_capsule(GpuBodyId body, float x1, float y1, float z1, float x2, float y2, float z2,
									   float radius, float density, float friction, float restitution, float rolling,
									   bool update_body_mass);
extern GpuShapeId gpu_b3_create_mesh(
	GpuBodyId body, const float* vertices, int vertex_count, const int32_t* triangles, int triangle_count,
	const uint8_t* flags, const uint8_t* materials, const void* nodes, int node_count, float scale_x, float scale_y,
	float scale_z, float density, float friction, float restitution, float rolling, bool update_body_mass);
extern GpuShapeId gpu_b3_create_compound_parent(GpuBodyId body, float density, float friction, float restitution,
											   float rolling, bool is_sensor);
extern bool gpu_b3_shape_attach_compound_child(GpuShapeId parent, GpuShapeId child, int32_t index);
extern bool gpu_b3_shape_attach_compound_child_materials(GpuShapeId parent, GpuShapeId child, int32_t index, const int32_t* materials);
extern GpuShapeId gpu_b3_create_height_field(
	GpuBodyId body, const uint16_t* heights, int column_count, int row_count, float min_height, float height_scale,
	float scale_x, float scale_y, float scale_z, const uint8_t* materials, const uint8_t* flags, bool clockwise,
	float density, float friction, float restitution, float rolling);
extern bool gpu_b3_replace_height_field(
	GpuShapeId id, const uint16_t* heights, int column_count, int row_count, float min_height, float height_scale,
	float scale_x, float scale_y, float scale_z, const uint8_t* materials, const uint8_t* flags, bool clockwise);
extern bool gpu_b3_replace_mesh(
	GpuShapeId id, const float* vertices, int vertex_count, const int32_t* triangles, int triangle_count,
	const uint8_t* flags, const uint8_t* materials, const void* nodes, int node_count, float scale_x, float scale_y,
	float scale_z);
extern void gpu_b3_shape_set_filter(GpuShapeId id, uint64_t category_bits, uint64_t mask_bits, int group_index,
									bool invoke_contacts);
extern void gpu_b3_shape_set_surface_material(GpuShapeId id, b3SurfaceMaterial material);
extern b3SurfaceMaterial gpu_b3_shape_get_surface_material(GpuShapeId id);
extern void gpu_b3_shape_set_mesh_material_count(GpuShapeId id, int count);
extern void gpu_b3_shape_set_mesh_material(GpuShapeId id, int index, b3SurfaceMaterial material);
extern b3SurfaceMaterial gpu_b3_shape_get_mesh_material(GpuShapeId id, int index);
extern void gpu_b3_shape_get_filter(GpuShapeId id, uint64_t* category_bits, uint64_t* mask_bits, int* group_index);
extern b3QueryFilter gpu_b3_default_query_filter(void);
extern b3Sphere gpu_b3_shape_get_sphere(GpuShapeId id);
extern b3Capsule gpu_b3_shape_get_capsule(GpuShapeId id);
extern float gpu_b3_shape_get_density(GpuShapeId id);
extern float gpu_b3_shape_get_friction(GpuShapeId id);
extern float gpu_b3_shape_get_restitution(GpuShapeId id);
extern void gpu_b3_shape_set_explosion_scale(GpuShapeId id, float scale);
extern void gpu_b3_shape_set_user_data(GpuShapeId id, uintptr_t value);
extern uintptr_t gpu_b3_shape_get_user_data(GpuShapeId id);
extern b3AABB gpu_b3_shape_get_aabb(GpuShapeId id);
extern b3AABB gpu_b3_body_compute_aabb(GpuBodyId id);
extern b3Vec3 gpu_b3_shape_get_closest_point(GpuShapeId id, b3Vec3 target);
extern float gpu_b3_body_get_closest_point(GpuBodyId id, b3Vec3* result, b3Vec3 target);
extern b3WorldCastOutput gpu_b3_shape_ray_cast(GpuShapeId id, b3Pos origin, b3Vec3 translation);
extern b3BodyCastResult gpu_b3_body_cast_ray(GpuBodyId id, b3Pos origin, b3Vec3 translation, b3QueryFilter filter,
											 float max_fraction, b3WorldTransform transform);
extern b3BodyCastResult gpu_b3_body_cast_shape(GpuBodyId id, b3Pos origin, const b3ShapeProxy* proxy,
											   b3Vec3 translation, b3QueryFilter filter, float max_fraction,
											   bool can_encroach, b3WorldTransform transform);
extern bool gpu_b3_body_overlap_shape(GpuBodyId id, b3Pos origin, const b3ShapeProxy* proxy, b3QueryFilter filter,
									  b3WorldTransform transform);
extern b3TreeStats gpu_b3_world_overlap_aabb(GpuWorldId id, b3AABB aabb, b3QueryFilter filter,
											 b3OverlapResultFcn* callback, void* context);
extern b3TreeStats gpu_b3_world_overlap_shape(GpuWorldId id, b3Pos origin, const b3ShapeProxy* proxy,
											  b3QueryFilter filter, b3OverlapResultFcn* callback, void* context);
extern b3TreeStats gpu_b3_world_cast_ray(GpuWorldId id, b3Pos origin, b3Vec3 translation, b3QueryFilter filter,
										 b3CastResultFcn* callback, void* context);
extern b3RayResult gpu_b3_world_cast_ray_closest(GpuWorldId id, b3Pos origin, b3Vec3 translation,
												 b3QueryFilter filter);
extern b3TreeStats gpu_b3_world_cast_shape(GpuWorldId id, b3Pos origin, const b3ShapeProxy* proxy,
										   b3Vec3 translation, b3QueryFilter filter, b3CastResultFcn* callback,
										   void* context);
extern float gpu_b3_world_cast_mover(GpuWorldId id, b3Pos origin, const b3Capsule* mover, b3Vec3 translation,
									b3QueryFilter filter, b3MoverFilterFcn* callback, void* context);
extern void gpu_b3_world_collide_mover(GpuWorldId id, b3Pos origin, const b3Capsule* mover, b3QueryFilter filter,
									  b3PlaneResultFcn* callback, void* context);
extern int gpu_b3_body_collide_mover(GpuBodyId id, b3BodyPlaneResult* planes, int capacity, b3Pos origin,
									const b3Capsule* mover, b3QueryFilter filter, b3WorldTransform transform);
extern b3PlaneSolverResult gpu_b3_solve_planes(b3Vec3 target_delta, b3CollisionPlane* planes, int count);
extern b3Vec3 gpu_b3_clip_vector(b3Vec3 vector, const b3CollisionPlane* planes, int count);
extern void gpu_b3_shape_enable_contact_events(GpuShapeId id, bool enable);
extern void gpu_b3_shape_enable_speculative_contact(GpuShapeId id, bool enable);
extern bool gpu_b3_shape_are_contact_events_enabled(GpuShapeId id);
extern void gpu_b3_shape_enable_custom_filtering(GpuShapeId id, bool enable);
extern bool gpu_b3_shape_is_custom_filtering_enabled(GpuShapeId id);
extern void gpu_b3_shape_enable_pre_solve_events(GpuShapeId id, bool enable);
extern bool gpu_b3_shape_are_pre_solve_events_enabled(GpuShapeId id);
extern bool gpu_b3_shape_is_sensor(GpuShapeId id);
extern void gpu_b3_shape_set_sensor(GpuShapeId id, bool is_sensor);
extern void gpu_b3_shape_enable_sensor_events(GpuShapeId id, bool enable);
extern bool gpu_b3_shape_are_sensor_events_enabled(GpuShapeId id);
extern int gpu_b3_shape_get_sensor_capacity(GpuShapeId id);
extern int gpu_b3_shape_get_sensor_data(GpuShapeId id, GpuShapeId* output, int capacity);
extern void gpu_b3_destroy_shape(GpuShapeId id, bool update_body_mass);
extern GpuJointId gpu_b3_create_revolute(GpuWorldId world, GpuBodyId a, GpuBodyId b, float ax, float ay, float az,
										float bx, float by, float bz, float qa_x, float qa_y, float qa_z, float qa_w,
										float qb_x, float qb_y, float qb_z, float qb_w, float hertz, float damping,
										bool collide_connected, float target_angle, bool enable_spring, float spring_hertz,
										float spring_damping, bool enable_limit, float lower_angle, float upper_angle,
										bool enable_motor, float max_motor_torque, float motor_speed);
extern void gpu_b3_joint_set_constraint_tuning(GpuJointId id, float hertz, float damping);
extern void gpu_b3_joint_get_constraint_tuning(GpuJointId id, float* hertz, float* damping);
extern void gpu_b3_joint_set_force_threshold(GpuJointId id, float threshold);
extern float gpu_b3_joint_get_force_threshold(GpuJointId id);
extern void gpu_b3_joint_set_torque_threshold(GpuJointId id, float threshold);
extern float gpu_b3_joint_get_torque_threshold(GpuJointId id);
extern void gpu_b3_joint_set_user_data(GpuJointId id, uintptr_t user_data);
extern uintptr_t gpu_b3_joint_get_user_data(GpuJointId id);
extern void gpu_b3_revolute_enable_spring(GpuJointId id, bool enable);
extern bool gpu_b3_revolute_is_spring_enabled(GpuJointId id);
extern void gpu_b3_revolute_set_spring_hertz(GpuJointId id, float value);
extern float gpu_b3_revolute_get_spring_hertz(GpuJointId id);
extern void gpu_b3_revolute_set_spring_damping(GpuJointId id, float value);
extern float gpu_b3_revolute_get_spring_damping(GpuJointId id);
extern void gpu_b3_revolute_set_target_angle(GpuJointId id, float value);
extern float gpu_b3_revolute_get_target_angle(GpuJointId id);
extern float gpu_b3_revolute_get_angle(GpuJointId id);
extern void gpu_b3_revolute_enable_limit(GpuJointId id, bool enable);
extern bool gpu_b3_revolute_is_limit_enabled(GpuJointId id);
extern float gpu_b3_revolute_get_lower_limit(GpuJointId id);
extern float gpu_b3_revolute_get_upper_limit(GpuJointId id);
extern void gpu_b3_revolute_set_limits(GpuJointId id, float lower, float upper);
extern void gpu_b3_revolute_enable_motor(GpuJointId id, bool enable);
extern bool gpu_b3_revolute_is_motor_enabled(GpuJointId id);
extern void gpu_b3_revolute_set_motor_speed(GpuJointId id, float value);
extern float gpu_b3_revolute_get_motor_speed(GpuJointId id);
extern float gpu_b3_revolute_get_motor_torque(GpuJointId id);
extern void gpu_b3_revolute_set_max_motor_torque(GpuJointId id, float value);
extern float gpu_b3_revolute_get_max_motor_torque(GpuJointId id);
extern GpuJointId gpu_b3_create_wheel(
	GpuWorldId world, GpuBodyId a, GpuBodyId b, float ax, float ay, float az, float bx, float by, float bz, float qa_x,
	float qa_y, float qa_z, float qa_w, float qb_x, float qb_y, float qb_z, float qb_w, float hertz, float damping,
	bool collide_connected, bool enable_suspension, float suspension_hertz, float suspension_damping,
	bool enable_suspension_limit, float lower_suspension, float upper_suspension, bool enable_spin_motor,
	float max_spin_torque, float spin_speed, bool enable_steering, float steering_hertz, float steering_damping,
	float target_steering, float max_steering_torque, bool enable_steering_limit, float lower_steering,
	float upper_steering);
extern void gpu_b3_wheel_enable_suspension(GpuJointId id, bool value);
extern bool gpu_b3_wheel_is_suspension_enabled(GpuJointId id);
extern void gpu_b3_wheel_set_suspension_hertz(GpuJointId id, float value);
extern float gpu_b3_wheel_get_suspension_hertz(GpuJointId id);
extern void gpu_b3_wheel_set_suspension_damping(GpuJointId id, float value);
extern float gpu_b3_wheel_get_suspension_damping(GpuJointId id);
extern void gpu_b3_wheel_enable_suspension_limit(GpuJointId id, bool value);
extern bool gpu_b3_wheel_is_suspension_limit_enabled(GpuJointId id);
extern float gpu_b3_wheel_get_lower_suspension_limit(GpuJointId id);
extern float gpu_b3_wheel_get_upper_suspension_limit(GpuJointId id);
extern void gpu_b3_wheel_set_suspension_limits(GpuJointId id, float lower, float upper);
extern void gpu_b3_wheel_enable_spin_motor(GpuJointId id, bool value);
extern bool gpu_b3_wheel_is_spin_motor_enabled(GpuJointId id);
extern void gpu_b3_wheel_set_spin_speed(GpuJointId id, float value);
extern float gpu_b3_wheel_get_spin_speed_setting(GpuJointId id);
extern void gpu_b3_wheel_set_max_spin_torque(GpuJointId id, float value);
extern float gpu_b3_wheel_get_max_spin_torque(GpuJointId id);
extern float gpu_b3_wheel_get_live_spin_speed(GpuJointId id);
extern float gpu_b3_wheel_get_spin_torque(GpuJointId id);
extern void gpu_b3_wheel_enable_steering(GpuJointId id, bool value);
extern bool gpu_b3_wheel_is_steering_enabled(GpuJointId id);
extern void gpu_b3_wheel_set_steering_hertz(GpuJointId id, float value);
extern float gpu_b3_wheel_get_steering_hertz(GpuJointId id);
extern void gpu_b3_wheel_set_steering_damping(GpuJointId id, float value);
extern float gpu_b3_wheel_get_steering_damping(GpuJointId id);
extern void gpu_b3_wheel_set_max_steering_torque(GpuJointId id, float value);
extern float gpu_b3_wheel_get_max_steering_torque(GpuJointId id);
extern void gpu_b3_wheel_enable_steering_limit(GpuJointId id, bool value);
extern bool gpu_b3_wheel_is_steering_limit_enabled(GpuJointId id);
extern float gpu_b3_wheel_get_lower_steering_limit(GpuJointId id);
extern float gpu_b3_wheel_get_upper_steering_limit(GpuJointId id);
extern void gpu_b3_wheel_set_steering_limits(GpuJointId id, float lower, float upper);
extern void gpu_b3_wheel_set_target_steering(GpuJointId id, float value);
extern float gpu_b3_wheel_get_target_steering(GpuJointId id);
extern float gpu_b3_wheel_get_steering_angle(GpuJointId id);
extern float gpu_b3_wheel_get_steering_torque(GpuJointId id);
extern GpuJointId gpu_b3_create_spherical(GpuWorldId world, GpuBodyId a, GpuBodyId b, float ax, float ay, float az,
										  float bx, float by, float bz, float qa_x, float qa_y, float qa_z, float qa_w,
										  float qb_x, float qb_y, float qb_z, float qb_w, float hertz, float damping,
										  bool collide_connected, bool enable_spring, float spring_hertz,
										  float spring_damping, float target_x, float target_y, float target_z,
										  float target_w, bool enable_cone_limit, float cone_angle,
										  bool enable_twist_limit, float lower_twist_angle, float upper_twist_angle,
										  bool enable_motor, float max_motor_torque, float motor_x, float motor_y,
										  float motor_z);
extern void gpu_b3_spherical_enable_cone_limit(GpuJointId id, bool enable);
extern bool gpu_b3_spherical_is_cone_limit_enabled(GpuJointId id);
extern void gpu_b3_spherical_set_cone_limit(GpuJointId id, float value);
extern float gpu_b3_spherical_get_cone_limit(GpuJointId id);
extern float gpu_b3_spherical_get_cone_angle(GpuJointId id);
extern void gpu_b3_spherical_enable_twist_limit(GpuJointId id, bool enable);
extern bool gpu_b3_spherical_is_twist_limit_enabled(GpuJointId id);
extern float gpu_b3_spherical_get_lower_twist_limit(GpuJointId id);
extern float gpu_b3_spherical_get_upper_twist_limit(GpuJointId id);
extern void gpu_b3_spherical_set_twist_limits(GpuJointId id, float lower, float upper);
extern float gpu_b3_spherical_get_twist_angle(GpuJointId id);
extern void gpu_b3_spherical_enable_spring(GpuJointId id, bool enable);
extern bool gpu_b3_spherical_is_spring_enabled(GpuJointId id);
extern void gpu_b3_spherical_set_spring_hertz(GpuJointId id, float value);
extern float gpu_b3_spherical_get_spring_hertz(GpuJointId id);
extern void gpu_b3_spherical_set_spring_damping(GpuJointId id, float value);
extern float gpu_b3_spherical_get_spring_damping(GpuJointId id);
extern void gpu_b3_spherical_set_target_rotation(GpuJointId id, float x, float y, float z, float w);
extern void gpu_b3_spherical_get_target_rotation(GpuJointId id, float* output);
extern void gpu_b3_spherical_enable_motor(GpuJointId id, bool enable);
extern bool gpu_b3_spherical_is_motor_enabled(GpuJointId id);
extern void gpu_b3_spherical_set_motor_velocity(GpuJointId id, float x, float y, float z);
extern void gpu_b3_spherical_get_motor_velocity(GpuJointId id, float* output);
extern void gpu_b3_spherical_get_motor_torque(GpuJointId id, float* output);
extern void gpu_b3_spherical_set_max_motor_torque(GpuJointId id, float value);
extern float gpu_b3_spherical_get_max_motor_torque(GpuJointId id);
extern GpuJointId gpu_b3_create_prismatic(GpuWorldId world, GpuBodyId a, GpuBodyId b, float ax, float ay, float az,
										 float bx, float by, float bz, float axis_x, float axis_y, float axis_z,
										 float qa_x, float qa_y, float qa_z, float qa_w, float qb_x, float qb_y,
										 float qb_z, float qb_w, float hertz, float damping, bool collide_connected,
										 bool enable_spring,
										 float spring_hertz, float spring_damping, float target_translation,
										 bool enable_limit, float lower_translation, float upper_translation,
										 bool enable_motor, float max_motor_force, float motor_speed);
extern GpuJointId gpu_b3_create_filter(GpuWorldId world, GpuBodyId a, GpuBodyId b, bool collide_connected);
extern GpuJointId gpu_b3_create_distance(
	GpuWorldId world, GpuBodyId a, GpuBodyId b, float ax, float ay, float az, float bx, float by, float bz,
	float hertz, float damping, bool collide_connected, float length, bool enable_spring, float lower_spring_force,
	float upper_spring_force, float spring_hertz, float spring_damping, bool enable_limit, float min_length,
	float max_length, bool enable_motor, float max_motor_force, float motor_speed);
extern void gpu_b3_distance_set_length(GpuJointId id, float length);
extern float gpu_b3_distance_get_length(GpuJointId id);
extern void gpu_b3_distance_enable_spring(GpuJointId id, bool enable);
extern bool gpu_b3_distance_is_spring_enabled(GpuJointId id);
extern void gpu_b3_distance_set_spring_force_range(GpuJointId id, float lower, float upper);
extern void gpu_b3_distance_get_spring_force_range(GpuJointId id, float* lower, float* upper);
extern void gpu_b3_distance_set_spring_hertz(GpuJointId id, float hertz);
extern float gpu_b3_distance_get_spring_hertz(GpuJointId id);
extern void gpu_b3_distance_set_spring_damping(GpuJointId id, float damping);
extern float gpu_b3_distance_get_spring_damping(GpuJointId id);
extern void gpu_b3_distance_enable_limit(GpuJointId id, bool enable);
extern bool gpu_b3_distance_is_limit_enabled(GpuJointId id);
extern void gpu_b3_distance_set_length_range(GpuJointId id, float min_length, float max_length);
extern float gpu_b3_distance_get_min_length(GpuJointId id);
extern float gpu_b3_distance_get_max_length(GpuJointId id);
extern float gpu_b3_distance_get_current_length(GpuJointId id);
extern void gpu_b3_distance_enable_motor(GpuJointId id, bool enable);
extern bool gpu_b3_distance_is_motor_enabled(GpuJointId id);
extern void gpu_b3_distance_set_motor_speed(GpuJointId id, float speed);
extern float gpu_b3_distance_get_motor_speed(GpuJointId id);
extern void gpu_b3_distance_set_max_motor_force(GpuJointId id, float force);
extern float gpu_b3_distance_get_max_motor_force(GpuJointId id);
extern float gpu_b3_distance_get_motor_force(GpuJointId id);
extern GpuJointId gpu_b3_create_parallel(GpuWorldId world, GpuBodyId a, GpuBodyId b, float qa_x, float qa_y,
										 float qa_z, float qa_w, float qb_x, float qb_y, float qb_z, float qb_w,
										 float hertz, float damping, float max_torque, bool collide_connected);
extern void gpu_b3_parallel_set_spring_hertz(GpuJointId id, float hertz);
extern float gpu_b3_parallel_get_spring_hertz(GpuJointId id);
extern void gpu_b3_parallel_set_spring_damping(GpuJointId id, float damping);
extern float gpu_b3_parallel_get_spring_damping(GpuJointId id);
extern void gpu_b3_parallel_set_max_torque(GpuJointId id, float torque);
extern float gpu_b3_parallel_get_max_torque(GpuJointId id);
extern GpuJointId gpu_b3_create_motor(
	GpuWorldId world, GpuBodyId a, GpuBodyId b, float ax, float ay, float az, float bx, float by, float bz,
	float qa_x, float qa_y, float qa_z, float qa_w, float qb_x, float qb_y, float qb_z, float qb_w, float linear_x,
	float linear_y, float linear_z, float max_velocity_force, float angular_x, float angular_y, float angular_z,
	float max_velocity_torque, float linear_hertz, float linear_damping, float max_spring_force, float angular_hertz,
	float angular_damping, float max_spring_torque, bool collide_connected);
extern void gpu_b3_motor_set_linear_velocity(GpuJointId id, float x, float y, float z);
extern b3Vec3 gpu_b3_motor_get_linear_velocity(GpuJointId id);
extern void gpu_b3_motor_set_angular_velocity(GpuJointId id, float x, float y, float z);
extern b3Vec3 gpu_b3_motor_get_angular_velocity(GpuJointId id);
extern void gpu_b3_motor_set_max_velocity_force(GpuJointId id, float value);
extern float gpu_b3_motor_get_max_velocity_force(GpuJointId id);
extern void gpu_b3_motor_set_max_velocity_torque(GpuJointId id, float value);
extern float gpu_b3_motor_get_max_velocity_torque(GpuJointId id);
extern void gpu_b3_motor_set_linear_hertz(GpuJointId id, float value);
extern float gpu_b3_motor_get_linear_hertz(GpuJointId id);
extern void gpu_b3_motor_set_linear_damping(GpuJointId id, float value);
extern float gpu_b3_motor_get_linear_damping(GpuJointId id);
extern void gpu_b3_motor_set_angular_hertz(GpuJointId id, float value);
extern float gpu_b3_motor_get_angular_hertz(GpuJointId id);
extern void gpu_b3_motor_set_angular_damping(GpuJointId id, float value);
extern float gpu_b3_motor_get_angular_damping(GpuJointId id);
extern void gpu_b3_motor_set_max_spring_force(GpuJointId id, float value);
extern float gpu_b3_motor_get_max_spring_force(GpuJointId id);
extern void gpu_b3_motor_set_max_spring_torque(GpuJointId id, float value);
extern float gpu_b3_motor_get_max_spring_torque(GpuJointId id);
extern void gpu_b3_prismatic_enable_spring(GpuJointId id, bool enable);
extern bool gpu_b3_prismatic_is_spring_enabled(GpuJointId id);
extern void gpu_b3_prismatic_set_spring_hertz(GpuJointId id, float hertz);
extern float gpu_b3_prismatic_get_spring_hertz(GpuJointId id);
extern void gpu_b3_prismatic_set_spring_damping(GpuJointId id, float damping);
extern float gpu_b3_prismatic_get_spring_damping(GpuJointId id);
extern void gpu_b3_prismatic_set_target_translation(GpuJointId id, float translation);
extern float gpu_b3_prismatic_get_target_translation(GpuJointId id);
extern void gpu_b3_prismatic_enable_limit(GpuJointId id, bool enable);
extern bool gpu_b3_prismatic_is_limit_enabled(GpuJointId id);
extern float gpu_b3_prismatic_get_lower_limit(GpuJointId id);
extern float gpu_b3_prismatic_get_upper_limit(GpuJointId id);
extern void gpu_b3_prismatic_set_limits(GpuJointId id, float lower, float upper);
extern void gpu_b3_prismatic_enable_motor(GpuJointId id, bool enable);
extern bool gpu_b3_prismatic_is_motor_enabled(GpuJointId id);
extern void gpu_b3_prismatic_set_motor_speed(GpuJointId id, float speed);
extern float gpu_b3_prismatic_get_motor_speed(GpuJointId id);
extern void gpu_b3_prismatic_set_max_motor_force(GpuJointId id, float force);
extern float gpu_b3_prismatic_get_max_motor_force(GpuJointId id);
extern float gpu_b3_prismatic_get_motor_force(GpuJointId id);
extern float gpu_b3_prismatic_get_translation(GpuJointId id);
extern float gpu_b3_prismatic_get_speed(GpuJointId id);
extern GpuJointId gpu_b3_create_weld(GpuWorldId world, GpuBodyId a, GpuBodyId b, float ax, float ay, float az, float bx,
									float by, float bz, float qa_x, float qa_y, float qa_z, float qa_w, float qb_x,
									float qb_y, float qb_z, float qb_w, float hertz, float damping,
									bool collide_connected, float linear_hertz, float linear_damping,
									float angular_hertz, float angular_damping);
extern void gpu_b3_weld_set_linear_hertz(GpuJointId id, float value);
extern float gpu_b3_weld_get_linear_hertz(GpuJointId id);
extern void gpu_b3_weld_set_linear_damping(GpuJointId id, float value);
extern float gpu_b3_weld_get_linear_damping(GpuJointId id);
extern void gpu_b3_weld_set_angular_hertz(GpuJointId id, float value);
extern float gpu_b3_weld_get_angular_hertz(GpuJointId id);
extern void gpu_b3_weld_set_angular_damping(GpuJointId id, float value);
extern float gpu_b3_weld_get_angular_damping(GpuJointId id);
extern void gpu_b3_body_get_position(GpuBodyId body, float* out);
extern void gpu_b3_body_get_rotation(GpuBodyId body, float* out);
extern void gpu_b3_body_set_linear_velocity(GpuBodyId body, float x, float y, float z);
extern void gpu_b3_body_set_angular_velocity(GpuBodyId body, float x, float y, float z);
extern b3Vec3 gpu_b3_body_get_linear_velocity(GpuBodyId body);
extern b3Vec3 gpu_b3_body_get_angular_velocity(GpuBodyId body);
extern void gpu_b3_body_apply_linear_impulse(GpuBodyId body, float ix, float iy, float iz, float px, float py,
											 float pz, bool wake);
extern void gpu_b3_body_apply_linear_impulse_to_center(GpuBodyId body, float x, float y, float z, bool wake);
extern void gpu_b3_body_apply_angular_impulse(GpuBodyId body, float x, float y, float z, bool wake);
extern void gpu_b3_body_set_transform(GpuBodyId body, float px, float py, float pz, float rx, float ry, float rz,
									 float rw);
extern int gpu_b3_body_get_type(GpuBodyId body);
extern int gpu_b3_body_get_shape_count(GpuBodyId body);
extern int gpu_b3_body_get_shapes(GpuBodyId body, GpuShapeId* out, int capacity);
extern void gpu_b3_body_apply_force(GpuBodyId body, float fx, float fy, float fz, float px, float py, float pz,
									 bool wake);
extern void gpu_b3_body_apply_force_to_center(GpuBodyId body, float x, float y, float z, bool wake);
extern void gpu_b3_body_apply_torque(GpuBodyId body, float x, float y, float z, bool wake);
extern float gpu_b3_body_get_mass(GpuBodyId body);
extern b3MassData gpu_b3_body_get_mass_data(GpuBodyId body);
extern void gpu_b3_body_set_mass_data(GpuBodyId body, b3MassData data);
extern b3Vec3 gpu_b3_body_get_world_point(GpuBodyId body, float x, float y, float z);
extern b3Vec3 gpu_b3_body_get_local_point(GpuBodyId body, float x, float y, float z);
extern b3Vec3 gpu_b3_body_get_world_vector(GpuBodyId body, float x, float y, float z);
extern b3Vec3 gpu_b3_body_get_local_vector(GpuBodyId body, float x, float y, float z);
extern b3Vec3 gpu_b3_body_get_world_point_velocity(GpuBodyId body, float x, float y, float z);
extern b3Vec3 gpu_b3_body_get_local_point_velocity(GpuBodyId body, float x, float y, float z);
extern bool gpu_b3_body_is_awake(GpuBodyId body);
extern bool gpu_b3_body_is_enabled(GpuBodyId body);
extern void gpu_b3_body_set_enabled(GpuBodyId body, bool enable);
extern void gpu_b3_body_set_motion_locks(GpuBodyId body, bool linear_x, bool linear_y, bool linear_z,
										  bool angular_x, bool angular_y, bool angular_z);
extern void gpu_b3_body_get_motion_locks(GpuBodyId body, bool* linear_x, bool* linear_y, bool* linear_z,
										  bool* angular_x, bool* angular_y, bool* angular_z);
extern b3Vec3 gpu_b3_body_get_local_center(GpuBodyId body);
extern float gpu_b3_body_get_linear_damping(GpuBodyId body);
extern float gpu_b3_body_get_angular_damping(GpuBodyId body);
extern float gpu_b3_body_get_gravity_scale(GpuBodyId body);
extern int gpu_b3_body_get_joint_count(GpuBodyId body);
extern int gpu_b3_body_get_joints(GpuBodyId body, GpuJointId* out, int capacity);
extern void gpu_b3_shape_set_friction(GpuShapeId shape, float friction);
extern void gpu_b3_shape_set_restitution(GpuShapeId shape, float restitution);
extern void gpu_b3_shape_apply_wind(GpuShapeId shape, float wx, float wy, float wz, float drag, float lift,
								   float max_speed, bool wake);
extern void gpu_b3_body_set_type(GpuBodyId body, int type);
extern void gpu_b3_body_set_target_transform(GpuBodyId body, float px, float py, float pz, float rx, float ry,
											float rz, float rw, float time_step, bool wake);
extern void gpu_b3_body_set_awake(GpuBodyId body, bool awake);
extern void gpu_b3_body_set_linear_damping(GpuBodyId body, float damping);
extern void gpu_b3_body_set_angular_damping(GpuBodyId body, float damping);
extern void gpu_b3_body_set_gravity_scale(GpuBodyId body, float scale);
extern void gpu_b3_body_get_world_center(GpuBodyId body, float* out);
extern void gpu_b3_body_apply_mass_from_shapes(GpuBodyId body);
extern float gpu_b3_body_inv_mass(GpuBodyId body);
extern void gpu_b3_body_set_user_data(GpuBodyId body, uintptr_t user_data);
extern uintptr_t gpu_b3_body_get_user_data(GpuBodyId body);
extern void gpu_b3_shape_enable_hit_events(GpuShapeId shape, bool enable);
extern bool gpu_b3_shape_are_hit_events_enabled(GpuShapeId shape);
extern void gpu_b3_shape_set_user_material_id(GpuShapeId shape, uint64_t value);

void gpu_samples_on_world_created(b3WorldId world, const b3WorldDef* def) ;
void gpu_samples_on_world_destroyed(b3WorldId world) ;
void gpu_samples_on_shape_created(b3ShapeId shapeId, b3BodyId bodyId, b3ShapeType type, const b3Sphere* sphere,
								  const b3Capsule* capsule, const b3HullData* hull) ;
void gpu_samples_on_mesh_shape_created(b3ShapeId shapeId, b3BodyId bodyId, const b3MeshData* mesh,
									 b3Vec3 scale) ;
void gpu_samples_on_compound_shape_created(b3ShapeId shapeId, b3BodyId bodyId, const b3CompoundData* compound)
	;
void gpu_samples_on_shape_destroyed(b3ShapeId shapeId) ;

















































































































































// Box hull factories come from the upstream hull.c already compiled by build.rs.
// An AABB-only placeholder is not a cooked hull: compound factories copy
// byteCount bytes and transformed shapes require its vertices/planes/topology.

static uint32_t locks_from_def(const b3BodyDef* def)
{
	uint32_t f = 0;
	if (!def->isAwake)
	{
		f |= 4u;
	}
	if (def->enableSleep)
	{
		f |= 8u;
	}
	if (!def->enableContactRecycling)
	{
		f |= (1u << 14);
	}
	if (def->motionLocks.linearX)
	{
		f |= (1u << 8);
	}
	if (def->motionLocks.linearY)
	{
		f |= (1u << 9);
	}
	if (def->motionLocks.linearZ)
	{
		f |= (1u << 10);
	}
	if (def->motionLocks.angularX)
	{
		f |= (1u << 11);
	}
	if (def->motionLocks.angularY)
	{
		f |= (1u << 12);
	}
	if (def->motionLocks.angularZ)
	{
		f |= (1u << 13);
	}
	return f;
}

extern void gpu_b3_world_set_contact_tuning(b3WorldId id, float hertz, float damping, float speed);
extern void gpu_b3_world_set_user_data(b3WorldId id, uintptr_t value);
extern uintptr_t gpu_b3_world_get_user_data(b3WorldId id);
extern int gpu_b3_world_get_awake_body_count(b3WorldId id);
extern void gpu_b3_body_enable_sleep(b3BodyId id, bool enable);
extern bool gpu_b3_body_is_sleep_enabled(b3BodyId id);
extern void gpu_b3_body_set_sleep_threshold(b3BodyId id, float value);
extern float gpu_b3_body_get_sleep_threshold(b3BodyId id);
extern void gpu_b3_body_enable_hit_events(b3BodyId id, bool enable);
extern void gpu_b3_body_set_name(b3BodyId id, const char* name);
extern const char* gpu_b3_body_get_name(b3BodyId id);























void gpu_shape_clear_world_geometry(b3WorldId worldId)
{
    if (worldId.index1 == 0 || worldId.index1 >= GPU_METADATA_WORLDS) return;
    GpuSlots* slots = &g_geometry[worldId.index1];
    for (uint32_t i = 1; i < slots->high_water; ++i)
    {
        GpuGeometryMirror* slot = (GpuGeometryMirror*)gpu_slots_get(slots, (int32_t)i, sizeof(GpuGeometryMirror), false);
        if (!slot) continue;
        free(slot->hull.data);
        free(slot->mesh.materials);
        free(slot->height.materials);
    }
    gpu_slots_release(slots);
}














































































































extern bool gpu_b3_body_is_valid(GpuBodyId body);
extern bool gpu_b3_shape_is_valid(GpuShapeId shape);

























extern void gpu_b3_body_enable_contact_recycling(GpuBodyId body, bool enable);
extern bool gpu_b3_body_is_contact_recycling_enabled(GpuBodyId body);





















































































































































































































































































































static void configure_shape(GpuShapeId id, const b3ShapeDef* def)
{
	if (def == NULL)
	{
		return;
	}
	gpu_b3_shape_set_filter(id, def->filter.categoryBits, def->filter.maskBits, def->filter.groupIndex, false);
	gpu_b3_shape_set_sensor(id, def->isSensor);
	gpu_b3_shape_enable_sensor_events(id, def->enableSensorEvents);
	gpu_b3_shape_enable_contact_events(id, def->enableContactEvents);
	gpu_b3_shape_enable_speculative_contact(id, def->enableSpeculativeContact);
	gpu_b3_shape_enable_hit_events(id, def->enableHitEvents);
	gpu_b3_shape_enable_custom_filtering(id, def->enableCustomFiltering);
	gpu_b3_shape_enable_pre_solve_events(id, def->enablePreSolveEvents);
	gpu_b3_shape_set_surface_material(id, def->baseMaterial);
	gpu_b3_shape_set_user_data(id, (uintptr_t)def->userData);
	gpu_b3_shape_set_explosion_scale(id, def->explosionScale);
}































































B3_API const b3HullData* gpu_shape_get_hull(b3ShapeId shapeId)
{
	if (shapeId.index1 <= 0)
	{
		return NULL;
	}
	GpuHullMirror* mirror = find_hull_mirror(shapeId, false);
	return mirror != NULL ? mirror->data : NULL;
}
























// Compatibility extension for samples built against Box3D revisions that expose
// height-field replacement. The current upstream public header only has the getter.
B3_API void b3Shape_SetHeightField(b3ShapeId shapeId, const b3HeightFieldData* heightField)
{
	if (heightField == NULL || heightField->version != B3_HEIGHT_FIELD_VERSION)
	{
		return;
	}
	bool replaced = gpu_b3_replace_height_field(
		shapeId, b3GetHeightFieldCompressedHeights(heightField), heightField->columnCount, heightField->rowCount,
		heightField->minHeight, heightField->heightScale, heightField->scale.x, heightField->scale.y,
		heightField->scale.z, b3GetHeightFieldMaterialIndices(heightField), b3GetHeightFieldFlags(heightField),
		heightField->clockwise != 0);
	if (replaced)
	{
		GpuHeightFieldMirror* mirror = find_height_field_mirror(shapeId, false);
		if (mirror != NULL)
		{
			mirror->data = heightField;
		}
	}
}













































































































































void gpu_shape_clear_geometry(b3ShapeId);









































































































































































































B3_API b3ShapeId b3CreateTransformedHullShape(b3BodyId bodyId, const b3ShapeDef* def, const b3HullData* hull,
											  b3Transform transform, b3Vec3 scale);
B3_API b3ShapeId b3CreateCapsuleShape(b3BodyId bodyId, const b3ShapeDef* def, const b3Capsule* capsule);

typedef struct GpuCompoundHullInstance
{
	b3Transform transform;
	uint32_t hullOffset;
	uint32_t materialIndex;
} GpuCompoundHullInstance;

typedef struct GpuCompoundMeshInstance
{
	b3Transform transform;
	b3Vec3 scale;
	uint32_t meshOffset;
	uint32_t materialIndices[B3_MAX_COMPOUND_MESH_MATERIALS];
} GpuCompoundMeshInstance;






























































































































#include "compound_mesh_instances.h"
extern void gpu_b3_world_set_fail(b3WorldId id, const char* message);






















































































































































static b3JointId finish_joint_create(b3JointId id, const b3JointDef* base)
{
	gpu_b3_joint_set_force_threshold(id, base->forceThreshold);
	gpu_b3_joint_set_torque_threshold(id, base->torqueThreshold);
	gpu_b3_joint_set_user_data(id, (uintptr_t)base->userData);
	return id;
}

















































































































































































































































































































































































































































































































































































































































































#define MOTOR_SCALAR_API(Name, suffix)                                                               \
	B3_API void b3MotorJoint_Set##Name(b3JointId jointId, float value)                                \
	{                                                                                                 \
		gpu_b3_motor_set_##suffix(jointId, value);                                                       \
	}                                                                                                 \
	B3_API float b3MotorJoint_Get##Name(b3JointId jointId)                                            \
	{                                                                                                 \
		return gpu_b3_motor_get_##suffix(jointId);                                                       \
	}


















#undef MOTOR_SCALAR_API



























































































































































































extern void gpu_b3_world_set_contact_recycle_distance(b3WorldId id, float distance);
extern float gpu_b3_world_get_contact_recycle_distance(b3WorldId id);







extern void gpu_b3_destroy_body(b3BodyId id);
extern void gpu_samples_destroy_body(b3BodyId id) ;
void gpu_shape_clear_body_geometry(b3BodyId);









extern b3Matrix3 gpu_b3_body_get_local_rotational_inertia(b3BodyId body);
extern b3Matrix3 gpu_b3_body_get_world_inverse_rotational_inertia(b3BodyId body);









extern void gpu_b3_shape_set_density(b3ShapeId id, float density, bool updateMass);





extern b3WorldId gpu_b3_body_get_world(b3BodyId id);
extern b3WorldId gpu_b3_shape_get_world(b3ShapeId id);
extern b3WorldId gpu_b3_joint_get_world(b3JointId id);
extern uint32_t gpu_b3_joint_get_kind(b3JointId id);
extern b3BodyId gpu_b3_joint_get_body(b3JointId id, bool second);






















extern void gpu_b3_joint_set_collide_connected(GpuJointId, bool);
extern bool gpu_b3_joint_get_collide_connected(GpuJointId);










extern void gpu_b3_world_set_contact_tuning(b3WorldId id, float hertz, float damping, float speed);


extern void gpu_b3_world_set_user_data(b3WorldId id, uintptr_t value);


extern uintptr_t gpu_b3_world_get_user_data(b3WorldId id);


extern int gpu_b3_world_get_awake_body_count(b3WorldId id);


extern void gpu_b3_body_enable_sleep(b3BodyId id, bool enable);


extern bool gpu_b3_body_is_sleep_enabled(b3BodyId id);


extern void gpu_b3_body_set_sleep_threshold(b3BodyId id, float value);


extern float gpu_b3_body_get_sleep_threshold(b3BodyId id);


extern void gpu_b3_body_enable_hit_events(b3BodyId id, bool enable);


extern void gpu_b3_body_set_name(b3BodyId id, const char* name);


extern const char* gpu_b3_body_get_name(b3BodyId id);


extern void gpu_b3_joint_set_local_frame(b3JointId, bool, float, float, float, float, float, float, float);
extern void gpu_b3_joint_get_local_frame(b3JointId, bool, float*);
extern void gpu_b3_joint_wake_bodies(b3JointId);

















extern b3MassData gpu_b3_shape_compute_mass_data(b3ShapeId);


extern b3BodyId gpu_b3_shape_get_body(b3ShapeId);
extern bool b3IsValidHull(const b3HullData*);
extern bool gpu_b3_set_sphere(b3ShapeId, b3Sphere);
extern bool gpu_b3_set_capsule(b3ShapeId, b3Capsule);
extern bool gpu_b3_set_box_hull(b3ShapeId, float, float, float, float, float, float);
extern bool gpu_b3_set_convex_hull(b3ShapeId, const float*, int, const float*, int, const uint8_t*, int,
    float, float, float, float, float, float, float, float, float, float, float,
    float, float, float, float, float, float);
void gpu_samples_on_shape_replaced(b3ShapeId, b3BodyId, b3ShapeType, const b3Sphere*, const b3Capsule*, const b3HullData*) ;

void gpu_shape_clear_geometry(b3ShapeId id)
{
    if (id.index1 <= 0) { return; }
    // Baked compound children are hidden public colliders but still own mirrors.
    if (id.world0 == 0 || id.world0 >= GPU_METADATA_WORLDS) return;
    GpuSlots* slots = &g_geometry[id.world0];
    for (uint32_t i = 1; i < slots->high_water; ++i) {
        GpuGeometryMirror* slot = (GpuGeometryMirror*)gpu_slots_get(slots, (int32_t)i, sizeof(GpuGeometryMirror), false);
        if (!slot) continue;
        if (slot->hull.parent == id.index1) {
            free(slot->hull.data); slot->hull = (GpuHullMirror){0};
        }
        if (slot->mesh.parent == id.index1) {
            free(slot->mesh.materials); slot->mesh = (GpuMeshMirror){0};
        }
    }
    GpuHullMirror* hull = find_hull_mirror(id, false);
    if (hull) { free(hull->data); *hull = (GpuHullMirror){0}; }
    GpuMeshMirror* mesh = find_mesh_mirror(id, false);
    if (mesh) { free(mesh->materials); *mesh = (GpuMeshMirror){0}; }
    GpuHeightFieldMirror* height = find_height_field_mirror(id, false);
    if (height) { free(height->materials); *height = (GpuHeightFieldMirror){0}; }
}

void gpu_shape_set_sphere(b3ShapeId id, const b3Sphere* sphere)
{
    if (!sphere || !gpu_b3_set_sphere(id, *sphere)) { return; }
    gpu_shape_clear_geometry(id);
    if (gpu_samples_on_shape_replaced) {
        gpu_samples_on_shape_replaced(id, gpu_b3_shape_get_body(id), b3_sphereShape, sphere, NULL, NULL);
    }
}

void gpu_shape_set_capsule(b3ShapeId id, const b3Capsule* capsule)
{
    if (!capsule || !gpu_b3_set_capsule(id, *capsule)) { return; }
    gpu_shape_clear_geometry(id);
    if (gpu_samples_on_shape_replaced) {
        gpu_samples_on_shape_replaced(id, gpu_b3_shape_get_body(id), b3_capsuleShape, NULL, capsule, NULL);
    }
}

void gpu_shape_set_hull(b3ShapeId id, const b3HullData* hull)
{
    if (!hull || !b3IsValidHull(hull)) { return; }
    const b3HullData* old = gpu_shape_get_hull(id);
    // Upstream checks all bytes; the hash alone does not establish identity.
    if (old && old->byteCount == hull->byteCount && memcmp(old, hull, (size_t)hull->byteCount) == 0) { return; }
    b3HullData* copy = clone_hull_blob(hull);
    if (!copy) { fprintf(stderr, "GPU shape replacement: hull allocation failed\n"); abort(); }
    hull = copy;
    b3Vec3 half = b3MulSV(0.5f, b3Sub(hull->aabb.upperBound, hull->aabb.lowerBound));
    b3Vec3 center = b3MulSV(0.5f, b3Add(hull->aabb.upperBound, hull->aabb.lowerBound));
    const b3Vec3* points = (const b3Vec3*)((const char*)hull + hull->pointOffset);
    const b3Plane* planes = (const b3Plane*)((const char*)hull + hull->planeOffset);
    const uint8_t* edges = (const uint8_t*)hull + hull->edgeOffset;
    bool box = hull->vertexCount == 8 && hull->faceCount == 6;
    for (int i = 0; box && i < hull->vertexCount; ++i) {
        box = (points[i].x == hull->aabb.lowerBound.x || points[i].x == hull->aabb.upperBound.x) &&
              (points[i].y == hull->aabb.lowerBound.y || points[i].y == hull->aabb.upperBound.y) &&
              (points[i].z == hull->aabb.lowerBound.z || points[i].z == hull->aabb.upperBound.z);
    }
    bool changed = box ? gpu_b3_set_box_hull(id, half.x, half.y, half.z, center.x, center.y, center.z) :
        gpu_b3_set_convex_hull(id, &points[0].x, hull->vertexCount, &planes[0].normal.x, hull->faceCount,
            edges, hull->edgeCount, half.x, half.y, half.z, center.x, center.y, center.z,
            hull->center.x, hull->center.y, hull->center.z, hull->innerRadius, hull->volume,
            hull->centralInertia.cx.x, hull->centralInertia.cy.y, hull->centralInertia.cz.z,
            hull->centralInertia.cx.y, hull->centralInertia.cx.z, hull->centralInertia.cy.z);
    if (changed) {
        gpu_shape_clear_geometry(id);
        GpuHullMirror* mirror = find_hull_mirror(id, true);
        if (!mirror) { fprintf(stderr, "GPU shape replacement: hull mirror capacity exhausted\n"); abort(); }
        mirror->data = copy;
        mirror->index1 = id.index1;
        mirror->world0 = id.world0;
        copy = NULL;
        if (gpu_samples_on_shape_replaced) {
            gpu_samples_on_shape_replaced(id, gpu_b3_shape_get_body(id), b3_hullShape, NULL, NULL, hull);
        }
    }
    free(copy);
}





void gpu_shape_clear_body_geometry(b3BodyId body)
{
    int count = gpu_b3_body_get_shape_count(body);
    if (count <= 0) { return; }
    b3ShapeId* shapes = malloc((size_t)count * sizeof(*shapes));
    if (!shapes) { abort(); }
    count = gpu_b3_body_get_shapes(body, shapes, count);
    for (int i = 0; i < count; ++i) { gpu_shape_clear_geometry(shapes[i]); }
    free(shapes);
}

// Test/diagnostic ownership count, independent of Rust shape-slot allocation.
int gpu_shape_geometry_mirror_count(void)
{
    int count = 0;
    for (unsigned world = 1; world < GPU_METADATA_WORLDS; ++world) {
        GpuSlots* slots = &g_geometry[world];
        for (uint32_t i = 1; i < slots->high_water; ++i) {
            GpuGeometryMirror* slot = (GpuGeometryMirror*)gpu_slots_get(slots, (int32_t)i, sizeof(GpuGeometryMirror), false);
            if (!slot) continue;
            count += slot->hull.data != NULL;
            count += slot->mesh.index1 != 0;
            count += slot->height.index1 != 0;
        }
    }
    return count;
}

extern float gpu_b3_joint_get_linear_separation(b3JointId);


extern float gpu_b3_joint_get_angular_separation(b3JointId);


extern void gpu_b3_joint_get_constraint_force(b3JointId, float*);


extern void gpu_b3_joint_get_constraint_torque(b3JointId, float*);


extern void gpu_b3_world_enable_warm_starting(b3WorldId id, bool enable);
extern void gpu_b3_world_enable_speculative(b3WorldId id, bool enable);





extern bool gpu_b3_world_is_warm_starting_enabled(b3WorldId id);
#ifndef BOTH_SAMPLES








#endif
