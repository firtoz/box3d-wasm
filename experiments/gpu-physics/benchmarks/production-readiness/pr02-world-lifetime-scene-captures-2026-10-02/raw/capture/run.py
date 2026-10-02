from pathlib import Path
import hashlib,json,subprocess,shutil,os,time,signal
R=Path.cwd();O=R/'artifacts/production-readiness/pr02-world-lifetime-scene-captures';P=O/'protocol-before-captures.json'
sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
def write(p,d):
 tmp=Path(str(p)+'.tmp');tmp.write_text(json.dumps(d,indent=2)+'\n');os.replace(tmp,p)
p=json.loads(P.read_text());assert not (O/'receipt.json').exists();assert sha(p['build_receipt'])==p['build_receipt_sha256'];assert sha(p['recorder']['binary'])==p['recorder']['sha256'];assert sha(p['oracle'])==p['oracle_sha256']
for n,h in p['wrapper_sources'].items():assert sha(R/n)==h
b=json.loads(Path(p['build_receipt']).read_text());assert b['status']=='built' and b['inputs_before']==b['inputs_after']
for n,h in b['inputs_before'].items():assert sha(R/n)==h,n
old=R/'recordings/snapshots'/p['cpu_label'];archive=O/'previous-cpu-column';assert not archive.exists();assert {x.name:sha(x) for x in old.iterdir() if x.is_file()}==p['prior_cpu_column'];shutil.move(str(old),archive);old.mkdir()
replacing=set(p['standard_scenes'])|{x['id'] for x in p['compound_scenes']};kept={}
for f in archive.iterdir():
 if f.is_file() and f.name.endswith(('.mp4','-recording-manifest.json')) and f.name not in {x+s for x in replacing for s in ['.mp4','-recording-manifest.json']} and f.name not in ['compound-properties-recording-manifest.json','recording-manifest.json']:
  shutil.copy2(f,old/f.name);kept[f.name]=sha(f)
write(O/'preserved-prior-cpu.json',{'archive_all_files':p['prior_cpu_column'],'unrelated_copies':kept,'old_metrics_not_copied':True})
e={k:v for k,v in os.environ.items() if not k.startswith(('GPU_','WGPU_','VK_','__NV','__GLX','LIBGL','NATIVE_CAPTURE','SKIP_'))}
e.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(O/'standard-pipelines'),GPU_RECORD_BIN=p['recorder']['binary'],GPU_RECORD_ORACLE=p['oracle'],FRAMES='300',SKIP_METRICS='1',METRIC_RUNS='1',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json')
state={'status':'running','driver_pid':os.getpid(),'protocol_sha256':sha(P),'driver_sha256':sha(__file__),'started_unix':time.time(),'results':[]};write(O/'receipt.json',state)
planned=[('cpu',s) for s in p['standard_scenes']]+[('native-cpu','six-compound-scenes')]+[('gpu',s) for s in p['standard_scenes']]+[('native-gpu','six-compound-scenes')]
try:
 for kind,scene in planned:
  out=O/(kind+'-'+scene);out.mkdir(exist_ok=False);env=e.copy();cpu=kind.endswith('cpu');script='scripts/record-box3d-oracle.sh' if cpu else 'scripts/record-snapshot.sh';label=p['cpu_label'] if cpu else p['gpu_label'];cmd=['bash',script,label]
  if kind.startswith('native'):env['NATIVE_CAPTURE_PROTOCOL']=str(O/'native/protocol.json')
  else:
   env['RECORD_SCENES']=scene
   if scene=='mixed-topology':env['GPU_RECORD_VIEW']=p['settings']['mixed_topology_camera']
   assert not (R/'recordings/snapshots'/label/(scene+'.mp4')).exists()
  row={'kind':kind,'scene':scene,'status':'running','command':cmd,'environment':{k:v for k,v in env.items() if k.startswith(('GPU_','WGPU_','VK_','NATIVE_','FRAMES','SKIP_','METRIC_','RECORD_'))},'started_unix':time.time()};state['results'].append(row);write(out/'launch.json',row);write(O/'receipt.json',state)
  with (out/'stdout.log').open('w') as f:
   app=subprocess.Popen(cmd,env=env,stdout=f,stderr=subprocess.STDOUT,start_new_session=True);row['wrapper_pid']=app.pid;write(O/'receipt.json',state)
   try:code=app.wait(timeout=(p['watchdog_seconds_per_scene']*6+120 if kind.startswith('native') else p['watchdog_seconds_per_scene']))
   except subprocess.TimeoutExpired:os.killpg(app.pid,signal.SIGTERM);app.wait(timeout=30);raise
  row.update(exit=code,finished_unix=time.time(),log_sha256=sha(out/'stdout.log'));assert code==0,row
  if not kind.startswith('native'):
   clip=R/'recordings/snapshots'/label/(scene+'.mp4');probe=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_streams','-of','json',str(clip)],text=True));stream=next(x for x in probe['streams'] if x['codec_type']=='video');write(out/'ffprobe.json',probe);assert stream['width']==1280 and stream['height']==720 and stream['r_frame_rate']=='30/1' and int(stream['nb_frames'])==300;row.update(clip_sha256=sha(clip),frames=300)
   # Oracle timings are retained as incidental capture data, never added to chart metrics.json.
   if cpu:row['incidental_cpu_metrics_sha256']=sha(R/'recordings/snapshots'/label/('metrics-'+scene+'.json'))
  else:
   n=json.loads((O/'native/receipt.json').read_text());assert n['status']==('cpu-complete' if cpu else 'captured')
  row['status']='pass';write(out/'result.json',row);write(O/'receipt.json',state);print(kind,scene,'capture pass',flush=True)
 state.update(status='captured',finished_unix=time.time());state.pop('driver_pid',None)
except BaseException as ex:
 state.update(status='stopped',error=repr(ex),finished_unix=time.time(),unlaunched=[{'kind':k,'scene':s} for k,s in planned if not any(x['kind']==k and x['scene']==s for x in state['results'])]);raise
finally:write(O/'receipt.json',state)
