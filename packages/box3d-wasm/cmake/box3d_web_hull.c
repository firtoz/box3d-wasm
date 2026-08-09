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

B3W_EXPORT int b3wGetHullVertexCount(int hullHandle)
{
	b3wHullSlot* slot = b3wGetHull(hullHandle);
	if (slot == NULL || slot->hull == NULL) return 0;
	return slot->hull->vertexCount;
}

/// Copy hull points as tightly packed xyz floats. Returns the vertex count written
/// (or needed when outPoints is NULL / capacity is too small).
B3W_EXPORT int b3wGetHullPoints(int hullHandle, float* outPoints, int capacityFloats)
{
	b3wHullSlot* slot = b3wGetHull(hullHandle);
	if (slot == NULL || slot->hull == NULL) return 0;
	const int vertexCount = slot->hull->vertexCount;
	const int needed = vertexCount * 3;
	if (outPoints == NULL || capacityFloats < needed) return needed;
	const b3Vec3* points = b3GetHullPoints(slot->hull);
	if (points == NULL) return 0;
	for (int i = 0; i < vertexCount; ++i)
	{
		outPoints[i * 3 + 0] = points[i].x;
		outPoints[i * 3 + 1] = points[i].y;
		outPoints[i * 3 + 2] = points[i].z;
	}
	return vertexCount;
}
