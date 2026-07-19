#include "box3d_web_shared.h"

#include <stdlib.h>

b3wWorldSlot g_worlds[B3W_MAX_WORLDS];
b3wBodySlot g_bodies[B3W_MAX_BODIES];
b3wJointSlot g_joints[B3W_MAX_JOINTS];
b3wHullSlot g_hulls[B3W_MAX_HULLS];
b3wShapeSlot g_shapes[B3W_MAX_SHAPES];
b3wMeshSlot g_meshes[B3W_MAX_MESHES];
b3wCompoundSlot g_compounds[B3W_MAX_COMPOUNDS];
b3wHumanSlot g_humans[B3W_MAX_HUMANS];

static int g_worldFreeHead = B3W_SLOT_FREE_NONE;
static int g_bodyFreeHead = B3W_SLOT_FREE_NONE;
static int g_jointFreeHead = B3W_SLOT_FREE_NONE;
static int g_hullFreeHead = B3W_SLOT_FREE_NONE;
static int g_shapeFreeHead = B3W_SLOT_FREE_NONE;
static int g_meshFreeHead = B3W_SLOT_FREE_NONE;
static int g_compoundFreeHead = B3W_SLOT_FREE_NONE;
static int g_humanFreeHead = B3W_SLOT_FREE_NONE;

static int g_worldActiveCount = 0;
static int g_bodyActiveCount = 0;
static int g_jointActiveCount = 0;
static int g_hullActiveCount = 0;
static int g_shapeActiveCount = 0;
static int g_meshActiveCount = 0;
static int g_compoundActiveCount = 0;
static int g_humanActiveCount = 0;

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

	for (int i = 0; i < B3W_MAX_BODIES; ++i)
	{
		g_bodies[i].active = false;
		g_bodies[i].nextFree = (i + 1 < B3W_MAX_BODIES) ? (i + 1) : B3W_SLOT_FREE_NONE;
		g_bodies[i].worldHandle = 0;
	}
	g_bodyFreeHead = 0;
	g_bodyActiveCount = 0;

	for (int i = 0; i < B3W_MAX_JOINTS; ++i)
	{
		g_joints[i].active = false;
		g_joints[i].nextFree = (i + 1 < B3W_MAX_JOINTS) ? (i + 1) : B3W_SLOT_FREE_NONE;
		g_joints[i].worldHandle = 0;
	}
	g_jointFreeHead = 0;
	g_jointActiveCount = 0;

	for (int i = 0; i < B3W_MAX_HULLS; ++i)
	{
		g_hulls[i].active = false;
		g_hulls[i].nextFree = (i + 1 < B3W_MAX_HULLS) ? (i + 1) : B3W_SLOT_FREE_NONE;
		g_hulls[i].hull = NULL;
	}
	g_hullFreeHead = 0;
	g_hullActiveCount = 0;

	for (int i = 0; i < B3W_MAX_SHAPES; ++i)
	{
		g_shapes[i].active = false;
		g_shapes[i].nextFree = (i + 1 < B3W_MAX_SHAPES) ? (i + 1) : B3W_SLOT_FREE_NONE;
		g_shapes[i].worldHandle = 0;
	}
	g_shapeFreeHead = 0;
	g_shapeActiveCount = 0;

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

b3wBodySlot* b3wGetBody(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_BODIES) return NULL;
	b3wBodySlot* slot = &g_bodies[handle - 1];
	return slot->active ? slot : NULL;
}

b3wHullSlot* b3wGetHull(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_HULLS) return NULL;
	b3wHullSlot* slot = &g_hulls[handle - 1];
	return slot->active ? slot : NULL;
}

b3wShapeSlot* b3wGetShape(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_SHAPES) return NULL;
	b3wShapeSlot* slot = &g_shapes[handle - 1];
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

void b3wFreeBodySlot(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_BODIES) return;
	b3wBodySlot* slot = &g_bodies[handle - 1];
	if (!slot->active) return;
	slot->active = false;
	slot->worldHandle = 0;
	slot->nextFree = g_bodyFreeHead;
	g_bodyFreeHead = handle - 1;
	g_bodyActiveCount -= 1;
}

void b3wFreeJointSlot(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_JOINTS) return;
	b3wJointSlot* slot = &g_joints[handle - 1];
	if (!slot->active) return;
	slot->active = false;
	slot->worldHandle = 0;
	slot->nextFree = g_jointFreeHead;
	g_jointFreeHead = handle - 1;
	g_jointActiveCount -= 1;
}

void b3wFreeHullSlot(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_HULLS) return;
	b3wHullSlot* slot = &g_hulls[handle - 1];
	if (!slot->active) return;
	slot->active = false;
	slot->hull = NULL;
	slot->nextFree = g_hullFreeHead;
	g_hullFreeHead = handle - 1;
	g_hullActiveCount -= 1;
}

void b3wFreeShapeSlot(int handle)
{
	b3wInitPools();
	if (handle <= 0 || handle > B3W_MAX_SHAPES) return;
	b3wShapeSlot* slot = &g_shapes[handle - 1];
	if (!slot->active) return;
	slot->active = false;
	slot->worldHandle = 0;
	slot->nextFree = g_shapeFreeHead;
	g_shapeFreeHead = handle - 1;
	g_shapeActiveCount -= 1;
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

int b3wAllocBodySlot(int worldHandle, b3BodyId bodyId)
{
	b3wInitPools();
	if (g_bodyFreeHead == B3W_SLOT_FREE_NONE)
	{
		b3DestroyBody(bodyId);
		return 0;
	}
	int index = g_bodyFreeHead;
	g_bodyFreeHead = g_bodies[index].nextFree;
	g_bodies[index].active = true;
	g_bodies[index].nextFree = B3W_SLOT_FREE_NONE;
	g_bodies[index].worldHandle = worldHandle;
	g_bodies[index].bodyId = bodyId;
	g_bodyActiveCount += 1;
	return index + 1;
}

int b3wAllocJointSlot(int worldHandle, b3JointId jointId)
{
	b3wInitPools();
	if (g_jointFreeHead == B3W_SLOT_FREE_NONE)
	{
		b3DestroyJoint(jointId, true);
		return 0;
	}
	int index = g_jointFreeHead;
	g_jointFreeHead = g_joints[index].nextFree;
	g_joints[index].active = true;
	g_joints[index].nextFree = B3W_SLOT_FREE_NONE;
	g_joints[index].worldHandle = worldHandle;
	g_joints[index].jointId = jointId;
	g_jointActiveCount += 1;
	return index + 1;
}

int b3wAllocHullSlot(b3HullData* hull)
{
	b3wInitPools();
	if (g_hullFreeHead == B3W_SLOT_FREE_NONE)
	{
		b3DestroyHull(hull);
		return 0;
	}
	int index = g_hullFreeHead;
	g_hullFreeHead = g_hulls[index].nextFree;
	g_hulls[index].active = true;
	g_hulls[index].nextFree = B3W_SLOT_FREE_NONE;
	g_hulls[index].hull = hull;
	g_hullActiveCount += 1;
	return index + 1;
}

int b3wAllocShapeSlot(int worldHandle, b3ShapeId shapeId)
{
	b3wInitPools();
	if (g_shapeFreeHead == B3W_SLOT_FREE_NONE)
	{
		return 0;
	}
	int index = g_shapeFreeHead;
	g_shapeFreeHead = g_shapes[index].nextFree;
	g_shapes[index].active = true;
	g_shapes[index].nextFree = B3W_SLOT_FREE_NONE;
	g_shapes[index].worldHandle = worldHandle;
	g_shapes[index].shapeId = shapeId;
	g_shapeActiveCount += 1;
	return index + 1;
}

int b3wFindShapeHandle(b3ShapeId shapeId)
{
	b3wInitPools();
	for (int i = 0; i < B3W_MAX_SHAPES; ++i)
	{
		if (!g_shapes[i].active)
		{
			continue;
		}

		if (B3_ID_EQUALS(g_shapes[i].shapeId, shapeId))
		{
			return i + 1;
		}
	}

	return 0;
}

int b3wFindBodyHandle(b3BodyId bodyId)
{
	b3wInitPools();
	for (int i = 0; i < B3W_MAX_BODIES; ++i)
	{
		if (!g_bodies[i].active)
		{
			continue;
		}

		if (B3_ID_EQUALS(g_bodies[i].bodyId, bodyId))
		{
			return i + 1;
		}
	}

	return 0;
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

void b3wReleaseBodyShapeSlots(b3BodyId bodyId)
{
	int count = b3Body_GetShapeCount(bodyId);
	if (count <= 0)
	{
		return;
	}

	b3ShapeId* shapeIds = (b3ShapeId*)malloc((size_t)count * sizeof(b3ShapeId));
	if (shapeIds == NULL)
	{
		return;
	}

	int written = b3Body_GetShapes(bodyId, shapeIds, count);
	for (int i = 0; i < written; ++i)
	{
		int handle = b3wFindShapeHandle(shapeIds[i]);
		if (handle != 0)
		{
			b3wFreeShapeSlot(handle);
		}
	}
	free(shapeIds);
}

B3W_EXPORT void b3wGetSlotLimits(int* outLimits)
{
	if (outLimits == NULL)
	{
		return;
	}

	outLimits[0] = B3W_MAX_WORLDS;
	outLimits[1] = B3W_MAX_BODIES;
	outLimits[2] = B3W_MAX_JOINTS;
	outLimits[3] = B3W_MAX_HULLS;
	outLimits[4] = B3W_MAX_SHAPES;
	outLimits[5] = B3W_MAX_MESHES;
	outLimits[6] = B3W_MAX_COMPOUNDS;
	outLimits[7] = B3W_MAX_HUMANS;
}

B3W_EXPORT void b3wGetSlotUsage(int* outUsage)
{
	b3wInitPools();
	if (outUsage == NULL)
	{
		return;
	}

	outUsage[0] = g_worldActiveCount;
	outUsage[1] = g_bodyActiveCount;
	outUsage[2] = g_jointActiveCount;
	outUsage[3] = g_hullActiveCount;
	outUsage[4] = g_shapeActiveCount;
	outUsage[5] = g_meshActiveCount;
	outUsage[6] = g_compoundActiveCount;
	outUsage[7] = g_humanActiveCount;
}

void b3wClearWorldSlots(int worldHandle)
{
	b3wInitPools();

	for (int i = 0; i < B3W_MAX_SHAPES; ++i)
	{
		if (g_shapes[i].active && g_shapes[i].worldHandle == worldHandle)
		{
			b3wFreeShapeSlot(i + 1);
		}
	}

	for (int i = 0; i < B3W_MAX_BODIES; ++i)
	{
		if (g_bodies[i].active && g_bodies[i].worldHandle == worldHandle)
		{
			b3wFreeBodySlot(i + 1);
		}
	}

	for (int i = 0; i < B3W_MAX_JOINTS; ++i)
	{
		if (g_joints[i].active && g_joints[i].worldHandle == worldHandle)
		{
			b3wFreeJointSlot(i + 1);
		}
	}

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
}
