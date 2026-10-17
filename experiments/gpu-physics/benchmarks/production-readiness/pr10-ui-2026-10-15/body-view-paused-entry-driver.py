from pathlib import Path
import json,hashlib,os,re,signal,stat,struct,subprocess,sys,time
R=Path.cwd();D=Path(__file__).resolve().parent;phase,label=sys.argv[1:3];assert phase in ['before','after'];build=json.loads((D/'body-metadata-build.json').read_text());v=next(x for x in build['viewers']if x['label']==label);base=next(x for x in json.loads((D/'shared-protocol.json').read_text())['viewers_CPU_first']if x['label']==label);tag=sys.argv[3]if len(sys.argv)>3 else label;assert re.fullmatch('[a-z0-9-]+',tag);dest=D/'raw'/'body-view-controls-runs'/tag;dest.mkdir(parents=True,exist_ok=False)
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
 assert time.monotonic()-start<180,'180s bodyview budget'
 p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'
 p=dest/'api.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'API cap'
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
 assert len(r['actions'])<100;before=state();t=time.monotonic();fn();frames();after=state();r['actions'].append({'kind':kind,'target':target,'state_before':before,'state_after':after,'seconds':time.monotonic()-t});save();return after

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
 key('p');until(lambda:state()['pause']==1,3);key('ctrl+o');action('initial picker Body Type',lambda:xdo('type','--clearmodifiers','--','Body Type'));n=find('Selectable','Bodies  >  Body Type');hover((n['rect'][0]+n['rect'][2])/2,(n['rect'][1]+n['rect'][3])/2);old=state()['world'];action('initial select Body Type paused',lambda:xdo('click',1),n);until(lambda:state()['world']!=old and state()['body_count']==8,25);until(lambda:any(w['name']=='Info'for w in ui()['windows']),25);check('BodyType initial8bodies/shapes',state()['body_count']==8 and state()['shape_count']==8);r['findings']=[];r['type_cases']=[];r['enable_cases']=[];r['view_proofs']=[];initial_world=state()['world']
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
 initial=metadata();r['initial_metadata']=initial;screenshot('initial');open_menu('View');open_menu('Transparency');tap('MenuItem','Dynamic');m=metadata();r['view_proofs'].append({'control':'Dynamic transparency on','metadata':m});recorded('dynamic transparency actual7instances',m['render_counts']['transparent']==7,{'metadata':m});screenshot('dynamic-transparent');open_menu('View');open_menu('Transparency');tap('MenuItem','Dynamic');m=metadata();recorded('dynamic transparency restores0',m['render_counts']['transparent']==0,{'metadata':m})
 change_type('Static',0);screenshot('static');change_type('Kinematic',1);open_menu('View');open_menu('Transparency');tap('MenuItem','Kinematic');open_menu('View');open_menu('Transparency');n=find('MenuItem','Kinematic');check('Kinematic transparency checked',n['value']=='true');r['kinematic_menu_item']=n;m=metadata();r['view_proofs'].append({'control':'Kinematic transparency on','metadata':m});recorded('kinematic transparency actual5instances',m['render_counts']['transparent']==5,{'metadata':m});screenshot('kinematic-transparent');tap('Menu','View');open_menu('View');open_menu('Transparency');tap('MenuItem','Kinematic');m=metadata();recorded('kinematic transparency restores0',m['render_counts']['transparent']==0,{'metadata':m});before=metadata();step_once();step_once();after=metadata();r['kinematic_motion']={'before':before,'after':after,'expected_dx':-.1}
 for owner in ['public']+(['mapped_cpu']if 'both'in label else []):
  b=body(before,'platform',owner);a=body(after,'platform',owner);recorded(owner+' commanded kinematic translation',abs(a['position'][0]-b['position'][0]+.1)<1e-3,{'before':b,'after':a,'steps':2,'dt':1/60,'vx':-3})
 change_type('Dynamic',2);screenshot('dynamic')
 for enabled in [False,True]:
  index=len(all_api());click('Checkbox','Enable');check('Enable display '+str(enabled),item('Checkbox','Enable')['value']==('true'if enabled else 'false'));names=['b3Body_Enable','cpu_b3Body_Enable']if enabled else ['b3Body_Disable','cpu_b3Body_Disable'];rows=[x for x in all_api()[index:]if x['api']in names];after=[x for x in rows if x['phase']=='after'];expected=6 if 'both'in label else 3;recorded('Enable actual '+str(enabled),len(after)==expected and all(x['body'].get('enabled')==enabled for x in after),{'rows':rows});r['enable_cases'].append({'enabled':enabled,'calls':rows,'metadata':metadata()})
  if not enabled:screenshot('disabled')
 recorded('body widgets keep world/counts',state()['world']==initial_world and state()['body_count']==8 and state()['shape_count']==8,{'state':state()})
 def radius():return float(re.search(r'radius m ([0-9.e+-]+)',next(x['label']for x in frames()['items']if x['label'].startswith('radius m ')))[1])
 before=metadata();recorded('framing has no body selection',not before['selected']['valid']and not before['selected_cpu']['valid'],{'metadata':before});open_menu('View');tap('MenuItem','Frame Camera');menu_radius=radius();key('f');key_radius=radius();r['camera_framing']={'menu_radius':menu_radius,'keyboard_radius':key_radius,'width':frames()['width'],'height':frames()['height'],'selected_metadata':before};recorded('menu Frame Camera matches F',abs(menu_radius-key_radius)<.2,r['camera_framing'])
 open_menu('View');open_menu('Scale');joint=float(find('InputFloat','Joint')['value']);force=float(find('InputFloat','Force')['value']);setting('InputFloat','Joint',joint*2);q=until(lambda:(q if(q:=draw())and abs(q['jointScale']-joint*2)<1e-5 else None),3);r['view_proofs'].append({'control':'Joint Scale','actual_draw':q,'metadata':metadata()});setting('InputFloat','Joint',joint);setting('InputFloat','Force',force*2);q=until(lambda:(q if(q:=draw())and abs(q['forceScale']-force*2)<1e-6 else None),3);r['view_proofs'].append({'control':'Force Scale','actual_draw':q,'metadata':metadata()});setting('InputFloat','Force',force);open_menu('View');tap('Menu','View')
 def select_query(query,count,expected):
  old=state()['world'];key('ctrl+o');action('picker '+query,lambda:xdo('type','--clearmodifiers','--',query));q=frames();rows=[x for x in q['items']if x['kind']=='Selectable'and x['label']==expected];check('picker exact '+expected,len(rows)==1);n=rows[0];hover((n['rect'][0]+n['rect'][2])/2,(n['rect'][1]+n['rect'][3])/2)
  action('picker select '+expected,lambda:xdo('click',1),n);until(lambda:state()['world']!=old and state()['body_count']==count,25);until(lambda:any(w['name']=='Info'for w in ui()['windows']),25);return old
 select_query('Hulls',1,'Compound  >  Hulls');baseline=metadata();open_menu('View');distance=float(find('SliderFloat','Draw Distance')['value']);setting('SliderFloat','Draw Distance',10);tap('Menu','View');culled=metadata();r['draw_distance_case']={'initial_distance':distance,'baseline':baseline,'culled':culled,'scene_state':state()};recorded('Draw Distance culls compound child instances',culled['render_counts']['opaque']<baseline['render_counts']['opaque'],r['draw_distance_case']);screenshot('culled');open_menu('View');setting('SliderFloat','Draw Distance',distance);tap('Menu','View');restored=metadata();r['draw_distance_case']['restored']=restored;recorded('Draw Distance restores compound instances',restored['render_counts']['opaque']==baseline['render_counts']['opaque'],r['draw_distance_case'])
 select_query('Body Type',8,'Bodies  >  Body Type');q=frames();check('BodyType defaults restored new scene',item('RadioButton','Dynamic')['value']=='true'and item('Checkbox','Enable')['value']=='true'and state()['hertz']==60 and state()['substeps']==4);screenshot('restored');m=metadata();r['restored_metadata']=m;recorded('old watched identities invalid in intervening world',all(not x['valid']for x in baseline['watched']),{'metadata':baseline,'note':'Final new BodyType CPU IDs may reuse world generation; not a permanent invalidation assertion or engine repair.'})
 focus();xdo('key','--clearmodifiers','ctrl+q');app.wait(timeout=5);check('clean quit',app.returncode==0);r['status']='REVIEWED_BODY_VIEW_WITH_FINDINGS'if r['findings']else 'PASS_BODY_VIEW_CONTROLS';save();emit('BodyType/View review closed findings='+str(len(r['findings'])))

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
