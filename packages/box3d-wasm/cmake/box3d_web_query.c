#include "box3d_web_shared.h"

#include <float.h>
#include <stdint.h>
#include <stddef.h>

enum
{
	b3w_castAny = 0,
	b3w_castClosest = 1,
	b3w_castMultiple = 2,
	b3w_castSorted = 3,
	b3w_castRay = 0,
	b3w_castSphere = 1,
	b3w_castCapsule = 2,
	b3w_castBox = 3,
	b3w_castMaxHits = 3,
};

typedef struct b3wCastContext
{
	b3Pos points[3];
	b3Vec3 normals[3];
	float fractions[3];
	uint64_t materialIds[3];
	int triangleIndices[3];
	int childIndices[3];
	int count;
	int initialOverlap;
} b3wCastContext;

static float b3wCastClosestCallback(b3ShapeId shapeId, b3Pos point, b3Vec3 normal, float fraction, uint64_t materialId,
	int triangleIndex, int childIndex, void* context)
{
	b3wCastContext* rayContext = (b3wCastContext*)context;
	if (rayContext->initialOverlap == 0 && fraction == 0.0f) return -1.0f;
	if ((intptr_t)b3Shape_GetUserData(shapeId) == 1) return -1.0f;
	rayContext->points[0] = point;
	rayContext->normals[0] = normal;
	rayContext->fractions[0] = fraction;
	rayContext->materialIds[0] = materialId;
	rayContext->triangleIndices[0] = triangleIndex;
	rayContext->childIndices[0] = childIndex;
	rayContext->count = 1;
	return fraction;
}

static float b3wCastAnyCallback(b3ShapeId shapeId, b3Pos point, b3Vec3 normal, float fraction, uint64_t materialId,
	int triangleIndex, int childIndex, void* context)
{
	b3wCastContext* rayContext = (b3wCastContext*)context;
	if (rayContext->initialOverlap == 0 && fraction == 0.0f) return -1.0f;
	if ((intptr_t)b3Shape_GetUserData(shapeId) == 1) return -1.0f;
	rayContext->points[0] = point;
	rayContext->normals[0] = normal;
	rayContext->fractions[0] = fraction;
	rayContext->materialIds[0] = materialId;
	rayContext->triangleIndices[0] = triangleIndex;
	rayContext->childIndices[0] = childIndex;
	rayContext->count = 1;
	return 0.0f;
}

static float b3wCastMultipleCallback(b3ShapeId shapeId, b3Pos point, b3Vec3 normal, float fraction, uint64_t materialId,
	int triangleIndex, int childIndex, void* context)
{
	b3wCastContext* rayContext = (b3wCastContext*)context;
	if (rayContext->initialOverlap == 0 && fraction == 0.0f) return -1.0f;
	if ((intptr_t)b3Shape_GetUserData(shapeId) == 1) return -1.0f;
	int count = rayContext->count;
	if (count >= b3w_castMaxHits) return 0.0f;
	rayContext->points[count] = point;
	rayContext->normals[count] = normal;
	rayContext->fractions[count] = fraction;
	rayContext->materialIds[count] = materialId;
	rayContext->triangleIndices[count] = triangleIndex;
	rayContext->childIndices[count] = childIndex;
	rayContext->count = count + 1;
	if (rayContext->count == b3w_castMaxHits) return 0.0f;
	return 1.0f;
}

static float b3wCastSortedCallback(b3ShapeId shapeId, b3Pos point, b3Vec3 normal, float fraction, uint64_t materialId,
	int triangleIndex, int childIndex, void* context)
{
	b3wCastContext* rayContext = (b3wCastContext*)context;
	if (rayContext->initialOverlap == 0 && fraction == 0.0f) return -1.0f;
	if ((intptr_t)b3Shape_GetUserData(shapeId) == 1) return -1.0f;
	int count = rayContext->count;
	int index = 3;
	while (fraction < rayContext->fractions[index - 1])
	{
		index -= 1;
		if (index == 0) break;
	}
	if (index == 3)
	{
		return rayContext->fractions[2];
	}
	for (int j = 2; j > index; --j)
	{
		rayContext->points[j] = rayContext->points[j - 1];
		rayContext->normals[j] = rayContext->normals[j - 1];
		rayContext->fractions[j] = rayContext->fractions[j - 1];
		rayContext->materialIds[j] = rayContext->materialIds[j - 1];
		rayContext->triangleIndices[j] = rayContext->triangleIndices[j - 1];
		rayContext->childIndices[j] = rayContext->childIndices[j - 1];
	}
	rayContext->points[index] = point;
	rayContext->normals[index] = normal;
	rayContext->fractions[index] = fraction;
	rayContext->materialIds[index] = materialId;
	rayContext->triangleIndices[index] = triangleIndex;
	rayContext->childIndices[index] = childIndex;
	rayContext->count = count < 3 ? count + 1 : 3;
	if (rayContext->count == 3) return rayContext->fractions[2];
	return 1.0f;
}

B3W_EXPORT int b3wWorldCast(int worldHandle, int mode, int initialOverlap, float ox, float oy, float oz, float tx, float ty,
	float tz, int proxyType, float radius, float* outHits)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL || outHits == NULL) return 0;
	b3CastResultFcn* functions[] = { b3wCastAnyCallback, b3wCastClosestCallback, b3wCastMultipleCallback, b3wCastSortedCallback };
	if (mode < 0 || mode > 3) mode = b3w_castClosest;
	b3wCastContext ctx = { 0 };
	ctx.initialOverlap = initialOverlap;
	ctx.fractions[0] = FLT_MAX;
	ctx.fractions[1] = FLT_MAX;
	ctx.fractions[2] = FLT_MAX;
	b3Sphere sphere = { b3Vec3_zero, radius };
	b3Capsule capsule = { b3Vec3_zero, { 0.0f, 1.0f, 0.0f }, radius };
	b3BoxHull box = { 0 };
	b3Vec3 pointBuffer[2];
	b3ShapeProxy proxy = { 0 };
	switch (proxyType)
	{
	case b3w_castSphere:
		proxy.count = 1;
		proxy.radius = radius;
		proxy.points = &sphere.center;
		break;
	case b3w_castCapsule:
		proxy.count = 2;
		proxy.radius = radius;
		pointBuffer[0] = capsule.center1;
		pointBuffer[1] = capsule.center2;
		proxy.points = pointBuffer;
		break;
	case b3w_castBox:
	{
		b3Vec3 extent = { radius, 0.5f * radius, 0.25f * radius };
		b3Transform boxXf = { b3Vec3_zero, b3Quat_identity };
		box = b3MakeTransformedBoxHull(extent.x, extent.y, extent.z, boxXf);
		proxy.points = box.boxPoints;
		proxy.count = box.base.vertexCount;
		break;
	}
	default:
		proxy.count = 0;
		break;
	}
	b3QueryFilter filter = b3DefaultQueryFilter();
	b3Pos origin = { ox, oy, oz };
	b3Vec3 translation = { tx, ty, tz };
	if (proxyType == b3w_castRay)
	{
		b3World_CastRay(slot->worldId, origin, translation, filter, functions[mode], &ctx);
	}
	else
	{
		b3World_CastShape(slot->worldId, origin, &proxy, translation, filter, functions[mode], &ctx);
	}
	for (int i = 0; i < ctx.count; ++i)
	{
		int o = i * 10;
		outHits[o + 0] = ctx.fractions[i];
		outHits[o + 1] = ctx.points[i].x;
		outHits[o + 2] = ctx.points[i].y;
		outHits[o + 3] = ctx.points[i].z;
		outHits[o + 4] = ctx.normals[i].x;
		outHits[o + 5] = ctx.normals[i].y;
		outHits[o + 6] = ctx.normals[i].z;
		outHits[o + 7] = (float)ctx.materialIds[i];
		outHits[o + 8] = (float)ctx.triangleIndices[i];
		outHits[o + 9] = (float)ctx.childIndices[i];
	}
	return ctx.count;
}

B3W_EXPORT void b3wBodyComputeAABB(uint64_t bodyPacked, float* outAabb)
{
	if (outAabb == NULL) return;
	b3BodyId bodyId = b3LoadBodyId(bodyPacked);
	if (!b3Body_IsValid(bodyId))
	{
		outAabb[0] = outAabb[1] = outAabb[2] = outAabb[3] = outAabb[4] = outAabb[5] = 0.0f;
		return;
	}
	b3AABB aabb = b3Body_ComputeAABB(bodyId);
	outAabb[0] = aabb.lowerBound.x;
	outAabb[1] = aabb.lowerBound.y;
	outAabb[2] = aabb.lowerBound.z;
	outAabb[3] = aabb.upperBound.x;
	outAabb[4] = aabb.upperBound.y;
	outAabb[5] = aabb.upperBound.z;
}
