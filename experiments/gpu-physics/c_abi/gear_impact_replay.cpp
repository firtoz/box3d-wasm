// Cold-start diagnostic: exact native terrain and rock geometry, recorded body states.
// Mechanism bodies/joints and previous contact caches are not restored.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "earcut.h"
#include <array>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include "gear_terrain_generated.inc"
extern "C" void gpu_b3_world_dump_mesh_candidates(b3WorldId) __attribute__((weak));
int main(int argc,char** argv) {
 if(argc!=3)return 64;
 FILE* in=std::fopen(argv[1],"r");if(!in)return 65;
 int count=0;if(std::fscanf(in,"%d",&count)!=1 || count<1 || count>120)return 66;
 auto wd=b3DefaultWorldDef();wd.enableSleep=false;
 if(std::getenv("GEAR_REPLAY_NO_CCD"))wd.enableContinuous=false;
 std::fprintf(stderr,"replay-continuous %d\n",wd.enableContinuous);
 auto world=b3CreateWorld(&wd);
 auto gd=b3DefaultBodyDef();gd.position={0,-1,0};auto floor=b3CreateBody(world,&gd);
 auto sd=b3DefaultShapeDef();auto box=b3MakeBoxHull(20,1,20);b3CreateHullShape(floor,&sd,&box.base);
 gd=b3DefaultBodyDef();auto ground=b3CreateBody(world,&gd);GearTerrain terrain;terrain.CreateMesh(ground);
 auto rock=b3CreateRock(.3f);sd=b3DefaultShapeDef();sd.baseMaterial.rollingResistance=.3f;
 struct Body {int source; b3BodyId id;};std::vector<Body> bodies;
 for(int i=0;i<count;++i){
  int source;float p[3],q[4],v[3],w[3];
  if(std::fscanf(in,"%d %f %f %f %f %f %f %f %f %f %f %f %f %f",&source,&p[0],&p[1],&p[2],&q[0],&q[1],&q[2],&q[3],&v[0],&v[1],&v[2],&w[0],&w[1],&w[2])!=14)return 67;
  auto bd=b3DefaultBodyDef();bd.type=b3_dynamicBody;bd.position={p[0],p[1],p[2]};bd.rotation={{q[0],q[1],q[2]},q[3]};
  bd.linearVelocity={v[0],v[1],v[2]};bd.angularVelocity={w[0],w[1],w[2]};
  auto id=b3CreateBody(world,&bd);b3CreateHullShape(id,&sd,rock);
  // Shape creation changes COM and adjusts velocity. Restore the recorded
  // COM velocity only after the final mass properties are established.
  b3Body_SetLinearVelocity(id,{v[0],v[1],v[2]});
  b3Body_SetAngularVelocity(id,{w[0],w[1],w[2]});
  auto md=b3Body_GetMassData(id);
  auto inertia=md.inertia;
  std::fprintf(stderr,"body-mass-data %d center %.9g %.9g %.9g\n",source,md.center.x,md.center.y,md.center.z);
  std::fprintf(stderr,"body-mass %d mass %.9g inverse %.9g local-inertia %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",source,b3Body_GetMass(id),b3Body_GetInverseMass(id),inertia.cx.x,inertia.cx.y,inertia.cx.z,inertia.cy.x,inertia.cy.y,inertia.cy.z,inertia.cz.x,inertia.cz.y,inertia.cz.z);
  bodies.push_back({source,id});
 }
 std::fclose(in);int steps=std::atoi(argv[2]);if(steps<1 || steps>120)return 68;
 for(int step=0;step<=steps;++step){
  if(step)b3World_Step(world,1.f/60,4);
  if(step && gpu_b3_world_dump_mesh_candidates)gpu_b3_world_dump_mesh_candidates(world);
  for(auto b:bodies){auto p=b3Body_GetPosition(b.id);auto q=b3Body_GetRotation(b.id);auto v=b3Body_GetLinearVelocity(b.id);auto w=b3Body_GetAngularVelocity(b.id);
   std::printf("%d %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",step,b.source,p.x,p.y,p.z,q.v.x,q.v.y,q.v.z,q.s,v.x,v.y,v.z,w.x,w.y,w.z);
   int capacity=b3Body_GetContactCapacity(b.id);
   if(capacity<0 || capacity>4096)return 69;
   std::vector<b3ContactData> contacts(capacity);
   int n=b3Body_GetContactData(b.id,contacts.data(),capacity);
   for(int i=0;i<n;++i)for(int j=0;j<contacts[i].manifoldCount;++j){
    const auto& m=contacts[i].manifolds[j];
    std::fprintf(stderr,"manifold %d %d %d %d normal %.9g %.9g %.9g rolling %.9g %.9g %.9g points %d\n",step,b.source,i,j,m.normal.x,m.normal.y,m.normal.z,m.rollingImpulse.x,m.rollingImpulse.y,m.rollingImpulse.z,m.pointCount);
    for(int k=0;k<m.pointCount;++k){const auto& pt=m.points[k];
     std::fprintf(stderr,"point %d %d %d %d %d tri %d sep %.9g impulse %.9g total %.9g anchors %.9g %.9g %.9g %.9g %.9g %.9g\n",step,b.source,i,j,k,pt.triangleIndex,pt.separation,pt.normalImpulse,pt.totalNormalImpulse,pt.anchorA.x,pt.anchorA.y,pt.anchorA.z,pt.anchorB.x,pt.anchorB.y,pt.anchorB.z);
    }
   }
  }
 }
 b3DestroyWorld(world);b3DestroyHull(rock);b3DestroyMesh(terrain.m_mesh);return 0;
}
