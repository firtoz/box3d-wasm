#include "box3d_web_shared.h"

B3W_EXPORT float b3wSin(float radians)
{
	return b3Sin(radians);
}

B3W_EXPORT float b3wCos(float radians)
{
	return b3Cos(radians);
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
