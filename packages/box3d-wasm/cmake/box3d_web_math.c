#include "box3d_web_shared.h"

#include <math.h>

B3W_EXPORT float b3wSin(float radians)
{
	return b3Sin(radians);
}

B3W_EXPORT float b3wCos(float radians)
{
	return b3Cos(radians);
}

// Direct float32 cos/sin from math.h (not Box3D's Bhaskara approximation),
// matching upstream C++ sample code that uses cosf/sinf directly.
B3W_EXPORT float b3wCosf(float radians)
{
	return cosf(radians);
}

B3W_EXPORT float b3wSinf(float radians)
{
	return sinf(radians);
}

B3W_EXPORT void b3wMakeQuatFromAxisAngle(float axisX, float axisY, float axisZ, float radians, float* outQuat)
{
	if (outQuat == NULL) return;
	b3Quat q = b3MakeQuatFromAxisAngle((b3Vec3){ axisX, axisY, axisZ }, radians);
	outQuat[0] = q.v.x;
	outQuat[1] = q.v.y;
	outQuat[2] = q.v.z;
	outQuat[3] = q.s;
}

B3W_EXPORT void b3wRotateVector(float qx, float qy, float qz, float qs, float vx, float vy, float vz, float* outVec)
{
	if (outVec == NULL) return;
	b3Quat q = { { qx, qy, qz }, qs };
	b3Vec3 v = { vx, vy, vz };
	b3Vec3 r = b3RotateVector(q, v);
	outVec[0] = r.x;
	outVec[1] = r.y;
	outVec[2] = r.z;
}
