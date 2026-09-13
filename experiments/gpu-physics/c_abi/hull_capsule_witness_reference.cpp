// Gear Lift step-414 hull/capsule witness and geometric controls.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cstdio>
#include <vector>
static void run(int variant) {
b3Vec3 vertices[] = {{-0.0600001365f,-1.22000015f,-1.375f},{0.0899999291f,-1.00000012f,-1.375f},{0.0599998832f,-1.22000015f,-1.625f},{0.0899999291f,-1.00000012f,-1.625f},{-0.0900001228f,-1.00000012f,-1.375f},{-0.0600001365f,-1.22000015f,-1.625f},{0.0599998832f,-1.22000015f,-1.375f},{-0.0900001228f,-1.00000012f,-1.625f}};
b3Pos ap={-2.24996972f,10.7498789f,7.92338994e-09f}, bp={-1.17941892f,11.3904905f,-1.49985588f};
b3Quat aq={{-6.9523999e-08f,2.15284373e-07f,0.833516657f},0.552494287f}, bq={{0.00019517928f,-0.00102875591f,0.280096263f},0.959971249f};
b3Capsule cap={{0.0f,-0.0700000003f,0.0f},{0.0f,0.0700000003f,0.0f},0.0500000007f};
if(variant>=3){for(int i=0;i<8;i++)vertices[i]={i&1?1.f:-1.f,i&2?1.f:-1.f,i&4?1.f:-1.f};ap={0,0,0};aq=b3Quat_identity;bq=b3Quat_identity;cap={{-.07f,0,0},{.07f,0,0},.05f};bp={0,1.049f,0};}
if(variant==4 || variant==5){cap={{0,0,-.07f},{0,0,.07f},.05f};bp=variant==4?b3Pos{1.03f,1.04f,0}:b3Pos{1.06f,1.06f,0};}
if(variant==6)bp={0,0,0};
if(variant==2){auto q=b3MakeQuatFromAxisAngle(b3Normalize({1,2,3}),.7f);auto x=b3RotateVector(q,{float(ap.x),float(ap.y),float(ap.z)});auto y=b3RotateVector(q,{float(bp.x),float(bp.y),float(bp.z)});ap={x.x+2,x.y-1,x.z+3};bp={y.x+2,y.y-1,y.z+3};aq=b3MulQuat(q,aq);bq=b3MulQuat(q,bq);}
auto wd=b3DefaultWorldDef();wd.gravity=b3Vec3_zero;wd.enableSleep=false;wd.enableContinuous=false;auto w=b3CreateWorld(&wd);
auto ad=b3DefaultBodyDef();ad.position=ap;ad.rotation=aq;
auto bd=b3DefaultBodyDef();bd.type=b3_dynamicBody;bd.position=bp;bd.rotation=bq;
auto hull=b3CreateHull(vertices,8,8);auto sd=b3DefaultShapeDef();b3BodyId ha,cb;
if(variant==1){cb=b3CreateBody(w,&bd);b3CreateCapsuleShape(cb,&sd,&cap);ha=b3CreateBody(w,&ad);b3CreateHullShape(ha,&sd,hull);}
else{ha=b3CreateBody(w,&ad);b3CreateHullShape(ha,&sd,hull);cb=b3CreateBody(w,&bd);b3CreateCapsuleShape(cb,&sd,&cap);}
b3World_Step(w,1e-7f,4);int capacity=b3Body_GetContactCapacity(cb);std::vector<b3ContactData> cs(capacity);int n=b3Body_GetContactData(cb,cs.data(),capacity);
printf("{\"case\":%d,\"points\":[",variant);bool first=true;
for(int i=0;i<n;i++)for(int j=0;j<cs[i].manifoldCount;j++){auto m=cs[i].manifolds[j];bool ca=b3Shape_GetType(cs[i].shapeIdA)==b3_capsuleShape;auto com=b3Body_GetWorldCenter(cb);auto normal=ca?b3Neg(m.normal):m.normal;
for(int k=0;k<m.pointCount;k++){auto pt=m.points[k];auto r=ca?pt.anchorA:pt.anchorB;printf("%s{\"p\":[%.9g,%.9g,%.9g],\"n\":[%.9g,%.9g,%.9g],\"s\":%.9g}",first?"":",",double(com.x+r.x),double(com.y+r.y),double(com.z+r.z),normal.x,normal.y,normal.z,pt.separation);first=false;}}
printf("]}\n");b3DestroyWorld(w);b3DestroyHull(hull);
}
int main(){for(int i=0;i<7;i++)run(i);}
