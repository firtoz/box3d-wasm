// Frozen PR10 failures and independent query boundary controls. No simulation.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cstdio>
#include <cstdint>
#include <initializer_list>
#include <cmath>
#include <cstdlib>
#include <cstring>

#ifdef PR11_GPU
// The viewer defines this one-line public wrapper in samples_api.c. Headless
// queries deliberately omit that file's rendering metadata and callbacks.
extern "C" b3BodyId gpu_b3_shape_get_body(b3ShapeId);
extern "C" b3BodyId b3Shape_GetBody(b3ShapeId id) { return gpu_b3_shape_get_body(id); }
#endif

struct Context { const char* name; b3ShapeId shape; b3BodyId body; int calls=0; };
static bool overlap(b3ShapeId id, void* p) {
  auto& c=*static_cast<Context*>(p); ++c.calls;
  printf("{\"case\":\"%s\",\"kind\":\"overlap\",\"identity\":%d}\n", c.name,
    B3_ID_EQUALS(id,c.shape)&&B3_ID_EQUALS(b3Shape_GetBody(id),c.body));
  return true;
}
static float cast(b3ShapeId id,b3Pos p,b3Vec3 n,float f,uint64_t material,int triangle,int child,void* data) {
  auto& c=*static_cast<Context*>(data); ++c.calls;
  printf("{\"case\":\"%s\",\"kind\":\"cast\",\"identity\":%d,\"fraction\":%.9g,\"point\":[%.9g,%.9g,%.9g],\"normal\":[%.9g,%.9g,%.9g],\"material\":%llu,\"triangle\":%d,\"child\":%d}\n",c.name,
    B3_ID_EQUALS(id,c.shape)&&B3_ID_EQUALS(b3Shape_GetBody(id),c.body),f,p.x,p.y,p.z,n.x,n.y,n.z,(unsigned long long)material,triangle,child);
  return f;
}
static void summary(const Context& c) { printf("{\"case\":\"%s\",\"kind\":\"summary\",\"calls\":%d}\n",c.name,c.calls); }
static b3WorldId world() {auto d=b3DefaultWorldDef();d.gravity=b3Vec3_zero;return b3CreateWorld(&d);}
static void initial_overlap() {
  auto w=world(); auto bd=b3DefaultBodyDef();
  bd.rotation=b3MakeQuatFromAxisAngle(b3Vec3_axisZ,10.f*B3_DEG_TO_RAD);
  auto b=b3CreateBody(w,&bd);auto sd=b3DefaultShapeDef();
  int indices[]={0,1,2,2,3,0};
  b3Vec3 vertices[]={{-.5f,.5f,.5f},{-.5f,.5f,-.5f},{-.5f,-.5f,-.5f},{-.5f,-.5f,.5f}};
  b3MeshDef def={};def.triangleCount=2;def.indices=indices;def.vertexCount=4;def.vertices=vertices;
  auto mesh=b3CreateMesh(&def,nullptr,0);auto s=b3CreateMeshShape(b,&sd,mesh,{4,4,4});
  b3Vec3 points[]={{-2.1f,-.8f,.95f},{-2.1f,.2f,.95f}};b3ShapeProxy proxy={points,2,.25f};
  Context c={"initial-overlap",s,b};b3World_CastShape(w,b3Pos_zero,&proxy,b3Vec3_zero,b3DefaultQueryFilter(),cast,&c);summary(c);
  b3DestroyWorld(w);b3DestroyMesh(mesh);
}
static void shifted_overlap() {
  auto w=world();auto bd=b3DefaultBodyDef();auto sd=b3DefaultShapeDef();
  auto box=b3MakeBoxHull(.6f,.6f,.6f);auto mesh=b3CreateTorusMesh(10,12,.65f,.35f);
  auto field=b3CreateWave(10,10,{.2f,.2f,.2f},.03f,.09f,false);
  b3ShapeId expected={};b3BodyId expectedBody={};
  for(int i=0;i<3;++i) {
    bd.type=b3BodyType(i);bd.position={-6,3.f+2*i,0};bd.rotation=b3MakeQuatFromAxisAngle(b3Vec3_axisX,.5f*B3_PI);
    auto b=b3CreateBody(w,&bd);b3Sphere sphere={b3Vec3_zero,.8f};b3CreateSphereShape(b,&sd,&sphere);
    bd.position={-3,3.f+2*i,0};bd.rotation=b3MakeQuatFromAxisAngle(b3Vec3_axisZ,.25f*B3_PI);
    b=b3CreateBody(w,&bd);b3Capsule capsule={{-.5f,0,0},{.5f,0,0},.5f};b3CreateCapsuleShape(b,&sd,&capsule);
    bd.position={0,3.f+2*i,0};b=b3CreateBody(w,&bd);b3CreateHullShape(b,&sd,&box.base);
    bd.position={3,3.f+2*i,0};bd.rotation=b3MakeQuatFromAxisAngle(b3Vec3_axisX,.5f*B3_PI);
    b=b3CreateBody(w,&bd);b3CreateMeshShape(b,&sd,mesh,{-.5f,1.5f,-1});
    bd.type=b3_staticBody;bd.position={5,2.f+2*i,0};bd.rotation=b3MakeQuatFromAxisAngle(b3Vec3_axisX,-.5f*B3_PI);
    b=b3CreateBody(w,&bd);auto s=b3CreateHeightFieldShape(b,&sd,field);if(i==2){expected=s;expectedBody=b;}
  }
  b3Vec3 points[]={{6.3f,7.3f,.3f},{5.7f,7.3f,.3f},{5.7f,6.7f,.3f},{6.3f,6.7f,.3f},{6.3f,7.3f,-.3f},{5.7f,7.3f,-.3f},{5.7f,6.7f,-.3f},{6.3f,6.7f,-.3f}};
  b3ShapeProxy proxy={points,8,0};Context c={"shifted-overlap",expected,expectedBody};
  b3World_OverlapShape(w,b3Pos_zero,&proxy,b3DefaultQueryFilter(),overlap,&c);summary(c);
  b3DestroyWorld(w);b3DestroyMesh(mesh);b3DestroyHeightField(field);
}
static void boundaries() {
  auto w=world();auto bd=b3DefaultBodyDef();auto b=b3CreateBody(w,&bd);auto sd=b3DefaultShapeDef();
  int indices[]={0,1,2,2,3,0};b3Vec3 vertices[]={{-2,0,-2},{-2,0,2},{2,0,2},{2,0,-2}};
  b3MeshDef def={};def.triangleCount=2;def.indices=indices;def.vertexCount=4;def.vertices=vertices;
  auto mesh=b3CreateMesh(&def,nullptr,0);auto s=b3CreateMeshShape(b,&sd,mesh,b3Vec3_one);
  for(int side:{-1,1}) for(int moving:{0,1}) {
    char name[80];snprintf(name,sizeof(name),"plane-%s-%s",side<0?"back":"front",moving?"moving":"stationary");
    b3Vec3 p={.3f,side*(moving?1.f:.1f),.2f};b3ShapeProxy proxy={&p,1,.25f};Context c={name,s,b};
    b3World_CastShape(w,b3Pos_zero,&proxy,{0,side*(moving?-2.f:0.f),0},b3DefaultQueryFilter(),cast,&c);summary(c);
    c.calls=0;b3World_OverlapShape(w,b3Pos_zero,&proxy,b3DefaultQueryFilter(),overlap,&c);summary(c);
  }
  b3DestroyWorld(w);b3DestroyMesh(mesh);
  w=world();b=b3CreateBody(w,&bd);b3Sphere sphere={b3Vec3_zero,1};s=b3CreateSphereShape(b,&sd,&sphere);
  for(uint64_t mask:{uint64_t(1),uint64_t(2),uint64_t(1)<<63}) {
    char name[80];snprintf(name,sizeof(name),"filter-%llu",(unsigned long long)mask);
    auto filter=b3DefaultQueryFilter();filter.maskBits=mask;Context c={name,s,b};
    b3World_CastRay(w,{-3,0,0},{6,0,0},filter,cast,&c);summary(c);
  }
  b3DestroyWorld(w);
}
static void closest(const char* name,b3WorldId w,b3ShapeId s,b3BodyId b,b3Vec3 origin,b3Vec3 delta,b3QueryFilter filter) {
  auto r=b3World_CastRayClosest(w,origin,delta,filter);
  printf("{\"case\":\"%s\",\"kind\":\"closest\",\"hit\":%d",name,r.hit);
  if(r.hit) printf(",\"identity\":%d,\"fraction\":%.9g,\"point\":[%.9g,%.9g,%.9g],\"normal\":[%.9g,%.9g,%.9g],\"triangle\":%d",B3_ID_EQUALS(r.shapeId,s)&&B3_ID_EQUALS(b3Shape_GetBody(r.shapeId),b),r.fraction,r.point.x,r.point.y,r.point.z,r.normal.x,r.normal.y,r.normal.z,r.triangleIndex);
  puts("}");
}
static void ray_contract() {
  for(int type=0;type<5;++type) for(int pose=0;pose<3;++pose) {
    auto w=world();auto bd=b3DefaultBodyDef();
    bd.position=pose?b3Pos{3,-2,4}:b3Pos_zero;
    bd.rotation=pose?b3MakeQuatFromAxisAngle(b3Normalize({1,2,-1}),.7f*pose):b3Quat_identity;
    auto b=b3CreateBody(w,&bd);auto sd=b3DefaultShapeDef();sd.baseMaterial.userMaterialId=1234;
    b3ShapeId s={};b3MeshData* mesh=nullptr;b3HeightFieldData* field=nullptr;
    if(type==0){b3Sphere g={{.2f,0,0},1};s=b3CreateSphereShape(b,&sd,&g);}
    if(type==1){b3Capsule g={{-.5f,0,0},{.5f,0,0},.5f};s=b3CreateCapsuleShape(b,&sd,&g);}
    if(type==2){auto g=b3MakeBoxHull(.8f,.6f,.7f);s=b3CreateHullShape(b,&sd,&g.base);}
    if(type==3){mesh=b3CreateTorusMesh(10,12,.65f,.35f);s=b3CreateMeshShape(b,&sd,mesh,{-.5f,1.5f,-1});}
    if(type==4){field=b3CreateWave(10,10,{.2f,.2f,.2f},.03f,.09f,false);s=b3CreateHeightFieldShape(b,&sd,field);}
    b3Transform xf={bd.position,bd.rotation};
    for(int i=0;i<12;++i) {
      b3Vec3 local={-.8f+.15f*i,3,-.31f+.07f*i};auto o=b3TransformPoint(xf,local);auto d=b3RotateVector(xf.q,{0,-6,0});
      char name[80];snprintf(name,sizeof(name),"ray-%d-%d-%d",type,pose,i);
      Context c={name,s,b};b3World_CastRay(w,o,d,b3DefaultQueryFilter(),cast,&c);summary(c);
      closest(name,w,s,b,o,d,b3DefaultQueryFilter());
    }
    // Teleport/refit, excluded masks, zero segment, finite misses and deletion.
    b3Body_SetTransform(b,{50,50,50},b3Quat_identity);
    closest("teleport-miss",w,s,b,{0,3,0},{0,-6,0},b3DefaultQueryFilter());
    closest("zero-miss",w,s,b,{0,3,0},b3Vec3_zero,b3DefaultQueryFilter());
    auto filter=b3DefaultQueryFilter();filter.maskBits=0;
    closest("filtered-miss",w,s,b,{50,53,50},{0,-6,0},filter);
    b3DestroyBody(b);closest("destroyed-miss",w,s,b,{50,53,50},{0,-6,0},b3DefaultQueryFilter());
    printf("{\"case\":\"destroyed-identity\",\"valid\":%d}\n",b3Shape_IsValid(s)||b3Body_IsValid(b));
    b3DestroyWorld(w);if(mesh)b3DestroyMesh(mesh);if(field)b3DestroyHeightField(field);
  }
  // Two collinear bodies establish nearest selection independently of creation order.
  for(int reverse=0;reverse<2;++reverse) {
    auto w=world();b3BodyId nearBody={};b3ShapeId nearShape={};
    for(int i=0;i<2;++i) {int side=reverse?1-i:i;auto bd=b3DefaultBodyDef();bd.position={4.f*side,0,0};auto b=b3CreateBody(w,&bd);auto sd=b3DefaultShapeDef();b3Sphere sphere={b3Vec3_zero,1};auto s=b3CreateSphereShape(b,&sd,&sphere);if(!side){nearBody=b;nearShape=s;}}
    closest("nearest",w,nearShape,nearBody,{-3,0,0},{10,0,0},b3DefaultQueryFilter());b3DestroyWorld(w);
  }
}
static void frozen_heightfield(const char* path) {
  FILE* f=fopen(path,"r");if(!f)exit(2);
  auto w=world();auto bd=b3DefaultBodyDef();bd.position={-25,0,-25};auto b=b3CreateBody(w,&bd);auto sd=b3DefaultShapeDef();
  auto field=b3CreateWave(50,50,b3Vec3_one,.02f,.04f,true);auto s=b3CreateHeightFieldShape(b,&sd,field);
  int index;float x,y,z,r,dx,dy,dz;
  while(fscanf(f,"%d %f %f %f %f %f %f %f",&index,&x,&y,&z,&r,&dx,&dy,&dz)==8) {
    char name[80];snprintf(name,sizeof(name),"heightfield-%d",index);b3Vec3 p={x,y,z};b3ShapeProxy proxy={&p,1,r};Context c={name,s,b};
    b3World_CastShape(w,b3Pos_zero,&proxy,{dx,dy,dz},b3DefaultQueryFilter(),cast,&c);summary(c);
  }
  fclose(f);b3DestroyWorld(w);b3DestroyHeightField(field);
}
static void body_cast_sample() {
  auto w=world();auto bd=b3DefaultBodyDef();bd.type=b3_kinematicBody;bd.position={5,5,0};bd.angularVelocity={.1f,-.1f,.1f};auto b=b3CreateBody(w,&bd);auto sd=b3DefaultShapeDef();
  auto hull=b3CreateCylinder(2,.5f,0,16);auto shape=b3CreateHullShape(b,&sd,hull);
  for(int pose=0;pose<2;++pose) {
    b3WorldTransform xf={{-10,2,0},b3MakeQuatFromAxisAngle(b3Normalize({1,-2,3}),.75f*B3_PI)};
    if(pose)xf={{-10.123953819274902f,1.7929139137268066f,-.4998455047607422f},{.246802583f,-.493605167f,.740407705f,.38371706f}};
    b3Vec3 point=b3Vec3_zero;b3ShapeProxy proxy={&point,1,.2f};
    auto ray=b3Body_CastRay(b,{-9.75f,3,-4},{0,0,8},b3DefaultQueryFilter(),1,xf);
    auto cast=b3Body_CastShape(b,{-14.5f,2.5f,.5f},&proxy,{8,0,0},b3DefaultQueryFilter(),1,true,xf);
    for(int type=0;type<2;++type) {
      auto r=type?cast:ray;printf("{\"case\":\"body-cast-%d-%d\",\"kind\":\"body\",\"hit\":%d",pose,type,r.hit);
      if(r.hit)printf(",\"identity\":%d,\"fraction\":%.9g,\"point\":[%.9g,%.9g,%.9g],\"normal\":[%.9g,%.9g,%.9g]",B3_ID_EQUALS(r.shapeId,shape)&&B3_ID_EQUALS(b3Shape_GetBody(r.shapeId),b),r.fraction,r.point.x,r.point.y,r.point.z,r.normal.x,r.normal.y,r.normal.z);
      puts("}");
    }
  }
  b3DestroyWorld(w);b3DestroyHull(hull);
}
static void encroachment() {
  for(int pose=0;pose<2;++pose){
    auto w=world();auto bd=b3DefaultBodyDef();auto b=b3CreateBody(w,&bd);auto sd=b3DefaultShapeDef();
    int indices[]={0,1,2,2,3,0};b3Vec3 vertices[]={{-2,0,-2},{-2,0,2},{2,0,2},{2,0,-2}};
    b3MeshDef def={};def.triangleCount=2;def.indices=indices;def.vertexCount=4;def.vertices=vertices;
    auto mesh=b3CreateMesh(&def,nullptr,0);auto s=b3CreateMeshShape(b,&sd,mesh,pose?b3Vec3{-1,.5f,2}:b3Vec3_one);
    b3WorldTransform xf=pose?b3WorldTransform{{3,-2,4},b3MakeQuatFromAxisAngle(b3Normalize({1,2,-1}),.7f)}:b3WorldTransform_identity;
    int i=0;for(int side:{-1,1})for(float height:{.001f,.1f,1.f})for(int motion:{-1,0,1})for(bool encroach:{false,true})for(float max:{0.f,.1f,1.f}){
      b3Vec3 p={.3f,side*height,.2f};p=b3TransformWorldPoint(xf,p);b3ShapeProxy proxy={&p,1,.25f};
      auto r=b3Body_CastShape(b,b3Pos_zero,&proxy,b3RotateVector(xf.q,{0,2.f*side*motion,0}),b3DefaultQueryFilter(),max,encroach,xf);
      printf("{\"case\":\"encroach-%d-%d\",\"kind\":\"body\",\"hit\":%d",pose,i++,r.hit);
      if(r.hit)printf(",\"identity\":%d,\"fraction\":%.9g,\"point\":[%.9g,%.9g,%.9g],\"normal\":[%.9g,%.9g,%.9g]",B3_ID_EQUALS(r.shapeId,s)&&B3_ID_EQUALS(b3Shape_GetBody(r.shapeId),b),r.fraction,r.point.x,r.point.y,r.point.z,r.normal.x,r.normal.y,r.normal.z);puts("}");
    }
    b3DestroyWorld(w);b3DestroyMesh(mesh);
  }
}
int main(int argc,char** argv){ setvbuf(stdout,nullptr,_IOLBF,0);if(argc>1){if(!std::strcmp(argv[1],"encroach"))encroachment();else frozen_heightfield(argv[1]);return 0;}auto f=b3DefaultFilter();printf("{\"case\":\"default-filter\",\"category\":\"%llu\",\"mask\":\"%llu\"}\n",(unsigned long long)f.categoryBits,(unsigned long long)f.maskBits);initial_overlap();shifted_overlap();boundaries();ray_contract();body_cast_sample();}
