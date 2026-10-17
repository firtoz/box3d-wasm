from pathlib import Path
import json,hashlib,os,re,signal,stat,struct,subprocess,sys,time
R=Path.cwd();D=Path(__file__).resolve().parent;phase,label=sys.argv[1:3];assert phase in ['before','after'];build=json.loads((D/'render-popup-build.json').read_text());v=next(x for x in build['viewers']if x['label']==label);base=next(x for x in json.loads((D/'shared-protocol.json').read_text())['viewers_CPU_first']if x['label']==label);tag=sys.argv[3]if len(sys.argv)>3 else label;assert re.fullmatch('[a-z0-9-]+',tag);dest=D/'raw'/'catalog-controls-runs'/tag;dest.mkdir(parents=True,exist_ok=False)
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
 assert time.monotonic()-start<120,'120s catalog budget'
 p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'
 p=dest/'api.jsonl';assert not p.exists()or p.stat().st_size<8*1024**2,'API cap'
 for name in ['stdout.txt','stderr.txt']:
  p=dest/name;assert not p.exists()or p.stat().st_size<1024**2,'Appoutput cap'
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
 assert len(r['screenshots'])<6;p=dest/(name+'.png');assert not p.exists();q=frames();small(['ffmpeg','-hide_banner','-loglevel','error','-nostdin','-threads','1','-f','x11grab','-draw_mouse','0','-window_id',str(wid),'-i',env['DISPLAY'],'-frames:v','1','-threads','1',str(p)],5)
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
 key('p');until(lambda:state()['pause']==1,3);inventory=json.loads((D/'inventory.json').read_text());expected={}
 for row in inventory['native_samples']:
  category,name=row['cpu_identity'].split('/',1);expected.setdefault(category,[]).append(name)
 r['catalog_categories']=[];r['scene_transitions']=[]
 def node(kind,field,root=False):
  q=frames();rows=[x for x in q['items']if x['kind']==kind and x['label']==field and x['visible']and (not root or x['window']=='##MainMenuBar')];assert rows,(kind,field,root);return rows[-1]
 def tap_node(n):
  assert not n['disabled'];hover((n['rect'][0]+n['rect'][2])/2,(n['rect'][1]+n['rect'][3])/2)
  def send():xdo('mousedown',1);held.add('mouse1');time.sleep(.04);xdo('mouseup',1);held.remove('mouse1')
  return action('catalog '+n['kind']+' '+n['label'],send,n)
 def samples_root():
  n=node('Menu','Samples',True);hover((n['rect'][0]+n['rect'][2])/2,(n['rect'][1]+n['rect'][3])/2);n=node('Menu','Samples',True)
  if n['value']!='true':tap_node(n)
  until(lambda:any(x['kind']=='Menu'and x['label']=='Samples'and x['window']=='##MainMenuBar'and x['value']=='true'for x in ui()['items']),3)
 def category_menu(field):
  samples_root();q=frames();n=next(x for x in q['items']if x['kind']=='Menu'and x['label']==field and x['window']!='##MainMenuBar'and x['visible']);hover((n['rect'][0]+n['rect'][2])/2,(n['rect'][1]+n['rect'][3])/2);q=frames();n=next(x for x in q['items']if x['kind']=='Menu'and x['label']==field and x['window']!='##MainMenuBar'and x['visible'])
  if n['value']!='true':tap_node(n)
  until(lambda:any(x['kind']=='Menu'and x['label']==field and x['window']!='##MainMenuBar'and x['value']=='true'for x in ui()['items']),3)
 for category,names in expected.items():
  category_menu(category);q=frames();leaves=[x for x in q['items']if x['kind']=='MenuItem'];actual=[x['label']for x in leaves];check('catalog '+category+' exactnames',actual==names);r['catalog_categories'].append({'category':category,'expected':names,'actual_items':leaves});save()
 check('catalog full165/22',sum(len(x['actual_items'])for x in r['catalog_categories'])==165 and len(r['catalog_categories'])==22);screenshot('catalog');tap_node(node('Menu','Samples',True))
 key('ctrl+o');q=frames();rows=[x for x in q['items']if x['kind']=='Selectable'];all_expected=[x['cpu_identity'].replace('/','  >  ',1)for x in inventory['native_samples']];check('picker lists all165 exact identities',[x['label']for x in rows]==all_expected);r['picker_all_labels']=[x['label']for x in rows];screenshot('picker');old_selected=next(x['label']for x in rows if x['value']=='true');key('Down');q=frames();selected=next(x['label']for x in q['items']if x['kind']=='Selectable'and x['value']=='true');check('picker Down selects next',selected==all_expected[1]);key('Up');q=frames();check('picker Up restores first',next(x['label']for x in q['items']if x['kind']=='Selectable'and x['value']=='true')==old_selected)
 action('picker filter Jenga',lambda:xdo('type','--clearmodifiers','--','Jenga'));q=frames();check('picker query displayed',any(x['kind']=='InputTextWithHint'and x['label']=='##q'and x['value']=='Jenga'for x in q['items']));rows=[x for x in q['items']if x['kind']=='Selectable'];check('picker Jenga unique result',len(rows)==1 and rows[0]['label']=='Stacking  >  Jenga Stack');old=state()['world'];tap_node(rows[0]);until(lambda:state()['world']!=old and state()['body_count']==61 and state()['shape_count']==61,25);until(lambda:any(w['name']=='Info'for w in ui()['windows']),25);r['scene_transitions'].append({'action':'picker mouse Jenga','before_world':old,'after_state':state()});check('Jenga newworld61bodies/shapes',state()['world']!=old and state()['body_count']==61);screenshot('jenga')
 category_menu('Stacking');old=state()['world'];tap_node(node('MenuItem','Single Box'));until(lambda:state()['world']!=old and state()['body_count']==2 and state()['shape_count']==2,25);until(lambda:any(w['name']=='Info'for w in ui()['windows']),25);r['scene_transitions'].append({'action':'Samples menu SingleBox','before_world':old,'after_state':state()});check('menu SingleBox newworld2',state()['world']!=old and state()['body_count']==2)
 open_menu('Sim');old=state()['world'];tap('MenuItem','Next Sample');until(lambda:state()['world']!=old and state()['body_count']==31 and state()['shape_count']==31,25);until(lambda:any(w['name']=='Info'for w in ui()['windows']),25);r['scene_transitions'].append({'action':'NextSample SphereStack','before_world':old,'after_state':state()});check('Next Sample creates SphereStack31',state()['body_count']==31)
 open_menu('Sim');old=state()['world'];tap('MenuItem','Previous Sample');until(lambda:state()['world']!=old and state()['body_count']==2 and state()['shape_count']==2,25);until(lambda:any(w['name']=='Info'for w in ui()['windows']),25);r['scene_transitions'].append({'action':'PreviousSample SingleBox','before_world':old,'after_state':state()});check('Previous Sample returns SingleBox2',state()['body_count']==2);check('selection preserves solver defaults',state()['hertz']==60 and state()['substeps']==4 and state()['world_flags']==[1,1,1]);screenshot('returned')
 open_menu('Sim');tap('MenuItem','Dump Mem Stats');r['memory_dump_requested_state']=state()
 if 'both'in label:
  initial=state();n=find('Checkbox','Split: CPU left / GPU right');check('combined initial split enabled',n['value']=='true');tap('Checkbox','Split: CPU left / GPU right');check('overlay checkbox state changes',find('Checkbox','Split: CPU left / GPU right')['value']=='false');check('overlay mode preserves identity/counts',state()['world']==initial['world']and state()['body_count']==2 and state()['shape_count']==2);screenshot('overlay');tap('Checkbox','Split: CPU left / GPU right');check('split checkbox restores',find('Checkbox','Split: CPU left / GPU right')['value']=='true');check('resplit preserves identity/counts',state()['world']==initial['world']and state()['body_count']==2 and state()['shape_count']==2);screenshot('resplit')
  from PIL import Image
  def actor_centroids(stage):
   im=Image.open(dest/(stage+'.png')).convert('RGB');w,h=im.size;positions={'CPU':[],'GPU':[]}
   for y in range(round(h*.25),round(h*.75),2):
    for x in range(0,round(w*.79),2):
     red,green,blue=im.getpixel((x,y))
     if red>green and green>blue*1.2 and red>130 and green>100:positions['CPU'].append(x)
     if blue>red*1.3 and green>red*1.15 and green>100 and red>70:positions['GPU'].append(x)
   assert all(positions.values()),positions;return {k:sum(v)/len(v)for k,v in positions.items()}
  split=actor_centroids('returned');overlay=actor_centroids('overlay');restored=actor_centroids('resplit');w=Image.open(dest/'overlay.png').width;r['split_image_effects']={'split':split,'overlay':overlay,'restored':restored};check('split actualCPUleftGPUright',split['CPU']<w*.45 and split['GPU']>w*.55);check('overlay actors share central viewport',abs(overlay['CPU']-overlay['GPU'])<w*.15 and all(w*.35<x<w*.65 for x in overlay.values()));check('resplit actualCPUleftGPUright',restored['CPU']<w*.45 and restored['GPU']>w*.55)
 focus();xdo('key','--clearmodifiers','ctrl+q');app.wait(timeout=5);check('clean quit',app.returncode==0);text=(dest/'stdout.txt').read_text()+(dest/'stderr.txt').read_text();r['memory_dump_output']=[line for line in text.splitlines()if any(x in line for x in ['id pools','body ids:','static tree:','dynamic tree:','GPU primary owned physics buffer bytes:','reserved slots bodies/shapes/joints/contacts:','excludes staging, cache copies, pipelines'])];check('DumpMemStats actual output',('id pools'in text and 'body ids:'in text)if label=='cpu'else 'GPU primary owned physics buffer bytes:'in text and 'excludes staging, cache copies, pipelines'in text);r['status']='PASS_FULL_CATALOG_NAVIGATION_AND_SPLIT';save();emit('165catalog/picker andscene lifetimes verified; combinedsplit actualimages checked')

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
