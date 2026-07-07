#include "box3d_web_shared.h"

b3wWorldSlot g_worlds[B3W_MAX_WORLDS];
b3wBodySlot g_bodies[B3W_MAX_BODIES];
b3wJointSlot g_joints[B3W_MAX_JOINTS];
b3wHullSlot g_hulls[B3W_MAX_HULLS];
b3wShapeSlot g_shapes[B3W_MAX_SHAPES];
b3wMeshSlot g_meshes[B3W_MAX_MESHES];
b3wCompoundSlot g_compounds[B3W_MAX_COMPOUNDS];
b3wHumanSlot g_humans[B3W_MAX_HUMANS];

b3wWorldSlot* b3wGetWorld(int handle)
{
	if (handle <= 0 || handle > B3W_MAX_WORLDS) return NULL;
	b3wWorldSlot* slot = &g_worlds[handle - 1];
	return slot->active ? slot : NULL;
}

b3wBodySlot* b3wGetBody(int handle)
{
	if (handle <= 0 || handle > B3W_MAX_BODIES) return NULL;
	b3wBodySlot* slot = &g_bodies[handle - 1];
	return slot->active ? slot : NULL;
}

b3wHullSlot* b3wGetHull(int handle)
{
	if (handle <= 0 || handle > B3W_MAX_HULLS) return NULL;
	b3wHullSlot* slot = &g_hulls[handle - 1];
	return slot->active ? slot : NULL;
}

b3wShapeSlot* b3wGetShape(int handle)
{
	if (handle <= 0 || handle > B3W_MAX_SHAPES) return NULL;
	b3wShapeSlot* slot = &g_shapes[handle - 1];
	return slot->active ? slot : NULL;
}

b3wMeshSlot* b3wGetMesh(int handle)
{
	if (handle <= 0 || handle > B3W_MAX_MESHES) return NULL;
	b3wMeshSlot* slot = &g_meshes[handle - 1];
	return slot->active ? slot : NULL;
}

b3wCompoundSlot* b3wGetCompound(int handle)
{
	if (handle <= 0 || handle > B3W_MAX_COMPOUNDS) return NULL;
	b3wCompoundSlot* slot = &g_compounds[handle - 1];
	return slot->active ? slot : NULL;
}

b3wHumanSlot* b3wGetHuman(int handle)
{
	if (handle <= 0 || handle > B3W_MAX_HUMANS) return NULL;
	b3wHumanSlot* slot = &g_humans[handle - 1];
	return slot->active ? slot : NULL;
}

int b3wAllocWorldSlot(b3WorldId worldId)
{
	for (int i = 0; i < B3W_MAX_WORLDS; ++i)
	{
		if (!g_worlds[i].active)
		{
			g_worlds[i].active = true;
			g_worlds[i].worldId = worldId;
			return i + 1;
		}
	}
	b3DestroyWorld(worldId);
	return 0;
}

int b3wAllocBodySlot(int worldHandle, b3BodyId bodyId)
{
	for (int i = 0; i < B3W_MAX_BODIES; ++i)
	{
		if (!g_bodies[i].active)
		{
			g_bodies[i].active = true;
			g_bodies[i].worldHandle = worldHandle;
			g_bodies[i].bodyId = bodyId;
			return i + 1;
		}
	}
	b3DestroyBody(bodyId);
	return 0;
}

int b3wAllocJointSlot(int worldHandle, b3JointId jointId)
{
	for (int i = 0; i < B3W_MAX_JOINTS; ++i)
	{
		if (!g_joints[i].active)
		{
			g_joints[i].active = true;
			g_joints[i].worldHandle = worldHandle;
			g_joints[i].jointId = jointId;
			return i + 1;
		}
	}
	b3DestroyJoint(jointId, true);
	return 0;
}

int b3wAllocHullSlot(b3HullData* hull)
{
	for (int i = 0; i < B3W_MAX_HULLS; ++i)
	{
		if (!g_hulls[i].active)
		{
			g_hulls[i].active = true;
			g_hulls[i].hull = hull;
			return i + 1;
		}
	}
	b3DestroyHull(hull);
	return 0;
}

int b3wAllocShapeSlot(b3ShapeId shapeId)
{
	for (int i = 0; i < B3W_MAX_SHAPES; ++i)
	{
		if (!g_shapes[i].active)
		{
			g_shapes[i].active = true;
			g_shapes[i].shapeId = shapeId;
			return i + 1;
		}
	}
	return 0;
}

int b3wAllocMeshSlot(int worldHandle, b3MeshData* mesh)
{
	for (int i = 0; i < B3W_MAX_MESHES; ++i)
	{
		if (!g_meshes[i].active)
		{
			g_meshes[i].active = true;
			g_meshes[i].worldHandle = worldHandle;
			g_meshes[i].mesh = mesh;
			return i + 1;
		}
	}
	b3DestroyMesh(mesh);
	return 0;
}

int b3wFindShapeHandle(b3ShapeId shapeId)
{
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

int b3wAllocCompoundSlot(b3CompoundData* compound)
{
	for (int i = 0; i < B3W_MAX_COMPOUNDS; ++i)
	{
		if (!g_compounds[i].active)
		{
			g_compounds[i].active = true;
			g_compounds[i].compound = compound;
			return i + 1;
		}
	}
	b3DestroyCompound(compound);
	return 0;
}

int b3wAllocHumanSlot(int worldHandle, Human human)
{
	for (int i = 0; i < B3W_MAX_HUMANS; ++i)
	{
		if (!g_humans[i].active)
		{
			g_humans[i].active = true;
			g_humans[i].worldHandle = worldHandle;
			g_humans[i].human = human;
			return i + 1;
		}
	}
	DestroyHuman(&human);
	return 0;
}

static int b3wCountActiveWorldSlots(void)
{
	int count = 0;
	for (int i = 0; i < B3W_MAX_WORLDS; ++i)
	{
		if (g_worlds[i].active)
		{
			count += 1;
		}
	}
	return count;
}

static int b3wCountActiveBodySlots(void)
{
	int count = 0;
	for (int i = 0; i < B3W_MAX_BODIES; ++i)
	{
		if (g_bodies[i].active)
		{
			count += 1;
		}
	}
	return count;
}

static int b3wCountActiveJointSlots(void)
{
	int count = 0;
	for (int i = 0; i < B3W_MAX_JOINTS; ++i)
	{
		if (g_joints[i].active)
		{
			count += 1;
		}
	}
	return count;
}

static int b3wCountActiveHullSlots(void)
{
	int count = 0;
	for (int i = 0; i < B3W_MAX_HULLS; ++i)
	{
		if (g_hulls[i].active)
		{
			count += 1;
		}
	}
	return count;
}

static int b3wCountActiveShapeSlots(void)
{
	int count = 0;
	for (int i = 0; i < B3W_MAX_SHAPES; ++i)
	{
		if (g_shapes[i].active)
		{
			count += 1;
		}
	}
	return count;
}

static int b3wCountActiveMeshSlots(void)
{
	int count = 0;
	for (int i = 0; i < B3W_MAX_MESHES; ++i)
	{
		if (g_meshes[i].active)
		{
			count += 1;
		}
	}
	return count;
}

static int b3wCountActiveCompoundSlots(void)
{
	int count = 0;
	for (int i = 0; i < B3W_MAX_COMPOUNDS; ++i)
	{
		if (g_compounds[i].active)
		{
			count += 1;
		}
	}
	return count;
}

static int b3wCountActiveHumanSlots(void)
{
	int count = 0;
	for (int i = 0; i < B3W_MAX_HUMANS; ++i)
	{
		if (g_humans[i].active)
		{
			count += 1;
		}
	}
	return count;
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
	if (outUsage == NULL)
	{
		return;
	}

	outUsage[0] = b3wCountActiveWorldSlots();
	outUsage[1] = b3wCountActiveBodySlots();
	outUsage[2] = b3wCountActiveJointSlots();
	outUsage[3] = b3wCountActiveHullSlots();
	outUsage[4] = b3wCountActiveShapeSlots();
	outUsage[5] = b3wCountActiveMeshSlots();
	outUsage[6] = b3wCountActiveCompoundSlots();
	outUsage[7] = b3wCountActiveHumanSlots();
}

void b3wClearWorldSlots(int worldHandle)
{
	for (int i = 0; i < B3W_MAX_BODIES; ++i)
	{
		if (g_bodies[i].active && g_bodies[i].worldHandle == worldHandle)
		{
			g_bodies[i].active = false;
		}
	}

	for (int i = 0; i < B3W_MAX_JOINTS; ++i)
	{
		if (g_joints[i].active && g_joints[i].worldHandle == worldHandle)
		{
			g_joints[i].active = false;
		}
	}

	for (int i = 0; i < B3W_MAX_HUMANS; ++i)
	{
		if (g_humans[i].active && g_humans[i].worldHandle == worldHandle)
		{
			g_humans[i].active = false;
		}
	}

	for (int i = 0; i < B3W_MAX_MESHES; ++i)
	{
		if (g_meshes[i].active && g_meshes[i].worldHandle == worldHandle)
		{
			b3DestroyMesh(g_meshes[i].mesh);
			g_meshes[i].active = false;
			g_meshes[i].worldHandle = 0;
			g_meshes[i].mesh = NULL;
		}
	}
}
