#include "box3d/box3d.h"

#include <stdio.h>
#include <stdint.h>

static const uint32_t kCheckpoints[] = {0, 50, 100, 200, 300};

int main(void)
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	b3WorldId world = b3CreateWorld(&worldDef);
	if (!b3World_IsValid(world))
	{
		fprintf(stderr, "b3CreateWorld failed (GPU?)\n");
		return 1;
	}

	b3BodyDef groundDef = b3DefaultBodyDef();
	groundDef.position = b3Pos{0.0f, -0.5f, 0.0f};
	b3BodyId ground = b3CreateBody(world, &groundDef);
	b3BoxHull groundHull = b3MakeBoxHull(12.0f, 0.5f, 12.0f);
	b3ShapeDef shapeDef = b3DefaultShapeDef();
	b3CreateHullShape(ground, &shapeDef, &groundHull.base);

	b3BoxHull cube = b3MakeBoxHull(0.5f, 0.5f, 0.5f);
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	b3BodyId boxes[6];
	for (int i = 0; i < 6; ++i)
	{
		bodyDef.position = b3Pos{0.0f, 0.5f + (float)i * 1.05f, 0.0f};
		boxes[i] = b3CreateBody(world, &bodyDef);
		b3CreateHullShape(boxes[i], &shapeDef, &cube.base);
	}

	uint32_t step = 0;
	size_t cp = 0;
	const uint32_t last = 300;
	while (1)
	{
		if (cp < 5 && step == kCheckpoints[cp])
		{
			printf("step %u\n", step);
			for (int i = 0; i < 6; ++i)
			{
				b3Pos p = b3Body_GetPosition(boxes[i]);
				printf("  %d %.9g %.9g %.9g\n", i, p.x, p.y, p.z);
			}
			cp++;
		}
		if (step == last)
		{
			break;
		}
		b3World_Step(world, 1.0f / 60.0f, 4);
		step++;
	}

	(void)b3Body_GetPosition(boxes[0]);
	b3Body_SetTransform(boxes[0], b3Pos{100.0f, 20.0f, 0.0f}, b3Quat_identity);
	b3Pos afterSet = b3Body_GetPosition(boxes[0]);
	if (afterSet.x < 99.0f || afterSet.y < 19.0f)
	{
		fprintf(stderr, "C ABI setter lost to pose snapshot: %g %g %g\n", afterSet.x, afterSet.y, afterSet.z);
		return 1;
	}

	b3DestroyWorld(world);
	return 0;
}
