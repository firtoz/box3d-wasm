#include "box3d/id.h"
#include "sokol_app.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
extern bool gpu_loading_active();
extern "C" bool ui25_previous_handle(const sapp_event*);
extern "C" bool ui25_previous_poll(b3WorldId);
extern "C" bool ui25_previous_slider(const char*,float*,float,float,const char*,int);
extern "C" bool ui25_previous_button(const char*,const ImVec2&);
static FILE* trace(const char* stage){if(!std::getenv("GPU_PR10_INPUT_LOADING_TRACE")||!ImGui::GetCurrentContext())return nullptr;const char* path=std::getenv("GPU_PR10_API_TRACE");FILE* f=path?std::fopen(path,"a"):nullptr;if(f){auto& g=*ImGui::GetCurrentContext();std::fprintf(f,"{\"api\":\"ui25_input_loading\",\"stage\":\"%s\",\"frame\":%d,\"active\":%u,\"alive\":%u,\"temp_input\":%u,\"loading\":%s,\"want_text\":%s,\"queued_events\":%d,\"queued_chars\":%d",stage,g.FrameCount,g.ActiveId,g.ActiveIdIsAlive,g.TempInputId,gpu_loading_active()?"true":"false",g.IO.WantTextInput?"true":"false",g.InputEventsQueue.Size,g.IO.InputQueueCharacters.Size);}return f;}
extern bool wrap_handle(const sapp_event*) asm("__wrap__Z11HandleEventPK10sapp_event");
bool wrap_handle(const sapp_event* e){if(e->type==SAPP_EVENTTYPE_CHAR||e->type==SAPP_EVENTTYPE_MOUSE_DOWN||e->type==SAPP_EVENTTYPE_MOUSE_UP)if(FILE* f=trace("event_before")){std::fprintf(f,",\"event_type\":%d,\"char\":%u,\"mouse\":[%.9g,%.9g]}\n",e->type,e->char_code,e->mouse_x,e->mouse_y);std::fclose(f);}const bool result=ui25_previous_handle(e);if(e->type==SAPP_EVENTTYPE_CHAR||e->type==SAPP_EVENTTYPE_MOUSE_DOWN||e->type==SAPP_EVENTTYPE_MOUSE_UP)if(FILE* f=trace("event_after")){std::fprintf(f,",\"event_type\":%d,\"char\":%u}\n",e->type,e->char_code);std::fclose(f);}return result;}
extern bool wrap_poll(b3WorldId) asm("__wrap__Z16gpu_loading_poll9b3WorldId");
bool wrap_poll(b3WorldId id){if(FILE* f=trace("poll_before")){std::fputs("}\n",f);std::fclose(f);}const bool result=ui25_previous_poll(id);if(FILE* f=trace("poll_after")){std::fprintf(f,",\"returned_loading\":%s}\n",result?"true":"false");std::fclose(f);}return result;}
extern bool wrap_slider(const char*,float*,float,float,const char*,int) asm("__wrap__ZN5ImGui11SliderFloatEPKcPfffS1_i");
bool wrap_slider(const char* label,float* value,float lo,float hi,const char* fmt,int flags){const float before=*value;const bool changed=ui25_previous_slider(label,value,lo,hi,fmt,flags);if((!std::strcmp(label,"amplitude")||!std::strcmp(label,"Extent"))&&(changed||ImGui::GetActiveID()==ImGui::GetItemID()))if(FILE* f=trace("slider_after")){std::fprintf(f,",\"label\":\"%s\",\"before\":%.9g,\"value\":%.9g,\"changed\":%s,\"item\":%u}\n",label,before,*value,changed?"true":"false",ImGui::GetItemID());std::fclose(f);}return changed;}
extern bool wrap_button(const char*,const ImVec2&) asm("__wrap__ZN5ImGui6ButtonEPKcRK6ImVec2");
bool wrap_button(const char* label,const ImVec2& size){const bool changed=ui25_previous_button(label,size);if(!std::strcmp(label,"Auto Generate"))if(FILE* f=trace("button_after")){std::fprintf(f,",\"changed\":%s,\"item\":%u}\n",changed?"true":"false",ImGui::GetItemID());std::fclose(f);}return changed;}
