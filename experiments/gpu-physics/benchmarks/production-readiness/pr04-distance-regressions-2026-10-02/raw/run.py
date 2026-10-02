from pathlib import Path
import json,hashlib,subprocess,os,time,signal,shutil,tarfile,re,resource
R=Path.cwd();A=R/'artifacts/production-readiness/pr04-distance-regressions';P=A/'protocol.json';p=json.loads(P.read_text());assert not(A/'receipt.json').exists();sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();resource.setrlimit(resource.RLIMIT_CORE,(0,0));s={'status':'preflight','protocol_sha256':sha(P),'driver_sha256':sha(__file__),'started_unix':time.time(),'source_revision_context_only':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'toolchain':subprocess.check_output(['rustc','-Vv'],text=True),'builds':[],'results':[],'test_binaries':{}}
def save():
 t=A/'receipt.tmp';t.write_text(json.dumps(s,indent=2)+'\n');os.replace(t,A/'receipt.json')
def check(inputs):
 for n,h in inputs.items():assert sha(R/n)==h,n
 assert sha(p['prior_receipt'])==p['prior_receipt_sha256']
def execute(command,name,env,watchdog):
 out=A/name;out.mkdir(parents=True);start=time.time()
 with (out/'stdout.log').open('w') as so,(out/'stderr.log').open('w') as se:
  proc=subprocess.Popen(command,stdout=so,stderr=se,cwd=R,env=env,start_new_session=True);s['running']={'pid':proc.pid,'name':name,'command':command};save()
  try:code=proc.wait(timeout=watchdog)
  except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGKILL);proc.wait();code='timeout'
 s.pop('running',None);return {'name':name,'command':command,'exit':code,'stdout_sha256':sha(out/'stdout.log'),'stderr_sha256':sha(out/'stderr.log'),'elapsed_seconds_incidental':time.time()-start}
shader=R/'shaders/physics/solve.wgsl';edited=False;save()
try:
 prior=json.loads(Path(p['prior_receipt']).read_text());assert prior['status']=='completed-diagnostic' and prior['all_original_assertions_pass'];check(p['original_inputs'])
 try:
  shutil.copyfile(A/'candidate-solve.wgsl',shader);edited=True;check(p['candidate_inputs']);s['status']='building';s['compiled_inputs_before']=p['candidate_inputs'];save()
  with tarfile.open(A/'candidate-source-inputs.tar.gz','w:gz') as t:
   for n in p['candidate_inputs']:t.add(R/n,arcname=n,recursive=False)
  for b in p['builds']:
   check(p['candidate_inputs']);row=execute(b['command'],b['backend']+'-build',os.environ.copy(),p['build_watchdog_seconds']);s['builds'].append(row);save();assert row['exit']==0,row
   records=[]
   for line in (A/row['name']/'stdout.log').read_text().splitlines():
    try:records.append(json.loads(line))
    except ValueError:pass
   artifacts=[x for x in records if x.get('reason')=='compiler-artifact' and x['target']['name']=='gpu_physics' and x.get('executable')];assert len(artifacts)==1;dest=A/(b['backend']+'-test-executable');shutil.copy2(artifacts[0]['executable'],dest);assert os.access(dest,os.X_OK);s['test_binaries'][b['backend']]={'path':str(dest),'sha256':sha(dest),'compiler_artifact':artifacts[0]};check(p['candidate_inputs']);save();print('Built',b['backend'],'testbinary',flush=True)
  s['compiled_inputs_after']={n:sha(R/n) for n in p['candidate_inputs']};s['native_generated_inputs']={n:sha(R/n) for n in prior['native_generated_inputs']};assert s['native_generated_inputs']==prior['native_generated_inputs'];save()
 finally:
  if edited:
   assert sha(shader)==p['candidate_inputs']['shaders/physics/solve.wgsl'],'Unexpected concurrent edit; do not overwrite'
   shutil.copyfile(A/'original-solve.wgsl',shader);s['production_shader_restored']=sha(shader)==p['original_inputs']['shaders/physics/solve.wgsl'];save()
 s['status']='checking';save()
 for c in p['cases']:
  check(p['original_inputs']);assert sha(c['command'][0])==s['test_binaries'][c['backend']]['sha256'];env={k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_','VK_','WGPU_'))};env.update(c['environment_overrides']);row=execute(c['command'],c['name'],env,p['test_watchdog_seconds']);row.update(backend=c['backend'],ordering=c['ordering'],environment_overrides=c['environment_overrides'],selectors=c['selectors'],binary_sha256=s['test_binaries'][c['backend']]['sha256']);stderr=(A/c['name']/'stderr.log').read_text();stdout=(A/c['name']/'stdout.log').read_text();row['actual_NVIDIA_Vulkan']='NVIDIA GeForce RTX 4070 SUPER' in stderr and 'backend=Vulkan' in stderr;match=re.search(r'test result: (ok|FAILED)\. (\d+) passed; (\d+) failed; (\d+) ignored;',stdout);row['harness_result']=list(match.groups()) if match else None;row['individual_results']=re.findall(r'^test (\S+) \.\.\. (ok|FAILED)$',stdout,re.M);row['pass']=row['exit']==0 and row['harness_result']==['ok','61','0','0'] and row['actual_NVIDIA_Vulkan'] and {n for n,result in row['individual_results']}==set(c['selectors']);s['results'].append(row);save();print(json.dumps({k:row[k] for k in ['name','exit','harness_result','pass']}),flush=True);assert row['actual_NVIDIA_Vulkan'] and row['exit']!='timeout' and row['harness_result'],row
 s.update(status='completed-regressions',finished_unix=time.time(),all_assertions_pass=all(v['pass'] for v in s['results']),original_inputs_after={n:sha(R/n) for n in p['original_inputs']});save()
except BaseException as e:
 s.update(status='stopped',error=repr(e),finished_unix=time.time());save();raise
