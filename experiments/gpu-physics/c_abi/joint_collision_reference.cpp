#include "box3d/box3d.h"
#include <cstdio>
int main() {
 auto wd=b3DefaultWorldDef();wd.gravity={0,0,0};auto w=b3CreateWorld(&wd);
 auto bd=b3DefaultBodyDef();auto a=b3CreateBody(w,&bd);bd.type=b3_dynamicBody;bd.position={0,0.9f,0};auto b=b3CreateBody(w,&bd);
 auto sd=b3DefaultShapeDef();sd.enableContactEvents=true;b3Sphere sphere={{0,0,0},0.5f};b3CreateSphereShape(a,&sd,&sphere);b3CreateSphereShape(b,&sd,&sphere);
 auto jd=b3DefaultFilterJointDef();jd.base.bodyIdA=a;jd.base.bodyIdB=b;auto j=b3CreateFilterJoint(w,&jd);auto other=b3CreateFilterJoint(w,&jd);
 b3ContactId previous={};
 auto observe=[&](const char* phase){b3ContactData data[8];int count=b3Body_GetContactData(b,data,8);auto events=b3World_GetContactEvents(w);
  std::printf("%s %d %d %d %d %d %d\n",phase,b3Joint_GetCollideConnected(j),b3Body_GetContactCapacity(b),count,b3Contact_IsValid(previous),events.beginCount,events.endCount);
  if(count)previous=data[0].contactId;
 };
 b3World_Step(w,1.f/60,4);observe("disabled");
 b3Joint_SetCollideConnected(j,true);observe("enable-before-step");
 b3World_Step(w,1.f/60,4);observe("other-veto");
 b3Joint_SetCollideConnected(other,true);
 b3World_Step(w,1.f/60,4);observe("enabled");
 b3Joint_SetCollideConnected(j,true);observe("enabled-noop");
 b3Joint_SetCollideConnected(j,false);observe("disable-immediate");
 b3Joint_SetCollideConnected(j,true);observe("reenable-immediate");
 b3World_Step(w,1.f/60,4);observe("reenabled-step");
 b3DestroyWorld(w);
}
