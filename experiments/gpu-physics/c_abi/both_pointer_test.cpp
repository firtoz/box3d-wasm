// Real CPU and GPU worlds deliberately disagree: query results must stay independent.
#include "both_view.h"
#include "host/camera.h"
#include "sokol_split_input.h"
extern "C" {
#include "both_ids.h"
b3Pos cpu_b3Body_GetPosition(b3BodyId);
b3WorldTransform cpu_b3Body_GetTransform(b3BodyId);
void cpu_b3Body_SetTransform(b3BodyId, b3Pos, b3Quat);
b3Pos cpu_b3Body_GetWorldPoint(b3BodyId, b3Vec3);
b3Vec3 cpu_b3Body_GetAngularVelocity(b3BodyId);
b3Vec3 cpu_b3Body_GetLocalPoint(b3BodyId, b3Pos);
void gpu_b3_body_set_transform(b3BodyId, float, float, float, float, float, float, float);
void gpu_b3_world_wait(b3WorldId);
void SetSelectedBody(b3BodyId) {}
void SetComparisonSelectedBody(b3BodyId) {}
}
#include <cassert>
#include <cmath>
#include <cstdio>
#include <initializer_list>
static bool near(float a, float b) { return fabsf(a-b) < 1e-4f; }
int main() {
    for (int width : {1280, 1920, 1919}) {
        Camera camera; camera.SetView(25,30,12,{1,2,3}); camera.Update(0,width/2,1080);
        for (float x : {0.0f, 140.25f, float(width/2-1)}) {
            auto a = camera.BuildPickRay(split_pointer_x(x,width),310.5f);
            auto b = camera.BuildPickRay(split_pointer_x(x+width/2,width),310.5f);
            assert(near(a.origin.x,b.origin.x) && near(a.origin.y,b.origin.y) && near(a.origin.z,b.origin.z));
            assert(near(a.translation.x,b.translation.x) && near(a.translation.y,b.translation.y) && near(a.translation.z,b.translation.z));
        }
        assert(split_pointer_x(width/2+20,width,0)==width/2+20);
        assert(split_pointer_x(width/2-20,width,1)==-20);
    }

    b3WorldDef wd = b3DefaultWorldDef();
    b3WorldId w = b3CreateWorld(&wd);
    b3BodyDef bd = b3DefaultBodyDef(); bd.type = b3_dynamicBody; bd.position = {0,2,0};
    b3BodyId body = b3CreateBody(w, &bd);
    b3BoxHull hull = b3MakeBoxHull(.5f,.5f,.5f);
    b3ShapeDef sd = b3DefaultShapeDef(); b3CreateHullShape(body,&sd,&hull.base);
    gpu_b3_body_set_transform(body,0,2,2,0,0,0,1);
    const b3Pos origin{.3f,2.3f,10}; const b3Vec3 ray{0,0,-20};
    both_pointer_down(w,origin,ray,true,1000);
    auto cpu = both_pointer_state(0), gpu = both_pointer_state(1);
    fprintf(stderr,"pick cpu body=%d joint=%d f=%g; gpu body=%d joint=%d f=%g\n",cpu.selected.index1,cpu.joint.index1,cpu.fraction,gpu.selected.index1,gpu.joint.index1,gpu.fraction);
    assert(cpu.joint.index1 && gpu.joint.index1);
    assert(near(cpu.fraction,.475f)); assert(near(gpu.fraction,.375f));
    for (auto state : {cpu,gpu}) {
        assert(near(state.local_anchor.x,.3f) && near(state.local_anchor.y,.3f) && near(state.local_anchor.z,.5f));
    }
    assert(near(cpu_b3Body_GetPosition(cpu.mouse).z,.5f));
    assert(near(b3Body_GetPosition(gpu.mouse).z,2.5f));
    // A move while paused changes neither engine. Each target retains its ray depth.
    both_pointer_move({.3f,4.3f,10},ray);
    b3World_Step(w,0,4);
    assert(near(cpu_b3Body_GetPosition(both_cpu_body(body)).y,2));
    assert(near(b3Body_GetPosition(body).y,2));
    b3World_Step(w,1.0f/60,4); gpu_b3_world_wait(w);
    float cy=cpu_b3Body_GetPosition(both_cpu_body(body)).y, gy=b3Body_GetPosition(body).y;
    assert(cy>2 && cy<3); assert(gy>2 && gy<3); // forces, not a 2 m teleport
    for (int k=0;k<600;++k) {
        b3World_Step(w,1.0f/60,4); gpu_b3_world_wait(w);
        // Read each world, just as native drawing does.
        auto cp=cpu_b3Body_GetPosition(both_cpu_body(body)); auto gp=b3Body_GetPosition(body);
        // The deliberate initial Z offset must persist; all relative motion
        // must agree, including sustained spring support while the mouse is held.
        assert(fabs(cp.x-gp.x)<5e-4 && fabs(cp.y-gp.y)<5e-4 && fabs(cp.z+2-gp.z)<5e-4);
        auto cq=cpu_b3Body_GetTransform(both_cpu_body(body)).q;
        auto gq=b3Body_GetRotation(body);
        assert(fabs(cq.v.x-gq.v.x)<5e-4 && fabs(cq.v.y-gq.v.y)<5e-4 && fabs(cq.v.z-gq.v.z)<5e-4 && fabs(cq.s-gq.s)<5e-4);
        if(k==599) {
            auto cm=cpu_b3Body_GetPosition(cpu.mouse); auto gm=b3Body_GetPosition(gpu.mouse);
            fprintf(stderr,"held cpu p=(%g,%g,%g) mouse=(%g,%g,%g); gpu p=(%g,%g,%g) mouse=(%g,%g,%g)\n",cp.x,cp.y,cp.z,cm.x,cm.y,cm.z,gp.x,gp.y,gp.z,gm.x,gm.y,gm.z);
            assert(near(cm.y,4.3f) && near(gm.y,4.3f));
            assert(near(cm.z,.5f) && near(gm.z,2.5f));
            auto ca=cpu_b3Body_GetWorldPoint(both_cpu_body(body),{.3f,.3f,.5f});
            auto ga=b3Body_GetWorldPoint(body,{.3f,.3f,.5f});
            fprintf(stderr,"held anchors cpu=(%g,%g,%g), gpu=(%g,%g,%g)\n",ca.x,ca.y,ca.z,ga.x,ga.y,ga.z);
        }
    }
    both_pointer_up(); assert(!both_pointer_state(0).joint.index1 && !both_pointer_state(1).joint.index1);
    cpu_b3Body_SetTransform(both_cpu_body(body), {0,2,0}, b3Quat_identity);
    // GPU misses but CPU still grabs. No forced body correspondence or shared hit.
    gpu_b3_body_set_transform(body,100,2,2,0,0,0,1);
    both_pointer_down(w,{0,2,10},ray,true,1000);
    assert(both_pointer_state(0).joint.index1); assert(!both_pointer_state(1).joint.index1);
    both_pointer_up();
    // CPU misses but GPU still grabs.
    gpu_b3_body_set_transform(body,3,2,2,0,0,0,1);
    both_pointer_down(w,{3,2,10},ray,true,1000);
    assert(!both_pointer_state(0).joint.index1); assert(both_pointer_state(1).joint.index1);
    b3DestroyWorld(w); assert(!both_pointer_state(1).joint.index1);
    w = b3CreateWorld(&wd);
    body = b3CreateBody(w,&bd); b3CreateHullShape(body,&sd,&hull.base);
    gpu_b3_body_set_transform(body,0,2,2,0,0,0,1);
    auto pp=cpu_b3Body_GetPosition(both_cpu_body(body));
    fprintf(stderr,"impulse body %d world %d cpu p %g %g %g\n",body.index1,body.world0,pp.x,pp.y,pp.z);
    both_pointer_impulse(w,origin,ray,{0,10,0});
    auto ca = cpu_b3Body_GetAngularVelocity(both_cpu_body(body));
    auto ga = b3Body_GetAngularVelocity(body);
    assert(near(ca.x,ga.x) && near(ca.y,ga.y) && near(ca.z,ga.z));
    fprintf(stderr,"impulse cpu %g %g %g gpu %g %g %g\n",ca.x,ca.y,ca.z,ga.x,ga.y,ga.z);
    assert(fabsf(ca.x)>0.01f && fabsf(ca.z)>0.01f);
    b3DestroyWorld(w);
    puts("PASS independent hits/depths, one-sided misses, force-driven dragging, pause, release, world destruction, independent impulse points, viewport rays");
}
