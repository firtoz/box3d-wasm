// Native viewer metadata and drawing. Engine C wrappers live in shim.c.
#include "box3d/box3d.h"
#include "box3d/collision.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "native_clock.h"
#include "growable_slots.h"

#define GPU_SAMPLES_WORLD_CAP GPU_METADATA_WORLDS

typedef struct WorldVis
{
	b3CreateDebugShapeCallback* create;
	b3DestroyDebugShapeCallback* destroy;
	void* ctx;
} WorldVis;

typedef struct ShapeVis
{
	uint8_t live;
	b3ShapeId id;
	b3ShapeType type;
	b3BodyId body;
	b3Sphere sphere;
	b3Capsule capsule;
	b3HullData* hull;
	b3Mesh mesh;
	const b3CompoundData* compound;
	int is_box;
	float box_h[3];
	float box_c[3];
	void* userShape;
	char name[64];
} ShapeVis;

static WorldVis g_worlds[GPU_SAMPLES_WORLD_CAP];
static GpuSlots g_shapes[GPU_SAMPLES_WORLD_CAP];
static float g_last_draw_list_ms;
static float g_last_pose_prep_ms;
static float g_last_import_ms;
static uint32_t g_last_draw_shape_count;

uint32_t gpu_samples_last_draw_shape_count(void)
{
    return g_last_draw_shape_count;
}

float gpu_samples_last_import_ms(void)
{
	return g_last_import_ms;
}

static uint64_t monotonic_ns(void)
{
	return gpu_monotonic_ns();
}

static float duration_ms_from_ns(uint64_t start_ns, uint64_t end_ns)
{
	return (float)((double)(end_ns - start_ns) * 1e-6);
}

float gpu_samples_duration_ms_from_ns(uint64_t start_ns, uint64_t end_ns)
{
	return duration_ms_from_ns(start_ns, end_ns);
}

float gpu_samples_last_draw_list_ms(void)
{
	return g_last_draw_list_ms;
}

float gpu_samples_last_pose_prep_ms(void)
{
	return g_last_pose_prep_ms;
}

static WorldVis* world_vis(b3WorldId id)
{
	if (id.index1 == 0 || id.index1 >= GPU_SAMPLES_WORLD_CAP)
	{
		return NULL;
	}
	return &g_worlds[id.index1];
}

static ShapeVis* shape_vis(b3ShapeId id, bool create)
{
    if (id.world0 == 0 || id.world0 >= GPU_SAMPLES_WORLD_CAP) return NULL;
    ShapeVis* s = (ShapeVis*)gpu_slots_get(&g_shapes[id.world0], id.index1, sizeof(ShapeVis), create);
    if (!create && s && (!s->live || s->id.generation != id.generation)) return NULL;
    return s;
}

static void destroy_user_shape(b3WorldId world, ShapeVis* s)
{
	if (s == NULL || s->userShape == NULL)
	{
		return;
	}
	WorldVis* w = world_vis(world);
	if (w != NULL && w->destroy != NULL)
	{
		w->destroy(s->userShape, w->ctx);
	}
	s->userShape = NULL;
}

void gpu_samples_on_world_created(b3WorldId world, const b3WorldDef* def)
{
	WorldVis* w = world_vis(world);
	if (w == NULL || def == NULL)
	{
		return;
	}
	w->create = def->createDebugShape;
	w->destroy = def->destroyDebugShape;
	w->ctx = def->userDebugShapeContext;
}

void gpu_samples_on_world_destroyed(b3WorldId world)
{
    WorldVis* w = world_vis(world);
    if (!w) return;
    GpuSlots* slots = &g_shapes[world.index1];
    for (uint32_t i = 1; i < slots->high_water; ++i)
    {
        ShapeVis* s = (ShapeVis*)gpu_slots_get(slots, (int32_t)i, sizeof(ShapeVis), false);
        if (s && s->live)
        {
            destroy_user_shape(world, s);
            if (s->hull) b3DestroyHull(s->hull);
            memset(s, 0, sizeof(*s));
        }
    }
    gpu_slots_release(slots);
    memset(w, 0, sizeof(*w));
}

void gpu_samples_on_shape_created(b3ShapeId shapeId, b3BodyId bodyId, b3ShapeType type, const b3Sphere* sphere,
								  const b3Capsule* capsule, const b3HullData* hull)
{
	ShapeVis* s = shape_vis(shapeId, true);
	if (s == NULL)
	{
		return;
	}
	memset(s, 0, sizeof(*s));
	s->live = 1;
	s->id = shapeId;
	s->type = type;
	s->body = bodyId;
	s->name[0] = 0;
	if (type == b3_sphereShape && sphere != NULL)
	{
		s->sphere = *sphere;
	}
	else if (type == b3_capsuleShape && capsule != NULL)
	{
		s->capsule = *capsule;
	}
	else if (type == b3_hullShape && hull != NULL)
	{
		s->hull = b3CloneHull(hull);
		if (s->hull == NULL)
		{
			s->is_box = 1;
			s->box_h[0] = 0.5f * (hull->aabb.upperBound.x - hull->aabb.lowerBound.x);
			s->box_h[1] = 0.5f * (hull->aabb.upperBound.y - hull->aabb.lowerBound.y);
			s->box_h[2] = 0.5f * (hull->aabb.upperBound.z - hull->aabb.lowerBound.z);
			s->box_c[0] = 0.5f * (hull->aabb.upperBound.x + hull->aabb.lowerBound.x);
			s->box_c[1] = 0.5f * (hull->aabb.upperBound.y + hull->aabb.lowerBound.y);
			s->box_c[2] = 0.5f * (hull->aabb.upperBound.z + hull->aabb.lowerBound.z);
		}
	}
}

void gpu_samples_on_compound_shape_created(b3ShapeId shapeId, b3BodyId bodyId, const b3CompoundData* compound)
{
	gpu_samples_on_shape_created(shapeId, bodyId, b3_compoundShape, NULL, NULL, NULL);
	ShapeVis* s = shape_vis(shapeId, false);
	if (s != NULL && s->live)
	{
		s->compound = compound;
	}
}

void gpu_samples_on_mesh_shape_created(b3ShapeId shapeId, b3BodyId bodyId, const b3MeshData* mesh, b3Vec3 scale)
{
	ShapeVis* s = shape_vis(shapeId, true);
	if (s == NULL)
	{
		return;
	}
	memset(s, 0, sizeof(*s));
	s->live = 1;
	s->id = shapeId;
	s->type = b3_meshShape;
	s->body = bodyId;
	s->mesh = (b3Mesh){mesh, scale};
}

void gpu_samples_on_shape_destroyed(b3ShapeId shapeId)
{
	ShapeVis* s = shape_vis(shapeId, false);
	if (s == NULL || !s->live)
	{
		return;
	}
	b3WorldId world = {shapeId.world0, 1};
	destroy_user_shape(world, s);
	if (s->hull != NULL)
	{
		b3DestroyHull(s->hull);
		s->hull = NULL;
	}
	s->live = 0;
}

void gpu_samples_on_shape_replaced(b3ShapeId id, b3BodyId body, b3ShapeType type,
    const b3Sphere* sphere, const b3Capsule* capsule, const b3HullData* hull)
{
    ShapeVis* s = shape_vis(id, false);
    if (!s) { return; }
    char name[sizeof(s->name)];
    memcpy(name, s->name, sizeof(name));
    gpu_samples_on_shape_destroyed(id);
    gpu_samples_on_shape_created(id, body, type, sphere, capsule, hull);
    memcpy(s->name, name, sizeof(name));
}

typedef struct GpuDrawItem
{
	float pos[3];
	uint32_t kind;
	float rot[4];
	float half[3];
	float axis[3];
	uint32_t flags;
} GpuDrawItem;

extern int gpu_b3_copy_draw_items(b3WorldId world, GpuDrawItem* out, int cap);
extern void gpu_b3_world_bounds(b3WorldId world, float* out6);
extern void gpu_b3_world_counts(b3WorldId world, int* bodies, int* shapes, int* joints);
extern int gpu_b3_live_world_count(void);
extern void gpu_b3_world_enable_sleeping(b3WorldId world, bool enable);
extern bool gpu_b3_world_is_sleeping_enabled(b3WorldId world);
extern b3Vec3 gpu_b3_world_get_gravity(b3WorldId world);
extern b3MassData gpu_b3_body_get_mass_data(b3BodyId body);
extern void gpu_b3_body_set_mass_data(b3BodyId body, b3MassData data);
extern bool gpu_b3_body_is_valid(b3BodyId body);
extern bool gpu_b3_shape_is_valid(b3ShapeId shape);
extern bool gpu_b3_joint_is_valid(b3JointId joint);
extern void gpu_b3_destroy_joint(b3JointId joint, bool wake_attached);
extern void gpu_b3_destroy_body(b3BodyId body);
extern b3BodyId gpu_b3_shape_get_body(b3ShapeId shape);
extern int gpu_b3_shape_kind(b3ShapeId shape);
extern bool gpu_b3_body_is_static(b3BodyId body);
extern float gpu_b3_body_inv_mass(b3BodyId body);
extern void gpu_b3_body_set_transform(b3BodyId body, float px, float py, float pz, float rx, float ry, float rz,
									  float rw);
extern void gpu_b3_body_get_position(b3BodyId body, float* out);
extern void gpu_b3_body_get_rotation(b3BodyId body, float* out);
extern void gpu_b3_body_get_transform(b3BodyId body, float* pos, float* rot);
extern void gpu_b3_world_prepare_pose_snapshot(b3WorldId world);
extern void gpu_b3_body_set_awake(b3BodyId body, bool awake);

static void quat_rotate(const float q[4], const float v[3], float out[3])
{
	// Share Box3D's rounding behavior with the Rust API and GPU integrator.
	const b3Quat rotation = {{q[0], q[1], q[2]}, q[3]};
	const b3Vec3 vector = {v[0], v[1], v[2]};
	const b3Vec3 result = b3RotateVector(rotation, vector);
	out[0] = result.x;
	out[1] = result.y;
	out[2] = result.z;
}

B3_API int b3GetWorldCount(void)
{
	return gpu_b3_live_world_count();
}

B3_API int b3GetMaxWorldCount(void)
{
	return 32;
}

static void* ensure_user_shape(b3WorldId worldId, b3ShapeId shapeId, ShapeVis* s)
{
	if (s->userShape != NULL)
	{
		return s->userShape;
	}
	WorldVis* w = world_vis(worldId);
	if (w == NULL || w->create == NULL)
	{
		return NULL;
	}
	b3DebugShape debugShape = {0};
	debugShape.shapeId = shapeId;
	debugShape.type = s->type;
	b3BoxHull box;
	if (s->type == b3_sphereShape)
	{
		debugShape.sphere = &s->sphere;
	}
	else if (s->type == b3_capsuleShape)
	{
		debugShape.capsule = &s->capsule;
	}
	else if (s->type == b3_hullShape)
	{
		if (s->hull != NULL)
		{
			debugShape.hull = s->hull;
		}
		else if (s->is_box)
		{
			box = b3MakeOffsetBoxHull(s->box_h[0], s->box_h[1], s->box_h[2],
									  (b3Vec3){s->box_c[0], s->box_c[1], s->box_c[2]});
			debugShape.hull = &box.base;
		}
		else
		{
			return NULL;
		}
	}
	else if (s->type == b3_meshShape && s->mesh.data != NULL)
	{
		debugShape.mesh = &s->mesh;
	}
	else if (s->type == b3_compoundShape && s->compound != NULL)
	{
		debugShape.compound = s->compound;
	}
	else
	{
		return NULL;
	}
	s->userShape = w->create(&debugShape, w->ctx);
	return s->userShape;
}

void gpu_samples_shape_set_name(b3ShapeId shapeId, const char* name)
{
	ShapeVis* s = shape_vis(shapeId, false);
	if (s == NULL || !s->live)
	{
		return;
	}
	if (name == NULL)
	{
		s->name[0] = 0;
		return;
	}
	strncpy(s->name, name, sizeof(s->name) - 1);
	s->name[sizeof(s->name) - 1] = 0;
}

const char* gpu_samples_shape_get_name(b3ShapeId shapeId)
{
	ShapeVis* s = shape_vis(shapeId, false);
	if (s == NULL || !s->live)
	{
		return "";
	}
	return s->name;
}

#ifndef BOTH_SAMPLES
B3_API void b3Shape_SetName(b3ShapeId shapeId, const char* name)
{
	gpu_samples_shape_set_name(shapeId, name);
}

B3_API const char* b3Shape_GetName(b3ShapeId shapeId)
{
	return gpu_samples_shape_get_name(shapeId);
}
#endif

typedef struct GpuPoseExport
{
	uint32_t mode;
	int32_t fd;
	uint64_t size;
	uint32_t stride;
	uint32_t count;
	uint32_t pad;
} GpuPoseExport;

extern GpuPoseExport gpu_b3_world_pose_export(b3WorldId worldId);
extern bool gpu_b3_world_pose_export_uuid(b3WorldId worldId, uint8_t* uuid);
extern void gpu_gl_import_pose_fd(int fd, uint64_t size, const uint8_t* uuid);
extern bool gpu_gl_poses_imported(void);

void gpu_samples_world_draw(b3WorldId worldId, b3DebugDraw* draw, uint64_t maskBits)
{
	(void)maskBits;
	g_last_draw_shape_count = 0;
	if (draw == NULL || draw->drawShapes == false || draw->DrawShapeFcn == NULL || world_vis(worldId) == NULL)
	{
		return;
	}
	uint64_t t_import = monotonic_ns();
	if (!gpu_gl_poses_imported())
	{
		GpuPoseExport exp = gpu_b3_world_pose_export(worldId);
		uint8_t uuid[16];
		if (exp.mode == 1 && exp.fd >= 0 && gpu_b3_world_pose_export_uuid(worldId, uuid))
		{
			gpu_gl_import_pose_fd(exp.fd, exp.size, uuid);
		}
	}
	g_last_import_ms = duration_ms_from_ns(t_import, monotonic_ns());
	uint64_t t_prep = monotonic_ns();
	gpu_b3_world_prepare_pose_snapshot(worldId);
	g_last_pose_prep_ms = duration_ms_from_ns(t_prep, monotonic_ns());
	uint64_t t_draw = monotonic_ns();
	for (uint32_t i = 1; i < g_shapes[worldId.index1].high_water; ++i)
	{
		ShapeVis* s = (ShapeVis*)gpu_slots_get(&g_shapes[worldId.index1], (int32_t)i, sizeof(ShapeVis), false);
		if (!s || !s->live || s->body.world0 != worldId.index1)
		{
			continue;
		}
		b3ShapeId shapeId = s->id;
		void* userShape = ensure_user_shape(worldId, shapeId, s);
		if (userShape == NULL)
		{
			continue;
		}
		float p[3] = {0};
		float q[4] = {0, 0, 0, 1};
		gpu_b3_body_get_transform(s->body, p, q);
		b3WorldTransform xf;
		xf.p.x = p[0];
		xf.p.y = p[1];
		xf.p.z = p[2];
		xf.q.v.x = q[0];
		xf.q.v.y = q[1];
		xf.q.v.z = q[2];
		xf.q.s = q[3];
		b3HexColor rgb = b3_colorTan;
		b3DebugMaterial mat = b3_debugMaterialSoft;
		const bool is_static = gpu_b3_body_is_static(s->body);
		if (is_static)
		{
			rgb = b3_colorDarkGray;
			mat = b3_debugMaterialMatte;
		}
#ifdef BOTH_SAMPLES
		/* Cool palette distinguishes the GPU ghost from Box3D's warm palette. */
		rgb = is_static ? b3_colorDarkSlateBlue : b3_colorCornflowerBlue;
#endif
		draw->DrawShapeFcn(userShape, xf, (b3HexColor)b3MakeDebugColor(rgb, mat), draw->context);
		++g_last_draw_shape_count;
	}
	g_last_draw_list_ms = duration_ms_from_ns(t_draw, monotonic_ns());
}

#ifndef BOTH_SAMPLES
B3_API void b3World_Draw(b3WorldId worldId, b3DebugDraw* draw, uint64_t maskBits)
{
	gpu_samples_world_draw(worldId, draw, maskBits);
}
#endif

b3AABB gpu_samples_world_bounds(b3WorldId worldId)
{
	float b[6];
	gpu_b3_world_bounds(worldId, b);
	b3AABB aabb;
	aabb.lowerBound.x = b[0];
	aabb.lowerBound.y = b[1];
	aabb.lowerBound.z = b[2];
	aabb.upperBound.x = b[3];
	aabb.upperBound.y = b[4];
	aabb.upperBound.z = b[5];
	return aabb;
}

#ifndef BOTH_SAMPLES
B3_API b3AABB b3World_GetBounds(b3WorldId worldId)
{
	return gpu_samples_world_bounds(worldId);
}
#endif

B3_API b3Profile b3World_GetProfile(b3WorldId worldId)
{
	(void)worldId;
	return (b3Profile){0};
}

B3_API b3Counters b3World_GetCounters(b3WorldId worldId)
{
	b3Counters c = {0};
	gpu_b3_world_counts(worldId, &c.bodyCount, &c.shapeCount, &c.jointCount);
	return c;
}

B3_API b3Capacity b3World_GetMaxCapacity(b3WorldId worldId)
{
	(void)worldId;
	return (b3Capacity){0};
}

B3_API void b3World_EnableSleeping(b3WorldId worldId, bool flag)
{
	gpu_b3_world_enable_sleeping(worldId, flag);
}

B3_API bool b3World_IsSleepingEnabled(b3WorldId worldId)
{
	return gpu_b3_world_is_sleeping_enabled(worldId);
}




#ifndef BOTH_SAMPLES
B3_API b3Vec3 b3World_GetGravity(b3WorldId worldId)
{
	return gpu_b3_world_get_gravity(worldId);
}
#endif

B3_API void b3World_DumpMemoryStats(b3WorldId worldId)
{
	(void)worldId;
}

B3_API bool b3Body_IsValid(b3BodyId id)
{
	return gpu_b3_body_is_valid(id);
}

B3_API b3BodyType b3Body_GetType(b3BodyId bodyId)
{
	return gpu_b3_body_is_static(bodyId) ? b3_staticBody : b3_dynamicBody;
}

b3WorldTransform gpu_samples_body_transform(b3BodyId bodyId)
{
	float p[3] = {0};
	float q[4] = {0, 0, 0, 1};
	gpu_b3_body_get_transform(bodyId, p, q);
	b3WorldTransform xf;
	xf.p.x = p[0];
	xf.p.y = p[1];
	xf.p.z = p[2];
	xf.q.v.x = q[0];
	xf.q.v.y = q[1];
	xf.q.v.z = q[2];
	xf.q.s = q[3];
	return xf;
}

#ifndef BOTH_SAMPLES
B3_API b3WorldTransform b3Body_GetTransform(b3BodyId bodyId)
{
	return gpu_samples_body_transform(bodyId);
}
#endif

#ifndef BOTH_SAMPLES
B3_API b3Vec3 b3Body_GetLocalPoint(b3BodyId bodyId, b3Pos worldPoint)
{
	float p[3] = {0};
	float q[4] = {0, 0, 0, 1};
	gpu_b3_body_get_position(bodyId, p);
	gpu_b3_body_get_rotation(bodyId, q);
	float d[3] = {worldPoint.x - p[0], worldPoint.y - p[1], worldPoint.z - p[2]};
	float inv[4] = {-q[0], -q[1], -q[2], q[3]};
	float local[3];
	quat_rotate(inv, d, local);
	b3Vec3 out = {local[0], local[1], local[2]};
	return out;
}

B3_API b3MassData b3Body_GetMassData(b3BodyId bodyId)
{
	return gpu_b3_body_get_mass_data(bodyId);
}

B3_API void b3Body_SetMassData(b3BodyId bodyId, b3MassData data)
{
	gpu_b3_body_set_mass_data(bodyId, data);
}

B3_API void b3Body_SetAwake(b3BodyId bodyId, bool awake)
{
	gpu_b3_body_set_awake(bodyId, awake);
}
#endif

void gpu_samples_body_set_transform(b3BodyId bodyId, b3WorldTransform target)
{
	gpu_b3_body_set_transform(bodyId, target.p.x, target.p.y, target.p.z, target.q.v.x, target.q.v.y, target.q.v.z,
							  target.q.s);
}



void gpu_samples_destroy_body(b3BodyId bodyId)
{
    if (bodyId.world0 > 0 && bodyId.world0 < GPU_SAMPLES_WORLD_CAP)
    {
        GpuSlots* slots = &g_shapes[bodyId.world0];
        for (uint32_t i = 1; i < slots->high_water; ++i)
        {
            ShapeVis* s = (ShapeVis*)gpu_slots_get(slots, (int32_t)i, sizeof(ShapeVis), false);
            if (s && s->live && s->body.index1 == bodyId.index1 && s->body.generation == bodyId.generation)
                gpu_samples_on_shape_destroyed(s->id);
        }
    }
    gpu_b3_destroy_body(bodyId);
}


B3_API bool b3Shape_IsValid(b3ShapeId id)
{
	return gpu_b3_shape_is_valid(id);
}

B3_API b3ShapeType b3Shape_GetType(b3ShapeId shapeId)
{
	int k = gpu_b3_shape_kind(shapeId);
	if (k == 0)
	{
		return b3_sphereShape;
	}
	if (k == 2)
	{
		return b3_capsuleShape;
	}
	if (k == 4)
	{
		return b3_meshShape;
	}
	if (k == 5)
	{
		return b3_heightShape;
	}
	if (k == 6)
	{
		return b3_compoundShape;
	}
	return b3_hullShape;
}

B3_API b3BodyId b3Shape_GetBody(b3ShapeId shapeId)
{
	return gpu_b3_shape_get_body(shapeId);
}

B3_API bool b3Joint_IsValid(b3JointId id)
{
	return gpu_b3_joint_is_valid(id);
}

#ifndef BOTH_SAMPLES
B3_API void b3DestroyJoint(b3JointId jointId, bool wakeAttached)
{
	gpu_b3_destroy_joint(jointId, wakeAttached);
}
#endif

static char g_dummy_recording;

B3_API b3Recording* b3CreateRecording(int byteCapacity)
{
	(void)byteCapacity;
	return (b3Recording*)&g_dummy_recording;
}

B3_API void b3DestroyRecording(b3Recording* recording)
{
	(void)recording;
}

B3_API void b3World_StartRecording(b3WorldId worldId, b3Recording* recording)
{
	(void)worldId;
	(void)recording;
}

B3_API void b3World_StopRecording(b3WorldId worldId)
{
	(void)worldId;
}

B3_API bool b3SaveRecordingToFile(const b3Recording* recording, const char* path)
{
	(void)recording;
	(void)path;
	fprintf(stderr, "GPU recording is not implemented; no recording file was saved.\n");
	return false;
}
