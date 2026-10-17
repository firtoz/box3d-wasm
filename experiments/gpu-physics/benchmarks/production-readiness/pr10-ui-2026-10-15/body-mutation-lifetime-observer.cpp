// Diagnostic-only GNU link wrappers. Call the actual ImGui implementation once;
// record its item rectangle/value and never inject input or access physics state.
#include "box3d/box3d.h"
#include <cstring>
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
bool wrap_slider3(const char* l,float* v,float lo,float hi,const char* fmt,int flags) {
 float width=ImGui::CalcItemWidth(),spacing=ImGui::GetStyle().ItemInnerSpacing.x;bool r=real_slider3(l,v,lo,hi,fmt,flags);
 record("SliderFloat3",l,std::to_string(v[0])+","+std::to_string(v[1])+","+std::to_string(v[2]),r);
 if(trace_path()&&!items.empty()) {Item group=items.back();float previous=0,x=group.rect.Min.x,total=width-2*spacing;
  for(int j=0;j<3;++j){float split=j==2?total:float(int(total*float(j+1)/3));float w=split-previous;if(w<1)w=1;Item child=group;child.kind="SliderFloat3Component";child.label=std::string(l)+":"+std::to_string(j);child.value=std::to_string(v[j]);child.rect.Min.x=x;child.rect.Max.x=x+w;child.visible=group.visible&&child.rect.Overlaps(child.clip);items.push_back(child);x+=w+spacing;previous=split;}
 }return r;
}
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
struct DrawOutputs { b3DebugDraw original{}; unsigned count[9]{};uint64_t digest[9]{}; };
thread_local DrawOutputs* active_output=nullptr;
thread_local b3DebugDraw last_original{};
void hash_bytes(int i,const void* bytes,size_t size) {auto* p=(const unsigned char*)bytes;uint64_t& h=active_output->digest[i];if(!h)h=1469598103934665603ull;for(size_t n=0;n<size;++n)h=(h^p[n])*1099511628211ull;}
template<class T>void hv(int i,const T& v){hash_bytes(i,&v,sizeof(v));}
void hp(int i,b3Pos p){hv(i,p.x);hv(i,p.y);hv(i,p.z);}
void hvec(int i,b3Vec3 p){hv(i,p.x);hv(i,p.y);hv(i,p.z);}
void hxf(int i,b3WorldTransform t){hp(i,t.p);hvec(i,t.q.v);hv(i,t.q.s);}
void observed_shape(void* u,b3WorldTransform t,b3HexColor c,void* ctx){++active_output->count[0];hxf(0,t);hv(0,c);active_output->original.DrawShapeFcn(u,t,c,ctx);}
void observed_segment(b3Pos a,b3Pos b,b3HexColor c,void* ctx){++active_output->count[1];hp(1,a);hp(1,b);hv(1,c);active_output->original.DrawSegmentFcn(a,b,c,ctx);}
void observed_transform(b3WorldTransform t,void* ctx){++active_output->count[2];hxf(2,t);active_output->original.DrawTransformFcn(t,ctx);}
void observed_point(b3Pos p,float size,b3HexColor c,void* ctx){++active_output->count[3];hp(3,p);hv(3,size);hv(3,c);active_output->original.DrawPointFcn(p,size,c,ctx);}
void observed_sphere(b3Pos p,float radius,b3HexColor c,float alpha,void* ctx){++active_output->count[4];hp(4,p);hv(4,radius);hv(4,c);hv(4,alpha);active_output->original.DrawSphereFcn(p,radius,c,alpha,ctx);}
void observed_capsule(b3Pos a,b3Pos b,float radius,b3HexColor c,float alpha,void* ctx){++active_output->count[5];hp(5,a);hp(5,b);hv(5,radius);hv(5,c);hv(5,alpha);active_output->original.DrawCapsuleFcn(a,b,radius,c,alpha,ctx);}
void observed_bounds(b3AABB a,b3HexColor c,void* ctx){++active_output->count[6];hvec(6,a.lowerBound);hvec(6,a.upperBound);hv(6,c);active_output->original.DrawBoundsFcn(a,c,ctx);}
void observed_box(b3Vec3 e,b3WorldTransform t,b3HexColor c,void* ctx){++active_output->count[7];hvec(7,e);hxf(7,t);hv(7,c);active_output->original.DrawBoxFcn(e,t,c,ctx);}
void observed_string(b3Pos p,const char* text,b3HexColor c,void* ctx){++active_output->count[8];hp(8,p);hv(8,c);if(text)hash_bytes(8,text,std::strlen(text));active_output->original.DrawStringFcn(p,text,c,ctx);}
void observe_callbacks(b3DebugDraw& d){
 if(d.DrawShapeFcn)d.DrawShapeFcn=observed_shape;if(d.DrawSegmentFcn)d.DrawSegmentFcn=observed_segment;if(d.DrawTransformFcn)d.DrawTransformFcn=observed_transform;
 if(d.DrawPointFcn)d.DrawPointFcn=observed_point;if(d.DrawSphereFcn)d.DrawSphereFcn=observed_sphere;if(d.DrawCapsuleFcn)d.DrawCapsuleFcn=observed_capsule;
 if(d.DrawBoundsFcn)d.DrawBoundsFcn=observed_bounds;if(d.DrawBoxFcn)d.DrawBoxFcn=observed_box;if(d.DrawStringFcn)d.DrawStringFcn=observed_string;
}
void write_output(const char* api,b3WorldId id,const DrawOutputs& o){
 if(const char* path=std::getenv("GPU_PR10_API_TRACE"))if(FILE* f=std::fopen(path,"a")){
  b3Counters counters=b3World_GetCounters(id);
  std::fprintf(f,"{\"api\":\"%s\",\"world\":[%u,%u],\"bodies\":%d,\"joints\":%d,\"contacts\":%d",api,(unsigned)id.index1,(unsigned)id.generation,counters.bodyCount,counters.jointCount,counters.contactCount);
  const char* names[]={"shape","segment","transform","point","sphere","capsule","bounds","box","string"};
  for(int i=0;i<9;++i)std::fprintf(f,",\"%s_callbacks\":%u,\"%s_digest\":\"%016llx\"",names[i],o.count[i],names[i],(unsigned long long)o.digest[i]);
  std::fputs("}\n",f);std::fclose(f);
 }
}
}
extern "C" void __wrap_b3World_Draw(b3WorldId id,b3DebugDraw* d,uint64_t mask){
 if(!d){__real_b3World_Draw(id,d,mask);return;}
 b3DebugDraw local=*d;DrawOutputs outputs;outputs.original=*d;last_original=*d;observe_callbacks(local);
 DrawOutputs* previous=active_output;active_output=&outputs;trace_draw_impl(id,&local,mask);active_output=previous;write_output("actual_draw_outputs",id,outputs);
}
extern "C" void __real_gpu_samples_world_draw(b3WorldId,b3DebugDraw*,uint64_t) __attribute__((weak));
extern "C" void __wrap_gpu_samples_world_draw(b3WorldId id,b3DebugDraw* d,uint64_t mask){
 assert(__real_gpu_samples_world_draw);if(!d){__real_gpu_samples_world_draw(id,d,mask);return;}
 // A split second-pane draw occurs after the outer wrapper returned. Restore
 // original callback provenance rather than chaining observed callbacks.
 DrawOutputs outputs;outputs.original=active_output?active_output->original:last_original;
 b3DebugDraw local=*d;observe_callbacks(local);DrawOutputs* previous=active_output;active_output=&outputs;
 __real_gpu_samples_world_draw(id,&local,mask);active_output=previous;
 if(previous)for(int i=0;i<9;++i){previous->count[i]+=outputs.count[i];previous->digest[i]=outputs.digest[i];}
 write_output("actual_gpu_draw_outputs",id,outputs);
}
#include "gfx/renderer.h"
#include "gfx/shadow.h"
#include "gfx/gtao.h"
static void body_metadata_request();
static void geometry_frame();
static void tool_frame();
static void primitive_frame();
static void mutation_frame();
extern "C" void __real_RenderFrame(const sg_swapchain*,const FrameInput*);
extern "C" void __wrap_RenderFrame(const sg_swapchain* sc,const FrameInput* frame) {
 if(const char* path=std::getenv("GPU_PR10_API_TRACE"))if(FILE* f=std::fopen(path,"a")) {
  Sun sun=GetSun();EdgeOverlayParams edges=GetEdgeOverlayParams();GtaoTraceParams gtao=GetGtaoTraceParams();
  std::fprintf(f,"{\"api\":\"actual_RenderFrame\",\"debugMode\":%d,\"disableShadows\":%d,\"disableAmbientOcclusion\":%d,\"exposure\":%.9g,\"sun\":%.9g,\"ibl\":%d,\"hullEdges\":%d,\"edgeConvexity\":%d,\"shadowStart\":%.9g,\"shadowRange\":%.9g,\"shadowDistribution\":%.9g,\"gtaoQuality\":%d}\n",frame->debugMode,frame->disableShadows,frame->disableAmbientOcclusion,GetExposure(),sun.strength,GetIblEnabled(),edges.showHulls,edges.showEdgeConvexity,GetShadowSplitNear(),GetShadowSplitFar(),GetShadowSplitLambda(),(int)gtao.quality);std::fclose(f);
 }
 __real_RenderFrame(sc,frame);body_metadata_request();geometry_frame();tool_frame();primitive_frame();mutation_frame();
}
#include "box3d/box3d.h"
#include "gfx/debug_adapter.h"
extern "C" bool cpu_b3Body_IsValid(b3BodyId) __attribute__((weak));
extern "C" b3BodyType cpu_b3Body_GetType(b3BodyId) __attribute__((weak));
extern "C" bool cpu_b3Body_IsEnabled(b3BodyId) __attribute__((weak));
extern "C" b3Pos cpu_b3Body_GetPosition(b3BodyId) __attribute__((weak));
extern "C" b3Vec3 cpu_b3Body_GetLinearVelocity(b3BodyId) __attribute__((weak));
extern "C" float cpu_b3Body_GetMass(b3BodyId) __attribute__((weak));
extern "C" const char* cpu_b3Body_GetName(b3BodyId) __attribute__((weak));
namespace {
struct WatchedBody { b3BodyId id; bool cpu; };
std::vector<WatchedBody> watched_bodies;
b3BodyId selected_cpu{};
bool body_valid(WatchedBody b) { return b.id.index1!=0&&(b.cpu?(cpu_b3Body_IsValid&&cpu_b3Body_IsValid(b.id)):b3Body_IsValid(b.id)); }
bool same_body(WatchedBody a,WatchedBody b) {return a.cpu==b.cpu&&a.id.index1==b.id.index1&&a.id.world0==b.id.world0&&a.id.generation==b.id.generation;}
void watch_body(WatchedBody b) {
 for(auto w:watched_bodies)if(same_body(w,b))return;
 if(watched_bodies.size()>=16) {
  for(auto i=watched_bodies.begin();i!=watched_bodies.end();)if(!body_valid(*i))i=watched_bodies.erase(i);else ++i;
 }
 if(watched_bodies.size()<16)watched_bodies.push_back(b);
}
void body_json(FILE* f,WatchedBody b) {
 const bool valid=body_valid(b);
 std::fprintf(f,"{\"owner\":\"%s\",\"id\":[%u,%u,%u],\"valid\":%s",b.cpu?"mapped_cpu":"public",(unsigned)b.id.index1,(unsigned)b.id.generation,(unsigned)b.id.world0,valid?"true":"false");
 if(valid) {
  b3BodyType type=b.cpu?cpu_b3Body_GetType(b.id):b3Body_GetType(b.id);
  bool enabled=b.cpu?cpu_b3Body_IsEnabled(b.id):b3Body_IsEnabled(b.id);
  b3Pos p=b.cpu?cpu_b3Body_GetPosition(b.id):b3Body_GetPosition(b.id);
  b3Vec3 v=b.cpu?cpu_b3Body_GetLinearVelocity(b.id):b3Body_GetLinearVelocity(b.id);
  float mass=b.cpu?cpu_b3Body_GetMass(b.id):b3Body_GetMass(b.id);
  const char* name=b.cpu?cpu_b3Body_GetName(b.id):b3Body_GetName(b.id);
  std::fprintf(f,",\"type\":%d,\"enabled\":%s,\"position\":[%.9g,%.9g,%.9g],\"velocity\":[%.9g,%.9g,%.9g],\"mass\":%.9g,\"name\":",(int)type,enabled?"true":"false",(double)p.x,(double)p.y,(double)p.z,(double)v.x,(double)v.y,(double)v.z,(double)mass);quote(f,name?name:"");
 }
 std::fputc('}',f);
}
void body_call(const char* api,const char* phase,WatchedBody b,int input=-1) {
 const char* path=std::getenv("GPU_PR10_API_TRACE");if(!path)return;
 if(FILE* f=std::fopen(path,"a")) {std::fprintf(f,"{\"api\":\"%s\",\"phase\":\"%s\",\"input\":%d,\"body\":",api,phase,input);body_json(f,b);std::fputs("}\n",f);std::fclose(f);}
}
}
#define BODYTYPE(NAME,CPU) extern "C" void __real_##NAME(b3BodyId,b3BodyType) __attribute__((weak)); extern "C" void __wrap_##NAME(b3BodyId id,b3BodyType type) { assert(__real_##NAME);WatchedBody b{id,CPU};watch_body(b);body_call(#NAME,"before",b,(int)type);__real_##NAME(id,type);body_call(#NAME,"after",b,(int)type); }
#define BODYFLAG(NAME,CPU) extern "C" void __real_##NAME(b3BodyId) __attribute__((weak)); extern "C" void __wrap_##NAME(b3BodyId id) { assert(__real_##NAME);WatchedBody b{id,CPU};watch_body(b);body_call(#NAME,"before",b);__real_##NAME(id);body_call(#NAME,"after",b); }
#define BODYVEL(NAME,CPU) extern "C" void __real_##NAME(b3BodyId,b3Vec3) __attribute__((weak)); extern "C" void __wrap_##NAME(b3BodyId id,b3Vec3 v) { assert(__real_##NAME);WatchedBody b{id,CPU};watch_body(b);body_call(#NAME,"before",b);__real_##NAME(id,v);body_call(#NAME,"after",b); }
BODYTYPE(b3Body_SetType,false)
BODYTYPE(cpu_b3Body_SetType,true)
BODYFLAG(b3Body_Enable,false)
BODYFLAG(b3Body_Disable,false)
BODYFLAG(cpu_b3Body_Enable,true)
BODYFLAG(cpu_b3Body_Disable,true)
BODYVEL(b3Body_SetLinearVelocity,false)
BODYVEL(cpu_b3Body_SetLinearVelocity,true)
extern "C" void __real_SetComparisonSelectedBody(b3BodyId) __attribute__((weak));
extern "C" void __wrap_SetComparisonSelectedBody(b3BodyId id) { assert(__real_SetComparisonSelectedBody);selected_cpu=id;__real_SetComparisonSelectedBody(id); }
static void body_metadata_request() {
 const char* path=std::getenv("GPU_PR10_API_TRACE"),* ui=std::getenv("GPU_PR10_UI_TRACE");if(!path||!ui)return;
 static unsigned last=0;unsigned request=0;std::string input=std::string(ui)+".request";FILE* rf=std::fopen(input.c_str(),"r");if(!rf)return;int read=std::fscanf(rf,"%u",&request);std::fclose(rf);if(read!=1||request==last)return;last=request;
 if(FILE* f=std::fopen(path,"a")) {
  RenderStats st=GetRenderStats();
  std::fprintf(f,"{\"api\":\"actual_body_metadata\",\"request\":%u,\"selected\":",request);body_json(f,{GetSelectedBody(),false});std::fputs(",\"selected_cpu\":",f);body_json(f,{selected_cpu,true});std::fputs(",\"watched\":[",f);
  for(size_t i=0;i<watched_bodies.size();++i) {if(i)std::fputc(',',f);body_json(f,watched_bodies[i]);}
  std::fprintf(f,"],\"render_counts\":{\"opaque\":%d,\"transparent\":%d,\"lines\":%d,\"points\":%d}}\n",st.cubeCount+st.sphereCount+st.capsuleCount+st.geomInstanceCount,st.cubeCountXp+st.sphereCountXp+st.capsuleCountXp+st.geomInstanceCountXp,st.lineCount,st.pointCount);std::fclose(f);
 }
}

#include "gfx/draw.h"
namespace {
std::vector<std::string> direct_hulls;unsigned direct_hull_count=0;unsigned factory_create=0,factory_clone=0,factory_box=0;
uint64_t geometry_hash(const b3HullData* hull){uint64_t h=1469598103934665603ull;const b3Vec3* points=b3GetHullPoints(hull);for(int i=0;i<hull->vertexCount;++i){float values[]={points[i].x,points[i].y,points[i].z};for(float v:values){const unsigned char* p=(const unsigned char*)&v;for(size_t j=0;j<sizeof(v);++j)h=(h^p[j])*1099511628211ull;}}return h;}
}
extern "C" void __real_DrawHull(b3WorldTransform,const b3HullData*,Vec4);
static void tool_shape(const char*,b3WorldTransform);
extern "C" void __wrap_DrawHull(b3WorldTransform t,const b3HullData* hull,Vec4 color){
 tool_shape("Hull",t);
 if(std::getenv("GPU_PR10_GEOMETRY_TRACE")){++direct_hull_count;if(direct_hulls.size()<16){char row[2048];
  if(hull&&hull->version==B3_HULL_VERSION&&hull->vertexCount>=0&&hull->vertexCount<=4096)std::snprintf(row,sizeof(row),"{\"valid\":true,\"vertices\":%d,\"faces\":%d,\"half_edges\":%d,\"volume\":%.9g,\"area\":%.9g,\"radius\":%.9g,\"center\":[%.9g,%.9g,%.9g],\"aabb\":[%.9g,%.9g,%.9g,%.9g,%.9g,%.9g],\"points_digest\":\"%016llx\",\"draw_position\":[%.17g,%.17g,%.17g]}",hull->vertexCount,hull->faceCount,hull->edgeCount,hull->volume,hull->surfaceArea,hull->innerRadius,hull->center.x,hull->center.y,hull->center.z,hull->aabb.lowerBound.x,hull->aabb.lowerBound.y,hull->aabb.lowerBound.z,hull->aabb.upperBound.x,hull->aabb.upperBound.y,hull->aabb.upperBound.z,(unsigned long long)geometry_hash(hull),t.p.x,t.p.y,t.p.z);
  else std::snprintf(row,sizeof(row),"{\"valid\":false}");direct_hulls.emplace_back(row);
 }}__real_DrawHull(t,hull,color);
}
static void geometry_frame(){
 if(std::getenv("GPU_PR10_GEOMETRY_TRACE"))if(const char* path=std::getenv("GPU_PR10_API_TRACE"))if(FILE* f=std::fopen(path,"a")){
  std::fprintf(f,"{\"api\":\"actual_geometry_frame\",\"hull_calls\":%u,\"factory_create\":%u,\"factory_clone\":%u,\"factory_box\":%u,\"hulls\":[",direct_hull_count,factory_create,factory_clone,factory_box);for(size_t i=0;i<direct_hulls.size();++i){if(i)std::fputc(',',f);std::fputs(direct_hulls[i].c_str(),f);}std::fputs("]}\n",f);std::fclose(f);
 }direct_hull_count=0;direct_hulls.clear();
}

extern "C" b3HullData* __real_b3CreateHull(const b3Vec3*,int,int);
extern "C" b3HullData* __wrap_b3CreateHull(const b3Vec3* points,int count,int maximum){++factory_create;return __real_b3CreateHull(points,count,maximum);}
extern "C" b3HullData* __real_b3CloneAndTransformHull(const b3HullData*,b3Transform,b3Vec3);
extern "C" b3HullData* __wrap_b3CloneAndTransformHull(const b3HullData* hull,b3Transform transform,b3Vec3 scale){++factory_clone;return __real_b3CloneAndTransformHull(hull,transform,scale);}
extern "C" b3BoxHull __real_b3MakeScaledBoxHull(b3Vec3,b3Transform,b3Vec3);
extern "C" b3BoxHull __wrap_b3MakeScaledBoxHull(b3Vec3 widths,b3Transform transform,b3Vec3 scale){++factory_box;return __real_b3MakeScaledBoxHull(widths,transform,scale);}
// Optional no-body geometry-tool probe. Capture values, never substitute calculations.
namespace {
std::vector<std::string> tool_shapes;
std::string tool_collision;
unsigned tool_collision_calls=0;
std::string transform_json(b3WorldTransform t){char s[512];std::snprintf(s,sizeof(s),"{\"p\":[%.17g,%.17g,%.17g],\"q\":[%.9g,%.9g,%.9g,%.9g]}",t.p.x,t.p.y,t.p.z,t.q.v.x,t.q.v.y,t.q.v.z,t.q.s);return s;}
std::string relative_json(b3Transform t){b3WorldTransform w;w.p={t.p.x,t.p.y,t.p.z};w.q=t.q;return transform_json(w);}
std::string cache_json(const b3SimplexCache* c){if(!c)return "null";char s[512];std::snprintf(s,sizeof(s),"{\"kind\":\"simplex\",\"count\":%u,\"metric\":%.9g,\"a\":[%u,%u,%u,%u],\"b\":[%u,%u,%u,%u]}",c->count,c->metric,c->indexA[0],c->indexA[1],c->indexA[2],c->indexA[3],c->indexB[0],c->indexB[1],c->indexB[2],c->indexB[3]);return s;}
std::string cache_json(const b3SATCache* c){if(!c)return "null";char s[512];std::snprintf(s,sizeof(s),"{\"kind\":\"sat\",\"type\":%u,\"separation\":%.9g,\"a\":%u,\"b\":%u,\"hit\":%u}",c->type,c->separation,c->indexA,c->indexB,c->hit);return s;}
std::string manifold_json(const b3LocalManifold* m,int capacity){char s[512];std::snprintf(s,sizeof(s),"{\"count\":%d,\"capacity\":%d,\"normal\":[%.9g,%.9g,%.9g],\"feature\":%d,\"points\":[",m->pointCount,capacity,m->normal.x,m->normal.y,m->normal.z,(int)m->feature);std::string out=s;for(int i=0;i<m->pointCount&&i<capacity&&i<64;++i){if(i)out+=",";auto p=m->points[i];std::snprintf(s,sizeof(s),"[%.9g,%.9g,%.9g,%.9g]",p.point.x,p.point.y,p.point.z,p.separation);out+=s;}return out+"]}";}
void collision_record(const char* name,const std::string& input,const std::string& before,const std::string& after,const b3LocalManifold* m,int capacity){if(!std::getenv("GPU_PR10_TOOL_TRACE"))return;++tool_collision_calls;tool_collision=std::string("{\"name\":\"")+name+"\",\"input\":"+input+",\"cache_before\":"+before+",\"cache_after\":"+after+",\"manifold\":"+manifold_json(m,capacity)+"}";}
std::string triangle_json(const b3Vec3* p){char s[1024];std::snprintf(s,sizeof(s),"{\"triangle\":[[%.9g,%.9g,%.9g],[%.9g,%.9g,%.9g],[%.9g,%.9g,%.9g]]}",p[0].x,p[0].y,p[0].z,p[1].x,p[1].y,p[1].z,p[2].x,p[2].y,p[2].z);return s;}
}
static void tool_shape(const char* name,b3WorldTransform t){if(std::getenv("GPU_PR10_TOOL_TRACE")&&tool_shapes.size()<32)tool_shapes.push_back(std::string("{\"kind\":\"")+name+"\",\"transform\":"+transform_json(t)+"}");}
extern "C" void __real_DrawSolidSphere(b3WorldTransform,b3Sphere,Vec4);
extern "C" void __wrap_DrawSolidSphere(b3WorldTransform t,b3Sphere s,Vec4 c){tool_shape("Sphere",t);__real_DrawSolidSphere(t,s,c);}
extern "C" void __real_DrawSolidCapsule(b3WorldTransform,b3Capsule,Vec4);
extern "C" void __wrap_DrawSolidCapsule(b3WorldTransform t,b3Capsule s,Vec4 c){tool_shape("Capsule",t);__real_DrawSolidCapsule(t,s,c);}
extern "C" void __real_DrawTriangle(b3WorldTransform,b3Vec3,b3Vec3,b3Vec3,Vec4);
extern "C" void __wrap_DrawTriangle(b3WorldTransform t,b3Vec3 a,b3Vec3 b,b3Vec3 c,Vec4 color){tool_shape("Triangle",t);__real_DrawTriangle(t,a,b,c,color);}
static void tool_frame(){
 if(std::getenv("GPU_PR10_TOOL_TRACE"))if(const char* ui=trace_path())if(const char* path=std::getenv("GPU_PR10_API_TRACE")){
  static unsigned last=0;unsigned request=0;FILE* rf=std::fopen((std::string(ui)+".request").c_str(),"r");if(rf){const int got=std::fscanf(rf,"%u",&request);std::fclose(rf);if(got==1&&request!=last)if(FILE* f=std::fopen(path,"a")){std::fprintf(f,"{\"api\":\"actual_tool_frame\",\"request\":%u,\"collision_calls\":%u,\"collision\":%s,\"shapes\":[",request,tool_collision_calls,tool_collision.empty()?"null":tool_collision.c_str());for(size_t i=0;i<tool_shapes.size();++i){if(i)std::fputc(',',f);std::fputs(tool_shapes[i].c_str(),f);}std::fputs("]}\n",f);std::fclose(f);last=request;}}
 }tool_shapes.clear();
}

extern "C" void __real_b3CollideSpheres(b3LocalManifold* manifold, int capacity, const b3Sphere* sphereA, const b3Sphere* sphereB, b3Transform transformBtoA);
extern "C" void __wrap_b3CollideSpheres(b3LocalManifold* manifold, int capacity, const b3Sphere* sphereA, const b3Sphere* sphereB, b3Transform transformBtoA){const std::string input=relative_json(transformBtoA),before=std::string("null");__real_b3CollideSpheres(manifold,capacity,sphereA,sphereB,transformBtoA);collision_record("b3CollideSpheres",input,before,std::string("null"),manifold,capacity);}

extern "C" void __real_b3CollideCapsuleAndSphere(b3LocalManifold* manifold, int capacity, const b3Capsule* capsuleA, const b3Sphere* sphereB, b3Transform transformBtoA);
extern "C" void __wrap_b3CollideCapsuleAndSphere(b3LocalManifold* manifold, int capacity, const b3Capsule* capsuleA, const b3Sphere* sphereB, b3Transform transformBtoA){const std::string input=relative_json(transformBtoA),before=std::string("null");__real_b3CollideCapsuleAndSphere(manifold,capacity,capsuleA,sphereB,transformBtoA);collision_record("b3CollideCapsuleAndSphere",input,before,std::string("null"),manifold,capacity);}

extern "C" void __real_b3CollideHullAndSphere(b3LocalManifold* manifold, int capacity, const b3HullData* hullA, const b3Sphere* sphereB, b3Transform transformBtoA, b3SimplexCache* cache);
extern "C" void __wrap_b3CollideHullAndSphere(b3LocalManifold* manifold, int capacity, const b3HullData* hullA, const b3Sphere* sphereB, b3Transform transformBtoA, b3SimplexCache* cache){const std::string input=relative_json(transformBtoA),before=cache_json(cache);__real_b3CollideHullAndSphere(manifold,capacity,hullA,sphereB,transformBtoA,cache);collision_record("b3CollideHullAndSphere",input,before,cache_json(cache),manifold,capacity);}

extern "C" void __real_b3CollideCapsules(b3LocalManifold* manifold, int capacity, const b3Capsule* capsuleA, const b3Capsule* capsuleB, b3Transform transformBtoA);
extern "C" void __wrap_b3CollideCapsules(b3LocalManifold* manifold, int capacity, const b3Capsule* capsuleA, const b3Capsule* capsuleB, b3Transform transformBtoA){const std::string input=relative_json(transformBtoA),before=std::string("null");__real_b3CollideCapsules(manifold,capacity,capsuleA,capsuleB,transformBtoA);collision_record("b3CollideCapsules",input,before,std::string("null"),manifold,capacity);}

extern "C" void __real_b3CollideHullAndCapsule(b3LocalManifold* manifold, int capacity, const b3HullData* hullA, const b3Capsule* capsuleB, b3Transform transformBtoA, b3SimplexCache* cache);
extern "C" void __wrap_b3CollideHullAndCapsule(b3LocalManifold* manifold, int capacity, const b3HullData* hullA, const b3Capsule* capsuleB, b3Transform transformBtoA, b3SimplexCache* cache){const std::string input=relative_json(transformBtoA),before=cache_json(cache);__real_b3CollideHullAndCapsule(manifold,capacity,hullA,capsuleB,transformBtoA,cache);collision_record("b3CollideHullAndCapsule",input,before,cache_json(cache),manifold,capacity);}

extern "C" void __real_b3CollideHulls(b3LocalManifold* manifold, int capacity, const b3HullData* hullA, const b3HullData* hullB, b3Transform transformBtoA, b3SATCache* cache);
extern "C" void __wrap_b3CollideHulls(b3LocalManifold* manifold, int capacity, const b3HullData* hullA, const b3HullData* hullB, b3Transform transformBtoA, b3SATCache* cache){const std::string input=relative_json(transformBtoA),before=cache_json(cache);__real_b3CollideHulls(manifold,capacity,hullA,hullB,transformBtoA,cache);collision_record("b3CollideHulls",input,before,cache_json(cache),manifold,capacity);}

extern "C" void __real_b3CollideTriangleAndCapsule(b3LocalManifold* manifold, int capacity, const b3Vec3* triangleA, const b3Capsule* capsuleB, b3SimplexCache* cache);
extern "C" void __wrap_b3CollideTriangleAndCapsule(b3LocalManifold* manifold, int capacity, const b3Vec3* triangleA, const b3Capsule* capsuleB, b3SimplexCache* cache){const std::string input=triangle_json(triangleA),before=cache_json(cache);__real_b3CollideTriangleAndCapsule(manifold,capacity,triangleA,capsuleB,cache);collision_record("b3CollideTriangleAndCapsule",input,before,cache_json(cache),manifold,capacity);}

extern "C" void __real_b3CollideTriangleAndHull(b3LocalManifold* manifold, int capacity, b3Vec3 v1, b3Vec3 v2, b3Vec3 v3, int triangleFlags, const b3HullData* hullB, b3SATCache* cache, bool enableSpeculative);
extern "C" void __wrap_b3CollideTriangleAndHull(b3LocalManifold* manifold, int capacity, b3Vec3 v1, b3Vec3 v2, b3Vec3 v3, int triangleFlags, const b3HullData* hullB, b3SATCache* cache, bool enableSpeculative){b3Vec3 triangle[3]={v1,v2,v3};const std::string input=triangle_json(triangle),before=cache_json(cache);__real_b3CollideTriangleAndHull(manifold,capacity,v1,v2,v3,triangleFlags,hullB,cache,enableSpeculative);collision_record("b3CollideTriangleAndHull",input,before,cache_json(cache),manifold,capacity);}

extern "C" void __real_b3CollideTriangleAndSphere(b3LocalManifold* manifold, int capacity, const b3Vec3* triangleA, const b3Sphere* sphereB);
extern "C" void __wrap_b3CollideTriangleAndSphere(b3LocalManifold* manifold, int capacity, const b3Vec3* triangleA, const b3Sphere* sphereB){const std::string input=triangle_json(triangleA),before=std::string("null");__real_b3CollideTriangleAndSphere(manifold,capacity,triangleA,sphereB);collision_record("b3CollideTriangleAndSphere",input,before,std::string("null"),manifold,capacity);}
#include "gfx/text.h"
#include <cmath>
namespace {
std::string last_distance,last_toi;
uint64_t line_hash=1469598103934665603ull,point_hash=1469598103934665603ull;
unsigned line_count=0,point_count=0;
void hash_value(uint64_t& h,float v){unsigned char bytes[sizeof(v)];std::memcpy(bytes,&v,sizeof(v));for(unsigned char c:bytes)h=(h^c)*1099511628211ull;}
std::string number_json(float v){if(!std::isfinite(v))return "null";char s[64];std::snprintf(s,sizeof(s),"%.9g",v);return s;}
std::string vec_json(b3Vec3 v){return "["+number_json(v.x)+","+number_json(v.y)+","+number_json(v.z)+"]";}
std::string proxy_json(const b3ShapeProxy& p){std::string s="{\"count\":"+std::to_string(p.count)+",\"radius\":"+number_json(p.radius)+",\"points\":[";if(p.points&&p.count>0&&p.count<=32)for(int i=0;i<p.count;++i){if(i)s+=",";s+=vec_json(p.points[i]);}return s+"]}";}
std::string sweep_json(const b3Sweep& s){return "{\"center1\":"+vec_json(s.c1)+",\"center2\":"+vec_json(s.c2)+",\"q1\":"+transform_json({{0,0,0},s.q1})+",\"q2\":"+transform_json({{0,0,0},s.q2})+"}";}
std::string toi_input_json(const b3TOIInput* input){return "{\"A\":"+proxy_json(input->proxyA)+",\"B\":"+proxy_json(input->proxyB)+",\"sweepA\":"+sweep_json(input->sweepA)+",\"sweepB\":"+sweep_json(input->sweepB)+",\"maxFraction\":"+number_json(input->maxFraction)+"}";}
}
extern "C" b3DistanceOutput __real_b3ShapeDistance(const b3DistanceInput*,b3SimplexCache*,b3Simplex*,int);
extern "C" b3DistanceOutput __wrap_b3ShapeDistance(const b3DistanceInput* input,b3SimplexCache* cache,b3Simplex* simplexes,int capacity){
 const bool trace=std::getenv("GPU_PR10_PRIMITIVE_TRACE")!=nullptr;
 const std::string before=trace?cache_json(cache):"null";
 bool invalid=cache&&cache->count>4;
 if(trace&&cache)for(int i=0;i<cache->count&&i<4;++i)invalid|=cache->indexA[i]>=input->proxyA.count||cache->indexB[i]>=input->proxyB.count;
 if(trace&&invalid)if(const char* p=std::getenv("GPU_PR10_API_TRACE"))if(FILE* f=std::fopen(p,"a")){std::string input_json="{\"A\":"+proxy_json(input->proxyA)+",\"B\":"+proxy_json(input->proxyB)+"}";std::fprintf(f,"{\"api\":\"actual_invalid_distance_cache\",\"input\":%s,\"cache_before\":%s}\n",input_json.c_str(),before.c_str());std::fclose(f);}
 b3DistanceOutput out=__real_b3ShapeDistance(input,cache,simplexes,capacity);
 if(trace){last_distance="{\"input\":{\"A\":"+proxy_json(input->proxyA)+",\"B\":"+proxy_json(input->proxyB)+",\"transform\":"+relative_json(input->transform)+",\"useRadii\":"+(input->useRadii?"true":"false")+"},\"cache_before\":"+before+",\"cache_after\":"+cache_json(cache)+",\"output\":{\"distance\":"+number_json(out.distance)+",\"pointA\":"+vec_json(out.pointA)+",\"pointB\":"+vec_json(out.pointB)+",\"normal\":"+vec_json(out.normal)+",\"iterations\":"+std::to_string(out.iterations)+",\"simplexCount\":"+std::to_string(out.simplexCount)+"},\"simplexes\":[";if(simplexes)for(int i=0;i<out.simplexCount&&i<capacity&&i<32;++i){if(i)last_distance+=",";const auto& s=simplexes[i];last_distance+="{\"count\":"+std::to_string(s.count)+",\"vertices\":[";for(int j=0;j<s.count&&j<4;++j){if(j)last_distance+=",";const auto& v=s.vertices[j];last_distance+="{\"A\":"+vec_json(v.wA)+",\"B\":"+vec_json(v.wB)+",\"w\":"+vec_json(v.w)+",\"a\":"+number_json(v.a)+",\"iA\":"+std::to_string(v.indexA)+",\"iB\":"+std::to_string(v.indexB)+"}";}last_distance+="]}";}last_distance+="]}";
 }return out;
}
extern "C" b3TOIOutput __real_b3TimeOfImpact(const b3TOIInput*);
extern "C" b3TOIOutput __wrap_b3TimeOfImpact(const b3TOIInput* input){
 const bool trace=std::getenv("GPU_PR10_PRIMITIVE_TRACE")!=nullptr;
 if(trace&&(input->proxyA.count<=0||input->proxyB.count<=0))if(const char* p=std::getenv("GPU_PR10_API_TRACE"))if(FILE* f=std::fopen(p,"a")){std::fprintf(f,"{\"api\":\"actual_invalid_TOI_input\",\"input\":%s}\n",toi_input_json(input).c_str());std::fclose(f);}
 b3TOIOutput out=__real_b3TimeOfImpact(input);
 if(trace)last_toi="{\"input\":"+toi_input_json(input)+",\"output\":{\"state\":"+std::to_string(out.state)+",\"fraction\":"+number_json(out.fraction)+",\"distance\":"+number_json(out.distance)+",\"iterations\":"+std::to_string(out.distanceIterations)+"}}";
 return out;
}
extern "C" void __real_DrawLine(b3Pos,b3Pos,Vec4);
extern "C" void __wrap_DrawLine(b3Pos a,b3Pos b,Vec4 c){if(std::getenv("GPU_PR10_PRIMITIVE_TRACE")){++line_count;for(float v:{(float)a.x,(float)a.y,(float)a.z,(float)b.x,(float)b.y,(float)b.z,c.x,c.y,c.z,c.w})hash_value(line_hash,v);}__real_DrawLine(a,b,c);}
extern "C" void __real_DrawPoint(b3Pos,float,Vec4);
extern "C" void __wrap_DrawPoint(b3Pos p,float size,Vec4 c){if(std::getenv("GPU_PR10_PRIMITIVE_TRACE")){++point_count;for(float v:{(float)p.x,(float)p.y,(float)p.z,size,c.x,c.y,c.z,c.w})hash_value(point_hash,v);}__real_DrawPoint(p,size,c);}
static void primitive_frame(){
 if(std::getenv("GPU_PR10_PRIMITIVE_TRACE"))if(const char* ui=trace_path())if(const char* path=std::getenv("GPU_PR10_API_TRACE")){
  static unsigned last=0;unsigned request=0;FILE* rf=std::fopen((std::string(ui)+".request").c_str(),"r");if(rf){int got=std::fscanf(rf,"%u",&request);std::fclose(rf);if(got==1&&request!=last)if(FILE* f=std::fopen(path,"a")){std::fprintf(f,"{\"api\":\"actual_primitive_frame\",\"request\":%u,\"distance\":%s,\"toi\":%s,\"draw\":{\"lines\":%u,\"points\":%u,\"line_digest\":\"%016llx\",\"point_digest\":\"%016llx\"},\"world_labels\":[",request,last_distance.empty()?"null":last_distance.c_str(),last_toi.empty()?"null":last_toi.c_str(),line_count,point_count,(unsigned long long)line_hash,(unsigned long long)point_hash);bool comma=false;for(int i=0;i<GetTextCount()&&i<256;++i){const TextEntry* e=GetTextAt(i);if(e->space!=TEXT_SPACE_WORLD)continue;if(comma)std::fputc(',',f);comma=true;quote(f,e->text?e->text:"");}std::fputs("]}\n",f);std::fclose(f);last=request;}}
 }line_count=point_count=0;line_hash=point_hash=1469598103934665603ull;
}

#include "body-mutation-observer.inc.cpp"
