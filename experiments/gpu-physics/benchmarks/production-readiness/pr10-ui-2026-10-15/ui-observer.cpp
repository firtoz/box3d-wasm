// Diagnostic-only GNU link wrappers. Call the actual ImGui implementation once;
// record its item rectangle/value and never inject input or access physics state.
#include "box3d/id.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <cstdio>
#include <cstdint>
#include <cstdarg>
#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>
extern "C" const void* sapp_x11_get_window(void);
extern "C" int sapp_width(void);
extern "C" int sapp_height(void);
namespace {
struct Item { std::string kind,label,value,window; ImRect rect,clip; bool visible,disabled,changed; unsigned id; };
std::vector<Item> items, changes;
int frame=-1;
const char* trace_path() { return std::getenv("GPU_PR10_UI_TRACE"); }
void quote(FILE* f,const std::string& s) {
    std::fputc('"',f);
    for (unsigned char c:s) { if(c=='"'||c=='\\')std::fprintf(f,"\\%c",c);else if(c<32)std::fprintf(f,"\\u%04x",c);else std::fputc(c,f); }
    std::fputc('"',f);
}
void record(const char* kind,const char* label,const std::string& value,bool changed=false) {
    if(!trace_path()||!ImGui::GetCurrentContext())return;
    ImGuiContext& g=*ImGui::GetCurrentContext(); ImGuiWindow* w=g.CurrentWindow;
    if(!w)return;
    if(frame!=g.FrameCount) { items.clear();frame=g.FrameCount; }
    Item i{kind,label?label:"",value,w->Name,g.LastItemData.Rect,w->InnerClipRect,ImGui::IsItemVisible(),(g.CurrentItemFlags&ImGuiItemFlags_Disabled)!=0,changed,g.LastItemData.ID};
    items.push_back(i);
    if(changed) { if(changes.size()>=64)changes.erase(changes.begin());changes.push_back(i); }
}
void write_item(FILE* f,const Item& i,float sx,float sy) {
    std::fputs("{\"kind\":",f);quote(f,i.kind);std::fputs(",\"label\":",f);quote(f,i.label);std::fputs(",\"value\":",f);quote(f,i.value);std::fputs(",\"window\":",f);quote(f,i.window);
    std::fprintf(f,",\"id\":%u,\"visible\":%s,\"disabled\":%s,\"changed\":%s,\"rect\":[%.6f,%.6f,%.6f,%.6f],\"clip\":[%.6f,%.6f,%.6f,%.6f]}",i.id,i.visible?"true":"false",i.disabled?"true":"false",i.changed?"true":"false",i.rect.Min.x*sx,i.rect.Min.y*sy,i.rect.Max.x*sx,i.rect.Max.y*sy,i.clip.Min.x*sx,i.clip.Min.y*sy,i.clip.Max.x*sx,i.clip.Max.y*sy);
}
void flush() {
    const char* path=trace_path(); if(!path||items.empty()||!ImGui::GetCurrentContext())return;
    // Publish only when the test asks for a fresh frame, avoiding a stream of
    // per-frame files. The request is a small monotonically increasing token.
    static unsigned last_request=0;unsigned request=0;
    std::string request_path=std::string(path)+".request";
    FILE* rf=std::fopen(request_path.c_str(),"r");if(!rf)return;
    const int read=std::fscanf(rf,"%u",&request);std::fclose(rf);
    if(read!=1||request==last_request)return;
    const ImGuiIO& io=ImGui::GetIO(); if(io.DisplaySize.x<=0||io.DisplaySize.y<=0)return;
    const float sx=float(sapp_width())/io.DisplaySize.x,sy=float(sapp_height())/io.DisplaySize.y;
    std::string tmp=std::string(path)+".tmp";FILE* f=std::fopen(tmp.c_str(),"w");if(!f)return;
    std::fprintf(f,"{\"request\":%u,\"frame\":%d,\"width\":%d,\"height\":%d,\"want_text_input\":%s,\"want_capture_mouse\":%s,\"items\":[",request,frame,sapp_width(),sapp_height(),io.WantTextInput?"true":"false",io.WantCaptureMouse?"true":"false");
    for(size_t n=0;n<items.size();++n) { if(n)std::fputc(',',f);write_item(f,items[n],sx,sy); }
    std::fputs("],\"changes\":[",f);for(size_t n=0;n<changes.size();++n) {if(n)std::fputc(',',f);write_item(f,changes[n],sx,sy);}
    std::fputs("]}\n",f);std::fclose(f);if(std::rename(tmp.c_str(),path)==0)last_request=request;
}
std::string format(const char* fmt,va_list ap) { char text[2048];std::vsnprintf(text,sizeof(text),fmt,ap);return text; }
}
extern bool real_poll(b3WorldId) asm("__real__Z16gpu_loading_poll9b3WorldId");
extern bool wrap_poll(b3WorldId) asm("__wrap__Z16gpu_loading_poll9b3WorldId");
bool wrap_poll(b3WorldId world) {
    flush();static bool seen=false;auto window=sapp_x11_get_window();
    if(!seen&&window&&sapp_width()>0&&sapp_height()>0) { seen=true;std::fprintf(stderr,"PR10WINDOW,pid=%ld,id=%llu,width=%d,height=%d\n",(long)getpid(),(unsigned long long)(uintptr_t)window,sapp_width(),sapp_height());std::fflush(stderr); }
    return real_poll(world);
}
#define DECL(NAME,SYMBOL,RET,ARGS) extern RET real_##NAME ARGS asm("__real_" SYMBOL); extern RET wrap_##NAME ARGS asm("__wrap_" SYMBOL)
DECL(check,"_ZN5ImGui8CheckboxEPKcPb",bool,(const char*,bool*));
bool wrap_check(const char* l,bool* v) { bool r=real_check(l,v);record("Checkbox",l,*v?"true":"false",r);return r; }
DECL(button,"_ZN5ImGui6ButtonEPKcRK6ImVec2",bool,(const char*,const ImVec2&));
bool wrap_button(const char* l,const ImVec2& size) { bool r=real_button(l,size);record("Button",l,"",r);return r; }
DECL(radio,"_ZN5ImGui11RadioButtonEPKcb",bool,(const char*,bool));
bool wrap_radio(const char* l,bool active) { bool r=real_radio(l,active);record("RadioButton",l,active?"true":"false",r);return r; }
DECL(radio_int,"_ZN5ImGui11RadioButtonEPKcPii",bool,(const char*,int*,int));
bool wrap_radio_int(const char* l,int* v,int button) { bool r=real_radio_int(l,v,button);record("RadioButton",l,*v==button?"true":"false",r);return r; }
DECL(header,"_ZN5ImGui16CollapsingHeaderEPKci",bool,(const char*,int));
bool wrap_header(const char* l,int flags) { bool r=real_header(l,flags);record("Header",l,r?"true":"false",ImGui::IsItemClicked());return r; }
DECL(tab,"_ZN5ImGui12BeginTabItemEPKcPbi",bool,(const char*,bool*,int));
bool wrap_tab(const char* l,bool* open,int flags) { bool r=real_tab(l,open,flags);record("Tab",l,r?"true":"false",ImGui::IsItemClicked());return r; }
DECL(slider_int,"_ZN5ImGui9SliderIntEPKcPiiiS1_i",bool,(const char*,int*,int,int,const char*,int));
bool wrap_slider_int(const char* l,int* v,int lo,int hi,const char* fmt,int flags) { bool r=real_slider_int(l,v,lo,hi,fmt,flags);record("SliderInt",l,std::to_string(*v),r);return r; }
DECL(slider_float,"_ZN5ImGui11SliderFloatEPKcPfffS1_i",bool,(const char*,float*,float,float,const char*,int));
bool wrap_slider_float(const char* l,float* v,float lo,float hi,const char* fmt,int flags) { bool r=real_slider_float(l,v,lo,hi,fmt,flags);record("SliderFloat",l,std::to_string(*v),r);return r; }
extern void wrap_text(const char*,...) asm("__wrap__ZN5ImGui4TextEPKcz");
void wrap_text(const char* fmt,...) { va_list ap,copy;va_start(ap,fmt);va_copy(copy,ap);auto text=format(fmt,copy);va_end(copy);ImGui::TextV(fmt,ap);va_end(ap);record("Text",text.c_str(),""); }
extern void wrap_disabled(const char*,...) asm("__wrap__ZN5ImGui12TextDisabledEPKcz");
void wrap_disabled(const char* fmt,...) { va_list ap,copy;va_start(ap,fmt);va_copy(copy,ap);auto text=format(fmt,copy);va_end(copy);ImGui::TextDisabledV(fmt,ap);va_end(ap);record("TextDisabled",text.c_str(),""); }
extern void wrap_colored(const ImVec4&,const char*,...) asm("__wrap__ZN5ImGui11TextColoredERK6ImVec4PKcz");
void wrap_colored(const ImVec4& color,const char* fmt,...) { va_list ap,copy;va_start(ap,fmt);va_copy(copy,ap);auto text=format(fmt,copy);va_end(copy);ImGui::TextColoredV(color,fmt,ap);va_end(ap);record("TextColored",text.c_str(),""); }
