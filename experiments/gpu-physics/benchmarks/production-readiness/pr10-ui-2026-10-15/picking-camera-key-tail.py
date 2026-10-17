from pathlib import Path
import json,hashlib,os,re,signal,stat,struct,subprocess,sys,time
R=Path.cwd();D=Path(__file__).resolve().parent;phase,label=sys.argv[1:3];assert phase in ['before','after'];build=json.loads((D/'overlay-callback-build.json').read_text());v=next(x for x in build['viewers']if x['label']==label);base=next(x for x in json.loads((D/'shared-protocol.json').read_text())['viewers_CPU_first']if x['label']==label);tag=sys.argv[3]if len(sys.argv)>3 else label;assert re.fullmatch('[a-z0-9-]+',tag);dest=D/'raw'/'picking-camera-text-capture-runs'/tag;dest.mkdir(parents=True,exist_ok=False)
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
env={k:x for k,x in os.environ.items()if not k.startswith(('GPU_','WGPU_','VK_','__NV','__GLX','LIBGL'))};env.update(base['env']);env.pop('GPU_PHYSICS_TRACE_PIPELINES',None);env['GPU_VIEWER_CONTROL_OBSERVER']=str(dest/'observer.jsonl');env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');env['GPU_PR10_API_TRACE']=str(dest/'api.jsonl');
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
 assert time.monotonic()-start<240,'240s bodyview budget'
 p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'
 p=dest/'api.jsonl';assert not p.exists()or p.stat().st_size<32*1024**2,'API cap'
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
 assert len(r['actions'])<160;before=state();t=time.monotonic();fn();frames();after=state();r['actions'].append({'kind':kind,'target':target,'state_before':before,'state_after':after,'seconds':time.monotonic()-t});save();return after

def key(k):
 focus()
 def run():
  xdo('keydown',k);held.add(k);time.sleep(.04);xdo('keyup',k);held.remove(k)
 return action('key '+k,run)

def hover(x,y):
 focus();xdo('mousemove','--window',wid,round(x),round(y));frames()

def item(kind,label):
 for attempt in range(18):
  q=frames();rows=[x for x in q['items']if x['kind']==kind and x['label']==label and x['window']=='Info'];
  if not rows:
   def available():
    z=frames();rr=[x for x in z['items']if x['kind']==kind and x['label']==label and x['window']=='Info'];return (z,rr) if rr else None
   q,rows=until(available,90)
  n=rows[-1]
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
 assert len(r['screenshots'])<8;p=dest/(name+'.png');assert not p.exists();q=frames();small(['ffmpeg','-hide_banner','-loglevel','error','-nostdin','-threads','1','-f','x11grab','-draw_mouse','0','-window_id',str(wid),'-i',env['DISPLAY'],'-frames:v','1','-threads','1',str(p)],5)
 (dest/(name+'.ui.json')).write_text(json.dumps(q,indent=2)+'\n');r['screenshots'][name]={'path':str(p),'sha256':sha(p),'state':state(),'dimensions':list(struct.unpack('>II',p.read_bytes()[16:24]))};text=[i for i in q['items']if i['window']=='Info'and i['kind'].startswith('Text')];clipped=[i for i in text if i['rect'][2]>i['clip'][2]+.5];r['UI01_clip_observations'].append({'name':name,'text_rows':len(text),'clipped_rows':clipped,'width':q['width'],'height':q['height']});save()

def cancel(signum,frame):raise SystemExit('Owned auto UI canceled')
signal.signal(signal.SIGTERM,cancel);signal.signal(signal.SIGINT,cancel)
try:
 stdout=(dest/'stdout.txt').open('w');stderr=(dest/'stderr.txt').open('w');app=subprocess.Popen(r['command'],cwd=dest,env=env,stdout=stdout,stderr=stderr,start_new_session=True);r['app_pid']=app.pid;save()
 def own_window():
  m=re.search(r'PR10WINDOW,pid=(\d+),id=(\d+),width=(\d+),height=(\d+)',(dest/'stderr.txt').read_text())
  if m:assert int(m[1])==app.pid;return int(m[2])
 wid=until(own_window,15);r['window_id']=wid;r['window_visible_seconds']=time.monotonic()-start;focus();until(state,25);q=frames();r['physics_ready_seconds']=time.monotonic()-start;emit('READY '+str(round(r['physics_ready_seconds'],3)));save()
 def find(kind,field):
  q=frames();rows=[x for x in q['items']if x['kind']==kind and x['label']==field and x['visible']];assert rows,(kind,field,'not submitted',[(x['kind'],x['label'],x['window'])for x in q['items']if x['kind']in ['Menu','MenuItem']]);return rows[-1]
 def tap(kind,field):
  n=find(kind,field);assert not n['disabled'];x=(n['rect'][0]+n['rect'][2])/2;y=(n['rect'][1]+n['rect'][3])/2;hover(x,y)
  def send():xdo('mousedown',1);held.add('mouse1');time.sleep(.04);xdo('mouseup',1);held.remove('mouse1')
  return action('menu '+field,send,n)
 def open_menu(field):
  n=find('Menu',field)
  if n['value']!='true':tap('Menu',field)
  until(lambda:any(x['kind']=='Menu'and x['label']==field and x['value']=='true'for x in ui()['items']),3)
 def draw():
  p=dest/'api.jsonl'
  if not p.exists():return None
  with p.open('rb')as f:f.seek(max(0,p.stat().st_size-16384));rows=f.read().splitlines()
  for line in reversed(rows):
   try:q=json.loads(line)
   except json.JSONDecodeError:continue
   if q['api']=='b3World_Draw':return q
  return None
 def draw_value(field,value):return until(lambda:(q if(q:=draw())and q[field]==value else None),3)
 key('p');until(lambda:state()['pause']==1,3);r['findings']=[];r['type_cases']=[];r['enable_cases']=[];r['view_proofs']=[];initial_world=state()['world']
 def recorded(name,predicate,context):
  r['checks'][name]=bool(predicate)
  if not predicate:r['findings'].append({'check':name,'context':context})
  save();emit(('PASS 'if predicate else 'FINDING ')+name)
 def all_api():
  p=dest/'api.jsonl';return [json.loads(line)for line in p.read_text().splitlines()]if p.exists()else []
 def latest(name):
  p=dest/'api.jsonl'
  if not p.exists():return None
  with p.open('rb')as f:f.seek(max(0,p.stat().st_size-65536));lines=f.read().splitlines()
  for line in reversed(lines):
   try:q=json.loads(line)
   except json.JSONDecodeError:continue
   if q['api']==name:return q
  return None
 def metadata():
  q=frames();return until(lambda:(m if(m:=latest('actual_body_metadata'))and m['request']>=q['request']else None),3)
 def body(m,name,owner='public'):return next(x for x in m['watched']if x.get('name')==name and x['owner']==owner and x['valid'])
 def setting(kind,field,value):
  n=find(kind,field);hover(n['rect'][0]+12,(n['rect'][1]+n['rect'][3])/2)
  def send():
   xdo('keydown','Control_L');held.add('Control_L');xdo('mousedown',1);held.add('mouse1');time.sleep(.04);xdo('mouseup',1);held.remove('mouse1');xdo('keyup','Control_L');held.remove('Control_L');frames();xdo('key','ctrl+a');xdo('type','--clearmodifiers','--',str(value));xdo('key','Return')
  action('edit '+field+'='+str(value),send,n);n=find(kind,field);check(field+' displayed '+str(value),abs(float(n['value'])-value)<1e-5)
 def change_type(field,target):
  index=len(all_api());click('RadioButton',field);check(field+' radio selected',item('RadioButton',field)['value']=='true');rows=[x for x in all_api()[index:]if x['api']in ['b3Body_SetType','cpu_b3Body_SetType']];after=[x for x in rows if x['phase']=='after'];before=[x for x in rows if x['phase']=='before'];expected=10 if 'both'in label else 5;recorded(field+' exact setter group',len(after)==expected,{'rows':rows});recorded(field+' actual body types',len(after)==expected and all(x['body'].get('type')==target for x in after),{'rows':rows});identity=lambda x:(x['api'],x['body']['owner'],tuple(x['body']['id'])); prior={identity(x):x for x in before}; pose_ok=len(prior)==len(after) and all(identity(y) in prior and prior[identity(y)]['body']['valid'] and y['body']['valid'] and max(abs(a-b)for a,b in zip(prior[identity(y)]['body']['position'],y['body']['position']))<1e-5 for y in after);recorded(field+' preserves body origins',pose_ok,{'rows':rows});r['type_cases'].append({'control':field,'target':target,'calls':rows,'after_metadata':metadata()});save();return rows
 def step_once():
  focus();before=state();index=len(all_api());t=time.monotonic();xdo('keydown','period');held.add('period');time.sleep(.04);xdo('keyup','period');held.remove('period');until(lambda:state()['sample_step']==before['sample_step']+1,90);frames();r['actions'].append({'kind':'controlled single step','state_before':before,'state_after':state(),'seconds':time.monotonic()-t});r.setdefault('step_calls',[]).append([x for x in all_api()[index:]if x['api']in ['b3World_Step','gpu_b3_world_step','cpu_b3World_Step']]);save()


 r['pointer_cases']=[];r['camera_cases']=[];q=frames();width=q['width'];height=q['height'];pane_width=width//2 if 'both'in label else width;cx=pane_width/2;cy=height/2
 def cam():
  q=frames();text=[x['label']for x in q['items']if x['window']=='Info'];pivot=next(x for x in text if x.startswith('pivot m '));angles=next(x for x in text if x.startswith('yaw/pitch '));rad=next(x for x in text if x.startswith('radius m '));nums=lambda t:[float(x)for x in re.findall(r'-?\d+(?:\.\d+)?',t)];return {'pivot':nums(pivot),'angles':nums(angles),'radius_speed':nums(rad),'frame':q['frame']}
 def selected(m,owner):return m['selected_cpu'if owner=='mapped_cpu'else'selected']
 owners=['public']+(['mapped_cpu']if 'both'in label else [])
 def press_button(button,modifier=None):
  if modifier:xdo('keydown',modifier);held.add(modifier);frames()
  xdo('mousedown',button);held.add('mouse'+str(button));frames()
 def release_button(button,modifier=None):
  xdo('mouseup',button);held.discard('mouse'+str(button))
  if modifier:xdo('keyup',modifier);held.discard(modifier)
  frames()
 def mouse_click(x,y):hover(x,y);return action('world click',lambda:xdo('click',1),{'pixel':[x,y]})
 def restore():
  old=state()['world'];key('r');until(lambda:state()['world']!=old and state()['body_count']==2,90);until(lambda:any(w['name']=='Info'for w in ui()['windows']),90);check('restart restored2bodies2shapes '+str(len(r['pointer_cases'])),state()['body_count']==2 and state()['shape_count']==2);m=metadata();recorded('restart clears selection '+str(len(r['pointer_cases'])),all(not selected(m,o)['valid']for o in owners),{'metadata':m});return m
 # FOV50degrees and originalyaw0/pitch25/radius10 imply this source-derived
 # projected center; the actual hit/name/ID proves the coordinate, not pixels.
 import math
 ypick=(1-(.5*math.cos(math.radians(25)))/(math.tan(math.radians(25))*(10-.5*math.sin(math.radians(25)))))*height/2
 import ctypes;xl=ctypes.CDLL('libX11.so.6');xl.XOpenDisplay.argtypes=[ctypes.c_char_p];xl.XOpenDisplay.restype=ctypes.c_void_p;xl.XDefaultRootWindow.argtypes=[ctypes.c_void_p];xl.XDefaultRootWindow.restype=ctypes.c_ulong;xl.XCloseDisplay.argtypes=[ctypes.c_void_p];display_handle=xl.XOpenDisplay(None);assert display_handle;root_window=xl.XDefaultRootWindow(display_handle);xl.XCloseDisplay(display_handle)
 # Key-up while a real text-edit widget owns keyboard capture must stop fly.
 hover(cx,cy);action('RMB begin key-release case',lambda:press_button(3));xdo('keydown','w');held.add('w');n=item('SliderInt','Sub-steps##Solver');hover((n['rect'][0]+n['rect'][2])/2,(n['rect'][1]+n['rect'][3])/2);xdo('keydown','Control_L');held.add('Control_L');frames();xdo('mousedown',1);held.add('mouse1');time.sleep(.05);frames();xdo('mouseup',1);held.remove('mouse1');frames();xdo('keyup','Control_L');held.remove('Control_L');q=frames();r['keyboard_release_capture']=q;check('actual slider text edit captures keyboard',q['want_text_input']);xdo('keyup','w');held.remove('w');frames();before=cam();time.sleep(.3);after=cam();recorded('key release during UI edit stops camera',before['pivot']==after['pivot'],{'before':before,'after':after,'want_text_input':q['want_text_input']});xdo('key','ctrl+a');frames();xdo('type','--clearmodifiers','--delay','1','--','4');frames();xdo('key','Return');frames();release_button(3);xdo('windowfocus',root_window);frames();focus();check('physics defaults unchanged by camera input',state()['hertz']==60 and state()['substeps']==4 and state()['body_count']==2 and state()['shape_count']==2);screenshot('camera-final')
 focus();xdo('key','--clearmodifiers','ctrl+q');app.wait(timeout=5);check('clean quit',app.returncode==0);r['status']='REVIEWED_POINTER_CAMERA_WITH_FINDINGS'if r['findings']else'PASS_POINTER_CAMERA';save();emit('Pointer/camera review closed')

except BaseException as e:r.update(status='FAILURE_RETAINED',error=repr(e));save();emit('FAIL '+repr(e));raise
finally:
 for k in held:
  try:xdo('mouseup',int(k[5:]))if k.startswith('mouse')else xdo('keyup',k)
  except Exception:pass
 if app and app.poll()is None:
  os.killpg(app.pid,signal.SIGTERM)
  try:app.wait(timeout=3)
  except subprocess.TimeoutExpired:os.killpg(app.pid,signal.SIGKILL);app.wait(timeout=3)
 r.update(app_terminal_exit=app.returncode if app else None,elapsed_seconds=time.monotonic()-start,shared_cache_after={str(p):metadata_tree(p)for p in cache_roots},free_bytes_after=os.statvfs(R).f_bavail*os.statvfs(R).f_frsize);save();emit('TERMINAL '+r['status'])
