#include "box3d/id.h"
#include <cstdio>
#include <cstdint>
#include <unistd.h>
extern "C" const void* sapp_x11_get_window(void);
extern "C" int sapp_width(void);
extern "C" int sapp_height(void);
extern bool real_poll(b3WorldId) asm("__real__Z16gpu_loading_poll9b3WorldId");
extern bool wrapped_poll(b3WorldId) asm("__wrap__Z16gpu_loading_poll9b3WorldId");
bool wrapped_poll(b3WorldId world) {
    static bool seen = false;
    const auto window = sapp_x11_get_window();
    if (!seen && window && sapp_width() > 0 && sapp_height() > 0) {
        seen = true;
        std::fprintf(stderr,"PR10WINDOW,pid=%ld,id=%llu,width=%d,height=%d\n",(long)getpid(),(unsigned long long)(uintptr_t)window,sapp_width(),sapp_height());
        std::fflush(stderr);
    }
    return real_poll(world);
}
