from pathlib import Path
import json,hashlib,subprocess,os,time,signal,shutil,tarfile,re,resource
R=Path.cwd();A=R/'artifacts/production-readiness/pr04-distance-focused-test';P=A/'protocol.json';p=json.loads(P.read_text());assert not(A/'receipt.json').exists()
sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();resource.setrlimit(resource.RLIMIT_CORE,(0,0))
s={'status':'preflight','protocol_sha256':sha(P),'driver_sha256':sha(__file__),'source_revision_context_only':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'toolchain':subprocess.check_output(['rustc','-Vv'],text=True),'started_unix':time.time(),'builds':[],'results':[],'test_binaries':{}}
def save():
 t=A/'receipt.tmp';t.write_text(json.dumps(s,indent=2)+'\n');os.replace(t,A/'receipt.json')
def check():
 for n,h in p['compiled_inputs'].items():assert sha(R/n)==h,n
 assert sha(p['prior_producer'])==p['prior_producer_sha256'] and sha(p['prior_regression_protocol'])==p['prior_regression_protocol_sha256']
def execute(command,name,env,watchdog):
 out=A/name;out.mkdir();start=time.time()
 with (out/'stdout.log').open('w') as so,(out/'stderr.log').open('w') as se:
  proc=subprocess.Popen(command,stdout=so,stderr=se,cwd=R,env=env,start_new_session=True);s['running']={'pid':proc.pid,'name':name,'command':command};save()
  try:code=proc.wait(timeout=watchdog)
  except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGKILL);proc.wait();code='timeout'
 s.pop('running',None)
 return {'name':name,'command':command,'exit':code,'stdout_sha256':sha(out/'stdout.log'),'stderr_sha256':sha(out/'stderr.log'),'elapsed_seconds_incidental':time.time()-start}
save()
try:
 check();s['compiled_inputs_before']=p['compiled_inputs'];s['status']='building';save()
 with tarfile.open(A/'source-inputs.tar.gz','w:gz') as t:
  for n in p['compiled_inputs']:t.add(R/n,arcname=n,recursive=False)
 for b in p['builds']:
  check();row=execute(b['command'],b['backend']+'-build',os.environ.copy(),p['build_watchdog_seconds']);s['builds'].append(row);save();assert row['exit']==0,row
  records=[]
  for line in (A/row['name']/'stdout.log').read_text().splitlines():
   try:records.append(json.loads(line))
   except ValueError:pass
  artifacts=[x for x in records if x.get('reason')=='compiler-artifact' and x['target']['name']=='gpu_physics' and x.get('executable')];assert len(artifacts)==1
  dest=A/(b['backend']+'-test-executable');shutil.copy2(artifacts[0]['executable'],dest);assert os.access(dest,os.X_OK)
  s['test_binaries'][b['backend']]={'path':str(dest),'sha256':sha(dest),'compiler_artifact':artifacts[0]};check();save();print('Built',b['backend'],'focused test',flush=True)
 prior=json.loads(Path(p['prior_producer']).read_text());s['native_generated_inputs']={n:sha(R/n) for n in prior['native_generated_inputs']};assert s['native_generated_inputs']==prior['native_generated_inputs']
 s['compiled_inputs_after']={n:sha(R/n) for n in p['compiled_inputs']};s['status']='checking';save()
 for c in p['cases']:
  check();assert sha(c['command'][0])==s['test_binaries'][c['backend']]['sha256']
  env={k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_','VK_','WGPU_'))};env.update(c['environment_overrides'])
  row=execute(c['command'],c['name'],env,p['test_watchdog_seconds']);row.update(backend=c['backend'],ordering=c['ordering'],environment_overrides=c['environment_overrides'],binary_sha256=s['test_binaries'][c['backend']]['sha256'])
  stderr=(A/c['name']/'stderr.log').read_text();stdout=(A/c['name']/'stdout.log').read_text();row['actual_NVIDIA_Vulkan']='NVIDIA GeForce RTX 4070 SUPER' in stderr and 'backend=Vulkan' in stderr
  match=re.search(r'test result: (ok|FAILED)\. (\d+) passed; (\d+) failed; (\d+) ignored;',stdout);row['harness_result']=list(match.groups()) if match else None
  row['pass']=row['exit']==0 and row['harness_result']==['ok','1','0','0'] and row['actual_NVIDIA_Vulkan'] and 'distance first step:' in stdout
  s['results'].append(row);save();print(json.dumps({k:row[k] for k in ['name','exit','harness_result','pass']}),flush=True)
  assert row['actual_NVIDIA_Vulkan'] and row['exit']!='timeout' and row['harness_result'],row
 s.update(status='completed-focused-test',finished_unix=time.time(),all_assertions_pass=all(x['pass'] for x in s['results']));save()
except BaseException as e:s.update(status='stopped',error=repr(e),finished_unix=time.time());save();raise
