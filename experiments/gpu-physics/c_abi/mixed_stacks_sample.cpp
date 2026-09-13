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
		fprintf(stderr, "b3CreateWorld failed\n");
		return 1;
	}

	b3ShapeDef shapeDef = b3DefaultShapeDef();
	for (int g = 0; g < 2; ++g)
	{
		b3BodyDef groundDef = b3DefaultBodyDef();
		groundDef.position = b3Pos{0.0f, -1.0f, 0.0f};
		b3BodyId ground = b3CreateBody(world, &groundDef);
		b3BoxHull hull = b3MakeBoxHull(100.0f, 1.0f, 100.0f);
		b3CreateHullShape(ground, &shapeDef, &hull.base);
	}

	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.enableSleep = false;
	b3BoxHull cube = b3MakeCubeHull(0.5f);
	b3BodyId bodies[600];
	for (int i = 0; i < 600; ++i)
	{
		bodyDef.position = {3.0f * (float)(i % 20), 0.5f + (float)(i / 300), 3.0f * (float)((i % 300) / 20)};
		bodies[i] = b3CreateBody(world, &bodyDef);
		b3CreateHullShape(bodies[i], &shapeDef, &cube.base);
	}

	for (int step = 0; step < 120; ++step)
	{
		b3World_Step(world, 1.0f / 60.0f, 4);
	}
	b3World_Wait(world);

	float min_y = 1.0e9f;
	float max_y = -1.0e9f;
	for (int i = 0; i < 600; ++i)
	{
		b3Pos p = b3Body_GetPosition(bodies[i]);
		min_y = fminf(min_y, (float)p.y);
		max_y = fmaxf(max_y, (float)p.y);
	}
	printf("mixed-stacks y %f .. %f\n", min_y, max_y);
	if (min_y < 0.45f || max_y > 1.6f)
	{
		fprintf(stderr, "exploded or collapsed\n");
		return 1;
	}
	b3DestroyWorld(world);
	return 0;
}
