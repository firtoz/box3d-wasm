// Send WM_DELETE_WINDOW to this fixture's exact window, without killing X11.
#include <X11/Xlib.h>
#include <cstdlib>
#include <cassert>
int main(int argc,char**argv){
    assert(argc==2);auto*d=XOpenDisplay(nullptr);assert(d);
    XEvent e{};e.xclient.type=ClientMessage;e.xclient.window=std::strtoul(argv[1],nullptr,0);
    e.xclient.message_type=XInternAtom(d,"WM_PROTOCOLS",False);e.xclient.format=32;
    e.xclient.data.l[0]=XInternAtom(d,"WM_DELETE_WINDOW",False);e.xclient.data.l[1]=CurrentTime;
    assert(XSendEvent(d,e.xclient.window,False,NoEventMask,&e));XSync(d,False);XCloseDisplay(d);
}
