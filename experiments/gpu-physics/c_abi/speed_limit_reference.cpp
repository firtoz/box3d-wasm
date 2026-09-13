#include "box3d/box3d.h"
#include <cstdio>

// Creation settings and runtime changes must affect integration, not just getters.
int main() {
    for (int kind = 0; kind < 4; ++kind) {
        auto wd = b3DefaultWorldDef();
        wd.gravity = {0, 0, 0};
        wd.maximumLinearSpeed = 9.0f;
        wd.enableSleep = false;
        auto w = b3CreateWorld(&wd);
        auto bd = b3DefaultBodyDef();
        bd.type = kind == 1 ? b3_kinematicBody : b3_dynamicBody;
        auto b = b3CreateBody(w, &bd);
        auto sd = b3DefaultShapeDef();
        b3Sphere sphere = {{0, 0, 0}, 0.5f};
        b3CreateSphereShape(b, &sd, &sphere);
        if (kind == 2) { b3MassData md = {}; b3Body_SetMassData(b, md); }
        if (kind == 3) { b3MotionLocks locks = {}; locks.linearY = true; b3Body_SetMotionLocks(b, locks); }
        const float limits[] = {9, 3, 17};
        for (int phase = 0; phase < 3; ++phase) {
            if (phase) b3World_SetMaximumLinearSpeed(w, limits[phase]);
            b3Body_SetLinearVelocity(b, {30, 40, 0});
            b3World_Step(w, 1.0f / 60.0f, 4);
            auto v = b3Body_GetLinearVelocity(b);
            auto p = b3Body_GetPosition(b);
            std::printf("%d %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n", kind, phase,
                b3World_GetMaximumLinearSpeed(w), v.x, v.y, v.z, p.x, p.y, p.z);
        }
        b3DestroyWorld(w);
    }
}
