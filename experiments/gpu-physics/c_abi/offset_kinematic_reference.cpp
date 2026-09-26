#include "box3d/box3d.h"
#include <cstdio>
#include <initializer_list>

extern "C" void b3_world_gpu_wait_with_mirror(b3WorldId) __attribute__((weak));

static void dump(int kind, int frame, b3BodyId body)
{
    auto mass = b3Body_GetMassData(body);
    auto p = b3Body_GetPosition(body);
    auto q = b3Body_GetRotation(body);
    auto center = b3Body_GetWorldPoint(body, mass.center);
    auto v = b3Body_GetLinearVelocity(body);
    auto omega = b3Body_GetAngularVelocity(body);
    std::printf("%d %d", kind, frame);
    for (float value : {mass.mass, mass.center.x, mass.center.y, mass.center.z,
                       p.x, p.y, p.z, q.v.x, q.v.y, q.v.z, q.s,
                       center.x, center.y, center.z,
                       v.x, v.y, v.z, omega.x, omega.y, omega.z})
        std::printf(" %.9g", value);
    std::puts("");
}

int main()
{
    for (int kind = 0; kind < 3; ++kind)
    {
        auto wd = b3DefaultWorldDef();
        auto world = b3CreateWorld(&wd);
        auto bd = b3DefaultBodyDef();
        bd.type = kind == 0 ? b3_staticBody : b3_kinematicBody;
        bd.position = {0, 3, 0};
        auto body = b3CreateBody(world, &bd);
        auto sd = b3DefaultShapeDef();
        const b3Vec3 offset = {0.5f, 0.5f, 1.0f};
        auto hull = b3MakeTransformedBoxHull(0.5f, 0.5f, 1.0f, {offset, b3Quat_identity});
        b3CreateHullShape(body, &sd, &hull.base);
        // Case 1 is the upstream Offset Kinematic setup. Case 2 also checks
        // preservation of the origin's velocity when the mass center changes.
        if (kind == 2)
        {
            b3Body_SetLinearVelocity(body, {0.2f, -0.1f, 0.3f});
            b3Body_SetAngularVelocity(body, {1, -1, 2});
        }
        auto mass = b3Body_GetMassData(body);
        mass.center = offset;
        b3Body_SetMassData(body, mass);
        b3Body_SetAngularVelocity(body, {1, -1, 2});
        dump(kind, 0, body);
        for (int frame = 1; frame <= 300; ++frame)
        {
            b3World_Step(world, 1.0f / 60.0f, 4);
            if (b3_world_gpu_wait_with_mirror) b3_world_gpu_wait_with_mirror(world);
            if (kind == 2 && frame == 150)
            {
                mass.center = {-0.25f, 0.75f, 0.5f};
                b3Body_SetMassData(body, mass);
            }
            dump(kind, frame, body);
        }
        b3DestroyWorld(world);
    }
}
