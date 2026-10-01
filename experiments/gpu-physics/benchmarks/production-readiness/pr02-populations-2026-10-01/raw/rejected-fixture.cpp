// Public diagnostic populations omitted by the original single-box contract.
// CPU runs establish the geometry/topology reference; GPU extension checks are
// enabled only for linked GPU/combined fixtures. No solver tolerance changes.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "box3d/constants.h"
#ifdef GPU_POPULATIONS
#include "native_diagnostics.h"
#endif
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
#ifdef GPU_API_DUAL
extern "C" {
void SetSelectedBody(b3BodyId) {}
void SetComparisonSelectedBody(b3BodyId) {}
b3ShapeId both_cpu_shape(b3ShapeId);
b3AABB cpu_b3Shape_GetAABB(b3ShapeId);
void gpu_samples_get_cpu_stats(b3WorldId, b3Profile*, b3Counters*);
}
#endif

static void near(float a, float b) {
    if (!std::isfinite(a) || !std::isfinite(b) || std::abs(a-b)>1e-5f) {
        std::fprintf(stderr,"bounds scalar GPU=%.9g reference=%.9g\n",a,b);
        std::abort();
    }
}
static void same_bounds(b3AABB a,b3AABB b) {
    const float* x=reinterpret_cast<const float*>(&a);
    const float* y=reinterpret_cast<const float*>(&b);
    for(int i=0;i<6;++i)near(x[i],y[i]);
}
static b3BodyId body(b3WorldId w,b3BodyType type,b3Vec3 position) {
    auto bd=b3DefaultBodyDef();bd.type=type;bd.position=position;
    return b3CreateBody(w,&bd);
}
static b3ShapeId sphere(b3BodyId b,float radius,bool sensor=false) {
    auto sd=b3DefaultShapeDef();sd.isSensor=sensor;sd.enableSensorEvents=true;
    b3Sphere s={{0,0,0},radius};return b3CreateSphereShape(b,&sd,&s);
}
static b3WorldId world() {
    auto wd=b3DefaultWorldDef();wd.gravity={0,0,0};wd.enableSleep=false;
    return b3CreateWorld(&wd);
}
static void step(b3WorldId w) {for(int i=0;i<4;++i)b3World_Step(w,1.f/60.f,4);}
static b3Counters counts(b3WorldId w,const char* population,int bodies,int shapes,int contacts) {
    auto c=b3World_GetCounters(w);
    std::printf("%s body=%d shape=%d contact=%d manifolds=",population,c.bodyCount,c.shapeCount,c.contactCount);
    for(int x:c.manifoldCounts)std::printf("%d,",x);std::puts("");std::fflush(stdout);
    assert(c.bodyCount==bodies && c.shapeCount==shapes && c.jointCount==0);
    assert(c.contactCount==contacts);
#ifdef GPU_API_DUAL
    b3Profile profile={};b3Counters cpu={};gpu_samples_get_cpu_stats(w,&profile,&cpu);
    assert(cpu.bodyCount==bodies && cpu.shapeCount==shapes && cpu.jointCount==0);
    assert(cpu.contactCount==contacts);
#endif
    return c;
}
#ifdef GPU_POPULATIONS
struct Bounds {b3WorldId world;std::vector<GpuNativeShapeBounds> records;};
static void visit(const GpuNativeShapeBounds* record,void* context) {
    auto& b=*static_cast<Bounds*>(context);assert(b3World_IsValid(b.world));
    b.records.push_back(*record);
}
static std::vector<GpuNativeShapeBounds> bounds(b3WorldId w,b3BodyType type,int expected) {
    Bounds b{w,{}};
    assert(gpu_b3_world_native_visit_shape_bounds(w,type,visit,&b)==GPU_DIAGNOSTIC_OK);
    assert(int(b.records.size())==expected);return b.records;
}
static void peaks(b3WorldId w,int staticBodies,int dynamicBodies,int staticShapes,int dynamicShapes) {
    auto p=b3World_GetMaxCapacity(w);
    assert(p.staticBodyCount==staticBodies && p.dynamicBodyCount==dynamicBodies);
    assert(p.staticShapeCount==staticShapes && p.dynamicShapeCount==dynamicShapes);
    GpuNativeAllocation a={};gpu_b3_world_native_allocation(w,&a);
    assert(a.valid==GPU_DIAGNOSTIC_OK && !a.physics_invalid && a.primary_buffer_bytes>0);
    assert(a.body_capacity>=unsigned(staticBodies+dynamicBodies));
}
static void profile_step(b3WorldId w,unsigned step) {
    GpuNativeProfile p={};gpu_b3_world_native_profile(w,&p);
    assert(p.valid==GPU_DIAGNOSTIC_OK && p.physics_step==step && p.timestamp_step<=step);
    assert((p.timestamp_step==0)==(p.measured_fields==0));
    for(int i=0;i<23;++i) {
        if(p.measured_fields&(1u<<i))assert(std::isfinite(p.values[i]) && p.values[i]>=0);
        else assert(std::isnan(p.values[i]));
    }
}
#endif
static void sensor_population() {
    auto w=world();auto sensorBody=body(w,b3_staticBody,{0,0,0});
    auto sensorShape=sphere(sensorBody,1,true);
    auto dynamic=body(w,b3_dynamicBody,{0,0,0});auto visitor=sphere(dynamic,.5f);
    auto disabled=body(w,b3_staticBody,{10,0,0});auto disabledShape=sphere(disabled,1);
    b3Body_Disable(disabled);b3World_Step(w,0,4);
#ifdef GPU_POPULATIONS
    auto pre=b3World_GetMaxCapacity(w);assert(pre.staticBodyCount==0 && pre.dynamicBodyCount==0);
#endif
    step(w);auto c=counts(w,"sensor",3,3,0);
    for(int x:c.manifoldCounts)assert(x==0);
    b3ShapeId overlaps[4]={};assert(b3Shape_GetSensorData(sensorShape,overlaps,4)==1);
    assert(B3_ID_EQUALS(overlaps[0],visitor));
    auto position=b3Body_GetPosition(dynamic);near(position.x,0);near(position.y,0);near(position.z,0);
#ifdef GPU_POPULATIONS
    peaks(w,2,1,1,1);assert(b3World_GetMaxCapacity(w).contactCount==0);
    auto records=bounds(w,b3_staticBody,2);bool found=false;
    for(auto& r:records)if(B3_ID_EQUALS(r.shape,disabledShape)) {
        found=true;assert(B3_ID_EQUALS(r.body,disabled));
        const float p=B3_SPECULATIVE_DISTANCE;
        same_bounds(r.bounds,{{9-p,-1-p,-1-p},{11+p,1+p,1+p}});
    }
    assert(found);profile_step(w,4);
    b3World_Step(w,0,4);profile_step(w,4);
    b3DestroyBody(sensorBody);b3DestroyBody(dynamic);
    auto current=b3World_GetCounters(w);assert(current.bodyCount==1 && current.shapeCount==1 && current.contactCount==0);
    peaks(w,2,1,1,1);assert(bounds(w,b3_staticBody,1).size()==1);
#endif
    b3DestroyWorld(w);
}
static void compound_population() {
    auto w=world();auto ground=body(w,b3_staticBody,{0,0,0});
    b3CompoundSphereDef spheres[2]={};
    for(int i=0;i<2;++i) {
        spheres[i].sphere={{i?.5f:-.5f,0,0},.6f};spheres[i].material=b3DefaultSurfaceMaterial();
    }
    b3CompoundDef cd={};cd.spheres=spheres;cd.sphereCount=2;
    auto compound=b3CreateCompound(&cd);auto sd=b3DefaultShapeDef();
    auto parent=b3CreateBakedCompoundShape(ground,&sd,compound);
    auto dynamic=body(w,b3_dynamicBody,{0,1.09f,0});auto hull=b3MakeBoxHull(.75f,.5f,.5f);
    auto childContact=b3CreateHullShape(dynamic,&sd,&hull.base);
    step(w);auto c=counts(w,"compound",2,2,1);
    b3ContactData data[4]={};assert(b3Shape_GetContactData(childContact,data,4)==1);
    assert(B3_ID_EQUALS(data[0].shapeIdA,parent) || B3_ID_EQUALS(data[0].shapeIdB,parent));
    assert(data[0].manifoldCount>=2);
    assert(c.manifoldCounts[std::min(data[0].manifoldCount,8)-1]==1);
#ifdef GPU_POPULATIONS
    peaks(w,1,1,1,1);auto r=bounds(w,b3_staticBody,1)[0];
    assert(B3_ID_EQUALS(r.shape,parent) && B3_ID_EQUALS(r.body,ground));
    const float p=B3_SPECULATIVE_DISTANCE;
    same_bounds(r.bounds,{{-1.1f-p,-.6f-p,-.6f-p},{1.1f+p,.6f+p,.6f+p}});
#ifdef GPU_API_DUAL
    same_bounds(r.bounds,cpu_b3Shape_GetAABB(both_cpu_shape(parent)));
#endif
    profile_step(w,4);
    b3DestroyShape(parent,false);assert(b3World_GetCounters(w).shapeCount==1);
    assert(b3World_GetCounters(w).contactCount==0);assert(bounds(w,b3_staticBody,0).empty());
    peaks(w,1,1,1,1);
#endif
    b3DestroyWorld(w);b3DestroyCompound(compound);
}
static void mesh_population() {
    auto w=world();auto ground=body(w,b3_staticBody,{0,0,0});
    auto mesh=b3CreateGridMesh(1,1,2,1,false);auto sd=b3DefaultShapeDef();
    auto meshShape=b3CreateMeshShape(ground,&sd,mesh,{1,1,1});
    auto dynamic=body(w,b3_dynamicBody,{0,.49f,0});auto hull=b3MakeBoxHull(.5f,.5f,.5f);
    auto shape=b3CreateHullShape(dynamic,&sd,&hull.base);
    auto kinematic=body(w,b3_kinematicBody,{10,0,0});sphere(kinematic,.5f);
    step(w);auto c=counts(w,"mesh",3,3,1);
    b3ContactData data[4]={};assert(b3Shape_GetContactData(shape,data,4)==1);
    assert(data[0].manifoldCount>=2);
    assert(c.manifoldCounts[std::min(data[0].manifoldCount,8)-1]==1);
#ifdef GPU_POPULATIONS
    peaks(w,1,2,1,2);auto r=bounds(w,b3_staticBody,1)[0];
    assert(B3_ID_EQUALS(r.shape,meshShape) && B3_ID_EQUALS(r.body,ground));
    const float p=B3_SPECULATIVE_DISTANCE;
    same_bounds(r.bounds,{{-1-p,-p,-1-p},{1+p,p,1+p}});
#ifdef GPU_API_DUAL
    same_bounds(r.bounds,cpu_b3Shape_GetAABB(both_cpu_shape(meshShape)));
#endif
    assert(bounds(w,b3_kinematicBody,1).size()==1);profile_step(w,4);
#endif
    b3DestroyWorld(w);b3DestroyMesh(mesh);
}
int main() {
    sensor_population();compound_population();mesh_population();
    std::puts("native diagnostic populations: sensor/compound/mesh contract passed");
}
