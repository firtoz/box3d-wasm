from pathlib import Path
import json,hashlib,os,re,signal,stat,struct,subprocess,sys,time
R=Path.cwd();D=Path(__file__).resolve().parent;phase,label=sys.argv[1:3];assert phase in ['before','after'];build=json.loads((D/'solver-controls-build.json').read_text());v=next(x for x in build['viewers']if x['label']==label);base=next(x for x in json.loads((D/'shared-protocol.json').read_text())['viewers_CPU_first']if x['label']==label);tag=sys.argv[3]if len(sys.argv)>3 else label;assert re.fullmatch('[a-z0-9-]+',tag);dest=D/'raw'/'solver-controls-runs'/tag;dest.mkdir(parents=True,exist_ok=False)
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
 assert time.monotonic()-start<60,'60s solvercontrol budget'
 p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'
 p=dest/'api.jsonl';assert not p.exists()or p.stat().st_size<8*1024**2,'API cap'
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
 assert len(r['actions'])<45;before=state();t=time.monotonic();fn();frames();after=state();r['actions'].append({'kind':kind,'target':target,'state_before':before,'state_after':after,'seconds':time.monotonic()-t});save();return after

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
 assert len(r['screenshots'])<2;p=dest/(name+'.png');assert not p.exists();q=frames();small(['ffmpeg','-hide_banner','-loglevel','error','-nostdin','-threads','1','-f','x11grab','-draw_mouse','0','-window_id',str(wid),'-i',env['DISPLAY'],'-frames:v','1','-threads','1',str(p)],5)
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
 def api():
  p=dest/'api.jsonl';return [json.loads(x)for x in p.read_text().splitlines()]if p.exists()else []
 def edit(kind,label,value):
  n=item(kind,label);x=n['rect'][0]+12;y=(n['rect'][1]+n['rect'][3])/2;hover(x,y)
  def send():
   xdo('keydown','Control_L');held.add('Control_L');xdo('mousedown',1);held.add('mouse1');time.sleep(.04);xdo('mouseup',1);held.remove('mouse1');xdo('keyup','Control_L');held.remove('Control_L');frames();xdo('key','ctrl+a');xdo('type','--clearmodifiers',str(value));xdo('key','Return')
  action('edit '+label+'='+str(value),send,n);q=frames();v=next(x for x in q['items']if x['kind']==kind and x['label']==label);check('display '+label+'='+str(value),abs(float(v['value'])-value)<1e-5);return q
 worker_label='CPU Workers##Solver'if 'both'in label else ('Workers##Solver'if label=='cpu'else None)
 initial={field:float(item(kind,field)['value'])for kind,field in [('SliderInt','Sub-steps##Solver'),('SliderFloat','Hertz##Solver'),('SliderFloat','Recycle##Solver')]+([('SliderInt',worker_label)]if worker_label else [])};r['initial_controls']=initial
 if not worker_label:
  q=frames();check('standalone GPU scheduling truthfully automatic',any(x['label']=='GPU scheduling is automatic'for x in q['items'])and not any('Workers##Solver'in x['label']for x in q['items']))
 if label!='cpu':
  q=frames();check('GPU recording unavailable explicitly',any(x['label']=='Recording unavailable with GPU physics'for x in q['items'])and not any(x['kind']=='Button'and '##Recording'in x['label']for x in q['items']))
 edit('SliderInt','Sub-steps##Solver',7);edit('SliderFloat','Hertz##Solver',120);check('context hertz/substeps updated',state()['hertz']==120 and state()['substeps']==7)
 old=state()['sample_step'];index=len(api());key('period');until(lambda:(s if(s:=state())['sample_step']==old+1 else None),3);rows=api()[index:];r['edited_step_calls']=rows
 names=['b3World_Step']+(['gpu_b3_world_step']if label!='cpu'else [])+(['cpu_b3World_Step']if 'both'in label else [])
 for name in names:check(name+' edited args',any(x['api']==name and abs(x['value']-1/120)<1e-8 and x['substeps']==7 for x in rows))
 index=len(api());edit('SliderFloat','Recycle##Solver',3.5);rows=api()[index:];r['edited_recycle_calls']=rows
 names=['b3World_SetContactRecycleDistance']+(['gpu_b3_world_set_contact_recycle_distance']if label!='cpu'else [])+(['cpu_b3World_SetContactRecycleDistance']if 'both'in label else [])
 for name in names:check(name+' centimeters→meters',any(x['api']==name and abs(x['value']-.035)<1e-7 for x in rows))
 old=state()['world']
 if worker_label:
  workers=2 if initial[worker_label]!=2 else 3;edit('SliderInt',worker_label,workers)
 else:click('Button','Restart')
 until(lambda:(s if(s:=state())['world']!=old else None),3);check('worker or restart creates new world',state()['world']!=old);check('edited settings retained across worker restart',state()['hertz']==120 and state()['substeps']==7);screenshot('edited')
 if worker_label:edit('SliderInt',worker_label,int(initial[worker_label]))
 edit('SliderFloat','Recycle##Solver',initial['Recycle##Solver']);edit('SliderFloat','Hertz##Solver',initial['Hertz##Solver']);edit('SliderInt','Sub-steps##Solver',int(initial['Sub-steps##Solver']));q=frames();r['restored_controls']={x['label']:x['value']for x in q['items']if x['label']in initial};check('all original slider defaults restored',all(abs(float(r['restored_controls'][k])-v)<1e-5 for k,v in initial.items()));screenshot('restored')
 old=state()['sample_step'];index=len(api());key('period');until(lambda:(s if(s:=state())['sample_step']==old+1 else None),3);rows=api()[index:];r['restored_step_calls']=rows
 for name in ['b3World_Step']+(['gpu_b3_world_step']if label!='cpu'else [])+(['cpu_b3World_Step']if 'both'in label else []):check(name+' default args restored',any(x['api']==name and abs(x['value']-1/initial['Hertz##Solver'])<1e-8 and x['substeps']==initial['Sub-steps##Solver']for x in rows))
 focus();xdo('key','--clearmodifiers','ctrl+q');app.wait(timeout=5);check('clean quit',app.returncode==0);r['status']='PASS_SHARED_SOLVER_SLIDERS_ACTUAL_API';save();emit('Solver sliders actual API and defaults pass')

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
