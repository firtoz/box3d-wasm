from pathlib import Path
import json,hashlib,os,re,signal,stat,struct,subprocess,sys,time
R=Path.cwd();D=Path(__file__).resolve().parent;phase,label=sys.argv[1:3];assert phase in ['before','after'];build=json.loads((D/'overlay-callback-build.json').read_text());v=next(x for x in build['viewers']if x['label']==label);base=next(x for x in json.loads((D/'shared-protocol.json').read_text())['viewers_CPU_first']if x['label']==label);tag=sys.argv[3]if len(sys.argv)>3 else label;assert re.fullmatch('[a-z0-9-]+',tag);dest=D/'raw'/'overlay-controls-runs'/tag;dest.mkdir(parents=True,exist_ok=False)
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
 key('p');until(lambda:state()['pause']==1,3);key('ctrl+o');action('initial picker Body Type',lambda:xdo('type','--clearmodifiers','--','Body Type'));n=find('Selectable','Bodies  >  Body Type');hover((n['rect'][0]+n['rect'][2])/2,(n['rect'][1]+n['rect'][3])/2);old=state()['world'];action('initial select Body Type paused',lambda:xdo('click',1),n);until(lambda:state()['world']!=old and state()['body_count']==8,90);until(lambda:any(w['name']=='Info'for w in ui()['windows']),90);check('BodyType initial8bodies/shapes',state()['body_count']==8 and state()['shape_count']==8);r['findings']=[];r['type_cases']=[];r['enable_cases']=[];r['view_proofs']=[];initial_world=state()['world']
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

 r['overlay_cases']=[];r['scale_cases']=[];r['initial_state']=state();step_once();step_once();check('paused two-step fixture',state()['pause']==1 and state()['sample_step']==2);screenshot('initial')
 fields={'Joints':'joints','Joint Extras':'extras','Bounds':'bounds','Mass':'mass','Sleep':'sleep','Body Names':'names','Graph Colors':'graph','Islands':'islands','Contact Points':'contacts','Contact Normals':'normals','Contact Features':'features','Contact Forces':'forces'}
 def outputs():
  frames();return {'public':latest('actual_draw_outputs'),'gpu_inner':latest('actual_gpu_draw_outputs')if 'both'in label else None}
 def set_flag(field,value):
  open_menu('View');n=find('MenuItem',field)
  if n['value']!=('true'if value else 'false'):tap('MenuItem',field)
  else:tap('Menu','View')
  q=draw_value(fields[field],int(value));check(field+' actualflag '+str(value),q[fields[field]]==int(value));frames();return outputs()
 def assess(field,out,keys,before=None,digest=False):
  keys=[keys]if isinstance(keys,str)else keys
  for owner in ['public']+(['gpu_inner']if 'both'in label else []):
   o=out[owner];assert o is not None,(field,owner)
   if digest:
    prior=before[owner];ok=any(o[k+'_digest']!=prior[k+'_digest']for k in keys)
   else:ok=any(o[k+'_callbacks']>(before[owner][k+'_callbacks']if before else 0)for k in keys)
   recorded(field+' '+owner+' visible callback effect',ok,{'after':o,'before':before[owner]if before else None,'callback_fields':keys})
  r['overlay_cases'].append({'control':field,'outputs':out,'before':before,'expected_output':keys});save()
 for field in fields:set_flag(field,False)
 base=outputs();r['baseline_outputs']=base;check('baseline8actualshapes',base['public']['shape_callbacks']==8)
 joints=set_flag('Joints',True);assess('Joints',joints,'segment',base);screenshot('joints');extras=set_flag('Joint Extras',True);assess('Joint Extras',extras,['segment','string'],joints);set_flag('Joint Extras',False);graph_before=outputs();graph=set_flag('Graph Colors',True);assess('Graph Colors',graph,'point',graph_before);set_flag('Graph Colors',False)
 def scale_case(field,factor,callback):
  before=outputs();open_menu('View');open_menu('Scale');old=float(find('InputFloat',field)['value']);setting('InputFloat',field,old*factor);until(lambda:(q:=draw())and abs(q['jointScale'if field=='Joint'else'forceScale']-old*factor)<1e-6,3);tap('Menu','View');changed=outputs();assess(field+' Scale',changed,callback,before,True)
  open_menu('View');open_menu('Scale');setting('InputFloat',field,old);tap('Menu','View');restored=outputs()
  recorded(field+' Scale public restored exact callback inputs',restored['public'][callback+'_digest']==before['public'][callback+'_digest'],{'before':before,'restored':restored});r['scale_cases'].append({'control':field,'old':old,'changed':old*factor,'before':before,'changed_output':changed,'restored':restored});save()
 scale_case('Joint',2,'segment');set_flag('Joints',False)
 for field,callback in [('Sleep','point'),('Body Names','string'),('Islands','bounds')]:
  out=set_flag(field,True);assess(field,out,callback,base);screenshot(field.lower().replace(' ','-'));set_flag(field,False)
 contacts=set_flag('Contact Points',True);r['contact_outputs']=contacts;assess('Contact Points',contacts,'point',base);screenshot('contacts')
 for field,callbacks in [('Contact Normals',['segment','string']),('Contact Features','string'),('Contact Forces',['segment','string'])]:
  out=set_flag(field,True);assess(field,out,callbacks,contacts)
  if field=='Contact Forces':scale_case('Force',2,'segment');screenshot('forces')
  set_flag(field,False)
 for anchor,value in [('Anchor A',1),('Anchor B',0)]:
  open_menu('View');open_menu('Anchor');tap('MenuItem',anchor);draw_value('anchorA',value);out=outputs();assess(anchor,out,'point',base);r['overlay_cases'][-1]['actual_anchorA']=value;save()
 set_flag('Contact Points',False);r['final_state']=state();recorded('overlays preserve world/body/shape/physicssteps',r['final_state']['world']==r['initial_state']['world']and r['final_state']['body_count']==8 and r['final_state']['shape_count']==8 and r['final_state']['sample_step']==2,{'before':r['initial_state'],'after':r['final_state']});focus();xdo('key','--clearmodifiers','ctrl+q');app.wait(timeout=5);check('clean quit',app.returncode==0);r['status']='REVIEWED_OVERLAYS_WITH_FINDINGS'if r['findings']else 'PASS_OVERLAY_OUTPUTS';save();emit('Overlay output review closed')

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
