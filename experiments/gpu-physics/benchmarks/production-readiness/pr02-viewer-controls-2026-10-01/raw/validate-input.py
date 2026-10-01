from pathlib import Path
import ctypes as C,subprocess,time,json,hashlib,os
root=Path.cwd();out=root/'artifacts/production-readiness/pr02-viewer-controls';dest=out/'input-receiver';dest.mkdir(exist_ok=False);display=':98';assert not Path('/tmp/.X98-lock').exists()
xvfb=root/'artifacts/production-readiness/pr01-speculative/handoff/xvfb/usr/bin/Xvfb';log=(dest/'xvfb.log').open('w');server=subprocess.Popen([str(xvfb),display,'-screen','0','1280x720x24','-nolisten','tcp','-ac'],stdout=log,stderr=subprocess.STDOUT)
X=C.CDLL('libX11.so.6');T=C.CDLL('libXtst.so.6');ptr=C.c_void_p;ulong=C.c_ulong
class Button(C.Structure):
 _fields_=[('type',C.c_int),('serial',ulong),('send_event',C.c_int),('display',ptr),('window',ulong),('root',ulong),('subwindow',ulong),('time',ulong),('x',C.c_int),('y',C.c_int),('x_root',C.c_int),('y_root',C.c_int),('state',C.c_uint),('button',C.c_uint),('same_screen',C.c_int)]
class Event(C.Union):_fields_=[('type',C.c_int),('button',Button),('padding',C.c_long*24)]
X.XOpenDisplay.argtypes=[C.c_char_p];X.XOpenDisplay.restype=ptr;X.XDefaultRootWindow.argtypes=[ptr];X.XDefaultRootWindow.restype=ulong
X.XCreateSimpleWindow.argtypes=[ptr,ulong,C.c_int,C.c_int,C.c_uint,C.c_uint,C.c_uint,ulong,ulong];X.XCreateSimpleWindow.restype=ulong
X.XSelectInput.argtypes=[ptr,ulong,C.c_long];X.XMapWindow.argtypes=[ptr,ulong];X.XSetInputFocus.argtypes=[ptr,ulong,C.c_int,ulong];X.XFlush.argtypes=[ptr];X.XPending.argtypes=[ptr];X.XNextEvent.argtypes=[ptr,C.POINTER(Event)];X.XCloseDisplay.argtypes=[ptr];X.XKeysymToKeycode.argtypes=[ptr,ulong];X.XKeysymToKeycode.restype=C.c_uint
T.XTestFakeMotionEvent.argtypes=[ptr,C.c_int,C.c_int,C.c_int,ulong];T.XTestFakeButtonEvent.argtypes=[ptr,C.c_uint,C.c_int,ulong];T.XTestFakeKeyEvent.argtypes=[ptr,C.c_uint,C.c_int,ulong]
r={'scope':'pure host X11 event receiver; no physics or viewer process','pid':os.getpid(),'server_pid':server.pid,'status':'running','events':[]};d=None
try:
 for _ in range(50):
  d=X.XOpenDisplay(display.encode())
  if d:break
  assert server.poll() is None;time.sleep(.1)
 assert d
 window=X.XCreateSimpleWindow(d,X.XDefaultRootWindow(d),0,0,1280,720,0,0,0);X.XSelectInput(d,window,(1<<0)|(1<<1)|(1<<2)|(1<<3));X.XMapWindow(d,window);X.XFlush(d);time.sleep(.15);X.XSetInputFocus(d,window,2,0);X.XFlush(d);time.sleep(.15)
 assert T.XTestFakeMotionEvent(d,-1,400,400,0);X.XFlush(d)
 assert T.XTestFakeButtonEvent(d,1,1,0);X.XFlush(d);time.sleep(.25)
 assert T.XTestFakeButtonEvent(d,1,0,0);X.XFlush(d);time.sleep(.05)
 code=X.XKeysymToKeycode(d,ord('p'));assert code
 assert T.XTestFakeKeyEvent(d,code,1,0);X.XFlush(d);time.sleep(.1)
 assert T.XTestFakeKeyEvent(d,code,0,0);X.XFlush(d);time.sleep(.1)
 while X.XPending(d):
  e=Event();X.XNextEvent(d,C.byref(e))
  if e.type in [2,3,4,5]:
   b=e.button;r['events'].append({'type':e.type,'window':b.window,'time':b.time,'x':b.x,'y':b.y,'x_root':b.x_root,'y_root':b.y_root,'key_or_button':b.button,'same_screen':b.same_screen})
 assert [e['type'] for e in r['events']]==[4,5,2,3]
 assert all(e['window']==window and e['same_screen'] for e in r['events'])
 assert all(e['key_or_button']==1 and (e['x_root'],e['y_root'])==(400,400) for e in r['events'][:2])
 assert all(e['key_or_button']==code for e in r['events'][2:])
 r['mouse_held_ms']=r['events'][1]['time']-r['events'][0]['time'];r['key_held_ms']=r['events'][3]['time']-r['events'][2]['time'];assert r['mouse_held_ms']>=200 and r['key_held_ms']>=90
 r['status']='pass';print('Input receiver pass: held mouse/key observed',r['mouse_held_ms'],r['key_held_ms'],flush=True)
except BaseException as error:
 r.update(status='failed',failure=repr(error));raise
finally:
 if d:X.XCloseDisplay(d)
 server.terminate();server.wait();log.close();r['server_exit']=server.returncode;r['protocol_sha256']=hashlib.sha256((out/'protocol-before-runs.json').read_bytes()).hexdigest();(dest/'receipt.json').write_text(json.dumps(r,indent=2)+'\n')
