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
