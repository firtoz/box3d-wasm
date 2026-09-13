// CPU oracle for the two multi-manifold fixtures in gpu_invariants.rs.
// Compile against oracle/build/box3d-build/src/libbox3d.a.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cstdio>
int main() {
  for (int test = 0; test < 2; test++) {
    auto wd = b3DefaultWorldDef();
    auto w = b3CreateWorld(&wd);
    auto gd = b3DefaultBodyDef();
    auto g = b3CreateBody(w, &gd);
    auto sd = b3DefaultShapeDef();
    b3Vec3 floor[] = {
        {-2, 0, -2}, {-2, 0, -.2f}, {2, 0, -.2f}, {-2, 0, 2}, {2, 0, 2}};
    int fi[] = {0, 1, 2, 1, 3, 4, 1, 4, 2};
    b3Vec3 wall[] = {{-2, 0, -2},    {-2, 0, 2},   {2, 0, 2},    {2, 0, -2},
                     {.25f, -1, -2}, {.25f, 0, 2}, {.25f, 0, -2}};
    int wi[] = {0, 1, 2, 0, 2, 3, 4, 5, 6};
    b3MeshDef md = {};
    md.vertices = test ? wall : floor;
    md.vertexCount = test ? 7 : 5;
    md.indices = test ? wi : fi;
    md.triangleCount = 3;
    md.identifyEdges = false;
    auto mesh = b3CreateMesh(&md, nullptr, 0);
    b3CreateMeshShape(g, &sd, mesh, b3Vec3_one);
    auto bd = b3DefaultBodyDef();
    bd.type = b3_dynamicBody;
    bd.position = test ? b3Vec3{0, .49f, 0} : b3Vec3{0, .5f, .2032f};
    bd.enableSleep = false;
    if (!test) {
      bd.motionLocks.angularX = true;
      bd.motionLocks.angularY = true;
      bd.motionLocks.angularZ = true;
    }
    auto b = b3CreateBody(w, &bd);
    auto h = b3MakeBoxHull(test ? .5f : .4f, .5f, test ? .5f : .4f);
    b3CreateHullShape(b, &sd, &h.base);
    b3World_Step(w, 1.f / 60, 4);
    b3ContactData d[16];
    int n = b3Body_GetContactData(b, d, 16);
    printf("case %d contacts %d\n", test, n);
    for (int i = 0; i < n; i++)
      for (int j = 0; j < d[i].manifoldCount; j++) {
        auto m = d[i].manifolds[j];
        printf("normal %g %g %g points %d\n", m.normal.x, m.normal.y,
               m.normal.z, m.pointCount);
      }
    b3DestroyWorld(w);
    b3DestroyMesh(mesh);
  }
}
