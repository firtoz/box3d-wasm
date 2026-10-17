// Read-only diagnostic at the actual native UI text drain boundary.
#include "imgui.h"
#include "gfx/text.h"
#include <cstdio>
#include <cstdlib>
#include <string>
extern void real_start_ui(float) asm("__real__Z12StartUIFramef");
extern void wrap_start_ui(float) asm("__wrap__Z12StartUIFramef");
static void hud_quote(FILE* f,const char* s){std::fputc('"',f);for(const unsigned char* c=(const unsigned char*)s;*c;++c){if(*c=='"'||*c=='\\')std::fprintf(f,"\\%c",*c);else if(*c<32)std::fprintf(f,"\\u%04x",*c);else std::fputc(*c,f);}std::fputc('"',f);}
void wrap_start_ui(float dt){
 real_start_ui(dt);
 if(!std::getenv("GPU_PR10_HUD_TRACE")||!ImGui::GetCurrentContext())return;
 const char* path=std::getenv("GPU_PR10_API_TRACE"),*ui=std::getenv("GPU_PR10_UI_TRACE");if(!path||!ui)return;
 static unsigned last=0;unsigned request=0;FILE* rf=std::fopen((std::string(ui)+".request").c_str(),"r");if(!rf)return;const int n=std::fscanf(rf,"%u",&request);std::fclose(rf);if(n!=1||request==last)return;last=request;
 if(FILE* f=std::fopen(path,"a")){std::fprintf(f,"{\"api\":\"actual_hud_frame\",\"request\":%u,\"frame\":%d,\"font_size\":%.9g,\"labels\":[",request,ImGui::GetFrameCount(),ImGui::GetFontSize());bool comma=false;for(int i=0;i<GetTextCount()&&i<256;++i){const TextEntry* e=GetTextAt(i);if(e->space!=TEXT_SPACE_SCREEN)continue;if(comma)std::fputc(',',f);comma=true;const ImVec2 size=ImGui::CalcTextSize(e->text);std::fputs("{\"text\":",f);hud_quote(f,e->text);std::fprintf(f,",\"position\":[%d,%d],\"color\":[%.9g,%.9g,%.9g,%.9g],\"bounds\":[%.9g,%.9g,%.9g,%.9g]}",e->screenX,e->screenY,e->color.x,e->color.y,e->color.z,e->color.w,float(e->screenX),float(e->screenY),e->screenX+size.x,e->screenY+size.y);}std::fputs("]}\n",f);std::fclose(f);}
}
