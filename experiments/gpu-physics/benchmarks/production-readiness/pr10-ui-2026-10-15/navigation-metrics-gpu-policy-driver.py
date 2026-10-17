from pathlib import Path
import json,hashlib,os,re,signal,stat,struct,subprocess,sys,time
R=Path.cwd();D=Path(__file__).resolve().parent;phase,label=sys.argv[1:3];assert phase in ['before','after'];build=json.loads((D/'render-popup-build.json').read_text());v=next(x for x in build['viewers']if x['label']==label);base=next(x for x in json.loads((D/'shared-protocol.json').read_text())['viewers_CPU_first']if x['label']==label);tag=sys.argv[3]if len(sys.argv)>3 else label;assert re.fullmatch('[a-z0-9-]+',tag);dest=D/'raw'/'navigation-metrics-runs'/tag;dest.mkdir(parents=True,exist_ok=False)
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
 assert time.monotonic()-start<120,'120s navigationmetrics budget'
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
 key('p');until(lambda:state()['pause']==1,3);r['findings']=[]
 def has_window(name):return any(x['name']==name for x in ui().get('windows',[]))
 def recorded_check(name,predicate,context):
  r['checks'][name]=bool(predicate)
  if not predicate:r['findings'].append({'check':name,'context':context})
  save();emit(('PASS 'if predicate else 'FINDING ')+name)
 def fresh_widget(kind,field):return until(lambda:(q if (q:=ui())and any(x['kind']==kind and x['label']==field for x in q['items'])else None),25)
 def reachable(kind,field):
  for attempt in range(8):
   q=frames();rows=[x for x in q['items']if x['kind']==kind and x['label']==field];assert rows,(kind,field);n=rows[-1]
   if n['visible']and n['rect'][1]>=n['clip'][1]and n['rect'][3]<=n['clip'][3]:return n
   hover((n['clip'][0]+n['clip'][2])/2,(n['clip'][1]+n['clip'][3])/2);direction=5 if(n['rect'][1]+n['rect'][3])/2>(n['clip'][1]+n['clip'][3])/2 else 4;action('scroll metrics to '+field,lambda:xdo('click','--repeat',2,'--delay',40,direction),n)
  recorded_check('Metrics '+field+' reachable',False,{'last_target':n,'actual_windows':q['windows'],'scroll_groups':8});return None
 def arrow(field):
  n=reachable('Tree',field)
  if n is None:return False
  hover(n['rect'][0]+5,(n['rect'][1]+n['rect'][3])/2)
  def send():xdo('mousedown',1);held.add('mouse1');time.sleep(.04);xdo('mouseup',1);held.remove('mouse1')
  action('tree arrow '+field,send,n);return True
 key('m');until(lambda:state()['show_metrics']==1,3);tap('Tab','Profile');check('Profile tab selected',find('Tab','Profile')['value']=='true');screenshot('profile')
 q=frames();check('GPU profile declares unavailable CPU substages',any(x['label']=='CPU substage timings are unavailable for GPU physics.'for x in q['items']));check('GPU profile omits CPU plots/reset/tree controls',not any((x['kind']=='Checkbox'and x['label']=='Show plots')or(x['kind']=='Button'and x['label']=='Reset')or x['kind']=='Tree'for x in q['items']if x['window']!='Info'))

 tap('Tab','Renderer');check('Renderer tab selected',find('Tab','Renderer')['value']=='true');q=frames();text=[x['label']for x in q['items']if x['kind'].startswith('Text')and x['window']=='Metrics'];r['renderer_text']=text;check('Renderer actual draw counters displayed',any(x.startswith('opaque ')for x in text)and any('draw calls'in x for x in text))
 tap('Tab','Frame Time');check('Frame Time tab selected',find('Tab','Frame Time')['value']=='true');screenshot('frame-time');tap('Tab','Profile');old_step=state()['sample_step'];key('period');until(lambda:state()['sample_step']==old_step+1,3);open_menu('Sim');tap('MenuItem','Reset Profile');until(lambda:state()['sample_step']==0,3);check('menu Reset Profile resets step/history',state()['sample_step']==0);key('m');until(lambda:state()['show_metrics']==0,3)
 open_menu('Help');tap('MenuItem','Controls');until(lambda:has_window('Controls'),3);check('Help Controls visible',has_window('Controls'));screenshot('controls');key('Escape');until(lambda:not has_window('Controls'),3);check('Escape closes Controls',not has_window('Controls'));open_menu('Help');tap('MenuItem','About');until(lambda:has_window('About'),3);q=frames();check('About version visible',any(x['window']=='About'and x['label'].startswith('Box3D ')for x in q['items']));screenshot('about');open_menu('Help');tap('MenuItem','About');until(lambda:not has_window('About'),3);check('About closes',not has_window('About'))
 n=item('Header','Solver');click('Header','Solver');q=frames();check('Solver collapse hides controls',not any(x['label']=='Hertz##Solver'for x in q['items']));click('Header','Solver');q=frames();check('Solver reopen restores controls',any(x['label']=='Hertz##Solver'for x in q['items']))
 q=frames();hover(q['width']*.25,q['height']*.4);action('focus world outside GUI',lambda:xdo('click',1));old=state()['world'];key('r');q=frames()
 if any(w['name']=='Preparing simulation'for w in q.get('windows',[])):until(lambda:state()['world']!=old,25)
 else:
  end=time.monotonic()+.75
  while time.monotonic()<end and state()['world']==old:guard();time.sleep(.025)
 after=state();recorded_check('R restart from world focus',after['world']!=old,{'before_world':old,'after_state':after,'windows':q.get('windows')})
 q=frames();info=next(w for w in q['windows']if w['name']=='Info');hover(info['rect'][0]+20,info['rect'][1]+20);action('focus Info text',lambda:xdo('click',1));old=state()['world'];key('r');q=frames()
 if any(w['name']=='Preparing simulation'for w in q.get('windows',[])):until(lambda:state()['world']!=old,25)
 else:
  end=time.monotonic()+.75
  while time.monotonic()<end and state()['world']==old:guard();time.sleep(.025)
 recorded_check('R restart with Info focus',state()['world']!=old,{'before_world':old,'after_state':state(),'actual_windows':q['windows']});until(lambda:has_window('Info'),25);check('restart keeps defaults',state()['hertz']==60 and state()['substeps']==4 and state()['context_flags']==[1,1,1]);screenshot('restored')
 open_menu('View');n=find('MenuItem','Hide UI');hover((n['rect'][0]+n['rect'][2])/2,(n['rect'][1]+n['rect'][3])/2);oldframe=state()['frame'];xdo('click',1);until(lambda:state()['frame']>oldframe+4,3);q=ui();recorded_check('Hide UI hides native windows',not any(w['name']in ['Info','##MainMenuBar']for w in q['windows']),{'actual_windows':q['windows'],'actual_world_state':state()})
 p=dest/'hidden.png';small(['ffmpeg','-hide_banner','-loglevel','error','-nostdin','-threads','1','-f','x11grab','-draw_mouse','0','-window_id',str(wid),'-i',env['DISPLAY'],'-frames:v','1','-threads','1',str(p)],5);r['screenshots']['hidden']={'path':str(p),'sha256':sha(p),'state':state(),'dimensions':list(struct.unpack('>II',p.read_bytes()[16:24]))};(dest/'hidden.ui.json').write_text(json.dumps(q,indent=2)+'\n');save();key('Tab');until(lambda:has_window('Info'),3);check('Tab restores UI',has_window('Info'))
 focus();xdo('key','--clearmodifiers','ctrl+q');app.wait(timeout=5);check('clean quit',app.returncode==0);r['status']='REVIEWED_SHARED_NAVIGATION_METRICS_WITH_FINDINGS'if r['findings']else 'PASS_SHARED_NAVIGATION_METRICS';save();emit('Sharednavigation/metricsreviewclosed, findings='+str(len(r['findings'])))

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
