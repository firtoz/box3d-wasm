from pathlib import Path
import os,sys,subprocess,time,json,ctypes,hashlib,re
root=Path.cwd();base=root/'artifacts/production-readiness/pr02-world-lifetime-viewer-builds';config=sys.argv[1];campaign=root/'artifacts/production-readiness/pr02-world-lifetime-adaptive-apps';protocol=campaign/'protocol-before-runs.json';P=json.loads(protocol.read_text());order=P['order'];assert config in order
assert json.loads(Path(P['offline_cpu_prerequisite']['acceptance_file']).read_text())['status']=='pass'
for prior in order[:order.index(config)]:assert json.loads((campaign/prior/'acceptance.json').read_text())['status']=='pass'
assert json.loads((base/'receipt.json').read_text())['status']=='built'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();build=json.loads((base/config/'receipt.json').read_text());binary=Path(build['executable']);assert sha(binary)==build['executable_sha256']
dest=campaign/config;dest.mkdir(exist_ok=False);(dest/'settings.ini').write_text('{}\n');display=':97';assert not Path('/tmp/.X97-lock').exists()
xlog=(dest/'xvfb.log').open('w');xvfb=root/'artifacts/production-readiness/pr01-speculative/handoff/xvfb/usr/bin/Xvfb';server=subprocess.Popen([str(xvfb),display,'-screen','0','1280x720x24','-nolisten','tcp','-ac'],stdout=xlog,stderr=subprocess.STDOUT)
env={k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_','WGPU_','GPU_RECORD_','GPU_BENCH_','GPU_VIEWER_'))}
env.update(DISPLAY=display,GPU_BENCH_WIDTH='1280',GPU_BENCH_HEIGHT='720',GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(campaign/'pipelines'),__NV_PRIME_RENDER_OFFLOAD='0',__GLX_VENDOR_LIBRARY_NAME='mesa',LIBGL_ALWAYS_SOFTWARE='1',GPU_VIEWER_CONTROL_OBSERVER=str(dest/'observer.jsonl'))
if config.startswith('native'):
 raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'])
 for item in raw.split(b'\0'):
  if item.startswith((b'GPU_PHYSICS_',b'WGPU_')):k,v=item.decode().split('=',1);env[k]=v
app=ff=None;d=None;X=ctypes.CDLL('libX11.so.6');T=ctypes.CDLL('libXtst.so.6');ptr=ctypes.c_void_p;ulong=ctypes.c_ulong
X.XOpenDisplay.argtypes=[ctypes.c_char_p];X.XOpenDisplay.restype=ptr;X.XKeysymToKeycode.argtypes=[ptr,ulong];X.XKeysymToKeycode.restype=ctypes.c_uint;X.XSetInputFocus.argtypes=[ptr,ulong,ctypes.c_int,ulong];X.XFlush.argtypes=[ptr];X.XCloseDisplay.argtypes=[ptr]
T.XTestFakeKeyEvent.argtypes=[ptr,ctypes.c_uint,ctypes.c_int,ulong];T.XTestFakeMotionEvent.argtypes=[ptr,ctypes.c_int,ctypes.c_int,ctypes.c_int,ulong];T.XTestFakeButtonEvent.argtypes=[ptr,ctypes.c_uint,ctypes.c_int,ulong]
r={'configuration':config,'purpose':'actual native controls, read-only observer, diagnostic only; no timing or sample-wide qualification','binary_sha256':sha(binary),'build_receipt_sha256':sha(base/config/'receipt.json'),'protocol_sha256':sha(protocol),'actions':[],'environment':{k:v for k,v in env.items()if k.startswith(('GPU_','WGPU_','DISPLAY','VK_','__NV','__GLX','LIBGL'))},'driver_pid':os.getpid(),'status':'starting','screenshots':{}}
def save():(dest/'receipt.json').write_text(json.dumps(r,indent=2)+'\n')
def state():
 p=dest/'observer.jsonl'
 if not p.exists():return None
 with p.open('rb')as f:
  f.seek(max(0,p.stat().st_size-16384));lines=f.read().splitlines()
 for line in reversed(lines):
  try:return json.loads(line)
  except json.JSONDecodeError:pass
 return None
def key(symbol,down):
 code=X.XKeysymToKeycode(d,symbol);assert code;assert T.XTestFakeKeyEvent(d,code,int(down),0);X.XFlush(d)
def tap(symbol):key(symbol,True);time.sleep(.12);key(symbol,False)
X.XDefaultRootWindow.argtypes=[ptr];X.XDefaultRootWindow.restype=ulong
X.XQueryTree.argtypes=[ptr,ulong,ctypes.POINTER(ulong),ctypes.POINTER(ulong),ctypes.POINTER(ctypes.POINTER(ulong)),ctypes.POINTER(ctypes.c_uint)];X.XQueryTree.restype=ctypes.c_int
X.XFetchName.argtypes=[ptr,ulong,ctypes.POINTER(ptr)];X.XFetchName.restype=ctypes.c_int
X.XGetInputFocus.argtypes=[ptr,ctypes.POINTER(ulong),ctypes.POINTER(ctypes.c_int)]
X.XSync.argtypes=[ptr,ctypes.c_int];X.XFree.argtypes=[ptr]
def focus():
 root_window=X.XDefaultRootWindow(d);root_return=ulong();parent=ulong();children=ctypes.POINTER(ulong)();count=ctypes.c_uint()
 assert X.XQueryTree(d,root_window,ctypes.byref(root_return),ctypes.byref(parent),ctypes.byref(children),ctypes.byref(count))
 names=[]
 try:
  for i in range(count.value):
   name=ptr();window=children[i]
   if X.XFetchName(d,window,ctypes.byref(name)) and name:
    text=ctypes.string_at(name).decode('utf-8',errors='replace');X.XFree(name)
    if 'Box3D'in text:names.append((window,text))
 finally:
  if children:X.XFree(children)
 assert len(names)==1,names
 X.XSetInputFocus(d,names[0][0],2,0);X.XSync(d,False);actual=ulong();revert=ctypes.c_int();X.XGetInputFocus(d,ctypes.byref(actual),ctypes.byref(revert));assert actual.value==names[0][0]
 r['focused_window']=names[0];r['focus_observed']=actual.value

try:
 save()
 for _ in range(100):
  d=X.XOpenDisplay(display.encode())
  if d:break
  assert server.poll()is None;time.sleep(.1)
 assert d
 ffcmd=['ffmpeg','-hide_banner','-loglevel','warning','-f','x11grab','-draw_mouse','0','-framerate','30','-video_size','1280x720','-i',display,'-c:v','libx264','-preset','veryfast','-crf','20','-pix_fmt','yuv420p','-movflags','+faststart',str(dest/'clip.mp4')];flog=(dest/'ffmpeg.log').open('w');ff=subprocess.Popen(ffcmd,stdin=subprocess.PIPE,stdout=flog,stderr=subprocess.STDOUT,env=env)
 stdout=(dest/'stdout.log').open('w');stderr=(dest/'stderr.log').open('w');cmd=[str(binary),'--sample-name','Single Box'];app=subprocess.Popen(cmd,cwd=dest,env=env,stdout=stdout,stderr=stderr);r.update(status='running',app_pid=app.pid,command=cmd,capture_command=ffcmd);save();print(config,'running',flush=True)
 start=time.monotonic();handled=set()
 while app.poll()is None and time.monotonic()-start<P['watchdog_seconds']:
  for path in sorted(dest.glob('action-*.json')):
   if path.name in handled:continue
   a=json.loads(path.read_text());assert len(handled)<P['max_actions_per_app'];before=state();at=time.monotonic()-start
   if a['type']in ['key','click','scroll','quit']:focus()
   if a['type']=='key':tap(a['symbol'])
   elif a['type']=='click':
    assert T.XTestFakeMotionEvent(d,-1,a['x'],a['y'],0);X.XFlush(d);time.sleep(.12);assert T.XTestFakeButtonEvent(d,1,1,0);X.XFlush(d);time.sleep(.27);assert T.XTestFakeButtonEvent(d,1,0,0);X.XFlush(d)
   elif a['type']=='scroll':
    assert 1<=abs(a['steps'])<=5;assert T.XTestFakeMotionEvent(d,-1,a['x'],a['y'],0);X.XFlush(d);time.sleep(.12)
    button=5 if a['steps']>0 else 4
    for _ in range(abs(a['steps'])):
     assert T.XTestFakeButtonEvent(d,button,1,0);assert T.XTestFakeButtonEvent(d,button,0,0);X.XFlush(d);time.sleep(.12)
   elif a['type']=='quit':key(0xffe3,True);tap(ord('q'));key(0xffe3,False)
   elif a['type']=='screenshot':
    assert len(r['screenshots'])<P['max_screenshots_per_app'];p=dest/(a['name']+'.png');assert not p.exists();subprocess.run(['ffmpeg','-v','error','-f','x11grab','-video_size','1280x720','-i',display,'-frames:v','1',str(p)],env=env,check=True);r['screenshots'][p.name]=sha(p)
   else:raise ValueError(a)
   time.sleep(.6);r['actions'].append({'file':path.name,'action':a,'at_seconds':at,'state_before':before,'state_after':state()});handled.add(path.name);save();print(path.name,'done',flush=True)
  time.sleep(.1)
 if app.poll()is None:r['watchdog']=True;app.terminate()
 app.wait(timeout=15);ff.communicate(b'q\n',timeout=30);r.update(status='complete'if app.returncode==0 and ff.returncode==0 and not r.get('watchdog')else'failed',exit=app.returncode,capture_exit=ff.returncode,video_sha256=sha(dest/'clip.mp4'),observer_sha256=sha(dest/'observer.jsonl'),settings_after_sha256=sha(dest/'settings.ini'));r.pop('driver_pid',None);save();print(config,r['status'],flush=True)
except BaseException as e:r.update(status='failed',failure=repr(e));save();raise
finally:
 if app and app.poll()is None:app.kill();app.wait()
 if ff and ff.poll()is None:ff.communicate(b'q\n',timeout=30)
 if d:X.XCloseDisplay(d)
 server.terminate();server.wait();xlog.close()
