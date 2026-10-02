from pathlib import Path
import os,sys,json,hashlib,subprocess,time,signal,re,resource,importlib.util
R=Path.cwd();A=R/'artifacts/production-readiness/pr03-rain-phase-diagnostic';B=R/'artifacts/production-readiness/pr03-rain-phase-remaining';P=A/'protocol.json';sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();p=json.loads(P.read_text());q=json.loads((B/'protocol.json').read_text());original=json.loads((A/'receipt.json').read_text());assert not(B/'receipt.json').exists();s={'status':'starting','protocol_sha256':sha(B/'protocol.json'),'original_protocol_sha256':sha(P),'driver_sha256':sha(__file__),'started_unix':time.time(),'builds':[],'results':[],'observer_executable_sha256':q['observer_executable_sha256'],'observer_object_sha256':q['observer_object_sha256']};resource.setrlimit(resource.RLIMIT_CORE,(0,0));sys.dont_write_bytecode=True;sys.path.insert(0,str(R/'scripts'));spec=importlib.util.spec_from_file_location('rain',R/'scripts/compare-rain-lifetimes.py');rain=importlib.util.module_from_spec(spec);spec.loader.exec_module(rain);from native_scene_validate import record_complete
s['evaluator_hashes']={n:sha(R/'scripts'/n) for n in ['compare-rain-lifetimes.py','health_identity.py','native_scene_validate.py']}
def save():
 t=B/'receipt.tmp';t.write_text(json.dumps(s,indent=2)+'\n');os.replace(t,B/'receipt.json')
def check():
 for key in ['parent_viewer_receipt','parent_viewer_parent_receipt']:assert sha(p[key])==p[key+'_sha256']
 for mapping in ['local_source_inputs','observer_sources','reused_object_archive_hashes','actual_link_inputs','prior_failed_protocols']:
  for n,h in p[mapping].items():assert sha(R/n if mapping=='local_source_inputs' else n)==h,n
 for n,h in s['evaluator_hashes'].items():assert sha(R/'scripts'/n)==h
 d=p['display_dependency'];assert sha(d['binary'])==d['sha256'] and sha(d['helper'])==d['helper_sha256']
def execute(command,cwd,env,out,timeout):
 start=time.time()
 with (out/'stdout.log').open('w') as so,(out/'stderr.log').open('w') as se:
  proc=subprocess.Popen(command,cwd=cwd,env=env,stdout=so,stderr=se,start_new_session=True);s['running']={'pid':proc.pid,'command':command};save()
  try:code=proc.wait(timeout=timeout)
  except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGKILL);proc.wait();code='timeout'
 s.pop('running',None);return {'exit':code,'command':command,'elapsed_seconds_incidental':time.time()-start,'stdout_sha256':sha(out/'stdout.log'),'stderr_sha256':sha(out/'stderr.log')}
def header(path):
 with path.open() as f:
  prefix=[]
  for line in f:
   if line.strip()=='"frames": [':break
   prefix.append(line)
  else:raise ValueError('missingframeheader')
  return json.loads(''.join(prefix)+'"frames": []\n}')
def validate(path,steps):
 h=header(path);assert (h['worker_count'],h['enable_sleep'],h['warmup'],h['unpaced'],h['completed_step_mode'])==(8,True,0,True,False);count=bodies=joints=0
 for count,frame in enumerate(rain.records(path,steps),1):
  status,detail=record_complete(dict(h,warmup=count-1,timed=1,measured=1,frames=[dict(frame,i=0)]),'Benchmark/Rain',1);assert status=='ok',detail
  for joint in frame['joints']:rain.spherical_limits(joint,True)
  bodies+=len(frame['bodies']);joints+=len(frame['joints'])
 assert count==steps;return {'frames':count,'body_observations':bodies,'joint_observations':joints,'status':'originalcomplete-record/sphericalchecks;no600stepclaim'}
save()
try:
 check()
 for n in ['protocol','driver','receipt']:assert sha(A/(n+'.json' if n!='driver' else 'run.py'))==q['original_'+n+'_sha256']
 for f,h in q['completed_control'].items():assert sha(A/'control30'/f)==h
 assert sha(A/'samples_gpu_observed')==q['observer_executable_sha256']
 assert sha(A/'sokol_bench_hooks.o')==q['observer_object_sha256']
 row=original['results'][0];assert row['sokol_errors']==[['30','0']] and row['phase_frames']==0 and row['actual_NVIDIA_Vulkan']
 row.update(health_sha256=sha(A/'control30/health.json'),health_bytes=(A/'control30/health.json').stat().st_size,record_validation=validate(A/'control30/health.json',30))
 s['retained_control']=row;s['status']='diagnosing';save();print(json.dumps({'retained_control_validation':row['record_validation']}),flush=True)
 for c in p['cases'][1:]:
  check();assert sha(A/'samples_gpu_observed')==s['observer_executable_sha256'];out=A/c['name'];out.mkdir();cwd=Path(c['cwd']);cwd.mkdir();(cwd/'settings.ini').write_text('{}\n');command=[sys.executable,p['display_dependency']['helper'],p['display_dependency']['binary'],str(out/'child-exit.json'),*c['command']];row=execute(command,cwd,c['environment'],out,p['watchdog_seconds_per_process']);row['name']=c['name'];err=(out/'stderr.log').read_text();row['actual_NVIDIA_Vulkan']='NVIDIA GeForce RTX 4070 SUPER' in err and 'Vulkan' in err;row['child_exit']=json.loads((out/'child-exit.json').read_text())['exit'] if (out/'child-exit.json').exists() else None;row['health_exists']=(out/'health.json').exists();row['sokol_errors']=[list(v) for v in re.findall(r'samples: (\d+) frames, (\d+) sokol errors',err)];rows=[json.loads(l[len('phase-progress '):]) for l in err.splitlines() if l.startswith('phase-progress ')];row['phase_rows']=len(rows);row['phase_frames']=sum(v['kind']=='frame' for v in rows);row['last_phase_row']=rows[-1] if rows else None;s['results'].append(row);save();print(json.dumps(row),flush=True);check();assert row['exit']==row['child_exit']==0 and row['actual_NVIDIA_Vulkan'] and row['sokol_errors']==[[str(c['steps']),'0']] and row['health_exists'],row
  row['health_sha256']=sha(out/'health.json');row['health_bytes']=(out/'health.json').stat().st_size;row['record_validation']=validate(out/'health.json',c['steps']);assert row['phase_frames']==(c['steps'] if c['observer_enabled'] else 0);s['results'][-1]=row;save()
  if c['name']=='observer30':
   count=0
   for count,(control,observed) in enumerate(zip(rain.records(A/'control30/health.json',30),rain.records(A/'observer30/health.json',30),strict=True),1):
    for field in ['sample','submitted_step','body_count','joint_count','nan_count','min_y','max_y','max_speed','exploded','bodies','joints']:assert control[field]==observed[field],(count,field)
   assert count==30;s['observer_equivalence']={'frames':30,'ordered_body_joint_semantic_fields_exact':True,'scope':'capturedpublicphysics/identity/residual subset;notfullfuture-state'};save()
 s.update(status='completed-diagnostic',finished_unix=time.time());save()
except BaseException as e:s.update(status='stopped',error=repr(e),finished_unix=time.time(),unlaunched=[c['name'] for c in p['cases'][1+len(s['results']):]]);s.pop('running',None);save();raise
