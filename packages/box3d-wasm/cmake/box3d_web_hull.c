#include "box3d_web_shared.h"

#include <stdlib.h>

B3W_EXPORT int b3wCreateCylinder(float height, float radius, float yOffset, int sides)
{
	b3HullData* hull = b3CreateCylinder(height, radius, yOffset, sides);
	return b3wAllocHullSlot(hull);
}

B3W_EXPORT int b3wCreateHullFromPoints(int numPoints, float* points, int maxVertexCount)
{
	b3Vec3* b3points = (b3Vec3*)points;
	int maxCount = maxVertexCount > 0 && maxVertexCount < numPoints ? maxVertexCount : numPoints;
	b3HullData* hull = b3CreateHull(b3points, numPoints, maxCount);
	if (hull == NULL) return 0;
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
	if (slot->boxStorage != NULL)
	{
		free(slot->boxStorage);
		slot->boxStorage = NULL;
		slot->hull = NULL;
	}
	else
	{
		b3DestroyHull(slot->hull);
	}
	b3wFreeHullSlot(hullHandle);
}

static int b3wAllocBoxHullStorage(b3BoxHull* box)
{
	if (box == NULL) return 0;
	int handle = b3wTryAllocHullSlot(&box->base);
	if (handle == 0)
	{
		free(box);
		return 0;
	}
	b3wHullSlot* slot = b3wGetHull(handle);
	if (slot == NULL)
	{
		free(box);
		return 0;
	}
	slot->boxStorage = box;
	return handle;
}

B3W_EXPORT int b3wMakeBoxHull(float hx, float hy, float hz)
{
	b3BoxHull* box = (b3BoxHull*)malloc(sizeof(b3BoxHull));
	if (box == NULL) return 0;
	*box = b3MakeBoxHull(hx, hy, hz);
	return b3wAllocBoxHullStorage(box);
}

B3W_EXPORT int b3wMakeTransformedBoxHull(float hx, float hy, float hz, float px, float py, float pz, float qx, float qy, float qz, float qs)
{
	b3BoxHull* box = (b3BoxHull*)malloc(sizeof(b3BoxHull));
	if (box == NULL) return 0;
	b3Transform transform = { { px, py, pz }, { { qx, qy, qz }, qs } };
	*box = b3MakeTransformedBoxHull(hx, hy, hz, transform);
	return b3wAllocBoxHullStorage(box);
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

B3W_EXPORT int b3wMakeScaledBoxHull(
	float hx, float hy, float hz,
	float px, float py, float pz,
	float qx, float qy, float qz, float qs,
	float sx, float sy, float sz)
{
	b3BoxHull* box = (b3BoxHull*)malloc(sizeof(b3BoxHull));
	if (box == NULL) return 0;
	b3Transform transform = { { px, py, pz }, { { qx, qy, qz }, qs } };
	*box = b3MakeScaledBoxHull((b3Vec3){ hx, hy, hz }, transform, (b3Vec3){ sx, sy, sz });
	return b3wAllocBoxHullStorage(box);
}

B3W_EXPORT int b3wCloneAndTransformHull(
	int hullHandle,
	float px, float py, float pz,
	float qx, float qy, float qz, float qs,
	float sx, float sy, float sz)
{
	b3wHullSlot* slot = b3wGetHull(hullHandle);
	if (slot == NULL || slot->hull == NULL) return 0;
	b3Transform transform = { { px, py, pz }, { { qx, qy, qz }, qs } };
	b3HullData* hull = b3CloneAndTransformHull(slot->hull, transform, (b3Vec3){ sx, sy, sz });
	if (hull == NULL) return 0;
	return b3wAllocHullSlot(hull);
}

B3W_EXPORT void b3wGetHullInfo(int hullHandle, float* out)
{
	if (out == NULL) return;
	b3wHullSlot* slot = b3wGetHull(hullHandle);
	if (slot == NULL || slot->hull == NULL)
	{
		out[0] = 0;
		out[1] = 0;
		out[2] = 0;
		out[3] = 0;
		out[4] = 0;
		out[5] = 0;
		return;
	}
	out[0] = (float)slot->hull->vertexCount;
	out[1] = (float)slot->hull->faceCount;
	out[2] = (float)(slot->hull->edgeCount / 2);
	out[3] = slot->hull->surfaceArea;
	out[4] = slot->hull->volume;
	out[5] = slot->hull->innerRadius;
}

B3W_EXPORT void b3wComputeHullMass(int hullHandle, float density, float* out)
{
	if (out == NULL) return;
	b3wHullSlot* slot = b3wGetHull(hullHandle);
	if (slot == NULL || slot->hull == NULL)
	{
		out[0] = 0;
		out[1] = 0;
		out[2] = 0;
		out[3] = 0;
		return;
	}
	b3MassData mass = b3ComputeHullMass(slot->hull, density);
	out[0] = mass.mass;
	out[1] = mass.inertia.cx.x;
	out[2] = mass.inertia.cy.y;
	out[3] = mass.inertia.cz.z;
}

B3W_EXPORT void b3wComputeCapsuleMass(
	float ax, float ay, float az,
	float bx, float by, float bz,
	float radius,
	float density,
	float* out)
{
	if (out == NULL) return;
	b3Capsule capsule = { { ax, ay, az }, { bx, by, bz }, radius };
	b3MassData mass = b3ComputeCapsuleMass(&capsule, density);
	out[0] = mass.mass;
	out[1] = mass.inertia.cx.x;
	out[2] = mass.inertia.cy.y;
	out[3] = mass.inertia.cz.z;
}
