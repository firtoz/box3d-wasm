#include "box3d_web_shared.h"

b3wWorldSlot g_worlds[B3W_MAX_WORLDS];
b3wHullSlot g_hulls[B3W_MAX_HULLS];
b3wMeshSlot g_meshes[B3W_MAX_MESHES];
b3wCompoundSlot g_compounds[B3W_MAX_COMPOUNDS];
b3wHumanSlot g_humans[B3W_MAX_HUMANS];
b3wHeightFieldSlot g_heightFields[B3W_MAX_HEIGHT_FIELDS];

static int g_worldFreeHead = B3W_SLOT_FREE_NONE;
static int g_hullFreeHead = B3W_SLOT_FREE_NONE;
static int g_meshFreeHead = B3W_SLOT_FREE_NONE;
static int g_compoundFreeHead = B3W_SLOT_FREE_NONE;
static int g_humanFreeHead = B3W_SLOT_FREE_NONE;
static int g_heightFieldFreeHead = B3W_SLOT_FREE_NONE;

static int g_worldActiveCount = 0;
static int g_hullActiveCount = 0;
static int g_meshActiveCount = 0;
static int g_compoundActiveCount = 0;
static int g_humanActiveCount = 0;
static int g_heightFieldActiveCount = 0;

static bool g_poolsReady = false;

static void b3wInitPools(void)
{
	if (g_poolsReady)
	{
		return;
	}

	for (int i = 0; i < B3W_MAX_WORLDS; ++i)
	{
		g_worlds[i].active = false;
		g_worlds[i].nextFree = (i + 1 < B3W_MAX_WORLDS) ? (i + 1) : B3W_SLOT_FREE_NONE;
	}
	g_worldFreeHead = 0;
	g_worldActiveCount = 0;

	for (int i = 0; i < B3W_MAX_HULLS; ++i)
	{
		g_hulls[i].active = false;
		g_hulls[i].nextFree = (i + 1 < B3W_MAX_HULLS) ? (i + 1) : B3W_SLOT_FREE_NONE;
		g_hulls[i].hull = NULL;
		g_hulls[i].boxStorage = NULL;
	}
	g_hullFreeHead = 0;
	g_hullActiveCount = 0;

	for (int i = 0; i < B3W_MAX_MESHES; ++i)
	{
		g_meshes[i].active = false;
		g_meshes[i].nextFree = (i + 1 < B3W_MAX_MESHES) ? (i + 1) : B3W_SLOT_FREE_NONE;
		g_meshes[i].worldHandle = 0;
		g_meshes[i].mesh = NULL;
	}
	g_meshFreeHead = 0;
	g_meshActiveCount = 0;

	for (int i = 0; i < B3W_MAX_COMPOUNDS; ++i)
	{
		g_compounds[i].active = false;
		g_compounds[i].nextFree = (i + 1 < B3W_MAX_COMPOUNDS) ? (i + 1) : B3W_SLOT_FREE_NONE;
		g_compounds[i].compound = NULL;
	}
	g_compoundFreeHead = 0;
	g_compoundActiveCount = 0;

	for (int i = 0; i < B3W_MAX_HUMANS; ++i)
	{
		g_humans[i].active = false;
		g_humans[i].nextFree = (i + 1 < B3W_MAX_HUMANS) ? (i + 1) : B3W_SLOT_FREE_NONE;
		g_humans[i].worldHandle = 0;
	}
	g_humanFreeHead = 0;
	g_humanActiveCount = 0;

	for (int i = 0; i < B3W_MAX_HEIGHT_FIELDS; ++i)
	{
		g_heightFields[i].active = false;
		g_heightFields[i].nextFree = (i + 1 < B3W_MAX_HEIGHT_FIELDS) ? (i + 1) : B3W_SLOT_FREE_NONE;
		g_heightFields[i].worldHandle = 0;
		g_heightFields[i].heightField = NULL;
	}
	g_heightFieldFreeHead = 0;
	g_heightFieldActiveCount = 0;

	g_poolsReady = true;
}

#ifdef __GNUC__
__attribute__((constructor))
#endif
static void b3wPoolsConstructor(void)
{
	b3wInitPools();
}

b3wWorldSlot* b3wGetWorld(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_WORLDS) return NULL;
	b3wWorldSlot* slot = &g_worlds[handle - 1];
	return slot->active ? slot : NULL;
}

b3wHullSlot* b3wGetHull(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_HULLS) return NULL;
	b3wHullSlot* slot = &g_hulls[handle - 1];
	return slot->active ? slot : NULL;
}

b3wMeshSlot* b3wGetMesh(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_MESHES) return NULL;
	b3wMeshSlot* slot = &g_meshes[handle - 1];
	return slot->active ? slot : NULL;
}

b3wCompoundSlot* b3wGetCompound(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_COMPOUNDS) return NULL;
	b3wCompoundSlot* slot = &g_compounds[handle - 1];
	return slot->active ? slot : NULL;
}

b3wHumanSlot* b3wGetHuman(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_HUMANS) return NULL;
	b3wHumanSlot* slot = &g_humans[handle - 1];
	return slot->active ? slot : NULL;
}

b3wHeightFieldSlot* b3wGetHeightField(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_HEIGHT_FIELDS) return NULL;
	b3wHeightFieldSlot* slot = &g_heightFields[handle - 1];
	return slot->active ? slot : NULL;
}

void b3wFreeWorldSlot(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_WORLDS) return;
	b3wWorldSlot* slot = &g_worlds[handle - 1];
	if (!slot->active) return;
	slot->active = false;
	slot->nextFree = g_worldFreeHead;
	g_worldFreeHead = handle - 1;
	g_worldActiveCount -= 1;
}

void b3wFreeHullSlot(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_HULLS) return;
	b3wHullSlot* slot = &g_hulls[handle - 1];
	if (!slot->active) return;
	slot->active = false;
	slot->hull = NULL;
	slot->boxStorage = NULL;
	slot->nextFree = g_hullFreeHead;
	g_hullFreeHead = handle - 1;
	g_hullActiveCount -= 1;
}

void b3wFreeMeshSlot(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_MESHES) return;
	b3wMeshSlot* slot = &g_meshes[handle - 1];
	if (!slot->active) return;
	slot->active = false;
	slot->worldHandle = 0;
	slot->mesh = NULL;
	slot->nextFree = g_meshFreeHead;
	g_meshFreeHead = handle - 1;
	g_meshActiveCount -= 1;
}

void b3wFreeCompoundSlot(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_COMPOUNDS) return;
	b3wCompoundSlot* slot = &g_compounds[handle - 1];
	if (!slot->active) return;
	slot->active = false;
	slot->compound = NULL;
	slot->nextFree = g_compoundFreeHead;
	g_compoundFreeHead = handle - 1;
	g_compoundActiveCount -= 1;
}

void b3wFreeHumanSlot(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_HUMANS) return;
	b3wHumanSlot* slot = &g_humans[handle - 1];
	if (!slot->active) return;
	slot->active = false;
	slot->worldHandle = 0;
	slot->nextFree = g_humanFreeHead;
	g_humanFreeHead = handle - 1;
	g_humanActiveCount -= 1;
}

void b3wFreeHeightFieldSlot(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_HEIGHT_FIELDS) return;
	b3wHeightFieldSlot* slot = &g_heightFields[handle - 1];
	if (!slot->active) return;
	slot->active = false;
	slot->worldHandle = 0;
	slot->heightField = NULL;
	slot->nextFree = g_heightFieldFreeHead;
	g_heightFieldFreeHead = handle - 1;
	g_heightFieldActiveCount -= 1;
}

int b3wAllocWorldSlot(b3WorldId worldId)
{
	b3wInitPools();
	if (g_worldFreeHead == B3W_SLOT_FREE_NONE)
	{
		b3DestroyWorld(worldId);
		return 0;
	}
	int index = g_worldFreeHead;
	g_worldFreeHead = g_worlds[index].nextFree;
	g_worlds[index].active = true;
	g_worlds[index].nextFree = B3W_SLOT_FREE_NONE;
	g_worlds[index].worldId = worldId;
	g_worldActiveCount += 1;
	return index + 1;
}

int b3wTryAllocHullSlot(b3HullData* hull)
{
	b3wInitPools();
	if (g_hullFreeHead == B3W_SLOT_FREE_NONE)
	{
		return 0;
	}
	int index = g_hullFreeHead;
	g_hullFreeHead = g_hulls[index].nextFree;
	g_hulls[index].active = true;
	g_hulls[index].nextFree = B3W_SLOT_FREE_NONE;
	g_hulls[index].hull = hull;
	g_hulls[index].boxStorage = NULL;
	g_hullActiveCount += 1;
	return index + 1;
}

int b3wAllocHullSlot(b3HullData* hull)
{
	int handle = b3wTryAllocHullSlot(hull);
	if (handle == 0)
	{
		b3DestroyHull(hull);
		return 0;
	}
	return handle;
}

int b3wAllocMeshSlot(int worldHandle, b3MeshData* mesh)
{
	b3wInitPools();
	if (g_meshFreeHead == B3W_SLOT_FREE_NONE)
	{
		b3DestroyMesh(mesh);
		return 0;
	}
	int index = g_meshFreeHead;
	g_meshFreeHead = g_meshes[index].nextFree;
	g_meshes[index].active = true;
	g_meshes[index].nextFree = B3W_SLOT_FREE_NONE;
	g_meshes[index].worldHandle = worldHandle;
	g_meshes[index].mesh = mesh;
	g_meshActiveCount += 1;
	return index + 1;
}

int b3wAllocCompoundSlot(b3CompoundData* compound)
{
	b3wInitPools();
	if (g_compoundFreeHead == B3W_SLOT_FREE_NONE)
	{
		b3DestroyCompound(compound);
		return 0;
	}
	int index = g_compoundFreeHead;
	g_compoundFreeHead = g_compounds[index].nextFree;
	g_compounds[index].active = true;
	g_compounds[index].nextFree = B3W_SLOT_FREE_NONE;
	g_compounds[index].compound = compound;
	g_compoundActiveCount += 1;
	return index + 1;
}

int b3wAllocHumanSlot(int worldHandle, Human human)
{
	b3wInitPools();
	if (g_humanFreeHead == B3W_SLOT_FREE_NONE)
	{
		DestroyHuman(&human);
		return 0;
	}
	int index = g_humanFreeHead;
	g_humanFreeHead = g_humans[index].nextFree;
	g_humans[index].active = true;
	g_humans[index].nextFree = B3W_SLOT_FREE_NONE;
	g_humans[index].worldHandle = worldHandle;
	g_humans[index].human = human;
	g_humanActiveCount += 1;
	return index + 1;
}

int b3wAllocHeightFieldSlot(int worldHandle, b3HeightFieldData* heightField)
{
	b3wInitPools();
	if (g_heightFieldFreeHead == B3W_SLOT_FREE_NONE)
	{
		b3DestroyHeightField(heightField);
		return 0;
	}
	int index = g_heightFieldFreeHead;
	g_heightFieldFreeHead = g_heightFields[index].nextFree;
	g_heightFields[index].active = true;
	g_heightFields[index].nextFree = B3W_SLOT_FREE_NONE;
	g_heightFields[index].worldHandle = worldHandle;
	g_heightFields[index].heightField = heightField;
	g_heightFieldActiveCount += 1;
	return index + 1;
}

B3W_EXPORT void b3wGetSlotLimits(int* outLimits)
{
	if (outLimits == NULL)
	{
		return;
	}

	// Order: worlds, hulls, meshes, compounds, humans, heightFields
	outLimits[0] = B3W_MAX_WORLDS;
	outLimits[1] = B3W_MAX_HULLS;
	outLimits[2] = B3W_MAX_MESHES;
	outLimits[3] = B3W_MAX_COMPOUNDS;
	outLimits[4] = B3W_MAX_HUMANS;
	outLimits[5] = B3W_MAX_HEIGHT_FIELDS;
}

B3W_EXPORT void b3wGetSlotUsage(int* outUsage)
{
	b3wInitPools();
	if (outUsage == NULL)
	{
		return;
	}

	// Order: worlds, hulls, meshes, compounds, humans, heightFields
	outUsage[0] = g_worldActiveCount;
	outUsage[1] = g_hullActiveCount;
	outUsage[2] = g_meshActiveCount;
	outUsage[3] = g_compoundActiveCount;
	outUsage[4] = g_humanActiveCount;
	outUsage[5] = g_heightFieldActiveCount;
}

void b3wClearWorldSlots(int worldHandle)
{
	b3wInitPools();

	for (int i = 0; i < B3W_MAX_HUMANS; ++i)
	{
		if (g_humans[i].active && g_humans[i].worldHandle == worldHandle)
		{
			b3wFreeHumanSlot(i + 1);
		}
	}

	for (int i = 0; i < B3W_MAX_MESHES; ++i)
	{
		if (g_meshes[i].active && g_meshes[i].worldHandle == worldHandle)
		{
			b3DestroyMesh(g_meshes[i].mesh);
			b3wFreeMeshSlot(i + 1);
		}
	}

	for (int i = 0; i < B3W_MAX_HEIGHT_FIELDS; ++i)
	{
		if (g_heightFields[i].active && g_heightFields[i].worldHandle == worldHandle)
		{
			b3DestroyHeightField(g_heightFields[i].heightField);
			b3wFreeHeightFieldSlot(i + 1);
		}
	}
}
