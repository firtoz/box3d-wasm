from pathlib import Path
import json,hashlib,os,re,signal,stat,struct,subprocess,sys,time
R=Path.cwd();D=Path(__file__).resolve().parent;phase,label=sys.argv[1:3];assert phase in ['before','after'];build=json.loads((D/'body-metadata-build.json').read_text());v=next(x for x in build['viewers']if x['label']==label);base=next(x for x in json.loads((D/'shared-protocol.json').read_text())['viewers_CPU_first']if x['label']==label);tag=sys.argv[3]if len(sys.argv)>3 else label;assert re.fullmatch('[a-z0-9-]+',tag);dest=D/'raw'/'recording-replay-corrected-runs'/tag;dest.mkdir(parents=True,exist_ok=False)
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
 assert time.monotonic()-start<120,'120s bodyview budget'
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

 key('p');until(lambda:state()['pause']==1,3);r['findings']=[];r['recording_cases']=[]
 def report(name,ok,context):
  r['checks'][name]=bool(ok)
  if not ok:r['findings'].append({'check':name,'context':context})
  save();emit(('PASS 'if ok else 'FINDING ')+name)
 def any_node(kind,field):
  q=frames();rows=[x for x in q['items']if x['kind']==kind and x['label']==field and x['visible']];assert rows,(kind,field);return rows[-1]
 def tap_node(n):
  assert not n['disabled'];hover((n['rect'][0]+n['rect'][2])/2,(n['rect'][1]+n['rect'][3])/2)
  def send():xdo('mousedown',1);held.add('mouse1');time.sleep(.04);xdo('mouseup',1);held.remove('mouse1')
  return action('click '+n['kind']+' '+n['label'],send,n)
 def set_text(field,text):
  n=item('InputText',field);hover((n['rect'][0]+n['rect'][2])/2,(n['rect'][1]+n['rect'][3])/2)
  action('focus text '+field,lambda:xdo('click',1),n);key('ctrl+a');action('replace text '+field,lambda:xdo('type','--clearmodifiers','--',text),n);key('Return');check(field+' displayed '+text,item('InputText',field)['value']==text)
 def step():
  old=state()['sample_step'];key('period');until(lambda:state()['sample_step']==old+1,3)
 def gtk_dialog():
  q=subprocess.run(['xdotool','search','--onlyvisible','--pid',str(app.pid)],env=env,capture_output=True,text=True,timeout=3)
  for ident in q.stdout.splitlines():
   if xdo('getwindowname',ident).stdout.strip()=='Open File':return int(ident)
  return None
 def open_file(path=None):
  q=frames();roots=[x for x in q['items']if x['kind']=='Menu'and x['label']=='Replay'and x['window']=='##MainMenuBar'];assert len(roots)==1;tap_node(roots[0]);n=any_node('MenuItem','Open...');hover((n['rect'][0]+n['rect'][2])/2,(n['rect'][1]+n['rect'][3])/2);xdo('click',1)
  dialog=until(gtk_dialog,8);r.setdefault('file_dialogs',[]).append({'window_id':dialog,'owned_pid':app.pid,'action':'cancel'if path is None else str(path)});save();xdo('windowraise',dialog);xdo('windowfocus','--sync',dialog);assert xdo('getwindowfocus').stdout.strip()==str(dialog)
  if path is None:xdo('key','Escape')
  else:xdo('key','ctrl+l');time.sleep(.08);xdo('type','--clearmodifiers','--',str(path));xdo('key','Return')
  until(lambda:not gtk_dialog(),8);focus();frames()
 if label=='cpu':
  for name,button,restart in [('pr10-now.b3rec','Record Now##Recording',False),('pr10-restart.b3rec','Record (restart)##Recording',True)]:
   set_text('File##Recording',name);old=state();click('Button',button);until(lambda:any(x['kind']=='Button'and x['label']=='Stop##Recording'for x in ui()['items']),3);now=state();report(button+' world lifetime',now['world']!=old['world']if restart else now['world']==old['world'],{'before':old,'after':now});report(button+' seed step',now['sample_step']==0 if restart else now['sample_step']==old['sample_step'],{'before':old,'after':now});screenshot('recording-restart'if restart else 'recording-now')
   for _ in range(10):step()
   click('Button','Stop##Recording');p=dest/name;until(p.exists,3);check(name+' nonempty file',0<p.stat().st_size<16*1024**2);r['recording_cases'].append({'button':button,'path':str(p),'sha256':sha(p),'bytes':p.stat().st_size,'head_hex':p.read_bytes()[:32].hex(),'state':state()});save()
  old=state()['world'];open_file();check('Replay Open cancel preserves world',state()['world']==old)
  open_file(dest/'pr10-restart.b3rec');until(lambda:any(w['name']=='Load Replay'for w in ui()['windows']),5);screenshot('load-popup');tap_node(any_node('Button','Load'));until(lambda:not any(w['name']=='Load Replay'for w in ui()['windows']),10);q=frames();r['loaded_ui']=q;check('CPU Replay transport available',any(x['kind']=='Button'and x['label']=='>'for x in q['items']));check('CPU Replay timeline frame0',any(x['kind']=='SliderInt'and x['label']=='##frame'and x['value']=='0'for x in q['items']));screenshot('loaded-replay')
  def replay_frame():return int(any_node('SliderInt','##frame')['value'])
  for button,expected in [('>',1),('>',2),('<',1),('>|',10),('|<',0)]:
   tap_node(any_node('Button',button));until(lambda:replay_frame()==expected,3);check('CPU Replay '+button+' frame'+str(expected),replay_frame()==expected)
  tap_node(any_node('Button','Play '));until(lambda:replay_frame()>0,3);tap_node(any_node('Button','Pause'));check('CPU Replay Pause stops',state()['pause']==1);screenshot('replay-transport')
 else:
  screenshot('recording-unavailable')
  cpu_file=D/'raw/recording-replay-corrected-runs/cpu/pr10-restart.b3rec';assert cpu_file.exists();open_file(cpu_file);until(lambda:any(w['name']=='Load Replay'for w in ui()['windows']),5);screenshot('load-popup');tap_node(any_node('Button','Load'));q=frames();r['failed_replay_ui']=q;report('GPU Replay exclusion explained in UI',any(any(t in x['label'].lower()for t in ['unavailable','not implemented','unsupported'])for x in q['items']),{'ui':q});check('GPU Replay failure visible',any('failed to open file'in x['label']for x in q['items']));screenshot('replay-exclusion');tap_node(any_node('Button','Cancel'))
 focus();xdo('key','--clearmodifiers','ctrl+q');app.wait(timeout=5);check('clean quit',app.returncode==0);r['status']='REVIEWED_RECORDING_REPLAY_WITH_FINDINGS'if r['findings']else 'PASS_RECORDING_REPLAY';save();emit('Recording/Replay review closed')

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
