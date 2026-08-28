#include "box3d_web_shared.h"

#include "box3d/collision.h"

#include <float.h>

#define B3W_MOVER_PLANE_CAP 64

typedef struct b3wCollideMoverContext
{
	b3PlaneResult* planes;
	int count;
	int capacity;
} b3wCollideMoverContext;

static bool b3wCollideMoverCallback(b3ShapeId shapeId, const b3PlaneResult* results, int planeCount, void* context)
{
	(void)shapeId;
	b3wCollideMoverContext* ctx = (b3wCollideMoverContext*)context;
	for (int i = 0; i < planeCount && ctx->count < ctx->capacity; ++i)
	{
		ctx->planes[ctx->count] = results[i];
		ctx->count += 1;
	}
	return ctx->count < ctx->capacity;
}

B3W_EXPORT int b3wCollideMover(int worldHandle, float ox, float oy, float oz, float c1x, float c1y, float c1z, float c2x, float c2y,
	float c2z, float radius, int capacity, float* outPlanes)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL || outPlanes == NULL) return 0;
	int cap = capacity;
	if (cap > B3W_MOVER_PLANE_CAP) cap = B3W_MOVER_PLANE_CAP;
	if (cap < 0) cap = 0;
	b3PlaneResult stack[B3W_MOVER_PLANE_CAP];
	b3wCollideMoverContext ctx = { stack, 0, cap };
	b3Capsule capsule = { { c1x, c1y, c1z }, { c2x, c2y, c2z }, radius };
	b3QueryFilter filter = b3DefaultQueryFilter();
	b3World_CollideMover(slot->worldId, (b3Pos){ ox, oy, oz }, &capsule, filter, b3wCollideMoverCallback, &ctx);
	for (int i = 0; i < ctx.count; ++i)
	{
		int o = i * 7;
		outPlanes[o + 0] = ctx.planes[i].plane.normal.x;
		outPlanes[o + 1] = ctx.planes[i].plane.normal.y;
		outPlanes[o + 2] = ctx.planes[i].plane.normal.z;
		outPlanes[o + 3] = ctx.planes[i].plane.offset;
		outPlanes[o + 4] = ctx.planes[i].point.x;
		outPlanes[o + 5] = ctx.planes[i].point.y;
		outPlanes[o + 6] = ctx.planes[i].point.z;
	}
	return ctx.count;
}

B3W_EXPORT void b3wSolvePlanes(float tx, float ty, float tz, const float* inPlanes, int count, float* outDelta)
{
	if (outDelta == NULL) return;
	int n = count;
	if (n > B3W_MOVER_PLANE_CAP) n = B3W_MOVER_PLANE_CAP;
	if (n < 0) n = 0;
	b3CollisionPlane planes[B3W_MOVER_PLANE_CAP];
	for (int i = 0; i < n; ++i)
	{
		int o = i * 6;
		float pushLimit = inPlanes != NULL ? inPlanes[o + 4] : FLT_MAX;
		planes[i].plane.normal = (b3Vec3){ inPlanes[o + 0], inPlanes[o + 1], inPlanes[o + 2] };
		planes[i].plane.offset = inPlanes[o + 3];
		planes[i].pushLimit = pushLimit;
		planes[i].push = 0.0f;
		planes[i].clipVelocity = inPlanes[o + 5] != 0.0f;
	}
	b3Vec3 target = { tx, ty, tz };
	b3PlaneSolverResult result = b3SolvePlanes(target, planes, n);
	outDelta[0] = result.delta.x;
	outDelta[1] = result.delta.y;
	outDelta[2] = result.delta.z;
	outDelta[3] = (float)result.iterationCount;
}
