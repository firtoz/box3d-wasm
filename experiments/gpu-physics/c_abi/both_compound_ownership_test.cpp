// Independent mapped CPU topology for all baked-compound child kinds.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "native_diagnostics.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
extern "C" {
void SetSelectedBody(b3BodyId) {}
void SetComparisonSelectedBody(b3BodyId) {}
b3ShapeId both_cpu_shape(b3ShapeId);
b3AABB cpu_b3Shape_GetAABB(b3ShapeId);
bool cpu_b3Shape_IsValid(b3ShapeId);
void gpu_samples_get_cpu_stats(b3WorldId,b3Profile*,b3Counters*);
}
static void counts(b3WorldId w,int gpuShapes,int cpuShapes,const char* kind,const char* phase) {
    auto g=b3World_GetCounters(w);b3Counters c={};b3Profile p={};
    gpu_samples_get_cpu_stats(w,&p,&c);
    std::printf("%s %s GPU bodies/shapes/joints=%d/%d/%d CPU=%d/%d/%d\n",kind,phase,
        g.bodyCount,g.shapeCount,g.jointCount,c.bodyCount,c.shapeCount,c.jointCount);
    std::fflush(stdout);
    assert(g.bodyCount==1 && c.bodyCount==1 && g.jointCount==0 && c.jointCount==0);
    assert(g.shapeCount==gpuShapes && c.shapeCount==cpuShapes);
}
struct Bounds {std::vector<GpuNativeShapeBounds> records;};
static void visit(const GpuNativeShapeBounds* p,void* raw) {static_cast<Bounds*>(raw)->records.push_back(*p);}
static void compare_bounds(b3WorldId w,b3ShapeId parent) {
    Bounds b;
    assert(gpu_b3_world_native_visit_shape_bounds(w,b3_staticBody,visit,&b)==GPU_DIAGNOSTIC_OK);
    assert(b.records.size()==1 && B3_ID_EQUALS(b.records[0].shape,parent));
    auto cpu=cpu_b3Shape_GetAABB(both_cpu_shape(parent));
    const auto* g=reinterpret_cast<const float*>(&b.records[0].bounds);
    const auto* c=reinterpret_cast<const float*>(&cpu);
    for(int i=0;i<6;++i) {
        if(!std::isfinite(g[i]) || !std::isfinite(c[i]) || std::abs(g[i]-c[i])>1e-5f) {
            std::fprintf(stderr,"compound bounds lane=%d GPU=%.9g CPU=%.9g\n",i,g[i],c[i]);
            assert(false);
        }
    }
}
int main(int argc,char** argv) {
    bool baseline=argc==2 && !std::strcmp(argv[1],"--baseline");
    const char* kinds[]={"sphere","capsule","hull","mesh"};
    auto hull=b3MakeBoxHull(.5f,.25f,.5f);
    auto mesh=b3CreateGridMesh(1,1,1,1,false);assert(mesh);
    for(int kind=0;kind<4;++kind) {
        b3CompoundSphereDef spheres[2]={};b3CompoundCapsuleDef capsules[2]={};
        b3CompoundHullDef hulls[2]={};b3CompoundMeshDef meshes[2]={};
        auto material=b3DefaultSurfaceMaterial();b3CompoundDef cd={};
        for(int i=0;i<2;++i) {
            float x=i?2.f:-2.f;
            spheres[i].sphere={{x,0,0},.6f};spheres[i].material=material;
            capsules[i].capsule={{x,-.3f,0},{x,.3f,0},.4f};capsules[i].material=material;
            hulls[i].hull=&hull.base;hulls[i].transform={{x,.1f,0},b3MakeQuatFromAxisAngle({0,0,1},.2f)};
            hulls[i].material=material;
            meshes[i].meshData=mesh;meshes[i].transform={{x,.1f,0},b3MakeQuatFromAxisAngle({1,0,0},.1f)};
            meshes[i].scale={1.2f,1, .8f};meshes[i].materials=&material;meshes[i].materialCount=1;
        }
        if(kind==0){cd.spheres=spheres;cd.sphereCount=2;}
        if(kind==1){cd.capsules=capsules;cd.capsuleCount=2;}
        if(kind==2){cd.hulls=hulls;cd.hullCount=2;}
        if(kind==3){cd.meshes=meshes;cd.meshCount=2;}
        auto compound=b3CreateCompound(&cd);assert(compound);
        auto wd=b3DefaultWorldDef();auto world=b3CreateWorld(&wd);
        auto bd=b3DefaultBodyDef();bd.position={1,.25f,-2};
        bd.rotation=b3MakeQuatFromAxisAngle({0,0,1},.2f);
        auto body=b3CreateBody(world,&bd);auto sd=b3DefaultShapeDef();
        int duplicates=baseline && kind!=3?2:0;
        for(int generation=0;generation<3;++generation) {
            auto shape=b3CreateBakedCompoundShape(body,&sd,compound);assert(b3Shape_IsValid(shape));
            int old=duplicates*generation;
            counts(world,1,1+old+duplicates,kinds[kind],"create");
            if(!baseline)compare_bounds(world,shape);
            // Public individual constructors must still create/mirror one shape.
            b3Sphere sphere={{0,10,0},.25f};b3Capsule capsule={{0,12,0},{0,13,0},.25f};
            b3ShapeId individual[]={b3CreateSphereShape(body,&sd,&sphere),
                b3CreateCapsuleShape(body,&sd,&capsule),b3CreateHullShape(body,&sd,&hull.base)};
            for(auto s:individual)assert(b3Shape_IsValid(s) && cpu_b3Shape_IsValid(both_cpu_shape(s)));
            counts(world,4,4+old+duplicates,kinds[kind],"individual");
            auto cpuShape=both_cpu_shape(shape);b3DestroyShape(shape,false);
            assert(!b3Shape_IsValid(shape) && !cpu_b3Shape_IsValid(cpuShape));
            counts(world,3,3+old+duplicates,kinds[kind],"delete-parent");
            for(auto s:individual)b3DestroyShape(s,false);
            counts(world,0,old+duplicates,kinds[kind],"delete-individual");
        }
        b3DestroyWorld(world);b3DestroyCompound(compound);
    }
    b3DestroyMesh(mesh);
    std::puts(baseline?"baseline reproduced: duplicate primitive CPU children; mesh unaffected":
        "combined compound ownership: four child kinds, bounds, public constructors and three lifetime cycles passed");
}
