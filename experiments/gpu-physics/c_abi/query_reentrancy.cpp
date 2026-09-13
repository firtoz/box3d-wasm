#include "box3d/box3d.h"
#include <cstdio>
#include <cmath>
struct Context {
 b3WorldId world; b3BodyId body; int mode; int calls; bool valid; float closest;
};
static float Count(b3ShapeId, b3Pos, b3Vec3, float, uint64_t, int, int, void* data) {
 ++*static_cast<int*>(data); return 1;
}
static float ReadState(b3ShapeId, b3Pos, b3Vec3, float fraction, uint64_t, int, int, void* data) {
 auto* context = static_cast<Context*>(data);
 auto position = b3Body_GetPosition(context->body);
 context->valid &= b3Body_IsValid(context->body) && position.x == 0;
 auto nested = b3World_CastRayClosest(context->world, {-3,0,0}, {10,0,0}, b3DefaultQueryFilter());
 context->valid &= nested.hit && std::fabs(nested.fraction - 0.2f) < 1e-5f;
 int nestedCalls = 0;
 b3World_CastRay(context->world, {-3,0,0}, {10,0,0}, b3DefaultQueryFilter(), Count, &nestedCalls);
 context->valid &= nestedCalls == 2;
 ++context->calls;
 context->closest = std::fmin(context->closest, fraction);
 if(context->mode == 1)return 0; // Stop immediately.
 if(context->mode == 2)return -1; // Ignore this hit without clipping.
 if(context->mode == 3)return fraction; // Clip subsequent exact casts.
 return 1;
}
int main() {
 auto wd=b3DefaultWorldDef(); auto world=b3CreateWorld(&wd);
 b3BodyId first{};
 for(int i=0;i<2;++i) {
  auto bd=b3DefaultBodyDef(); bd.position={float(4*i),0,0};
  auto body=b3CreateBody(world,&bd); if(i==0)first=body;
  auto sd=b3DefaultShapeDef(); b3Sphere sphere={b3Vec3_zero,1};
  b3CreateSphereShape(body,&sd,&sphere);
 }
 bool valid=true;
 for(int mode=0;mode<4;++mode) {
  Context context={world,first,mode,0,true,1};
  b3World_CastRay(world,{-3,0,0},{10,0,0},b3DefaultQueryFilter(),ReadState,&context);
  valid &= context.valid;
  valid &= mode==1 ? context.calls==1 : mode==3 ? context.calls>=1 && context.calls<=2 : context.calls==2;
  if(mode!=1)valid &= std::fabs(context.closest-0.2f)<1e-5f;
  // Traversal order is not an API promise; compare semantic outcomes, not order.
  printf("mode=%d valid=%d\n",mode,valid);
 }
 b3DestroyWorld(world);
 return valid ? 0 : 32;
}
