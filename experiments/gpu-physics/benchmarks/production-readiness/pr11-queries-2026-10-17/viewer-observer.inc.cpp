#include "box3d/box3d.h"
#include "gfx/draw.h"
#include "gfx/renderer.h"
#include "gfx/projection.h"
#include "sample.h"
#include <sstream>
#include <iomanip>
namespace pr11 {
Sample* sample=nullptr;
int last_frame=-1;
std::vector<std::string> draws,queries;
void begin(){int f=ImGui::GetFrameCount();if(f!=last_frame){draws.clear();queries.clear();last_frame=f;}}
std::string v(b3Vec3 p){std::ostringstream s;s<<std::setprecision(9)<<'['<<p.x<<','<<p.y<<','<<p.z<<']';return s.str();}
std::string color(Vec4 p){std::ostringstream s;s<<std::setprecision(9)<<'['<<p.x<<','<<p.y<<','<<p.z<<','<<p.w<<']';return s.str();}
void draw(const char* kind,b3Pos a,b3Pos b,Vec4 c){begin();if(draws.size()<2048)draws.push_back("{\"kind\":\""+std::string(kind)+"\",\"a\":"+v(a)+",\"b\":"+v(b)+",\"color\":"+color(c)+"}");}
void query(const char* kind,bool hit,b3Pos point,b3Vec3 normal,float fraction){begin();std::ostringstream s;s<<std::setprecision(9)<<"{\"kind\":\""<<kind<<"\",\"hit\":"<<hit;if(hit)s<<",\"point\":"<<v(point)<<",\"normal\":"<<v(normal)<<",\"fraction\":"<<fraction;s<<'}';if(queries.size()<2048)queries.push_back(s.str());}
struct Callback {b3CastResultFcn* f;void* data;const char* kind;};
float callback(b3ShapeId id,b3Pos p,b3Vec3 n,float f,uint64_t m,int t,int c,void* x){auto& a=*(Callback*)x;query(a.kind,true,p,n,f);return a.f(id,p,n,f,m,t,c,a.data);}
void array(FILE* f,const std::vector<std::string>& a){fputc('[',f);for(size_t i=0;i<a.size();++i){if(i)fputc(',',f);fputs(a[i].c_str(),f);}fputc(']',f);}
void matrix(FILE* f,Mat4 m){auto* p=(float*)&m;fputc('[',f);for(int i=0;i<16;++i)fprintf(f,"%s%.9g",i?",":"",p[i]);fputc(']',f);}
void flush(const FrameInput* frame){
 if(sample)if(const char* path=getenv("GPU_VIEWER_CONTROL_OBSERVER"))if(FILE* f=fopen(path,"a")){
  auto* ctx=sample->m_context;auto c=b3World_GetCounters(sample->m_worldId);
  fprintf(f,"{\"frame\":%d,\"world\":[%u,%u],\"pause\":%d,\"hertz\":%.9g,\"substeps\":%d,\"body_count\":%d,\"shape_count\":%d,\"sample_step\":%d}\n",ImGui::GetFrameCount(),sample->m_worldId.index1,sample->m_worldId.generation,ctx->pause,ctx->hertz,ctx->subStepCount,c.bodyCount,c.shapeCount,sample->m_stepCount);fclose(f);
 }
 const char* path=getenv("GPU_PR10_API_TRACE"),*ui=getenv("GPU_PR10_UI_TRACE");if(!path||!ui)return;
 unsigned request=0;auto requestPath=std::string(ui)+".request";FILE* in=fopen(requestPath.c_str(),"r");if(!in)return;fscanf(in,"%u",&request);fclose(in);static unsigned last=0;if(!request||request==last)return;last=request;
 auto camera=GetCameraState();FILE* f=fopen(path,"a");if(!f)return;
 fprintf(f,"{\"api\":\"pr11_frame\",\"request\":%u,\"frame\":%d,\"viewport\":[%d,%d],\"origin\":%s,\"view\":",request,last_frame,camera.viewportW,camera.viewportH,v(frame->drawOrigin).c_str());matrix(f,camera.view);fputs(",\"proj\":",f);matrix(f,camera.proj);fputs(",\"draws\":",f);array(f,draws);fputs(",\"queries\":",f);array(f,queries);
 // Projection is checked independently from the retained camera matrices.
 fputs("}\n",f);fclose(f);
}
extern void pr11_real_ctor(Sample*,SampleContext*) asm("__real__ZN6SampleC2EP13SampleContext");
extern void pr11_wrap_ctor(Sample*,SampleContext*) asm("__wrap__ZN6SampleC2EP13SampleContext");
void pr11_wrap_ctor(Sample* s,SampleContext* c){pr11_real_ctor(s,c);pr11::sample=s;}
}
extern "C" void __real_DrawLine(b3Pos,b3Pos,Vec4);
extern "C" void __wrap_DrawLine(b3Pos a,b3Pos b,Vec4 c){pr11::draw("line",a,b,c);__real_DrawLine(a,b,c);}
extern "C" void __real_DrawPoint(b3Pos,float,Vec4);
extern "C" void __wrap_DrawPoint(b3Pos a,float s,Vec4 c){pr11::draw("point",a,a,c);__real_DrawPoint(a,s,c);}
extern "C" void __real_DrawSolidSphere(b3WorldTransform,b3Sphere,Vec4);
extern "C" void __wrap_DrawSolidSphere(b3WorldTransform t,b3Sphere s,Vec4 c){auto p=b3TransformWorldPoint(t,s.center);pr11::draw("sphere",p,p,c);__real_DrawSolidSphere(t,s,c);}
extern "C" b3BodyCastResult __real_b3Body_CastRay(b3BodyId,b3Pos,b3Vec3,b3QueryFilter,float,b3WorldTransform);
extern "C" b3BodyCastResult __wrap_b3Body_CastRay(b3BodyId id,b3Pos p,b3Vec3 t,b3QueryFilter f,float m,b3WorldTransform x){auto r=__real_b3Body_CastRay(id,p,t,f,m,x);pr11::query("body-ray",r.hit,r.point,r.normal,r.fraction);return r;}
extern "C" b3BodyCastResult __real_b3Body_CastShape(b3BodyId,b3Pos,const b3ShapeProxy*,b3Vec3,b3QueryFilter,float,bool,b3WorldTransform);
extern "C" b3BodyCastResult __wrap_b3Body_CastShape(b3BodyId id,b3Pos p,const b3ShapeProxy* proxy,b3Vec3 t,b3QueryFilter f,float m,bool e,b3WorldTransform x){auto r=__real_b3Body_CastShape(id,p,proxy,t,f,m,e,x);pr11::query("body-shape",r.hit,r.point,r.normal,r.fraction);return r;}
#define WORLD_CAST(NAME,KIND,PARAM,ARG) extern "C" b3TreeStats __real_##NAME(b3WorldId,b3Pos,PARAM b3Vec3,b3QueryFilter,b3CastResultFcn*,void*);extern "C" b3TreeStats __wrap_##NAME(b3WorldId id,b3Pos p,PARAM b3Vec3 t,b3QueryFilter f,b3CastResultFcn* cb,void* x){pr11::Callback c{cb,x,KIND};return __real_##NAME(id,p,ARG t,f,pr11::callback,&c);}
WORLD_CAST(b3World_CastRay,"world-ray",,)
// Shape cast has an extra proxy argument; keep the public ABI explicit.
extern "C" b3TreeStats __real_b3World_CastShape(b3WorldId,b3Pos,const b3ShapeProxy*,b3Vec3,b3QueryFilter,b3CastResultFcn*,void*);
extern "C" b3TreeStats __wrap_b3World_CastShape(b3WorldId id,b3Pos p,const b3ShapeProxy* proxy,b3Vec3 t,b3QueryFilter f,b3CastResultFcn* cb,void* x){pr11::Callback c{cb,x,"world-shape"};return __real_b3World_CastShape(id,p,proxy,t,f,pr11::callback,&c);}
extern "C" void __real_RenderFrame(const sg_swapchain*,const FrameInput*);
extern "C" void __wrap_RenderFrame(const sg_swapchain* s,const FrameInput* f){__real_RenderFrame(s,f);pr11::flush(f);}
