from pathlib import Path
import json,hashlib,os,re,signal,stat,struct,subprocess,sys,time,resource
R=Path.cwd();D=Path(__file__).resolve().parent;phase,label=sys.argv[1:3];assert phase in ['before','after'];build=json.loads((D/'geometry-control-build.json').read_text());v=next(x for x in build['viewers']if x['label']==label);base=next(x for x in json.loads((D/'shared-protocol.json').read_text())['viewers_CPU_first']if x['label']==label);tag=sys.argv[3]if len(sys.argv)>3 else label;assert re.fullmatch('[a-z0-9-]+',tag);dest=D/'raw'/'geometry-null-debug-runs'/tag;dest.mkdir(parents=True,exist_ok=False)
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
env={k:x for k,x in os.environ.items()if not k.startswith(('GPU_','WGPU_','VK_','__NV','__GLX','LIBGL'))};env.update(base['env']);env.pop('GPU_PHYSICS_TRACE_PIPELINES',None);env['GPU_VIEWER_CONTROL_OBSERVER']=str(dest/'observer.jsonl');env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');env['GPU_PR10_API_TRACE']=str(dest/'api.jsonl');env['GPU_PR10_GEOMETRY_TRACE']='1';
if base['cache_directory']:env['GPU_PHYSICS_PIPELINE_CACHE_DIR']=base['cache_directory']
resource.setrlimit(resource.RLIMIT_CORE,(0,0));assert sha(v['binary'])==v['sha256'];(dest/'settings.ini').write_text('{}\n');(dest/'data').symlink_to(R/'box3d/data',target_is_directory=True);r.update(binary_sha256=v['sha256'],command=[v['binary'],'--sample-name','Stacking/Single Box'],environment={k:x for k,x in env.items()if k.startswith(('GPU_','WGPU_','VK_','DISPLAY'))});save()
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
 assert time.monotonic()-start<45,'240s bodyview budget'
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
 assert len(r['actions'])<30;before=state();t=time.monotonic();fn();frames();after=state();r['actions'].append({'kind':kind,'target':target,'state_before':before,'state_after':after,'seconds':time.monotonic()-t});save();return after

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
 assert len(r['screenshots'])<1;p=dest/(name+'.png');assert not p.exists();q=frames();small(['ffmpeg','-hide_banner','-loglevel','error','-nostdin','-threads','1','-f','x11grab','-draw_mouse','0','-window_id',str(wid),'-i',env['DISPLAY'],'-frames:v','1','-threads','1',str(p)],5)
 (dest/(name+'.ui.json')).write_text(json.dumps(q,indent=2)+'\n');r['screenshots'][name]={'path':str(p),'sha256':sha(p),'state':state(),'dimensions':list(struct.unpack('>II',p.read_bytes()[16:24]))};text=[i for i in q['items']if i['window']=='Info'and i['kind'].startswith('Text')];clipped=[i for i in text if i['rect'][2]>i['clip'][2]+.5];r['UI01_clip_observations'].append({'name':name,'text_rows':len(text),'clipped_rows':clipped,'width':q['width'],'height':q['height']});save()

def cancel(signum,frame):raise SystemExit('Owned auto UI canceled')
signal.signal(signal.SIGTERM,cancel);signal.signal(signal.SIGINT,cancel)
try:
 stdout=(dest/'stdout.txt').open('w');stderr=(dest/'stderr.txt').open('w');app=subprocess.Popen(['gdb','--nx','--batch','-x',str(D/'geometry-null-gdb.cmd'),'--args',*r['command']],cwd=dest,env={**env,'DEBUGINFOD_URLS':''},stdout=stdout,stderr=stderr,start_new_session=True);r['app_pid']=app.pid;save()
 def own_window():
  m=re.search(r'PR10WINDOW,pid=(\d+),id=(\d+),width=(\d+),height=(\d+)',(dest/'stderr.txt').read_text())
  if m:
   child=int(m[1]);status=Path('/proc')/str(child)/'status';assert 'PPid:\t'+str(app.pid) in status.read_text();r['inferior_pid']=child;r['inferior_starttime']=(Path('/proc')/str(child)/'stat').read_text().rsplit(')',1)[1].split()[19];save();return int(m[2])
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



 r['geometry_cases']=[];r['scene_cases']=[]
 def geometry(expected=2):
  frames();frames();return until(lambda:(z if(z:=latest('actual_geometry_frame'))and z['hull_calls']==expected and len(z['hulls'])==expected else None),3)
 def select_geometry(name,expected=2):
  old=state()['world'];key('ctrl+o');action('picker '+name,lambda:xdo('type','--clearmodifiers','--',name));n=find('Selectable','Geometry  >  '+name);hover((n['rect'][0]+n['rect'][2])/2,(n['rect'][1]+n['rect'][3])/2);action('select Geometry/'+name,lambda:xdo('click',1),n);until(lambda:state()['world']!=old and state()['body_count']==0,90);until(lambda:any(w['name']=='Info'for w in ui()['windows']),90);q=frames();check(name+' actual scene label',any(x['label']==name for x in q['items']));check(name+' original no-body geometrytool',state()['body_count']==0 and state()['shape_count']==0);z=geometry(expected);r['scene_cases'].append({'scene':name,'initial':z,'state':state()});save();return z
 def edit(kind,field,value):
  n=item(kind,field);hover((n['rect'][0]+n['rect'][2])/2,(n['rect'][1]+n['rect'][3])/2)
  def enter():
   xdo('keydown','Control_L');held.add('Control_L');frames();xdo('mousedown',1);held.add('mouse1');time.sleep(.05);xdo('mouseup',1);held.remove('mouse1');frames();xdo('keyup','Control_L');held.remove('Control_L')
  action('activate edit '+field,enter,n);until(lambda:ui()['want_text_input'],3);key('ctrl+a');action('type '+field+'='+str(value),lambda:xdo('type','--clearmodifiers','--delay','1','--',str(value)));key('Return');n=item(kind,field);check(field+' actualvalue '+str(value),abs(float(n['value'])-value)<1e-5)
 def close_case(scene,control,before,after,restored,ok,why):
  recorded(scene+'/'+control+' actual geometry effect',ok,{'before':before,'after':after,'expectation':why});r['geometry_cases'].append({'scene':scene,'control':control,'before':before,'after':after,'restored':restored,'expectation':why});save()
 def near(a,b):return abs(a-b)<1e-5
 before=select_geometry('Box Hull');r['before_null_input']=before;screenshot('before-null-input');edit('SliderFloat3Component','h:1',0)

except BaseException as e:r.update(status='FAILURE_RETAINED',error=repr(e));save();emit('FAIL '+repr(e));raise
finally:
 for k in held:
  try:xdo('mouseup',int(k[5:]))if k.startswith('mouse')else xdo('keyup',k)
  except Exception:pass
 if app and app.poll()is None:
  os.killpg(app.pid,signal.SIGTERM)
  try:app.wait(timeout=3)
  except subprocess.TimeoutExpired:os.killpg(app.pid,signal.SIGKILL);app.wait(timeout=3)
 
 if r.get('inferior_pid'):
  proc=Path('/proc')/str(r['inferior_pid'])/'stat'
  if proc.exists() and proc.read_text().rsplit(')',1)[1].split()[19]==r['inferior_starttime']:
   try:os.kill(r['inferior_pid'],signal.SIGKILL)
   except ProcessLookupError:pass
 r['core_dumps_disabled']=True
 r.update(app_terminal_exit=app.returncode if app else None,elapsed_seconds=time.monotonic()-start,shared_cache_after={str(p):metadata_tree(p)for p in cache_roots},free_bytes_after=os.statvfs(R).f_bavail*os.statvfs(R).f_frsize);save();emit('TERMINAL '+r['status'])
