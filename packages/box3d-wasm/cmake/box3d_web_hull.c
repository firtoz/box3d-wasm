#include "box3d_web_shared.h"

B3W_EXPORT int b3wCreateCylinder(float height, float radius, float yOffset, int sides)
{
	b3HullData* hull = b3CreateCylinder(height, radius, yOffset, sides);
	return b3wAllocHullSlot(hull);
}

B3W_EXPORT void b3wDestroyHull(int hullHandle)
{
	b3wHullSlot* slot = b3wGetHull(hullHandle);
	if (slot == NULL) return;
	b3DestroyHull(slot->hull);
	slot->active = false;
}
