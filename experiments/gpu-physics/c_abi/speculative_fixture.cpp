// The pinned CPU world flag has no collision consumer. CPU_REFERENCE checks
// its native no-op explicitly and uses per-shape hull/mesh flags as the oracle.
#include "box3d/box3d.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#ifndef CPU_REFERENCE
extern "C" void gpu_b3_shape_enable_speculative_contact(b3ShapeId,bool);
#endif
#ifdef GPU_API_DUAL
extern "C" b3WorldId both_cpu_world(b3WorldId);
extern "C" void SetSelectedBody(b3BodyId) {}
extern "C" void SetComparisonSelectedBody(b3BodyId) {}
extern "C" void __real_cpu_b3World_EnableSpeculative(b3WorldId,bool);
static int forwarded=0;
static b3WorldId forwardedWorld{};
static bool forwardedFlag=true;
extern "C" void __wrap_cpu_b3World_EnableSpeculative(b3WorldId w,bool enabled) {
    ++forwarded;forwardedWorld=w;forwardedFlag=enabled;
    __real_cpu_b3World_EnableSpeculative(w,enabled);
}
#endif

static void setWorld(b3WorldId w,bool enabled) {
#ifdef GPU_API_DUAL
    const int before=forwarded;
#endif
    b3World_EnableSpeculative(w,enabled);
#ifdef GPU_API_DUAL
    const auto cpu=both_cpu_world(w);
    assert(forwarded==before+1 && forwardedWorld.index1==cpu.index1 && forwardedFlag==enabled);
#endif
}

struct Fixture {b3WorldId world; b3BodyId body; b3MeshData* mesh; b3ShapeId convexShape; b3ShapeId meshShape;};
static Fixture create(int kind,bool convexSpec=true,bool meshSpec=true,bool initialWorld=true,float height=.51f) {
    auto wd=b3DefaultWorldDef();wd.gravity=b3Vec3_zero;wd.enableSleep=false;
    auto w=b3CreateWorld(&wd);setWorld(w,initialWorld);
    auto bd=b3DefaultBodyDef();auto g=b3CreateBody(w,&bd);
    auto sd=b3DefaultShapeDef();sd.enableSpeculativeContact=meshSpec;
    b3MeshData* mesh=nullptr; b3ShapeId groundShape{},bodyShape{};
    if(kind==3) {
        auto floor=b3MakeBoxHull(4,.5f,4);
        b3Body_SetTransform(g,{0,-.5f,0},b3Quat_identity);
        groundShape=b3CreateHullShape(g,&sd,&floor.base);
    } else {
        b3Vec3 v[]={{-4,0,-4},{-4,0,4},{4,0,4},{4,0,-4}};int ix[]={0,1,2,2,3,0};
        b3MeshDef md{};md.vertices=v;md.vertexCount=4;md.indices=ix;md.triangleCount=2;md.identifyEdges=true;
        mesh=b3CreateMesh(&md,nullptr,0);assert(mesh);
        groundShape=b3CreateMeshShape(g,&sd,mesh,{1,1,1});
    }
    bd.type=b3_dynamicBody;bd.position={0,height,0};auto b=b3CreateBody(w,&bd);
    sd.enableSpeculativeContact=convexSpec;
    if(kind==1) {b3Sphere s={b3Vec3_zero,.5f};bodyShape=b3CreateSphereShape(b,&sd,&s);}
    else if(kind==2) {b3Capsule c={{0,-.25f,0},{0,.25f,0},.25f};bodyShape=b3CreateCapsuleShape(b,&sd,&c);}
    else {auto h=b3MakeBoxHull(.5f,.5f,.5f);bodyShape=b3CreateHullShape(b,&sd,&h.base);}
    return {w,b,mesh,bodyShape,groundShape};
}
static void destroy(Fixture f) {b3DestroyWorld(f.world);if(f.mesh)b3DestroyMesh(f.mesh);}
static int contacts(Fixture f) {b3ContactData data[8]{};return b3Body_GetContactData(f.body,data,8);}
static void step(Fixture f,int sub=4) {b3World_Step(f.world,1.f/60,sub);}
static void report(const char* label,Fixture f,int expected) {
    const auto p=b3Body_GetPosition(f.body);const auto v=b3Body_GetLinearVelocity(f.body);
    int n=contacts(f);
    std::printf("%s contacts=%d y=%.9g vy=%.9g\n",label,n,double(p.y),double(v.y));std::fflush(stdout);
    assert(n==expected && std::isfinite(p.y) && std::fabs(v.y)<1e-5f);
}
int main(int argc,char** argv) {
    // Baseline reproducer is intentionally the same public API call.
    if(argc>1 && std::strcmp(argv[1],"reproduce")==0) {
        auto f=create(0);step(f);assert(contacts(f)==1);
        setWorld(f.world,false);step(f);report("world-off-reproducer",f,0);destroy(f);return 0;
    }
    if(!(argc>1 && std::strcmp(argv[1],"ccd-on")==0)) {
    for(int sub: {1,4}) {
        for(int kind=0;kind<4;++kind) {
            std::printf("case sub=%d kind=%d\n",sub,kind);
            auto f=create(kind);step(f,sub);report("default-on",f,1);
            for(bool enabled: {false,false,true,true,false,true}) {
                setWorld(f.world,enabled);
                // A zero-duration step must not consume the pending refresh.
                b3World_Step(f.world,0,sub);step(f,sub);
#ifdef CPU_REFERENCE
                int expected=1;
#else
                int expected=(kind!=0 || enabled)?1:0;
#endif
                report(enabled?"transition-on":"transition-off",f,expected);
            }
            destroy(f);
        }
        for(int kind=0;kind<4;++kind) for(bool convexSpec: {false,true}) for(bool meshSpec: {false,true}) {
            std::printf("flags sub=%d kind=%d convex=%d mesh=%d\n",sub,kind,int(convexSpec),int(meshSpec));
            auto f=create(kind,convexSpec,meshSpec);step(f,sub);
            report("shape-flags",f,(kind!=0 || (convexSpec&&meshSpec))?1:0);destroy(f);
        }
    }
    // Rust extension: upstream exposes shape flags only at creation.
#ifndef CPU_REFERENCE
    for(int endpoint: {0,1}) {
        auto f=create(0);step(f);
        for(bool enabled: {false,true,false,true}) {
            gpu_b3_shape_enable_speculative_contact(endpoint?f.meshShape:f.convexShape,enabled);
            step(f);report(enabled?"shape-transition-on":"shape-transition-off",f,enabled?1:0);
        }
        destroy(f);
    }
#endif
    auto before=create(0,true,true,false);step(before);
#ifdef CPU_REFERENCE
    report("before-creation-off",before,1);
#else
    report("before-creation-off",before,0);
#endif
    destroy(before);
    auto a=create(0),b=create(0);
    setWorld(a.world,false);step(a);step(b);
#ifdef CPU_REFERENCE
    report("independent-off",a,1);
#else
    report("independent-off",a,0);
#endif
    report("independent-on",b,1);destroy(a);
    a=create(0);step(a);report("recreated-default",a,1);step(b);report("surviving-world",b,1);
    destroy(a);destroy(b);
    // Equivalent CPU oracle for a world-off hull/mesh gap uses the documented
    // shape flag. This does not pretend the native CPU world flag is operative.
    auto off=create(0,false,true,false);step(off);report("world-equivalent-shape-off",off,0);destroy(off);
    auto overlap=create(0,true,true,false,.49f);step(overlap);
    assert(contacts(overlap)>0);destroy(overlap);
    }
    // CCD must still block fast hulls even with speculative hull/mesh points off.
    for(bool enabled: {false,true}) {
        if(argc>1 && std::strcmp(argv[1],"ccd-on")==0 && !enabled) continue;
#ifdef CPU_REFERENCE
        auto f=create(0,enabled,true,true,2);
#else
        auto f=create(0,true,true,enabled,2);
#endif
        b3World_SetGravity(f.world,{0,-10,0});
        b3Body_SetLinearVelocity(f.body,{0,-150,0});
        for(int frame=0;frame<30;++frame) {
            step(f);auto p=b3Body_GetPosition(f.body);auto v=b3Body_GetLinearVelocity(f.body);
            std::printf("ccd enabled=%d frame=%d y=%.9g vy=%.9g\n",int(enabled),frame,double(p.y),double(v.y));std::fflush(stdout);
            assert(std::isfinite(p.y)&&p.y>=.495f);
        }
        destroy(f);
    }
    puts("speculative-controls=pass");return 0;
}
