// Read-only trace around the actual shared GUI event handler.
#include "sokol_app.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <cstdio>
#include <cstdlib>
extern bool real_event(const sapp_event*) asm("__real__Z11HandleEventPK10sapp_event");
extern bool wrap_event(const sapp_event*) asm("__wrap__Z11HandleEventPK10sapp_event");
bool wrap_event(const sapp_event* e) {
    bool wanted_before=false;int frame=-1;ImRect clip{};bool inside=false;
    if(ImGui::GetCurrentContext()) {
        wanted_before=ImGui::GetIO().WantCaptureMouse;frame=ImGui::GetFrameCount();
        if(auto* w=ImGui::FindWindowByName("Info")) {clip=w->InnerClipRect;inside=clip.Contains(ImVec2(e->mouse_x,e->mouse_y));}
    }
    const bool captured=real_event(e);
    if(const char* path=std::getenv("GPU_PR10_INPUT_TRACE")) {
        if(FILE* f=std::fopen(path,"a")) {
            std::fprintf(f,"{\"type\":%d,\"sapp_frame\":%llu,\"imgui_frame_before\":%d,\"point\":[%.6f,%.6f],\"scroll\":[%.6f,%.6f],\"modifiers\":%u,\"inside_info\":%s,\"wanted_before\":%s,\"wanted_after\":%s,\"captured\":%s,\"info_clip\":[%.6f,%.6f,%.6f,%.6f]}\n",(int)e->type,(unsigned long long)e->frame_count,frame,e->mouse_x,e->mouse_y,e->scroll_x,e->scroll_y,e->modifiers,inside?"true":"false",wanted_before?"true":"false",ImGui::GetCurrentContext()&&ImGui::GetIO().WantCaptureMouse?"true":"false",captured?"true":"false",clip.Min.x,clip.Min.y,clip.Max.x,clip.Max.y);
            std::fclose(f);
        }
    }
    return captured;
}
