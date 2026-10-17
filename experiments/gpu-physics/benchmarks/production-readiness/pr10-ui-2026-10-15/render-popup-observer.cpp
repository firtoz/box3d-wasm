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
void record(const char* kind,const char* label,const std::string& value,bool changed=false,ImGuiWindow* parent=nullptr) {
    if(!trace_path()||!ImGui::GetCurrentContext())return;
    ImGuiContext& g=*ImGui::GetCurrentContext(); ImGuiWindow* w=parent?parent:g.CurrentWindow;
    if(!w)return;
    if(frame!=g.FrameCount) { items.clear();frame=g.FrameCount; }
    Item i{kind,label?label:"",value,w->Name,g.LastItemData.Rect,ImRect(w->DrawList->_ClipRectStack.back()),(parent?g.LastItemData.Rect.Overlaps(ImRect(w->DrawList->_ClipRectStack.back())):ImGui::IsItemVisible()),(g.CurrentItemFlags&ImGuiItemFlags_Disabled)!=0,changed,g.LastItemData.ID};
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
    std::fputs("],\"windows\":[",f);bool first=true;
    for (ImGuiWindow* w: ImGui::GetCurrentContext()->Windows) {
        if(!w->Active||w->Hidden)continue;
        if(!first)std::fputc(',',f);first=false;
        std::fputs("{\"name\":",f);quote(f,w->Name);
        std::fprintf(f,",\"rect\":[%.6f,%.6f,%.6f,%.6f],\"clip\":[%.6f,%.6f,%.6f,%.6f],\"cursor_start\":[%.6f,%.6f],\"row_height\":%.6f,\"font_height\":%.6f,\"scroll\":[%.6f,%.6f]}",w->Pos.x*sx,w->Pos.y*sy,(w->Pos.x+w->Size.x)*sx,(w->Pos.y+w->Size.y)*sy,w->InnerClipRect.Min.x*sx,w->InnerClipRect.Min.y*sy,w->InnerClipRect.Max.x*sx,w->InnerClipRect.Max.y*sy,w->DC.CursorStartPos.x*sx,w->DC.CursorStartPos.y*sy,ImGui::GetTextLineHeightWithSpacing()*sy,ImGui::GetFontSize()*sy,w->Scroll.x,w->Scroll.y);
    }
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

#include "solver-controls-observer.cpp"
DECL(menu,"_ZN5ImGui9BeginMenuEPKcb",bool,(const char*,bool));
bool wrap_menu(const char* l,bool enabled) { auto* parent=ImGui::GetCurrentWindow();bool r=real_menu(l,enabled);record("Menu",l,r?"true":"false",false,parent);items.back().disabled|=!enabled;return r; }
DECL(menu_item,"_ZN5ImGui8MenuItemEPKcS1_bb",bool,(const char*,const char*,bool,bool));
bool wrap_menu_item(const char* l,const char* shortcut,bool selected,bool enabled) { bool r=real_menu_item(l,shortcut,selected,enabled);record("MenuItem",l,selected?"true":"false",r);items.back().disabled|=!enabled;return r; }
DECL(menu_ptr,"_ZN5ImGui8MenuItemEPKcS1_Pbb",bool,(const char*,const char*,bool*,bool));
bool wrap_menu_ptr(const char* l,const char* shortcut,bool* selected,bool enabled) { bool r=real_menu_ptr(l,shortcut,selected,enabled);record("MenuItem",l,selected&&*selected?"true":"false",r);items.back().disabled|=!enabled;return r; }
DECL(small,"_ZN5ImGui11SmallButtonEPKc",bool,(const char*));
bool wrap_small(const char* l) { bool r=real_small(l);record("SmallButton",l,"",r);return r; }
DECL(slider3,"_ZN5ImGui12SliderFloat3EPKcPfffS1_i",bool,(const char*,float*,float,float,const char*,int));
bool wrap_slider3(const char* l,float* v,float lo,float hi,const char* fmt,int flags) { bool r=real_slider3(l,v,lo,hi,fmt,flags);record("SliderFloat3",l,std::to_string(v[0])+","+std::to_string(v[1])+","+std::to_string(v[2]),r);return r; }
DECL(input_int,"_ZN5ImGui8InputIntEPKcPiiii",bool,(const char*,int*,int,int,int));
bool wrap_input_int(const char* l,int* v,int step,int fast,int flags) { bool r=real_input_int(l,v,step,fast,flags);record("InputInt",l,std::to_string(*v),r);return r; }
DECL(input_float,"_ZN5ImGui10InputFloatEPKcPfffS1_i",bool,(const char*,float*,float,float,const char*,int));
bool wrap_input_float(const char* l,float* v,float step,float fast,const char* fmt,int flags) { bool r=real_input_float(l,v,step,fast,fmt,flags);record("InputFloat",l,std::to_string(*v),r);return r; }
DECL(combo_arr,"_ZN5ImGui5ComboEPKcPiPKS1_ii",bool,(const char*,int*,const char* const*,int,int));
bool wrap_combo_arr(const char* l,int* v,const char*const* opts,int count,int height) { bool r=real_combo_arr(l,v,opts,count,height);record("Combo",l,std::to_string(*v),r);return r; }
DECL(combo_str,"_ZN5ImGui5ComboEPKcPiS1_i",bool,(const char*,int*,const char*,int));
bool wrap_combo_str(const char* l,int* v,const char* opts,int height) { bool r=real_combo_str(l,v,opts,height);record("Combo",l,std::to_string(*v),r);return r; }
DECL(combo_get,"_ZN5ImGui5ComboEPKcPiPFS1_PviES3_ii",bool,(const char*,int*,const char*(*)(void*,int),void*,int,int));
bool wrap_combo_get(const char* l,int* v,const char*(*get)(void*,int),void* data,int count,int height) { bool r=real_combo_get(l,v,get,data,count,height);record("Combo",l,std::to_string(*v),r);return r; }
DECL(input_text,"_ZN5ImGui9InputTextEPKcPcmiPFiP26ImGuiInputTextCallbackDataEPv",bool,(const char*,char*,size_t,int,ImGuiInputTextCallback,void*));
bool wrap_input_text(const char* l,char* buf,size_t size,int flags,ImGuiInputTextCallback cb,void* data) { bool r=real_input_text(l,buf,size,flags,cb,data);record("InputText",l,buf,r);return r; }
DECL(input_hint,"_ZN5ImGui17InputTextWithHintEPKcS1_PcmiPFiP26ImGuiInputTextCallbackDataEPv",bool,(const char*,const char*,char*,size_t,int,ImGuiInputTextCallback,void*));
bool wrap_input_hint(const char* l,const char* hint,char* buf,size_t size,int flags,ImGuiInputTextCallback cb,void* data) { bool r=real_input_hint(l,hint,buf,size,flags,cb,data);record("InputTextWithHint",l,buf,r);return r; }
DECL(select,"_ZN5ImGui10SelectableEPKcbiRK6ImVec2",bool,(const char*,bool,int,const ImVec2&));
bool wrap_select(const char* l,bool selected,int flags,const ImVec2& size) { bool r=real_select(l,selected,flags,size);record("Selectable",l,selected?"true":"false",r);return r; }
DECL(select_ptr,"_ZN5ImGui10SelectableEPKcPbiRK6ImVec2",bool,(const char*,bool*,int,const ImVec2&));
bool wrap_select_ptr(const char* l,bool* selected,int flags,const ImVec2& size) { bool r=real_select_ptr(l,selected,flags,size);record("Selectable",l,selected&&*selected?"true":"false",r);return r; }
DECL(tree,"_ZN5ImGui10TreeNodeExEPKci",bool,(const char*,int));
bool wrap_tree(const char* l,int flags) { bool r=real_tree(l,flags);record("Tree",l,r?"true":"false",false);return r; }
#include "box3d/types.h"
extern "C" void __real_b3World_Draw(b3WorldId,b3DebugDraw*,uint64_t);
static void trace_draw_impl(b3WorldId id,b3DebugDraw* d,uint64_t mask) {
 if(const char* path=std::getenv("GPU_PR10_API_TRACE"))if(FILE* f=std::fopen(path,"a")) {
  std::fprintf(f,"{\"api\":\"b3World_Draw\",\"world\":[%u,%u],\"shapes\":%d,\"joints\":%d,\"extras\":%d,\"bounds\":%d,\"mass\":%d,\"sleep\":%d,\"names\":%d,\"contacts\":%d,\"anchorA\":%d,\"graph\":%d,\"features\":%d,\"normals\":%d,\"forces\":%d,\"islands\":%d,\"jointScale\":%.9g,\"forceScale\":%.9g}\n",(unsigned)id.index1,(unsigned)id.generation,d->drawShapes,d->drawJoints,d->drawJointExtras,d->drawBounds,d->drawMass,d->drawSleep,d->drawBodyNames,d->drawContacts,d->drawAnchorA,d->drawGraphColors,d->drawContactFeatures,d->drawContactNormals,d->drawContactForces,d->drawIslands,d->jointScale,d->forceScale);std::fclose(f);
 }
 __real_b3World_Draw(id,d,mask);
}
namespace {
struct DrawOutputs { unsigned bounds=0,transforms=0; void(*bounds_fn)(b3AABB,b3HexColor,void*)=nullptr;void(*transform_fn)(b3WorldTransform,void*)=nullptr; };
thread_local DrawOutputs* active_output=nullptr;
void observed_bounds(b3AABB b,b3HexColor color,void* context) { ++active_output->bounds;active_output->bounds_fn(b,color,context); }
void observed_transform(b3WorldTransform t,void* context) { ++active_output->transforms;active_output->transform_fn(t,context); }
void write_output(const char* api,b3WorldId id,unsigned bounds,unsigned transforms) {
 if(const char* path=std::getenv("GPU_PR10_API_TRACE"))if(FILE* f=std::fopen(path,"a")) {std::fprintf(f,"{\"api\":\"%s\",\"world\":[%u,%u],\"bounds_callbacks\":%u,\"transform_callbacks\":%u}\n",api,(unsigned)id.index1,(unsigned)id.generation,bounds,transforms);std::fclose(f);}
}
}
extern "C" void __wrap_b3World_Draw(b3WorldId id,b3DebugDraw* d,uint64_t mask) {
 if(!d) { __real_b3World_Draw(id,d,mask);return; }
 b3DebugDraw local=*d;DrawOutputs outputs;outputs.bounds_fn=d->DrawBoundsFcn;outputs.transform_fn=d->DrawTransformFcn;
 if(local.DrawBoundsFcn)local.DrawBoundsFcn=observed_bounds;
 if(local.DrawTransformFcn)local.DrawTransformFcn=observed_transform;
 DrawOutputs* previous=active_output;active_output=&outputs;trace_draw_impl(id,&local,mask);active_output=previous;
 write_output("actual_draw_outputs",id,outputs.bounds,outputs.transforms);
}
extern "C" void __real_gpu_samples_world_draw(b3WorldId,b3DebugDraw*,uint64_t) __attribute__((weak));
extern "C" void __wrap_gpu_samples_world_draw(b3WorldId id,b3DebugDraw* d,uint64_t mask) {
 assert(__real_gpu_samples_world_draw);unsigned b=active_output?active_output->bounds:0,t=active_output?active_output->transforms:0;
 __real_gpu_samples_world_draw(id,d,mask);
 write_output("actual_gpu_draw_outputs",id,active_output?active_output->bounds-b:0,active_output?active_output->transforms-t:0);
}
#include "gfx/renderer.h"
#include "gfx/shadow.h"
#include "gfx/gtao.h"
extern "C" void __real_RenderFrame(const sg_swapchain*,const FrameInput*);
extern "C" void __wrap_RenderFrame(const sg_swapchain* sc,const FrameInput* frame) {
 if(const char* path=std::getenv("GPU_PR10_API_TRACE"))if(FILE* f=std::fopen(path,"a")) {
  Sun sun=GetSun();EdgeOverlayParams edges=GetEdgeOverlayParams();GtaoTraceParams gtao=GetGtaoTraceParams();
  std::fprintf(f,"{\"api\":\"actual_RenderFrame\",\"debugMode\":%d,\"disableShadows\":%d,\"disableAmbientOcclusion\":%d,\"exposure\":%.9g,\"sun\":%.9g,\"ibl\":%d,\"hullEdges\":%d,\"edgeConvexity\":%d,\"shadowStart\":%.9g,\"shadowRange\":%.9g,\"shadowDistribution\":%.9g,\"gtaoQuality\":%d}\n",frame->debugMode,frame->disableShadows,frame->disableAmbientOcclusion,GetExposure(),sun.strength,GetIblEnabled(),edges.showHulls,edges.showEdgeConvexity,GetShadowSplitNear(),GetShadowSplitFar(),GetShadowSplitLambda(),(int)gtao.quality);std::fclose(f);
 }
 __real_RenderFrame(sc,frame);
}
