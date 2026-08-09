#pragma once

#include "box3d/box3d.h"  // IWYU pragma: export
#include "human.h"  // IWYU pragma: export

#include <stdbool.h>
#include <stdint.h>

#ifdef __EMSCRIPTEN__
#define B3W_EXPORT __attribute__((used))
#else
#define B3W_EXPORT
#endif

#ifndef B3W_MAX_WORLDS
#define B3W_MAX_WORLDS 16
#endif
#ifndef B3W_MAX_HULLS
#define B3W_MAX_HULLS 16384
#endif
#ifndef B3W_MAX_MESHES
#define B3W_MAX_MESHES 1024
#endif
#ifndef B3W_MAX_COMPOUNDS
#define B3W_MAX_COMPOUNDS 1024
#endif
#ifndef B3W_MAX_HUMANS
#define B3W_MAX_HUMANS 512
#endif
#ifndef B3W_MAX_HEIGHT_FIELDS
#define B3W_MAX_HEIGHT_FIELDS 256
#endif

// Slot kinds (GetSlotLimits / GetSlotUsage order):
// 0 worlds, 1 hulls, 2 meshes, 3 compounds, 4 humans, 5 heightFields
#define B3W_SLOT_KIND_COUNT 6
#define B3W_SLOT_FREE_NONE (-1)

typedef struct b3wWorldSlot
{
	bool active;
	int nextFree;
	b3WorldId worldId;
} b3wWorldSlot;

typedef struct b3wHullSlot
{
	bool active;
	int nextFree;
	b3HullData* hull;
} b3wHullSlot;

typedef struct b3wMeshSlot
{
	bool active;
	int nextFree;
	int worldHandle;
	b3MeshData* mesh;
} b3wMeshSlot;

typedef struct b3wCompoundSlot
{
	bool active;
	int nextFree;
	b3CompoundData* compound;
} b3wCompoundSlot;

typedef struct b3wHumanSlot
{
	bool active;
	int nextFree;
	int worldHandle;
	Human human;
} b3wHumanSlot;

typedef struct b3wHeightFieldSlot
{
	bool active;
	int nextFree;
	int worldHandle;
	b3HeightFieldData* heightField;
} b3wHeightFieldSlot;

extern b3wWorldSlot g_worlds[B3W_MAX_WORLDS];
extern b3wHullSlot g_hulls[B3W_MAX_HULLS];
extern b3wMeshSlot g_meshes[B3W_MAX_MESHES];
extern b3wCompoundSlot g_compounds[B3W_MAX_COMPOUNDS];
extern b3wHumanSlot g_humans[B3W_MAX_HUMANS];
extern b3wHeightFieldSlot g_heightFields[B3W_MAX_HEIGHT_FIELDS];

b3wWorldSlot* b3wGetWorld(int handle);
b3wHullSlot* b3wGetHull(int handle);
b3wMeshSlot* b3wGetMesh(int handle);
b3wCompoundSlot* b3wGetCompound(int handle);
b3wHumanSlot* b3wGetHuman(int handle);
b3wHeightFieldSlot* b3wGetHeightField(int handle);

int b3wAllocWorldSlot(b3WorldId worldId);
int b3wAllocHullSlot(b3HullData* hull);
int b3wAllocMeshSlot(int worldHandle, b3MeshData* mesh);
int b3wAllocCompoundSlot(b3CompoundData* compound);
int b3wAllocHumanSlot(int worldHandle, Human human);
int b3wAllocHeightFieldSlot(int worldHandle, b3HeightFieldData* heightField);

void b3wFreeWorldSlot(int handle);
void b3wFreeHullSlot(int handle);
void b3wFreeMeshSlot(int handle);
void b3wFreeCompoundSlot(int handle);
void b3wFreeHumanSlot(int handle);
void b3wFreeHeightFieldSlot(int handle);

void b3wClearWorldSlots(int worldHandle);

void b3wGetSlotLimits(int* outLimits);
void b3wGetSlotUsage(int* outUsage);

/** Debug draw color for a native body id (shared by dense gather + move events). */
b3HexColor b3wGetBodyDebugColorForId(b3BodyId bodyId);
