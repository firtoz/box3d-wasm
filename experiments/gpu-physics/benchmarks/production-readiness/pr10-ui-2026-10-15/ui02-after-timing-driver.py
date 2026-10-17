from pathlib import Path
import json,hashlib,os,re,signal,stat,struct,subprocess,sys,time
R=Path.cwd();D=Path(__file__).resolve().parent;phase,label=sys.argv[1:3];assert phase in ['before','after'];build=json.loads((D/('ui02-build-'+phase+'.json')).read_text());v=next(x for x in build['viewers']if x['label']==label);base=next(x for x in json.loads((D/'shared-protocol.json').read_text())['viewers_CPU_first']if x['label']==label);tag=sys.argv[3]if len(sys.argv)>3 else label;assert re.fullmatch('[a-z0-9-]+',tag);dest=D/'raw'/('ui02-'+phase+'-runs')/tag;dest.mkdir(parents=True,exist_ok=False)
start=time.monotonic();app=None;wid=None;held=set();request=0;r={'status':'STARTING','phase':phase,'label':label,'driver_pid':os.getpid(),'actions':[],'screenshots':{},'checks':{},'UI01_clip_observations':[]}
def sha(p):
 with Path(p).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def save():
 p=dest/'receipt.tmp';p.write_text(json.dumps(r,indent=2)+'\n');os.replace(p,dest/'receipt.json')
def emit(s):print(label,s,flush=True)
def metadata_tree(root):
 rows=[]
 if root.exists():
  for p in root.rglob('*'):
   try:s=p.stat()
   except FileNotFoundError:continue  # Atomic publication removed this temporary name.
   if stat.S_ISREG(s.st_mode):rows.append(s.st_size)
 return {'files':len(rows),'logical_bytes':sum(rows)}
cache_roots=[Path.home()/'.cache/box3d-gpu-physics/pipelines',Path.home()/'.cache/nvidia',Path.home()/'.nv/ComputeCache']
r['shared_cache_before']={str(p):metadata_tree(p)for p in cache_roots}
env={k:x for k,x in os.environ.items()if not k.startswith(('GPU_','WGPU_','VK_','__NV','__GLX','LIBGL'))};env.update(base['env']);env.pop('GPU_PHYSICS_TRACE_PIPELINES',None);env['GPU_VIEWER_CONTROL_OBSERVER']=str(dest/'observer.jsonl');env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');env['GPU_PR10_INPUT_TRACE']=str(dest/'inputs.jsonl');
if base['cache_directory']:env['GPU_PHYSICS_PIPELINE_CACHE_DIR']=base['cache_directory']
assert sha(v['binary'])==v['sha256'];(dest/'settings.ini').write_text('{}\n');(dest/'data').symlink_to(R/'box3d/data',target_is_directory=True);r.update(binary_sha256=v['sha256'],command=[v['binary'],'--sample-name','Stacking/Single Box'],environment={k:x for k,x in env.items()if k.startswith(('GPU_','WGPU_','VK_','DISPLAY'))});save()
def state():
 p=dest/'observer.jsonl'
 if not p.exists():return None
 with p.open('rb')as f:f.seek(max(0,p.stat().st_size-16384));lines=f.read().splitlines()
 for line in reversed(lines):
  try:return json.loads(line)
  except json.JSONDecodeError:pass
 return None
def small(cmd,cap=3):return subprocess.run(cmd,env=env,capture_output=True,text=True,check=True,timeout=cap)
def xdo(*args):return small(['xdotool',*map(str,args)])
def guard():
 assert app.poll()is None,('Viewer exited',app.returncode)
 assert time.monotonic()-start<60,'60s UI02 budget'
 p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'
 assert metadata_tree(D/'raw')['logical_bytes']<1024**3,'1GiB raw cap'
def until(pred,seconds):
 stop=time.monotonic()+seconds
 while time.monotonic()<stop:
  guard();x=pred()
  if x:return x
  time.sleep(.025)
 raise AssertionError('Transition did not satisfy predicate within '+str(seconds)+'s')
def focus():
 guard();xdo('windowraise',wid);xdo('windowfocus','--sync',wid);assert xdo('getwindowfocus').stdout.strip()==str(wid)
def ui():
 global request
 request+=1;p=dest/'ui.json.request';temp=dest/'ui.request.tmp';temp.write_text(str(request));os.replace(temp,p)
 def read():
  q=dest/'ui.json'
  if not q.exists():return None
  j=json.loads(q.read_text());return j if j['request']==request else None
 return until(read,3)
def frames():
 a=ui();b=until(lambda:(q if(q:=ui())['frame']>a['frame']else None),3);return b

def action(kind,fn,target=None):
 assert len(r['actions'])<40;before=state();t=time.monotonic();fn();frames();after=state();r['actions'].append({'kind':kind,'target':target,'state_before':before,'state_after':after,'seconds':time.monotonic()-t});save();return after

def key(k):
 focus()
 def run():
  xdo('keydown',k);held.add(k);time.sleep(.04);xdo('keyup',k);held.remove(k)
 return action('key '+k,run)

def hover(x,y):
 focus();xdo('mousemove','--window',wid,round(x),round(y));frames()

def item(kind,label):
 for attempt in range(18):
  q=frames();rows=[x for x in q['items']if x['kind']==kind and x['label']==label and x['window']=='Info'];assert rows,(kind,label,'not submitted',q['frame']);n=rows[-1]
  if n['visible']and n['rect'][1]>=n['clip'][1]and n['rect'][3]<=n['clip'][3]:return n
  clip=n['clip'];hover((clip[0]+clip[2])/2,(clip[1]+clip[3])/2);button=5 if(n['rect'][1]+n['rect'][3])/2>(clip[1]+clip[3])/2 else 4
  action('scroll Info',lambda:xdo('click','--repeat',3,'--delay',45,button),{'label':label,'frame':q['frame'],'item':n})
 raise AssertionError('Item not reachable by bounded panel scroll: '+label)

def click(kind,label,window='Info'):
 if window=='Info':n=item(kind,label)
 else:
  q=frames();n=next(x for x in q['items']if x['kind']==kind and x['label']==label and x['window']==window and x['visible'])
 assert not n['disabled'];x=(n['rect'][0]+n['rect'][2])/2;y=(n['rect'][1]+n['rect'][3])/2;hover(x,y)
 def send():
  xdo('mousedown',1);held.add('mouse1');time.sleep(.04);xdo('mouseup',1);held.remove('mouse1')
 return action('click '+kind+' '+label,send,n)

def check(name,predicate):
 assert predicate,name;r['checks'][name]=True;save();emit('PASS '+name)

def screenshot(name):
 assert len(r['screenshots'])<3;p=dest/(name+'.png');assert not p.exists();q=frames();small(['ffmpeg','-hide_banner','-loglevel','error','-nostdin','-threads','1','-f','x11grab','-draw_mouse','0','-window_id',str(wid),'-i',env['DISPLAY'],'-frames:v','1','-threads','1',str(p)],5)
 (dest/(name+'.ui.json')).write_text(json.dumps(q,indent=2)+'\n');r['screenshots'][name]={'path':str(p),'sha256':sha(p),'state':state(),'dimensions':list(struct.unpack('>II',p.read_bytes()[16:24]))};text=[i for i in q['items']if i['window']=='Info'and i['kind'].startswith('Text')];clipped=[i for i in text if i['rect'][2]>i['clip'][2]+.5];r['UI01_clip_observations'].append({'name':name,'text_rows':len(text),'clipped_rows':clipped,'width':q['width'],'height':q['height']});save()

def cancel(signum,frame):raise SystemExit('Owned auto UI canceled')
signal.signal(signal.SIGTERM,cancel);signal.signal(signal.SIGINT,cancel)
try:
 stdout=(dest/'stdout.txt').open('w');stderr=(dest/'stderr.txt').open('w');app=subprocess.Popen(r['command'],cwd=dest,env=env,stdout=stdout,stderr=stderr,start_new_session=True);r['app_pid']=app.pid;save()
 def own_window():
  m=re.search(r'PR10WINDOW,pid=(\d+),id=(\d+),width=(\d+),height=(\d+)',(dest/'stderr.txt').read_text())
  if m:assert int(m[1])==app.pid;return int(m[2])
 wid=until(own_window,15);r['window_id']=wid;r['window_visible_seconds']=time.monotonic()-start;focus();until(state,25);q=frames();r['physics_ready_seconds']=time.monotonic()-start;emit('READY '+str(round(r['physics_ready_seconds'],3)));save()
 key('p');until(lambda:(s if(s:=state())['pause']==1 else None),3)
 import ctypes as c
 X=c.CDLL('libX11.so.6');T=c.CDLL('libXtst.so.6');ptr=c.c_void_p;ulong=c.c_ulong
 X.XOpenDisplay.argtypes=[c.c_char_p];X.XOpenDisplay.restype=ptr;X.XDefaultRootWindow.argtypes=[ptr];X.XDefaultRootWindow.restype=ulong;X.XTranslateCoordinates.argtypes=[ptr,ulong,ulong,c.c_int,c.c_int,c.POINTER(c.c_int),c.POINTER(c.c_int),c.POINTER(ulong)];X.XTranslateCoordinates.restype=c.c_int;X.XFlush.argtypes=[ptr];X.XCloseDisplay.argtypes=[ptr]
 T.XTestFakeMotionEvent.argtypes=[ptr,c.c_int,c.c_int,c.c_int,ulong];T.XTestFakeMotionEvent.restype=c.c_int;T.XTestFakeButtonEvent.argtypes=[ptr,c.c_uint,c.c_int,ulong];T.XTestFakeButtonEvent.restype=c.c_int
 d=X.XOpenDisplay(env['DISPLAY'].encode());assert d
 def radius(q):
  line=next(x['label']for x in q['items']if x['window']=='Info'and x['label'].startswith('radius m '));return float(re.search(r'radius m ([0-9.e+-]+)',line)[1])
 def events():
  p=dest/'inputs.jsonl';return [json.loads(l)for l in p.read_text().splitlines()]if p.exists()else []
 def burst(x,y):
  focus();root=X.XDefaultRootWindow(d);rx=c.c_int();ry=c.c_int();child=ulong();assert X.XTranslateCoordinates(d,wid,root,round(x),round(y),c.byref(rx),c.byref(ry),c.byref(child));assert T.XTestFakeMotionEvent(d,-1,rx.value,ry.value,0);assert T.XTestFakeButtonEvent(d,5,1,0);assert T.XTestFakeButtonEvent(d,5,0,0);X.XFlush(d)
 try:
  q=frames();world=[q['width']*.25,q['height']*.5];hover(*world);q=frames();assert not q['want_capture_mouse'];before=radius(q);screenshot('before');clip=next(x['clip']for x in q['items']if x['window']=='Info');panel=[(clip[0]+clip[2])/2,(clip[1]+clip[3])/2];index=len(events());action('queued enter panel plus wheel',lambda:burst(*panel),{'point':panel,'last_want_capture_mouse':q['want_capture_mouse']});q=frames();after=radius(q);rows=events()[index:];wheel=next(x for x in rows if abs(x['scroll'][1])>0);r['enter_case']={'radius_before':before,'radius_after':after,'actual_handler_scroll':wheel,'paired_events':rows,'camera_changed':abs(after-before)>.05};check('actual enter point inside Info',wheel['inside_info']);check('original enter capture observed consistently',wheel['captured']==wheel['wanted_after']);check('panel first wheel leaves camera unchanged',abs(after-before)<.05);screenshot('enter')
  assert q['want_capture_mouse'];before=radius(q);index=len(events());action('queued leave panel plus wheel',lambda:burst(*world),{'point':world,'last_want_capture_mouse':q['want_capture_mouse']});q=frames();after=radius(q);rows=events()[index:];wheel=next(x for x in rows if abs(x['scroll'][1])>0);r['leave_case']={'radius_before':before,'radius_after':after,'actual_handler_scroll':wheel,'paired_events':rows,'camera_changed':abs(after-before)>.05};check('actual leave point outside Info',not wheel['inside_info']);check('original leave capture observed consistently',wheel['captured']==wheel['wanted_after']);check('world first wheel reaches camera',abs(after-before)>.05);screenshot('leave')

  # An existing world-owned button gesture retains the original capture route.
  hover(*world);q=frames();before=radius(q);index=len(events())
  action('world-owned left down',lambda:(xdo('mousedown',1),held.add('mouse1')))
  action('held world gesture wheel over panel',lambda:burst(*panel));q=frames();after=radius(q);rows=events()[index:];wheel=next(x for x in rows if abs(x['scroll'][1])>0)
  r['held_case']={'radius_before':before,'radius_after':after,'actual_handler_scroll':wheel}
  check('held world button modifier preserved',bool(wheel['modifiers']&256));check('held world capture route preserved',not wheel['captured']and abs(after-before)>.05)
  action('release world-owned left',lambda:(xdo('mouseup',1),held.remove('mouse1')))
  hover(*world);q=frames();before=radius(q);key('ctrl+o');q=frames();index=len(events());action('popup outside wheel',lambda:burst(*world));q=frames();after=radius(q);rows=events()[index:];wheel=next(x for x in rows if abs(x['scroll'][1])>0)
  r['popup_case']={'radius_before':before,'radius_after':after,'actual_handler_scroll':wheel,'ui_windows':sorted(set(x['window']for x in q['items']))}
  check('popup owns outside wheel',wheel['captured']and abs(after-before)<.05);key('Escape');q=frames()
  key('period');check('keyboard step after popup preserved',state()['sample_step']==r['actions'][0]['state_after']['sample_step']+1)
 finally:X.XCloseDisplay(d)
 focus();xdo('key','--clearmodifiers','ctrl+q');app.wait(timeout=5);check('clean quit',app.returncode==0);r['status']='UI02_REPAIR_VERIFIED_ACTUAL_INPUT';save();emit('UI02 both directions repaired; held gesture/popup/keyboard checked')

except BaseException as e:r.update(status='FAILURE_RETAINED',error=repr(e));save();emit('FAIL '+repr(e));raise
finally:
 for k in held:
  try:xdo('mouseup',1)if k=='mouse1'else xdo('keyup',k)
  except Exception:pass
 if app and app.poll()is None:
  os.killpg(app.pid,signal.SIGTERM)
  try:app.wait(timeout=3)
  except subprocess.TimeoutExpired:os.killpg(app.pid,signal.SIGKILL);app.wait(timeout=3)
 r.update(app_terminal_exit=app.returncode if app else None,elapsed_seconds=time.monotonic()-start,shared_cache_after={str(p):metadata_tree(p)for p in cache_roots},free_bytes_after=os.statvfs(R).f_bavail*os.statvfs(R).f_frsize);save();emit('TERMINAL '+r['status'])
