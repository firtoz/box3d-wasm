// Compile with function sections + linker GC: exercise actual C mirror ownership
// independently of GPU startup, including the former slot-zero cleanup hole.
#include "shim.c"
#include <assert.h>

int main(void)
{
    GpuHullMirror* first = find_hull_mirror((b3ShapeId){1, 1, 1}, true);
    for (int32_t i = 1; i <= 100001; ++i)
    {
        b3ShapeId id = {i, 1, 1};
        GpuHullMirror* m = find_hull_mirror(id, true);
        m->data = (b3HullData*)malloc(8);
        m->index1 = i; m->world0 = 1;
    }
    assert(first == find_hull_mirror((b3ShapeId){1, 1, 1}, false));
    assert(gpu_shape_geometry_mirror_count() == 100001);
    b3ShapeId independent = {100001, 2, 1};
    GpuHullMirror* other = find_hull_mirror(independent, true);
    other->data = (b3HullData*)malloc(8); other->index1 = independent.index1; other->world0 = 2;
    assert(other != find_hull_mirror((b3ShapeId){100001, 1, 1}, false));
    GpuMeshMirror* mesh = find_mesh_mirror((b3ShapeId){100002, 1, 1}, true);
    mesh->index1 = 100002; mesh->world0 = 1; mesh->parent = 100001;
    mesh->materials = (b3SurfaceMaterial*)malloc(sizeof(b3SurfaceMaterial));
    gpu_shape_clear_geometry((b3ShapeId){100001, 1, 1});
    assert(!find_hull_mirror((b3ShapeId){100001, 1, 1}, false));
    assert(!find_mesh_mirror((b3ShapeId){100002, 1, 1}, false));
    assert(find_hull_mirror(independent, false) == other);
    GpuHeightFieldMirror* height = find_height_field_mirror((b3ShapeId){100001, 1, 2}, true);
    height->index1 = 100001; height->world0 = 1;
    height->materials = (b3SurfaceMaterial*)malloc(sizeof(b3SurfaceMaterial));
    gpu_shape_clear_world_geometry((b3WorldId){1, 1});
    assert(gpu_shape_geometry_mirror_count() == 1);
    assert(!g_geometry[1].chunks);
    gpu_shape_clear_world_geometry((b3WorldId){2, 1});
    assert(gpu_shape_geometry_mirror_count() == 0);
    puts("geometry mirrors: high indices, stable pointers, child cleanup, reuse and world cleanup passed");
}
