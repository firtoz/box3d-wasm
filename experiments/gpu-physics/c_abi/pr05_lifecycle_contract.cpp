// Repeated public C lifecycle and owned resources, without a scene sweep.
#include "box3d/box3d.h"
#include "native_diagnostics.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <dirent.h>
extern "C" int gpu_b3_live_world_count();
extern "C" int gpu_shape_geometry_mirror_count();
#ifdef GPU_API_DUAL
extern "C" void SetSelectedBody(b3BodyId) {}
extern "C" void SetComparisonSelectedBody(b3BodyId) {}
#endif
static int entries(const char* path) {
    DIR* dir = opendir(path); assert(dir);
    int count = 0;
    while (auto* e = readdir(dir)) if (e->d_name[0] != '.') ++count;
    closedir(dir); return count;
}
int main() {
    int fdBaseline = 0, threadBaseline = 0, maxFd = 0, maxThreads = 0;
    const auto hull = b3MakeBoxHull(0.25f, 0.25f, 0.25f);
    b3WorldId stale{}; b3BodyId staleBody{}; b3ShapeId staleShape{};
    for (int cycle = 0; cycle < 12; ++cycle) {
        auto wd = b3DefaultWorldDef(); wd.gravity = {0, 0, 0};
        auto world = b3CreateWorld(&wd), other = b3CreateWorld(&wd);
        assert(b3World_IsValid(world) && b3World_IsValid(other));
        if (cycle) assert(!b3World_IsValid(stale) && !b3Body_IsValid(staleBody) && !b3Shape_IsValid(staleShape));
        auto bd = b3DefaultBodyDef(); bd.type = b3_dynamicBody;
        auto a = b3CreateBody(world, &bd); bd.position = {3, 0, 0};
        auto b = b3CreateBody(world, &bd);
        auto sd = b3DefaultShapeDef();
        auto shape = b3CreateHullShape(a, &sd, &hull.base);
        const b3Sphere sphere{{0, 0, 0}, 0.25f};
        b3CreateSphereShape(b, &sd, &sphere);
        auto jd = b3DefaultDistanceJointDef(); jd.base.bodyIdA = a; jd.base.bodyIdB = b; jd.length = 3;
        auto joint = b3CreateDistanceJoint(world, &jd);
        assert(b3Joint_IsValid(joint));
        b3Body_SetAwake(a, false); assert(!b3Body_IsAwake(a));
        b3Body_SetAwake(a, true); assert(b3Body_IsAwake(a));
        b3Body_Disable(b); assert(!b3Body_IsEnabled(b));
        b3Body_Enable(b); assert(b3Body_IsEnabled(b));
        b3World_Step(world, 0, 1);
        GpuNativeProfile profile{}; gpu_b3_world_native_profile(world, &profile);
        assert(profile.physics_step == 0);
        for (int step = 0; step < 3; ++step) b3World_Step(world, 1.0f / 60, 1);
        const auto position = b3Body_GetPosition(a);
        assert(std::isfinite(position.x) && std::isfinite(position.y) && std::isfinite(position.z));
        const auto before = b3Shape_GetBody(shape);
        b3Shape_SetSphere(shape, &sphere);
        assert(b3Shape_IsValid(shape) && B3_ID_EQUALS(before, b3Shape_GetBody(shape)));
        b3Shape_SetHull(shape, &hull.base);
        auto counts = b3World_GetCounters(world);
        assert(counts.bodyCount == 2 && counts.shapeCount == 2 && counts.jointCount == 1);
        b3DestroyJoint(joint, true); assert(!b3Joint_IsValid(joint));
        // Destroy while the newest submission may still be pending.
        b3World_Step(world, 1.0f / 60, 1);
        stale = world; staleBody = a; staleShape = shape;
        b3DestroyWorld(world);
        assert(b3World_IsValid(other)); b3DestroyWorld(other);
        assert(gpu_b3_live_world_count() == 0 && gpu_shape_geometry_mirror_count() == 0);
        const int fd = entries("/proc/self/fd"), threads = entries("/proc/self/task");
        if (cycle == 2) { fdBaseline = fd; threadBaseline = threads; }
        if (cycle >= 3) { assert(fd <= fdBaseline + 1); assert(threads <= threadBaseline + 2); }
        maxFd = fd > maxFd ? fd : maxFd; maxThreads = threads > maxThreads ? threads : maxThreads;
        std::printf("cycle=%d live_worlds=0 owned_geometry=0 fd=%d threads=%d\n", cycle, fd, threads);
    }
    std::printf("PASS 12 lifecycle cycles; FD baseline/max=%d/%d threads baseline/max=%d/%d\n", fdBaseline, maxFd, threadBaseline, maxThreads);
}
