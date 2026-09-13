#ifndef GPU_COMPOUND_MESH_BAKE_H
#define GPU_COMPOUND_MESH_BAKE_H

#include "box3d/collision.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef GPU_COMPOUND_ALLOC
#define GPU_COMPOUND_ALLOC(size) malloc(size)
#endif

/* Shared by GPU-only and combined imports. The input remains borrowed; all
   owned arrays are released together on success or failure. */
typedef struct GpuCompoundMeshBake
{
    b3Vec3* vertices;
    const b3MeshTriangle* triangles;
    const uint8_t* flags;
    b3MeshTriangle* flipped;
    uint8_t* flippedFlags;
} GpuCompoundMeshBake;

static void gpu_compound_mesh_bake_free(GpuCompoundMeshBake* bake)
{
    free(bake->vertices);
    free(bake->flipped);
    free(bake->flippedFlags);
    memset(bake, 0, sizeof(*bake));
}

static bool gpu_compound_mesh_bake(const b3ChildShape* child, GpuCompoundMeshBake* bake)
{
    memset(bake, 0, sizeof(*bake));
    const b3MeshData* mesh = child->mesh.data;
    if (mesh == NULL || mesh->vertexCount <= 0 || mesh->triangleCount <= 0 ||
        (size_t)mesh->vertexCount > SIZE_MAX / sizeof(b3Vec3) ||
        (size_t)mesh->triangleCount > SIZE_MAX / sizeof(b3MeshTriangle))
    {
        return false;
    }
    bake->vertices = GPU_COMPOUND_ALLOC((size_t)mesh->vertexCount * sizeof(b3Vec3));
    if (bake->vertices == NULL) { return false; }
    const b3Vec3* source = b3GetMeshVertices(mesh);
    for (int v = 0; v < mesh->vertexCount; ++v)
    {
        b3Vec3 scaled = {source[v].x * child->mesh.scale.x, source[v].y * child->mesh.scale.y,
                        source[v].z * child->mesh.scale.z};
        bake->vertices[v] = b3TransformPoint(child->transform, scaled);
    }
    bake->triangles = b3GetMeshTriangles(mesh);
    bake->flags = b3GetMeshFlags(mesh);
    if (child->mesh.scale.x * child->mesh.scale.y * child->mesh.scale.z < 0.0f)
    {
        bake->flipped = GPU_COMPOUND_ALLOC((size_t)mesh->triangleCount * sizeof(b3MeshTriangle));
        bake->flippedFlags = GPU_COMPOUND_ALLOC((size_t)mesh->triangleCount);
        if (bake->flipped == NULL || bake->flippedFlags == NULL)
        {
            gpu_compound_mesh_bake_free(bake);
            return false;
        }
        for (int t = 0; t < mesh->triangleCount; ++t)
        {
            const b3MeshTriangle triangle = bake->triangles[t];
            bake->flipped[t] = (b3MeshTriangle){triangle.index1, triangle.index3, triangle.index2};
            bake->flippedFlags[t] = bake->flags != NULL ? (bake->flags[t] >> 4) & 7 : 0;
        }
        bake->triangles = bake->flipped;
        bake->flags = bake->flippedFlags;
    }
    return true;
}
#endif
