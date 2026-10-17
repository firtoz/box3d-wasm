// Small public-ABI reproducer: contact callbacks already own the world lock.
#include "box3d/box3d.h"
#include "native_api_status.h"
#include "native_diagnostics.h"
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#ifdef GPU_API_DUAL
extern "C" void SetSelectedBody(b3BodyId) {}
extern "C" void SetComparisonSelectedBody(b3BodyId) {}
#endif
#ifdef PR05_GPU
extern "C" b3Vec3 gpu_b3_body_get_linear_velocity(b3BodyId);
#endif

struct Context {
    b3WorldId world;
    b3BodyId bodies[2];
    b3ShapeId shapes[2];
    bool baseline;
    int filters = 0, preSolves = 0, denied = 0;
};
template<class Call> static void denied(Context& c, const char* name, Call call) {
#ifdef PR05_GPU
    gpu_b3_native_api_clear_error();
    call();
    assert(errno == EBUSY);
    const char* error = gpu_b3_native_api_last_error();
    assert(error && (!name || std::strcmp(error, name) == 0));
    ++c.denied;
    gpu_b3_native_api_clear_error();
#endif
}
static void inspect(b3ShapeId a, b3ShapeId b, Context& c) {
    std::puts("callback-enter");
#ifdef PR05_GPU
    GpuNativeProfile profile{};
    gpu_b3_world_native_profile(c.world, &profile);
    assert(profile.valid == GPU_DIAGNOSTIC_BUSY);
    std::printf("diagnostic-busy=%u; shape-valid=%d; next=shape-owner-read\n", profile.valid, b3Shape_IsValid(a));
#endif
    // Baseline CPU completes; original GPU hangs exactly at this second lock.
    const auto owner = b3Shape_GetBody(a);
    assert(B3_ID_EQUALS(owner, c.bodies[0]) || B3_ID_EQUALS(owner, c.bodies[1]));
    assert(b3Body_IsValid(owner) && b3World_IsValid(c.world));
    assert(b3Shape_IsValid(a) && b3Shape_IsValid(b));
    if (c.baseline) return;
#ifdef PR05_GPU
    denied(c, "b3World_SetGravity", [&] { b3World_SetGravity(c.world, {0, 123, 0}); });
    denied(c, "b3Body_SetLinearVelocity", [&] { b3Body_SetLinearVelocity(owner, {123, 0, 0}); });
    denied(c, "b3Shape_SetUserData", [&] { b3Shape_SetUserData(a, (void*)123); });
    denied(c, "b3DestroyShape", [&] { b3DestroyShape(a, false); });
    denied(c, "b3DestroyBody", [&] { b3DestroyBody(owner); });
    denied(c, "b3DestroyWorld", [&] { b3DestroyWorld(c.world); });
    denied(c, "b3World_Step", [&] { b3World_Step(c.world, 1.0f / 60, 1); });
    denied(c, nullptr, [&] { (void)gpu_b3_body_get_linear_velocity(owner); });
    assert(b3Shape_IsValid(a) && b3Body_IsValid(owner) && b3World_IsValid(c.world));
#endif
}
static bool filter(b3ShapeId a, b3ShapeId b, void* raw) {
    auto& c = *static_cast<Context*>(raw); ++c.filters; inspect(a, b, c); return true;
}
static bool preSolve(b3ShapeId a, b3ShapeId b, b3Pos, b3Vec3, void* raw) {
    auto& c = *static_cast<Context*>(raw); ++c.preSolves; inspect(a, b, c); return true;
}
int main(int argc, char** argv) {
    setvbuf(stdout, nullptr, _IOLBF, 0);
    Context c{}; c.baseline = argc > 1 && std::strcmp(argv[1], "baseline") == 0;
    auto wd = b3DefaultWorldDef(); wd.gravity = {0, 0, 0}; wd.enableSleep = false;
    c.world = b3CreateWorld(&wd);
    for (int i = 0; i < 2; ++i) {
        auto bd = b3DefaultBodyDef(); bd.type = b3_dynamicBody; bd.position = {0.75f * i, 0, 0};
        c.bodies[i] = b3CreateBody(c.world, &bd);
        auto sd = b3DefaultShapeDef(); sd.enableCustomFiltering = true; sd.enablePreSolveEvents = true;
        const b3Sphere sphere{{0, 0, 0}, 0.5f};
        c.shapes[i] = b3CreateSphereShape(c.bodies[i], &sd, &sphere);
    }
    b3World_SetCustomFilterCallback(c.world, filter, &c);
    b3World_SetPreSolveCallback(c.world, preSolve, &c);
    for (int i = 0; i < 2; ++i) b3World_Step(c.world, 1.0f / 60, 1);
    assert(c.filters > 0 && c.preSolves > 0);
    assert(b3World_IsValid(c.world) && b3Shape_IsValid(c.shapes[0]) && b3Shape_IsValid(c.shapes[1]));
    assert(b3World_GetGravity(c.world).y == 0);
    for (auto s : c.shapes) assert(b3Shape_GetUserData(s) == nullptr);
    b3DestroyWorld(c.world);
    std::printf("PASS callback identity and rejection; filter=%d presolve=%d denied=%d\n", c.filters, c.preSolves, c.denied);
}
