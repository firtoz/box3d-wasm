// Exercise the production dual-world draw/name operation against real Box3D.
// GPU renderer endpoints are isolated stubs; CPU IDs, callbacks and names are real.
#include API_SOURCE
#include <assert.h>
extern __typeof__(b3DefaultWorldDef) cpu_b3DefaultWorldDef;
extern __typeof__(b3DefaultBodyDef) cpu_b3DefaultBodyDef;
extern __typeof__(b3DefaultShapeDef) cpu_b3DefaultShapeDef;
extern __typeof__(b3DefaultDebugDraw) cpu_b3DefaultDebugDraw;
extern __typeof__(b3MakeBoxHull) cpu_b3MakeBoxHull;
extern __typeof__(b3CreateBody) cpu_b3CreateBody;
extern __typeof__(b3CreateHullShape) cpu_b3CreateHullShape;

static int created, submitted, gpu_submitted;
static b3ShapeId expected_id;
static const char* expected_name;
static const char* ground_name = "ground";
static bool ground;
static int token;

const char* gpu_samples_shape_get_name(b3ShapeId id) { return id.world0 == 1 ? "gpu-name" : ""; }
void gpu_b3_world_wait(b3WorldId id) { (void)id; }
void SetSelectedBody(b3BodyId id) { (void)id; }
void SetComparisonSelectedBody(b3BodyId id) { (void)id; }
b3AABB gpu_samples_world_bounds(b3WorldId id) { (void)id; return (b3AABB){0}; }
void gpu_samples_world_draw(b3WorldId id, b3DebugDraw* draw, uint64_t mask)
{
    (void)id; (void)draw; (void)mask;
    // The CPU namespace scope must have ended before the GPU draw.
    assert(!strcmp(b3Shape_GetName((b3ShapeId){1, 1, 1}), "gpu-name"));
    ++gpu_submitted;
}
static void* capture(const b3DebugShape* shape, void* context)
{
    (void)context;
    assert(B3_ID_EQUALS(shape->shapeId, expected_id));
    const char* actual = b3Shape_GetName(shape->shapeId);
    // This is the adapter's exact ground predicate and material decision.
    ground = strcmp(actual, ground_name) == 0;
#ifdef EXPECT_FIXED
    assert(!strcmp(actual, expected_name));
    assert(ground == !strcmp(expected_name, ground_name));
#else
    assert(!strcmp(actual, "") && !ground);
#endif
    printf("cpuID=[%d,%d,%d] CPU_name='%s' dual_name='%s' isGround=%d material=%s cell=%g\n",
        shape->shapeId.index1, shape->shapeId.world0, shape->shapeId.generation,
        expected_name, actual, ground, ground ? "GROUND_GRID" : "SOLID", ground ? 1.0 : 0.0);
    ++created;
    return &token;
}
static void release(void* user, void* ctx) { (void)ctx; assert(user == &token); }
static void submit(void* user, b3WorldTransform xf, b3HexColor color, void* ctx)
{ (void)xf; (void)color; (void)ctx; assert(user == &token); ++submitted; }

int main(void)
{
    for (int scenario=0; scenario<12; ++scenario)
    {
        created = submitted = gpu_submitted = 0;
        g_split = scenario % 2 == 0; g_second_view = false;
        expected_name = (scenario / 2) % 3 == 0 ? ground_name : (scenario / 2) % 3 == 1 ? "ordinary-hull" : "";
        b3WorldDef wd = cpu_b3DefaultWorldDef();
        wd.createDebugShape = capture; wd.destroyDebugShape = release;
        b3WorldId cpu = cpu_b3CreateWorld(&wd), gpu = {1, (uint16_t)(scenario+1)};
        both_map_world(gpu, cpu);
        b3BodyDef bd = cpu_b3DefaultBodyDef();
        b3BodyId body = cpu_b3CreateBody(cpu, &bd);
        b3BoxHull hull = cpu_b3MakeBoxHull(10, 1, 10);
        b3ShapeDef sd = cpu_b3DefaultShapeDef();
        expected_id = cpu_b3CreateHullShape(body, &sd, &hull.base);
        cpu_b3Shape_SetName(expected_id, expected_name);
        b3ShapeId gpu_shape = expected_id; gpu_shape.world0 = 1;
        both_map_shape(gpu_shape, expected_id);
        assert(!strcmp(cpu_b3Shape_GetName(expected_id), expected_name));
        b3DebugDraw draw = cpu_b3DefaultDebugDraw();
        draw.DrawShapeFcn = submit; draw.DrawStringFcn = NULL;
        draw.drawShapes = true; draw.drawingBounds = (b3AABB){{-100, -100, -100}, {100, 100, 100}};
        b3World_Draw(gpu, &draw, UINT64_MAX);
        assert(created == 1 && submitted == 1);
        b3World_Draw(gpu, &draw, UINT64_MAX);
        assert(created == 1 && submitted == 2); // no duplicate/cached geometry changes
        assert(!strcmp(b3Shape_GetName(gpu_shape), "gpu-name"));
        assert(gpu_submitted == (g_split ? 0 : 2));
        assert(B3_ID_EQUALS(both_cpu_shape(gpu_shape), expected_id));
        cpu_b3DestroyWorld(cpu); both_unmap_world(gpu); both_unmap_shape(gpu_shape);

    }
    puts("12 actual dual draw/name cases passed; cached draw, identity and scope restored");
    return 0;
}
