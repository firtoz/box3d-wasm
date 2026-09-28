// CPU reference for trace_contact_island_wake_propagation: four unit-mass cubes,
// three touching, one separate, no gravity. Preserve defaults except density.
#include "box3d/box3d.h"
#include <cstdio>
int main() {
    auto wd=b3DefaultWorldDef();wd.gravity={0,0,0};auto world=b3CreateWorld(&wd);
    auto bd=b3DefaultBodyDef();bd.type=b3_dynamicBody;
    auto sd=b3DefaultShapeDef();sd.density=1.0f;
    auto hull=b3MakeBoxHull(0.5f,0.5f,0.5f);
    b3BodyId bodies[4];const float positions[4]={0,1,2,10};
    for(int i=0;i<4;++i) {
        bd.position={positions[i],0,0};bodies[i]=b3CreateBody(world,&bd);
        b3CreateHullShape(bodies[i],&sd,&hull.base);
    }
    for(int step=1;step<=73;++step) {
        if(step==65) {
            for(auto body:bodies) if(b3Body_IsAwake(body)) return 2;
            b3Body_ApplyLinearImpulseToCenter(bodies[0],{1,0,0},false);
        }
        if(step==66) b3Body_ApplyLinearImpulseToCenter(bodies[0],{1,0,0},true);
        b3World_Step(world,1.0f/60.0f,4);
        for(int i=0;i<4;++i) {
            auto p=b3Body_GetPosition(bodies[i]);auto v=b3Body_GetLinearVelocity(bodies[i]);auto w=b3Body_GetAngularVelocity(bodies[i]);
            bool awake=b3Body_IsAwake(bodies[i]);
            if(step==65 && awake) return 3;
            if(step>=66 && awake!=(i<3)) return 4;
            std::printf("%d %d %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",step,i,int(awake),p.x,p.y,p.z,v.x,v.y,v.z,w.x,w.y,w.z);
        }
    }
    b3DestroyWorld(world);
}
