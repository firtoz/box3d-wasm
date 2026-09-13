#ifndef GPU_COMPOUND_MESH_INSTANCES_H
#define GPU_COMPOUND_MESH_INSTANCES_H
#include <stdint.h>
#include <stdlib.h>

#ifndef GPU_MESH_CACHE_CALLOC
#define GPU_MESH_CACHE_CALLOC(count, size) calloc(count, size)
#endif

extern GpuShapeId gpu_b3_create_mesh_instance(GpuBodyId, GpuShapeId,
    const float*, const float*, const float*, float, float, float, float);

typedef struct GpuMeshCacheEntry {
    const b3MeshData* key;
    GpuShapeId source;
} GpuMeshCacheEntry;
typedef struct GpuMeshCache {
    GpuMeshCacheEntry* entries;
    size_t capacity;
} GpuMeshCache;

/* Borrow native pointers only within this import. The source shapes own copies
   of raw geometry; every instance retains those arrays after cache cleanup. */
static bool gpu_mesh_cache_init(GpuMeshCache* cache, int count)
{
    *cache = (GpuMeshCache){0};
    if (count < 0 || count > 65536) return false;
    if (count == 0) return true;
    size_t capacity = 2;
    while (capacity < (size_t)count * 2) capacity *= 2;
    cache->entries = GPU_MESH_CACHE_CALLOC(capacity, sizeof(GpuMeshCacheEntry));
    if (!cache->entries) return false;
    cache->capacity = capacity;
    return true;
}
static void gpu_mesh_cache_free(GpuMeshCache* cache)
{
    for (size_t i = 0; i < cache->capacity; ++i)
        if (cache->entries[i].source.index1 > 0)
            gpu_b3_destroy_shape(cache->entries[i].source, false);
    free(cache->entries);
    *cache = (GpuMeshCache){0};
}
static GpuShapeId gpu_mesh_cache_instance(GpuMeshCache* cache, GpuBodyId body,
    const b3ChildShape* child, const b3ShapeDef* def)
{
    const b3MeshData* mesh = child->mesh.data;
    if (!mesh || !cache->capacity || mesh->vertexCount <= 0 || mesh->triangleCount <= 0)
        return (GpuShapeId){0};
    size_t slot = ((uintptr_t)mesh >> 4) * (size_t)2654435761u;
    for (size_t probe = 0; probe < cache->capacity; ++probe, ++slot) {
        GpuMeshCacheEntry* entry = &cache->entries[slot & (cache->capacity - 1)];
        if (entry->key && entry->key != mesh) continue;
        if (!entry->key) {
            entry->key = mesh;
            entry->source = gpu_b3_create_mesh(body, &b3GetMeshVertices(mesh)[0].x,
                mesh->vertexCount, &b3GetMeshTriangles(mesh)[0].index1,
                mesh->triangleCount, b3GetMeshFlags(mesh), b3GetMeshMaterialIndices(mesh),
                NULL, 0, 1, 1, 1, 0, 0, 0, 0, false);
        }
        if (entry->source.index1 <= 0) return (GpuShapeId){0};
        return gpu_b3_create_mesh_instance(body, entry->source,
            &child->transform.p.x, &child->transform.q.v.x, &child->mesh.scale.x,
            def->density, def->baseMaterial.friction, def->baseMaterial.restitution,
            def->baseMaterial.rollingResistance);
    }
    return (GpuShapeId){0};
}
#endif
