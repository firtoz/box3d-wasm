from pathlib import Path
import hashlib,json,os,re,select,signal,subprocess,sys,time
R=Path.cwd();D=Path(__file__).resolve().parent;protocol=json.loads((D/'shared-protocol.json').read_text());P=protocol['budget'];label=sys.argv[1];v=next(x for x in protocol['viewers_CPU_first']if x['label']==label);tag=sys.argv[2]if len(sys.argv)>2 else label;assert re.fullmatch('[a-z0-9-]+',tag);dest=D/'raw/shared'/tag;dest.mkdir(parents=True,exist_ok=False)
def sha(p):
 with Path(p).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def save():
 temp=dest/'receipt.tmp';temp.write_text(json.dumps(r,indent=2)+'\n');os.replace(temp,dest/'receipt.json')
def emit(x):print(json.dumps(x),flush=True)
def cache_manifest():
 c=Path(v['cache_directory']) if v['cache_directory']else None
 return {str(f.relative_to(c)):{'bytes':f.stat().st_size,'sha256':sha(f)}for f in c.rglob('*')if f.is_file()}if c and c.exists()else {}
def state():
 p=dest/'observer.jsonl'
 if not p.exists():return None
 with p.open('rb')as f:
  f.seek(max(0,p.stat().st_size-16384));lines=f.read().splitlines()
 for line in reversed(lines):
  try:return json.loads(line)
  except json.JSONDecodeError:pass
 return None
app=None;wid=None;held=set();t0=time.monotonic();r={'status':'STARTING','label':label,'driver_pid':os.getpid(),'actions':[],'screenshots':{},'cache_before':cache_manifest(),'protocol_sha256':sha(D/'shared-protocol.json')}
assert sha(v['binary'])==v['sha256']
env={k:x for k,x in os.environ.items()if not k.startswith(('GPU_','WGPU_','VK_','__NV','__GLX','LIBGL'))};env.update(v['env']);env['GPU_VIEWER_CONTROL_OBSERVER']=str(dest/'observer.jsonl')
if v['cache_directory']:env['GPU_PHYSICS_PIPELINE_CACHE_DIR']=v['cache_directory']
(dest/'settings.ini').write_text('{}\n');(dest/'data').symlink_to(R/'box3d/data',target_is_directory=True)
cmd=[v['binary'],'--sample-name','Stacking/Single Box'];r.update(command=cmd,binary_sha256=v['sha256'],environment={k:x for k,x in env.items()if k.startswith(('GPU_','WGPU_','VK_','DISPLAY'))});save()
def small(cmd,seconds=3):return subprocess.run(cmd,env=env,capture_output=True,text=True,timeout=seconds,check=True)
def xdo(*args):return small(['xdotool',*map(str,args)])
def focus():
 # Sokol does not publish _NET_WM_PID here. The marker comes from this exact
 # owned child's stderr; require its window to remain viewable and child alive.
 assert app.poll()is None and int(r['window_marker'].split('pid=')[1].split(',')[0])==app.pid
 check=json.loads(small([sys.executable,str(R/'experiments/gpu-physics/benchmarks/production-readiness/pr09-repairs-2026-10-02/PERF04/general-mesh-normal-viewers-2026-10-10/x11-viewability.py'),str(wid)],2).stdout)
 assert check['window']==wid and check['is_viewable'],check
 xdo('windowraise',wid);xdo('windowfocus','--sync',wid);assert xdo('getwindowfocus').stdout.strip()==str(wid)
def snapshot(name):
 assert re.fullmatch('[a-z0-9-]{1,50}',name)and len(r['screenshots'])<P['screenshots_per_viewer_maximum'];p=dest/(name+'.png');assert not p.exists()
 small(['ffmpeg','-hide_banner','-loglevel','error','-nostdin','-threads','1','-f','x11grab','-draw_mouse','0','-window_id',str(wid),'-i',env['DISPLAY'],'-frames:v','1','-threads','1',str(p)],5)
 r['screenshots'][name]={'path':str(p),'sha256':sha(p),'state':state()};r['latest_screenshot']=name;return r['screenshots'][name]
def interrupt(signum,frame):raise SystemExit('Owned PR10 driver canceled')
signal.signal(signal.SIGTERM,interrupt);signal.signal(signal.SIGINT,interrupt)
try:
 stdout=(dest/'stdout.txt').open('w');stderr=(dest/'stderr.txt').open('w');app=subprocess.Popen(cmd,cwd=dest,env=env,stdout=stdout,stderr=stderr,start_new_session=True);r['app_pid']=app.pid;save()
 while app.poll()is None and time.monotonic()-t0<P['first_window_seconds']:
  txt=(dest/'stderr.txt').read_text();m=re.search(r'(?:PR10WINDOW|PERF05WINDOW),pid=(\d+),id=(\d+),width=(\d+),height=(\d+)',txt)
  if m:
   assert int(m[1])==app.pid;wid=int(m[2]);r['window_marker']=m[0]
   check=json.loads(small([sys.executable,str(R/'experiments/gpu-physics/benchmarks/production-readiness/pr09-repairs-2026-10-02/PERF04/general-mesh-normal-viewers-2026-10-10/x11-viewability.py'),str(wid)],2).stdout)
   if check['is_viewable']:break
  time.sleep(.05)
 assert wid,{'window':wid,'observer':state(),'exit':app.poll()}
 focus();r.update(status='WINDOW_VISIBLE',window_id=wid,first_state=state());save();emit({'status':'WINDOW_VISIBLE','window':wid,'state':state(),'screenshot':snapshot('initial')});save()
 while app.poll()is None:
  assert time.monotonic()-t0<P['per_viewer_watchdog_seconds'],'Per-viewer300s watchdog'
  obs=dest/'observer.jsonl';assert not obs.exists() or obs.stat().st_size<=P['observer_file_per_viewer_bytes'],'Observer size cap'
  assert sum(p.stat().st_size for p in (D/'raw').rglob('*')if p.is_file())<P['batch_raw_bytes'],'Batch raw cap'
  if not select.select([sys.stdin],[],[],.1)[0]:continue
  line=sys.stdin.readline()
  if not line:break
  action=json.loads(line);assert len(r['actions'])<P['actions_per_viewer_maximum'];before=state();begin=time.monotonic();kind=action['type'];result=None
  if kind in ['click','scroll']:
   last=r['screenshots'][r['latest_screenshot']];assert action['proof']==last['sha256'],'Current reviewed screenshot required';assert before and last['state'] and before['world']==last['state']['world'],'World lifetime changed or still loading since reviewed screen'
  if kind in ['key','click','scroll','resize','quit']:focus()
  if kind=='key':
   xdo('keydown',action['key']);held.add(action['key']);time.sleep(.12);xdo('keyup',action['key']);held.remove(action['key'])
  elif kind=='click':
   xdo('mousemove','--window',wid,action['x'],action['y']);time.sleep(.12);xdo('mousedown',1);held.add('mouse1');time.sleep(.18);xdo('mouseup',1);held.remove('mouse1')
  elif kind=='scroll':
   assert 1<=abs(action['steps'])<=5;xdo('mousemove','--window',wid,action['x'],action['y']);xdo('click','--repeat',abs(action['steps']),'--delay',100,5 if action['steps']>0 else 4)
  elif kind=='snapshot':result=snapshot(action['name'])
  elif kind=='wait':assert 0<=action['seconds']<=2;time.sleep(action['seconds'])
  elif kind=='ready':
   deadline=time.monotonic()+P['loading_ready_operation_seconds'];last_progress=0
   while state()is None and app.poll()is None and time.monotonic()<deadline:
    time.sleep(.1)
    if time.monotonic()-last_progress>=10:
     last_progress=time.monotonic();emit({'status':'LOADING_WINDOW_VISIBLE','elapsed':time.monotonic()-t0,'ready':False})
   assert state(),'Physics readiness cutoff; loading window is separate from readiness'
  elif kind=='resize':
   assert 800<=action['width']<=1920 and 600<=action['height']<=1080;xdo('windowsize',wid,action['width'],action['height'])
  elif kind=='quit':xdo('key','--clearmodifiers','ctrl+q')
  else:raise ValueError(kind)
  time.sleep(.35);record={'action':action,'state_before':before,'state_after':state(),'result':result,'seconds':time.monotonic()-begin};r['actions'].append(record);save();emit(record)
  if kind=='quit':app.wait(timeout=3);break
 if app.poll()is None:raise RuntimeError('Controller stdin ended before clean quit')
 r.update(status='TERMINAL_REVIEW_PENDING',app_exit=app.returncode);assert app.returncode==0
except BaseException as e:r.update(status='FAILURE_RETAINED',error=repr(e));emit({'failure':repr(e)});raise
finally:
 for k in held:
  try:xdo('mouseup',1)if k=='mouse1'else xdo('keyup',k)
  except Exception:pass
 if app and app.poll()is None:
  os.killpg(app.pid,signal.SIGTERM)
  try:app.wait(timeout=3)
  except subprocess.TimeoutExpired:os.killpg(app.pid,signal.SIGKILL);app.wait(timeout=3)
 r.update(app_terminal_exit=app.returncode if app else None,elapsed_seconds=time.monotonic()-t0,cache_after=cache_manifest(),free_bytes_after=os.statvfs(R).f_bavail*os.statvfs(R).f_frsize);save();emit({'status':r['status'],'app_exit':r['app_terminal_exit'],'receipt':str(dest/'receipt.json')})
