// Read-only diagnostic for actual native adapter metadata and resolved materials.
#include "box3d/box3d.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <set>
extern "C" bool cpu_b3Shape_IsValid(b3ShapeId) __attribute__((weak));
extern "C" b3BodyId cpu_b3Shape_GetBody(b3ShapeId) __attribute__((weak));
extern "C" bool cpu_b3Body_IsValid(b3BodyId) __attribute__((weak));
extern "C" b3BodyType cpu_b3Body_GetType(b3BodyId) __attribute__((weak));
extern "C" void __real_cpu_b3World_Draw(b3WorldId,b3DebugDraw*,uint64_t) __attribute__((weak));
static thread_local bool adapter_cpu_draw=false;
extern "C" void __wrap_cpu_b3World_Draw(b3WorldId w,b3DebugDraw* d,uint64_t mask){const bool previous=adapter_cpu_draw;adapter_cpu_draw=true;__real_cpu_b3World_Draw(w,d,mask);adapter_cpu_draw=previous;}
static unsigned adapter_request(){const char* ui=std::getenv("GPU_PR10_UI_TRACE");if(!ui)return 0;unsigned r=0;if(FILE* f=std::fopen((std::string(ui)+".request").c_str(),"r")){std::fscanf(f,"%u",&r);std::fclose(f);}return r;}
extern "C" void ui06_adapter_trace(const char* phase,const void* user,b3ShapeId shape,b3BodyId body,int type,b3HexColor color,float metallic,float roughness,float alpha,bool dynamic,bool kinematic,bool material){
 if(!std::getenv("GPU_PR10_ADAPTER_TRACE"))return;const char* path=std::getenv("GPU_PR10_API_TRACE");if(!path)return;
 const unsigned request=adapter_request();static unsigned last=~0u;static std::set<std::string> seen;if(request!=last){seen.clear();last=request;}char key[160];std::snprintf(key,sizeof(key),"%s:%p:%d",phase,user,adapter_cpu_draw);if(!seen.insert(key).second)return;if(seen.size()>128)return;
 const bool valid=b3Body_IsValid(body);const int live=valid?(int)b3Body_GetType(body):-1;const b3BodyId publicBody=b3Shape_GetBody(shape);b3BodyId cpuBody={};int cpuType=-1;bool cpuValid=false;if(adapter_cpu_draw&&cpu_b3Shape_IsValid&&cpu_b3Shape_IsValid(shape)){cpuBody=cpu_b3Shape_GetBody(shape);cpuValid=cpu_b3Body_IsValid(cpuBody);if(cpuValid)cpuType=(int)cpu_b3Body_GetType(cpuBody);}
 if(FILE* f=std::fopen(path,"a")){std::fprintf(f,"{\"api\":\"ui06_adapter\",\"request\":%u,\"phase\":\"%s\",\"cpu_callback\":%s,\"user_pointer\":\"%p\",\"shape\":[%u,%u,%u],\"cached_body\":[%u,%u,%u],\"cached_type\":%d,\"public_body_valid\":%s,\"live_public_type\":%d,\"public_shape_body\":[%u,%u,%u],\"expected_cpu_body\":[%u,%u,%u],\"expected_cpu_type\":%d,\"expected_cpu_valid\":%s,\"resolved\":{\"packed_color\":%u,\"preset\":%u,\"metallic\":%.9g,\"roughness\":%.9g,\"alpha\":%.9g},\"transparency_inputs\":[%s,%s],\"material_override\":%s}\n",request,phase,adapter_cpu_draw?"true":"false",user,shape.index1,shape.generation,shape.world0,body.index1,body.generation,body.world0,type,valid?"true":"false",live,publicBody.index1,publicBody.generation,publicBody.world0,cpuBody.index1,cpuBody.generation,cpuBody.world0,cpuType,cpuValid?"true":"false",(unsigned)color,((unsigned)color>>24)&255,metallic,roughness,alpha,dynamic?"true":"false",kinematic?"true":"false",material?"true":"false");std::fclose(f);}
}
