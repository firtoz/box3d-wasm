// Copied into a separate scratch directory and linked against release archives.
// Uses only the public Box3D API and headers, with original default physics.
#include <box3d/box3d.h>
#include <cassert>
#include <cmath>
#include <cstdio>
#ifdef GPU_API_DUAL
extern "C" void SetSelectedBody(b3BodyId) {}
extern "C" void SetComparisonSelectedBody(b3BodyId) {}
#endif
int main() {
    const auto wd=b3DefaultWorldDef();
    const auto w=b3CreateWorld(&wd); assert(b3World_IsValid(w));
    auto bd=b3DefaultBodyDef(); bd.type=b3_dynamicBody;bd.position={1,5,2};
    const auto b=b3CreateBody(w,&bd); const auto sd=b3DefaultShapeDef();
    const b3Sphere sphere{{0,0,0},0.5f};const auto s=b3CreateSphereShape(b,&sd,&sphere);
    assert(b3Body_IsValid(b)&&b3Shape_IsValid(s)&&B3_ID_EQUALS(b3Shape_GetBody(s),b));
    b3World_Step(w,1.0f/60.0f,1);
    const auto p=b3Body_GetPosition(b);
    const auto v=b3Body_GetLinearVelocity(b);
    const float mass=b3Body_GetMass(b);
    assert(std::isfinite(p.y)&&p.y<5&&v.y<0&&mass>0);
    const auto hit=b3World_CastRayClosest(w,{p.x,p.y+2,p.z},{0,-4,0},b3DefaultQueryFilter());
    assert(B3_ID_EQUALS(hit.shapeId,s)&&std::abs(hit.fraction-0.375f)<1e-5f);
    const auto counts=b3World_GetCounters(w);assert(counts.bodyCount==1&&counts.shapeCount==1);
    b3DestroyWorld(w); assert(!b3World_IsValid(w)&&!b3Body_IsValid(b)&&!b3Shape_IsValid(s));
    std::printf("{\"position\":[%.17g,%.17g,%.17g],\"velocity\":[%.9g,%.9g,%.9g],\"mass\":%.9g,\"ray_fraction\":%.9g,\"destroyed\":true}\n",p.x,p.y,p.z,v.x,v.y,v.z,mass,hit.fraction);
}
