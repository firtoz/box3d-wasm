#include "box3d_web_shared.h"

B3W_EXPORT int b3wCreateWorld(float gravityX, float gravityY, float gravityZ)
{
	b3WorldDef def = b3DefaultWorldDef();
	def.gravity = (b3Vec3){ gravityX, gravityY, gravityZ };
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
