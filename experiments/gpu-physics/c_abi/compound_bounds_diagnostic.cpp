// Diagnose public/extended bounds against CPU compound geometry; not physics acceptance.
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
b3AABB cpu_b3Shape_GetAABB(b3ShapeId);
}
struct Bounds {std::vector<GpuNativeShapeBounds> records;};
static void visit(const GpuNativeShapeBounds* p,void* raw) {static_cast<Bounds*>(raw)->records.push_back(*p);}
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
            auto parent=b3CreateBakedCompoundShape(body,&sd,compound);assert(b3Shape_IsValid(parent));
            Bounds records;assert(gpu_b3_world_native_visit_shape_bounds(world,b3_staticBody,visit,&records)==GPU_DIAGNOSTIC_OK);
            assert(records.records.size()==1 && B3_ID_EQUALS(records.records[0].shape,parent));
            auto cpu=cpu_b3Shape_GetAABB(both_cpu_shape(parent));
            auto expected=b3ComputeCompoundAABB(compound,{bd.position,bd.rotation});
            auto actual=b3Shape_GetAABB(parent);
            for(int axis=0;axis<3;++axis) {
                (&expected.lowerBound.x)[axis]-=B3_SPECULATIVE_DISTANCE;
                (&expected.upperBound.x)[axis]+=B3_SPECULATIVE_DISTANCE;
            }
            for(int lane=0;lane<6;++lane) {
                const float c=reinterpret_cast<const float*>(&cpu)[lane];
                const float e=reinterpret_cast<const float*>(&expected)[lane];
                const float g=reinterpret_cast<const float*>(&actual)[lane];
                const float n=reinterpret_cast<const float*>(&records.records[0].bounds)[lane];
                assert(std::isfinite(c) && std::isfinite(e) && std::isfinite(g) && std::isfinite(n));
                assert(std::abs(c-e)<=1e-5f);
                std::printf("%s rotated=%d lane=%d CPU=%.9g independentCPU=%.9g GPUpublic=%.9g GPUnative=%.9g publicError=%.9g nativeError=%.9g\n",
                    kinds[kind],rotated,lane,c,e,g,n,std::abs(c-g),std::abs(c-n));
            }
            std::fflush(stdout);
            b3DestroyWorld(world);
        }
        b3DestroyCompound(compound);
    }
    b3DestroyMesh(mesh);
    std::puts("compound bounds diagnosis: eight cases complete; observed GPU discrepancies are not acceptance");
}
