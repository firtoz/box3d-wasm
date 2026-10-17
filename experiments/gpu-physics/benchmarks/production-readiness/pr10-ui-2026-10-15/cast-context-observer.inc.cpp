#include <dlfcn.h>
#include "cast-context-layout.inc.cpp"
namespace {
uintptr_t cast_callback_offset(b3CastResultFcn* fn){uintptr_t n=(uintptr_t)fn;if(std::getenv("GPU_PR10_CALLBACK_PIE")){Dl_info info{};if(!dladdr((void*)fn,&info))return 0;n-=(uintptr_t)info.dli_fbase;}return n;}
bool trusted_cast_context(b3CastResultFcn* fn,void* user){if(!user||!std::getenv("GPU_PR10_CAST_CONTEXT"))return false;const char* list=std::getenv("GPU_PR10_CALLBACK_OFFSETS");if(!list)return false;uintptr_t value=cast_callback_offset(fn);while(*list){char* end=nullptr;auto n=std::strtoull(list,&end,16);if(end==list)return false;if(n==value)return true;list=end;if(*list==',')++list;else if(*list)return false;}return false;}
std::string observed_cast_context(b3CastResultFcn* fn,void* user){if(!trusted_cast_context(fn,user))return "null";ObservedCastContext c{};std::memcpy(&c,user,sizeof(c));std::string out="{\"count\":"+std::to_string(c.count)+",\"initialOverlap\":"+(c.initialOverlap?"true":"false")+",\"fractions\":[";for(int i=0;i<c.count&&i<3;++i){if(i)out+=",";out+=number_json(c.fractions[i]);}return out+"]}";}
}
static void cast_camera_targets(FILE*);
