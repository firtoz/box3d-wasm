// Read-only native event, held-key and actual Update boundary.
#include "host/camera.h"
#include "sample.h"
#include "sokol_app.h"
#include "imgui.h"
#include <cstdio>
#include <cstdlib>
#include <string>
static Camera* camera;
static FILE* trace(){if(!std::getenv("GPU_PR10_KEY_TRACE"))return nullptr;const char* p=std::getenv("GPU_PR10_API_TRACE");return p?std::fopen(p,"a"):nullptr;}
static void fields(FILE* f,Camera* c){std::fprintf(f,"\"keys\":[%d,%d,%d,%d,%d,%d,%d],\"right_down\":%d,\"host_w\":%d,\"pivot\":[%.17g,%.17g,%.17g]",c->m_wDown,c->m_aDown,c->m_sDown,c->m_dDown,c->m_qDown,c->m_eDown,c->m_altDown,c->m_rightDown,IsKeyDown(SAPP_KEYCODE_W),c->m_pivot.x,c->m_pivot.y,c->m_pivot.z);}
extern bool real_handle(const sapp_event*) asm("__real__Z11HandleEventPK10sapp_event");extern bool wrap_handle(const sapp_event*) asm("__wrap__Z11HandleEventPK10sapp_event");
bool wrap_handle(const sapp_event* e){const bool capture=real_handle(e);if(camera&&(e->type==SAPP_EVENTTYPE_KEY_DOWN||e->type==SAPP_EVENTTYPE_KEY_UP))if(FILE* f=trace()){std::fprintf(f,"{\"api\":\"ui10_event_capture\",\"type\":%d,\"key\":%d,\"captured\":%s,\"want_keyboard\":%s,\"want_text\":%s,",e->type,e->key_code,capture?"true":"false",ImGui::GetIO().WantCaptureKeyboard?"true":"false",ImGui::GetIO().WantTextInput?"true":"false");fields(f,camera);std::fputs("}\n",f);std::fclose(f);}return capture;}
extern void real_event(Camera*,const sapp_event*) asm("__real__ZN6Camera7OnEventEPK10sapp_event");extern void wrap_event(Camera*,const sapp_event*) asm("__wrap__ZN6Camera7OnEventEPK10sapp_event");
void wrap_event(Camera* c,const sapp_event* e){real_event(c,e);if(e->type==SAPP_EVENTTYPE_KEY_DOWN||e->type==SAPP_EVENTTYPE_KEY_UP||e->type==SAPP_EVENTTYPE_MOUSE_UP||e->type==SAPP_EVENTTYPE_UNFOCUSED)if(FILE* f=trace()){std::fprintf(f,"{\"api\":\"ui10_camera_event\",\"type\":%d,\"key\":%d,",e->type,e->key_code);fields(f,c);std::fputs("}\n",f);std::fclose(f);}}
extern void real_update(Camera*,float,int,int) asm("__real__ZN6Camera6UpdateEfii");extern void wrap_update(Camera*,float,int,int) asm("__wrap__ZN6Camera6UpdateEfii");
void wrap_update(Camera* c,float dt,int w,int h){camera=c;real_update(c,dt,w,h);const char* p=std::getenv("GPU_PR10_UI_TRACE");if(!p)return;unsigned request=0;static unsigned last=0;FILE* rf=std::fopen((std::string(p)+".request").c_str(),"r");if(!rf)return;const int n=std::fscanf(rf,"%u",&request);std::fclose(rf);if(n!=1||request==last)return;last=request;if(FILE* f=trace()){std::fprintf(f,"{\"api\":\"ui10_camera_update\",\"request\":%u,\"dt\":%.9g,\"width\":%d,\"height\":%d,",request,dt,w,h);fields(f,c);std::fputs("}\n",f);std::fclose(f);}}
