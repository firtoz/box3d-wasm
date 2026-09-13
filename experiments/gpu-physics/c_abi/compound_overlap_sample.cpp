#include "box3d/box3d.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>

extern "C" void b3World_Wait(b3WorldId worldId);
extern "C" uint32_t gpu_b3_world_last_static_sort_dispatches(b3WorldId id);

int main(void)
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.enableSleep = false;
	b3WorldId world = b3CreateWorld(&worldDef);
	if (!b3World_IsValid(world))
	{
		fprintf(stderr, "b3CreateWorld failed\n");
		return 1;
	}

	b3BodyDef groundDef = b3DefaultBodyDef();
	groundDef.position = b3Pos{0.0f, -1.0f, 0.0f};
	b3BodyId ground = b3CreateBody(world, &groundDef);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	b3BoxHull groundHull = b3MakeBoxHull(20.0f, 1.0f, 20.0f);
	b3CreateHullShape(ground, &shapeDef, &groundHull.base);

	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.enableSleep = false;
	b3BoxHull cube = b3MakeBoxHull(0.5f, 0.5f, 0.5f);
	b3BodyId bodies[3];
	const float xs[3] = {0.0f, 3.0f, 6.0f};
	for (int i = 0; i < 3; ++i)
	{
		bodyDef.position = {xs[i], 0.5f, 0.0f};
		bodies[i] = b3CreateBody(world, &bodyDef);
		b3CreateHullShape(bodies[i], &shapeDef, &cube.base);
		b3CreateHullShape(bodies[i], &shapeDef, &cube.base);
	}

	for (int step = 0; step < 120; ++step)
	{
		b3World_Step(world, 1.0f / 60.0f, 4);
	}
	b3World_Wait(world);

	uint32_t sorts = gpu_b3_world_last_static_sort_dispatches(world);
	if (sorts == 0)
	{
		fprintf(stderr, "compound degree>1 skipped the general static sort\n");
		return 1;
	}

	for (int i = 0; i < 3; ++i)
	{
		b3Pos p = b3Body_GetPosition(bodies[i]);
		printf("compound[%d] y=%f\n", i, (float)p.y);
		if (fabsf((float)p.y - 0.5f) > 0.05f)
		{
			fprintf(stderr, "compound body %d fell to y=%f\n", i, (float)p.y);
			return 1;
		}
	}
	b3DestroyWorld(world);
	return 0;
}
