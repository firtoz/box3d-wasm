// Independent CPU compound geometry verifies public/extended/body AABB semantics.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "box3d/constants.h"
#include "native_diagnostics.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>
extern "C" {
void SetSelectedBody(b3BodyId) {}
void SetComparisonSelectedBody(b3BodyId) {}
b3ShapeId both_cpu_shape(b3ShapeId);
b3BodyId both_cpu_body(b3BodyId);
b3AABB cpu_b3Body_ComputeAABB(b3BodyId);
bool cpu_b3Shape_IsValid(b3ShapeId);
b3AABB cpu_b3Shape_GetAABB(b3ShapeId);
}
struct Bounds {std::vector<GpuNativeShapeBounds> records;};
static void visit(const GpuNativeShapeBounds* p,void* raw) {static_cast<Bounds*>(raw)->records.push_back(*p);}
static void check_bounds(b3WorldId world,b3BodyId body,b3ShapeId parent,const b3CompoundData* compound,
                         b3Transform transform,const char* kind,int rotation,int generation,const char* phase) {
    auto expected=b3ComputeCompoundAABB(compound,transform);
    for(int axis=0;axis<3;++axis) {
        (&expected.lowerBound.x)[axis]-=B3_SPECULATIVE_DISTANCE;
        (&expected.upperBound.x)[axis]+=B3_SPECULATIVE_DISTANCE;
    }
    Bounds records;
    assert(gpu_b3_world_native_visit_shape_bounds(world,b3_staticBody,visit,&records)==GPU_DIAGNOSTIC_OK);
    assert(records.records.size()==1 && B3_ID_EQUALS(records.records[0].shape,parent));
    b3AABB observed[]={cpu_b3Shape_GetAABB(both_cpu_shape(parent)),b3Shape_GetAABB(parent),
        records.records[0].bounds,b3Body_ComputeAABB(body),cpu_b3Body_ComputeAABB(both_cpu_body(body))};
    const char* names[]={"CPUparent","GPUpublic","GPUnative","GPUbody","CPUbody"};
    for(int query=0;query<5;++query) {
        float maximum=0.f;
        for(int lane=0;lane<6;++lane) {
            float got=reinterpret_cast<const float*>(&observed[query])[lane];
            float want=reinterpret_cast<const float*>(&expected)[lane];
            if(!std::isfinite(got)||!std::isfinite(want)||std::abs(got-want)>1e-5f) {
                std::fprintf(stderr,"%s rotated=%d generation=%d %s %s lane=%d got=%.9g expected=%.9g\n",
                    kind,rotation,generation,phase,names[query],lane,got,want);assert(false);
            }
            maximum=std::fmax(maximum,std::abs(got-want));
        }
        std::printf("%s rotated=%d generation=%d %s %s maxError=%.9g\n",kind,rotation,generation,phase,names[query],maximum);
    }
    std::fflush(stdout);
}
int main() {
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
        for(int rotated=0;rotated<2;++rotated) {
            auto wd=b3DefaultWorldDef();auto world=b3CreateWorld(&wd);
            auto bd=b3DefaultBodyDef();bd.position={1,.25f,-2};
            bd.rotation=b3MakeQuatFromAxisAngle({0,0,1},rotated?.2f:0.f);
            auto body=b3CreateBody(world,&bd);auto sd=b3DefaultShapeDef();
            for(int generation=0;generation<2;++generation) {
                b3Transform transform={bd.position,bd.rotation};
                b3Body_SetTransform(body,transform.p,transform.q);
                auto parent=b3CreateBakedCompoundShape(body,&sd,compound);assert(b3Shape_IsValid(parent));
                check_bounds(world,body,parent,compound,transform,kinds[kind],rotated,generation,"create");
                transform={{6,-1,4},b3MakeQuatFromAxisAngle({0,0,1},rotated?-.3f:.1f)};
                b3Body_SetTransform(body,transform.p,transform.q);
                check_bounds(world,body,parent,compound,transform,kinds[kind],rotated,generation,"transform");
                b3Body_Disable(body);
                check_bounds(world,body,parent,compound,transform,kinds[kind],rotated,generation,"disable");
                b3Body_Enable(body);
                check_bounds(world,body,parent,compound,transform,kinds[kind],rotated,generation,"enable");
                auto oldCpu=both_cpu_shape(parent);b3DestroyShape(parent,false);
                assert(!b3Shape_IsValid(parent) && !cpu_b3Shape_IsValid(oldCpu));
                auto stale=b3Shape_GetAABB(parent);
                for(float value: {stale.lowerBound.x,stale.lowerBound.y,stale.lowerBound.z,
                                  stale.upperBound.x,stale.upperBound.y,stale.upperBound.z})assert(value==0.f);
            }
            b3DestroyWorld(world);
        }
        b3DestroyCompound(compound);
    }
    b3DestroyMesh(mesh);
    std::puts("compound AABB contract: public/native/body/CPU geometry, transforms, disabled access and stale lifetimes passed");
}
