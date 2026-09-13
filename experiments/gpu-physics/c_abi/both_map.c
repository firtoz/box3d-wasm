#include "both_ids.h"

#include <string.h>

#define WORLD_CAP 64
#define ID_CAP 65536

static b3WorldId g_world[WORLD_CAP];
static uint8_t g_world_used[WORLD_CAP];
static b3BodyId g_body[ID_CAP];
static uint8_t g_body_used[ID_CAP];
static b3ShapeId g_shape[ID_CAP];
static uint8_t g_shape_used[ID_CAP];
static b3JointId g_joint[ID_CAP];
static uint8_t g_joint_used[ID_CAP];

static int world_slot(b3WorldId id)
{
	return (id.index1 > 0 && id.index1 < WORLD_CAP) ? (int)id.index1 : 0;
}

static int id_slot(int32_t index1)
{
	return (index1 > 0 && index1 < ID_CAP) ? index1 : 0;
}

void both_map_world(b3WorldId gpu, b3WorldId cpu)
{
	int i = world_slot(gpu);
	if (i == 0)
	{
		return;
	}
	g_world[i] = cpu;
	g_world_used[i] = 1;
}

void both_map_body(b3BodyId gpu, b3BodyId cpu)
{
	int i = id_slot(gpu.index1);
	if (i == 0)
	{
		return;
	}
	g_body[i] = cpu;
	g_body_used[i] = 1;
}

void both_map_shape(b3ShapeId gpu, b3ShapeId cpu)
{
	int i = id_slot(gpu.index1);
	if (i == 0)
	{
		return;
	}
	g_shape[i] = cpu;
	g_shape_used[i] = 1;
}

void both_map_joint(b3JointId gpu, b3JointId cpu)
{
	int i = id_slot(gpu.index1);
	if (i == 0)
	{
		return;
	}
	g_joint[i] = cpu;
	g_joint_used[i] = 1;
}

void both_unmap_world(b3WorldId gpu)
{
	int i = world_slot(gpu);
	if (i == 0)
	{
		return;
	}
	g_world_used[i] = 0;
	memset(&g_world[i], 0, sizeof(g_world[i]));
}

void both_unmap_body(b3BodyId gpu)
{
	int i = id_slot(gpu.index1);
	if (i == 0)
	{
		return;
	}
	g_body_used[i] = 0;
	memset(&g_body[i], 0, sizeof(g_body[i]));
}

void both_unmap_shape(b3ShapeId gpu)
{
	int i = id_slot(gpu.index1);
	if (i == 0)
	{
		return;
	}
	g_shape_used[i] = 0;
	memset(&g_shape[i], 0, sizeof(g_shape[i]));
}

void both_unmap_joint(b3JointId gpu)
{
	int i = id_slot(gpu.index1);
	if (i == 0)
	{
		return;
	}
	g_joint_used[i] = 0;
	memset(&g_joint[i], 0, sizeof(g_joint[i]));
}

b3WorldId both_cpu_world(b3WorldId gpu)
{
	int i = world_slot(gpu);
	if (i != 0 && g_world_used[i])
	{
		return g_world[i];
	}
	return gpu;
}

b3BodyId both_cpu_body(b3BodyId gpu)
{
	int i = id_slot(gpu.index1);
	if (i != 0 && g_body_used[i])
	{
		return g_body[i];
	}
	return gpu;
}

b3ShapeId both_cpu_shape(b3ShapeId gpu)
{
	int i = id_slot(gpu.index1);
	if (i != 0 && g_shape_used[i])
	{
		return g_shape[i];
	}
	return gpu;
}

b3JointId both_cpu_joint(b3JointId gpu)
{
	int i = id_slot(gpu.index1);
	if (i != 0 && g_joint_used[i])
	{
		return g_joint[i];
	}
	return gpu;
}

bool both_has_cpu_world(b3WorldId gpu)
{
	int i = world_slot(gpu);
	return i != 0 && g_world_used[i];
}

bool both_has_cpu_body(b3BodyId gpu)
{
	int i = id_slot(gpu.index1);
	return i != 0 && g_body_used[i];
}

bool both_has_cpu_shape(b3ShapeId gpu)
{
	int i = id_slot(gpu.index1);
	return i != 0 && g_shape_used[i];
}

bool both_has_cpu_joint(b3JointId gpu)
{
	int i = id_slot(gpu.index1);
	return i != 0 && g_joint_used[i];
}

void both_clear_maps(void)
{
	memset(g_world_used, 0, sizeof(g_world_used));
	memset(g_body_used, 0, sizeof(g_body_used));
	memset(g_shape_used, 0, sizeof(g_shape_used));
	memset(g_joint_used, 0, sizeof(g_joint_used));
}
