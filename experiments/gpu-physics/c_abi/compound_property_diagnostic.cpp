// Independent CPU oracle for compound public properties; diagnostic logs every mismatch.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#ifdef GPU_PROPERTY_DIAGNOSTIC
extern "C" {
b3WorldId cpu_b3CreateWorld(const b3WorldDef*);
void cpu_b3DestroyWorld(b3WorldId);
b3BodyId cpu_b3CreateBody(b3WorldId,const b3BodyDef*);
b3ShapeId cpu_b3CreateBakedCompoundShape(b3BodyId,b3ShapeDef*,const b3CompoundData*);
void cpu_b3DestroyShape(b3ShapeId,bool);
void cpu_b3Shape_SetUserData(b3ShapeId,void*);
void cpu_b3Shape_SetDensity(b3ShapeId,float,bool);
void* cpu_b3Shape_GetUserData(b3ShapeId);
float cpu_b3Shape_GetDensity(b3ShapeId);
float cpu_b3Shape_GetFriction(b3ShapeId);
float cpu_b3Shape_GetRestitution(b3ShapeId);
b3SurfaceMaterial cpu_b3Shape_GetSurfaceMaterial(b3ShapeId);
int cpu_b3Shape_GetMeshMaterialCount(b3ShapeId);
b3SurfaceMaterial cpu_b3Shape_GetMeshSurfaceMaterial(b3ShapeId,int);
void cpu_b3World_Step(b3WorldId,float,int);
}
#define ORACLE(name) cpu_##name
#else
#define ORACLE(name) name
#endif
extern "C" { void SetSelectedBody(b3BodyId){} void SetComparisonSelectedBody(b3BodyId){} }
static int mismatches=0,observations=0;
static const char* kindName;static int generation;static const char* phase;
static void number(const char* field,double got,double expected) {
    bool pass=std::isfinite(got)&&std::isfinite(expected)&&std::abs(got-expected)<=1e-5;
    std::printf("{\"kind\":\"%s\",\"generation\":%d,\"phase\":\"%s\",\"field\":\"%s\",\"got\":%.9g,\"expected\":%.9g,\"match\":%s}\n",kindName,generation,phase,field,got,expected,pass?"true":"false");
    ++observations;if(!pass)++mismatches;
}
static void integer(const char* field,uintptr_t got,uintptr_t expected) {
    bool pass=got==expected;
    std::printf("{\"kind\":\"%s\",\"generation\":%d,\"phase\":\"%s\",\"field\":\"%s\",\"got\":%llu,\"expected\":%llu,\"match\":%s}\n",kindName,generation,phase,field,(unsigned long long)got,(unsigned long long)expected,pass?"true":"false");
    ++observations;if(!pass)++mismatches;
}
static void material(const char* prefix,b3SurfaceMaterial got,b3SurfaceMaterial expected) {
    char field[80];const char* names[]={"friction","restitution","rolling","tangentX","tangentY","tangentZ"};
    float g[]={got.friction,got.restitution,got.rollingResistance,got.tangentVelocity.x,got.tangentVelocity.y,got.tangentVelocity.z};
    float e[]={expected.friction,expected.restitution,expected.rollingResistance,expected.tangentVelocity.x,expected.tangentVelocity.y,expected.tangentVelocity.z};
    for(int i=0;i<6;++i){std::snprintf(field,sizeof field,"%s.%s",prefix,names[i]);number(field,g[i],e[i]);}
    std::snprintf(field,sizeof field,"%s.userMaterialId",prefix);integer(field,got.userMaterialId,expected.userMaterialId);
    std::snprintf(field,sizeof field,"%s.customColor",prefix);integer(field,got.customColor,expected.customColor);
}
static void check(b3ShapeId observed,b3ShapeId reference,const b3CompoundData* data) {
    auto table=b3GetCompoundMaterials(data);assert(data->materialCount==2);
    // Confirm the independent CPU follows geometry-owned materials, not shapeDef.baseMaterial.
    assert(std::abs(ORACLE(b3Shape_GetFriction)(reference)-table[0].friction)<=1e-5f);
    assert(ORACLE(b3Shape_GetMeshMaterialCount)(reference)==2);
    number("friction",b3Shape_GetFriction(observed),ORACLE(b3Shape_GetFriction)(reference));
    number("restitution",b3Shape_GetRestitution(observed),ORACLE(b3Shape_GetRestitution)(reference));
    number("density",b3Shape_GetDensity(observed),ORACLE(b3Shape_GetDensity)(reference));
    integer("userData",(uintptr_t)b3Shape_GetUserData(observed),(uintptr_t)ORACLE(b3Shape_GetUserData)(reference));
    integer("materialCount",b3Shape_GetMeshMaterialCount(observed),ORACLE(b3Shape_GetMeshMaterialCount)(reference));
    material("surface",b3Shape_GetSurfaceMaterial(observed),ORACLE(b3Shape_GetSurfaceMaterial)(reference));
    for(int i=0;i<data->materialCount;++i){char name[32];std::snprintf(name,sizeof name,"table%d",i);material(name,b3Shape_GetMeshSurfaceMaterial(observed,i),ORACLE(b3Shape_GetMeshSurfaceMaterial)(reference,i));}
}
int main() {
    const char* kinds[]={"sphere","capsule","hull","mesh"};auto hull=b3MakeBoxHull(.5f,.25f,.5f);auto mesh=b3CreateGridMesh(1,1,1,1,false);assert(mesh);
    for(int kind=0;kind<4;++kind) {
        kindName=kinds[kind];b3CompoundSphereDef spheres[2]={};b3CompoundCapsuleDef capsules[2]={};b3CompoundHullDef hulls[2]={};b3CompoundMeshDef meshes[2]={};b3SurfaceMaterial materials[2];b3CompoundDef cd={};
        for(int i=0;i<2;++i){materials[i]=b3DefaultSurfaceMaterial();materials[i].friction=i?.72f:.23f;materials[i].restitution=i?.81f:.31f;materials[i].rollingResistance=i?.12f:.04f;materials[i].tangentVelocity=i?b3Vec3{-.1f,0,.2f}:b3Vec3{.1f,.2f,-.3f};materials[i].userMaterialId=41+i;materials[i].customColor=i?0x112233:0x445566;float x=i?2.f:-2.f;
            spheres[i].sphere={{x,0,0},.6f};spheres[i].material=materials[i];capsules[i].capsule={{x,-.3f,0},{x,.3f,0},.4f};capsules[i].material=materials[i];hulls[i].hull=&hull.base;hulls[i].transform={{x,.1f,0},b3MakeQuatFromAxisAngle({0,0,1},.2f)};hulls[i].material=materials[i];meshes[i].meshData=mesh;meshes[i].transform={{x,.1f,0},b3MakeQuatFromAxisAngle({1,0,0},.1f)};meshes[i].scale={1.2f,1,.8f};meshes[i].materials=&materials[i];meshes[i].materialCount=1;
        }
        if(kind==0){cd.spheres=spheres;cd.sphereCount=2;}if(kind==1){cd.capsules=capsules;cd.capsuleCount=2;}if(kind==2){cd.hulls=hulls;cd.hullCount=2;}if(kind==3){cd.meshes=meshes;cd.meshCount=2;}
        auto compound=b3CreateCompound(&cd);assert(compound&&compound->materialCount==2);auto wd=b3DefaultWorldDef();auto world=b3CreateWorld(&wd);auto oracle=ORACLE(b3CreateWorld)(&wd);auto isolated=b3CreateWorld(&wd);auto isolatedOracle=ORACLE(b3CreateWorld)(&wd);auto bd=b3DefaultBodyDef();bd.position={1,.25f,-2};bd.rotation=b3MakeQuatFromAxisAngle({0,0,1},.2f);
        auto body=b3CreateBody(world,&bd);auto referenceBody=ORACLE(b3CreateBody)(oracle,&bd);auto otherBody=b3CreateBody(isolated,&bd);auto otherReferenceBody=ORACLE(b3CreateBody)(isolatedOracle,&bd);
        auto sd=b3DefaultShapeDef();sd.density=2.5f;sd.baseMaterial.friction=.99f;sd.baseMaterial.restitution=.01f;sd.userData=(void*)(uintptr_t)0x1357;auto otherDef=sd;otherDef.userData=(void*)(uintptr_t)0x2468;otherDef.density=4.f;
        auto other=b3CreateBakedCompoundShape(otherBody,&otherDef,compound);auto otherReference=ORACLE(b3CreateBakedCompoundShape)(otherReferenceBody,&otherDef,compound);
        b3ShapeId stale={};
        for(generation=0;generation<2;++generation){auto parent=b3CreateBakedCompoundShape(body,&sd,compound);auto reference=ORACLE(b3CreateBakedCompoundShape)(referenceBody,&sd,compound);assert(b3Shape_IsValid(parent));
            phase="create";check(parent,reference,compound);
#ifdef GPU_PROPERTY_DIAGNOSTIC
            if(generation){phase="stale-after-reuse";integer("valid",b3Shape_IsValid(stale),0);number("friction",b3Shape_GetFriction(stale),0);number("restitution",b3Shape_GetRestitution(stale),0);number("density",b3Shape_GetDensity(stale),0);integer("userData",(uintptr_t)b3Shape_GetUserData(stale),0);material("surface",b3Shape_GetSurfaceMaterial(stale),b3DefaultSurfaceMaterial());material("table0",b3Shape_GetMeshSurfaceMaterial(stale,0),b3DefaultSurfaceMaterial());integer("materialCount",b3Shape_GetMeshMaterialCount(stale),0);}
#endif
            b3Shape_SetUserData(parent,(void*)(uintptr_t)0x3579);ORACLE(b3Shape_SetUserData)(reference,(void*)(uintptr_t)0x3579);b3Shape_SetDensity(parent,3.75f,false);ORACLE(b3Shape_SetDensity)(reference,3.75f,false);phase="mutate-valid-properties";check(parent,reference,compound);
            b3World_Step(world,1.f/60.f,4);ORACLE(b3World_Step)(oracle,1.f/60.f,4);phase="completed-step";check(parent,reference,compound);phase="world-isolation";check(other,otherReference,compound);
            stale=parent;b3DestroyShape(parent,false);ORACLE(b3DestroyShape)(reference,false);assert(!b3Shape_IsValid(parent));
        }
        b3DestroyWorld(world);ORACLE(b3DestroyWorld)(oracle);b3DestroyWorld(isolated);ORACLE(b3DestroyWorld)(isolatedOracle);b3DestroyCompound(compound);
    }
    b3DestroyMesh(mesh);std::printf("{\"summary\":true,\"observations\":%d,\"mismatches\":%d}\n",observations,mismatches);std::fflush(stdout);
#ifndef GPU_PROPERTY_DIAGNOSTIC
    assert(mismatches==0);
#endif
    return 0;
}
