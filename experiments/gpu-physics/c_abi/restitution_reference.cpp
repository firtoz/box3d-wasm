// Analytical vertical sphere/floor restitution diagnostic, matching the Rust fixture.
#include "box3d/box3d.h"
#include <cstdio>
int main() {
    auto wd=b3DefaultWorldDef();auto world=b3CreateWorld(&wd);
    auto sd=b3DefaultShapeDef();sd.density=1.0f;sd.baseMaterial.friction=0.0f;sd.baseMaterial.restitution=0.0f;
    auto bd=b3DefaultBodyDef();bd.position={0.0f,-0.5f,0.0f};
    auto ground=b3CreateBody(world,&bd);auto hull=b3MakeBoxHull(5.0f,0.5f,5.0f);
    b3CreateHullShape(ground,&sd,&hull.base);bd.type=b3_dynamicBody;
    b3BodyId bodies[3];const float e[3]={0.0f,0.5f,1.0f};
    for(int i=0;i<3;++i) {
        bd.position={-2.0f+2.0f*i,2.5f,0.0f};sd.baseMaterial.restitution=e[i];
        bodies[i]=b3CreateBody(world,&bd);b3Sphere sphere={{0.0f,0.0f,0.0f},0.5f};
        b3CreateSphereShape(bodies[i],&sd,&sphere);
    }
    for(int frame=1;frame<=240;++frame) {
        b3World_Step(world,1.0f/60.0f,4);
        for(int i=0;i<3;++i) {
            auto p=b3Body_GetPosition(bodies[i]);auto v=b3Body_GetLinearVelocity(bodies[i]);auto w=b3Body_GetAngularVelocity(bodies[i]);
            std::printf("%d %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",frame,i,p.x,p.y,p.z,v.x,v.y,v.z,w.x,w.y,w.z);
        }
    }
    b3DestroyWorld(world);
}
