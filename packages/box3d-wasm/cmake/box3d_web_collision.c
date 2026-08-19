#include "box3d_web_shared.h"

#include <string.h>

#define B3W_MANIFOLD_MAX_POINTS 64
#define B3W_MANIFOLD_HEADER_FLOATS 14
#define B3W_MANIFOLD_POINT_FLOATS 8

static b3WorldTransform b3wReadWorldTransform(const float* xf)
{
	b3WorldTransform t;
	t.p.x = xf[0];
	t.p.y = xf[1];
	t.p.z = xf[2];
	t.q.v.x = xf[3];
	t.q.v.y = xf[4];
	t.q.v.z = xf[5];
	t.q.s = xf[6];
	return t;
}

static b3Transform b3wRelativeBtoA(const float* xfA, const float* xfB)
{
	return b3InvMulWorldTransforms(b3wReadWorldTransform(xfA), b3wReadWorldTransform(xfB));
}

static void b3wWriteEmptyManifold(float* out, int capacityFloats)
{
	if (out == NULL || capacityFloats < B3W_MANIFOLD_HEADER_FLOATS)
	{
		return;
	}
	memset(out, 0, (size_t)B3W_MANIFOLD_HEADER_FLOATS * sizeof(float));
}

static void b3wWriteManifold(const b3LocalManifold* manifold, float* out, int capacityFloats)
{
	if (out == NULL || capacityFloats < B3W_MANIFOLD_HEADER_FLOATS)
	{
		return;
	}

	out[0] = manifold->normal.x;
	out[1] = manifold->normal.y;
	out[2] = manifold->normal.z;
	out[3] = manifold->triangleNormal.x;
	out[4] = manifold->triangleNormal.y;
	out[5] = manifold->triangleNormal.z;
	out[6] = (float)manifold->pointCount;
	out[7] = (float)manifold->feature;
	out[8] = (float)manifold->triangleIndex;
	out[9] = (float)manifold->i1;
	out[10] = (float)manifold->i2;
	out[11] = (float)manifold->i3;
	out[12] = manifold->squaredDistance;
	out[13] = (float)manifold->triangleFlags;

	int maxPoints = (capacityFloats - B3W_MANIFOLD_HEADER_FLOATS) / B3W_MANIFOLD_POINT_FLOATS;
	int count = manifold->pointCount;
	if (count > maxPoints)
	{
		count = maxPoints;
	}
	if (count > B3W_MANIFOLD_MAX_POINTS)
	{
		count = B3W_MANIFOLD_MAX_POINTS;
	}
	out[6] = (float)count;

	for (int i = 0; i < count; ++i)
	{
		const b3LocalManifoldPoint* point = &manifold->points[i];
		float* dst = out + B3W_MANIFOLD_HEADER_FLOATS + i * B3W_MANIFOLD_POINT_FLOATS;
		dst[0] = point->point.x;
		dst[1] = point->point.y;
		dst[2] = point->point.z;
		dst[3] = point->separation;
		dst[4] = (float)point->pair.owner1;
		dst[5] = (float)point->pair.index1;
		dst[6] = (float)point->pair.owner2;
		dst[7] = (float)point->pair.index2;
	}
}

static void b3wInitManifold(b3LocalManifold* manifold, b3LocalManifoldPoint* points, int capacity)
{
	memset(manifold, 0, sizeof(*manifold));
	memset(points, 0, (size_t)capacity * sizeof(b3LocalManifoldPoint));
	manifold->points = points;
}

static const b3HullData* b3wRequireHull(int hullHandle)
{
	b3wHullSlot* slot = b3wGetHull(hullHandle);
	if (slot == NULL)
	{
		return NULL;
	}
	return slot->hull;
}

B3W_EXPORT void b3wCollideSpheres(
	float ax, float ay, float az, float ar,
	float bx, float by, float bz, float br,
	const float* xfA, const float* xfB,
	int capacity, float* out, int capacityFloats)
{
	b3LocalManifoldPoint points[B3W_MANIFOLD_MAX_POINTS];
	b3LocalManifold manifold;
	int cap = capacity < 1 ? 1 : capacity;
	if (cap > B3W_MANIFOLD_MAX_POINTS)
	{
		cap = B3W_MANIFOLD_MAX_POINTS;
	}
	b3wInitManifold(&manifold, points, cap);
	b3Sphere sphereA = { { ax, ay, az }, ar };
	b3Sphere sphereB = { { bx, by, bz }, br };
	b3CollideSpheres(&manifold, cap, &sphereA, &sphereB, b3wRelativeBtoA(xfA, xfB));
	b3wWriteManifold(&manifold, out, capacityFloats);
}

B3W_EXPORT void b3wCollideCapsuleAndSphere(
	float c1x, float c1y, float c1z, float c2x, float c2y, float c2z, float cr,
	float sx, float sy, float sz, float sr,
	const float* xfA, const float* xfB,
	int capacity, float* out, int capacityFloats)
{
	b3LocalManifoldPoint points[B3W_MANIFOLD_MAX_POINTS];
	b3LocalManifold manifold;
	int cap = capacity < 1 ? 1 : capacity;
	if (cap > B3W_MANIFOLD_MAX_POINTS)
	{
		cap = B3W_MANIFOLD_MAX_POINTS;
	}
	b3wInitManifold(&manifold, points, cap);
	b3Capsule capsule = { { c1x, c1y, c1z }, { c2x, c2y, c2z }, cr };
	b3Sphere sphere = { { sx, sy, sz }, sr };
	b3CollideCapsuleAndSphere(&manifold, cap, &capsule, &sphere, b3wRelativeBtoA(xfA, xfB));
	b3wWriteManifold(&manifold, out, capacityFloats);
}

B3W_EXPORT void b3wCollideHullAndSphere(
	int hullHandle,
	float sx, float sy, float sz, float sr,
	const float* xfA, const float* xfB,
	int capacity, float* out, int capacityFloats)
{
	const b3HullData* hull = b3wRequireHull(hullHandle);
	if (hull == NULL)
	{
		b3wWriteEmptyManifold(out, capacityFloats);
		return;
	}
	b3LocalManifoldPoint points[B3W_MANIFOLD_MAX_POINTS];
	b3LocalManifold manifold;
	int cap = capacity < 1 ? 1 : capacity;
	if (cap > B3W_MANIFOLD_MAX_POINTS)
	{
		cap = B3W_MANIFOLD_MAX_POINTS;
	}
	b3wInitManifold(&manifold, points, cap);
	b3Sphere sphere = { { sx, sy, sz }, sr };
	b3SimplexCache cache = { 0 };
	b3CollideHullAndSphere(&manifold, cap, hull, &sphere, b3wRelativeBtoA(xfA, xfB), &cache);
	b3wWriteManifold(&manifold, out, capacityFloats);
}

B3W_EXPORT void b3wCollideCapsules(
	float a1x, float a1y, float a1z, float a2x, float a2y, float a2z, float ar,
	float b1x, float b1y, float b1z, float b2x, float b2y, float b2z, float br,
	const float* xfA, const float* xfB,
	int capacity, float* out, int capacityFloats)
{
	b3LocalManifoldPoint points[B3W_MANIFOLD_MAX_POINTS];
	b3LocalManifold manifold;
	int cap = capacity < 1 ? 1 : capacity;
	if (cap > B3W_MANIFOLD_MAX_POINTS)
	{
		cap = B3W_MANIFOLD_MAX_POINTS;
	}
	b3wInitManifold(&manifold, points, cap);
	b3Capsule capsuleA = { { a1x, a1y, a1z }, { a2x, a2y, a2z }, ar };
	b3Capsule capsuleB = { { b1x, b1y, b1z }, { b2x, b2y, b2z }, br };
	b3CollideCapsules(&manifold, cap, &capsuleA, &capsuleB, b3wRelativeBtoA(xfA, xfB));
	b3wWriteManifold(&manifold, out, capacityFloats);
}

B3W_EXPORT void b3wCollideHullAndCapsule(
	int hullHandle,
	float c1x, float c1y, float c1z, float c2x, float c2y, float c2z, float cr,
	const float* xfA, const float* xfB,
	int capacity, float* out, int capacityFloats)
{
	const b3HullData* hull = b3wRequireHull(hullHandle);
	if (hull == NULL)
	{
		b3wWriteEmptyManifold(out, capacityFloats);
		return;
	}
	b3LocalManifoldPoint points[B3W_MANIFOLD_MAX_POINTS];
	b3LocalManifold manifold;
	int cap = capacity < 1 ? 1 : capacity;
	if (cap > B3W_MANIFOLD_MAX_POINTS)
	{
		cap = B3W_MANIFOLD_MAX_POINTS;
	}
	b3wInitManifold(&manifold, points, cap);
	b3Capsule capsule = { { c1x, c1y, c1z }, { c2x, c2y, c2z }, cr };
	b3SimplexCache cache = { 0 };
	b3CollideHullAndCapsule(&manifold, cap, hull, &capsule, b3wRelativeBtoA(xfA, xfB), &cache);
	b3wWriteManifold(&manifold, out, capacityFloats);
}

B3W_EXPORT void b3wCollideHulls(
	int hullAHandle, int hullBHandle,
	const float* xfA, const float* xfB,
	int capacity, float* out, int capacityFloats)
{
	const b3HullData* hullA = b3wRequireHull(hullAHandle);
	const b3HullData* hullB = b3wRequireHull(hullBHandle);
	if (hullA == NULL || hullB == NULL)
	{
		b3wWriteEmptyManifold(out, capacityFloats);
		return;
	}
	b3LocalManifoldPoint points[B3W_MANIFOLD_MAX_POINTS];
	b3LocalManifold manifold;
	int cap = capacity < 1 ? 1 : capacity;
	if (cap > B3W_MANIFOLD_MAX_POINTS)
	{
		cap = B3W_MANIFOLD_MAX_POINTS;
	}
	b3wInitManifold(&manifold, points, cap);
	b3SATCache cache = { 0 };
	b3CollideHulls(&manifold, cap, hullA, hullB, b3wRelativeBtoA(xfA, xfB), &cache);
	b3wWriteManifold(&manifold, out, capacityFloats);
}

static void b3wTriangleInFrameB(const float* triangle, const float* xfA, const float* xfB, b3Vec3* localTriangle)
{
	b3Transform xf = b3InvMulWorldTransforms(b3wReadWorldTransform(xfB), b3wReadWorldTransform(xfA));
	for (int i = 0; i < 3; ++i)
	{
		b3Vec3 v = { triangle[i * 3 + 0], triangle[i * 3 + 1], triangle[i * 3 + 2] };
		localTriangle[i] = b3TransformPoint(xf, v);
	}
}

B3W_EXPORT void b3wCollideTriangleAndSphere(
	const float* triangle,
	float sx, float sy, float sz, float sr,
	const float* xfA, const float* xfB,
	int capacity, float* out, int capacityFloats)
{
	b3LocalManifoldPoint points[B3W_MANIFOLD_MAX_POINTS];
	b3LocalManifold manifold;
	int cap = capacity < 1 ? 1 : capacity;
	if (cap > B3W_MANIFOLD_MAX_POINTS)
	{
		cap = B3W_MANIFOLD_MAX_POINTS;
	}
	b3wInitManifold(&manifold, points, cap);
	b3Vec3 localTriangle[3];
	b3wTriangleInFrameB(triangle, xfA, xfB, localTriangle);
	b3Sphere sphere = { { sx, sy, sz }, sr };
	b3CollideTriangleAndSphere(&manifold, cap, localTriangle, &sphere);
	b3wWriteManifold(&manifold, out, capacityFloats);
}

B3W_EXPORT void b3wCollideTriangleAndCapsule(
	const float* triangle,
	float c1x, float c1y, float c1z, float c2x, float c2y, float c2z, float cr,
	const float* xfA, const float* xfB,
	int capacity, float* out, int capacityFloats)
{
	b3LocalManifoldPoint points[B3W_MANIFOLD_MAX_POINTS];
	b3LocalManifold manifold;
	int cap = capacity < 1 ? 1 : capacity;
	if (cap > B3W_MANIFOLD_MAX_POINTS)
	{
		cap = B3W_MANIFOLD_MAX_POINTS;
	}
	b3wInitManifold(&manifold, points, cap);
	b3Vec3 localTriangle[3];
	b3wTriangleInFrameB(triangle, xfA, xfB, localTriangle);
	b3Capsule capsule = { { c1x, c1y, c1z }, { c2x, c2y, c2z }, cr };
	b3SimplexCache cache = { 0 };
	b3CollideTriangleAndCapsule(&manifold, cap, localTriangle, &capsule, &cache);
	b3wWriteManifold(&manifold, out, capacityFloats);
}

B3W_EXPORT void b3wCollideTriangleAndHull(
	const float* triangle, int triangleFlags, int hullHandle,
	const float* xfA, const float* xfB,
	int capacity, int enableSpeculative, float* out, int capacityFloats)
{
	const b3HullData* hull = b3wRequireHull(hullHandle);
	if (hull == NULL)
	{
		b3wWriteEmptyManifold(out, capacityFloats);
		return;
	}
	b3LocalManifoldPoint points[B3W_MANIFOLD_MAX_POINTS];
	b3LocalManifold manifold;
	int cap = capacity < 1 ? 1 : capacity;
	if (cap > B3W_MANIFOLD_MAX_POINTS)
	{
		cap = B3W_MANIFOLD_MAX_POINTS;
	}
	b3wInitManifold(&manifold, points, cap);
	b3Vec3 localTriangle[3];
	b3wTriangleInFrameB(triangle, xfA, xfB, localTriangle);
	b3SATCache cache = { 0 };
	b3CollideTriangleAndHull(&manifold, cap, localTriangle[0], localTriangle[1], localTriangle[2], triangleFlags, hull, &cache,
		enableSpeculative != 0);
	b3wWriteManifold(&manifold, out, capacityFloats);
}

B3W_EXPORT void b3wShapeCast(
	const float* pointsA, int countA, float radiusA,
	const float* pointsB, int countB, float radiusB,
	const float* transformBtoA, const float* translationB,
	float maxFraction, int canEncroach,
	float* out)
{
	if (out == NULL)
	{
		return;
	}
	b3ShapeCastPairInput input = { 0 };
	input.proxyA.points = (const b3Vec3*)pointsA;
	input.proxyA.count = countA;
	input.proxyA.radius = radiusA;
	input.proxyB.points = (const b3Vec3*)pointsB;
	input.proxyB.count = countB;
	input.proxyB.radius = radiusB;
	input.transform.p.x = transformBtoA[0];
	input.transform.p.y = transformBtoA[1];
	input.transform.p.z = transformBtoA[2];
	input.transform.q.v.x = transformBtoA[3];
	input.transform.q.v.y = transformBtoA[4];
	input.transform.q.v.z = transformBtoA[5];
	input.transform.q.s = transformBtoA[6];
	input.translationB.x = translationB[0];
	input.translationB.y = translationB[1];
	input.translationB.z = translationB[2];
	input.maxFraction = maxFraction;
	input.canEncroach = canEncroach != 0;
	b3CastOutput result = b3ShapeCast(&input);
	out[0] = result.hit ? 1.0f : 0.0f;
	out[1] = result.fraction;
	out[2] = result.point.x;
	out[3] = result.point.y;
	out[4] = result.point.z;
	out[5] = result.normal.x;
	out[6] = result.normal.y;
	out[7] = result.normal.z;
	out[8] = (float)result.iterations;
}
