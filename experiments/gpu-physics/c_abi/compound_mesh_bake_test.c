#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
static int allocation_count, fail_at;
static void* test_allocate(size_t bytes)
{
    ++allocation_count;
    return allocation_count == fail_at ? NULL : malloc(bytes);
}
#define GPU_COMPOUND_ALLOC(size) test_allocate(size)
#include "compound_mesh_bake.h"

typedef struct MeshFixture {
    b3MeshData mesh;
    b3Vec3 vertices[3];
    b3MeshTriangle triangles[1];
    uint8_t flags[1];
} MeshFixture;

int main(void)
{
    MeshFixture source = {0};
    source.mesh.vertexCount = 3;
    source.mesh.triangleCount = 1;
    source.mesh.vertexOffset = offsetof(MeshFixture, vertices);
    source.mesh.triangleOffset = offsetof(MeshFixture, triangles);
    source.mesh.flagsOffset = offsetof(MeshFixture, flags);
    source.vertices[0] = (b3Vec3){1, 0, 0};
    source.vertices[1] = (b3Vec3){0, 1, 0};
    source.vertices[2] = (b3Vec3){0, 0, 1};
    source.triangles[0] = (b3MeshTriangle){0, 1, 2};
    source.flags[0] = 0x52; /* inverse flags differ from positive-scale flags */
    const MeshFixture original = source;
    b3ChildShape child = {0};
    child.mesh.data = &source.mesh;
    child.transform = b3Transform_identity;
    child.transform.p = (b3Vec3){2, 3, 4};
    for (int bits = 0; bits < 8; ++bits) {
        child.mesh.scale = (b3Vec3){bits & 1 ? -2 : 2, bits & 2 ? -3 : 3, bits & 4 ? -4 : 4};
        GpuCompoundMeshBake bake;
        allocation_count = 0; fail_at = 0;
        assert(gpu_compound_mesh_bake(&child, &bake));
        assert(bake.vertices[0].x == (bits & 1 ? 0 : 4));
        assert(bake.vertices[1].y == (bits & 2 ? 0 : 6));
        assert(bake.vertices[2].z == (bits & 4 ? 0 : 8));
        bool negative = ((bits & 1) != 0) ^ ((bits & 2) != 0) ^ ((bits & 4) != 0);
        assert(bake.triangles[0].index2 == (negative ? 2 : 1));
        assert(bake.triangles[0].index3 == (negative ? 1 : 2));
        assert(bake.flags[0] == (negative ? 5 : 0x52));
        assert(memcmp(&source, &original, sizeof(source)) == 0);
        gpu_compound_mesh_bake_free(&bake);
    }
    child.mesh.scale = (b3Vec3){-1, 1, 1};
    for (fail_at = 1; fail_at <= 3; ++fail_at) {
        allocation_count = 0;
        GpuCompoundMeshBake bake;
        assert(!gpu_compound_mesh_bake(&child, &bake));
        assert(bake.vertices == NULL && bake.flipped == NULL && bake.flippedFlags == NULL);
        gpu_compound_mesh_bake_free(&bake); /* failure cleanup is idempotent */
        assert(memcmp(&source, &original, sizeof(source)) == 0);
    }
    puts("pass: 8 scale orientations, 3 allocation failures, immutable source and cleanup");
}
