#include "box3d_web_shared.h"

B3W_EXPORT int b3wCreateWorld(float gravityX, float gravityY, float gravityZ, int workerCount)
{
	b3WorldDef def = b3DefaultWorldDef();
	def.gravity = (b3Vec3){ gravityX, gravityY, gravityZ };
	def.workerCount = workerCount > 0 ? workerCount : 1;
	b3WorldId worldId = b3CreateWorld(&def);
	return b3wAllocWorldSlot(worldId);
}

B3W_EXPORT void b3wDestroyWorld(int worldHandle)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return;
	b3DestroyWorld(slot->worldId);
	b3wClearWorldSlots(worldHandle);
	slot->active = false;
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
#include <emscripten/threading.h>
B3W_EXPORT int b3wCheckThreadingSupport(void)
{
	// Returns bitmask: bit0=SharedArrayBuffer available, bit1=pthread_create works
	int result = 0;
	if (emscripten_has_threading_support()) result |= 1;
	return result;
}
#endif

B3W_EXPORT int b3wGetWorldAwakeBodyCount(int worldHandle)
{
	b3wWorldSlot* slot = b3wGetWorld(worldHandle);
	if (slot == NULL) return 0;
	return b3World_GetAwakeBodyCount(slot->worldId);
}

B3W_EXPORT void b3wRayCastClosest(int worldHandle, float originX, float originY, float originZ, float translationX, float translationY, float translationZ, int categoryBits, int maskBits, int* outShapeHandle, float* outPoint, float* outNormal, float* outFraction)
{
	if (outShapeHandle != NULL) *outShapeHandle = 0;
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
	if (outShapeHandle != NULL) *outShapeHandle = result.shapeId.index1;
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
