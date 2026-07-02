#pragma once

#include "box3d/box3d.h"  // IWYU pragma: export
#include "human.h"  // IWYU pragma: export

#include <stdbool.h>

#ifdef __EMSCRIPTEN__
#define B3W_EXPORT __attribute__((used))
#else
#define B3W_EXPORT
#endif

enum
{
	B3W_MAX_WORLDS = 128,
	B3W_MAX_BODIES = 65536,
	B3W_MAX_JOINTS = 65536,
	B3W_MAX_HULLS = 65536,
	B3W_MAX_SHAPES = 65536,
	B3W_MAX_COMPOUNDS = 4096,
	B3W_MAX_HUMANS = 256,
};

typedef struct b3wWorldSlot
{
	bool active;
	b3WorldId worldId;
} b3wWorldSlot;

typedef struct b3wBodySlot
{
	bool active;
	int worldHandle;
	b3BodyId bodyId;
} b3wBodySlot;

typedef struct b3wJointSlot
{
	bool active;
	int worldHandle;
	b3JointId jointId;
} b3wJointSlot;

typedef struct b3wHullSlot
{
	bool active;
	b3HullData* hull;
} b3wHullSlot;

typedef struct b3wShapeSlot
{
	bool active;
	b3ShapeId shapeId;
} b3wShapeSlot;

typedef struct b3wCompoundSlot
{
	bool active;
	b3CompoundData* compound;
} b3wCompoundSlot;

typedef struct b3wHumanSlot
{
	bool active;
	int worldHandle;
	Human human;
} b3wHumanSlot;

extern b3wWorldSlot g_worlds[B3W_MAX_WORLDS];
extern b3wBodySlot g_bodies[B3W_MAX_BODIES];
extern b3wJointSlot g_joints[B3W_MAX_JOINTS];
extern b3wHullSlot g_hulls[B3W_MAX_HULLS];
extern b3wShapeSlot g_shapes[B3W_MAX_SHAPES];
extern b3wCompoundSlot g_compounds[B3W_MAX_COMPOUNDS];
extern b3wHumanSlot g_humans[B3W_MAX_HUMANS];

b3wWorldSlot* b3wGetWorld(int handle);
b3wBodySlot* b3wGetBody(int handle);
b3wHullSlot* b3wGetHull(int handle);
b3wShapeSlot* b3wGetShape(int handle);
b3wCompoundSlot* b3wGetCompound(int handle);
b3wHumanSlot* b3wGetHuman(int handle);

int b3wAllocWorldSlot(b3WorldId worldId);
int b3wAllocBodySlot(int worldHandle, b3BodyId bodyId);
int b3wAllocJointSlot(int worldHandle, b3JointId jointId);
int b3wAllocHullSlot(b3HullData* hull);
int b3wAllocShapeSlot(b3ShapeId shapeId);
int b3wAllocCompoundSlot(b3CompoundData* compound);
int b3wAllocHumanSlot(int worldHandle, Human human);
void b3wClearWorldSlots(int worldHandle);
