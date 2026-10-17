// Read-only inputs and independent final overlay sinks, scoped to world draw.
#include "box3d/box3d.h"
#include "gfx/overlay.h"
#include "gfx/text.h"
#include "gfx/draw.h"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <string>
#include <vector>
namespace {
struct Capture { std::vector<std::string> inputs, sinks; b3Pos origin; bool truncated=false; };
thread_local Capture* active;
void add(bool sink,const char* format,...){if(!active)return;auto& rows=sink?active->sinks:active->inputs;if(rows.size()>=512){active->truncated=true;return;}char text[4096];va_list ap;va_start(ap,format);vsnprintf(text,sizeof(text),format,ap);va_end(ap);rows.emplace_back(text);}
std::string quoted(const char* s){std::string r="\"";for(;s&&*s;++s){unsigned char c=*s;if(c=='"'||c=='\\'){r+='\\';r+=c;}else if(c>=32)r+=c;}return r+'"';}
unsigned request(){const char* p=getenv("GPU_PR10_UI_TRACE");if(!p)return 0;FILE* f=fopen((std::string(p)+".request").c_str(),"r");unsigned n=0;if(f){fscanf(f,"%u",&n);fclose(f);}return n;}
void output(Capture& c,const char* owner,b3WorldId id,unsigned req){const char* p=getenv("GPU_PR10_API_TRACE");if(!p)return;FILE* f=fopen(p,"a");if(!f)return;fprintf(f,"{\"api\":\"overlay_boundary\",\"owner\":\"%s\",\"request\":%u,\"world\":%u,\"origin\":[%.9g,%.9g,%.9g],\"truncated\":%s,\"inputs\":[",owner,req,id.index1,(double)c.origin.x,(double)c.origin.y,(double)c.origin.z,c.truncated?"true":"false");for(size_t i=0;i<c.inputs.size();++i)fprintf(f,"%s%s",i?",":"",c.inputs[i].c_str());fputs("],\"sinks\":[",f);for(size_t i=0;i<c.sinks.size();++i)fprintf(f,"%s%s",i?",":"",c.sinks[i].c_str());fputs("]}\n",f);fclose(f);}
template<class F>void scope(const char* owner,b3WorldId id,b3DebugDraw* d,uint64_t mask,F call){static unsigned last_by_owner[2]={0,0};unsigned& last=last_by_owner[owner[0]=='g'?1:0];unsigned req=request();bool enabled=getenv("GPU_PR10_OVERLAY_BOUNDARY")&&req&&req!=last;Capture c;Capture* prior=active;if(enabled){c.origin=GetDrawOrigin();active=&c;add(false,"{\"op\":\"flags\",\"shapes\":%d,\"bounds\":%d,\"mass\":%d,\"names\":%d,\"joints\":%d,\"extras\":%d,\"contacts\":%d,\"anchorA\":%d,\"normals\":%d,\"features\":%d,\"forces\":%d,\"jointScale\":%.9g,\"forceScale\":%.9g}",d->drawShapes,d->drawBounds,d->drawMass,d->drawBodyNames,d->drawJoints,d->drawJointExtras,d->drawContacts,d->drawAnchorA,d->drawContactNormals,d->drawContactFeatures,d->drawContactForces,d->jointScale,d->forceScale);}else active=nullptr;call(id,d,mask);active=prior;if(enabled){output(c,owner,id,req);last=req;}}
}
extern "C" {
void ui03_previous_b3World_Draw(b3WorldId,b3DebugDraw*,uint64_t);
void ui03_previous_gpu_samples_world_draw(b3WorldId,b3DebugDraw*,uint64_t);
void __wrap_b3World_Draw(b3WorldId id,b3DebugDraw* d,uint64_t mask){scope("public",id,d,mask,ui03_previous_b3World_Draw);}
void __wrap_gpu_samples_world_draw(b3WorldId id,b3DebugDraw* d,uint64_t mask){scope("gpu",id,d,mask,ui03_previous_gpu_samples_world_draw);}
void __real_OverlayAppendLine(b3Vec3,b3Vec3,Vec4,float,OverlayThicknessUnit,OverlayOcclusionMode);
void __wrap_OverlayAppendLine(b3Vec3 a,b3Vec3 b,Vec4 c,float size,OverlayThicknessUnit unit,OverlayOcclusionMode occlusion){add(true,"{\"op\":\"line\",\"a\":[%.9g,%.9g,%.9g],\"b\":[%.9g,%.9g,%.9g]}",a.x,a.y,a.z,b.x,b.y,b.z);__real_OverlayAppendLine(a,b,c,size,unit,occlusion);}
void __real_OverlayAppendPoint(b3Vec3,Vec4,float,OverlayThicknessUnit,OverlayOcclusionMode);
void __wrap_OverlayAppendPoint(b3Vec3 p,Vec4 c,float size,OverlayThicknessUnit unit,OverlayOcclusionMode occlusion){add(true,"{\"op\":\"point\",\"p\":[%.9g,%.9g,%.9g],\"size\":%.9g}",p.x,p.y,p.z,size);__real_OverlayAppendPoint(p,c,size,unit,occlusion);}
void __real_DrawString(b3Vec3,Vec4,const char*);
void __wrap_DrawString(b3Vec3 p,Vec4 c,const char* text){add(true,"{\"op\":\"text\",\"p\":[%.9g,%.9g,%.9g],\"text\":%s}",p.x,p.y,p.z,quoted(text).c_str());__real_DrawString(p,c,text);}
void __real_gpu_b3_body_get_transform(b3BodyId,float*,float*) __attribute__((weak));
void __wrap_gpu_b3_body_get_transform(b3BodyId id,float* p,float* q){__real_gpu_b3_body_get_transform(id,p,q);add(false,"{\"op\":\"transform\",\"id\":[%d,%u,%u],\"p\":[%.9g,%.9g,%.9g],\"q\":[%.9g,%.9g,%.9g,%.9g]}",id.index1,id.world0,id.generation,p[0],p[1],p[2],q[0],q[1],q[2],q[3]);}
void __real_gpu_b3_body_get_world_center(b3BodyId,float*) __attribute__((weak));
void __wrap_gpu_b3_body_get_world_center(b3BodyId id,float* p){__real_gpu_b3_body_get_world_center(id,p);add(false,"{\"op\":\"center\",\"id\":[%d,%u,%u],\"p\":[%.9g,%.9g,%.9g]}",id.index1,id.world0,id.generation,p[0],p[1],p[2]);}
b3AABB __real_gpu_b3_shape_get_aabb(b3ShapeId) __attribute__((weak));
b3AABB __wrap_gpu_b3_shape_get_aabb(b3ShapeId id){auto a=__real_gpu_b3_shape_get_aabb(id);add(false,"{\"op\":\"aabb\",\"id\":[%d,%u,%u],\"min\":[%.9g,%.9g,%.9g],\"max\":[%.9g,%.9g,%.9g]}",id.index1,id.world0,id.generation,a.lowerBound.x,a.lowerBound.y,a.lowerBound.z,a.upperBound.x,a.upperBound.y,a.upperBound.z);return a;}
const char* __real_gpu_b3_body_get_name(b3BodyId) __attribute__((weak));
const char* __wrap_gpu_b3_body_get_name(b3BodyId id){const char* s=__real_gpu_b3_body_get_name(id);add(false,"{\"op\":\"name\",\"id\":[%d,%u,%u],\"text\":%s}",id.index1,id.world0,id.generation,quoted(s).c_str());return s;}
float __real_gpu_b3_body_get_mass(b3BodyId) __attribute__((weak));
float __wrap_gpu_b3_body_get_mass(b3BodyId id){float v=__real_gpu_b3_body_get_mass(id);add(false,"{\"op\":\"mass\",\"id\":[%d,%u,%u],\"value\":%.9g}",id.index1,id.world0,id.generation,v);return v;}
void __real_gpu_b3_joint_get_local_frame(b3JointId,bool,float*) __attribute__((weak));
void __wrap_gpu_b3_joint_get_local_frame(b3JointId id,bool side,float* f){__real_gpu_b3_joint_get_local_frame(id,side,f);add(false,"{\"op\":\"joint_frame\",\"id\":[%d,%u,%u],\"side\":%d,\"p\":[%.9g,%.9g,%.9g],\"q\":[%.9g,%.9g,%.9g,%.9g]}",id.index1,id.world0,id.generation,side,f[0],f[1],f[2],f[3],f[4],f[5],f[6]);}
b3BodyId __real_gpu_b3_joint_get_body(b3JointId,bool) __attribute__((weak));
b3BodyId __wrap_gpu_b3_joint_get_body(b3JointId id,bool side){auto b=__real_gpu_b3_joint_get_body(id,side);add(false,"{\"op\":\"joint_body\",\"id\":[%d,%u,%u],\"side\":%d,\"body\":[%d,%u,%u]}",id.index1,id.world0,id.generation,side,b.index1,b.world0,b.generation);return b;}
#define REACTION(NAME,OP) void __real_##NAME(b3JointId,float*) __attribute__((weak));void __wrap_##NAME(b3JointId id,float* v){__real_##NAME(id,v);add(false,"{\"op\":\"" OP "\",\"id\":[%d,%u,%u],\"v\":[%.9g,%.9g,%.9g]}",id.index1,id.world0,id.generation,v[0],v[1],v[2]);}
REACTION(gpu_b3_joint_get_constraint_force,"joint_force")
REACTION(gpu_b3_joint_get_constraint_torque,"joint_torque")
int __real_b3Body_GetContactData(b3BodyId,b3ContactData*,int);
int __wrap_b3Body_GetContactData(b3BodyId id,b3ContactData* out,int cap){int n=__real_b3Body_GetContactData(id,out,cap);for(int i=0;active&&i<n;++i){auto& c=out[i];for(int j=0;j<c.manifoldCount;++j){const auto& m=c.manifolds[j];add(false,"{\"op\":\"manifold\",\"id\":[%d,%u,%u],\"shapes\":[[%d,%u,%u],[%d,%u,%u]],\"index\":%d,\"count\":%d,\"normal\":[%.9g,%.9g,%.9g],\"friction\":[%.9g,%.9g,%.9g]}",c.contactId.index1,c.contactId.world0,c.contactId.generation,c.shapeIdA.index1,c.shapeIdA.world0,c.shapeIdA.generation,c.shapeIdB.index1,c.shapeIdB.world0,c.shapeIdB.generation,j,m.pointCount,m.normal.x,m.normal.y,m.normal.z,m.frictionImpulse.x,m.frictionImpulse.y,m.frictionImpulse.z);for(int k=0;k<m.pointCount;++k){const auto& p=m.points[k];add(false,"{\"op\":\"contact_point\",\"id\":[%d,%u,%u],\"manifold\":%d,\"index\":%d,\"a\":[%.9g,%.9g,%.9g],\"b\":[%.9g,%.9g,%.9g],\"separation\":%.9g,\"impulse\":%.9g,\"feature\":%u,\"persisted\":%s}",c.contactId.index1,c.contactId.world0,c.contactId.generation,j,k,p.anchorA.x,p.anchorA.y,p.anchorA.z,p.anchorB.x,p.anchorB.y,p.anchorB.z,p.separation,p.totalNormalImpulse,p.featureId,p.persisted?"true":"false");}}}return n;}
}
