// Test-only X11 observation; forwards the exact call and reports its actual XID.
// Sokol does not publish _NET_WM_PID, so PID-property window searches cannot work.
#define _GNU_SOURCE
#include <X11/Xlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
Window XCreateWindow(Display* d,Window parent,int x,int y,unsigned w,unsigned h,
                     unsigned border,int depth,unsigned cls,Visual* visual,
                     unsigned long mask,XSetWindowAttributes* attributes) {
    typedef Window(*Create)(Display*,Window,int,int,unsigned,unsigned,unsigned,int,unsigned,Visual*,unsigned long,XSetWindowAttributes*);
    Create original=(Create)dlsym(RTLD_NEXT,"XCreateWindow");
    Window window=original(d,parent,x,y,w,h,border,depth,cls,visual,mask,attributes);
    fprintf(stderr,"PR05WINDOW pid=%ld id=%lu\n",(long)getpid(),window);
    return window;
}
