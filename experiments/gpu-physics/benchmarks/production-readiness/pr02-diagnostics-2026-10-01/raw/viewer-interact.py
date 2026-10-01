from pathlib import Path
import os,sys,subprocess,time,json,ctypes,hashlib
root=Path.cwd();out=root/'artifacts/production-readiness/pr02-diagnostics';config=sys.argv[1];dest=out/'viewer-interactions'/config;dest.mkdir(parents=True,exist_ok=False)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
if config=='cpu':binary=root/'artifacts/production-readiness/pr01-speculative/cpu-cmake/bin/samples_cpu';expected=json.loads((root/'artifacts/production-readiness/pr01-speculative/handoff/native-scene-cpu/build.json').read_text())['binary_sha256']
else:binary=out/f'c/{config}/cmake/bin/samples_{config.split("-")[1]}';expected=json.loads((out/f'viewers/{config}/receipt.json').read_text())['executable_sha256']
assert sha(binary)==expected
display=':97';assert not Path('/tmp/.X97-lock').exists()
xlog=(dest/'xvfb.log').open('w');xvfb=root/'artifacts/production-readiness/pr01-speculative/handoff/xvfb/usr/bin/Xvfb';server=subprocess.Popen([str(xvfb),display,'-screen','0','1280x720x24','-nolisten','tcp','-ac'],stdout=xlog,stderr=subprocess.STDOUT)
env={k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_','WGPU_','GPU_RECORD_'))}
env.update(DISPLAY=display,GPU_BENCH_WIDTH='1280',GPU_BENCH_HEIGHT='720',GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(out/'pipelines'),__NV_PRIME_RENDER_OFFLOAD='0',__GLX_VENDOR_LIBRARY_NAME='mesa',LIBGL_ALWAYS_SOFTWARE='1')
if config.startswith('native'):
 raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'])
 for item in raw.split(b'\0'):
  if item.startswith((b'GPU_PHYSICS_',b'WGPU_')):k,v=item.decode().split('=',1);env[k]=v
app=ff=None;X=ctypes.CDLL('libX11.so.6');T=ctypes.CDLL('libXtst.so.6');X.XOpenDisplay.argtypes=[ctypes.c_char_p];X.XOpenDisplay.restype=ctypes.c_void_p;X.XKeysymToKeycode.argtypes=[ctypes.c_void_p,ctypes.c_ulong];X.XKeysymToKeycode.restype=ctypes.c_uint;X.XFlush.argtypes=[ctypes.c_void_p];X.XCloseDisplay.argtypes=[ctypes.c_void_p]
T.XTestFakeKeyEvent.argtypes=[ctypes.c_void_p,ctypes.c_uint,ctypes.c_int,ctypes.c_ulong];T.XTestFakeMotionEvent.argtypes=[ctypes.c_void_p,ctypes.c_int,ctypes.c_int,ctypes.c_int,ctypes.c_ulong];T.XTestFakeButtonEvent.argtypes=[ctypes.c_void_p,ctypes.c_uint,ctypes.c_int,ctypes.c_ulong]
def key(symbol,down):T.XTestFakeKeyEvent(d,X.XKeysymToKeycode(d,symbol),int(down),0);X.XFlush(d)
def tap(symbol):key(symbol,True);time.sleep(.08);key(symbol,False)
def click(x,y,button=1):T.XTestFakeMotionEvent(d,-1,x,y,0);T.XTestFakeButtonEvent(d,button,1,0);T.XTestFakeButtonEvent(d,button,0,0);X.XFlush(d)
receipt={'configuration':config,'purpose':'Actual UI interaction/recording, not performance or broader physics qualification','binary_sha256':expected,'actions':[],'environment':{k:v for k,v in env.items() if k.startswith(('GPU_','WGPU_','DISPLAY','VK_','__NV','__GLX','LIBGL'))},'pid':os.getpid(),'status':'starting'}
def save(): (dest/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
try:
 for _ in range(100):
  if subprocess.run(['xdpyinfo','-display',display],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL).returncode==0:break
  assert server.poll() is None;time.sleep(.1)
 d=X.XOpenDisplay(display.encode());assert d
 video=dest/'clip.mp4';ffcmd=['ffmpeg','-hide_banner','-loglevel','warning','-f','x11grab','-draw_mouse','0','-framerate','30','-video_size','1280x720','-i',display,'-c:v','libx264','-preset','veryfast','-crf','20','-pix_fmt','yuv420p','-movflags','+faststart',str(video)]
 flog=(dest/'ffmpeg.log').open('w');ff=subprocess.Popen(ffcmd,stdin=subprocess.PIPE,stdout=flog,stderr=subprocess.STDOUT,env=env)
 stdout=(dest/'stdout.log').open('w');stderr=(dest/'stderr.log').open('w');cmd=[str(binary),'--sample-name','Single Box','--frames','7200'];app=subprocess.Popen(cmd,cwd=dest,env=env,stdout=stdout,stderr=stderr);receipt.update(status='running',app_pid=app.pid,command=cmd,capture_command=ffcmd);save();print(config,'running',flush=True)
 start=time.monotonic();handled=set()
 while app.poll() is None and time.monotonic()-start<180:
  for action in sorted(dest.glob('action-*.json')):
   if action.name in handled:continue
   a=json.loads(action.read_text());assert len(handled)<16
   if a['type']=='key':tap(a['symbol'])
   elif a['type']=='click':click(a['x'],a['y'],a.get('button',1))
   elif a['type']=='quit':key(0xffe3,True);tap(ord('q'));key(0xffe3,False)
   elif a['type']=='screenshot':
    image=dest/(a['name']+'.png');assert not image.exists();subprocess.run(['ffmpeg','-v','error','-f','x11grab','-video_size','1280x720','-i',display,'-frames:v','1',str(image)],env=env,check=True)
   else:raise ValueError(a)
   time.sleep(.5)
   receipt['actions'].append(a);handled.add(action.name);save();print(action.name,'done',flush=True)
  time.sleep(.1)
 if app.poll() is None:receipt['watchdog']=True;app.terminate()
 app.wait(timeout=15);ff.communicate(b'q\n',timeout=30);receipt.update(status='complete' if app.returncode==0 and ff.returncode==0 else 'failed',exit=app.returncode,capture_exit=ff.returncode,video_sha256=sha(video));save();print(config,receipt['status'],flush=True)
finally:
 if app and app.poll() is None:app.kill();app.wait()
 if ff and ff.poll() is None:ff.communicate(b'q\n',timeout=30)
 server.terminate();server.wait();xlog.close()
