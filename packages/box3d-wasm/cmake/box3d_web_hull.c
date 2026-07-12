#include "box3d_web_shared.h"

B3W_EXPORT int b3wCreateCylinder(float height, float radius, float yOffset, int sides)
{
	b3HullData* hull = b3CreateCylinder(height, radius, yOffset, sides);
	return b3wAllocHullSlot(hull);
}

B3W_EXPORT int b3wCreateHullFromPoints(int numPoints, float* points)
{
	b3Vec3* b3points = (b3Vec3*)points;
	b3HullData* hull = b3CreateHull(b3points, numPoints, numPoints);
	return b3wAllocHullSlot(hull);
}

B3W_EXPORT int b3wCreateRock(float radius)
{
	b3HullData* hull = b3CreateRock(radius);
	if (hull == NULL) return 0;
	return b3wAllocHullSlot(hull);
}

B3W_EXPORT void b3wDestroyHull(int hullHandle)
{
	b3wHullSlot* slot = b3wGetHull(hullHandle);
	if (slot == NULL) return;
	b3DestroyHull(slot->hull);
	b3wFreeHullSlot(hullHandle);
}
