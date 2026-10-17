// Read-only boundary around the actual renderer ClearSelection operation.
#include "gfx/debug_adapter.h"
#include "both_view.h"
#include <cstdio>
#include <cstdlib>
extern "C" BothPointerState both_pointer_state(int) __attribute__((weak));
extern "C" void __real_ClearSelection(void);
static void state(FILE* f){const b3BodyId id=GetSelectedBody();std::fprintf(f,"{\"rendered\":[%u,%u,%u],\"owner_available\":%s,\"owners\":[",id.index1,id.generation,id.world0,both_pointer_state?"true":"false");for(int i=0;i<2;++i){const BothPointerState s=both_pointer_state?both_pointer_state(i):BothPointerState{};if(i)std::fputc(',',f);std::fprintf(f,"{\"selected\":[%u,%u,%u],\"mouse\":[%u,%u,%u],\"joint\":[%u,%u,%u]}",s.selected.index1,s.selected.generation,s.selected.world0,s.mouse.index1,s.mouse.generation,s.mouse.world0,s.joint.index1,s.joint.generation,s.joint.world0);}std::fputs("]}",f);}
extern "C" void __wrap_ClearSelection(void){const char* p=std::getenv("GPU_PR10_API_TRACE");FILE* f=std::getenv("GPU_PR10_SELECTION_TRACE")&&p?std::fopen(p,"a"):nullptr;if(f){std::fputs("{\"api\":\"ui11_clear_selection\",\"before\":",f);state(f);}__real_ClearSelection();if(f){std::fputs(",\"after\":",f);state(f);std::fputs("}\n",f);std::fclose(f);}}
