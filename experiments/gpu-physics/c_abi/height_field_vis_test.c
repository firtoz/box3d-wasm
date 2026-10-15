// Exercise the actual lazy renderer metadata operation with immutable inputs.
// Before/after builds select the original/current samples_api.c, not a model.
#include API_SOURCE
#include <assert.h>

static int created, destroyed;
static const b3HeightFieldData* expected;
static b3ShapeId expected_id;
static void* capture(const b3DebugShape* shape, void* context)
{
    (void)context;
    assert(shape->type == b3_heightShape);
    assert(B3_ID_EQUALS(shape->shapeId, expected_id));
    assert(shape->heightField == expected);
    ++created;
    return malloc(1);
}
static void release(void* shape, void* context)
{
    (void)context;
    assert(shape);
    free(shape);
    ++destroyed;
}
static void record(b3ShapeId id, b3BodyId body, const b3HeightFieldData* data)
{
#ifdef EXPECT_FIXED
    gpu_samples_on_height_field_shape_created(id, body, data);
#else
    (void)data;
    gpu_samples_on_shape_created(id, body, b3_heightShape, NULL, NULL, NULL);
#endif
}

int main(void)
{
    for (int scenario = 0; scenario < 6; ++scenario)
    {
        created = destroyed = 0;
        b3WorldId world = {1, 1};
        b3WorldDef def = {0};
        def.createDebugShape = capture;
        def.destroyDebugShape = release;
        gpu_samples_on_world_created(world, &def);
        b3HeightFieldData data = {0}, replacement = {0};
        data.version = replacement.version = B3_HEIGHT_FIELD_VERSION;
        data.scale = (b3Vec3){1, .6f, 1};
        replacement.scale = (b3Vec3){2, 1.5f, 2};
        data.columnCount = data.rowCount = 21;
        replacement.columnCount = replacement.rowCount = 400;
        b3ShapeId id = {.index1=17, .world0=1, .generation=9};
        b3BodyId body = {.index1=13, .world0=1, .generation=7};
        expected = &data;
        expected_id = id;
        record(id, body, &data);
        ShapeVis* s = shape_vis(id, false);
        assert(s && B3_ID_EQUALS(s->body, body));
        void* token = ensure_user_shape(world, id, s);
#ifndef EXPECT_FIXED
        assert(token == NULL && created == 0);
        printf("before case%d type=%d inputGeometry=%p callback=NULL drawMetadata=NULL\n",
               scenario, s->type, (void*)&data);
#else
        assert(token && created == 1 && s->heightField == &data);
        assert(ensure_user_shape(world, id, s) == token && created == 1);
        if (scenario == 1)
        {
            gpu_samples_shape_set_name(id, "terrain");
            expected = &replacement;
            gpu_samples_on_height_field_shape_replaced(id, body, &replacement);
            assert(destroyed == 1 && !s->userShape && !strcmp(s->name, "terrain"));
            assert(B3_ID_EQUALS(s->id, id) && B3_ID_EQUALS(s->body, body));
            assert(ensure_user_shape(world, id, s) && created == 2);
        }
        if (scenario == 2)
        {
            b3WorldId other = {2, 1};
            b3ShapeId other_id = id; other_id.world0 = 2;
            b3BodyId other_body = body; other_body.world0 = 2;
            gpu_samples_on_world_created(other, &def);
            record(other_id, other_body, &replacement);
            expected = &replacement; expected_id = other_id;
            assert(ensure_user_shape(other, other_id, shape_vis(other_id, false)));
            assert(shape_vis(id, false)->heightField == &data);
            gpu_samples_on_world_destroyed(other);
        }
        if (scenario == 3)
        {
            gpu_samples_on_shape_destroyed(id);
            assert(!shape_vis(id, false));
            b3ShapeId newer = id; newer.generation++;
            expected_id = newer;
            record(newer, body, &data);
            assert(!shape_vis(id, false));
            assert(ensure_user_shape(world, newer, shape_vis(newer, false)));
        }
        if (scenario == 4)
        {
            gpu_samples_on_shape_destroyed(id);
            gpu_samples_on_shape_destroyed(id);
            assert(destroyed == 1 && !shape_vis(id, false));
        }
        if (scenario == 5)
        {
            gpu_samples_on_world_destroyed(world);
            assert(!shape_vis(id, false) && !g_shapes[1].chunks);
            gpu_samples_on_world_created(world, &def);
            record(id, body, &replacement);
            expected = &replacement;
            assert(ensure_user_shape(world, id, shape_vis(id, false)));
        }
        printf("after case%d type=%d identity/geometry/cache/lifecycle preserved\n", scenario, b3_heightShape);
#endif
        gpu_samples_on_world_destroyed(world);
        assert(created == destroyed);
        assert(!g_shapes[1].chunks);
    }
}
