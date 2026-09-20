// Thin C wrappers so a Box3D sample can include box3d.h and link this crate instead of libbox3d.

#include "box3d/box3d.h"
#include "box3d/collision.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "native_clock.h"

/* One public parent plus children must fit the packed 16-bit shape space.
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

B3_API b3CompoundData* __wrap_b3CreateCompound(const b3CompoundDef* def)
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

#define GPU_HULL_MIRROR_CAP 65536
typedef struct GpuHullMirror
{
	b3HullData* data;
	int32_t index1;
	uint16_t world0;
	int32_t parent;
} GpuHullMirror;
static GpuHullMirror g_gpu_hulls[GPU_HULL_MIRROR_CAP];

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
static GpuMeshMirror g_gpu_meshes[GPU_HULL_MIRROR_CAP];

typedef struct GpuHeightFieldMirror
{
	const b3HeightFieldData* data;
	b3SurfaceMaterial* materials;
	int materialCount;
	int32_t index1;
	uint16_t world0;
} GpuHeightFieldMirror;
static GpuHeightFieldMirror g_gpu_height_fields[GPU_HULL_MIRROR_CAP];

static GpuHeightFieldMirror* find_height_field_mirror(b3ShapeId id, bool create)
{
	GpuHeightFieldMirror* freeSlot = NULL;
	for (int i = 0; i < GPU_HULL_MIRROR_CAP; ++i)
	{
		GpuHeightFieldMirror* mirror = g_gpu_height_fields + i;
		if (mirror->index1 == id.index1 && mirror->world0 == id.world0)
		{
			return mirror;
		}
		if (freeSlot == NULL && mirror->index1 == 0)
		{
			freeSlot = mirror;
		}
	}
	return create ? freeSlot : NULL;
}

static GpuMeshMirror* find_mesh_mirror(b3ShapeId id, bool create)
{
	GpuMeshMirror* freeSlot = NULL;
	for (int i = 0; i < GPU_HULL_MIRROR_CAP; ++i)
	{
		GpuMeshMirror* mirror = g_gpu_meshes + i;
		if (mirror->index1 == id.index1 && mirror->world0 == id.world0)
		{
			return mirror;
		}
		if (freeSlot == NULL && mirror->index1 == 0)
		{
			freeSlot = mirror;
		}
	}
	return create ? freeSlot : NULL;
}

static GpuHullMirror* find_hull_mirror(b3ShapeId id, bool create)
{
	GpuHullMirror* freeSlot = NULL;
	for (int i = 0; i < GPU_HULL_MIRROR_CAP; ++i)
	{
		GpuHullMirror* mirror = g_gpu_hulls + i;
		if (mirror->index1 == id.index1 && mirror->world0 == id.world0)
		{
			return mirror;
		}
		if (freeSlot == NULL && mirror->index1 == 0)
		{
			freeSlot = mirror;
		}
	}
	return create ? freeSlot : NULL;
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

float gpu_samples_last_gpu_step_ms(void)
{
	return g_gpu_step_ms;
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

extern GpuWorldId gpu_b3_create_world(float gx, float gy, float gz);
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

void gpu_samples_on_world_created(b3WorldId world, const b3WorldDef* def) __attribute__((weak));
void gpu_samples_on_world_destroyed(b3WorldId world) __attribute__((weak));
void gpu_samples_on_shape_created(b3ShapeId shapeId, b3BodyId bodyId, b3ShapeType type, const b3Sphere* sphere,
								  const b3Capsule* capsule, const b3HullData* hull) __attribute__((weak));
void gpu_samples_on_mesh_shape_created(b3ShapeId shapeId, b3BodyId bodyId, const b3MeshData* mesh,
									 b3Vec3 scale) __attribute__((weak));
void gpu_samples_on_compound_shape_created(b3ShapeId shapeId, b3BodyId bodyId, const b3CompoundData* compound)
	__attribute__((weak));
void gpu_samples_on_shape_destroyed(b3ShapeId shapeId) __attribute__((weak));

B3_API b3WorldDef b3DefaultWorldDef(void)
{
	b3WorldDef def = {0};
	def.gravity.y = -10.0f;
	def.hitEventThreshold = 1.0f;
	def.restitutionThreshold = 1.0f;
	def.contactSpeed = 3.0f;
	def.contactHertz = 30.0f;
	def.contactDampingRatio = 10.0f;
	def.maximumLinearSpeed = 400.0f;
	def.enableSleep = true;
	def.enableContinuous = true;
	def.internalValue = 1152023;
	return def;
}

B3_API void b3World_EnableContinuous(b3WorldId worldId, bool flag)
{
	gpu_b3_world_enable_continuous(worldId, flag);
}

B3_API bool b3World_IsContinuousEnabled(b3WorldId worldId)
{
	return gpu_b3_world_is_continuous_enabled(worldId);
}

B3_API void b3World_SetGravity(b3WorldId worldId, b3Vec3 gravity)
{
	gpu_b3_world_set_gravity(worldId, gravity.x, gravity.y, gravity.z);
}

B3_API b3Vec3 b3World_GetGravity(b3WorldId worldId)
{
	return gpu_b3_world_get_gravity(worldId);
}

B3_API b3BodyDef b3DefaultBodyDef(void)
{
	b3BodyDef def = {0};
	def.type = b3_staticBody;
	def.rotation = b3Quat_identity;
	def.sleepThreshold = 0.05f;
	def.gravityScale = 1.0f;
	def.enableSleep = true;
	def.isAwake = true;
	def.isEnabled = true;
	def.enableContactRecycling = true;
	def.internalValue = 1152023;
	return def;
}

B3_API b3Filter b3DefaultFilter(void)
{
	b3Filter filter = {0};
	filter.categoryBits = 1;
	filter.maskBits = UINT64_MAX;
	return filter;
}

B3_API b3QueryFilter b3DefaultQueryFilter(void)
{
	return gpu_b3_default_query_filter();
}

B3_API b3TreeStats b3World_OverlapAABB(b3WorldId worldId, b3AABB aabb, b3QueryFilter filter,
										b3OverlapResultFcn* fcn, void* context)
{
	return gpu_b3_world_overlap_aabb(worldId, aabb, filter, fcn, context);
}

B3_API b3TreeStats b3World_OverlapShape(b3WorldId worldId, b3Pos origin, const b3ShapeProxy* proxy,
										b3QueryFilter filter, b3OverlapResultFcn* fcn, void* context)
{
	return gpu_b3_world_overlap_shape(worldId, origin, proxy, filter, fcn, context);
}

B3_API b3TreeStats b3World_CastRay(b3WorldId worldId, b3Pos origin, b3Vec3 translation, b3QueryFilter filter,
									b3CastResultFcn* fcn, void* context)
{
	return gpu_b3_world_cast_ray(worldId, origin, translation, filter, fcn, context);
}

B3_API b3RayResult b3World_CastRayClosest(b3WorldId worldId, b3Pos origin, b3Vec3 translation,
										  b3QueryFilter filter)
{
	return gpu_b3_world_cast_ray_closest(worldId, origin, translation, filter);
}

B3_API b3TreeStats b3World_CastShape(b3WorldId worldId, b3Pos origin, const b3ShapeProxy* proxy,
									 b3Vec3 translation, b3QueryFilter filter, b3CastResultFcn* fcn, void* context)
{
	return gpu_b3_world_cast_shape(worldId, origin, proxy, translation, filter, fcn, context);
}

B3_API float b3World_CastMover(b3WorldId worldId, b3Pos origin, const b3Capsule* mover, b3Vec3 translation,
							   b3QueryFilter filter, b3MoverFilterFcn* fcn, void* context)
{
	return gpu_b3_world_cast_mover(worldId, origin, mover, translation, filter, fcn, context);
}

B3_API void b3World_CollideMover(b3WorldId worldId, b3Pos origin, const b3Capsule* mover, b3QueryFilter filter,
								 b3PlaneResultFcn* fcn, void* context)
{
	gpu_b3_world_collide_mover(worldId, origin, mover, filter, fcn, context);
}

B3_API b3PlaneSolverResult b3SolvePlanes(b3Vec3 targetDelta, b3CollisionPlane* planes, int count)
{
	return gpu_b3_solve_planes(targetDelta, planes, count);
}

B3_API b3Vec3 b3ClipVector(b3Vec3 vector, const b3CollisionPlane* planes, int count)
{
	return gpu_b3_clip_vector(vector, planes, count);
}

B3_API b3SurfaceMaterial b3DefaultSurfaceMaterial(void)
{
	b3SurfaceMaterial m = {0};
	m.friction = 0.6f;
	return m;
}

B3_API b3ShapeDef b3DefaultShapeDef(void)
{
	b3ShapeDef def = {0};
	def.baseMaterial = b3DefaultSurfaceMaterial();
	def.density = 1000.0f;
	def.explosionScale = 1.0f;
	def.filter = b3DefaultFilter();
	def.updateBodyMass = true;
	def.invokeContactCreation = true;
	def.enableSpeculativeContact = true;
	def.internalValue = 1152023;
	return def;
}

B3_API b3ExplosionDef b3DefaultExplosionDef(void)
{
	b3ExplosionDef def = {0};
	def.maskBits = UINT64_MAX;
	return def;
}

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
B3_API b3WorldId b3CreateWorld(const b3WorldDef* def)
{
	if (!def)
	{
		return (b3WorldId){0};
	}
	GpuWorldId id = gpu_b3_create_world(def->gravity.x, def->gravity.y, def->gravity.z);
	gpu_b3_world_set_contact_tuning(id, def->contactHertz, def->contactDampingRatio, def->contactSpeed);
	gpu_b3_world_set_user_data(id, (uintptr_t)def->userData);
	gpu_b3_world_enable_sleeping(id, def->enableSleep);
	gpu_b3_world_enable_continuous(id, def->enableContinuous);
	gpu_b3_world_set_hit_event_threshold(id, def->hitEventThreshold);
	gpu_b3_world_set_restitution_threshold(id, def->restitutionThreshold);
	gpu_b3_world_set_maximum_linear_speed(id, def->maximumLinearSpeed);
	if (gpu_samples_on_world_created)
	{
		gpu_samples_on_world_created(id, def);
	}
	return id;
}

void gpu_shape_clear_world_geometry(b3WorldId worldId)
{
	for (int i = 1; i < GPU_HULL_MIRROR_CAP; ++i)
	{
		if (g_gpu_hulls[i].index1 != 0 && g_gpu_hulls[i].world0 == worldId.index1)
		{
			free(g_gpu_hulls[i].data);
			g_gpu_hulls[i] = (GpuHullMirror){0};
		}
		if (g_gpu_meshes[i].index1 != 0 && g_gpu_meshes[i].world0 == worldId.index1)
		{
			free(g_gpu_meshes[i].materials);
			g_gpu_meshes[i] = (GpuMeshMirror){0};
		}
		if (g_gpu_height_fields[i].index1 != 0 && g_gpu_height_fields[i].world0 == worldId.index1)
		{
			free(g_gpu_height_fields[i].materials);
			g_gpu_height_fields[i] = (GpuHeightFieldMirror){0};
		}
	}
}

B3_API void b3DestroyWorld(b3WorldId worldId)
{
	if (gpu_samples_on_world_destroyed)
	{
		gpu_samples_on_world_destroyed(worldId);
	}
	gpu_shape_clear_world_geometry(worldId);
	gpu_b3_destroy_world(worldId);
}

B3_API bool b3World_IsValid(b3WorldId id)
{
	return gpu_b3_world_is_valid(id);
}

B3_API void b3World_SetFrictionCallback(b3WorldId worldId, b3FrictionCallback* callback)
{
	gpu_b3_world_set_friction_callback(worldId, callback);
}

B3_API void b3World_SetRestitutionCallback(b3WorldId worldId, b3RestitutionCallback* callback)
{
	gpu_b3_world_set_restitution_callback(worldId, callback);
}

B3_API void b3World_Step(b3WorldId worldId, float timeStep, int subStepCount)
{
	gpu_b3_world_step(worldId, timeStep, subStepCount);
	g_gpu_step_ms = gpu_b3_world_last_encode_ms(worldId);
}

B3_API void b3World_Wait(b3WorldId worldId)
{
	gpu_b3_world_wait(worldId);
}

B3_API void b3World_SetCustomFilterCallback(b3WorldId worldId, b3CustomFilterFcn* fcn, void* context)
{
	gpu_b3_world_set_custom_filter_callback(worldId, fcn, context);
}

B3_API void b3World_SetPreSolveCallback(b3WorldId worldId, b3PreSolveFcn* fcn, void* context)
{
	gpu_b3_world_set_pre_solve_callback(worldId, fcn, context);
}

B3_API void b3World_Explode(b3WorldId worldId, const b3ExplosionDef* def)
{
	if (def == NULL)
	{
		return;
	}
	gpu_b3_world_explode(worldId, def->maskBits, (float)def->position.x, (float)def->position.y,
						 (float)def->position.z, def->radius, def->falloff, def->impulsePerArea);
}

B3_API b3ContactEvents b3World_GetContactEvents(b3WorldId worldId)
{
	b3ContactEvents events = {0};
	const b3ContactBeginTouchEvent* beginEvents = NULL;
	const b3ContactEndTouchEvent* endEvents = NULL;
	const b3ContactHitEvent* hitEvents = NULL;
	gpu_b3_world_get_contact_events(worldId, &beginEvents, &events.beginCount, &endEvents, &events.endCount,
									&hitEvents, &events.hitCount);
	events.beginEvents = (b3ContactBeginTouchEvent*)beginEvents;
	events.endEvents = (b3ContactEndTouchEvent*)endEvents;
	events.hitEvents = (b3ContactHitEvent*)hitEvents;
	return events;
}

B3_API b3BodyEvents b3World_GetBodyEvents(b3WorldId worldId)
{
	b3BodyEvents events = {0};
	const b3BodyMoveEvent* moveEvents = NULL;
	gpu_b3_world_get_body_events(worldId, &moveEvents, &events.moveCount);
	events.moveEvents = (b3BodyMoveEvent*)moveEvents;
	return events;
}

B3_API b3JointEvents b3World_GetJointEvents(b3WorldId worldId)
{
	b3JointEvents events = {0};
	const b3JointEvent* jointEvents = NULL;
	gpu_b3_world_get_joint_events(worldId, &jointEvents, &events.count);
	events.jointEvents = (b3JointEvent*)jointEvents;
	return events;
}

B3_API void b3World_SetHitEventThreshold(b3WorldId worldId, float value)
{
	gpu_b3_world_set_hit_event_threshold(worldId, value);
}

B3_API float b3World_GetHitEventThreshold(b3WorldId worldId)
{
	return gpu_b3_world_get_hit_event_threshold(worldId);
}

B3_API b3SensorEvents b3World_GetSensorEvents(b3WorldId worldId)
{
	b3SensorEvents events = {0};
	const b3SensorBeginTouchEvent* beginEvents = NULL;
	const b3SensorEndTouchEvent* endEvents = NULL;
	gpu_b3_world_get_sensor_events(worldId, &beginEvents, &events.beginCount, &endEvents, &events.endCount);
	events.beginEvents = (b3SensorBeginTouchEvent*)beginEvents;
	events.endEvents = (b3SensorEndTouchEvent*)endEvents;
	return events;
}

extern bool gpu_b3_body_is_valid(GpuBodyId body);
extern bool gpu_b3_shape_is_valid(GpuShapeId shape);
B3_API bool b3Body_IsValid(b3BodyId id) { return gpu_b3_body_is_valid(id); }
B3_API bool b3Shape_IsValid(b3ShapeId id) { return gpu_b3_shape_is_valid(id); }

B3_API b3BodyId b3CreateBody(b3WorldId worldId, const b3BodyDef* def)
{
	if (!def)
	{
		return (b3BodyId){0};
	}
	GpuBodyId id = gpu_b3_create_body(worldId, (int)def->type, def->position.x, def->position.y, def->position.z,
									 def->rotation.v.x, def->rotation.v.y, def->rotation.v.z, def->rotation.s,
									 def->linearVelocity.x, def->linearVelocity.y, def->linearVelocity.z,
									 def->angularVelocity.x, def->angularVelocity.y, def->angularVelocity.z,
									 def->gravityScale, locks_from_def(def));
	gpu_b3_body_set_name(id, def->name);
	gpu_b3_body_set_sleep_threshold(id, def->sleepThreshold);
	gpu_b3_body_set_user_data(id, (uintptr_t)def->userData);
	gpu_b3_body_set_bullet(id, def->isBullet);
	gpu_b3_body_allow_fast_rotation(id, def->allowFastRotation);
	gpu_b3_body_set_linear_damping(id, def->linearDamping);
	gpu_b3_body_set_angular_damping(id, def->angularDamping);
	if (!def->isEnabled) gpu_b3_body_set_enabled(id, false);
	return id;
}

extern void gpu_b3_body_enable_contact_recycling(GpuBodyId body, bool enable);
extern bool gpu_b3_body_is_contact_recycling_enabled(GpuBodyId body);

B3_API void b3Body_EnableContactRecycling(b3BodyId bodyId, bool enable)
{
    gpu_b3_body_enable_contact_recycling(bodyId, enable);
}

B3_API bool b3Body_IsContactRecyclingEnabled(b3BodyId bodyId)
{
    return gpu_b3_body_is_contact_recycling_enabled(bodyId);
}

B3_API void b3Body_SetBullet(b3BodyId bodyId, bool flag)
{
	gpu_b3_body_set_bullet(bodyId, flag);
}

B3_API bool b3Body_IsBullet(b3BodyId bodyId)
{
	return gpu_b3_body_is_bullet(bodyId);
}

B3_API void b3Body_AllowFastRotation(b3BodyId bodyId, bool flag)
{
	gpu_b3_body_allow_fast_rotation(bodyId, flag);
}

B3_API bool b3Body_IsFastRotationAllowed(b3BodyId bodyId)
{
	return gpu_b3_body_is_fast_rotation_allowed(bodyId);
}

B3_API void b3Body_SetUserData(b3BodyId bodyId, void* userData)
{
	gpu_b3_body_set_user_data(bodyId, (uintptr_t)userData);
}

B3_API void* b3Body_GetUserData(b3BodyId bodyId)
{
	return (void*)gpu_b3_body_get_user_data(bodyId);
}

B3_API void b3Body_SetLinearVelocity(b3BodyId bodyId, b3Vec3 linearVelocity)
{
	gpu_b3_body_set_linear_velocity(bodyId, linearVelocity.x, linearVelocity.y, linearVelocity.z);
}

B3_API void b3Body_SetAngularVelocity(b3BodyId bodyId, b3Vec3 angularVelocity)
{
	gpu_b3_body_set_angular_velocity(bodyId, angularVelocity.x, angularVelocity.y, angularVelocity.z);
}

B3_API b3Vec3 b3Body_GetLinearVelocity(b3BodyId bodyId)
{
	return gpu_b3_body_get_linear_velocity(bodyId);
}

B3_API b3Vec3 b3Body_GetAngularVelocity(b3BodyId bodyId)
{
	return gpu_b3_body_get_angular_velocity(bodyId);
}

B3_API void b3Body_ApplyLinearImpulse(b3BodyId bodyId, b3Vec3 impulse, b3Pos point, bool wake)
{
	gpu_b3_body_apply_linear_impulse(bodyId, impulse.x, impulse.y, impulse.z, point.x, point.y, point.z, wake);
}

B3_API void b3Body_ApplyLinearImpulseToCenter(b3BodyId bodyId, b3Vec3 impulse, bool wake)
{
	gpu_b3_body_apply_linear_impulse_to_center(bodyId, impulse.x, impulse.y, impulse.z, wake);
}

B3_API void b3Body_ApplyAngularImpulse(b3BodyId bodyId, b3Vec3 impulse, bool wake)
{
	gpu_b3_body_apply_angular_impulse(bodyId, impulse.x, impulse.y, impulse.z, wake);
}

B3_API void b3Body_ApplyForce(b3BodyId bodyId, b3Vec3 force, b3Pos point, bool wake)
{
	gpu_b3_body_apply_force(bodyId, force.x, force.y, force.z, (float)point.x, (float)point.y, (float)point.z,
						   wake);
}

B3_API void b3Body_ApplyForceToCenter(b3BodyId bodyId, b3Vec3 force, bool wake)
{
	gpu_b3_body_apply_force_to_center(bodyId, force.x, force.y, force.z, wake);
}

B3_API void b3Body_ApplyTorque(b3BodyId bodyId, b3Vec3 torque, bool wake)
{
	gpu_b3_body_apply_torque(bodyId, torque.x, torque.y, torque.z, wake);
}

B3_API float b3Body_GetMass(b3BodyId bodyId)
{
	return gpu_b3_body_get_mass(bodyId);
}

B3_API float b3Body_GetInverseMass(b3BodyId bodyId)
{
	return gpu_b3_body_inv_mass(bodyId);
}

B3_API b3MassData b3Body_GetMassData(b3BodyId bodyId)
{
	return gpu_b3_body_get_mass_data(bodyId);
}

B3_API void b3Body_SetMassData(b3BodyId bodyId, b3MassData massData)
{
	gpu_b3_body_set_mass_data(bodyId, massData);
}

B3_API b3Vec3 b3Body_GetLocalPoint(b3BodyId bodyId, b3Pos worldPoint)
{
	return gpu_b3_body_get_local_point(bodyId, (float)worldPoint.x, (float)worldPoint.y, (float)worldPoint.z);
}

B3_API b3Pos b3Body_GetWorldPoint(b3BodyId bodyId, b3Vec3 localPoint)
{
	b3Vec3 v = gpu_b3_body_get_world_point(bodyId, localPoint.x, localPoint.y, localPoint.z);
	b3Pos p = {v.x, v.y, v.z};
	return p;
}

B3_API b3Vec3 b3Body_GetLocalVector(b3BodyId bodyId, b3Vec3 worldVector)
{
	return gpu_b3_body_get_local_vector(bodyId, worldVector.x, worldVector.y, worldVector.z);
}

B3_API b3Vec3 b3Body_GetWorldVector(b3BodyId bodyId, b3Vec3 localVector)
{
	return gpu_b3_body_get_world_vector(bodyId, localVector.x, localVector.y, localVector.z);
}

B3_API b3Vec3 b3Body_GetWorldPointVelocity(b3BodyId bodyId, b3Pos worldPoint)
{
	return gpu_b3_body_get_world_point_velocity(bodyId, (float)worldPoint.x, (float)worldPoint.y,
											   (float)worldPoint.z);
}

B3_API b3Vec3 b3Body_GetLocalPointVelocity(b3BodyId bodyId, b3Vec3 localPoint)
{
	return gpu_b3_body_get_local_point_velocity(bodyId, localPoint.x, localPoint.y, localPoint.z);
}

B3_API bool b3Body_IsAwake(b3BodyId bodyId)
{
	return gpu_b3_body_is_awake(bodyId);
}

B3_API bool b3Body_IsEnabled(b3BodyId bodyId)
{
	return gpu_b3_body_is_enabled(bodyId);
}

B3_API void b3Body_Disable(b3BodyId bodyId)
{
	gpu_b3_body_set_enabled(bodyId, false);
}

B3_API void b3Body_Enable(b3BodyId bodyId)
{
	gpu_b3_body_set_enabled(bodyId, true);
}

B3_API void b3Body_SetMotionLocks(b3BodyId bodyId, b3MotionLocks locks)
{
	gpu_b3_body_set_motion_locks(bodyId, locks.linearX, locks.linearY, locks.linearZ, locks.angularX,
								 locks.angularY, locks.angularZ);
}

B3_API b3MotionLocks b3Body_GetMotionLocks(b3BodyId bodyId)
{
	b3MotionLocks locks = {0};
	gpu_b3_body_get_motion_locks(bodyId, &locks.linearX, &locks.linearY, &locks.linearZ, &locks.angularX,
								 &locks.angularY, &locks.angularZ);
	return locks;
}

B3_API b3Vec3 b3Body_GetLocalCenter(b3BodyId bodyId)
{
	return gpu_b3_body_get_local_center(bodyId);
}

B3_API float b3Body_GetLinearDamping(b3BodyId bodyId)
{
	return gpu_b3_body_get_linear_damping(bodyId);
}

B3_API float b3Body_GetAngularDamping(b3BodyId bodyId)
{
	return gpu_b3_body_get_angular_damping(bodyId);
}

B3_API float b3Body_GetGravityScale(b3BodyId bodyId)
{
	return gpu_b3_body_get_gravity_scale(bodyId);
}

B3_API int b3Body_GetJointCount(b3BodyId bodyId)
{
	return gpu_b3_body_get_joint_count(bodyId);
}

B3_API int b3Body_GetJoints(b3BodyId bodyId, b3JointId* jointArray, int capacity)
{
	return gpu_b3_body_get_joints(bodyId, jointArray, capacity);
}

B3_API b3BodyType b3Body_GetType(b3BodyId bodyId)
{
	return (b3BodyType)gpu_b3_body_get_type(bodyId);
}

B3_API int b3Body_GetShapeCount(b3BodyId bodyId)
{
	return gpu_b3_body_get_shape_count(bodyId);
}

B3_API int b3Body_GetShapes(b3BodyId bodyId, b3ShapeId* shapeArray, int capacity)
{
	return gpu_b3_body_get_shapes(bodyId, shapeArray, capacity);
}

B3_API void b3Body_SetType(b3BodyId bodyId, b3BodyType type)
{
	gpu_b3_body_set_type(bodyId, (int)type);
}

B3_API void b3Body_SetTransform(b3BodyId bodyId, b3Pos position, b3Quat rotation)
{
	gpu_b3_body_set_transform(bodyId, position.x, position.y, position.z, rotation.v.x, rotation.v.y, rotation.v.z,
							  rotation.s);
}

B3_API void b3Body_SetTargetTransform(b3BodyId bodyId, b3WorldTransform target, float timeStep, bool wake)
{
	gpu_b3_body_set_target_transform(bodyId, target.p.x, target.p.y, target.p.z, target.q.v.x, target.q.v.y,
									target.q.v.z, target.q.s, timeStep, wake);
}

B3_API void b3Body_SetAwake(b3BodyId bodyId, bool awake)
{
	gpu_b3_body_set_awake(bodyId, awake);
}

B3_API void b3Body_SetLinearDamping(b3BodyId bodyId, float linearDamping)
{
	gpu_b3_body_set_linear_damping(bodyId, linearDamping);
}

B3_API void b3Body_SetAngularDamping(b3BodyId bodyId, float angularDamping)
{
	gpu_b3_body_set_angular_damping(bodyId, angularDamping);
}

B3_API void b3Body_SetGravityScale(b3BodyId bodyId, float gravityScale)
{
	gpu_b3_body_set_gravity_scale(bodyId, gravityScale);
}

B3_API b3Pos b3Body_GetWorldCenter(b3BodyId bodyId)
{
	b3Pos center = {0};
	gpu_b3_body_get_world_center(bodyId, &center.x);
	return center;
}

B3_API void b3Body_ApplyMassFromShapes(b3BodyId bodyId)
{
	gpu_b3_body_apply_mass_from_shapes(bodyId);
}

B3_API b3AABB b3Body_ComputeAABB(b3BodyId bodyId)
{
	return gpu_b3_body_compute_aabb(bodyId);
}

B3_API float b3Body_GetClosestPoint(b3BodyId bodyId, b3Vec3* result, b3Vec3 target)
{
	return gpu_b3_body_get_closest_point(bodyId, result, target);
}

B3_API b3BodyCastResult b3Body_CastRay(b3BodyId bodyId, b3Pos origin, b3Vec3 translation,
									   b3QueryFilter filter, float maxFraction, b3WorldTransform bodyTransform)
{
	return gpu_b3_body_cast_ray(bodyId, origin, translation, filter, maxFraction, bodyTransform);
}

B3_API b3BodyCastResult b3Body_CastShape(b3BodyId bodyId, b3Pos origin, const b3ShapeProxy* proxy,
										 b3Vec3 translation, b3QueryFilter filter, float maxFraction,
										 bool canEncroach, b3WorldTransform bodyTransform)
{
	return gpu_b3_body_cast_shape(bodyId, origin, proxy, translation, filter, maxFraction, canEncroach,
								  bodyTransform);
}

B3_API bool b3Body_OverlapShape(b3BodyId bodyId, b3Pos origin, const b3ShapeProxy* proxy, b3QueryFilter filter,
								b3WorldTransform bodyTransform)
{
	return gpu_b3_body_overlap_shape(bodyId, origin, proxy, filter, bodyTransform);
}

B3_API int b3Body_CollideMover(b3BodyId bodyId, b3BodyPlaneResult* bodyPlanes, int planeCapacity, b3Pos origin,
							   const b3Capsule* mover, b3QueryFilter filter, b3WorldTransform bodyTransform)
{
	return gpu_b3_body_collide_mover(bodyId, bodyPlanes, planeCapacity, origin, mover, filter, bodyTransform);
}

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

B3_API b3Filter b3Shape_GetFilter(b3ShapeId shapeId)
{
	b3Filter filter = {0};
	gpu_b3_shape_get_filter(shapeId, &filter.categoryBits, &filter.maskBits, &filter.groupIndex);
	return filter;
}

B3_API void b3Shape_SetFilter(b3ShapeId shapeId, b3Filter filter, bool invokeContacts)
{
	gpu_b3_shape_set_filter(shapeId, filter.categoryBits, filter.maskBits, filter.groupIndex, invokeContacts);
}

B3_API void b3Shape_SetUserData(b3ShapeId shapeId, void* userData)
{
	gpu_b3_shape_set_user_data(shapeId, (uintptr_t)userData);
}

B3_API void* b3Shape_GetUserData(b3ShapeId shapeId)
{
	return (void*)gpu_b3_shape_get_user_data(shapeId);
}

B3_API float b3Shape_GetDensity(b3ShapeId shapeId)
{
	return gpu_b3_shape_get_density(shapeId);
}

B3_API float b3Shape_GetFriction(b3ShapeId shapeId)
{
	return gpu_b3_shape_get_friction(shapeId);
}

B3_API float b3Shape_GetRestitution(b3ShapeId shapeId)
{
	return gpu_b3_shape_get_restitution(shapeId);
}

B3_API void b3Shape_SetFriction(b3ShapeId shapeId, float friction)
{
	gpu_b3_shape_set_friction(shapeId, friction);
}

B3_API void b3Shape_SetRestitution(b3ShapeId shapeId, float restitution)
{
	gpu_b3_shape_set_restitution(shapeId, restitution);
}

B3_API void b3Shape_ApplyWind(b3ShapeId shapeId, b3Vec3 wind, float drag, float lift, float maxSpeed, bool wake)
{
	gpu_b3_shape_apply_wind(shapeId, wind.x, wind.y, wind.z, drag, lift, maxSpeed, wake);
}

B3_API b3Sphere b3Shape_GetSphere(b3ShapeId shapeId)
{
	return gpu_b3_shape_get_sphere(shapeId);
}

B3_API b3Capsule b3Shape_GetCapsule(b3ShapeId shapeId)
{
	return gpu_b3_shape_get_capsule(shapeId);
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

B3_API const b3HullData* b3Shape_GetHull(b3ShapeId id) { return gpu_shape_get_hull(id); }

B3_API const b3HeightFieldData* b3Shape_GetHeightField(b3ShapeId shapeId)
{
	GpuHeightFieldMirror* mirror = find_height_field_mirror(shapeId, false);
	return mirror != NULL ? mirror->data : NULL;
}

B3_API void b3Shape_SetSurfaceMaterial(b3ShapeId shapeId, b3SurfaceMaterial surfaceMaterial)
{
	gpu_b3_shape_set_surface_material(shapeId, surfaceMaterial);
	GpuMeshMirror* mirror = find_mesh_mirror(shapeId, false);
	if (mirror != NULL && mirror->materials != NULL && mirror->materialCount > 0)
	{
		mirror->materials[0] = surfaceMaterial;
	}
}

B3_API b3SurfaceMaterial b3Shape_GetSurfaceMaterial(b3ShapeId shapeId)
{
	return gpu_b3_shape_get_surface_material(shapeId);
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

B3_API b3Mesh b3Shape_GetMesh(b3ShapeId shapeId)
{
	GpuMeshMirror* mirror = find_mesh_mirror(shapeId, false);
	return mirror != NULL ? (b3Mesh){mirror->data, mirror->scale} : (b3Mesh){0};
}

B3_API int b3Shape_GetMeshMaterialCount(b3ShapeId shapeId)
{
	GpuMeshMirror* mirror = find_mesh_mirror(shapeId, false);
	if (mirror != NULL)
	{
		return mirror->materialCount;
	}
	GpuHeightFieldMirror* heightField = find_height_field_mirror(shapeId, false);
	return heightField != NULL ? heightField->materialCount : 0;
}

B3_API void b3Shape_SetMeshMaterial(b3ShapeId shapeId, b3SurfaceMaterial material, int index)
{
	GpuMeshMirror* mirror = find_mesh_mirror(shapeId, false);
	if (mirror != NULL && mirror->materials != NULL && 0 <= index && index < mirror->materialCount)
	{
		mirror->materials[index] = material;
		gpu_b3_shape_set_mesh_material(shapeId, index, material);
		return;
	}
	GpuHeightFieldMirror* heightField = find_height_field_mirror(shapeId, false);
	if (heightField != NULL && heightField->materials != NULL && 0 <= index && index < heightField->materialCount)
	{
		heightField->materials[index] = material;
		gpu_b3_shape_set_mesh_material(shapeId, index, material);
	}
}

B3_API b3SurfaceMaterial b3Shape_GetMeshSurfaceMaterial(b3ShapeId shapeId, int index)
{
	GpuMeshMirror* mirror = find_mesh_mirror(shapeId, false);
	if (mirror != NULL && mirror->materials != NULL && 0 <= index && index < mirror->materialCount)
	{
		return gpu_b3_shape_get_mesh_material(shapeId, index);
	}
	GpuHeightFieldMirror* heightField = find_height_field_mirror(shapeId, false);
	if (heightField != NULL && heightField->materials != NULL && 0 <= index && index < heightField->materialCount)
	{
		return gpu_b3_shape_get_mesh_material(shapeId, index);
	}
	return b3DefaultSurfaceMaterial();
}

B3_API void b3Shape_SetMesh(b3ShapeId shapeId, const b3MeshData* mesh, b3Vec3 scale)
{
	if (mesh == NULL || mesh->version != B3_MESH_VERSION)
	{
		return;
	}
	bool replaced = gpu_b3_replace_mesh(
		shapeId, &b3GetMeshVertices(mesh)[0].x, mesh->vertexCount, &b3GetMeshTriangles(mesh)[0].index1,
		mesh->triangleCount, b3GetMeshFlags(mesh), b3GetMeshMaterialIndices(mesh), b3GetMeshNodes(mesh),
		mesh->nodeCount, scale.x, scale.y, scale.z);
	if (replaced)
	{
		GpuMeshMirror* mirror = find_mesh_mirror(shapeId, false);
		if (mirror != NULL)
		{
			mirror->data = mesh;
			mirror->scale = scale;
		}
	}
}

B3_API b3AABB b3Shape_GetAABB(b3ShapeId shapeId)
{
	return gpu_b3_shape_get_aabb(shapeId);
}

B3_API b3Vec3 b3Shape_GetClosestPoint(b3ShapeId shapeId, b3Vec3 target)
{
	return gpu_b3_shape_get_closest_point(shapeId, target);
}

B3_API b3WorldCastOutput b3Shape_RayCast(b3ShapeId shapeId, b3Pos origin, b3Vec3 translation)
{
	return gpu_b3_shape_ray_cast(shapeId, origin, translation);
}

B3_API void b3Shape_EnableContactEvents(b3ShapeId shapeId, bool flag)
{
	gpu_b3_shape_enable_contact_events(shapeId, flag);
}

B3_API bool b3Shape_AreContactEventsEnabled(b3ShapeId shapeId)
{
	return gpu_b3_shape_are_contact_events_enabled(shapeId);
}

B3_API void b3Shape_EnablePreSolveEvents(b3ShapeId shapeId, bool flag)
{
	gpu_b3_shape_enable_pre_solve_events(shapeId, flag);
}

B3_API bool b3Shape_ArePreSolveEventsEnabled(b3ShapeId shapeId)
{
	return gpu_b3_shape_are_pre_solve_events_enabled(shapeId);
}

B3_API void b3Shape_EnableHitEvents(b3ShapeId shapeId, bool flag)
{
	gpu_b3_shape_enable_hit_events(shapeId, flag);
}

B3_API bool b3Shape_AreHitEventsEnabled(b3ShapeId shapeId)
{
	return gpu_b3_shape_are_hit_events_enabled(shapeId);
}

B3_API bool b3Shape_IsSensor(b3ShapeId shapeId)
{
	return gpu_b3_shape_is_sensor(shapeId);
}

B3_API void b3Shape_EnableSensorEvents(b3ShapeId shapeId, bool flag)
{
	gpu_b3_shape_enable_sensor_events(shapeId, flag);
}

B3_API bool b3Shape_AreSensorEventsEnabled(b3ShapeId shapeId)
{
	return gpu_b3_shape_are_sensor_events_enabled(shapeId);
}

B3_API int b3Shape_GetSensorCapacity(b3ShapeId shapeId)
{
	return gpu_b3_shape_get_sensor_capacity(shapeId);
}

B3_API int b3Shape_GetSensorData(b3ShapeId shapeId, b3ShapeId* visitorIds, int capacity)
{
	return gpu_b3_shape_get_sensor_data(shapeId, visitorIds, capacity);
}

void gpu_shape_clear_geometry(b3ShapeId);
B3_API void b3DestroyShape(b3ShapeId shapeId, bool updateBodyMass)
{
    if (gpu_samples_on_shape_destroyed) { gpu_samples_on_shape_destroyed(shapeId); }
    gpu_shape_clear_geometry(shapeId);
    gpu_b3_destroy_shape(shapeId, updateBodyMass);
}

B3_API b3ShapeId b3CreateSphereShape(b3BodyId bodyId, const b3ShapeDef* def, const b3Sphere* sphere)
{
	float density = 1000.0f;
	float friction = 0.6f;
	float restitution = 0.0f;
	float rolling = 0.0f;
	if (def)
	{
		density = def->density;
		friction = def->baseMaterial.friction;
		restitution = def->baseMaterial.restitution;
		rolling = def->baseMaterial.rollingResistance;
	}
	float radius = sphere ? sphere->radius : 0.5f;
	b3Vec3 center = sphere ? sphere->center : b3Vec3_zero;
	GpuShapeId id = gpu_b3_create_sphere(bodyId, center.x, center.y, center.z, radius, density, friction, restitution,
									 rolling, def == NULL || def->updateBodyMass);
	configure_shape(id, def);
	if (gpu_samples_on_shape_created)
	{
		gpu_samples_on_shape_created(id, bodyId, b3_sphereShape, sphere, NULL, NULL);
	}
	return id;
}

B3_API b3ShapeId b3CreateHullShape(b3BodyId bodyId, const b3ShapeDef* def, const b3HullData* hull)
{
	float density = 1000.0f;
	float friction = 0.6f;
	float restitution = 0.0f;
	float rolling = 0.0f;
	if (def)
	{
		density = def->density;
		friction = def->baseMaterial.friction;
		restitution = def->baseMaterial.restitution;
		rolling = def->baseMaterial.rollingResistance;
	}
	float hx = 0.5f;
	float hy = 0.5f;
	float hz = 0.5f;
	float ox = 0.0f;
	float oy = 0.0f;
	float oz = 0.0f;
	if (hull)
	{
		hx = 0.5f * (hull->aabb.upperBound.x - hull->aabb.lowerBound.x);
		hy = 0.5f * (hull->aabb.upperBound.y - hull->aabb.lowerBound.y);
		hz = 0.5f * (hull->aabb.upperBound.z - hull->aabb.lowerBound.z);
		ox = 0.5f * (hull->aabb.upperBound.x + hull->aabb.lowerBound.x);
		oy = 0.5f * (hull->aabb.upperBound.y + hull->aabb.lowerBound.y);
		oz = 0.5f * (hull->aabb.upperBound.z + hull->aabb.lowerBound.z);
	}
	bool axisBox = hull && hull->vertexCount == 8 && hull->faceCount == 6;
	const b3Vec3* points = hull ? (const b3Vec3*)((const char*)hull + hull->pointOffset) : NULL;
	const b3Plane* planes = hull ? (const b3Plane*)((const char*)hull + hull->planeOffset) : NULL;
	const b3HullHalfEdge* edges = hull ? (const b3HullHalfEdge*)((const char*)hull + hull->edgeOffset) : NULL;
	for (int i = 0; axisBox && i < hull->vertexCount; ++i)
	{
		axisBox = (points[i].x == hull->aabb.lowerBound.x || points[i].x == hull->aabb.upperBound.x) &&
				  (points[i].y == hull->aabb.lowerBound.y || points[i].y == hull->aabb.upperBound.y) &&
				  (points[i].z == hull->aabb.lowerBound.z || points[i].z == hull->aabb.upperBound.z);
	}
	GpuShapeId id;
	if (axisBox || hull == NULL || hull->vertexCount < 4)
	{
		id = gpu_b3_create_hull(bodyId, hx, hy, hz, ox, oy, oz, density, friction, restitution, rolling,
							   def == NULL || def->updateBodyMass);
	}
	else
	{
		id = gpu_b3_create_convex_hull(
			bodyId, &points[0].x, hull->vertexCount, &planes[0].normal.x, hull->faceCount, (const uint8_t*)edges,
			hull->edgeCount, hx, hy, hz, ox, oy, oz, hull->center.x, hull->center.y, hull->center.z, hull->innerRadius, hull->volume,
			hull->centralInertia.cx.x, hull->centralInertia.cy.y, hull->centralInertia.cz.z,
			hull->centralInertia.cx.y, hull->centralInertia.cx.z, hull->centralInertia.cy.z, density, friction,
			restitution, rolling, def == NULL || def->updateBodyMass);
	}
	configure_shape(id, def);
	gpu_shape_mirror_hull(id, hull);
	if (gpu_samples_on_shape_created)
	{
		gpu_samples_on_shape_created(id, bodyId, b3_hullShape, NULL, NULL, hull);
	}
	return id;
}

B3_API b3ShapeId b3CreateMeshShape(b3BodyId bodyId, const b3ShapeDef* def, const b3MeshData* mesh, b3Vec3 scale)
{
	if (mesh == NULL || mesh->version != B3_MESH_VERSION || mesh->vertexCount <= 0 ||
		mesh->triangleCount <= 0 || mesh->nodeCount <= 0)
	{
		return (b3ShapeId){0};
	}
	float density = def ? def->density : 1000.0f;
	float friction = def ? def->baseMaterial.friction : 0.6f;
	float restitution = def ? def->baseMaterial.restitution : 0.0f;
	float rolling = def ? def->baseMaterial.rollingResistance : 0.0f;
	const b3Vec3* vertices = b3GetMeshVertices(mesh);
	const b3MeshTriangle* triangles = b3GetMeshTriangles(mesh);
	const uint8_t* flags = b3GetMeshFlags(mesh);
	const uint8_t* materials = b3GetMeshMaterialIndices(mesh);
	const b3MeshNode* nodes = b3GetMeshNodes(mesh);
	GpuShapeId id = gpu_b3_create_mesh(
		bodyId, &vertices[0].x, mesh->vertexCount, &triangles[0].index1, mesh->triangleCount, flags, materials, nodes,
		mesh->nodeCount, scale.x, scale.y, scale.z, density, friction, restitution, rolling,
		def == NULL || def->updateBodyMass);
	configure_shape(id, def);
	if (id.index1 > 0)
	{
		GpuMeshMirror* mirror = find_mesh_mirror(id, true);
		if (mirror != NULL)
		{
			mirror->data = mesh;
			mirror->scale = scale;
			mirror->index1 = id.index1;
			mirror->world0 = id.world0;
			mirror->materialCount = def != NULL && def->materialCount > 0 ? def->materialCount : 1;
			gpu_b3_shape_set_mesh_material_count(id, mirror->materialCount);
			mirror->materials = malloc((size_t)mirror->materialCount * sizeof(b3SurfaceMaterial));
			if (mirror->materials != NULL)
			{
				if (def != NULL && def->materials != NULL && def->materialCount > 0)
				{
					memcpy(mirror->materials, def->materials,
						   (size_t)mirror->materialCount * sizeof(b3SurfaceMaterial));
				}
				else
				{
					mirror->materials[0] = def != NULL ? def->baseMaterial : b3DefaultSurfaceMaterial();
				}
				for (int i = 0; i < mirror->materialCount; ++i)
				{
					gpu_b3_shape_set_mesh_material(id, i, mirror->materials[i]);
				}
			}
		}
	}
	if (gpu_samples_on_mesh_shape_created)
	{
		gpu_samples_on_mesh_shape_created(id, bodyId, mesh, scale);
	}
	return id;
}

B3_API b3ShapeId b3CreateHeightFieldShape(b3BodyId bodyId, const b3ShapeDef* def,
										  const b3HeightFieldData* heightField)
{
	if (heightField == NULL || heightField->version != B3_HEIGHT_FIELD_VERSION ||
		heightField->columnCount < 2 || heightField->rowCount < 2 || b3Body_GetType(bodyId) != b3_staticBody)
	{
		return (b3ShapeId){0};
	}
	float density = def ? def->density : 1000.0f;
	float friction = def ? def->baseMaterial.friction : 0.6f;
	float restitution = def ? def->baseMaterial.restitution : 0.0f;
	float rolling = def ? def->baseMaterial.rollingResistance : 0.0f;
	GpuShapeId id = gpu_b3_create_height_field(
		bodyId, b3GetHeightFieldCompressedHeights(heightField), heightField->columnCount, heightField->rowCount,
		heightField->minHeight, heightField->heightScale, heightField->scale.x, heightField->scale.y,
		heightField->scale.z, b3GetHeightFieldMaterialIndices(heightField), b3GetHeightFieldFlags(heightField),
		heightField->clockwise != 0, density, friction, restitution, rolling);
	configure_shape(id, def);
	if (id.index1 > 0)
	{
		GpuHeightFieldMirror* mirror = find_height_field_mirror(id, true);
		if (mirror != NULL)
		{
			mirror->data = heightField;
			mirror->index1 = id.index1;
			mirror->world0 = id.world0;
			mirror->materialCount = def != NULL && def->materialCount > 0 ? def->materialCount : 1;
			mirror->materials = malloc((size_t)mirror->materialCount * sizeof(b3SurfaceMaterial));
			gpu_b3_shape_set_mesh_material_count(id, mirror->materialCount);
			if (mirror->materials != NULL)
			{
				for (int i = 0; i < mirror->materialCount; ++i)
				{
					mirror->materials[i] =
						def != NULL && def->materials != NULL && def->materialCount > 0
							? def->materials[i]
							: (def != NULL ? def->baseMaterial : b3DefaultSurfaceMaterial());
					gpu_b3_shape_set_mesh_material(id, i, mirror->materials[i]);
				}
			}
		}
	}
	if (gpu_samples_on_shape_created)
	{
		gpu_samples_on_shape_created(id, bodyId, b3_heightShape, NULL, NULL, NULL);
	}
	return id;
}

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

B3_API const b3SurfaceMaterial* b3GetCompoundMaterials(const b3CompoundData* compound)
{
	if (compound == NULL || compound->materialOffset == 0)
	{
		return NULL;
	}
	return (const b3SurfaceMaterial*)((intptr_t)compound + compound->materialOffset);
}

B3_API b3CompoundCapsule b3GetCompoundCapsule(const b3CompoundData* compound, int index)
{
	b3CompoundCapsule result = {0};
	if (compound == NULL || compound->capsuleOffset == 0 || index < 0 || index >= compound->capsuleCount)
	{
		return result;
	}
	const b3CompoundCapsule* capsules =
		(const b3CompoundCapsule*)((intptr_t)compound + compound->capsuleOffset);
	return capsules[index];
}

B3_API b3CompoundHull b3GetCompoundHull(const b3CompoundData* compound, int index)
{
	b3CompoundHull result = {0};
	if (compound == NULL || compound->hullOffset == 0 || index < 0 || index >= compound->hullCount)
	{
		return result;
	}
	const GpuCompoundHullInstance* hullInstances =
		(const GpuCompoundHullInstance*)((intptr_t)compound + compound->hullOffset);
	uint32_t hullOffset = hullInstances[index].hullOffset;
	result.hull = (const b3HullData*)((intptr_t)compound + hullOffset);
	result.transform = hullInstances[index].transform;
	result.materialIndex = (int)hullInstances[index].materialIndex;
	return result;
}

B3_API b3CompoundMesh b3GetCompoundMesh(const b3CompoundData* compound, int index)
{
	b3CompoundMesh result = {0};
	if (compound == NULL || compound->meshOffset == 0 || index < 0 || index >= compound->meshCount)
	{
		return result;
	}
	const GpuCompoundMeshInstance* meshInstances =
		(const GpuCompoundMeshInstance*)((intptr_t)compound + compound->meshOffset);
	result.meshData = (const b3MeshData*)((intptr_t)compound + meshInstances[index].meshOffset);
	result.transform = meshInstances[index].transform;
	result.scale = meshInstances[index].scale;
	for (int i = 0; i < B3_MAX_COMPOUND_MESH_MATERIALS; ++i)
	{
		result.materialIndices[i] = (int)meshInstances[index].materialIndices[i];
	}
	return result;
}

B3_API b3CompoundSphere b3GetCompoundSphere(const b3CompoundData* compound, int index)
{
	b3CompoundSphere result = {0};
	if (compound == NULL || compound->sphereOffset == 0 || index < 0 || index >= compound->sphereCount)
	{
		return result;
	}
	const b3CompoundSphere* spheres = (const b3CompoundSphere*)((intptr_t)compound + compound->sphereOffset);
	return spheres[index];
}

B3_API b3ChildShape b3GetCompoundChild(const b3CompoundData* compound, int childIndex)
{
	b3ChildShape result = {0};
	if (compound == NULL)
	{
		return result;
	}
	if (0 <= childIndex && childIndex < compound->capsuleCount)
	{
		b3CompoundCapsule compoundCapsule = b3GetCompoundCapsule(compound, childIndex);
		return (b3ChildShape){
			.capsule = compoundCapsule.capsule,
			.transform = b3Transform_identity,
			.materialIndices = {compoundCapsule.materialIndex},
			.type = b3_capsuleShape,
		};
	}
	childIndex -= compound->capsuleCount;
	if (0 <= childIndex && childIndex < compound->hullCount)
	{
		b3CompoundHull compoundHull = b3GetCompoundHull(compound, childIndex);
		return (b3ChildShape){
			.hull = compoundHull.hull,
			.transform = compoundHull.transform,
			.materialIndices = {compoundHull.materialIndex},
			.type = b3_hullShape,
		};
	}
	childIndex -= compound->hullCount;
	if (0 <= childIndex && childIndex < compound->meshCount)
	{
		b3CompoundMesh compoundMesh = b3GetCompoundMesh(compound, childIndex);
		const int* m = compoundMesh.materialIndices;
		return (b3ChildShape){
			.mesh =
				{
					.data = compoundMesh.meshData,
					.scale = compoundMesh.scale,
				},
			.transform = compoundMesh.transform,
			.materialIndices = {m[0], m[1], m[2], m[3]},
			.type = b3_meshShape,
		};
	}
	childIndex -= compound->meshCount;
	if (0 <= childIndex && childIndex < compound->sphereCount)
	{
		b3CompoundSphere compoundSphere = b3GetCompoundSphere(compound, childIndex);
		return (b3ChildShape){
			.sphere = compoundSphere.sphere,
			.transform = b3Transform_identity,
			.materialIndices = {compoundSphere.materialIndex},
			.type = b3_sphereShape,
		};
	}
	return result;
}

#include "compound_mesh_instances.h"
extern void gpu_b3_world_set_fail(b3WorldId id, const char* message);

B3_API b3ShapeId b3CreateBakedCompoundShape(b3BodyId bodyId, b3ShapeDef* def, const b3CompoundData* compound)
{
	int childCount = 0;
	if (compound != NULL)
	{
		childCount =
			gpu_compound_count(compound->capsuleCount, compound->hullCount, compound->meshCount, compound->sphereCount);
	}
	if (compound == NULL || compound->version != B3_COMPOUND_VERSION || b3Body_GetType(bodyId) != b3_staticBody ||
		(def != NULL && def->isSensor) || (childCount < 0 || childCount > gpu_compound_child_limit()))
	{
		extern void gpu_b3_world_set_fail(b3WorldId id, const char* message);
		b3WorldId world = {.index1 = bodyId.world0, .generation = 1};
		if (childCount < 0 || childCount > gpu_compound_child_limit())
		{
			refuse_compound(childCount);
		}
		gpu_b3_world_set_fail(world, "unsupported compound (invalid data or child count)");
		return (b3ShapeId){0};
	}
	float density = def ? def->density : 1000.0f;
	float friction = def ? def->baseMaterial.friction : 0.6f;
	float restitution = def ? def->baseMaterial.restitution : 0.0f;
	float rolling = def ? def->baseMaterial.rollingResistance : 0.0f;
	GpuShapeId parent = gpu_b3_create_compound_parent(bodyId, density, friction, restitution, rolling,
													   def != NULL && def->isSensor);
	if (parent.index1 <= 0)
	{
		gpu_b3_world_set_fail((b3WorldId){bodyId.world0, 1}, "compound parent allocation failed");
		return (b3ShapeId){0};
	}
	configure_shape(parent, def);
	const b3SurfaceMaterial* materials = b3GetCompoundMaterials(compound);
    GpuMeshCache meshCache;
    if (!gpu_mesh_cache_init(&meshCache, compound->meshCount)) {
        gpu_b3_world_set_fail((b3WorldId){bodyId.world0, 1}, "compound mesh cache allocation failed");
        return (b3ShapeId){0};
    }

	for (int i = 0; i < childCount; ++i)
	{
		b3ChildShape child = b3GetCompoundChild(compound, i);
		b3ShapeDef childDef = def != NULL ? *def : b3DefaultShapeDef();
		childDef.updateBodyMass = false;
		childDef.isSensor = false;
		int materialIndex = child.materialIndices[0];
		if (materials != NULL && materialIndex >= 0 && materialIndex < compound->materialCount)
		{
			childDef.baseMaterial = materials[materialIndex];
		}
		b3ShapeId childId = {0};
		if (child.type == b3_sphereShape)
		{
			childId = b3CreateSphereShape(bodyId, &childDef, &child.sphere);
		}
		else if (child.type == b3_capsuleShape)
		{
			childId = b3CreateCapsuleShape(bodyId, &childDef, &child.capsule);
		}
		else if (child.type == b3_hullShape && child.hull != NULL)
		{
			childId = b3CreateTransformedHullShape(bodyId, &childDef, child.hull, child.transform, b3Vec3_one);
		}
		else if (child.type == b3_meshShape && child.mesh.data != NULL)
		{
			const b3MeshData* mesh = child.mesh.data;
			GpuShapeId meshId = gpu_mesh_cache_instance(&meshCache, bodyId, &child, &childDef);
			configure_shape(meshId, &childDef);
			/* Triangle indices are local to this mesh; the child map resolves them
			   into the compound's shared surface-material table. */
			gpu_b3_shape_set_mesh_material_count(meshId, mesh->materialCount);
			for (int j = 0; j < mesh->materialCount; ++j)
			{
				gpu_b3_shape_set_mesh_material(meshId, j, materials[child.materialIndices[j]]);
			}
			childId = meshId;
		}
		if (childId.index1 <= 0 ||
			!gpu_b3_shape_attach_compound_child_materials(parent, childId, i, child.materialIndices))
		{
			gpu_mesh_cache_free(&meshCache);
			gpu_b3_world_set_fail((b3WorldId){bodyId.world0, 1}, "compound child creation or attachment failed");
			return (b3ShapeId){0};
		}
		gpu_shape_mirror_parent(childId, parent);
		{
			if (gpu_samples_on_shape_destroyed)
			{
				gpu_samples_on_shape_destroyed(childId);
			}
		}
	}
	gpu_mesh_cache_free(&meshCache);
	if (gpu_samples_on_compound_shape_created)
	{
		gpu_samples_on_compound_shape_created(parent, bodyId, compound);
	}
	else if (gpu_samples_on_shape_created)
	{
		gpu_samples_on_shape_created(parent, bodyId, b3_compoundShape, NULL, NULL, NULL);
	}
	return parent;
}

B3_API b3ShapeId b3CreateTransformedHullShape(b3BodyId bodyId, const b3ShapeDef* def, const b3HullData* hull,
											  b3Transform transform, b3Vec3 scale)
{
	b3HullData* transformed = b3CloneAndTransformHull(hull, transform, scale);
	if (transformed == NULL)
	{
		return (b3ShapeId){0};
	}
	b3ShapeId shape = b3CreateHullShape(bodyId, def, transformed);
	b3DestroyHull(transformed);
	return shape;
}

B3_API b3ShapeId b3CreateCapsuleShape(b3BodyId bodyId, const b3ShapeDef* def, const b3Capsule* capsule)
{
	float density = 1000.0f;
	float friction = 0.6f;
	float restitution = 0.0f;
	float rolling = 0.0f;
	if (def)
	{
		density = def->density;
		friction = def->baseMaterial.friction;
		restitution = def->baseMaterial.restitution;
		rolling = def->baseMaterial.rollingResistance;
	}
	b3Vec3 c1 = { -1.0f, 0.0f, 0.0f };
	b3Vec3 c2 = { 1.0f, 0.0f, 0.0f };
	float radius = 0.5f;
	if (capsule)
	{
		c1 = capsule->center1;
		c2 = capsule->center2;
		radius = capsule->radius;
	}
	GpuShapeId id = gpu_b3_create_capsule(bodyId, c1.x, c1.y, c1.z, c2.x, c2.y, c2.z, radius, density, friction,
									 restitution, rolling, def == NULL || def->updateBodyMass);
	configure_shape(id, def);
	if (gpu_samples_on_shape_created)
	{
		gpu_samples_on_shape_created(id, bodyId, b3_capsuleShape, NULL, capsule, NULL);
	}
	return id;
}

static b3JointId finish_joint_create(b3JointId id, const b3JointDef* base)
{
	gpu_b3_joint_set_force_threshold(id, base->forceThreshold);
	gpu_b3_joint_set_torque_threshold(id, base->torqueThreshold);
	gpu_b3_joint_set_user_data(id, (uintptr_t)base->userData);
	return id;
}

B3_API b3RevoluteJointDef b3DefaultRevoluteJointDef(void)
{
	b3RevoluteJointDef def = {0};
	def.base.localFrameA.q = b3Quat_identity;
	def.base.localFrameB.q = b3Quat_identity;
	def.base.constraintHertz = 60.0f;
	def.base.constraintDampingRatio = 2.0f;
	def.base.forceThreshold = FLT_MAX;
	def.base.torqueThreshold = FLT_MAX;
	def.base.internalValue = 1152023;
	return def;
}

B3_API b3WeldJointDef b3DefaultWeldJointDef(void)
{
	b3WeldJointDef def = {0};
	def.base.localFrameA.q = b3Quat_identity;
	def.base.localFrameB.q = b3Quat_identity;
	def.base.constraintHertz = 60.0f;
	def.base.constraintDampingRatio = 2.0f;
	def.base.forceThreshold = FLT_MAX;
	def.base.torqueThreshold = FLT_MAX;
	def.base.internalValue = 1152023;
	return def;
}

B3_API b3WheelJointDef b3DefaultWheelJointDef(void)
{
	b3WheelJointDef def = {0};
	def.base.localFrameA.q = b3Quat_identity;
	def.base.localFrameB.q = b3Quat_identity;
	def.base.constraintHertz = 60.0f;
	def.base.constraintDampingRatio = 2.0f;
	def.base.forceThreshold = FLT_MAX;
	def.base.torqueThreshold = FLT_MAX;
	def.base.internalValue = 1152023;
	def.enableSuspensionSpring = true;
	def.suspensionHertz = 1.0f;
	def.suspensionDampingRatio = 0.7f;
	def.steeringHertz = 1.0f;
	def.steeringDampingRatio = 0.7f;
	return def;
}

B3_API b3PrismaticJointDef b3DefaultPrismaticJointDef(void)
{
	b3PrismaticJointDef def = {0};
	def.base.localFrameA.q = b3Quat_identity;
	def.base.localFrameB.q = b3Quat_identity;
	def.base.constraintHertz = 60.0f;
	def.base.constraintDampingRatio = 2.0f;
	def.base.forceThreshold = FLT_MAX;
	def.base.torqueThreshold = FLT_MAX;
	def.base.internalValue = 1152023;
	return def;
}

B3_API b3DistanceJointDef b3DefaultDistanceJointDef(void)
{
	b3DistanceJointDef def = {0};
	def.base.localFrameA.q = b3Quat_identity;
	def.base.localFrameB.q = b3Quat_identity;
	def.base.constraintHertz = 60.0f;
	def.base.constraintDampingRatio = 2.0f;
	def.base.forceThreshold = FLT_MAX;
	def.base.torqueThreshold = FLT_MAX;
	def.base.internalValue = 1152023;
	def.length = 1.0f;
	def.lowerSpringForce = -FLT_MAX;
	def.upperSpringForce = FLT_MAX;
	def.maxLength = 1.0e5f;
	return def;
}

B3_API b3MotorJointDef b3DefaultMotorJointDef(void)
{
	b3MotorJointDef def = {0};
	def.base.localFrameA.q = b3Quat_identity;
	def.base.localFrameB.q = b3Quat_identity;
	def.base.constraintHertz = 60.0f;
	def.base.constraintDampingRatio = 2.0f;
	def.base.forceThreshold = FLT_MAX;
	def.base.torqueThreshold = FLT_MAX;
	def.base.internalValue = 1152023;
	return def;
}

B3_API b3ParallelJointDef b3DefaultParallelJointDef(void)
{
	b3ParallelJointDef def = {0};
	def.base.localFrameA.q = b3Quat_identity;
	def.base.localFrameB.q = b3Quat_identity;
	def.base.constraintHertz = 60.0f;
	def.base.constraintDampingRatio = 2.0f;
	def.base.forceThreshold = FLT_MAX;
	def.base.torqueThreshold = FLT_MAX;
	def.base.internalValue = 1152023;
	def.hertz = 1.0f;
	def.dampingRatio = 1.0f;
	def.maxTorque = FLT_MAX;
	return def;
}

B3_API b3FilterJointDef b3DefaultFilterJointDef(void)
{
	b3FilterJointDef def = {0};
	def.base.localFrameA.q = b3Quat_identity;
	def.base.localFrameB.q = b3Quat_identity;
	def.base.constraintHertz = 60.0f;
	def.base.constraintDampingRatio = 2.0f;
	def.base.forceThreshold = FLT_MAX;
	def.base.torqueThreshold = FLT_MAX;
	def.base.internalValue = 1152023;
	return def;
}

B3_API b3JointId b3CreateRevoluteJoint(b3WorldId worldId, const b3RevoluteJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	return finish_joint_create(gpu_b3_create_revolute(
		worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.localFrameA.p.x, def->base.localFrameA.p.y,
		def->base.localFrameA.p.z, def->base.localFrameB.p.x, def->base.localFrameB.p.y, def->base.localFrameB.p.z,
		def->base.localFrameA.q.v.x, def->base.localFrameA.q.v.y, def->base.localFrameA.q.v.z,
		def->base.localFrameA.q.s, def->base.localFrameB.q.v.x, def->base.localFrameB.q.v.y,
		def->base.localFrameB.q.v.z, def->base.localFrameB.q.s, def->base.constraintHertz,
		def->base.constraintDampingRatio, def->base.collideConnected, def->targetAngle, def->enableSpring, def->hertz,
		def->dampingRatio, def->enableLimit, def->lowerAngle, def->upperAngle, def->enableMotor, def->maxMotorTorque,
		def->motorSpeed), &def->base);
}

B3_API b3JointId b3CreateWheelJoint(b3WorldId worldId, const b3WheelJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	return finish_joint_create(gpu_b3_create_wheel(
		worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.localFrameA.p.x, def->base.localFrameA.p.y,
		def->base.localFrameA.p.z, def->base.localFrameB.p.x, def->base.localFrameB.p.y, def->base.localFrameB.p.z,
		def->base.localFrameA.q.v.x, def->base.localFrameA.q.v.y, def->base.localFrameA.q.v.z,
		def->base.localFrameA.q.s, def->base.localFrameB.q.v.x, def->base.localFrameB.q.v.y,
		def->base.localFrameB.q.v.z, def->base.localFrameB.q.s, def->base.constraintHertz,
		def->base.constraintDampingRatio, def->base.collideConnected, def->enableSuspensionSpring, def->suspensionHertz,
		def->suspensionDampingRatio, def->enableSuspensionLimit, def->lowerSuspensionLimit,
		def->upperSuspensionLimit, def->enableSpinMotor, def->maxSpinTorque, def->spinSpeed, def->enableSteering,
		def->steeringHertz, def->steeringDampingRatio, def->targetSteeringAngle, def->maxSteeringTorque,
		def->enableSteeringLimit, def->lowerSteeringLimit, def->upperSteeringLimit), &def->base);
}

B3_API void b3WheelJoint_EnableSuspension(b3JointId id, bool value) { gpu_b3_wheel_enable_suspension(id, value); }
B3_API bool b3WheelJoint_IsSuspensionEnabled(b3JointId id) { return gpu_b3_wheel_is_suspension_enabled(id); }
B3_API void b3WheelJoint_SetSuspensionHertz(b3JointId id, float value) { gpu_b3_wheel_set_suspension_hertz(id, value); }
B3_API float b3WheelJoint_GetSuspensionHertz(b3JointId id) { return gpu_b3_wheel_get_suspension_hertz(id); }
B3_API void b3WheelJoint_SetSuspensionDampingRatio(b3JointId id, float value) { gpu_b3_wheel_set_suspension_damping(id, value); }
B3_API float b3WheelJoint_GetSuspensionDampingRatio(b3JointId id) { return gpu_b3_wheel_get_suspension_damping(id); }
B3_API void b3WheelJoint_EnableSuspensionLimit(b3JointId id, bool value) { gpu_b3_wheel_enable_suspension_limit(id, value); }
B3_API bool b3WheelJoint_IsSuspensionLimitEnabled(b3JointId id) { return gpu_b3_wheel_is_suspension_limit_enabled(id); }
B3_API float b3WheelJoint_GetLowerSuspensionLimit(b3JointId id) { return gpu_b3_wheel_get_lower_suspension_limit(id); }
B3_API float b3WheelJoint_GetUpperSuspensionLimit(b3JointId id) { return gpu_b3_wheel_get_upper_suspension_limit(id); }
B3_API void b3WheelJoint_SetSuspensionLimits(b3JointId id, float lower, float upper) { gpu_b3_wheel_set_suspension_limits(id, lower, upper); }
B3_API void b3WheelJoint_EnableSpinMotor(b3JointId id, bool value) { gpu_b3_wheel_enable_spin_motor(id, value); }
B3_API bool b3WheelJoint_IsSpinMotorEnabled(b3JointId id) { return gpu_b3_wheel_is_spin_motor_enabled(id); }
B3_API void b3WheelJoint_SetSpinMotorSpeed(b3JointId id, float value) { gpu_b3_wheel_set_spin_speed(id, value); }
B3_API float b3WheelJoint_GetSpinMotorSpeed(b3JointId id) { return gpu_b3_wheel_get_spin_speed_setting(id); }
B3_API void b3WheelJoint_SetMaxSpinTorque(b3JointId id, float value) { gpu_b3_wheel_set_max_spin_torque(id, value); }
B3_API float b3WheelJoint_GetMaxSpinTorque(b3JointId id) { return gpu_b3_wheel_get_max_spin_torque(id); }
B3_API float b3WheelJoint_GetSpinSpeed(b3JointId id) { return gpu_b3_wheel_get_live_spin_speed(id); }
B3_API float b3WheelJoint_GetSpinTorque(b3JointId id) { return gpu_b3_wheel_get_spin_torque(id); }
B3_API void b3WheelJoint_EnableSteering(b3JointId id, bool value) { gpu_b3_wheel_enable_steering(id, value); }
B3_API bool b3WheelJoint_IsSteeringEnabled(b3JointId id) { return gpu_b3_wheel_is_steering_enabled(id); }
B3_API void b3WheelJoint_SetSteeringHertz(b3JointId id, float value) { gpu_b3_wheel_set_steering_hertz(id, value); }
B3_API float b3WheelJoint_GetSteeringHertz(b3JointId id) { return gpu_b3_wheel_get_steering_hertz(id); }
B3_API void b3WheelJoint_SetSteeringDampingRatio(b3JointId id, float value) { gpu_b3_wheel_set_steering_damping(id, value); }
B3_API float b3WheelJoint_GetSteeringDampingRatio(b3JointId id) { return gpu_b3_wheel_get_steering_damping(id); }
B3_API void b3WheelJoint_SetMaxSteeringTorque(b3JointId id, float value) { gpu_b3_wheel_set_max_steering_torque(id, value); }
B3_API float b3WheelJoint_GetMaxSteeringTorque(b3JointId id) { return gpu_b3_wheel_get_max_steering_torque(id); }
B3_API void b3WheelJoint_EnableSteeringLimit(b3JointId id, bool value) { gpu_b3_wheel_enable_steering_limit(id, value); }
B3_API bool b3WheelJoint_IsSteeringLimitEnabled(b3JointId id) { return gpu_b3_wheel_is_steering_limit_enabled(id); }
B3_API float b3WheelJoint_GetLowerSteeringLimit(b3JointId id) { return gpu_b3_wheel_get_lower_steering_limit(id); }
B3_API float b3WheelJoint_GetUpperSteeringLimit(b3JointId id) { return gpu_b3_wheel_get_upper_steering_limit(id); }
B3_API void b3WheelJoint_SetSteeringLimits(b3JointId id, float lower, float upper) { gpu_b3_wheel_set_steering_limits(id, lower, upper); }
B3_API void b3WheelJoint_SetTargetSteeringAngle(b3JointId id, float value) { gpu_b3_wheel_set_target_steering(id, value); }
B3_API float b3WheelJoint_GetTargetSteeringAngle(b3JointId id) { return gpu_b3_wheel_get_target_steering(id); }
B3_API float b3WheelJoint_GetSteeringAngle(b3JointId id) { return gpu_b3_wheel_get_steering_angle(id); }
B3_API float b3WheelJoint_GetSteeringTorque(b3JointId id) { return gpu_b3_wheel_get_steering_torque(id); }

B3_API b3SphericalJointDef b3DefaultSphericalJointDef(void)
{
	b3SphericalJointDef def = {0};
	def.base.localFrameA.q = b3Quat_identity;
	def.base.localFrameB.q = b3Quat_identity;
	def.base.constraintHertz = 60.0f;
	def.base.constraintDampingRatio = 2.0f;
	def.base.forceThreshold = FLT_MAX;
	def.base.torqueThreshold = FLT_MAX;
	def.base.internalValue = 1152023;
	def.targetRotation = b3Quat_identity;
	return def;
}

B3_API b3JointId b3CreateSphericalJoint(b3WorldId worldId, const b3SphericalJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	return finish_joint_create(gpu_b3_create_spherical(worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.localFrameA.p.x,
								   def->base.localFrameA.p.y, def->base.localFrameA.p.z, def->base.localFrameB.p.x,
								   def->base.localFrameB.p.y, def->base.localFrameB.p.z, def->base.localFrameA.q.v.x,
								   def->base.localFrameA.q.v.y, def->base.localFrameA.q.v.z, def->base.localFrameA.q.s,
								   def->base.localFrameB.q.v.x, def->base.localFrameB.q.v.y,
								   def->base.localFrameB.q.v.z, def->base.localFrameB.q.s, def->base.constraintHertz,
								   def->base.constraintDampingRatio, def->base.collideConnected, def->enableSpring,
								   def->hertz, def->dampingRatio, def->targetRotation.v.x, def->targetRotation.v.y,
								   def->targetRotation.v.z, def->targetRotation.s, def->enableConeLimit, def->coneAngle,
								   def->enableTwistLimit, def->lowerTwistAngle, def->upperTwistAngle, def->enableMotor,
								   def->maxMotorTorque, def->motorVelocity.x, def->motorVelocity.y,
								   def->motorVelocity.z), &def->base);
}

B3_API void b3SphericalJoint_EnableConeLimit(b3JointId id, bool enable) { gpu_b3_spherical_enable_cone_limit(id, enable); }
B3_API bool b3SphericalJoint_IsConeLimitEnabled(b3JointId id) { return gpu_b3_spherical_is_cone_limit_enabled(id); }
B3_API float b3SphericalJoint_GetConeLimit(b3JointId id) { return gpu_b3_spherical_get_cone_limit(id); }
B3_API void b3SphericalJoint_SetConeLimit(b3JointId id, float angle) { gpu_b3_spherical_set_cone_limit(id, angle); }
B3_API float b3SphericalJoint_GetConeAngle(b3JointId id) { return gpu_b3_spherical_get_cone_angle(id); }
B3_API void b3SphericalJoint_EnableTwistLimit(b3JointId id, bool enable) { gpu_b3_spherical_enable_twist_limit(id, enable); }
B3_API bool b3SphericalJoint_IsTwistLimitEnabled(b3JointId id) { return gpu_b3_spherical_is_twist_limit_enabled(id); }
B3_API float b3SphericalJoint_GetLowerTwistLimit(b3JointId id) { return gpu_b3_spherical_get_lower_twist_limit(id); }
B3_API float b3SphericalJoint_GetUpperTwistLimit(b3JointId id) { return gpu_b3_spherical_get_upper_twist_limit(id); }
B3_API void b3SphericalJoint_SetTwistLimits(b3JointId id, float lower, float upper) { gpu_b3_spherical_set_twist_limits(id, lower, upper); }
B3_API float b3SphericalJoint_GetTwistAngle(b3JointId id) { return gpu_b3_spherical_get_twist_angle(id); }
B3_API void b3SphericalJoint_EnableSpring(b3JointId id, bool enable) { gpu_b3_spherical_enable_spring(id, enable); }
B3_API bool b3SphericalJoint_IsSpringEnabled(b3JointId id) { return gpu_b3_spherical_is_spring_enabled(id); }
B3_API void b3SphericalJoint_SetSpringHertz(b3JointId id, float value) { gpu_b3_spherical_set_spring_hertz(id, value); }
B3_API float b3SphericalJoint_GetSpringHertz(b3JointId id) { return gpu_b3_spherical_get_spring_hertz(id); }
B3_API void b3SphericalJoint_SetSpringDampingRatio(b3JointId id, float value) { gpu_b3_spherical_set_spring_damping(id, value); }
B3_API float b3SphericalJoint_GetSpringDampingRatio(b3JointId id) { return gpu_b3_spherical_get_spring_damping(id); }
B3_API void b3SphericalJoint_SetTargetRotation(b3JointId id, b3Quat value) { gpu_b3_spherical_set_target_rotation(id, value.v.x, value.v.y, value.v.z, value.s); }
B3_API b3Quat b3SphericalJoint_GetTargetRotation(b3JointId id)
{
	b3Quat value;
	gpu_b3_spherical_get_target_rotation(id, &value.v.x);
	return value;
}
B3_API void b3SphericalJoint_EnableMotor(b3JointId id, bool enable) { gpu_b3_spherical_enable_motor(id, enable); }
B3_API bool b3SphericalJoint_IsMotorEnabled(b3JointId id) { return gpu_b3_spherical_is_motor_enabled(id); }
B3_API void b3SphericalJoint_SetMotorVelocity(b3JointId id, b3Vec3 value) { gpu_b3_spherical_set_motor_velocity(id, value.x, value.y, value.z); }
B3_API b3Vec3 b3SphericalJoint_GetMotorVelocity(b3JointId id)
{
	b3Vec3 value;
	gpu_b3_spherical_get_motor_velocity(id, &value.x);
	return value;
}
B3_API b3Vec3 b3SphericalJoint_GetMotorTorque(b3JointId id)
{
	b3Vec3 value;
	gpu_b3_spherical_get_motor_torque(id, &value.x);
	return value;
}
B3_API void b3SphericalJoint_SetMaxMotorTorque(b3JointId id, float value) { gpu_b3_spherical_set_max_motor_torque(id, value); }
B3_API float b3SphericalJoint_GetMaxMotorTorque(b3JointId id) { return gpu_b3_spherical_get_max_motor_torque(id); }

B3_API b3JointId b3CreatePrismaticJoint(b3WorldId worldId, const b3PrismaticJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	b3Vec3 axis = b3RotateVector(def->base.localFrameA.q, b3Vec3_axisX);
	return finish_joint_create(gpu_b3_create_prismatic(
		worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.localFrameA.p.x, def->base.localFrameA.p.y,
		def->base.localFrameA.p.z, def->base.localFrameB.p.x, def->base.localFrameB.p.y, def->base.localFrameB.p.z,
		axis.x, axis.y, axis.z, def->base.localFrameA.q.v.x, def->base.localFrameA.q.v.y,
		def->base.localFrameA.q.v.z, def->base.localFrameA.q.s, def->base.localFrameB.q.v.x,
		def->base.localFrameB.q.v.y, def->base.localFrameB.q.v.z, def->base.localFrameB.q.s,
		def->base.constraintHertz, def->base.constraintDampingRatio, def->base.collideConnected, def->enableSpring,
		def->hertz, def->dampingRatio,
		def->targetTranslation, def->enableLimit, def->lowerTranslation, def->upperTranslation, def->enableMotor,
		def->maxMotorForce, def->motorSpeed), &def->base);
}

B3_API b3JointId b3CreateDistanceJoint(b3WorldId worldId, const b3DistanceJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	return finish_joint_create(gpu_b3_create_distance(
		worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.localFrameA.p.x, def->base.localFrameA.p.y,
		def->base.localFrameA.p.z, def->base.localFrameB.p.x, def->base.localFrameB.p.y, def->base.localFrameB.p.z,
		def->base.constraintHertz, def->base.constraintDampingRatio, def->base.collideConnected, def->length,
		def->enableSpring,
		def->lowerSpringForce, def->upperSpringForce, def->hertz, def->dampingRatio, def->enableLimit, def->minLength,
		def->maxLength, def->enableMotor, def->maxMotorForce, def->motorSpeed), &def->base);
}

B3_API b3JointId b3CreateParallelJoint(b3WorldId worldId, const b3ParallelJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	return finish_joint_create(gpu_b3_create_parallel(
		worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.localFrameA.q.v.x,
		def->base.localFrameA.q.v.y, def->base.localFrameA.q.v.z, def->base.localFrameA.q.s,
		def->base.localFrameB.q.v.x, def->base.localFrameB.q.v.y, def->base.localFrameB.q.v.z,
		def->base.localFrameB.q.s, def->hertz, def->dampingRatio, def->maxTorque, def->base.collideConnected),
		&def->base);
}

B3_API b3JointId b3CreateMotorJoint(b3WorldId worldId, const b3MotorJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	return finish_joint_create(gpu_b3_create_motor(
		worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.localFrameA.p.x, def->base.localFrameA.p.y,
		def->base.localFrameA.p.z, def->base.localFrameB.p.x, def->base.localFrameB.p.y, def->base.localFrameB.p.z,
		def->base.localFrameA.q.v.x, def->base.localFrameA.q.v.y, def->base.localFrameA.q.v.z,
		def->base.localFrameA.q.s, def->base.localFrameB.q.v.x, def->base.localFrameB.q.v.y,
		def->base.localFrameB.q.v.z, def->base.localFrameB.q.s, def->linearVelocity.x, def->linearVelocity.y,
		def->linearVelocity.z, def->maxVelocityForce, def->angularVelocity.x, def->angularVelocity.y,
		def->angularVelocity.z, def->maxVelocityTorque, def->linearHertz, def->linearDampingRatio,
		def->maxSpringForce, def->angularHertz, def->angularDampingRatio, def->maxSpringTorque,
		def->base.collideConnected), &def->base);
}

B3_API b3JointId b3CreateFilterJoint(b3WorldId worldId, const b3FilterJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	return finish_joint_create(
		gpu_b3_create_filter(worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.collideConnected), &def->base);
}

B3_API void b3Joint_SetConstraintTuning(b3JointId jointId, float hertz, float dampingRatio)
{
	gpu_b3_joint_set_constraint_tuning(jointId, hertz, dampingRatio);
}

B3_API void b3Joint_GetConstraintTuning(b3JointId jointId, float* hertz, float* dampingRatio)
{
	gpu_b3_joint_get_constraint_tuning(jointId, hertz, dampingRatio);
}

B3_API void b3Joint_SetForceThreshold(b3JointId jointId, float threshold)
{
	gpu_b3_joint_set_force_threshold(jointId, threshold);
}

B3_API float b3Joint_GetForceThreshold(b3JointId jointId)
{
	return gpu_b3_joint_get_force_threshold(jointId);
}

B3_API void b3Joint_SetTorqueThreshold(b3JointId jointId, float threshold)
{
	gpu_b3_joint_set_torque_threshold(jointId, threshold);
}

B3_API float b3Joint_GetTorqueThreshold(b3JointId jointId)
{
	return gpu_b3_joint_get_torque_threshold(jointId);
}

B3_API void b3Joint_SetUserData(b3JointId jointId, void* userData)
{
	gpu_b3_joint_set_user_data(jointId, (uintptr_t)userData);
}

B3_API void* b3Joint_GetUserData(b3JointId jointId)
{
	return (void*)gpu_b3_joint_get_user_data(jointId);
}

B3_API void b3RevoluteJoint_EnableSpring(b3JointId jointId, bool enableSpring)
{
	gpu_b3_revolute_enable_spring(jointId, enableSpring);
}

B3_API bool b3RevoluteJoint_IsSpringEnabled(b3JointId jointId)
{
	return gpu_b3_revolute_is_spring_enabled(jointId);
}

B3_API void b3RevoluteJoint_SetSpringHertz(b3JointId jointId, float hertz)
{
	gpu_b3_revolute_set_spring_hertz(jointId, hertz);
}

B3_API float b3RevoluteJoint_GetSpringHertz(b3JointId jointId)
{
	return gpu_b3_revolute_get_spring_hertz(jointId);
}

B3_API void b3RevoluteJoint_SetSpringDampingRatio(b3JointId jointId, float dampingRatio)
{
	gpu_b3_revolute_set_spring_damping(jointId, dampingRatio);
}

B3_API float b3RevoluteJoint_GetSpringDampingRatio(b3JointId jointId)
{
	return gpu_b3_revolute_get_spring_damping(jointId);
}

B3_API void b3RevoluteJoint_SetTargetAngle(b3JointId jointId, float targetRadians)
{
	gpu_b3_revolute_set_target_angle(jointId, targetRadians);
}

B3_API float b3RevoluteJoint_GetTargetAngle(b3JointId jointId)
{
	return gpu_b3_revolute_get_target_angle(jointId);
}

B3_API float b3RevoluteJoint_GetAngle(b3JointId jointId)
{
	return gpu_b3_revolute_get_angle(jointId);
}

B3_API void b3RevoluteJoint_EnableLimit(b3JointId jointId, bool enableLimit)
{
	gpu_b3_revolute_enable_limit(jointId, enableLimit);
}

B3_API bool b3RevoluteJoint_IsLimitEnabled(b3JointId jointId)
{
	return gpu_b3_revolute_is_limit_enabled(jointId);
}

B3_API float b3RevoluteJoint_GetLowerLimit(b3JointId jointId)
{
	return gpu_b3_revolute_get_lower_limit(jointId);
}

B3_API float b3RevoluteJoint_GetUpperLimit(b3JointId jointId)
{
	return gpu_b3_revolute_get_upper_limit(jointId);
}

B3_API void b3RevoluteJoint_SetLimits(b3JointId jointId, float lowerLimitRadians, float upperLimitRadians)
{
	gpu_b3_revolute_set_limits(jointId, lowerLimitRadians, upperLimitRadians);
}

B3_API void b3RevoluteJoint_EnableMotor(b3JointId jointId, bool enableMotor)
{
	gpu_b3_revolute_enable_motor(jointId, enableMotor);
}

B3_API bool b3RevoluteJoint_IsMotorEnabled(b3JointId jointId)
{
	return gpu_b3_revolute_is_motor_enabled(jointId);
}

B3_API void b3RevoluteJoint_SetMotorSpeed(b3JointId jointId, float motorSpeed)
{
	gpu_b3_revolute_set_motor_speed(jointId, motorSpeed);
}

B3_API float b3RevoluteJoint_GetMotorSpeed(b3JointId jointId)
{
	return gpu_b3_revolute_get_motor_speed(jointId);
}

B3_API float b3RevoluteJoint_GetMotorTorque(b3JointId jointId)
{
	return gpu_b3_revolute_get_motor_torque(jointId);
}

B3_API void b3RevoluteJoint_SetMaxMotorTorque(b3JointId jointId, float torque)
{
	gpu_b3_revolute_set_max_motor_torque(jointId, torque);
}

B3_API float b3RevoluteJoint_GetMaxMotorTorque(b3JointId jointId)
{
	return gpu_b3_revolute_get_max_motor_torque(jointId);
}

B3_API void b3DistanceJoint_SetLength(b3JointId jointId, float length)
{
	gpu_b3_distance_set_length(jointId, length);
}

B3_API float b3DistanceJoint_GetLength(b3JointId jointId)
{
	return gpu_b3_distance_get_length(jointId);
}

B3_API void b3DistanceJoint_EnableSpring(b3JointId jointId, bool enableSpring)
{
	gpu_b3_distance_enable_spring(jointId, enableSpring);
}

B3_API bool b3DistanceJoint_IsSpringEnabled(b3JointId jointId)
{
	return gpu_b3_distance_is_spring_enabled(jointId);
}

B3_API void b3DistanceJoint_SetSpringForceRange(b3JointId jointId, float lowerForce, float upperForce)
{
	gpu_b3_distance_set_spring_force_range(jointId, lowerForce, upperForce);
}

B3_API void b3DistanceJoint_GetSpringForceRange(b3JointId jointId, float* lowerForce, float* upperForce)
{
	gpu_b3_distance_get_spring_force_range(jointId, lowerForce, upperForce);
}

B3_API void b3DistanceJoint_SetSpringHertz(b3JointId jointId, float hertz)
{
	gpu_b3_distance_set_spring_hertz(jointId, hertz);
}

B3_API float b3DistanceJoint_GetSpringHertz(b3JointId jointId)
{
	return gpu_b3_distance_get_spring_hertz(jointId);
}

B3_API void b3DistanceJoint_SetSpringDampingRatio(b3JointId jointId, float dampingRatio)
{
	gpu_b3_distance_set_spring_damping(jointId, dampingRatio);
}

B3_API float b3DistanceJoint_GetSpringDampingRatio(b3JointId jointId)
{
	return gpu_b3_distance_get_spring_damping(jointId);
}

B3_API void b3DistanceJoint_EnableLimit(b3JointId jointId, bool enableLimit)
{
	gpu_b3_distance_enable_limit(jointId, enableLimit);
}

B3_API bool b3DistanceJoint_IsLimitEnabled(b3JointId jointId)
{
	return gpu_b3_distance_is_limit_enabled(jointId);
}

B3_API void b3DistanceJoint_SetLengthRange(b3JointId jointId, float minLength, float maxLength)
{
	gpu_b3_distance_set_length_range(jointId, minLength, maxLength);
}

B3_API float b3DistanceJoint_GetMinLength(b3JointId jointId)
{
	return gpu_b3_distance_get_min_length(jointId);
}

B3_API float b3DistanceJoint_GetMaxLength(b3JointId jointId)
{
	return gpu_b3_distance_get_max_length(jointId);
}

B3_API float b3DistanceJoint_GetCurrentLength(b3JointId jointId)
{
	return gpu_b3_distance_get_current_length(jointId);
}

B3_API void b3DistanceJoint_EnableMotor(b3JointId jointId, bool enableMotor)
{
	gpu_b3_distance_enable_motor(jointId, enableMotor);
}

B3_API bool b3DistanceJoint_IsMotorEnabled(b3JointId jointId)
{
	return gpu_b3_distance_is_motor_enabled(jointId);
}

B3_API void b3DistanceJoint_SetMotorSpeed(b3JointId jointId, float motorSpeed)
{
	gpu_b3_distance_set_motor_speed(jointId, motorSpeed);
}

B3_API float b3DistanceJoint_GetMotorSpeed(b3JointId jointId)
{
	return gpu_b3_distance_get_motor_speed(jointId);
}

B3_API void b3DistanceJoint_SetMaxMotorForce(b3JointId jointId, float force)
{
	gpu_b3_distance_set_max_motor_force(jointId, force);
}

B3_API float b3DistanceJoint_GetMaxMotorForce(b3JointId jointId)
{
	return gpu_b3_distance_get_max_motor_force(jointId);
}

B3_API float b3DistanceJoint_GetMotorForce(b3JointId jointId)
{
	return gpu_b3_distance_get_motor_force(jointId);
}

B3_API void b3ParallelJoint_SetSpringHertz(b3JointId jointId, float hertz)
{
	gpu_b3_parallel_set_spring_hertz(jointId, hertz);
}

B3_API float b3ParallelJoint_GetSpringHertz(b3JointId jointId)
{
	return gpu_b3_parallel_get_spring_hertz(jointId);
}

B3_API void b3ParallelJoint_SetSpringDampingRatio(b3JointId jointId, float dampingRatio)
{
	gpu_b3_parallel_set_spring_damping(jointId, dampingRatio);
}

B3_API float b3ParallelJoint_GetSpringDampingRatio(b3JointId jointId)
{
	return gpu_b3_parallel_get_spring_damping(jointId);
}

B3_API void b3ParallelJoint_SetMaxTorque(b3JointId jointId, float maxTorque)
{
	gpu_b3_parallel_set_max_torque(jointId, maxTorque);
}

B3_API float b3ParallelJoint_GetMaxTorque(b3JointId jointId)
{
	return gpu_b3_parallel_get_max_torque(jointId);
}

B3_API void b3MotorJoint_SetLinearVelocity(b3JointId jointId, b3Vec3 velocity)
{
	gpu_b3_motor_set_linear_velocity(jointId, velocity.x, velocity.y, velocity.z);
}

B3_API b3Vec3 b3MotorJoint_GetLinearVelocity(b3JointId jointId)
{
	return gpu_b3_motor_get_linear_velocity(jointId);
}

B3_API void b3MotorJoint_SetAngularVelocity(b3JointId jointId, b3Vec3 velocity)
{
	gpu_b3_motor_set_angular_velocity(jointId, velocity.x, velocity.y, velocity.z);
}

B3_API b3Vec3 b3MotorJoint_GetAngularVelocity(b3JointId jointId)
{
	return gpu_b3_motor_get_angular_velocity(jointId);
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

MOTOR_SCALAR_API(MaxVelocityForce, max_velocity_force)
MOTOR_SCALAR_API(MaxVelocityTorque, max_velocity_torque)
MOTOR_SCALAR_API(LinearHertz, linear_hertz)
MOTOR_SCALAR_API(LinearDampingRatio, linear_damping)
MOTOR_SCALAR_API(AngularHertz, angular_hertz)
MOTOR_SCALAR_API(AngularDampingRatio, angular_damping)
MOTOR_SCALAR_API(MaxSpringForce, max_spring_force)
MOTOR_SCALAR_API(MaxSpringTorque, max_spring_torque)

#undef MOTOR_SCALAR_API

B3_API void b3PrismaticJoint_EnableSpring(b3JointId jointId, bool enableSpring)
{
	gpu_b3_prismatic_enable_spring(jointId, enableSpring);
}

B3_API bool b3PrismaticJoint_IsSpringEnabled(b3JointId jointId)
{
	return gpu_b3_prismatic_is_spring_enabled(jointId);
}

B3_API void b3PrismaticJoint_SetSpringHertz(b3JointId jointId, float hertz)
{
	gpu_b3_prismatic_set_spring_hertz(jointId, hertz);
}

B3_API float b3PrismaticJoint_GetSpringHertz(b3JointId jointId)
{
	return gpu_b3_prismatic_get_spring_hertz(jointId);
}

B3_API void b3PrismaticJoint_SetSpringDampingRatio(b3JointId jointId, float dampingRatio)
{
	gpu_b3_prismatic_set_spring_damping(jointId, dampingRatio);
}

B3_API float b3PrismaticJoint_GetSpringDampingRatio(b3JointId jointId)
{
	return gpu_b3_prismatic_get_spring_damping(jointId);
}

B3_API void b3PrismaticJoint_SetTargetTranslation(b3JointId jointId, float targetTranslation)
{
	gpu_b3_prismatic_set_target_translation(jointId, targetTranslation);
}

B3_API float b3PrismaticJoint_GetTargetTranslation(b3JointId jointId)
{
	return gpu_b3_prismatic_get_target_translation(jointId);
}

B3_API void b3PrismaticJoint_EnableLimit(b3JointId jointId, bool enableLimit)
{
	gpu_b3_prismatic_enable_limit(jointId, enableLimit);
}

B3_API bool b3PrismaticJoint_IsLimitEnabled(b3JointId jointId)
{
	return gpu_b3_prismatic_is_limit_enabled(jointId);
}

B3_API float b3PrismaticJoint_GetLowerLimit(b3JointId jointId)
{
	return gpu_b3_prismatic_get_lower_limit(jointId);
}

B3_API float b3PrismaticJoint_GetUpperLimit(b3JointId jointId)
{
	return gpu_b3_prismatic_get_upper_limit(jointId);
}

B3_API void b3PrismaticJoint_SetLimits(b3JointId jointId, float lower, float upper)
{
	gpu_b3_prismatic_set_limits(jointId, lower, upper);
}

B3_API void b3PrismaticJoint_EnableMotor(b3JointId jointId, bool enableMotor)
{
	gpu_b3_prismatic_enable_motor(jointId, enableMotor);
}

B3_API bool b3PrismaticJoint_IsMotorEnabled(b3JointId jointId)
{
	return gpu_b3_prismatic_is_motor_enabled(jointId);
}

B3_API void b3PrismaticJoint_SetMotorSpeed(b3JointId jointId, float motorSpeed)
{
	gpu_b3_prismatic_set_motor_speed(jointId, motorSpeed);
}

B3_API float b3PrismaticJoint_GetMotorSpeed(b3JointId jointId)
{
	return gpu_b3_prismatic_get_motor_speed(jointId);
}

B3_API void b3PrismaticJoint_SetMaxMotorForce(b3JointId jointId, float force)
{
	gpu_b3_prismatic_set_max_motor_force(jointId, force);
}

B3_API float b3PrismaticJoint_GetMaxMotorForce(b3JointId jointId)
{
	return gpu_b3_prismatic_get_max_motor_force(jointId);
}

B3_API float b3PrismaticJoint_GetMotorForce(b3JointId jointId)
{
	return gpu_b3_prismatic_get_motor_force(jointId);
}

B3_API float b3PrismaticJoint_GetTranslation(b3JointId jointId)
{
	return gpu_b3_prismatic_get_translation(jointId);
}

B3_API float b3PrismaticJoint_GetSpeed(b3JointId jointId)
{
	return gpu_b3_prismatic_get_speed(jointId);
}

B3_API b3JointId b3CreateWeldJoint(b3WorldId worldId, const b3WeldJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	return finish_joint_create(gpu_b3_create_weld(
		worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.localFrameA.p.x, def->base.localFrameA.p.y,
		def->base.localFrameA.p.z, def->base.localFrameB.p.x, def->base.localFrameB.p.y, def->base.localFrameB.p.z,
		def->base.localFrameA.q.v.x, def->base.localFrameA.q.v.y, def->base.localFrameA.q.v.z,
		def->base.localFrameA.q.s, def->base.localFrameB.q.v.x, def->base.localFrameB.q.v.y,
		def->base.localFrameB.q.v.z, def->base.localFrameB.q.s, def->base.constraintHertz,
		def->base.constraintDampingRatio, def->base.collideConnected, def->linearHertz, def->linearDampingRatio,
		def->angularHertz, def->angularDampingRatio), &def->base);
}

B3_API void b3WeldJoint_SetLinearHertz(b3JointId jointId, float hertz)
{
	gpu_b3_weld_set_linear_hertz(jointId, hertz);
}

B3_API float b3WeldJoint_GetLinearHertz(b3JointId jointId)
{
	return gpu_b3_weld_get_linear_hertz(jointId);
}

B3_API void b3WeldJoint_SetLinearDampingRatio(b3JointId jointId, float dampingRatio)
{
	gpu_b3_weld_set_linear_damping(jointId, dampingRatio);
}

B3_API float b3WeldJoint_GetLinearDampingRatio(b3JointId jointId)
{
	return gpu_b3_weld_get_linear_damping(jointId);
}

B3_API void b3WeldJoint_SetAngularHertz(b3JointId jointId, float hertz)
{
	gpu_b3_weld_set_angular_hertz(jointId, hertz);
}

B3_API float b3WeldJoint_GetAngularHertz(b3JointId jointId)
{
	return gpu_b3_weld_get_angular_hertz(jointId);
}

B3_API void b3WeldJoint_SetAngularDampingRatio(b3JointId jointId, float dampingRatio)
{
	gpu_b3_weld_set_angular_damping(jointId, dampingRatio);
}

B3_API float b3WeldJoint_GetAngularDampingRatio(b3JointId jointId)
{
	return gpu_b3_weld_get_angular_damping(jointId);
}

B3_API b3Pos b3Body_GetPosition(b3BodyId bodyId)
{
	float p[3] = {0};
	gpu_b3_body_get_position(bodyId, p);
	b3Pos out = {p[0], p[1], p[2]};
	return out;
}

B3_API b3Quat b3Body_GetRotation(b3BodyId bodyId)
{
	float q[4] = {0, 0, 0, 1};
	gpu_b3_body_get_rotation(bodyId, q);
	b3Quat out;
	out.v.x = q[0];
	out.v.y = q[1];
	out.v.z = q[2];
	out.s = q[3];
	return out;
}

extern void gpu_b3_world_set_contact_recycle_distance(b3WorldId id, float distance);
extern float gpu_b3_world_get_contact_recycle_distance(b3WorldId id);
B3_API void b3World_SetContactRecycleDistance(b3WorldId id, float distance) {
    gpu_b3_world_set_contact_recycle_distance(id, distance);
}
B3_API float b3World_GetContactRecycleDistance(b3WorldId id) {
    return gpu_b3_world_get_contact_recycle_distance(id);
}

extern void gpu_b3_destroy_body(b3BodyId id);
extern void gpu_samples_destroy_body(b3BodyId id) __attribute__((weak));
void gpu_shape_clear_body_geometry(b3BodyId);
B3_API void b3DestroyBody(b3BodyId id) {
    gpu_shape_clear_body_geometry(id);
    if (gpu_samples_destroy_body) {
        gpu_samples_destroy_body(id);
    } else {
        gpu_b3_destroy_body(id);
    }
}

extern b3Matrix3 gpu_b3_body_get_local_rotational_inertia(b3BodyId body);
extern b3Matrix3 gpu_b3_body_get_world_inverse_rotational_inertia(b3BodyId body);
B3_API b3Matrix3 b3Body_GetLocalRotationalInertia(b3BodyId bodyId)
{
    return gpu_b3_body_get_local_rotational_inertia(bodyId);
}
B3_API b3Matrix3 b3Body_GetWorldInverseRotationalInertia(b3BodyId bodyId)
{
    return gpu_b3_body_get_world_inverse_rotational_inertia(bodyId);
}

extern void gpu_b3_shape_set_density(b3ShapeId id, float density, bool updateMass);
B3_API void b3Shape_SetDensity(b3ShapeId id, float density, bool updateMass)
{
    gpu_b3_shape_set_density(id, density, updateMass);
}

extern b3WorldId gpu_b3_body_get_world(b3BodyId id);
extern b3WorldId gpu_b3_shape_get_world(b3ShapeId id);
extern b3WorldId gpu_b3_joint_get_world(b3JointId id);
extern uint32_t gpu_b3_joint_get_kind(b3JointId id);
extern b3BodyId gpu_b3_joint_get_body(b3JointId id, bool second);
B3_API b3WorldId b3Body_GetWorld(b3BodyId id) { return gpu_b3_body_get_world(id); }
B3_API b3WorldId b3Shape_GetWorld(b3ShapeId id) { return gpu_b3_shape_get_world(id); }
B3_API b3WorldId b3Joint_GetWorld(b3JointId id) { return gpu_b3_joint_get_world(id); }
B3_API b3BodyId b3Joint_GetBodyA(b3JointId id) { return gpu_b3_joint_get_body(id, false); }
B3_API b3BodyId b3Joint_GetBodyB(b3JointId id) { return gpu_b3_joint_get_body(id, true); }
B3_API b3JointType b3Joint_GetType(b3JointId id) {
    switch (gpu_b3_joint_get_kind(id)) {
        case 1: return b3_revoluteJoint; case 2: return b3_weldJoint;
        case 3: return b3_sphericalJoint; case 4: return b3_prismaticJoint;
        case 5: return b3_filterJoint; case 6: return b3_distanceJoint;
        case 7: return b3_parallelJoint; case 8: return b3_motorJoint;
        case 9: return b3_wheelJoint; default: return b3_parallelJoint;
    }
}

B3_API void b3World_SetMaximumLinearSpeed(b3WorldId id, float speed) {
    gpu_b3_world_set_maximum_linear_speed(id, speed);
}
B3_API float b3World_GetMaximumLinearSpeed(b3WorldId id) {
    return gpu_b3_world_get_maximum_linear_speed(id);
}

extern void gpu_b3_joint_set_collide_connected(GpuJointId, bool);
extern bool gpu_b3_joint_get_collide_connected(GpuJointId);
B3_API bool b3Joint_GetCollideConnected(b3JointId id) { return gpu_b3_joint_get_collide_connected(id); }
B3_API void b3Joint_SetCollideConnected(b3JointId id, bool enable) {
    gpu_b3_joint_set_collide_connected(id, enable);
}

B3_API float b3World_GetRestitutionThreshold(b3WorldId id) { return gpu_b3_world_get_restitution_threshold(id); }
B3_API void b3World_SetRestitutionThreshold(b3WorldId id, float value) {
    gpu_b3_world_set_restitution_threshold(id, value);
}

extern void gpu_b3_world_set_contact_tuning(b3WorldId id, float hertz, float damping, float speed);
B3_API void b3World_SetContactTuning(b3WorldId id, float hertz, float damping, float speed) { gpu_b3_world_set_contact_tuning(id, hertz, damping, speed); }

extern void gpu_b3_world_set_user_data(b3WorldId id, uintptr_t value);
B3_API void b3World_SetUserData(b3WorldId id, void* value) { gpu_b3_world_set_user_data(id, (uintptr_t)value); }

extern uintptr_t gpu_b3_world_get_user_data(b3WorldId id);
B3_API void* b3World_GetUserData(b3WorldId id) { return (void*)gpu_b3_world_get_user_data(id); }

extern int gpu_b3_world_get_awake_body_count(b3WorldId id);
B3_API int b3World_GetAwakeBodyCount(b3WorldId id) { return gpu_b3_world_get_awake_body_count(id); }

extern void gpu_b3_body_enable_sleep(b3BodyId id, bool enable);
B3_API void b3Body_EnableSleep(b3BodyId id, bool enable) { gpu_b3_body_enable_sleep(id, enable); }

extern bool gpu_b3_body_is_sleep_enabled(b3BodyId id);
B3_API bool b3Body_IsSleepEnabled(b3BodyId id) { return gpu_b3_body_is_sleep_enabled(id); }

extern void gpu_b3_body_set_sleep_threshold(b3BodyId id, float value);
B3_API void b3Body_SetSleepThreshold(b3BodyId id, float value) { gpu_b3_body_set_sleep_threshold(id, value); }

extern float gpu_b3_body_get_sleep_threshold(b3BodyId id);
B3_API float b3Body_GetSleepThreshold(b3BodyId id) { return gpu_b3_body_get_sleep_threshold(id); }

extern void gpu_b3_body_enable_hit_events(b3BodyId id, bool enable);
B3_API void b3Body_EnableHitEvents(b3BodyId id, bool enable) { gpu_b3_body_enable_hit_events(id, enable); }

extern void gpu_b3_body_set_name(b3BodyId id, const char* name);
B3_API void b3Body_SetName(b3BodyId id, const char* name) { gpu_b3_body_set_name(id, name); }

extern const char* gpu_b3_body_get_name(b3BodyId id);
B3_API const char* b3Body_GetName(b3BodyId id) { return gpu_b3_body_get_name(id); }

extern void gpu_b3_joint_set_local_frame(b3JointId, bool, float, float, float, float, float, float, float);
extern void gpu_b3_joint_get_local_frame(b3JointId, bool, float*);
extern void gpu_b3_joint_wake_bodies(b3JointId);
B3_API void b3Joint_SetLocalFrameA(b3JointId id, b3Transform frame) {
    gpu_b3_joint_set_local_frame(id, false, frame.p.x, frame.p.y, frame.p.z, frame.q.v.x, frame.q.v.y, frame.q.v.z, frame.q.s);
}
B3_API b3Transform b3Joint_GetLocalFrameA(b3JointId id) {
    float f[7]; gpu_b3_joint_get_local_frame(id, false, f);
    return (b3Transform){{f[0],f[1],f[2]},{{f[3],f[4],f[5]},f[6]}};
}
B3_API void b3Joint_SetLocalFrameB(b3JointId id, b3Transform frame) {
    gpu_b3_joint_set_local_frame(id, true, frame.p.x, frame.p.y, frame.p.z, frame.q.v.x, frame.q.v.y, frame.q.v.z, frame.q.s);
}
B3_API b3Transform b3Joint_GetLocalFrameB(b3JointId id) {
    float f[7]; gpu_b3_joint_get_local_frame(id, true, f);
    return (b3Transform){{f[0],f[1],f[2]},{{f[3],f[4],f[5]},f[6]}};
}
B3_API void b3Joint_WakeBodies(b3JointId id) { gpu_b3_joint_wake_bodies(id);
}

extern b3MassData gpu_b3_shape_compute_mass_data(b3ShapeId);
B3_API b3MassData b3Shape_ComputeMassData(b3ShapeId id) { return gpu_b3_shape_compute_mass_data(id); }

extern b3BodyId gpu_b3_shape_get_body(b3ShapeId);
extern bool b3IsValidHull(const b3HullData*);
extern bool gpu_b3_set_sphere(b3ShapeId, b3Sphere);
extern bool gpu_b3_set_capsule(b3ShapeId, b3Capsule);
extern bool gpu_b3_set_box_hull(b3ShapeId, float, float, float, float, float, float);
extern bool gpu_b3_set_convex_hull(b3ShapeId, const float*, int, const float*, int, const uint8_t*, int,
    float, float, float, float, float, float, float, float, float, float, float,
    float, float, float, float, float, float);
void gpu_samples_on_shape_replaced(b3ShapeId, b3BodyId, b3ShapeType, const b3Sphere*, const b3Capsule*, const b3HullData*) __attribute__((weak));

void gpu_shape_clear_geometry(b3ShapeId id)
{
    if (id.index1 <= 0) { return; }
    // Baked compound children are hidden public colliders but still own mirrors.
    for (int i = 1; i < GPU_HULL_MIRROR_CAP; ++i) {
        if (g_gpu_hulls[i].world0 == id.world0 && g_gpu_hulls[i].parent == id.index1) {
            free(g_gpu_hulls[i].data); g_gpu_hulls[i] = (GpuHullMirror){0};
        }
        if (g_gpu_meshes[i].world0 == id.world0 && g_gpu_meshes[i].parent == id.index1) {
            free(g_gpu_meshes[i].materials); g_gpu_meshes[i] = (GpuMeshMirror){0};
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

B3_API void b3Shape_SetSphere(b3ShapeId id, const b3Sphere* sphere) { gpu_shape_set_sphere(id, sphere); }
B3_API void b3Shape_SetCapsule(b3ShapeId id, const b3Capsule* capsule) { gpu_shape_set_capsule(id, capsule); }
B3_API void b3Shape_SetHull(b3ShapeId id, const b3HullData* hull) { gpu_shape_set_hull(id, hull); }

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
    for (int i = 1; i < GPU_HULL_MIRROR_CAP; ++i) {
        count += g_gpu_hulls[i].data != NULL;
        count += g_gpu_meshes[i].index1 != 0;
        count += g_gpu_height_fields[i].index1 != 0;
    }
    return count;
}

extern float gpu_b3_joint_get_linear_separation(b3JointId);
B3_API float b3Joint_GetLinearSeparation(b3JointId id) { return gpu_b3_joint_get_linear_separation(id); }

extern float gpu_b3_joint_get_angular_separation(b3JointId);
B3_API float b3Joint_GetAngularSeparation(b3JointId id) { return gpu_b3_joint_get_angular_separation(id); }
