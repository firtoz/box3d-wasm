#include "samples_api.c"
#include <assert.h>

static unsigned destroyed;
static void destroy_shape(void* shape, void* ctx)
{
    (void)ctx;
    assert(shape == (void*)(uintptr_t)42);
    ++destroyed;
}
static void* create_shape(const b3DebugShape* shape, void* ctx)
{
    (void)ctx;
    assert(shape->shapeId.index1 == 1);
    b3Sphere sphere = {{0,0,0}, 1};
    // Force directory growth while ensure_user_shape holds its original entry.
    for (int32_t i = 2; i <= 100001; ++i)
        gpu_samples_on_shape_created((b3ShapeId){i,1,2}, (b3BodyId){i,1,2}, b3_sphereShape, &sphere, NULL, NULL);
    return (void*)(uintptr_t)42;
}
int main(void)
{
    b3WorldDef def = {0}; def.createDebugShape = create_shape; def.destroyDebugShape = destroy_shape;
    gpu_samples_on_world_created((b3WorldId){1,1}, &def);
    gpu_samples_on_world_created((b3WorldId){2,1}, &def);
    b3Sphere sphere = {{0,0,0}, 1};
    b3ShapeId id = {1,1,2};
    gpu_samples_on_shape_created(id, (b3BodyId){1,1,2}, b3_sphereShape, &sphere, NULL, NULL);
    ShapeVis* first = shape_vis(id, false);
    assert(ensure_user_shape((b3WorldId){1,1}, id, first) == (void*)(uintptr_t)42);
    assert(first == shape_vis(id, false) && first->userShape == (void*)(uintptr_t)42);
    b3ShapeId high = {100001,1,2}, other = {100001,2,2};
    assert(shape_vis(high, false)->id.index1 == 100001);
    gpu_samples_on_shape_created(other, (b3BodyId){100001,2,2}, b3_sphereShape, &sphere, NULL, NULL);
    gpu_samples_shape_set_name(high, "high first world");
    gpu_samples_shape_set_name(other, "independent second world");
    assert(strcmp(gpu_samples_shape_get_name(high), "high first world") == 0);
    assert(strcmp(gpu_samples_shape_get_name(other), "independent second world") == 0);
    gpu_samples_on_shape_destroyed(high);
    b3ShapeId fresh = {100001,1,3};
    gpu_samples_on_shape_created(fresh, (b3BodyId){100001,1,3}, b3_sphereShape, &sphere, NULL, NULL);
    gpu_samples_on_shape_destroyed(high);
    assert(!shape_vis(high, false) && shape_vis(fresh, false));
    gpu_samples_on_world_destroyed((b3WorldId){1,1});
    assert(destroyed == 1 && !g_shapes[1].chunks && shape_vis(other, false));
    gpu_samples_on_world_destroyed((b3WorldId){2,1});
    assert(!g_shapes[2].chunks);
    puts("visual metadata: high indices, reentrant creation, world isolation, reuse and cleanup passed");
}
