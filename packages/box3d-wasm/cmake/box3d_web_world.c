#include "box3d_web_shared.h"

#include "box3d/constants.h"

#include <stdint.h>

B3W_EXPORT int b3wCreateWorld(float gravityX, float gravityY, float gravityZ, int workerCount, int staticShapeCount,
							  int dynamicShapeCount, int staticBodyCount, int dynamicBodyCount, int contactCount)
{
	b3WorldDef def = b3DefaultWorldDef();
	def.gravity = (b3Vec3){ gravityX, gravityY, gravityZ };
	def.workerCount = workerCount > 0 ? workerCount : 1;
	if (staticShapeCount > 0 || dynamicShapeCount > 0 || staticBodyCount > 0 || dynamicBodyCount > 0 || contactCount > 0)
	{
		def.capacity.staticShapeCount = staticShapeCount;
		def.capacity.dynamicShapeCount = dynamicShapeCount;
		def.capacity.staticBodyCount = staticBodyCount;
		def.capacity.dynamicBodyCount = dynamicBodyCount;
		def.capacity.contactCount = contactCount;
	}
	b3WorldId worldId = b3CreateWorld(&def);
	return b3wAllocWorldSlot(worldId);
}

B3W_EXPORT void b3wDestroyWorld(int worldHandle)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	// Free bridge slots owned by this world (meshes/humans/heightfields) before destroying the engine world.
	b3wClearWorldSlots(worldHandle);
	b3DestroyWorld(slot->worldId);
	b3wFreeWorldSlot(worldHandle);
}

B3W_EXPORT void b3wStep(int worldHandle, float timeStep, int subStepCount)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3World_Step(slot->worldId, timeStep, subStepCount);
}

B3W_EXPORT void b3wGetWorldCounters(int worldHandle, int* outCounters)
{
	if (outCounters == NULL) return;
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3Counters counters = b3World_GetCounters(slot->worldId);
	outCounters[0] = counters.bodyCount;
	outCounters[1] = counters.shapeCount;
	outCounters[2] = counters.contactCount;
	outCounters[3] = counters.jointCount;
	outCounters[4] = counters.islandCount;
	outCounters[5] = counters.staticTreeHeight;
	outCounters[6] = counters.treeHeight;
}

B3W_EXPORT void b3wGetWorldProfile(int worldHandle, float* outProfile)
{
	if (outProfile == NULL) return;
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3Profile profile = b3World_GetProfile(slot->worldId);
	outProfile[0] = profile.step;
	outProfile[1] = profile.pairs;
	outProfile[2] = profile.collide;
	outProfile[3] = profile.solve;
	outProfile[4] = profile.solverSetup;
	outProfile[5] = profile.constraints;
	outProfile[6] = profile.prepareConstraints;
	outProfile[7] = profile.integrateVelocities;
	outProfile[8] = profile.warmStart;
	outProfile[9] = profile.solveImpulses;
	outProfile[10] = profile.integratePositions;
	outProfile[11] = profile.relaxImpulses;
	outProfile[12] = profile.applyRestitution;
	outProfile[13] = profile.storeImpulses;
	outProfile[14] = profile.splitIslands;
	outProfile[15] = profile.transforms;
	outProfile[16] = profile.sensorHits;
	outProfile[17] = profile.jointEvents;
	outProfile[18] = profile.hitEvents;
	outProfile[19] = profile.refit;
	outProfile[20] = profile.bullets;
	outProfile[21] = profile.sleepIslands;
	outProfile[22] = profile.sensors;
}

B3W_EXPORT int b3wGetWorldWorkerCount(int worldHandle)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return -1;
	return b3World_GetWorkerCount(slot->worldId);
}

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#include <emscripten/threading.h>
B3W_EXPORT int b3wCheckThreadingSupport(void)
{
	// Returns bitmask: bit0=SharedArrayBuffer available, bit1=pthread_create works
	int result = 0;
	if (emscripten_has_threading_support()) result |= 1;
	return result;
}

// PThread lives in the Emscripten glue closure, not on the exported Module object.
EM_JS(void, b3w_js_terminate_pthreads, (), {
	if (typeof PThread !== "undefined" && PThread.terminateAllThreads) {
		PThread.terminateAllThreads();
	}
});

B3W_EXPORT void b3wTerminatePthreads(void)
{
	b3w_js_terminate_pthreads();
}
#else
B3W_EXPORT void b3wTerminatePthreads(void)
{
}
#endif

B3W_EXPORT void b3wEnableSleeping(int worldHandle, int flag)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3World_EnableSleeping(slot->worldId, flag);
}

B3W_EXPORT void b3wEnableContinuous(int worldHandle, int flag)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3World_EnableContinuous(slot->worldId, flag);
}

B3W_EXPORT void b3wEnableWarmStarting(int worldHandle, int flag)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3World_EnableWarmStarting(slot->worldId, flag);
}

B3W_EXPORT void b3wSetProfileLevel(int worldHandle, int level)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3ProfileLevel profileLevel = b3_profileFull;
	if (level <= 0) profileLevel = b3_profileOff;
	else if (level == 1) profileLevel = b3_profileCoarse;
	else profileLevel = b3_profileFull;
	b3World_SetProfileLevel(slot->worldId, profileLevel);
}

B3W_EXPORT int b3wGetProfileLevel(int worldHandle)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return (int)b3_profileFull;
	return (int)b3World_GetProfileLevel(slot->worldId);
}

B3W_EXPORT void b3wSetContactTuning(int worldHandle, float hertz, float dampingRatio, float contactSpeed)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3World_SetContactTuning(slot->worldId, hertz, dampingRatio, contactSpeed);
}

B3W_EXPORT void b3wSetContactRecycleDistance(int worldHandle, float distance)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3World_SetContactRecycleDistance(slot->worldId, distance);
}

B3W_EXPORT void b3wSetWorkerCount(int worldHandle, int count)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3World_SetWorkerCount(slot->worldId, count);
}

B3W_EXPORT int b3wGetWorldAwakeBodyCount(int worldHandle)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return 0;
	return b3World_GetAwakeBodyCount(slot->worldId);
}

B3W_EXPORT void b3wWorldExplode(int worldHandle, float px, float py, float pz, float radius, float falloff, float impulsePerArea, uint64_t maskBits)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3ExplosionDef def = b3DefaultExplosionDef();
	def.position = (b3Pos){ px, py, pz };
	def.radius = radius;
	def.falloff = falloff;
	def.impulsePerArea = impulsePerArea;
	def.maskBits = maskBits;
	b3World_Explode(slot->worldId, &def);
}

B3W_EXPORT void b3wRayCastClosest(int worldHandle, float originX, float originY, float originZ, float translationX, float translationY, float translationZ, int categoryBits, int maskBits, uint64_t* outShapePacked, float* outPoint, float* outNormal, float* outFraction)
{
	if (outShapePacked != NULL) *outShapePacked = 0;
	if (outPoint != NULL)
	{
		outPoint[0] = 0.0f;
		outPoint[1] = 0.0f;
		outPoint[2] = 0.0f;
	}
	if (outNormal != NULL)
	{
		outNormal[0] = 0.0f;
		outNormal[1] = 0.0f;
		outNormal[2] = 0.0f;
	}
	if (outFraction != NULL) *outFraction = 1.0f;
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3QueryFilter filter = b3DefaultQueryFilter();
	filter.categoryBits = (uint64_t)categoryBits;
	filter.maskBits = (uint64_t)maskBits;
	b3RayResult result = b3World_CastRayClosest(slot->worldId, (b3Pos){ originX, originY, originZ }, (b3Vec3){ translationX, translationY, translationZ }, filter);
	if (b3Shape_IsValid(result.shapeId) == false) return;
	if (outShapePacked != NULL)
	{
		*outShapePacked = b3StoreShapeId(result.shapeId);
	}
	if (outPoint != NULL)
	{
		outPoint[0] = result.point.x;
		outPoint[1] = result.point.y;
		outPoint[2] = result.point.z;
	}
	if (outNormal != NULL)
	{
		outNormal[0] = result.normal.x;
		outNormal[1] = result.normal.y;
		outNormal[2] = result.normal.z;
	}
	if (outFraction != NULL) *outFraction = result.fraction;
}

B3W_EXPORT float b3wGetStallThreshold(void)
{
	return b3GetStallThreshold();
}

B3W_EXPORT void b3wSetStallThreshold(float seconds)
{
	b3SetStallThreshold(seconds);
}

typedef struct b3wOverlapCountContext
{
	int count;
} b3wOverlapCountContext;

static bool b3wOverlapCountCallback(b3ShapeId shapeId, void* context)
{
	(void)shapeId;
	b3wOverlapCountContext* ctx = (b3wOverlapCountContext*)context;
	ctx->count += 1;
	return true;
}

B3W_EXPORT int b3wOverlapAABB(int worldHandle, float minX, float minY, float minZ, float maxX, float maxY, float maxZ,
	int categoryBits, int maskBits)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return -1;
	b3QueryFilter filter = b3DefaultQueryFilter();
	filter.categoryBits = (uint64_t)categoryBits;
	filter.maskBits = (uint64_t)maskBits;
	b3AABB aabb;
	aabb.lowerBound = (b3Vec3){ minX, minY, minZ };
	aabb.upperBound = (b3Vec3){ maxX, maxY, maxZ };
	b3wOverlapCountContext ctx = { 0 };
	b3World_OverlapAABB(slot->worldId, aabb, filter, b3wOverlapCountCallback, &ctx);
	return ctx.count;
}

typedef struct b3wCastShapeClosestContext
{
	float fraction;
	int hit;
} b3wCastShapeClosestContext;

static float b3wCastShapeClosestCallback(b3ShapeId shapeId, b3Pos point, b3Vec3 normal, float fraction, uint64_t userMaterialId,
	int triangleIndex, int childIndex, void* context)
{
	(void)shapeId;
	(void)point;
	(void)normal;
	(void)userMaterialId;
	(void)triangleIndex;
	(void)childIndex;
	b3wCastShapeClosestContext* ctx = (b3wCastShapeClosestContext*)context;
	ctx->hit = 1;
	ctx->fraction = fraction;
	return fraction;
}

B3W_EXPORT float b3wCastShapeSphere(int worldHandle, float originX, float originY, float originZ, float translationX,
	float translationY, float translationZ, float radius, int categoryBits, int maskBits)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return 1.0f;
	b3QueryFilter filter = b3DefaultQueryFilter();
	filter.categoryBits = (uint64_t)categoryBits;
	filter.maskBits = (uint64_t)maskBits;
	b3Vec3 proxyPoint = b3Vec3_zero;
	b3ShapeProxy proxy = { &proxyPoint, 1, radius };
	b3wCastShapeClosestContext ctx = { 1.0f, 0 };
	b3World_CastShape(slot->worldId, (b3Pos){ originX, originY, originZ }, &proxy,
		(b3Vec3){ translationX, translationY, translationZ }, filter, b3wCastShapeClosestCallback, &ctx);
	return ctx.fraction;
}

B3W_EXPORT void b3wSetGravity(int worldHandle, float gx, float gy, float gz)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3World_SetGravity(slot->worldId, (b3Vec3){ gx, gy, gz });
}

B3W_EXPORT void b3wGetGravity(int worldHandle, float* out)
{
	if (out == NULL) return;
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL)
	{
		out[0] = out[1] = out[2] = 0.0f;
		return;
	}
	b3Vec3 g = b3World_GetGravity(slot->worldId);
	out[0] = g.x;
	out[1] = g.y;
	out[2] = g.z;
}

B3W_EXPORT void b3wEnableSpeculative(int worldHandle, int flag)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3World_EnableSpeculative(slot->worldId, flag != 0);
}

B3W_EXPORT void b3wSetRestitutionThreshold(int worldHandle, float value)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3World_SetRestitutionThreshold(slot->worldId, value);
}

B3W_EXPORT float b3wGetRestitutionThreshold(int worldHandle)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return 0.0f;
	return b3World_GetRestitutionThreshold(slot->worldId);
}

B3W_EXPORT void b3wSetHitEventThreshold(int worldHandle, float value)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3World_SetHitEventThreshold(slot->worldId, value);
}

B3W_EXPORT float b3wGetHitEventThreshold(int worldHandle)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return 0.0f;
	return b3World_GetHitEventThreshold(slot->worldId);
}

B3W_EXPORT void b3wSetMaximumLinearSpeed(int worldHandle, float value)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3World_SetMaximumLinearSpeed(slot->worldId, value);
}

B3W_EXPORT float b3wGetMaximumLinearSpeed(int worldHandle)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return 0.0f;
	return b3World_GetMaximumLinearSpeed(slot->worldId);
}

B3W_EXPORT float b3wGetContactRecycleDistance(int worldHandle)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return 0.0f;
	return b3World_GetContactRecycleDistance(slot->worldId);
}

#define B3W_OVERLAP_MAX 32

typedef struct b3wOverlapShapeContext
{
	uint64_t* out;
	int capacity;
	int count;
} b3wOverlapShapeContext;

static bool b3wOverlapShapeCallback(b3ShapeId shapeId, void* context)
{
	b3wOverlapShapeContext* ctx = (b3wOverlapShapeContext*)context;
	if (ctx->count >= ctx->capacity) return false;
	ctx->out[ctx->count++] = b3StoreShapeId(shapeId);
	return ctx->count < ctx->capacity;
}

B3W_EXPORT int b3wOverlapShape(int worldHandle, float ox, float oy, float oz, int proxyType, float radius,
	float ax, float ay, float az, float bx, float by, float bz, uint64_t* outShapes, int maxCount)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL || outShapes == NULL || maxCount <= 0) return 0;
	int cap = maxCount < B3W_OVERLAP_MAX ? maxCount : B3W_OVERLAP_MAX;
	b3Vec3 pointBuffer[8];
	b3ShapeProxy proxy = { 0 };
	b3BoxHull box = { 0 };
	switch (proxyType)
	{
	case 1:
		pointBuffer[0] = b3Vec3_zero;
		proxy.points = pointBuffer;
		proxy.count = 1;
		proxy.radius = radius;
		break;
	case 2:
		pointBuffer[0] = (b3Vec3){ ax, ay, az };
		pointBuffer[1] = (b3Vec3){ bx, by, bz };
		proxy.points = pointBuffer;
		proxy.count = 2;
		proxy.radius = radius;
		break;
	default:
	{
		b3Vec3 extent = { radius, 0.5f * radius, 0.25f * radius };
		box = b3MakeTransformedBoxHull(extent.x, extent.y, extent.z, (b3Transform){ b3Vec3_zero, b3Quat_identity });
		proxy.points = box.boxPoints;
		proxy.count = box.base.vertexCount;
		break;
	}
	}
	b3wOverlapShapeContext ctx = { outShapes, cap, 0 };
	b3World_OverlapShape(slot->worldId, (b3Pos){ ox, oy, oz }, &proxy, b3DefaultQueryFilter(), b3wOverlapShapeCallback, &ctx);
	return ctx.count;
}

B3W_EXPORT void b3wRebuildStaticTree(int worldHandle)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3World_RebuildStaticTree(slot->worldId);
}

#define B3W_ONE_WAY_USER_BIT 0x10000000

static bool b3wPreSolveOneWay(b3ShapeId shapeIdA, b3ShapeId shapeIdB, b3Pos point, b3Vec3 normal, void* context)
{
	(void)point;
	b3wWorldSlot* slot = (b3wWorldSlot*)context;
	intptr_t dataA = (intptr_t)b3Shape_GetUserData(shapeIdA);
	intptr_t dataB = (intptr_t)b3Shape_GetUserData(shapeIdB);
	int oneWayA = ((int)dataA & B3W_ONE_WAY_USER_BIT) != 0;
	int oneWayB = ((int)dataB & B3W_ONE_WAY_USER_BIT) != 0;
	float ny = normal.y;
	int keep = 1;
	if (oneWayA && !oneWayB)
	{
		keep = ny >= slot->preSolveMinNormalY;
	}
	else if (oneWayB && !oneWayA)
	{
		keep = -ny >= slot->preSolveMinNormalY;
	}
	if (keep)
	{
		slot->preSolveKeepCount += 1;
		return true;
	}
	slot->preSolveSkipCount += 1;
	return false;
}

B3W_EXPORT void b3wSetPreSolveOneWay(int worldHandle, int enabled, float minNormalY)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	slot->preSolveEnabled = enabled != 0;
	slot->preSolveMinNormalY = minNormalY;
	if (slot->preSolveEnabled)
	{
		b3World_SetPreSolveCallback(slot->worldId, b3wPreSolveOneWay, slot);
	}
	else
	{
		b3World_SetPreSolveCallback(slot->worldId, NULL, NULL);
	}
}

B3W_EXPORT void b3wGetPreSolveStats(int worldHandle, int* outKeep, int* outSkip)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (outKeep != NULL) *outKeep = slot == NULL ? 0 : slot->preSolveKeepCount;
	if (outSkip != NULL) *outSkip = slot == NULL ? 0 : slot->preSolveSkipCount;
}
