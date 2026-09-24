#include "both_ids.h"
#include "growable_slots.h"

#define WORLD_CAP GPU_METADATA_WORLDS
static b3WorldId g_world[WORLD_CAP];
static b3WorldId g_gpu_world[WORLD_CAP];

static int world_slot(b3WorldId id)
{
    return (id.index1 > 0 && id.index1 < WORLD_CAP) ? (int)id.index1 : 0;
}

void both_map_world(b3WorldId gpu, b3WorldId cpu)
{
    int i = world_slot(gpu);
    if (i) { g_world[i] = cpu; g_gpu_world[i] = gpu; }
}

bool both_has_cpu_world(b3WorldId gpu)
{
    int i = world_slot(gpu);
    return i && g_gpu_world[i].index1 && g_gpu_world[i].generation == gpu.generation;
}

b3WorldId both_cpu_world(b3WorldId gpu)
{
    return both_has_cpu_world(gpu) ? g_world[gpu.index1] : gpu;
}

typedef struct { b3BodyId gpu, cpu; } BodyMap;
static GpuSlots g_body[WORLD_CAP];

static BodyMap* body_slot(b3BodyId gpu, bool create)
{
    if (gpu.world0 == 0 || gpu.world0 >= WORLD_CAP) return NULL;
    return (BodyMap*)gpu_slots_get(&g_body[gpu.world0], gpu.index1, sizeof(BodyMap), create);
}

void both_map_body(b3BodyId gpu, b3BodyId cpu)
{
    BodyMap* slot = body_slot(gpu, true);
    if (slot) { slot->gpu = gpu; slot->cpu = cpu; }
}

bool both_has_cpu_body(b3BodyId gpu)
{
    BodyMap* slot = body_slot(gpu, false);
    return slot && slot->gpu.index1 == gpu.index1 && slot->gpu.generation == gpu.generation;
}

b3BodyId both_cpu_body(b3BodyId gpu)
{
    return both_has_cpu_body(gpu) ? body_slot(gpu, false)->cpu : gpu;
}

void both_unmap_body(b3BodyId gpu)
{
    if (both_has_cpu_body(gpu)) memset(body_slot(gpu, false), 0, sizeof(BodyMap));
}

typedef struct { b3ShapeId gpu, cpu; } ShapeMap;
static GpuSlots g_shape[WORLD_CAP];

static ShapeMap* shape_slot(b3ShapeId gpu, bool create)
{
    if (gpu.world0 == 0 || gpu.world0 >= WORLD_CAP) return NULL;
    return (ShapeMap*)gpu_slots_get(&g_shape[gpu.world0], gpu.index1, sizeof(ShapeMap), create);
}

void both_map_shape(b3ShapeId gpu, b3ShapeId cpu)
{
    ShapeMap* slot = shape_slot(gpu, true);
    if (slot) { slot->gpu = gpu; slot->cpu = cpu; }
}

bool both_has_cpu_shape(b3ShapeId gpu)
{
    ShapeMap* slot = shape_slot(gpu, false);
    return slot && slot->gpu.index1 == gpu.index1 && slot->gpu.generation == gpu.generation;
}

b3ShapeId both_cpu_shape(b3ShapeId gpu)
{
    return both_has_cpu_shape(gpu) ? shape_slot(gpu, false)->cpu : gpu;
}

void both_unmap_shape(b3ShapeId gpu)
{
    if (both_has_cpu_shape(gpu)) memset(shape_slot(gpu, false), 0, sizeof(ShapeMap));
}

typedef struct { b3JointId gpu, cpu; } JointMap;
static GpuSlots g_joint[WORLD_CAP];

static JointMap* joint_slot(b3JointId gpu, bool create)
{
    if (gpu.world0 == 0 || gpu.world0 >= WORLD_CAP) return NULL;
    return (JointMap*)gpu_slots_get(&g_joint[gpu.world0], gpu.index1, sizeof(JointMap), create);
}

void both_map_joint(b3JointId gpu, b3JointId cpu)
{
    JointMap* slot = joint_slot(gpu, true);
    if (slot) { slot->gpu = gpu; slot->cpu = cpu; }
}

bool both_has_cpu_joint(b3JointId gpu)
{
    JointMap* slot = joint_slot(gpu, false);
    return slot && slot->gpu.index1 == gpu.index1 && slot->gpu.generation == gpu.generation;
}

b3JointId both_cpu_joint(b3JointId gpu)
{
    return both_has_cpu_joint(gpu) ? joint_slot(gpu, false)->cpu : gpu;
}

void both_unmap_joint(b3JointId gpu)
{
    if (both_has_cpu_joint(gpu)) memset(joint_slot(gpu, false), 0, sizeof(JointMap));
}

void both_unmap_world(b3WorldId gpu)
{
    if (!both_has_cpu_world(gpu)) return;
    int i = world_slot(gpu);
    gpu_slots_release(&g_body[i]);
    gpu_slots_release(&g_shape[i]);
    gpu_slots_release(&g_joint[i]);
    memset(&g_world[i], 0, sizeof(g_world[i]));
    memset(&g_gpu_world[i], 0, sizeof(g_gpu_world[i]));
}

void both_clear_maps(void)
{
    for (unsigned i = 0; i < WORLD_CAP; ++i)
    {
        gpu_slots_release(&g_body[i]);
        gpu_slots_release(&g_shape[i]);
        gpu_slots_release(&g_joint[i]);
    }
    memset(g_world, 0, sizeof(g_world));
    memset(g_gpu_world, 0, sizeof(g_gpu_world));
}
