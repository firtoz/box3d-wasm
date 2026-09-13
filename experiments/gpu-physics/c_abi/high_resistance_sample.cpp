#include "box3d/box3d.h"

#include <math.h>
#include <stdio.h>

extern "C" void b3World_Wait(b3WorldId worldId);

int main(void)
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.enableSleep = false;
	b3WorldId world = b3CreateWorld(&worldDef);
	if (!b3World_IsValid(world))
	{
		fprintf(stderr, "b3CreateWorld failed (GPU?)\n");
		return 1;
	}

	b3BodyDef groundDef = b3DefaultBodyDef();
	groundDef.position = b3Pos{0.0f, -1.0f, 0.0f};
	b3BodyId ground = b3CreateBody(world, &groundDef);
	b3BoxHull groundHull = b3MakeBoxHull(50.0f, 1.0f, 50.0f);
	b3ShapeDef groundShape = b3DefaultShapeDef();
	b3CreateHullShape(ground, &groundShape, &groundHull.base);

	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.enableSleep = false;
	bodyDef.isAwake = true;
	bodyDef.rotation = b3MakeQuatFromAxisAngle(b3Vec3_axisZ, B3_DEG_TO_RAD * 30.0f);
	b3Capsule capsule = {{0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.5f};
	b3BodyId bodies[10];
	for (int index = 0; index < 10; ++index)
	{
		bodyDef.position = {-22.0f + 5.0f * (float)index, 1.5f, 0.0f};
		bodies[index] = b3CreateBody(world, &bodyDef);
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.baseMaterial.rollingResistance = 0.2f * (float)index;
		b3CreateCapsuleShape(bodies[index], &shapeDef, &capsule);
	}

	for (uint32_t step = 0; step < 600; ++step)
	{
		b3World_Step(world, 1.0f / 60.0f, 4);
	}
	b3World_Wait(world);

	for (int i = 0; i < 10; ++i)
	{
		b3Pos p = b3Body_GetPosition(bodies[i]);
		b3Quat q = b3Body_GetRotation(bodies[i]);
		b3Vec3 v = b3Body_GetLinearVelocity(bodies[i]);
		float speed = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
		float axis_y = fabsf(1.0f - 2.0f * (q.v.x * q.v.x + q.v.z * q.v.z));
		printf("%d y=%.6f speed=%.4e axis_y=%.4f\n", i, (float)p.y, speed, axis_y);
		if (speed > 0.01f)
		{
			fprintf(stderr, "body %d still moving\n", i);
			return 1;
		}
		float bottom = (float)p.y - axis_y * 1.0f - 0.5f;
		if (bottom < -0.02f || bottom > 0.02f)
		{
			fprintf(stderr, "body %d support y=%f\n", i, bottom);
			return 1;
		}
		if (i < 6 && axis_y > 0.25f)
		{
			fprintf(stderr, "body %d should be flat\n", i);
			return 1;
		}
		if (i >= 6 && axis_y < 0.25f)
		{
			fprintf(stderr, "body %d should be tilted\n", i);
			return 1;
		}
	}

	b3Pos before = b3Body_GetPosition(bodies[0]);
	(void)b3Body_GetRotation(bodies[0]);
	(void)b3Body_GetLinearVelocity(bodies[0]);
	b3Pos after = b3Body_GetPosition(bodies[0]);
	if (fabsf((float)(after.y - before.y)) > 1.0e-5f)
	{
		fprintf(stderr, "getter advanced physics\n");
		return 1;
	}

	b3DestroyWorld(world);
	return 0;
}
