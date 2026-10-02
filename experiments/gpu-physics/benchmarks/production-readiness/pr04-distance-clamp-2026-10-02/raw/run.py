from pathlib import Path
import json,hashlib,subprocess,os,time,signal,shutil,tarfile,re,resource
R=Path.cwd();A=R/'artifacts/production-readiness/pr04-distance-clamp';P=A/'protocol.json';p=json.loads(P.read_text());assert not(A/'receipt.json').exists();sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();resource.setrlimit(resource.RLIMIT_CORE,(0,0));s={'status':'preflight','protocol_sha256':sha(P),'driver_sha256':sha(__file__),'started_unix':time.time(),'source_revision_context_only':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'toolchains':{'rustc':subprocess.check_output(['rustc','-Vv'],text=True),'g++':subprocess.check_output(['g++','--version'],text=True)},'builds':[],'links':[],'results':[],'libraries':{}}
def save():
 t=A/'receipt.tmp';t.write_text(json.dumps(s,indent=2)+'\n');os.replace(t,A/'receipt.json')
def check(inputs):
 for n,h in inputs.items():assert sha(R/n)==h,n
 for n,h in p['references'].items():assert sha(n)==h,n
 for l in p['links']:
  assert sha(l['source'])==l['source_sha256']
  for n,h in l['reused_inputs'].items():assert sha(n)==h,n
# Preserve exact compiler-consumed fixture header/source inputs from the
# independent original linker/preprocessor receipt, rather than just checkout.
header_receipt=R/'artifacts/production-readiness/pr03-baseline-cpu-link-repair/protocol-before-builds.json';headers=json.loads(header_receipt.read_text())['inputs_before']
for n,h in headers.items():assert sha(n)==h,n
s['C_fixture_inputs_before']=headers;s['C_fixture_input_receipt_sha256']=sha(header_receipt)
def execute(command,name,env,watchdog):
 out=A/name;out.mkdir(parents=True);start=time.time()
 with (out/'stdout.log').open('w') as so,(out/'stderr.log').open('w') as se:
  proc=subprocess.Popen(command,stdout=so,stderr=se,cwd=R,env=env,start_new_session=True);s['running']={'pid':proc.pid,'name':name,'command':command};save()
  try:code=proc.wait(timeout=watchdog)
  except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGKILL);proc.wait();code='timeout'
 s.pop('running',None);return {'name':name,'command':command,'exit':code,'stdout_sha256':sha(out/'stdout.log'),'stderr_sha256':sha(out/'stderr.log'),'elapsed_seconds_incidental':time.time()-start}
shader=R/'shaders/physics/solve.wgsl';edited=False;save()
try:
 check(p['original_inputs']);shutil.copyfile(A/'candidate-solve.wgsl',shader);edited=True;check(p['candidate_inputs']);s['status']='building';s['compiled_inputs_before']=p['candidate_inputs'];save()
 with tarfile.open(A/'candidate-source-inputs.tar.gz','w:gz') as t:
  for n in p['candidate_inputs']:t.add(R/n,arcname=n,recursive=False)
 for b in p['builds']:
  check(p['candidate_inputs']);row=execute(b['command'],b['backend']+'-build',os.environ.copy(),p['build_watchdog_seconds']);s['builds'].append(row);save();assert row['exit']==0,row
  records=[]
  for line in (A/row['name']/'stdout.log').read_text().splitlines():
   try:records.append(json.loads(line))
   except ValueError:pass
  artifacts=[x for x in records if x.get('reason')=='compiler-artifact' and x['target']['name']=='gpu_physics'];files=[Path(f) for x in artifacts for f in x['filenames'] if f.endswith('.a')];assert len(files)==1;dest=A/(b['backend']+'-candidate.a');shutil.copy2(files[0],dest);s['libraries'][b['backend']]={'path':str(dest),'sha256':sha(dest),'compiler_artifacts':artifacts};s['compiled_inputs_after']={n:sha(R/n) for n in p['candidate_inputs']};check(p['candidate_inputs']);save();print('Built',b['backend'],flush=True)
 # Record generated native build/configuration bytes as distinct provenance.
 generated={}
 for base in [R/'target/native-backend',R/'target/native-workspace/experiments/gpu-physics']:
  for f in base.rglob('*'):
   if f.is_file() and not f.is_symlink() and f.suffix in ['.rs','.toml','.lock','.py','.patch'] and not f.name.endswith('cache'):generated[str(f.relative_to(R))]=sha(f)
 s['native_generated_inputs']=generated
 with tarfile.open(A/'native-generated-inputs.tar.gz','w:gz') as t:
  for n in generated:t.add(R/n,arcname=n,recursive=False)
finally:
 if edited:
  assert sha(shader)==p['candidate_shader_sha256'],'Unexpected simultaneous shader edit; do not overwrite'
  shutil.copyfile(A/'original-solve.wgsl',shader);s['production_shader_restored']=sha(shader)==p['original_inputs']['shaders/physics/solve.wgsl'];save()
try:
 check(p['original_inputs']);assert len(s['libraries'])==2
 for l in p['links']:
  for n,h in headers.items():assert sha(n)==h,n
  row=execute(l['command'],l['backend']+'-link',os.environ.copy(),p['link_watchdog_seconds']);s['links'].append(row);save();assert row['exit']==0,row
  dest=A/(l['backend']+'-distance');row.update(binary_sha256=sha(dest),source_sha256=l['source_sha256'],reused_inputs=l['reused_inputs'],Rust_library_sha256=s['libraries'][l['backend']]['sha256']);save()
 s['status']='checking';save()
 for c in p['cases']:
  if c['ordering']==1 and not all(v['pass'] for v in s['results'][:2]):break
  check(p['original_inputs']);env={k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_','VK_','WGPU_'))};env.update(c['environment_overrides']);row=execute(c['command'],c['name'],env,p['engine_watchdog_seconds']);row.update(backend=c['backend'],ordering=c['ordering'],environment_overrides=c['environment_overrides']);stderr=(A/c['name']/'stderr.log').read_text();stdout=(A/c['name']/'stdout.log').read_text();row['actual_NVIDIA_Vulkan']='NVIDIA GeForce RTX 4070 SUPER' in stderr and 'Vulkan' in stderr;row['first_discrepancy']=re.findall(r'(distance-joint|sphere-ground) step=(\d+) lane=(\d+) GPU=([^ ]+) CPU=([^\n]+)',stderr);records=[json.loads(line) for line in stdout.splitlines() if line.startswith('{')];row['completed_original_branches']=[{'scene':d['scene'],'substeps':d['substeps'],'recorded_postwarmup_steps':d['steps'],'timing_only':d['timing_only']} for d in records];row['pass']=row['exit']==0 and row['actual_NVIDIA_Vulkan'] and len(records)==4 and all(d['timing_only'] is False for d in records);s['results'].append(row);save();print(json.dumps({k:row[k] for k in ['name','exit','pass','first_discrepancy']}),flush=True);assert row['actual_NVIDIA_Vulkan'] and row['exit']!='timeout',row
 s.update(status='completed-diagnostic',finished_unix=time.time(),all_original_assertions_pass=len(s['results'])==4 and all(r['pass'] for r in s['results']),unlaunched_cases=[c['name'] for c in p['cases'][len(s['results']):]],original_inputs_after={n:sha(R/n) for n in p['original_inputs']});save()
except BaseException as e:
 s.update(status='stopped',error=repr(e),finished_unix=time.time());save();raise
