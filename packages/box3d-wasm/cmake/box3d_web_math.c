#include "box3d_web_shared.h"
#include "utils.h"

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

B3W_EXPORT void b3wRandomVec3(float lox, float loy, float loz, float hix, float hiy, float hiz, float* outVec)
{
	if (outVec == NULL) return;
	b3Vec3 v = RandomVec3( (b3Vec3){ lox, loy, loz }, (b3Vec3){ hix, hiy, hiz } );
	outVec[0] = v.x;
	outVec[1] = v.y;
	outVec[2] = v.z;
}

B3W_EXPORT void b3wLerpVec3(float ax, float ay, float az, float bx, float by, float bz, float alpha, float* outVec)
{
	if (outVec == NULL) return;
	b3Vec3 c = b3Lerp( (b3Vec3){ ax, ay, az }, (b3Vec3){ bx, by, bz }, alpha );
	outVec[0] = c.x;
	outVec[1] = c.y;
	outVec[2] = c.z;
}

B3W_EXPORT float b3wGetLengthAndNormalize( float vx, float vy, float vz, float* outDirection )
{
	b3Vec3 v = { vx, vy, vz };
	float length;
	b3Vec3 direction = b3GetLengthAndNormalize( &length, v );
	if ( outDirection != NULL )
	{
		outDirection[0] = direction.x;
		outDirection[1] = direction.y;
		outDirection[2] = direction.z;
	}
	return length;
}
