#include "box3d/box3d.h"
#include "both_ids.h"
#include "both_view.h"
#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "native_clock.h"

extern void gpu_shape_set_sphere(b3ShapeId, const b3Sphere*);
extern void gpu_shape_set_capsule(b3ShapeId, const b3Capsule*);
extern void gpu_shape_set_hull(b3ShapeId, const b3HullData*);
extern void gpu_shape_mirror_parent(b3ShapeId, b3ShapeId);
extern void gpu_shape_mirror_hull(b3ShapeId, const b3HullData*);
extern const b3HullData* gpu_shape_get_hull(b3ShapeId);
extern void cpu_b3Shape_SetSphere(b3ShapeId, const b3Sphere*);
extern void cpu_b3Shape_SetCapsule(b3ShapeId, const b3Capsule*);
extern void cpu_b3Shape_SetHull(b3ShapeId, const b3HullData*);
extern void gpu_shape_clear_geometry(b3ShapeId);
extern void gpu_shape_clear_body_geometry(b3BodyId);
extern void gpu_shape_clear_world_geometry(b3WorldId);
static float g_gpu_step_ms;

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

extern bool gpu_b3_joint_is_valid(GpuJointId joint);
extern void gpu_b3_destroy_joint(GpuJointId joint, bool wake_attached);
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
extern GpuBodyId gpu_b3_create_body(GpuWorldId world, int body_type, float px, float py, float pz, float rx, float ry,
								   float rz, float rw, float vx, float vy, float vz, float wx, float wy, float wz,
								   float gravity_scale, uint32_t locks);
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
extern void gpu_b3_shape_enable_contact_events(GpuShapeId id, bool enable);
extern void gpu_b3_shape_enable_speculative_contact(GpuShapeId id, bool enable);
extern bool gpu_b3_shape_are_contact_events_enabled(GpuShapeId id);
extern void gpu_b3_shape_enable_custom_filtering(GpuShapeId id, bool enable);
extern void gpu_b3_shape_enable_pre_solve_events(GpuShapeId id, bool enable);
extern bool gpu_b3_shape_are_pre_solve_events_enabled(GpuShapeId id);
extern void gpu_b3_shape_enable_hit_events(GpuShapeId id, bool enable);
extern bool gpu_b3_shape_are_hit_events_enabled(GpuShapeId id);
extern void gpu_b3_shape_set_user_material_id(GpuShapeId id, uint64_t value);
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
	GpuWorldId, GpuBodyId, GpuBodyId, float, float, float, float, float, float, float, float, float, float, float, float,
	float, float, float, float, bool, bool, float, float, bool, float, float, bool, float, float, bool, float, float,
	float, float, bool, float, float);
#define GPU_WHEEL_BOOL_DECL(s) extern void gpu_b3_wheel_enable_##s(GpuJointId, bool); extern bool gpu_b3_wheel_is_##s##_enabled(GpuJointId)
#define GPU_WHEEL_SCALAR_DECL(s) extern void gpu_b3_wheel_set_##s(GpuJointId, float); extern float gpu_b3_wheel_get_##s(GpuJointId)
GPU_WHEEL_BOOL_DECL(suspension);
GPU_WHEEL_BOOL_DECL(suspension_limit);
GPU_WHEEL_BOOL_DECL(spin_motor);
GPU_WHEEL_BOOL_DECL(steering);
GPU_WHEEL_BOOL_DECL(steering_limit);
GPU_WHEEL_SCALAR_DECL(suspension_hertz);
GPU_WHEEL_SCALAR_DECL(suspension_damping);
GPU_WHEEL_SCALAR_DECL(max_spin_torque);
GPU_WHEEL_SCALAR_DECL(steering_hertz);
GPU_WHEEL_SCALAR_DECL(steering_damping);
GPU_WHEEL_SCALAR_DECL(max_steering_torque);
#undef GPU_WHEEL_BOOL_DECL
#undef GPU_WHEEL_SCALAR_DECL
extern void gpu_b3_wheel_set_spin_speed(GpuJointId, float);
extern float gpu_b3_wheel_get_spin_speed_setting(GpuJointId);
extern void gpu_b3_wheel_set_target_steering(GpuJointId, float);
extern float gpu_b3_wheel_get_target_steering(GpuJointId);
extern void gpu_b3_wheel_set_suspension_limits(GpuJointId, float, float);
extern float gpu_b3_wheel_get_lower_suspension_limit(GpuJointId);
extern float gpu_b3_wheel_get_upper_suspension_limit(GpuJointId);
extern void gpu_b3_wheel_set_steering_limits(GpuJointId, float, float);
extern float gpu_b3_wheel_get_lower_steering_limit(GpuJointId);
extern float gpu_b3_wheel_get_upper_steering_limit(GpuJointId);
extern float gpu_b3_wheel_get_live_spin_speed(GpuJointId);
extern float gpu_b3_wheel_get_spin_torque(GpuJointId);
extern float gpu_b3_wheel_get_steering_angle(GpuJointId);
extern float gpu_b3_wheel_get_steering_torque(GpuJointId);
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
extern void gpu_b3_distance_enable_spring(GpuJointId id, bool enable);
extern void gpu_b3_distance_set_spring_force_range(GpuJointId id, float lower, float upper);
extern void gpu_b3_distance_set_spring_hertz(GpuJointId id, float hertz);
extern void gpu_b3_distance_set_spring_damping(GpuJointId id, float damping);
extern void gpu_b3_distance_enable_limit(GpuJointId id, bool enable);
extern void gpu_b3_distance_set_length_range(GpuJointId id, float min_length, float max_length);
extern void gpu_b3_distance_enable_motor(GpuJointId id, bool enable);
extern void gpu_b3_distance_set_motor_speed(GpuJointId id, float speed);
extern void gpu_b3_distance_set_max_motor_force(GpuJointId id, float force);
extern GpuJointId gpu_b3_create_parallel(GpuWorldId world, GpuBodyId a, GpuBodyId b, float qa_x, float qa_y,
										 float qa_z, float qa_w, float qb_x, float qb_y, float qb_z, float qb_w,
										 float hertz, float damping, float max_torque, bool collide_connected);
extern void gpu_b3_parallel_set_spring_hertz(GpuJointId id, float hertz);
extern void gpu_b3_parallel_set_spring_damping(GpuJointId id, float damping);
extern void gpu_b3_parallel_set_max_torque(GpuJointId id, float torque);
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
extern void gpu_b3_prismatic_set_spring_hertz(GpuJointId id, float hertz);
extern void gpu_b3_prismatic_set_spring_damping(GpuJointId id, float damping);
extern void gpu_b3_prismatic_set_target_translation(GpuJointId id, float translation);
extern void gpu_b3_prismatic_enable_limit(GpuJointId id, bool enable);
extern void gpu_b3_prismatic_set_limits(GpuJointId id, float lower, float upper);
extern void gpu_b3_prismatic_enable_motor(GpuJointId id, bool enable);
extern void gpu_b3_prismatic_set_motor_speed(GpuJointId id, float speed);
extern void gpu_b3_prismatic_set_max_motor_force(GpuJointId id, float force);
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
extern bool gpu_b3_body_is_valid(GpuBodyId body);
extern void gpu_b3_body_set_linear_velocity(GpuBodyId body, float x, float y, float z);
extern void gpu_b3_body_set_angular_velocity(GpuBodyId body, float x, float y, float z);
extern b3Vec3 gpu_b3_body_get_linear_velocity(GpuBodyId body);
extern b3Vec3 gpu_b3_body_get_angular_velocity(GpuBodyId body);
extern void gpu_b3_body_apply_linear_impulse(GpuBodyId body, float ix, float iy, float iz, float px, float py,
											 float pz, bool wake);
extern void gpu_b3_body_apply_linear_impulse_to_center(GpuBodyId body, float x, float y, float z, bool wake);
extern void gpu_b3_body_apply_angular_impulse(GpuBodyId body, float x, float y, float z, bool wake);
extern void gpu_b3_body_apply_force(GpuBodyId body, float fx, float fy, float fz, float px, float py, float pz,
									 bool wake);
extern void gpu_b3_body_apply_force_to_center(GpuBodyId body, float x, float y, float z, bool wake);
extern void gpu_b3_body_apply_torque(GpuBodyId body, float x, float y, float z, bool wake);
extern float gpu_b3_body_get_mass(GpuBodyId body);
extern float gpu_b3_body_inv_mass(GpuBodyId body);
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
extern void gpu_b3_body_set_target_transform(GpuBodyId body, float px, float py, float pz, float rx, float ry,
											float rz, float rw, float time_step, bool wake);
extern int gpu_b3_body_get_type(GpuBodyId body);
extern void gpu_b3_body_set_type(GpuBodyId body, int type);
extern void gpu_b3_body_set_transform(GpuBodyId body, float px, float py, float pz, float rx, float ry, float rz,
									 float rw);
extern void gpu_b3_body_set_awake(GpuBodyId body, bool awake);
extern void gpu_b3_body_set_bullet(GpuBodyId body, bool bullet);
extern bool gpu_b3_body_is_bullet(GpuBodyId body);
extern void gpu_b3_body_allow_fast_rotation(GpuBodyId body, bool allow);
extern bool gpu_b3_body_is_fast_rotation_allowed(GpuBodyId body);
extern void gpu_b3_body_set_linear_damping(GpuBodyId body, float damping);
extern void gpu_b3_body_set_angular_damping(GpuBodyId body, float damping);
extern void gpu_b3_body_set_gravity_scale(GpuBodyId body, float scale);
extern void gpu_b3_body_get_world_center(GpuBodyId body, float* out);
extern void gpu_b3_body_apply_mass_from_shapes(GpuBodyId body);
extern void gpu_b3_body_set_user_data(GpuBodyId body, uintptr_t user_data);
extern uintptr_t gpu_b3_body_get_user_data(GpuBodyId body);
extern void cpu_b3Body_SetLinearVelocity(b3BodyId bodyId, b3Vec3 linearVelocity);
extern void cpu_b3Body_SetAngularVelocity(b3BodyId bodyId, b3Vec3 angularVelocity);
extern void cpu_b3Body_ApplyLinearImpulse(b3BodyId bodyId, b3Vec3 impulse, b3Pos point, bool wake);
extern void cpu_b3Body_ApplyLinearImpulseToCenter(b3BodyId bodyId, b3Vec3 impulse, bool wake);
extern void cpu_b3Body_ApplyAngularImpulse(b3BodyId bodyId, b3Vec3 impulse, bool wake);
extern void cpu_b3Body_ApplyForce(b3BodyId bodyId, b3Vec3 force, b3Pos point, bool wake);
extern void cpu_b3Body_ApplyForceToCenter(b3BodyId bodyId, b3Vec3 force, bool wake);
extern void cpu_b3Body_ApplyTorque(b3BodyId bodyId, b3Vec3 torque, bool wake);
extern float cpu_b3Body_GetMass(b3BodyId bodyId);
extern float cpu_b3Body_GetInverseMass(b3BodyId bodyId);
extern b3MassData cpu_b3Body_GetMassData(b3BodyId bodyId);
extern void cpu_b3Body_SetMassData(b3BodyId bodyId, b3MassData massData);
extern b3Vec3 cpu_b3Body_GetLocalPoint(b3BodyId bodyId, b3Pos worldPoint);
extern b3Pos cpu_b3Body_GetWorldPoint(b3BodyId bodyId, b3Vec3 localPoint);
extern b3Vec3 cpu_b3Body_GetLocalVector(b3BodyId bodyId, b3Vec3 worldVector);
extern b3Vec3 cpu_b3Body_GetWorldVector(b3BodyId bodyId, b3Vec3 localVector);
extern b3Vec3 cpu_b3Body_GetWorldPointVelocity(b3BodyId bodyId, b3Pos worldPoint);
extern b3Vec3 cpu_b3Body_GetLocalPointVelocity(b3BodyId bodyId, b3Vec3 localPoint);
extern bool cpu_b3Body_IsAwake(b3BodyId bodyId);
extern bool cpu_b3Body_IsEnabled(b3BodyId bodyId);
extern void cpu_b3Body_Disable(b3BodyId bodyId);
extern void cpu_b3Body_Enable(b3BodyId bodyId);
extern void cpu_b3Body_SetMotionLocks(b3BodyId bodyId, b3MotionLocks locks);
extern b3MotionLocks cpu_b3Body_GetMotionLocks(b3BodyId bodyId);
extern b3Vec3 cpu_b3Body_GetLocalCenter(b3BodyId bodyId);
extern float cpu_b3Body_GetLinearDamping(b3BodyId bodyId);
extern float cpu_b3Body_GetAngularDamping(b3BodyId bodyId);
extern float cpu_b3Body_GetGravityScale(b3BodyId bodyId);
extern int cpu_b3Body_GetJointCount(b3BodyId bodyId);
extern int cpu_b3Body_GetJoints(b3BodyId bodyId, b3JointId* jointArray, int capacity);
extern void cpu_b3World_SetGravity(b3WorldId worldId, b3Vec3 gravity);
extern b3Vec3 cpu_b3World_GetGravity(b3WorldId worldId);
extern void cpu_b3Shape_SetFriction(b3ShapeId shapeId, float friction);
extern void cpu_b3Shape_SetRestitution(b3ShapeId shapeId, float restitution);
extern void cpu_b3Shape_ApplyWind(b3ShapeId shapeId, b3Vec3 wind, float drag, float lift, float maxSpeed,
								  bool wake);
extern void cpu_b3Body_ApplyMassFromShapes(b3BodyId bodyId);
extern b3BodyType cpu_b3Body_GetType(b3BodyId bodyId);
extern void cpu_b3Body_SetType(b3BodyId bodyId, b3BodyType type);
extern void cpu_b3Body_SetBullet(b3BodyId bodyId, bool flag);
extern bool cpu_b3Body_IsBullet(b3BodyId bodyId);
extern void cpu_b3Body_AllowFastRotation(b3BodyId bodyId, bool flag);
extern bool cpu_b3Body_IsFastRotationAllowed(b3BodyId bodyId);
extern void cpu_b3World_EnableContinuous(b3WorldId worldId, bool flag);
extern void cpu_b3Body_SetTransform(b3BodyId bodyId, b3Pos position, b3Quat rotation);
extern void cpu_b3Body_SetAwake(b3BodyId bodyId, bool awake);
extern void cpu_b3Body_SetLinearDamping(b3BodyId bodyId, float damping);
extern void cpu_b3Body_SetAngularDamping(b3BodyId bodyId, float damping);
extern void cpu_b3Body_SetGravityScale(b3BodyId bodyId, float scale);

extern void gpu_samples_on_world_created(b3WorldId world, const b3WorldDef* def);
extern void gpu_samples_on_world_destroyed(b3WorldId world);
extern void gpu_samples_on_shape_created(b3ShapeId shapeId, b3BodyId bodyId, b3ShapeType type, const b3Sphere* sphere,
										 const b3Capsule* capsule, const b3HullData* hull);
extern void gpu_samples_on_mesh_shape_created(b3ShapeId shapeId, b3BodyId bodyId, const b3MeshData* mesh, b3Vec3 scale);
extern void gpu_samples_on_compound_shape_created(b3ShapeId shapeId, b3BodyId bodyId, const b3CompoundData* compound);
extern void gpu_samples_on_shape_destroyed(b3ShapeId shapeId);
extern void gpu_samples_world_draw(b3WorldId worldId, b3DebugDraw* draw, uint64_t maskBits);
extern b3AABB gpu_samples_world_bounds(b3WorldId worldId);
extern void gpu_samples_shape_set_name(b3ShapeId shapeId, const char* name);
extern const char* gpu_samples_shape_get_name(b3ShapeId shapeId);
extern void gpu_samples_destroy_body(b3BodyId bodyId);
extern b3WorldTransform gpu_samples_body_transform(b3BodyId bodyId);
extern void gpu_samples_body_set_transform(b3BodyId bodyId, b3WorldTransform target);

static bool g_split = true;
static bool g_second_view;
static b3DebugDraw g_split_draw;
static b3WorldId g_draw_world;
static uint64_t g_draw_mask;
typedef struct BothDrag {
    b3WorldId world;
    b3BodyId mouse[2], grabbed[2], selected[2];
    b3JointId joint[2];
    float fraction[2];
    b3Vec3 local_anchor[2];
    b3Pos origin;
    b3Vec3 translation;
} BothDrag;
static BothDrag g_drag;
static void both_drag_step(b3WorldId world, float dt);
bool both_split_enabled(void) { return g_split; }
void both_set_split(bool enabled) { both_pointer_up(); g_split = enabled; }
void both_begin_frame(void) { g_draw_world = (b3WorldId){0}; g_second_view = false; }

extern b3WorldId cpu_b3CreateWorld(const b3WorldDef* def);
extern void cpu_b3DestroyWorld(b3WorldId worldId);
extern bool cpu_b3World_IsValid(b3WorldId id);
extern void cpu_b3World_SetFrictionCallback(b3WorldId worldId, b3FrictionCallback* callback);
extern void cpu_b3World_SetRestitutionCallback(b3WorldId worldId, b3RestitutionCallback* callback);
extern void cpu_b3World_Step(b3WorldId worldId, float timeStep, int subStepCount);
extern void cpu_b3World_Explode(b3WorldId worldId, const b3ExplosionDef* explosionDef);
extern void cpu_b3World_Draw(b3WorldId worldId, b3DebugDraw* draw, uint64_t maskBits);
extern b3AABB cpu_b3World_GetBounds(b3WorldId worldId);
extern void cpu_b3World_EnableSleeping(b3WorldId worldId, bool flag);
extern bool cpu_b3World_IsSleepingEnabled(b3WorldId worldId);
extern void cpu_b3World_SetHitEventThreshold(b3WorldId worldId, float value);
extern float cpu_b3World_GetHitEventThreshold(b3WorldId worldId);
extern b3Profile cpu_b3World_GetProfile(b3WorldId worldId);
extern b3Counters cpu_b3World_GetCounters(b3WorldId worldId);
extern b3BodyId cpu_b3CreateBody(b3WorldId worldId, const b3BodyDef* def);
extern void cpu_b3DestroyBody(b3BodyId bodyId);
extern void cpu_b3Body_SetUserData(b3BodyId bodyId, void* userData);
extern b3Vec3 cpu_b3Body_GetLinearVelocity(b3BodyId bodyId);
extern b3Vec3 cpu_b3Body_GetAngularVelocity(b3BodyId bodyId);
extern b3ShapeId cpu_b3CreateSphereShape(b3BodyId bodyId, const b3ShapeDef* def, const b3Sphere* sphere);
extern b3ShapeId cpu_b3CreateHullShape(b3BodyId bodyId, const b3ShapeDef* def, const b3HullData* hull);
extern b3ShapeId cpu_b3CreateCapsuleShape(b3BodyId bodyId, const b3ShapeDef* def, const b3Capsule* capsule);
extern b3ShapeId cpu_b3CreateMeshShape(b3BodyId bodyId, const b3ShapeDef* def, const b3MeshData* mesh, b3Vec3 scale);
extern b3ShapeId cpu_b3CreateHeightFieldShape(b3BodyId bodyId, const b3ShapeDef* def,
											 const b3HeightFieldData* heightField);
extern b3ShapeId cpu_b3CreateBakedCompoundShape(b3BodyId bodyId, b3ShapeDef* def, const b3CompoundData* compound);
extern const b3HeightFieldData* cpu_b3Shape_GetHeightField(b3ShapeId shapeId);
extern b3Mesh cpu_b3Shape_GetMesh(b3ShapeId shapeId);
extern int cpu_b3Shape_GetMeshMaterialCount(b3ShapeId shapeId);
extern void cpu_b3Shape_SetMeshMaterial(b3ShapeId shapeId, b3SurfaceMaterial material, int index);
extern b3SurfaceMaterial cpu_b3Shape_GetMeshSurfaceMaterial(b3ShapeId shapeId, int index);
extern void cpu_b3Shape_SetSurfaceMaterial(b3ShapeId shapeId, b3SurfaceMaterial material);
extern b3SurfaceMaterial cpu_b3Shape_GetSurfaceMaterial(b3ShapeId shapeId);
extern void cpu_b3Shape_SetMesh(b3ShapeId shapeId, const b3MeshData* mesh, b3Vec3 scale);
extern void cpu_b3Shape_SetFilter(b3ShapeId shapeId, b3Filter filter, bool invokeContacts);
extern void cpu_b3Shape_EnableSensorEvents(b3ShapeId shapeId, bool flag);
extern void cpu_b3Shape_EnableContactEvents(b3ShapeId shapeId, bool flag);
extern void cpu_b3Shape_EnableHitEvents(b3ShapeId shapeId, bool flag);
extern void cpu_b3Shape_EnablePreSolveEvents(b3ShapeId shapeId, bool flag);
extern bool cpu_b3Shape_ArePreSolveEventsEnabled(b3ShapeId shapeId);
extern void cpu_b3Shape_SetUserData(b3ShapeId shapeId, void* userData);
extern void cpu_b3DestroyShape(b3ShapeId shapeId, bool updateBodyMass);
extern b3JointId cpu_b3CreateRevoluteJoint(b3WorldId worldId, const b3RevoluteJointDef* def);
extern void cpu_b3Joint_SetConstraintTuning(b3JointId jointId, float hertz, float dampingRatio);
extern void cpu_b3Joint_SetForceThreshold(b3JointId jointId, float threshold);
extern void cpu_b3Joint_SetTorqueThreshold(b3JointId jointId, float threshold);
extern void cpu_b3Joint_SetUserData(b3JointId jointId, void* userData);
extern void cpu_b3RevoluteJoint_EnableSpring(b3JointId jointId, bool enableSpring);
extern void cpu_b3RevoluteJoint_SetSpringHertz(b3JointId jointId, float hertz);
extern void cpu_b3RevoluteJoint_SetSpringDampingRatio(b3JointId jointId, float dampingRatio);
extern void cpu_b3RevoluteJoint_SetTargetAngle(b3JointId jointId, float targetRadians);
extern void cpu_b3RevoluteJoint_EnableLimit(b3JointId jointId, bool enableLimit);
extern void cpu_b3RevoluteJoint_SetLimits(b3JointId jointId, float lowerLimitRadians, float upperLimitRadians);
extern void cpu_b3RevoluteJoint_EnableMotor(b3JointId jointId, bool enableMotor);
extern void cpu_b3RevoluteJoint_SetMotorSpeed(b3JointId jointId, float motorSpeed);
extern void cpu_b3RevoluteJoint_SetMaxMotorTorque(b3JointId jointId, float torque);
extern b3JointId cpu_b3CreateWheelJoint(b3WorldId, const b3WheelJointDef*);
extern void cpu_b3WheelJoint_EnableSuspension(b3JointId, bool);
extern void cpu_b3WheelJoint_SetSuspensionHertz(b3JointId, float);
extern void cpu_b3WheelJoint_SetSuspensionDampingRatio(b3JointId, float);
extern void cpu_b3WheelJoint_EnableSuspensionLimit(b3JointId, bool);
extern void cpu_b3WheelJoint_SetSuspensionLimits(b3JointId, float, float);
extern void cpu_b3WheelJoint_EnableSpinMotor(b3JointId, bool);
extern void cpu_b3WheelJoint_SetSpinMotorSpeed(b3JointId, float);
extern void cpu_b3WheelJoint_SetMaxSpinTorque(b3JointId, float);
extern void cpu_b3WheelJoint_EnableSteering(b3JointId, bool);
extern void cpu_b3WheelJoint_SetSteeringHertz(b3JointId, float);
extern void cpu_b3WheelJoint_SetSteeringDampingRatio(b3JointId, float);
extern void cpu_b3WheelJoint_SetMaxSteeringTorque(b3JointId, float);
extern void cpu_b3WheelJoint_EnableSteeringLimit(b3JointId, bool);
extern void cpu_b3WheelJoint_SetSteeringLimits(b3JointId, float, float);
extern void cpu_b3WheelJoint_SetTargetSteeringAngle(b3JointId, float);
extern b3JointId cpu_b3CreateSphericalJoint(b3WorldId worldId, const b3SphericalJointDef* def);
extern void cpu_b3SphericalJoint_EnableConeLimit(b3JointId jointId, bool enable);
extern void cpu_b3SphericalJoint_SetConeLimit(b3JointId jointId, float value);
extern void cpu_b3SphericalJoint_EnableTwistLimit(b3JointId jointId, bool enable);
extern void cpu_b3SphericalJoint_SetTwistLimits(b3JointId jointId, float lower, float upper);
extern void cpu_b3SphericalJoint_EnableSpring(b3JointId jointId, bool enable);
extern void cpu_b3SphericalJoint_SetSpringHertz(b3JointId jointId, float value);
extern void cpu_b3SphericalJoint_SetSpringDampingRatio(b3JointId jointId, float value);
extern void cpu_b3SphericalJoint_SetTargetRotation(b3JointId jointId, b3Quat value);
extern void cpu_b3SphericalJoint_EnableMotor(b3JointId jointId, bool enable);
extern void cpu_b3SphericalJoint_SetMotorVelocity(b3JointId jointId, b3Vec3 value);
extern void cpu_b3SphericalJoint_SetMaxMotorTorque(b3JointId jointId, float value);
extern b3JointId cpu_b3CreatePrismaticJoint(b3WorldId worldId, const b3PrismaticJointDef* def);
extern b3JointId cpu_b3CreateFilterJoint(b3WorldId worldId, const b3FilterJointDef* def);
extern b3JointId cpu_b3CreateDistanceJoint(b3WorldId worldId, const b3DistanceJointDef* def);
extern void cpu_b3DistanceJoint_SetLength(b3JointId jointId, float length);
extern void cpu_b3DistanceJoint_EnableSpring(b3JointId jointId, bool enableSpring);
extern void cpu_b3DistanceJoint_SetSpringForceRange(b3JointId jointId, float lowerForce, float upperForce);
extern void cpu_b3DistanceJoint_SetSpringHertz(b3JointId jointId, float hertz);
extern void cpu_b3DistanceJoint_SetSpringDampingRatio(b3JointId jointId, float dampingRatio);
extern void cpu_b3DistanceJoint_EnableLimit(b3JointId jointId, bool enableLimit);
extern void cpu_b3DistanceJoint_SetLengthRange(b3JointId jointId, float minLength, float maxLength);
extern void cpu_b3DistanceJoint_EnableMotor(b3JointId jointId, bool enableMotor);
extern void cpu_b3DistanceJoint_SetMotorSpeed(b3JointId jointId, float motorSpeed);
extern void cpu_b3DistanceJoint_SetMaxMotorForce(b3JointId jointId, float force);
extern b3JointId cpu_b3CreateParallelJoint(b3WorldId worldId, const b3ParallelJointDef* def);
extern void cpu_b3ParallelJoint_SetSpringHertz(b3JointId jointId, float hertz);
extern void cpu_b3ParallelJoint_SetSpringDampingRatio(b3JointId jointId, float dampingRatio);
extern void cpu_b3ParallelJoint_SetMaxTorque(b3JointId jointId, float maxTorque);
extern void cpu_b3PrismaticJoint_EnableSpring(b3JointId jointId, bool enableSpring);
extern void cpu_b3PrismaticJoint_SetSpringHertz(b3JointId jointId, float hertz);
extern void cpu_b3PrismaticJoint_SetSpringDampingRatio(b3JointId jointId, float dampingRatio);
extern void cpu_b3PrismaticJoint_SetTargetTranslation(b3JointId jointId, float targetTranslation);
extern void cpu_b3PrismaticJoint_EnableLimit(b3JointId jointId, bool enableLimit);
extern void cpu_b3PrismaticJoint_SetLimits(b3JointId jointId, float lower, float upper);
extern void cpu_b3PrismaticJoint_EnableMotor(b3JointId jointId, bool enableMotor);
extern void cpu_b3PrismaticJoint_SetMotorSpeed(b3JointId jointId, float motorSpeed);
extern void cpu_b3PrismaticJoint_SetMaxMotorForce(b3JointId jointId, float force);
extern b3JointId cpu_b3CreateWeldJoint(b3WorldId worldId, const b3WeldJointDef* def);
extern void cpu_b3WeldJoint_SetLinearHertz(b3JointId jointId, float hertz);
extern float cpu_b3WeldJoint_GetLinearHertz(b3JointId jointId);
extern void cpu_b3WeldJoint_SetLinearDampingRatio(b3JointId jointId, float dampingRatio);
extern float cpu_b3WeldJoint_GetLinearDampingRatio(b3JointId jointId);
extern void cpu_b3WeldJoint_SetAngularHertz(b3JointId jointId, float hertz);
extern float cpu_b3WeldJoint_GetAngularHertz(b3JointId jointId);
extern void cpu_b3WeldJoint_SetAngularDampingRatio(b3JointId jointId, float dampingRatio);
extern float cpu_b3WeldJoint_GetAngularDampingRatio(b3JointId jointId);
extern b3JointId cpu_b3CreateMotorJoint(b3WorldId worldId, const b3MotorJointDef* def);
extern void cpu_b3MotorJoint_SetLinearVelocity(b3JointId jointId, b3Vec3 velocity);
extern b3Vec3 cpu_b3MotorJoint_GetLinearVelocity(b3JointId jointId);
extern void cpu_b3MotorJoint_SetAngularVelocity(b3JointId jointId, b3Vec3 velocity);
extern b3Vec3 cpu_b3MotorJoint_GetAngularVelocity(b3JointId jointId);
extern void cpu_b3MotorJoint_SetMaxVelocityForce(b3JointId jointId, float value);
extern void cpu_b3MotorJoint_SetMaxVelocityTorque(b3JointId jointId, float value);
extern void cpu_b3MotorJoint_SetLinearHertz(b3JointId jointId, float value);
extern void cpu_b3MotorJoint_SetLinearDampingRatio(b3JointId jointId, float value);
extern void cpu_b3MotorJoint_SetAngularHertz(b3JointId jointId, float value);
extern void cpu_b3MotorJoint_SetAngularDampingRatio(b3JointId jointId, float value);
extern void cpu_b3MotorJoint_SetMaxSpringForce(b3JointId jointId, float value);
extern void cpu_b3MotorJoint_SetMaxSpringTorque(b3JointId jointId, float value);
extern void cpu_b3DestroyJoint(b3JointId jointId, bool wakeAttached);
extern b3Pos cpu_b3Body_GetPosition(b3BodyId bodyId);
extern b3Quat cpu_b3Body_GetRotation(b3BodyId bodyId);
extern b3WorldTransform cpu_b3Body_GetTransform(b3BodyId bodyId);
extern void cpu_b3Body_SetTargetTransform(b3BodyId bodyId, b3WorldTransform target, float timeStep, bool wake);
extern void cpu_b3Shape_SetName(b3ShapeId shapeId, const char* name);
extern const char* cpu_b3Shape_GetName(b3ShapeId shapeId);

static void both_banner(void)
{
	fprintf(stderr, "samples_both: CPU left / GPU right; split-view toggle available, same step. HUD 'step N' is shared.\n");
}

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

static void materials(const b3ShapeDef* def, float* density, float* friction, float* restitution, float* rolling)
{
	*density = 1000.0f;
	*friction = 0.6f;
	*restitution = 0.0f;
	*rolling = 0.0f;
	if (def)
	{
		*density = def->density;
		*friction = def->baseMaterial.friction;
		*restitution = def->baseMaterial.restitution;
		*rolling = def->baseMaterial.rollingResistance;
	}
}

static float g_draw_dx;
static b3DebugDraw g_draw_orig;

static b3Pos shift_pos(b3Pos p)
{
	p.x += g_draw_dx;
	return p;
}

static b3WorldTransform shift_xf(b3WorldTransform xf)
{
	xf.p.x += g_draw_dx;
	return xf;
}

static b3AABB shift_aabb(b3AABB a)
{
	a.lowerBound.x += g_draw_dx;
	a.upperBound.x += g_draw_dx;
	return a;
}

static void w_shape(void* user, b3WorldTransform xf, b3HexColor color, void* ctx)
{
	if (g_draw_orig.DrawShapeFcn)
	{
		g_draw_orig.DrawShapeFcn(user, shift_xf(xf), color, ctx);
	}
}

static void w_segment(b3Pos p1, b3Pos p2, b3HexColor color, void* ctx)
{
	if (g_draw_orig.DrawSegmentFcn)
	{
		g_draw_orig.DrawSegmentFcn(shift_pos(p1), shift_pos(p2), color, ctx);
	}
}

static void w_transform(b3WorldTransform xf, void* ctx)
{
	if (g_draw_orig.DrawTransformFcn)
	{
		g_draw_orig.DrawTransformFcn(shift_xf(xf), ctx);
	}
}

static void w_point(b3Pos p, float size, b3HexColor color, void* ctx)
{
	if (g_draw_orig.DrawPointFcn)
	{
		g_draw_orig.DrawPointFcn(shift_pos(p), size, color, ctx);
	}
}

static void w_sphere(b3Pos p, float radius, b3HexColor color, float alpha, void* ctx)
{
	if (g_draw_orig.DrawSphereFcn)
	{
		g_draw_orig.DrawSphereFcn(shift_pos(p), radius, color, alpha, ctx);
	}
}

static void w_capsule(b3Pos p1, b3Pos p2, float radius, b3HexColor color, float alpha, void* ctx)
{
	if (g_draw_orig.DrawCapsuleFcn)
	{
		g_draw_orig.DrawCapsuleFcn(shift_pos(p1), shift_pos(p2), radius, color, alpha, ctx);
	}
}

static void w_bounds(b3AABB aabb, b3HexColor color, void* ctx)
{
	if (g_draw_orig.DrawBoundsFcn)
	{
		g_draw_orig.DrawBoundsFcn(shift_aabb(aabb), color, ctx);
	}
}

static void w_box(b3Vec3 extents, b3WorldTransform xf, b3HexColor color, void* ctx)
{
	if (g_draw_orig.DrawBoxFcn)
	{
		g_draw_orig.DrawBoxFcn(extents, shift_xf(xf), color, ctx);
	}
}

static void w_string(b3Pos p, const char* s, b3HexColor color, void* ctx)
{
	if (g_draw_orig.DrawStringFcn)
	{
		g_draw_orig.DrawStringFcn(shift_pos(p), s, color, ctx);
	}
}

static b3DebugDraw offset_draw(b3DebugDraw* src, float dx)
{
	g_draw_orig = *src;
	g_draw_dx = dx;
	b3DebugDraw d = *src;
	d.DrawShapeFcn = w_shape;
	d.DrawSegmentFcn = w_segment;
	d.DrawTransformFcn = w_transform;
	d.DrawPointFcn = w_point;
	d.DrawSphereFcn = w_sphere;
	d.DrawCapsuleFcn = w_capsule;
	d.DrawBoundsFcn = w_bounds;
	d.DrawBoxFcn = w_box;
	d.DrawStringFcn = w_string;
	return d;
}

/* Sim copies remain close enough to compare, with a 50 cm center-to-center gap. */
static const float kBothSimDx = 0.25f;
static const float kBothLabelDx = 0.45f;

static float scene_dx(b3WorldId gpu)
{
	(void)gpu;
	return kBothSimDx;
}

static b3AABB union_aabb(b3AABB a, b3AABB b)
{
	if (a.lowerBound.x > b.lowerBound.x)
	{
		a.lowerBound.x = b.lowerBound.x;
	}
	if (a.lowerBound.y > b.lowerBound.y)
	{
		a.lowerBound.y = b.lowerBound.y;
	}
	if (a.lowerBound.z > b.lowerBound.z)
	{
		a.lowerBound.z = b.lowerBound.z;
	}
	if (a.upperBound.x < b.upperBound.x)
	{
		a.upperBound.x = b.upperBound.x;
	}
	if (a.upperBound.y < b.upperBound.y)
	{
		a.upperBound.y = b.upperBound.y;
	}
	if (a.upperBound.z < b.upperBound.z)
	{
		a.upperBound.z = b.upperBound.z;
	}
	return a;
}

extern void gpu_b3_world_set_contact_tuning(b3WorldId, float, float, float);
extern void gpu_b3_world_set_user_data(b3WorldId, uintptr_t);
extern void gpu_b3_body_set_name(b3BodyId, const char*);
extern void gpu_b3_body_set_sleep_threshold(b3BodyId, float);
B3_API b3WorldId b3CreateWorld(const b3WorldDef* def)
{
	static bool banner_shown;
	if (!banner_shown) { both_banner(); banner_shown = true; }
	if (!def)
	{
		return (b3WorldId){0};
	}
	GpuWorldId gpu = gpu_b3_create_world_with_capacity(def->gravity.x, def->gravity.y, def->gravity.z,
        def->capacity.staticBodyCount, def->capacity.dynamicBodyCount,
        def->capacity.staticShapeCount, def->capacity.dynamicShapeCount);
	gpu_b3_world_set_contact_tuning(gpu, def->contactHertz, def->contactDampingRatio, def->contactSpeed);
	gpu_b3_world_set_user_data(gpu, (uintptr_t)def->userData);
	gpu_b3_world_enable_sleeping(gpu, def->enableSleep);
	gpu_b3_world_enable_continuous(gpu, def->enableContinuous);
	gpu_b3_world_set_hit_event_threshold(gpu, def->hitEventThreshold);
	gpu_b3_world_set_restitution_threshold(gpu, def->restitutionThreshold);
	gpu_b3_world_set_maximum_linear_speed(gpu, def->maximumLinearSpeed);
	gpu_samples_on_world_created(gpu, def);
	b3WorldId cpu = cpu_b3CreateWorld(def);
	both_map_world(gpu, cpu);
	return gpu;
}

B3_API void b3DestroyWorld(b3WorldId worldId)
{
    if (g_drag.world.index1 == worldId.index1) {
        both_pointer_up(); memset(&g_drag, 0, sizeof(g_drag));
    }
    g_draw_world = (b3WorldId){0};
	gpu_shape_clear_world_geometry(worldId);
	gpu_samples_on_world_destroyed(worldId);
	if (both_has_cpu_world(worldId))
	{
		cpu_b3DestroyWorld(both_cpu_world(worldId));
	}
	gpu_b3_destroy_world(worldId);
	both_clear_maps();
}

B3_API bool b3World_IsValid(b3WorldId id)
{
	return gpu_b3_world_is_valid(id);
}

B3_API void b3World_SetFrictionCallback(b3WorldId worldId, b3FrictionCallback* callback)
{
	gpu_b3_world_set_friction_callback(worldId, callback);
	if (both_has_cpu_world(worldId))
	{
		cpu_b3World_SetFrictionCallback(both_cpu_world(worldId), callback);
	}
}

B3_API void b3World_SetRestitutionCallback(b3WorldId worldId, b3RestitutionCallback* callback)
{
	gpu_b3_world_set_restitution_callback(worldId, callback);
	if (both_has_cpu_world(worldId))
	{
		cpu_b3World_SetRestitutionCallback(both_cpu_world(worldId), callback);
	}
}

B3_API void b3World_Step(b3WorldId worldId, float timeStep, int subStepCount)
{
    both_drag_step(worldId, timeStep);
	gpu_b3_world_step(worldId, timeStep, subStepCount);
	if (both_has_cpu_world(worldId))
	{
		cpu_b3World_Step(both_cpu_world(worldId), timeStep, subStepCount);
	}
	g_gpu_step_ms = gpu_b3_world_last_encode_ms(worldId);
}

B3_API void b3World_Wait(b3WorldId worldId)
{
	gpu_b3_world_wait(worldId);
}

B3_API void b3World_SetCustomFilterCallback(b3WorldId worldId, b3CustomFilterFcn* fcn, void* context)
{
	/* GPU is the public/event-authoritative path. Installing this on the CPU
	 * oracle too would duplicate user-visible callback side effects. */
	gpu_b3_world_set_custom_filter_callback(worldId, fcn, context);
}

B3_API void b3World_SetPreSolveCallback(b3WorldId worldId, b3PreSolveFcn* fcn, void* context)
{
	/* See custom filtering above: CPU comparison intentionally runs without
	 * user callbacks when samples_both is selected. */
	gpu_b3_world_set_pre_solve_callback(worldId, fcn, context);
}

B3_API void b3World_Explode(b3WorldId worldId, const b3ExplosionDef* def)
{
	if (def == NULL)
	{
		return;
	}
	if (both_has_cpu_world(worldId))
	{
		cpu_b3World_Explode(both_cpu_world(worldId), def);
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
	if (both_has_cpu_world(worldId))
	{
		cpu_b3World_SetHitEventThreshold(both_cpu_world(worldId), value);
	}
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

void gpu_samples_get_cpu_stats(b3WorldId worldId, b3Profile* profile, b3Counters* counters)
{
	if (profile != NULL)
	{
		*profile = (b3Profile){0};
	}
	if (counters != NULL)
	{
		*counters = (b3Counters){0};
	}
	if (!both_has_cpu_world(worldId))
	{
		return;
	}
	b3WorldId cpu_world = both_cpu_world(worldId);
	if (profile != NULL)
	{
		*profile = cpu_b3World_GetProfile(cpu_world);
	}
	if (counters != NULL)
	{
		*counters = cpu_b3World_GetCounters(cpu_world);
	}
}

B3_API void b3World_EnableSleeping(b3WorldId worldId, bool flag)
{
	gpu_b3_world_enable_sleeping(worldId, flag);
	if (both_has_cpu_world(worldId))
	{
		cpu_b3World_EnableSleeping(both_cpu_world(worldId), flag);
	}
}

B3_API bool b3World_IsSleepingEnabled(b3WorldId worldId)
{
	return gpu_b3_world_is_sleeping_enabled(worldId);
}

B3_API void b3World_SetGravity(b3WorldId worldId, b3Vec3 gravity)
{
	gpu_b3_world_set_gravity(worldId, gravity.x, gravity.y, gravity.z);
	if (both_has_cpu_world(worldId))
	{
		cpu_b3World_SetGravity(both_cpu_world(worldId), gravity);
	}
}

B3_API b3Vec3 b3World_GetGravity(b3WorldId worldId)
{
	return gpu_b3_world_get_gravity(worldId);
}

B3_API void b3World_EnableContinuous(b3WorldId worldId, bool flag)
{
	gpu_b3_world_enable_continuous(worldId, flag);
	if (both_has_cpu_world(worldId))
	{
		cpu_b3World_EnableContinuous(both_cpu_world(worldId), flag);
	}
}

B3_API bool b3World_IsContinuousEnabled(b3WorldId worldId)
{
	return gpu_b3_world_is_continuous_enabled(worldId);
}

B3_API void b3World_Draw(b3WorldId worldId, b3DebugDraw* draw, uint64_t maskBits)
{
	if (draw == NULL)
	{
		return;
	}
    if (g_split) {
        if (g_second_view) { gpu_samples_world_draw(worldId, draw, maskBits); return; }
        extern void SetSelectedBody(b3BodyId);
        extern void SetComparisonSelectedBody(b3BodyId);
        gpu_b3_world_wait(worldId);
        g_split_draw = *draw; g_draw_world = worldId; g_draw_mask = maskBits;
        SetComparisonSelectedBody(g_drag.selected[0]);
        if (both_has_cpu_world(worldId)) cpu_b3World_Draw(both_cpu_world(worldId), draw, maskBits);
        SetSelectedBody(g_drag.selected[1]);
        return;
    }
	float dx = scene_dx(worldId);
	if (both_has_cpu_world(worldId))
	{
		b3DebugDraw cpu_draw = offset_draw(draw, -dx);
		cpu_b3World_Draw(both_cpu_world(worldId), &cpu_draw, maskBits);
	}
	b3DebugDraw gpu_draw = offset_draw(draw, dx);
	gpu_samples_world_draw(worldId, &gpu_draw, maskBits);
	if (draw->DrawStringFcn)
	{
		b3AABB gpu_bounds = gpu_samples_world_bounds(worldId);
		b3AABB cpu_bounds = both_has_cpu_world(worldId)
			? cpu_b3World_GetBounds(both_cpu_world(worldId))
			: gpu_bounds;
		float cpu_y = cpu_bounds.upperBound.y + 0.5f;
		float gpu_y = gpu_bounds.upperBound.y + 0.5f;
		float cpu_z = 0.5f * (cpu_bounds.lowerBound.z + cpu_bounds.upperBound.z);
		float gpu_z = 0.5f * (gpu_bounds.lowerBound.z + gpu_bounds.upperBound.z);
		draw->DrawStringFcn((b3Pos){-dx, cpu_y, cpu_z}, "CPU", (b3HexColor)b3_colorOrange, draw->context);
		draw->DrawStringFcn((b3Pos){dx, gpu_y, gpu_z}, "GPU", (b3HexColor)b3_colorCornflowerBlue, draw->context);
	}
}

B3_API b3AABB b3World_GetBounds(b3WorldId worldId)
{
	float dx = g_split ? 0.0f : scene_dx(worldId);
	b3AABB gpu = gpu_samples_world_bounds(worldId);
	gpu.lowerBound.x += dx;
	gpu.upperBound.x += dx;
	if (!both_has_cpu_world(worldId))
	{
		return gpu;
	}
	b3AABB cpu = cpu_b3World_GetBounds(both_cpu_world(worldId));
	cpu.lowerBound.x -= dx;
	cpu.upperBound.x -= dx;
	return union_aabb(cpu, gpu);
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
	GpuBodyId gpu = gpu_b3_create_body(worldId, (int)def->type, def->position.x, def->position.y, def->position.z,
									   def->rotation.v.x, def->rotation.v.y, def->rotation.v.z, def->rotation.s,
									   def->linearVelocity.x, def->linearVelocity.y, def->linearVelocity.z,
									   def->angularVelocity.x, def->angularVelocity.y, def->angularVelocity.z,
									   def->gravityScale, locks_from_def(def));
	gpu_b3_body_set_name(gpu, def->name);
	gpu_b3_body_set_sleep_threshold(gpu, def->sleepThreshold);
	gpu_b3_body_set_user_data(gpu, (uintptr_t)def->userData);
	gpu_b3_body_set_bullet(gpu, def->isBullet);
	gpu_b3_body_allow_fast_rotation(gpu, def->allowFastRotation);
	gpu_b3_body_set_linear_damping(gpu, def->linearDamping);
	gpu_b3_body_set_angular_damping(gpu, def->angularDamping);
	if (!def->isEnabled) gpu_b3_body_set_enabled(gpu, false);
	if (both_has_cpu_world(worldId))
	{
		b3BodyId cpu = cpu_b3CreateBody(both_cpu_world(worldId), def);
		both_map_body(gpu, cpu);
	}
	return gpu;
}

B3_API void b3Body_SetUserData(b3BodyId bodyId, void* userData)
{
	gpu_b3_body_set_user_data(bodyId, (uintptr_t)userData);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_SetUserData(both_cpu_body(bodyId), userData);
	}
}

B3_API void* b3Body_GetUserData(b3BodyId bodyId)
{
	return (void*)gpu_b3_body_get_user_data(bodyId);
}

B3_API void b3DestroyBody(b3BodyId bodyId)
{
    gpu_shape_clear_body_geometry(bodyId);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3DestroyBody(both_cpu_body(bodyId));
		both_unmap_body(bodyId);
	}
	gpu_samples_destroy_body(bodyId);
}

static void configure_gpu_shape(GpuShapeId id, const b3ShapeDef* def)
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
	if (both_has_cpu_shape(shapeId))
	{
		cpu_b3Shape_SetFilter(both_cpu_shape(shapeId), filter, invokeContacts);
	}
}

B3_API void b3Shape_EnableContactEvents(b3ShapeId shapeId, bool flag)
{
	gpu_b3_shape_enable_contact_events(shapeId, flag);
	if (both_has_cpu_shape(shapeId))
	{
		cpu_b3Shape_EnableContactEvents(both_cpu_shape(shapeId), flag);
	}
}

B3_API bool b3Shape_AreContactEventsEnabled(b3ShapeId shapeId)
{
	return gpu_b3_shape_are_contact_events_enabled(shapeId);
}

B3_API void b3Shape_EnablePreSolveEvents(b3ShapeId shapeId, bool flag)
{
	gpu_b3_shape_enable_pre_solve_events(shapeId, flag);
	if (both_has_cpu_shape(shapeId))
	{
		cpu_b3Shape_EnablePreSolveEvents(both_cpu_shape(shapeId), flag);
	}
}

B3_API bool b3Shape_ArePreSolveEventsEnabled(b3ShapeId shapeId)
{
	return gpu_b3_shape_are_pre_solve_events_enabled(shapeId);
}

B3_API void b3Shape_EnableHitEvents(b3ShapeId shapeId, bool flag)
{
	gpu_b3_shape_enable_hit_events(shapeId, flag);
	if (both_has_cpu_shape(shapeId))
	{
		cpu_b3Shape_EnableHitEvents(both_cpu_shape(shapeId), flag);
	}
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
	if (both_has_cpu_shape(shapeId))
	{
		cpu_b3Shape_EnableSensorEvents(both_cpu_shape(shapeId), flag);
	}
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

B3_API void b3DestroyShape(b3ShapeId shapeId, bool updateBodyMass)
{
    gpu_shape_clear_geometry(shapeId);
	if (both_has_cpu_shape(shapeId))
	{
		cpu_b3DestroyShape(both_cpu_shape(shapeId), updateBodyMass);
		both_unmap_shape(shapeId);
	}
	gpu_samples_on_shape_destroyed(shapeId);
	gpu_b3_destroy_shape(shapeId, updateBodyMass);
}

B3_API b3ShapeId b3CreateSphereShape(b3BodyId bodyId, const b3ShapeDef* def, const b3Sphere* sphere)
{
	float density, friction, restitution, rolling;
	materials(def, &density, &friction, &restitution, &rolling);
	float radius = sphere ? sphere->radius : 0.5f;
	b3Vec3 center = sphere ? sphere->center : b3Vec3_zero;
	GpuShapeId gpu = gpu_b3_create_sphere(bodyId, center.x, center.y, center.z, radius, density, friction, restitution,
										 rolling, def == NULL || def->updateBodyMass);
	configure_gpu_shape(gpu, def);
	gpu_samples_on_shape_created(gpu, bodyId, b3_sphereShape, sphere, NULL, NULL);
	if (both_has_cpu_body(bodyId))
	{
		both_map_shape(gpu, cpu_b3CreateSphereShape(both_cpu_body(bodyId), def, sphere));
	}
	return gpu;
}

B3_API b3ShapeId b3CreateHullShape(b3BodyId bodyId, const b3ShapeDef* def, const b3HullData* hull)
{
	float density, friction, restitution, rolling;
	materials(def, &density, &friction, &restitution, &rolling);
	float hx = 0.5f, hy = 0.5f, hz = 0.5f, ox = 0.0f, oy = 0.0f, oz = 0.0f;
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
	GpuShapeId gpu;
	if (axisBox || hull == NULL || hull->vertexCount < 4)
	{
		gpu = gpu_b3_create_hull(bodyId, hx, hy, hz, ox, oy, oz, density, friction, restitution, rolling,
								def == NULL || def->updateBodyMass);
	}
	else
	{
		gpu = gpu_b3_create_convex_hull(
			bodyId, &points[0].x, hull->vertexCount, &planes[0].normal.x, hull->faceCount, (const uint8_t*)edges,
			hull->edgeCount, hx, hy, hz, ox, oy, oz, hull->center.x, hull->center.y, hull->center.z, hull->innerRadius, hull->volume,
			hull->centralInertia.cx.x, hull->centralInertia.cy.y, hull->centralInertia.cz.z,
			hull->centralInertia.cx.y, hull->centralInertia.cx.z, hull->centralInertia.cy.z, density, friction,
			restitution, rolling, def == NULL || def->updateBodyMass);
	}
	configure_gpu_shape(gpu, def);
	gpu_shape_mirror_hull(gpu, hull);
	gpu_samples_on_shape_created(gpu, bodyId, b3_hullShape, NULL, NULL, hull);
	if (both_has_cpu_body(bodyId))
	{
		both_map_shape(gpu, cpu_b3CreateHullShape(both_cpu_body(bodyId), def, hull));
	}
	return gpu;
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
	float density, friction, restitution, rolling;
	materials(def, &density, &friction, &restitution, &rolling);
	b3Vec3 c1 = {-1.0f, 0.0f, 0.0f};
	b3Vec3 c2 = {1.0f, 0.0f, 0.0f};
	float radius = 0.5f;
	if (capsule)
	{
		c1 = capsule->center1;
		c2 = capsule->center2;
		radius = capsule->radius;
	}
	GpuShapeId gpu = gpu_b3_create_capsule(bodyId, c1.x, c1.y, c1.z, c2.x, c2.y, c2.z, radius, density, friction,
										 restitution, rolling, def == NULL || def->updateBodyMass);
	configure_gpu_shape(gpu, def);
	gpu_samples_on_shape_created(gpu, bodyId, b3_capsuleShape, NULL, capsule, NULL);
	if (both_has_cpu_body(bodyId))
	{
		both_map_shape(gpu, cpu_b3CreateCapsuleShape(both_cpu_body(bodyId), def, capsule));
	}
	return gpu;
}

B3_API b3ShapeId b3CreateMeshShape(b3BodyId bodyId, const b3ShapeDef* def, const b3MeshData* mesh, b3Vec3 scale)
{
	if (mesh == NULL || mesh->version != B3_MESH_VERSION)
	{
		return (b3ShapeId){0};
	}
	float density, friction, restitution, rolling;
	materials(def, &density, &friction, &restitution, &rolling);
	GpuShapeId gpu = gpu_b3_create_mesh(
		bodyId, &b3GetMeshVertices(mesh)[0].x, mesh->vertexCount, &b3GetMeshTriangles(mesh)[0].index1,
		mesh->triangleCount, b3GetMeshFlags(mesh), b3GetMeshMaterialIndices(mesh), b3GetMeshNodes(mesh),
		mesh->nodeCount, scale.x, scale.y, scale.z, density, friction, restitution, rolling,
		def == NULL || def->updateBodyMass);
	configure_gpu_shape(gpu, def);
	int materialCount = def != NULL && def->materialCount > 0 ? def->materialCount : 1;
	gpu_b3_shape_set_mesh_material_count(gpu, materialCount);
	for (int i = 0; i < materialCount; ++i)
	{
		b3SurfaceMaterial material =
			def != NULL && def->materials != NULL && def->materialCount > 0 ? def->materials[i]
																		   : (def != NULL ? def->baseMaterial : b3DefaultSurfaceMaterial());
		gpu_b3_shape_set_mesh_material(gpu, i, material);
	}
	gpu_samples_on_mesh_shape_created(gpu, bodyId, mesh, scale);
	if (both_has_cpu_body(bodyId))
	{
		both_map_shape(gpu, cpu_b3CreateMeshShape(both_cpu_body(bodyId), def, mesh, scale));
	}
	return gpu;
}

B3_API b3ShapeId b3CreateHeightFieldShape(b3BodyId bodyId, const b3ShapeDef* def,
										  const b3HeightFieldData* heightField)
{
	if (heightField == NULL || heightField->version != B3_HEIGHT_FIELD_VERSION ||
		heightField->columnCount < 2 || heightField->rowCount < 2 || b3Body_GetType(bodyId) != b3_staticBody)
	{
		return (b3ShapeId){0};
	}
	float density, friction, restitution, rolling;
	materials(def, &density, &friction, &restitution, &rolling);
	GpuShapeId gpu = gpu_b3_create_height_field(
		bodyId, b3GetHeightFieldCompressedHeights(heightField), heightField->columnCount, heightField->rowCount,
		heightField->minHeight, heightField->heightScale, heightField->scale.x, heightField->scale.y,
		heightField->scale.z, b3GetHeightFieldMaterialIndices(heightField), b3GetHeightFieldFlags(heightField),
		heightField->clockwise != 0, density, friction, restitution, rolling);
	configure_gpu_shape(gpu, def);
	int materialCount = def != NULL && def->materialCount > 0 ? def->materialCount : 1;
	gpu_b3_shape_set_mesh_material_count(gpu, materialCount);
	for (int i = 0; i < materialCount; ++i)
	{
		b3SurfaceMaterial material =
			def != NULL && def->materials != NULL && def->materialCount > 0 ? def->materials[i]
																		   : (def != NULL ? def->baseMaterial : b3DefaultSurfaceMaterial());
		gpu_b3_shape_set_mesh_material(gpu, i, material);
	}
	gpu_samples_on_shape_created(gpu, bodyId, b3_heightShape, NULL, NULL, NULL);
	if (both_has_cpu_body(bodyId))
	{
		both_map_shape(gpu, cpu_b3CreateHeightFieldShape(both_cpu_body(bodyId), def, heightField));
	}
	return gpu;
}

#include "compound_mesh_instances.h"
extern void gpu_b3_world_set_fail(b3WorldId id, const char* message);

B3_API b3ShapeId b3CreateBakedCompoundShape(b3BodyId bodyId, b3ShapeDef* def, const b3CompoundData* compound)
{
	if (compound == NULL || compound->version != B3_COMPOUND_VERSION || b3Body_GetType(bodyId) != b3_staticBody ||
		(def != NULL && def->isSensor))
	{
		gpu_b3_world_set_fail((b3WorldId){bodyId.world0, 1}, "unsupported or rejected compound data");
		return (b3ShapeId){0};
	}
	float density, friction, restitution, rolling;
	materials(def, &density, &friction, &restitution, &rolling);
	GpuShapeId parent = gpu_b3_create_compound_parent(bodyId, density, friction, restitution, rolling,
													   def != NULL && def->isSensor);
	if (parent.index1 <= 0)
	{
		gpu_b3_world_set_fail((b3WorldId){bodyId.world0, 1}, "compound parent allocation failed");
		return (b3ShapeId){0};
	}
	configure_gpu_shape(parent, def);
	const b3SurfaceMaterial* compoundMaterials = b3GetCompoundMaterials(compound);
	int childCount =
		compound->capsuleCount + compound->hullCount + compound->meshCount + compound->sphereCount;
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
		if (compoundMaterials != NULL && materialIndex >= 0 && materialIndex < compound->materialCount)
		{
			childDef.baseMaterial = compoundMaterials[materialIndex];
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
			childId = gpu_mesh_cache_instance(&meshCache, bodyId, &child, &childDef);
			configure_gpu_shape(childId, &childDef);
			/* Triangle indices are local to this mesh; the child map resolves them
			   into the compound's shared surface-material table. */
			gpu_b3_shape_set_mesh_material_count(childId, mesh->materialCount);
			for (int j = 0; j < mesh->materialCount; ++j)
			{
				gpu_b3_shape_set_mesh_material(childId, j, compoundMaterials[child.materialIndices[j]]);
			}
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
			gpu_samples_on_shape_destroyed(childId);
		}
	}
	gpu_mesh_cache_free(&meshCache);
	gpu_samples_on_compound_shape_created(parent, bodyId, compound);
	if (both_has_cpu_body(bodyId))
	{
		both_map_shape(parent, cpu_b3CreateBakedCompoundShape(both_cpu_body(bodyId), def, compound));
	}
	return parent;
}

B3_API const b3HeightFieldData* b3Shape_GetHeightField(b3ShapeId shapeId)
{
	return both_has_cpu_shape(shapeId) ? cpu_b3Shape_GetHeightField(both_cpu_shape(shapeId)) : NULL;
}

B3_API b3Mesh b3Shape_GetMesh(b3ShapeId shapeId)
{
	return both_has_cpu_shape(shapeId) ? cpu_b3Shape_GetMesh(both_cpu_shape(shapeId)) : (b3Mesh){0};
}

B3_API int b3Shape_GetMeshMaterialCount(b3ShapeId shapeId)
{
	return both_has_cpu_shape(shapeId) ? cpu_b3Shape_GetMeshMaterialCount(both_cpu_shape(shapeId)) : 0;
}

B3_API void b3Shape_SetMeshMaterial(b3ShapeId shapeId, b3SurfaceMaterial material, int index)
{
	gpu_b3_shape_set_mesh_material(shapeId, index, material);
	if (both_has_cpu_shape(shapeId))
	{
		cpu_b3Shape_SetMeshMaterial(both_cpu_shape(shapeId), material, index);
	}
}

B3_API b3SurfaceMaterial b3Shape_GetMeshSurfaceMaterial(b3ShapeId shapeId, int index)
{
	return gpu_b3_shape_get_mesh_material(shapeId, index);
}

B3_API void b3Shape_SetSurfaceMaterial(b3ShapeId shapeId, b3SurfaceMaterial material)
{
	gpu_b3_shape_set_surface_material(shapeId, material);
	if (both_has_cpu_shape(shapeId))
	{
		cpu_b3Shape_SetSurfaceMaterial(both_cpu_shape(shapeId), material);
	}
}

B3_API b3SurfaceMaterial b3Shape_GetSurfaceMaterial(b3ShapeId shapeId)
{
	return gpu_b3_shape_get_surface_material(shapeId);
}

B3_API void b3Shape_SetMesh(b3ShapeId shapeId, const b3MeshData* mesh, b3Vec3 scale)
{
	if (mesh == NULL || mesh->version != B3_MESH_VERSION)
	{
		return;
	}
	gpu_b3_replace_mesh(
		shapeId, &b3GetMeshVertices(mesh)[0].x, mesh->vertexCount, &b3GetMeshTriangles(mesh)[0].index1,
		mesh->triangleCount, b3GetMeshFlags(mesh), b3GetMeshMaterialIndices(mesh), b3GetMeshNodes(mesh),
		mesh->nodeCount, scale.x, scale.y, scale.z);
	if (both_has_cpu_shape(shapeId))
	{
		cpu_b3Shape_SetMesh(both_cpu_shape(shapeId), mesh, scale);
	}
}

static void configure_gpu_joint(GpuJointId joint, const b3JointDef* base)
{
	gpu_b3_joint_set_force_threshold(joint, base->forceThreshold);
	gpu_b3_joint_set_torque_threshold(joint, base->torqueThreshold);
	gpu_b3_joint_set_user_data(joint, (uintptr_t)base->userData);
}

B3_API b3JointId b3CreateRevoluteJoint(b3WorldId worldId, const b3RevoluteJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	GpuJointId gpu = gpu_b3_create_revolute(
		worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.localFrameA.p.x, def->base.localFrameA.p.y,
		def->base.localFrameA.p.z, def->base.localFrameB.p.x, def->base.localFrameB.p.y, def->base.localFrameB.p.z,
		def->base.localFrameA.q.v.x, def->base.localFrameA.q.v.y, def->base.localFrameA.q.v.z,
		def->base.localFrameA.q.s, def->base.localFrameB.q.v.x, def->base.localFrameB.q.v.y,
		def->base.localFrameB.q.v.z, def->base.localFrameB.q.s, def->base.constraintHertz,
		def->base.constraintDampingRatio, def->base.collideConnected, def->targetAngle, def->enableSpring, def->hertz,
		def->dampingRatio, def->enableLimit, def->lowerAngle, def->upperAngle, def->enableMotor, def->maxMotorTorque,
		def->motorSpeed);
	configure_gpu_joint(gpu, &def->base);
	if (both_has_cpu_world(worldId))
	{
		b3RevoluteJointDef cpu_def = *def;
		cpu_def.base.bodyIdA = both_cpu_body(def->base.bodyIdA);
		cpu_def.base.bodyIdB = both_cpu_body(def->base.bodyIdB);
		both_map_joint(gpu, cpu_b3CreateRevoluteJoint(both_cpu_world(worldId), &cpu_def));
	}
	return gpu;
}

B3_API void b3Joint_SetConstraintTuning(b3JointId jointId, float hertz, float dampingRatio)
{
	gpu_b3_joint_set_constraint_tuning(jointId, hertz, dampingRatio);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3Joint_SetConstraintTuning(both_cpu_joint(jointId), hertz, dampingRatio);
	}
}

B3_API void b3Joint_GetConstraintTuning(b3JointId jointId, float* hertz, float* dampingRatio)
{
	gpu_b3_joint_get_constraint_tuning(jointId, hertz, dampingRatio);
}

B3_API void b3Joint_SetForceThreshold(b3JointId jointId, float threshold)
{
	gpu_b3_joint_set_force_threshold(jointId, threshold);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3Joint_SetForceThreshold(both_cpu_joint(jointId), threshold);
	}
}

B3_API float b3Joint_GetForceThreshold(b3JointId jointId)
{
	return gpu_b3_joint_get_force_threshold(jointId);
}

B3_API void b3Joint_SetTorqueThreshold(b3JointId jointId, float threshold)
{
	gpu_b3_joint_set_torque_threshold(jointId, threshold);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3Joint_SetTorqueThreshold(both_cpu_joint(jointId), threshold);
	}
}

B3_API float b3Joint_GetTorqueThreshold(b3JointId jointId)
{
	return gpu_b3_joint_get_torque_threshold(jointId);
}

B3_API void b3Joint_SetUserData(b3JointId jointId, void* userData)
{
	gpu_b3_joint_set_user_data(jointId, (uintptr_t)userData);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3Joint_SetUserData(both_cpu_joint(jointId), userData);
	}
}

B3_API void* b3Joint_GetUserData(b3JointId jointId)
{
	return (void*)gpu_b3_joint_get_user_data(jointId);
}

#define BOTH_REVOLUTE_BOOL_API(Name, suffix)                                                         \
	B3_API void b3RevoluteJoint_Enable##Name(b3JointId jointId, bool enable)                          \
	{                                                                                                 \
		gpu_b3_revolute_enable_##suffix(jointId, enable);                                                \
		if (both_has_cpu_joint(jointId))                                                                 \
		{                                                                                               \
			cpu_b3RevoluteJoint_Enable##Name(both_cpu_joint(jointId), enable);                              \
		}                                                                                               \
	}                                                                                                 \
	B3_API bool b3RevoluteJoint_Is##Name##Enabled(b3JointId jointId)                                  \
	{                                                                                                 \
		return gpu_b3_revolute_is_##suffix##_enabled(jointId);                                           \
	}

BOTH_REVOLUTE_BOOL_API(Spring, spring)
BOTH_REVOLUTE_BOOL_API(Limit, limit)
BOTH_REVOLUTE_BOOL_API(Motor, motor)

#undef BOTH_REVOLUTE_BOOL_API

#define BOTH_REVOLUTE_SCALAR_API(Name, suffix)                                                       \
	B3_API void b3RevoluteJoint_Set##Name(b3JointId jointId, float value)                             \
	{                                                                                                 \
		gpu_b3_revolute_set_##suffix(jointId, value);                                                    \
		if (both_has_cpu_joint(jointId))                                                                 \
		{                                                                                               \
			cpu_b3RevoluteJoint_Set##Name(both_cpu_joint(jointId), value);                                 \
		}                                                                                               \
	}                                                                                                 \
	B3_API float b3RevoluteJoint_Get##Name(b3JointId jointId)                                        \
	{                                                                                                 \
		return gpu_b3_revolute_get_##suffix(jointId);                                                    \
	}

BOTH_REVOLUTE_SCALAR_API(SpringHertz, spring_hertz)
BOTH_REVOLUTE_SCALAR_API(SpringDampingRatio, spring_damping)
BOTH_REVOLUTE_SCALAR_API(TargetAngle, target_angle)
BOTH_REVOLUTE_SCALAR_API(MotorSpeed, motor_speed)
BOTH_REVOLUTE_SCALAR_API(MaxMotorTorque, max_motor_torque)

#undef BOTH_REVOLUTE_SCALAR_API

B3_API float b3RevoluteJoint_GetAngle(b3JointId jointId)
{
	return gpu_b3_revolute_get_angle(jointId);
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
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3RevoluteJoint_SetLimits(both_cpu_joint(jointId), lowerLimitRadians, upperLimitRadians);
	}
}

B3_API float b3RevoluteJoint_GetMotorTorque(b3JointId jointId)
{
	return gpu_b3_revolute_get_motor_torque(jointId);
}

B3_API b3JointId b3CreateWheelJoint(b3WorldId worldId, const b3WheelJointDef* def)
{
	if (!def) return (b3JointId){0};
	GpuJointId gpu = gpu_b3_create_wheel(
		worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.localFrameA.p.x, def->base.localFrameA.p.y,
		def->base.localFrameA.p.z, def->base.localFrameB.p.x, def->base.localFrameB.p.y, def->base.localFrameB.p.z,
		def->base.localFrameA.q.v.x, def->base.localFrameA.q.v.y, def->base.localFrameA.q.v.z,
		def->base.localFrameA.q.s, def->base.localFrameB.q.v.x, def->base.localFrameB.q.v.y,
		def->base.localFrameB.q.v.z, def->base.localFrameB.q.s, def->base.constraintHertz,
		def->base.constraintDampingRatio, def->base.collideConnected, def->enableSuspensionSpring, def->suspensionHertz,
		def->suspensionDampingRatio, def->enableSuspensionLimit, def->lowerSuspensionLimit,
		def->upperSuspensionLimit, def->enableSpinMotor, def->maxSpinTorque, def->spinSpeed, def->enableSteering,
		def->steeringHertz, def->steeringDampingRatio, def->targetSteeringAngle, def->maxSteeringTorque,
		def->enableSteeringLimit, def->lowerSteeringLimit, def->upperSteeringLimit);
	configure_gpu_joint(gpu, &def->base);
	if (both_has_cpu_world(worldId))
	{
		b3WheelJointDef cpu_def = *def;
		cpu_def.base.bodyIdA = both_cpu_body(def->base.bodyIdA);
		cpu_def.base.bodyIdB = both_cpu_body(def->base.bodyIdB);
		both_map_joint(gpu, cpu_b3CreateWheelJoint(both_cpu_world(worldId), &cpu_def));
	}
	return gpu;
}

#define BOTH_WHEEL_BOOL(Name, gpu_suffix) \
	B3_API void b3WheelJoint_Enable##Name(b3JointId id, bool value) { \
		gpu_b3_wheel_enable_##gpu_suffix(id, value); \
		if (both_has_cpu_joint(id)) cpu_b3WheelJoint_Enable##Name(both_cpu_joint(id), value); \
	} \
	B3_API bool b3WheelJoint_Is##Name##Enabled(b3JointId id) { return gpu_b3_wheel_is_##gpu_suffix##_enabled(id); }
BOTH_WHEEL_BOOL(Suspension, suspension)
BOTH_WHEEL_BOOL(SuspensionLimit, suspension_limit)
BOTH_WHEEL_BOOL(SpinMotor, spin_motor)
BOTH_WHEEL_BOOL(Steering, steering)
BOTH_WHEEL_BOOL(SteeringLimit, steering_limit)
#undef BOTH_WHEEL_BOOL

#define BOTH_WHEEL_SCALAR(Name, gpu_suffix) \
	B3_API void b3WheelJoint_Set##Name(b3JointId id, float value) { \
		gpu_b3_wheel_set_##gpu_suffix(id, value); \
		if (both_has_cpu_joint(id)) cpu_b3WheelJoint_Set##Name(both_cpu_joint(id), value); \
	} \
	B3_API float b3WheelJoint_Get##Name(b3JointId id) { return gpu_b3_wheel_get_##gpu_suffix(id); }
BOTH_WHEEL_SCALAR(SuspensionHertz, suspension_hertz)
BOTH_WHEEL_SCALAR(SuspensionDampingRatio, suspension_damping)
BOTH_WHEEL_SCALAR(MaxSpinTorque, max_spin_torque)
BOTH_WHEEL_SCALAR(SteeringHertz, steering_hertz)
BOTH_WHEEL_SCALAR(SteeringDampingRatio, steering_damping)
BOTH_WHEEL_SCALAR(MaxSteeringTorque, max_steering_torque)
#undef BOTH_WHEEL_SCALAR

B3_API void b3WheelJoint_SetSpinMotorSpeed(b3JointId id, float value) {
	gpu_b3_wheel_set_spin_speed(id, value);
	if (both_has_cpu_joint(id)) cpu_b3WheelJoint_SetSpinMotorSpeed(both_cpu_joint(id), value);
}
B3_API float b3WheelJoint_GetSpinMotorSpeed(b3JointId id) { return gpu_b3_wheel_get_spin_speed_setting(id); }
B3_API void b3WheelJoint_SetTargetSteeringAngle(b3JointId id, float value) {
	gpu_b3_wheel_set_target_steering(id, value);
	if (both_has_cpu_joint(id)) cpu_b3WheelJoint_SetTargetSteeringAngle(both_cpu_joint(id), value);
}
B3_API float b3WheelJoint_GetTargetSteeringAngle(b3JointId id) { return gpu_b3_wheel_get_target_steering(id); }
B3_API void b3WheelJoint_SetSuspensionLimits(b3JointId id, float lower, float upper) {
	gpu_b3_wheel_set_suspension_limits(id, lower, upper);
	if (both_has_cpu_joint(id)) cpu_b3WheelJoint_SetSuspensionLimits(both_cpu_joint(id), lower, upper);
}
B3_API float b3WheelJoint_GetLowerSuspensionLimit(b3JointId id) { return gpu_b3_wheel_get_lower_suspension_limit(id); }
B3_API float b3WheelJoint_GetUpperSuspensionLimit(b3JointId id) { return gpu_b3_wheel_get_upper_suspension_limit(id); }
B3_API void b3WheelJoint_SetSteeringLimits(b3JointId id, float lower, float upper) {
	gpu_b3_wheel_set_steering_limits(id, lower, upper);
	if (both_has_cpu_joint(id)) cpu_b3WheelJoint_SetSteeringLimits(both_cpu_joint(id), lower, upper);
}
B3_API float b3WheelJoint_GetLowerSteeringLimit(b3JointId id) { return gpu_b3_wheel_get_lower_steering_limit(id); }
B3_API float b3WheelJoint_GetUpperSteeringLimit(b3JointId id) { return gpu_b3_wheel_get_upper_steering_limit(id); }
B3_API float b3WheelJoint_GetSpinSpeed(b3JointId id) { return gpu_b3_wheel_get_live_spin_speed(id); }
B3_API float b3WheelJoint_GetSpinTorque(b3JointId id) { return gpu_b3_wheel_get_spin_torque(id); }
B3_API float b3WheelJoint_GetSteeringAngle(b3JointId id) { return gpu_b3_wheel_get_steering_angle(id); }
B3_API float b3WheelJoint_GetSteeringTorque(b3JointId id) { return gpu_b3_wheel_get_steering_torque(id); }

B3_API b3JointId b3CreateSphericalJoint(b3WorldId worldId, const b3SphericalJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	GpuJointId gpu = gpu_b3_create_spherical(worldId, def->base.bodyIdA, def->base.bodyIdB,
											 def->base.localFrameA.p.x, def->base.localFrameA.p.y,
											 def->base.localFrameA.p.z, def->base.localFrameB.p.x,
											 def->base.localFrameB.p.y, def->base.localFrameB.p.z,
											 def->base.localFrameA.q.v.x, def->base.localFrameA.q.v.y,
											 def->base.localFrameA.q.v.z, def->base.localFrameA.q.s,
											 def->base.localFrameB.q.v.x, def->base.localFrameB.q.v.y,
											 def->base.localFrameB.q.v.z, def->base.localFrameB.q.s,
											 def->base.constraintHertz, def->base.constraintDampingRatio,
											 def->base.collideConnected, def->enableSpring, def->hertz,
											 def->dampingRatio, def->targetRotation.v.x, def->targetRotation.v.y,
											 def->targetRotation.v.z, def->targetRotation.s, def->enableConeLimit,
											 def->coneAngle, def->enableTwistLimit, def->lowerTwistAngle,
											 def->upperTwistAngle, def->enableMotor, def->maxMotorTorque,
											 def->motorVelocity.x, def->motorVelocity.y, def->motorVelocity.z);
	configure_gpu_joint(gpu, &def->base);
	if (both_has_cpu_world(worldId))
	{
		b3SphericalJointDef cpu_def = *def;
		cpu_def.base.bodyIdA = both_cpu_body(def->base.bodyIdA);
		cpu_def.base.bodyIdB = both_cpu_body(def->base.bodyIdB);
		both_map_joint(gpu, cpu_b3CreateSphericalJoint(both_cpu_world(worldId), &cpu_def));
	}
	return gpu;
}

#define BOTH_SPHERICAL_BOOL_API(Name, suffix)                                                        \
	B3_API void b3SphericalJoint_Enable##Name(b3JointId jointId, bool enable)                         \
	{                                                                                                 \
		gpu_b3_spherical_enable_##suffix(jointId, enable);                                               \
		if (both_has_cpu_joint(jointId))                                                                 \
		{                                                                                               \
			cpu_b3SphericalJoint_Enable##Name(both_cpu_joint(jointId), enable);                             \
		}                                                                                               \
	}                                                                                                 \
	B3_API bool b3SphericalJoint_Is##Name##Enabled(b3JointId jointId)                                 \
	{                                                                                                 \
		return gpu_b3_spherical_is_##suffix##_enabled(jointId);                                          \
	}

BOTH_SPHERICAL_BOOL_API(ConeLimit, cone_limit)
BOTH_SPHERICAL_BOOL_API(TwistLimit, twist_limit)
BOTH_SPHERICAL_BOOL_API(Spring, spring)
BOTH_SPHERICAL_BOOL_API(Motor, motor)

#undef BOTH_SPHERICAL_BOOL_API

#define BOTH_SPHERICAL_SCALAR_API(Name, suffix)                                                      \
	B3_API void b3SphericalJoint_Set##Name(b3JointId jointId, float value)                            \
	{                                                                                                 \
		gpu_b3_spherical_set_##suffix(jointId, value);                                                   \
		if (both_has_cpu_joint(jointId))                                                                 \
		{                                                                                               \
			cpu_b3SphericalJoint_Set##Name(both_cpu_joint(jointId), value);                                \
		}                                                                                               \
	}                                                                                                 \
	B3_API float b3SphericalJoint_Get##Name(b3JointId jointId)                                       \
	{                                                                                                 \
		return gpu_b3_spherical_get_##suffix(jointId);                                                   \
	}

BOTH_SPHERICAL_SCALAR_API(ConeLimit, cone_limit)
BOTH_SPHERICAL_SCALAR_API(SpringHertz, spring_hertz)
BOTH_SPHERICAL_SCALAR_API(SpringDampingRatio, spring_damping)
BOTH_SPHERICAL_SCALAR_API(MaxMotorTorque, max_motor_torque)

#undef BOTH_SPHERICAL_SCALAR_API

B3_API float b3SphericalJoint_GetConeAngle(b3JointId jointId)
{
	return gpu_b3_spherical_get_cone_angle(jointId);
}

B3_API float b3SphericalJoint_GetLowerTwistLimit(b3JointId jointId)
{
	return gpu_b3_spherical_get_lower_twist_limit(jointId);
}

B3_API float b3SphericalJoint_GetUpperTwistLimit(b3JointId jointId)
{
	return gpu_b3_spherical_get_upper_twist_limit(jointId);
}

B3_API void b3SphericalJoint_SetTwistLimits(b3JointId jointId, float lower, float upper)
{
	gpu_b3_spherical_set_twist_limits(jointId, lower, upper);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3SphericalJoint_SetTwistLimits(both_cpu_joint(jointId), lower, upper);
	}
}

B3_API float b3SphericalJoint_GetTwistAngle(b3JointId jointId)
{
	return gpu_b3_spherical_get_twist_angle(jointId);
}

B3_API void b3SphericalJoint_SetTargetRotation(b3JointId jointId, b3Quat value)
{
	gpu_b3_spherical_set_target_rotation(jointId, value.v.x, value.v.y, value.v.z, value.s);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3SphericalJoint_SetTargetRotation(both_cpu_joint(jointId), value);
	}
}

B3_API b3Quat b3SphericalJoint_GetTargetRotation(b3JointId jointId)
{
	b3Quat value;
	gpu_b3_spherical_get_target_rotation(jointId, &value.v.x);
	return value;
}

B3_API void b3SphericalJoint_SetMotorVelocity(b3JointId jointId, b3Vec3 value)
{
	gpu_b3_spherical_set_motor_velocity(jointId, value.x, value.y, value.z);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3SphericalJoint_SetMotorVelocity(both_cpu_joint(jointId), value);
	}
}

B3_API b3Vec3 b3SphericalJoint_GetMotorVelocity(b3JointId jointId)
{
	b3Vec3 value;
	gpu_b3_spherical_get_motor_velocity(jointId, &value.x);
	return value;
}

B3_API b3Vec3 b3SphericalJoint_GetMotorTorque(b3JointId jointId)
{
	b3Vec3 value;
	gpu_b3_spherical_get_motor_torque(jointId, &value.x);
	return value;
}

B3_API b3JointId b3CreatePrismaticJoint(b3WorldId worldId, const b3PrismaticJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	b3Vec3 axis = b3RotateVector(def->base.localFrameA.q, b3Vec3_axisX);
	GpuJointId gpu = gpu_b3_create_prismatic(
		worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.localFrameA.p.x, def->base.localFrameA.p.y,
		def->base.localFrameA.p.z, def->base.localFrameB.p.x, def->base.localFrameB.p.y, def->base.localFrameB.p.z,
		axis.x, axis.y, axis.z, def->base.localFrameA.q.v.x, def->base.localFrameA.q.v.y,
		def->base.localFrameA.q.v.z, def->base.localFrameA.q.s, def->base.localFrameB.q.v.x,
		def->base.localFrameB.q.v.y, def->base.localFrameB.q.v.z, def->base.localFrameB.q.s,
		def->base.constraintHertz, def->base.constraintDampingRatio, def->base.collideConnected, def->enableSpring,
		def->hertz, def->dampingRatio,
		def->targetTranslation, def->enableLimit, def->lowerTranslation, def->upperTranslation, def->enableMotor,
		def->maxMotorForce, def->motorSpeed);
	configure_gpu_joint(gpu, &def->base);
	if (both_has_cpu_world(worldId))
	{
		b3PrismaticJointDef cpu_def = *def;
		cpu_def.base.bodyIdA = both_cpu_body(def->base.bodyIdA);
		cpu_def.base.bodyIdB = both_cpu_body(def->base.bodyIdB);
		both_map_joint(gpu, cpu_b3CreatePrismaticJoint(both_cpu_world(worldId), &cpu_def));
	}
	return gpu;
}

B3_API b3JointId b3CreateDistanceJoint(b3WorldId worldId, const b3DistanceJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	GpuJointId gpu = gpu_b3_create_distance(
		worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.localFrameA.p.x, def->base.localFrameA.p.y,
		def->base.localFrameA.p.z, def->base.localFrameB.p.x, def->base.localFrameB.p.y, def->base.localFrameB.p.z,
		def->base.constraintHertz, def->base.constraintDampingRatio, def->base.collideConnected, def->length,
		def->enableSpring,
		def->lowerSpringForce, def->upperSpringForce, def->hertz, def->dampingRatio, def->enableLimit, def->minLength,
		def->maxLength, def->enableMotor, def->maxMotorForce, def->motorSpeed);
	configure_gpu_joint(gpu, &def->base);
	if (both_has_cpu_world(worldId))
	{
		b3DistanceJointDef cpu_def = *def;
		cpu_def.base.bodyIdA = both_cpu_body(def->base.bodyIdA);
		cpu_def.base.bodyIdB = both_cpu_body(def->base.bodyIdB);
		both_map_joint(gpu, cpu_b3CreateDistanceJoint(both_cpu_world(worldId), &cpu_def));
	}
	return gpu;
}

B3_API b3JointId b3CreateParallelJoint(b3WorldId worldId, const b3ParallelJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	GpuJointId gpu = gpu_b3_create_parallel(
		worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.localFrameA.q.v.x,
		def->base.localFrameA.q.v.y, def->base.localFrameA.q.v.z, def->base.localFrameA.q.s,
		def->base.localFrameB.q.v.x, def->base.localFrameB.q.v.y, def->base.localFrameB.q.v.z,
		def->base.localFrameB.q.s, def->hertz, def->dampingRatio, def->maxTorque, def->base.collideConnected);
	configure_gpu_joint(gpu, &def->base);
	if (both_has_cpu_world(worldId))
	{
		b3ParallelJointDef cpu_def = *def;
		cpu_def.base.bodyIdA = both_cpu_body(def->base.bodyIdA);
		cpu_def.base.bodyIdB = both_cpu_body(def->base.bodyIdB);
		both_map_joint(gpu, cpu_b3CreateParallelJoint(both_cpu_world(worldId), &cpu_def));
	}
	return gpu;
}

B3_API b3JointId b3CreateFilterJoint(b3WorldId worldId, const b3FilterJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	GpuJointId gpu =
		gpu_b3_create_filter(worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.collideConnected);
	configure_gpu_joint(gpu, &def->base);
	if (both_has_cpu_world(worldId))
	{
		b3FilterJointDef cpu_def = *def;
		cpu_def.base.bodyIdA = both_cpu_body(def->base.bodyIdA);
		cpu_def.base.bodyIdB = both_cpu_body(def->base.bodyIdB);
		both_map_joint(gpu, cpu_b3CreateFilterJoint(both_cpu_world(worldId), &cpu_def));
	}
	// Apply after mapping so the public setters preserve both stored definitions.
	b3Joint_SetLocalFrameA(gpu, def->base.localFrameA);
	b3Joint_SetLocalFrameB(gpu, def->base.localFrameB);
	b3Joint_SetConstraintTuning(gpu, def->base.constraintHertz, def->base.constraintDampingRatio);
	return gpu;
}

B3_API void b3DistanceJoint_SetLength(b3JointId jointId, float length)
{
	gpu_b3_distance_set_length(jointId, length);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3DistanceJoint_SetLength(both_cpu_joint(jointId), length);
	}
}

B3_API void b3DistanceJoint_EnableSpring(b3JointId jointId, bool enableSpring)
{
	gpu_b3_distance_enable_spring(jointId, enableSpring);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3DistanceJoint_EnableSpring(both_cpu_joint(jointId), enableSpring);
	}
}

B3_API void b3DistanceJoint_SetSpringForceRange(b3JointId jointId, float lowerForce, float upperForce)
{
	gpu_b3_distance_set_spring_force_range(jointId, lowerForce, upperForce);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3DistanceJoint_SetSpringForceRange(both_cpu_joint(jointId), lowerForce, upperForce);
	}
}

B3_API void b3DistanceJoint_SetSpringHertz(b3JointId jointId, float hertz)
{
	gpu_b3_distance_set_spring_hertz(jointId, hertz);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3DistanceJoint_SetSpringHertz(both_cpu_joint(jointId), hertz);
	}
}

B3_API void b3DistanceJoint_SetSpringDampingRatio(b3JointId jointId, float dampingRatio)
{
	gpu_b3_distance_set_spring_damping(jointId, dampingRatio);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3DistanceJoint_SetSpringDampingRatio(both_cpu_joint(jointId), dampingRatio);
	}
}

B3_API void b3DistanceJoint_EnableLimit(b3JointId jointId, bool enableLimit)
{
	gpu_b3_distance_enable_limit(jointId, enableLimit);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3DistanceJoint_EnableLimit(both_cpu_joint(jointId), enableLimit);
	}
}

B3_API void b3DistanceJoint_SetLengthRange(b3JointId jointId, float minLength, float maxLength)
{
	gpu_b3_distance_set_length_range(jointId, minLength, maxLength);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3DistanceJoint_SetLengthRange(both_cpu_joint(jointId), minLength, maxLength);
	}
}

B3_API void b3DistanceJoint_EnableMotor(b3JointId jointId, bool enableMotor)
{
	gpu_b3_distance_enable_motor(jointId, enableMotor);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3DistanceJoint_EnableMotor(both_cpu_joint(jointId), enableMotor);
	}
}

B3_API void b3DistanceJoint_SetMotorSpeed(b3JointId jointId, float motorSpeed)
{
	gpu_b3_distance_set_motor_speed(jointId, motorSpeed);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3DistanceJoint_SetMotorSpeed(both_cpu_joint(jointId), motorSpeed);
	}
}

B3_API void b3DistanceJoint_SetMaxMotorForce(b3JointId jointId, float force)
{
	gpu_b3_distance_set_max_motor_force(jointId, force);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3DistanceJoint_SetMaxMotorForce(both_cpu_joint(jointId), force);
	}
}

B3_API void b3ParallelJoint_SetSpringHertz(b3JointId jointId, float hertz)
{
	gpu_b3_parallel_set_spring_hertz(jointId, hertz);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3ParallelJoint_SetSpringHertz(both_cpu_joint(jointId), hertz);
	}
}

B3_API void b3ParallelJoint_SetSpringDampingRatio(b3JointId jointId, float dampingRatio)
{
	gpu_b3_parallel_set_spring_damping(jointId, dampingRatio);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3ParallelJoint_SetSpringDampingRatio(both_cpu_joint(jointId), dampingRatio);
	}
}

B3_API void b3ParallelJoint_SetMaxTorque(b3JointId jointId, float maxTorque)
{
	gpu_b3_parallel_set_max_torque(jointId, maxTorque);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3ParallelJoint_SetMaxTorque(both_cpu_joint(jointId), maxTorque);
	}
}

B3_API void b3PrismaticJoint_EnableSpring(b3JointId jointId, bool enableSpring)
{
	gpu_b3_prismatic_enable_spring(jointId, enableSpring);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3PrismaticJoint_EnableSpring(both_cpu_joint(jointId), enableSpring);
	}
}

B3_API void b3PrismaticJoint_SetSpringHertz(b3JointId jointId, float hertz)
{
	gpu_b3_prismatic_set_spring_hertz(jointId, hertz);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3PrismaticJoint_SetSpringHertz(both_cpu_joint(jointId), hertz);
	}
}

B3_API void b3PrismaticJoint_SetSpringDampingRatio(b3JointId jointId, float dampingRatio)
{
	gpu_b3_prismatic_set_spring_damping(jointId, dampingRatio);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3PrismaticJoint_SetSpringDampingRatio(both_cpu_joint(jointId), dampingRatio);
	}
}

B3_API void b3PrismaticJoint_SetTargetTranslation(b3JointId jointId, float targetTranslation)
{
	gpu_b3_prismatic_set_target_translation(jointId, targetTranslation);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3PrismaticJoint_SetTargetTranslation(both_cpu_joint(jointId), targetTranslation);
	}
}

B3_API void b3PrismaticJoint_EnableLimit(b3JointId jointId, bool enableLimit)
{
	gpu_b3_prismatic_enable_limit(jointId, enableLimit);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3PrismaticJoint_EnableLimit(both_cpu_joint(jointId), enableLimit);
	}
}

B3_API void b3PrismaticJoint_SetLimits(b3JointId jointId, float lower, float upper)
{
	gpu_b3_prismatic_set_limits(jointId, lower, upper);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3PrismaticJoint_SetLimits(both_cpu_joint(jointId), lower, upper);
	}
}

B3_API void b3PrismaticJoint_EnableMotor(b3JointId jointId, bool enableMotor)
{
	gpu_b3_prismatic_enable_motor(jointId, enableMotor);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3PrismaticJoint_EnableMotor(both_cpu_joint(jointId), enableMotor);
	}
}

B3_API void b3PrismaticJoint_SetMotorSpeed(b3JointId jointId, float motorSpeed)
{
	gpu_b3_prismatic_set_motor_speed(jointId, motorSpeed);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3PrismaticJoint_SetMotorSpeed(both_cpu_joint(jointId), motorSpeed);
	}
}

B3_API void b3PrismaticJoint_SetMaxMotorForce(b3JointId jointId, float force)
{
	gpu_b3_prismatic_set_max_motor_force(jointId, force);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3PrismaticJoint_SetMaxMotorForce(both_cpu_joint(jointId), force);
	}
}

B3_API b3JointId b3CreateWeldJoint(b3WorldId worldId, const b3WeldJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	GpuJointId gpu = gpu_b3_create_weld(
		worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.localFrameA.p.x, def->base.localFrameA.p.y,
		def->base.localFrameA.p.z, def->base.localFrameB.p.x, def->base.localFrameB.p.y, def->base.localFrameB.p.z,
		def->base.localFrameA.q.v.x, def->base.localFrameA.q.v.y, def->base.localFrameA.q.v.z,
		def->base.localFrameA.q.s, def->base.localFrameB.q.v.x, def->base.localFrameB.q.v.y,
		def->base.localFrameB.q.v.z, def->base.localFrameB.q.s, def->base.constraintHertz,
		def->base.constraintDampingRatio, def->base.collideConnected, def->linearHertz, def->linearDampingRatio,
		def->angularHertz, def->angularDampingRatio);
	configure_gpu_joint(gpu, &def->base);
	if (both_has_cpu_world(worldId))
	{
		b3WeldJointDef cpu_def = *def;
		cpu_def.base.bodyIdA = both_cpu_body(def->base.bodyIdA);
		cpu_def.base.bodyIdB = both_cpu_body(def->base.bodyIdB);
		both_map_joint(gpu, cpu_b3CreateWeldJoint(both_cpu_world(worldId), &cpu_def));
	}
	return gpu;
}

B3_API void b3WeldJoint_SetLinearHertz(b3JointId jointId, float hertz)
{
	gpu_b3_weld_set_linear_hertz(jointId, hertz);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3WeldJoint_SetLinearHertz(both_cpu_joint(jointId), hertz);
	}
}

B3_API float b3WeldJoint_GetLinearHertz(b3JointId jointId)
{
	return gpu_b3_weld_get_linear_hertz(jointId);
}

B3_API void b3WeldJoint_SetLinearDampingRatio(b3JointId jointId, float dampingRatio)
{
	gpu_b3_weld_set_linear_damping(jointId, dampingRatio);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3WeldJoint_SetLinearDampingRatio(both_cpu_joint(jointId), dampingRatio);
	}
}

B3_API float b3WeldJoint_GetLinearDampingRatio(b3JointId jointId)
{
	return gpu_b3_weld_get_linear_damping(jointId);
}

B3_API void b3WeldJoint_SetAngularHertz(b3JointId jointId, float hertz)
{
	gpu_b3_weld_set_angular_hertz(jointId, hertz);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3WeldJoint_SetAngularHertz(both_cpu_joint(jointId), hertz);
	}
}

B3_API float b3WeldJoint_GetAngularHertz(b3JointId jointId)
{
	return gpu_b3_weld_get_angular_hertz(jointId);
}

B3_API void b3WeldJoint_SetAngularDampingRatio(b3JointId jointId, float dampingRatio)
{
	gpu_b3_weld_set_angular_damping(jointId, dampingRatio);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3WeldJoint_SetAngularDampingRatio(both_cpu_joint(jointId), dampingRatio);
	}
}

B3_API float b3WeldJoint_GetAngularDampingRatio(b3JointId jointId)
{
	return gpu_b3_weld_get_angular_damping(jointId);
}

static b3JointId create_gpu_motor(b3WorldId worldId, const b3MotorJointDef* def)
{
	if (!def)
	{
		return (b3JointId){0};
	}
	GpuJointId gpu = gpu_b3_create_motor(
		worldId, def->base.bodyIdA, def->base.bodyIdB, def->base.localFrameA.p.x, def->base.localFrameA.p.y,
		def->base.localFrameA.p.z, def->base.localFrameB.p.x, def->base.localFrameB.p.y, def->base.localFrameB.p.z,
		def->base.localFrameA.q.v.x, def->base.localFrameA.q.v.y, def->base.localFrameA.q.v.z,
		def->base.localFrameA.q.s, def->base.localFrameB.q.v.x, def->base.localFrameB.q.v.y,
		def->base.localFrameB.q.v.z, def->base.localFrameB.q.s, def->linearVelocity.x, def->linearVelocity.y,
		def->linearVelocity.z, def->maxVelocityForce, def->angularVelocity.x, def->angularVelocity.y,
		def->angularVelocity.z, def->maxVelocityTorque, def->linearHertz, def->linearDampingRatio,
		def->maxSpringForce, def->angularHertz, def->angularDampingRatio, def->maxSpringTorque,
		def->base.collideConnected);
	configure_gpu_joint(gpu, &def->base);
    return gpu;
}

B3_API b3JointId b3CreateMotorJoint(b3WorldId worldId, const b3MotorJointDef* def)
{
    if (!def) return (b3JointId){0};
    b3JointId gpu = create_gpu_motor(worldId, def);
	if (both_has_cpu_world(worldId))
	{
		b3MotorJointDef cpu_def = *def;
		cpu_def.base.bodyIdA = both_cpu_body(def->base.bodyIdA);
		cpu_def.base.bodyIdB = both_cpu_body(def->base.bodyIdB);
		both_map_joint(gpu, cpu_b3CreateMotorJoint(both_cpu_world(worldId), &cpu_def));
	}
	return gpu;
}

B3_API void b3MotorJoint_SetLinearVelocity(b3JointId jointId, b3Vec3 velocity)
{
	gpu_b3_motor_set_linear_velocity(jointId, velocity.x, velocity.y, velocity.z);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3MotorJoint_SetLinearVelocity(both_cpu_joint(jointId), velocity);
	}
}

B3_API b3Vec3 b3MotorJoint_GetLinearVelocity(b3JointId jointId)
{
	return gpu_b3_motor_get_linear_velocity(jointId);
}

B3_API void b3MotorJoint_SetAngularVelocity(b3JointId jointId, b3Vec3 velocity)
{
	gpu_b3_motor_set_angular_velocity(jointId, velocity.x, velocity.y, velocity.z);
	if (both_has_cpu_joint(jointId))
	{
		cpu_b3MotorJoint_SetAngularVelocity(both_cpu_joint(jointId), velocity);
	}
}

B3_API b3Vec3 b3MotorJoint_GetAngularVelocity(b3JointId jointId)
{
	return gpu_b3_motor_get_angular_velocity(jointId);
}

#define BOTH_MOTOR_SCALAR_API(Name, suffix)                                                          \
	B3_API void b3MotorJoint_Set##Name(b3JointId jointId, float value)                                \
	{                                                                                                 \
		gpu_b3_motor_set_##suffix(jointId, value);                                                       \
		if (both_has_cpu_joint(jointId))                                                                 \
		{                                                                                               \
			cpu_b3MotorJoint_Set##Name(both_cpu_joint(jointId), value);                                    \
		}                                                                                               \
	}                                                                                                 \
	B3_API float b3MotorJoint_Get##Name(b3JointId jointId)                                            \
	{                                                                                                 \
		return gpu_b3_motor_get_##suffix(jointId);                                                       \
	}

BOTH_MOTOR_SCALAR_API(MaxVelocityForce, max_velocity_force)
BOTH_MOTOR_SCALAR_API(MaxVelocityTorque, max_velocity_torque)
BOTH_MOTOR_SCALAR_API(LinearHertz, linear_hertz)
BOTH_MOTOR_SCALAR_API(LinearDampingRatio, linear_damping)
BOTH_MOTOR_SCALAR_API(AngularHertz, angular_hertz)
BOTH_MOTOR_SCALAR_API(AngularDampingRatio, angular_damping)
BOTH_MOTOR_SCALAR_API(MaxSpringForce, max_spring_force)
BOTH_MOTOR_SCALAR_API(MaxSpringTorque, max_spring_torque)

#undef BOTH_MOTOR_SCALAR_API

B3_API bool b3Joint_IsValid(b3JointId jointId)
{

	return both_has_cpu_joint(jointId);
}

B3_API void b3DestroyJoint(b3JointId jointId, bool wakeAttached)
{

	if (both_has_cpu_joint(jointId))
	{
		cpu_b3DestroyJoint(both_cpu_joint(jointId), wakeAttached);
		both_unmap_joint(jointId);
	}
	gpu_b3_destroy_joint(jointId, wakeAttached);
}

B3_API b3Pos b3Body_GetPosition(b3BodyId bodyId)
{
	b3WorldTransform xf = gpu_samples_body_transform(bodyId);
	return xf.p;
}

B3_API b3Quat b3Body_GetRotation(b3BodyId bodyId)
{
	return gpu_samples_body_transform(bodyId).q;
}

B3_API b3WorldTransform b3Body_GetTransform(b3BodyId bodyId)
{
	if (gpu_b3_body_is_valid(bodyId))
	{
		return gpu_samples_body_transform(bodyId);
	}
	return cpu_b3Body_GetTransform(both_cpu_body(bodyId));
}

B3_API void b3Body_SetTargetTransform(b3BodyId bodyId, b3WorldTransform target, float timeStep, bool wake)
{
	gpu_b3_body_set_target_transform(bodyId, target.p.x, target.p.y, target.p.z, target.q.v.x, target.q.v.y,
									target.q.v.z, target.q.s, timeStep, wake);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_SetTargetTransform(both_cpu_body(bodyId), target, timeStep, wake);
	}
}

B3_API b3BodyType b3Body_GetType(b3BodyId bodyId)
{
	return (b3BodyType)gpu_b3_body_get_type(bodyId);
}

B3_API void b3Body_SetType(b3BodyId bodyId, b3BodyType type)
{
	gpu_b3_body_set_type(bodyId, (int)type);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_SetType(both_cpu_body(bodyId), type);
	}
}

B3_API void b3Body_SetTransform(b3BodyId bodyId, b3Pos position, b3Quat rotation)
{
	b3WorldTransform transform = {position, rotation};
	gpu_samples_body_set_transform(bodyId, transform);
	gpu_b3_body_set_transform(bodyId, position.x, position.y, position.z, rotation.v.x, rotation.v.y, rotation.v.z,
							  rotation.s);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_SetTransform(both_cpu_body(bodyId), position, rotation);
	}
}

B3_API void b3Body_SetAwake(b3BodyId bodyId, bool awake)
{
	gpu_b3_body_set_awake(bodyId, awake);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_SetAwake(both_cpu_body(bodyId), awake);
	}
}

extern void gpu_b3_body_enable_contact_recycling(GpuBodyId body, bool enable);
extern bool gpu_b3_body_is_contact_recycling_enabled(GpuBodyId body);
extern void cpu_b3Body_EnableContactRecycling(b3BodyId body, bool enable);

B3_API void b3Body_EnableContactRecycling(b3BodyId bodyId, bool enable)
{
    gpu_b3_body_enable_contact_recycling(bodyId, enable);
    if (both_has_cpu_body(bodyId))
    {
        cpu_b3Body_EnableContactRecycling(both_cpu_body(bodyId), enable);
    }
}

B3_API bool b3Body_IsContactRecyclingEnabled(b3BodyId bodyId)
{
    return gpu_b3_body_is_contact_recycling_enabled(bodyId);
}

B3_API void b3Body_SetBullet(b3BodyId bodyId, bool flag)
{
	gpu_b3_body_set_bullet(bodyId, flag);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_SetBullet(both_cpu_body(bodyId), flag);
	}
}

B3_API bool b3Body_IsBullet(b3BodyId bodyId)
{
	return gpu_b3_body_is_bullet(bodyId);
}

B3_API void b3Body_AllowFastRotation(b3BodyId bodyId, bool flag)
{
	gpu_b3_body_allow_fast_rotation(bodyId, flag);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_AllowFastRotation(both_cpu_body(bodyId), flag);
	}
}

B3_API bool b3Body_IsFastRotationAllowed(b3BodyId bodyId)
{
	return gpu_b3_body_is_fast_rotation_allowed(bodyId);
}

B3_API void b3Body_SetLinearDamping(b3BodyId bodyId, float damping)
{
	gpu_b3_body_set_linear_damping(bodyId, damping);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_SetLinearDamping(both_cpu_body(bodyId), damping);
	}
}

B3_API void b3Body_SetAngularDamping(b3BodyId bodyId, float damping)
{
	gpu_b3_body_set_angular_damping(bodyId, damping);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_SetAngularDamping(both_cpu_body(bodyId), damping);
	}
}

B3_API void b3Body_SetGravityScale(b3BodyId bodyId, float scale)
{
	gpu_b3_body_set_gravity_scale(bodyId, scale);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_SetGravityScale(both_cpu_body(bodyId), scale);
	}
}

B3_API void b3Body_SetLinearVelocity(b3BodyId bodyId, b3Vec3 linearVelocity)
{
	gpu_b3_body_set_linear_velocity(bodyId, linearVelocity.x, linearVelocity.y, linearVelocity.z);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_SetLinearVelocity(both_cpu_body(bodyId), linearVelocity);
	}
}

B3_API void b3Body_SetAngularVelocity(b3BodyId bodyId, b3Vec3 angularVelocity)
{
	gpu_b3_body_set_angular_velocity(bodyId, angularVelocity.x, angularVelocity.y, angularVelocity.z);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_SetAngularVelocity(both_cpu_body(bodyId), angularVelocity);
	}
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
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_ApplyLinearImpulse(both_cpu_body(bodyId), impulse, point, wake);
	}
}

B3_API void b3Body_ApplyLinearImpulseToCenter(b3BodyId bodyId, b3Vec3 impulse, bool wake)
{
	gpu_b3_body_apply_linear_impulse_to_center(bodyId, impulse.x, impulse.y, impulse.z, wake);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_ApplyLinearImpulseToCenter(both_cpu_body(bodyId), impulse, wake);
	}
}

B3_API void b3Body_ApplyAngularImpulse(b3BodyId bodyId, b3Vec3 impulse, bool wake)
{
	gpu_b3_body_apply_angular_impulse(bodyId, impulse.x, impulse.y, impulse.z, wake);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_ApplyAngularImpulse(both_cpu_body(bodyId), impulse, wake);
	}
}

B3_API void b3Body_ApplyForce(b3BodyId bodyId, b3Vec3 force, b3Pos point, bool wake)
{
	gpu_b3_body_apply_force(bodyId, force.x, force.y, force.z, (float)point.x, (float)point.y, (float)point.z,
						   wake);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_ApplyForce(both_cpu_body(bodyId), force, point, wake);
	}
}

B3_API void b3Body_ApplyForceToCenter(b3BodyId bodyId, b3Vec3 force, bool wake)
{
	gpu_b3_body_apply_force_to_center(bodyId, force.x, force.y, force.z, wake);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_ApplyForceToCenter(both_cpu_body(bodyId), force, wake);
	}
}

B3_API void b3Body_ApplyTorque(b3BodyId bodyId, b3Vec3 torque, bool wake)
{
	gpu_b3_body_apply_torque(bodyId, torque.x, torque.y, torque.z, wake);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_ApplyTorque(both_cpu_body(bodyId), torque, wake);
	}
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
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_SetMassData(both_cpu_body(bodyId), massData);
	}
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
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_Disable(both_cpu_body(bodyId));
	}
}

B3_API void b3Body_Enable(b3BodyId bodyId)
{
	gpu_b3_body_set_enabled(bodyId, true);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_Enable(both_cpu_body(bodyId));
	}
}

B3_API void b3Body_SetMotionLocks(b3BodyId bodyId, b3MotionLocks locks)
{
	gpu_b3_body_set_motion_locks(bodyId, locks.linearX, locks.linearY, locks.linearZ, locks.angularX,
								 locks.angularY, locks.angularZ);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_SetMotionLocks(both_cpu_body(bodyId), locks);
	}
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

B3_API b3Pos b3Body_GetWorldCenter(b3BodyId bodyId)
{
	b3Pos center = {0};
	gpu_b3_body_get_world_center(bodyId, &center.x);
	return center;
}

B3_API void b3Body_ApplyMassFromShapes(b3BodyId bodyId)
{
	gpu_b3_body_apply_mass_from_shapes(bodyId);
	if (both_has_cpu_body(bodyId))
	{
		cpu_b3Body_ApplyMassFromShapes(both_cpu_body(bodyId));
	}
}

B3_API void b3Shape_SetName(b3ShapeId shapeId, const char* name)
{
	gpu_samples_shape_set_name(shapeId, name);
	if (both_has_cpu_shape(shapeId))
	{
		cpu_b3Shape_SetName(both_cpu_shape(shapeId), name);
	}
}

B3_API const char* b3Shape_GetName(b3ShapeId shapeId)
{
	const char* n = gpu_samples_shape_get_name(shapeId);
	if (n != NULL && n[0] != 0)
	{
		return n;
	}
	if (both_has_cpu_shape(shapeId))
	{
		return cpu_b3Shape_GetName(both_cpu_shape(shapeId));
	}
	return n;
}

B3_API void b3Shape_SetUserData(b3ShapeId shapeId, void* userData)
{
	gpu_b3_shape_set_user_data(shapeId, (uintptr_t)userData);
	if (both_has_cpu_shape(shapeId))
	{
		cpu_b3Shape_SetUserData(both_cpu_shape(shapeId), userData);
	}
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
	if (both_has_cpu_shape(shapeId))
	{
		cpu_b3Shape_SetFriction(both_cpu_shape(shapeId), friction);
	}
}

B3_API void b3Shape_SetRestitution(b3ShapeId shapeId, float restitution)
{
	gpu_b3_shape_set_restitution(shapeId, restitution);
	if (both_has_cpu_shape(shapeId))
	{
		cpu_b3Shape_SetRestitution(both_cpu_shape(shapeId), restitution);
	}
}

B3_API void b3Shape_ApplyWind(b3ShapeId shapeId, b3Vec3 wind, float drag, float lift, float maxSpeed, bool wake)
{
	gpu_b3_shape_apply_wind(shapeId, wind.x, wind.y, wind.z, drag, lift, maxSpeed, wake);
	if (both_has_cpu_shape(shapeId))
	{
		cpu_b3Shape_ApplyWind(both_cpu_shape(shapeId), wind, drag, lift, maxSpeed, wake);
	}
}

B3_API b3Sphere b3Shape_GetSphere(b3ShapeId shapeId)
{
	return gpu_b3_shape_get_sphere(shapeId);
}

B3_API b3Capsule b3Shape_GetCapsule(b3ShapeId shapeId)
{
	return gpu_b3_shape_get_capsule(shapeId);
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

extern void gpu_b3_world_set_contact_recycle_distance(GpuWorldId id, float distance);
extern float gpu_b3_world_get_contact_recycle_distance(GpuWorldId id);
extern void cpu_b3World_SetContactRecycleDistance(b3WorldId id, float distance);
B3_API void b3World_SetContactRecycleDistance(b3WorldId id, float distance) {
    gpu_b3_world_set_contact_recycle_distance(id, distance);
    cpu_b3World_SetContactRecycleDistance(both_cpu_world(id), distance);
}
B3_API float b3World_GetContactRecycleDistance(b3WorldId id) {
    return gpu_b3_world_get_contact_recycle_distance(id);
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
extern void cpu_b3Shape_SetDensity(b3ShapeId id, float density, bool updateMass);
B3_API void b3Shape_SetDensity(b3ShapeId id, float density, bool updateMass)
{
    gpu_b3_shape_set_density(id, density, updateMass);
    if (both_has_cpu_shape(id)) cpu_b3Shape_SetDensity(both_cpu_shape(id), density, updateMass);
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

extern void cpu_b3World_SetMaximumLinearSpeed(b3WorldId, float);
B3_API void b3World_SetMaximumLinearSpeed(b3WorldId id, float speed) {
    gpu_b3_world_set_maximum_linear_speed(id, speed);
    if (both_has_cpu_world(id)) cpu_b3World_SetMaximumLinearSpeed(both_cpu_world(id), speed);
}
B3_API float b3World_GetMaximumLinearSpeed(b3WorldId id) {
    return gpu_b3_world_get_maximum_linear_speed(id);
}

extern void cpu_b3Joint_SetCollideConnected(b3JointId, bool);
extern void gpu_b3_joint_set_collide_connected(GpuJointId, bool);
extern bool gpu_b3_joint_get_collide_connected(GpuJointId);
B3_API bool b3Joint_GetCollideConnected(b3JointId id) { return gpu_b3_joint_get_collide_connected(id); }
B3_API void b3Joint_SetCollideConnected(b3JointId id, bool enable) {
    gpu_b3_joint_set_collide_connected(id, enable);
    if (both_has_cpu_joint(id)) cpu_b3Joint_SetCollideConnected(both_cpu_joint(id), enable);
}

B3_API float b3World_GetRestitutionThreshold(b3WorldId id) { return gpu_b3_world_get_restitution_threshold(id); }
extern void cpu_b3World_SetRestitutionThreshold(b3WorldId, float);
B3_API void b3World_SetRestitutionThreshold(b3WorldId id, float value) {
    gpu_b3_world_set_restitution_threshold(id, value);
    if (both_has_cpu_world(id)) cpu_b3World_SetRestitutionThreshold(both_cpu_world(id), value);
}

// Comparison input: identical rays, independent hits, masses, anchors and depth.
extern b3RayResult cpu_b3World_CastRayClosest(b3WorldId, b3Pos, b3Vec3, b3QueryFilter);
extern b3BodyId cpu_b3Shape_GetBody(b3ShapeId);
extern bool cpu_b3Body_IsValid(b3BodyId);
extern bool cpu_b3Joint_IsValid(b3JointId);
void both_draw_second(void) {
    g_second_view = true;
    if (!g_draw_world.index1) return;
    extern void SetSelectedBody(b3BodyId);
    SetSelectedBody(g_drag.selected[1]);
    gpu_samples_world_draw(g_draw_world, &g_split_draw, g_draw_mask);

}
void both_pointer_up(void) {
    if (g_drag.joint[0].index1 && cpu_b3Joint_IsValid(g_drag.joint[0])) cpu_b3DestroyJoint(g_drag.joint[0], true);
    if (g_drag.mouse[0].index1 && cpu_b3Body_IsValid(g_drag.mouse[0])) cpu_b3DestroyBody(g_drag.mouse[0]);
    if (g_drag.joint[1].index1 && gpu_b3_joint_is_valid(g_drag.joint[1])) gpu_b3_destroy_joint(g_drag.joint[1], true);
    if (g_drag.mouse[1].index1 && gpu_b3_body_is_valid(g_drag.mouse[1])) gpu_samples_destroy_body(g_drag.mouse[1]);
    memset(g_drag.mouse, 0, sizeof(g_drag.mouse));
    memset(g_drag.joint, 0, sizeof(g_drag.joint));
}
void both_pointer_move(b3Pos origin, b3Vec3 translation) {
    g_drag.origin = origin; g_drag.translation = translation;
}
void both_pointer_down(b3WorldId world, b3Pos origin, b3Vec3 translation, bool grab, float force_scale) {
    both_pointer_up();
    g_drag.world = world;
    both_pointer_move(origin, translation);
    b3QueryFilter filter = b3DefaultQueryFilter(); filter.name = grab ? "grab" : "select";
    b3RayResult hits[2] = {
        cpu_b3World_CastRayClosest(both_cpu_world(world), origin, translation, filter),
        gpu_b3_world_cast_ray_closest(world, origin, translation, filter)
    };
    for (int i = 0; i < 2; ++i) {
        b3RayResult hit = hits[i];
        b3BodyId body = hit.hit ? (i ? b3Shape_GetBody(hit.shapeId) : cpu_b3Shape_GetBody(hit.shapeId)) : (b3BodyId){0};
        g_drag.selected[i] = body;
        if (!grab || !hit.hit || (i ? b3Body_GetType(body) : cpu_b3Body_GetType(body)) != b3_dynamicBody) continue;
        g_drag.grabbed[i] = body;
        g_drag.fraction[i] = hit.fraction;
        b3BodyDef bd = b3DefaultBodyDef(); bd.type = b3_kinematicBody; bd.position = hit.point; bd.enableSleep = false;
        b3BodyId mouse = i ? gpu_b3_create_body(world, b3_kinematicBody, hit.point.x, hit.point.y, hit.point.z,
            0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0) : cpu_b3CreateBody(both_cpu_world(world), &bd);
        g_drag.mouse[i] = mouse;
        b3MotorJointDef jd = b3DefaultMotorJointDef();
        jd.base.bodyIdA = mouse; jd.base.bodyIdB = body;
        jd.base.localFrameB.p = i ? b3Body_GetLocalPoint(body, hit.point) : cpu_b3Body_GetLocalPoint(body, hit.point);
        g_drag.local_anchor[i] = jd.base.localFrameB.p;
        jd.linearHertz = 7.5f; jd.linearDampingRatio = 1.0f;
        b3MassData mass = i ? b3Body_GetMassData(body) : cpu_b3Body_GetMassData(body);
        b3Vec3 gravity = i ? b3World_GetGravity(world) : cpu_b3World_GetGravity(both_cpu_world(world));
        float mg = mass.mass * b3Length(gravity);
        jd.maxSpringForce = force_scale * mg;
        if (mass.mass > 0) {
            float trace = mass.inertia.cx.x + mass.inertia.cy.y + mass.inertia.cz.z;
            jd.maxVelocityTorque = 0.5f * sqrtf(trace / (3.0f * mass.mass)) * mg;
        }
        if (getenv("BOTH_DRAG_MASS_TRACE")) {
            fprintf(stderr, "drag-mass %d mass %.9g inertia %.9g %.9g %.9g torque %.9g\n",
                i, mass.mass, mass.inertia.cx.x, mass.inertia.cy.y, mass.inertia.cz.z, jd.maxVelocityTorque);
        }
        g_drag.joint[i] = i ? create_gpu_motor(world, &jd) : cpu_b3CreateMotorJoint(both_cpu_world(world), &jd);
        if (i) gpu_b3_body_set_awake(body, true); else cpu_b3Body_SetAwake(body, true);
    }
    extern void SetSelectedBody(b3BodyId);
    SetSelectedBody(g_drag.selected[1]);
}
static void both_drag_step(b3WorldId world, float dt) {
    if (dt <= 0 || world.index1 != g_drag.world.index1) return;
    for (int i = 0; i < 2; ++i) {
        if (!g_drag.mouse[i].index1) continue;
        bool valid = i ? gpu_b3_joint_is_valid(g_drag.joint[i]) : cpu_b3Joint_IsValid(g_drag.joint[i]);
        if (!valid) { both_pointer_up(); return; }
        b3Pos p = {
            g_drag.origin.x + g_drag.fraction[i] * g_drag.translation.x,
            g_drag.origin.y + g_drag.fraction[i] * g_drag.translation.y,
            g_drag.origin.z + g_drag.fraction[i] * g_drag.translation.z
        };
        if (i) gpu_b3_body_set_target_transform(g_drag.mouse[i], p.x, p.y, p.z, 0, 0, 0, 1, dt, true);
        else cpu_b3Body_SetTargetTransform(g_drag.mouse[i], (b3WorldTransform){p, b3Quat_identity}, dt, true);
    }
}

BothPointerState both_pointer_state(int engine) {
    if (engine < 0 || engine > 1) return (BothPointerState){0};
    return (BothPointerState){g_drag.selected[engine], g_drag.mouse[engine], g_drag.joint[engine], g_drag.fraction[engine], g_drag.local_anchor[engine]};
}

extern b3BodyId gpu_b3_shape_get_body(b3ShapeId shape);
B3_API b3BodyId b3Shape_GetBody(b3ShapeId shape) { return gpu_b3_shape_get_body(shape); }

void both_pointer_impulse(b3WorldId world, b3Pos origin, b3Vec3 translation, b3Vec3 impulse) {
    b3QueryFilter filter = b3DefaultQueryFilter();
    b3RayResult cpu = cpu_b3World_CastRayClosest(both_cpu_world(world), origin, translation, filter);
    b3RayResult gpu = gpu_b3_world_cast_ray_closest(world, origin, translation, filter);
    if (cpu.hit) cpu_b3Body_ApplyLinearImpulse(cpu_b3Shape_GetBody(cpu.shapeId), impulse, cpu.point, true);
    if (gpu.hit) gpu_b3_body_apply_linear_impulse(gpu_b3_shape_get_body(gpu.shapeId),
        impulse.x, impulse.y, impulse.z, gpu.point.x, gpu.point.y, gpu.point.z, true);
}

extern void gpu_b3_world_set_contact_tuning(b3WorldId id, float hertz, float damping, float speed);
extern void cpu_b3World_SetContactTuning(b3WorldId id, float hertz, float damping, float speed);
B3_API void b3World_SetContactTuning(b3WorldId id, float hertz, float damping, float speed) { gpu_b3_world_set_contact_tuning(id, hertz, damping, speed);
    if (both_has_cpu_world(id)) cpu_b3World_SetContactTuning(both_cpu_world(id), hertz, damping, speed); }

extern void gpu_b3_world_set_user_data(b3WorldId id, uintptr_t value);
extern void cpu_b3World_SetUserData(b3WorldId id, void* value);
B3_API void b3World_SetUserData(b3WorldId id, void* value) { gpu_b3_world_set_user_data(id, (uintptr_t)value);
    if (both_has_cpu_world(id)) cpu_b3World_SetUserData(both_cpu_world(id), value); }

extern uintptr_t gpu_b3_world_get_user_data(b3WorldId id);
extern void* cpu_b3World_GetUserData(b3WorldId id);
B3_API void* b3World_GetUserData(b3WorldId id) { return (void*)gpu_b3_world_get_user_data(id); }

extern int gpu_b3_world_get_awake_body_count(b3WorldId id);
extern int cpu_b3World_GetAwakeBodyCount(b3WorldId id);
B3_API int b3World_GetAwakeBodyCount(b3WorldId id) { return gpu_b3_world_get_awake_body_count(id); }

extern void gpu_b3_body_enable_sleep(b3BodyId id, bool enable);
extern void cpu_b3Body_EnableSleep(b3BodyId id, bool enable);
B3_API void b3Body_EnableSleep(b3BodyId id, bool enable) { gpu_b3_body_enable_sleep(id, enable);
    if (both_has_cpu_body(id)) cpu_b3Body_EnableSleep(both_cpu_body(id), enable); }

extern bool gpu_b3_body_is_sleep_enabled(b3BodyId id);
extern bool cpu_b3Body_IsSleepEnabled(b3BodyId id);
B3_API bool b3Body_IsSleepEnabled(b3BodyId id) { return gpu_b3_body_is_sleep_enabled(id); }

extern void gpu_b3_body_set_sleep_threshold(b3BodyId id, float value);
extern void cpu_b3Body_SetSleepThreshold(b3BodyId id, float value);
B3_API void b3Body_SetSleepThreshold(b3BodyId id, float value) { gpu_b3_body_set_sleep_threshold(id, value);
    if (both_has_cpu_body(id)) cpu_b3Body_SetSleepThreshold(both_cpu_body(id), value); }

extern float gpu_b3_body_get_sleep_threshold(b3BodyId id);
extern float cpu_b3Body_GetSleepThreshold(b3BodyId id);
B3_API float b3Body_GetSleepThreshold(b3BodyId id) { return gpu_b3_body_get_sleep_threshold(id); }

extern void gpu_b3_body_enable_hit_events(b3BodyId id, bool enable);
extern void cpu_b3Body_EnableHitEvents(b3BodyId id, bool enable);
B3_API void b3Body_EnableHitEvents(b3BodyId id, bool enable) { gpu_b3_body_enable_hit_events(id, enable);
    if (both_has_cpu_body(id)) cpu_b3Body_EnableHitEvents(both_cpu_body(id), enable); }

extern void gpu_b3_body_set_name(b3BodyId id, const char* name);
extern void cpu_b3Body_SetName(b3BodyId id, const char* name);
B3_API void b3Body_SetName(b3BodyId id, const char* name) { gpu_b3_body_set_name(id, name);
    if (both_has_cpu_body(id)) cpu_b3Body_SetName(both_cpu_body(id), name); }

extern const char* gpu_b3_body_get_name(b3BodyId id);
extern const char* cpu_b3Body_GetName(b3BodyId id);
B3_API const char* b3Body_GetName(b3BodyId id) { return gpu_b3_body_get_name(id); }

extern void gpu_b3_joint_set_local_frame(b3JointId, bool, float, float, float, float, float, float, float);
extern void gpu_b3_joint_get_local_frame(b3JointId, bool, float*);
extern void gpu_b3_joint_wake_bodies(b3JointId);
extern void cpu_b3Joint_SetLocalFrameA(b3JointId, b3Transform);
B3_API void b3Joint_SetLocalFrameA(b3JointId id, b3Transform frame) {
    gpu_b3_joint_set_local_frame(id, false, frame.p.x, frame.p.y, frame.p.z, frame.q.v.x, frame.q.v.y, frame.q.v.z, frame.q.s);
    if (both_has_cpu_joint(id)) cpu_b3Joint_SetLocalFrameA(both_cpu_joint(id), frame);
}
B3_API b3Transform b3Joint_GetLocalFrameA(b3JointId id) {
    float f[7]; gpu_b3_joint_get_local_frame(id, false, f);
    return (b3Transform){{f[0],f[1],f[2]},{{f[3],f[4],f[5]},f[6]}};
}
extern void cpu_b3Joint_SetLocalFrameB(b3JointId, b3Transform);
B3_API void b3Joint_SetLocalFrameB(b3JointId id, b3Transform frame) {
    gpu_b3_joint_set_local_frame(id, true, frame.p.x, frame.p.y, frame.p.z, frame.q.v.x, frame.q.v.y, frame.q.v.z, frame.q.s);
    if (both_has_cpu_joint(id)) cpu_b3Joint_SetLocalFrameB(both_cpu_joint(id), frame);
}
B3_API b3Transform b3Joint_GetLocalFrameB(b3JointId id) {
    float f[7]; gpu_b3_joint_get_local_frame(id, true, f);
    return (b3Transform){{f[0],f[1],f[2]},{{f[3],f[4],f[5]},f[6]}};
}
extern void cpu_b3Joint_WakeBodies(b3JointId);
B3_API void b3Joint_WakeBodies(b3JointId id) { gpu_b3_joint_wake_bodies(id);
    if (both_has_cpu_joint(id)) cpu_b3Joint_WakeBodies(both_cpu_joint(id));
}

extern b3MassData gpu_b3_shape_compute_mass_data(b3ShapeId);
B3_API b3MassData b3Shape_ComputeMassData(b3ShapeId id) { return gpu_b3_shape_compute_mass_data(id); }

// Read comparison state from the GPU world; CPU references use explicit cpu_ APIs.
extern int gpu_b3_body_get_shape_count(GpuBodyId body);
extern int gpu_b3_body_get_shapes(GpuBodyId body, GpuShapeId* out, int capacity);
extern float gpu_b3_distance_get_length(GpuJointId id);
extern bool gpu_b3_distance_is_spring_enabled(GpuJointId id);
extern void gpu_b3_distance_get_spring_force_range(GpuJointId id, float* lower, float* upper);
extern float gpu_b3_distance_get_spring_hertz(GpuJointId id);
extern float gpu_b3_distance_get_spring_damping(GpuJointId id);
extern bool gpu_b3_distance_is_limit_enabled(GpuJointId id);
extern float gpu_b3_distance_get_min_length(GpuJointId id);
extern float gpu_b3_distance_get_max_length(GpuJointId id);
extern float gpu_b3_distance_get_current_length(GpuJointId id);
extern bool gpu_b3_distance_is_motor_enabled(GpuJointId id);
extern float gpu_b3_distance_get_motor_speed(GpuJointId id);
extern float gpu_b3_distance_get_max_motor_force(GpuJointId id);
extern float gpu_b3_distance_get_motor_force(GpuJointId id);
extern float gpu_b3_parallel_get_spring_hertz(GpuJointId id);
extern float gpu_b3_parallel_get_spring_damping(GpuJointId id);
extern float gpu_b3_parallel_get_max_torque(GpuJointId id);
extern bool gpu_b3_prismatic_is_spring_enabled(GpuJointId id);
extern float gpu_b3_prismatic_get_spring_hertz(GpuJointId id);
extern float gpu_b3_prismatic_get_spring_damping(GpuJointId id);
extern float gpu_b3_prismatic_get_target_translation(GpuJointId id);
extern bool gpu_b3_prismatic_is_limit_enabled(GpuJointId id);
extern float gpu_b3_prismatic_get_lower_limit(GpuJointId id);
extern float gpu_b3_prismatic_get_upper_limit(GpuJointId id);
extern bool gpu_b3_prismatic_is_motor_enabled(GpuJointId id);
extern float gpu_b3_prismatic_get_motor_speed(GpuJointId id);
extern float gpu_b3_prismatic_get_max_motor_force(GpuJointId id);
extern float gpu_b3_prismatic_get_motor_force(GpuJointId id);
extern float gpu_b3_prismatic_get_translation(GpuJointId id);
extern float gpu_b3_prismatic_get_speed(GpuJointId id);

B3_API int b3Body_GetShapeCount(b3BodyId bodyId)
{
	return gpu_b3_body_get_shape_count(bodyId);
}

B3_API int b3Body_GetShapes(b3BodyId bodyId, b3ShapeId* shapeArray, int capacity)
{
	return gpu_b3_body_get_shapes(bodyId, shapeArray, capacity);
}

B3_API float b3DistanceJoint_GetLength(b3JointId jointId)
{
	return gpu_b3_distance_get_length(jointId);
}

B3_API bool b3DistanceJoint_IsSpringEnabled(b3JointId jointId)
{
	return gpu_b3_distance_is_spring_enabled(jointId);
}

B3_API void b3DistanceJoint_GetSpringForceRange(b3JointId jointId, float* lowerForce, float* upperForce)
{
	gpu_b3_distance_get_spring_force_range(jointId, lowerForce, upperForce);
}

B3_API float b3DistanceJoint_GetSpringHertz(b3JointId jointId)
{
	return gpu_b3_distance_get_spring_hertz(jointId);
}

B3_API float b3DistanceJoint_GetSpringDampingRatio(b3JointId jointId)
{
	return gpu_b3_distance_get_spring_damping(jointId);
}

B3_API bool b3DistanceJoint_IsLimitEnabled(b3JointId jointId)
{
	return gpu_b3_distance_is_limit_enabled(jointId);
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

B3_API bool b3DistanceJoint_IsMotorEnabled(b3JointId jointId)
{
	return gpu_b3_distance_is_motor_enabled(jointId);
}

B3_API float b3DistanceJoint_GetMotorSpeed(b3JointId jointId)
{
	return gpu_b3_distance_get_motor_speed(jointId);
}

B3_API float b3DistanceJoint_GetMaxMotorForce(b3JointId jointId)
{
	return gpu_b3_distance_get_max_motor_force(jointId);
}

B3_API float b3DistanceJoint_GetMotorForce(b3JointId jointId)
{
	return gpu_b3_distance_get_motor_force(jointId);
}

B3_API float b3ParallelJoint_GetSpringHertz(b3JointId jointId)
{
	return gpu_b3_parallel_get_spring_hertz(jointId);
}

B3_API float b3ParallelJoint_GetSpringDampingRatio(b3JointId jointId)
{
	return gpu_b3_parallel_get_spring_damping(jointId);
}

B3_API float b3ParallelJoint_GetMaxTorque(b3JointId jointId)
{
	return gpu_b3_parallel_get_max_torque(jointId);
}

B3_API bool b3PrismaticJoint_IsSpringEnabled(b3JointId jointId)
{
	return gpu_b3_prismatic_is_spring_enabled(jointId);
}

B3_API float b3PrismaticJoint_GetSpringHertz(b3JointId jointId)
{
	return gpu_b3_prismatic_get_spring_hertz(jointId);
}

B3_API float b3PrismaticJoint_GetSpringDampingRatio(b3JointId jointId)
{
	return gpu_b3_prismatic_get_spring_damping(jointId);
}

B3_API float b3PrismaticJoint_GetTargetTranslation(b3JointId jointId)
{
	return gpu_b3_prismatic_get_target_translation(jointId);
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

B3_API bool b3PrismaticJoint_IsMotorEnabled(b3JointId jointId)
{
	return gpu_b3_prismatic_is_motor_enabled(jointId);
}

B3_API float b3PrismaticJoint_GetMotorSpeed(b3JointId jointId)
{
	return gpu_b3_prismatic_get_motor_speed(jointId);
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

B3_API void b3Shape_SetSphere(b3ShapeId id, const b3Sphere* sphere)
{
    if (both_has_cpu_shape(id)) { cpu_b3Shape_SetSphere(both_cpu_shape(id), sphere); }
    gpu_shape_set_sphere(id, sphere);
}
B3_API void b3Shape_SetCapsule(b3ShapeId id, const b3Capsule* capsule)
{
    if (both_has_cpu_shape(id)) { cpu_b3Shape_SetCapsule(both_cpu_shape(id), capsule); }
    gpu_shape_set_capsule(id, capsule);
}
B3_API void b3Shape_SetHull(b3ShapeId id, const b3HullData* hull)
{
    // CPU consumes the immutable geometry input before GPU mirror replacement.
    if (both_has_cpu_shape(id)) { cpu_b3Shape_SetHull(both_cpu_shape(id), hull); }
    gpu_shape_set_hull(id, hull);
}
B3_API const b3HullData* b3Shape_GetHull(b3ShapeId id) { return gpu_shape_get_hull(id); }

extern float gpu_b3_joint_get_linear_separation(b3JointId);
B3_API float b3Joint_GetLinearSeparation(b3JointId id) { return gpu_b3_joint_get_linear_separation(id); }

extern float gpu_b3_joint_get_angular_separation(b3JointId);
B3_API float b3Joint_GetAngularSeparation(b3JointId id) { return gpu_b3_joint_get_angular_separation(id); }

extern void gpu_b3_joint_get_constraint_force(b3JointId, float*);
B3_API b3Vec3 b3Joint_GetConstraintForce(b3JointId id) { b3Vec3 v; gpu_b3_joint_get_constraint_force(id, &v.x); return v; }

extern void gpu_b3_joint_get_constraint_torque(b3JointId, float*);
B3_API b3Vec3 b3Joint_GetConstraintTorque(b3JointId id) { b3Vec3 v; gpu_b3_joint_get_constraint_torque(id, &v.x); return v; }
