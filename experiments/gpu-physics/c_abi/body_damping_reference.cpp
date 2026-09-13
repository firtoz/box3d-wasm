#include "box3d/box3d.h"
#include <cstdio>
int main() {
  for (int type = 0; type < 3; ++type)
    for (int shapes = 0; shapes < 2; ++shapes) {
      auto wd = b3DefaultWorldDef();
      wd.gravity = {0, 0, 0};
      auto w = b3CreateWorld(&wd);
      auto bd = b3DefaultBodyDef();
      bd.type = (b3BodyType)type;
      bd.enableSleep = false;
      bd.linearVelocity = {2, 0, 0};
      bd.angularVelocity = {0, 3, 0};
      bd.linearDamping = 2.5f;
      bd.angularDamping = 4;
      auto b = b3CreateBody(w, &bd);
      if (shapes) {
        auto sd = b3DefaultShapeDef();
        b3Sphere sphere = {{0, 0, 0}, .25f};
        b3CreateSphereShape(b, &sd, &sphere);
      }
      for (int phase = 0; phase < 2; ++phase) {
        if (phase) {
          b3Body_SetLinearDamping(b, .75f);
          b3Body_SetAngularDamping(b, 1.25f);
        }
        for (int step = 0; step <= 4; ++step) {
          if (step)
            b3World_Step(w, 1.f / 60, 4);
          if (step == 0 || step == 1 || step == 4) {
            auto p = b3Body_GetPosition(b);
            auto q = b3Body_GetRotation(b);
            auto v = b3Body_GetLinearVelocity(b);
            auto a = b3Body_GetAngularVelocity(b);
            printf("%d %d %d %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n", type,
                   shapes, phase, step, float(p.x), q.v.y, q.s, v.x, a.y,
                   b3Body_GetLinearDamping(b), b3Body_GetAngularDamping(b));
          }
        }
      }
      b3DestroyWorld(w);
    }
}
