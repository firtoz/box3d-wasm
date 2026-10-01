from pathlib import Path
import os,json,hashlib,subprocess,signal,time,shutil
R=Path.cwd();A=R/'artifacts/production-readiness/pr02-world-lifetime-CPU-contract';B=R/'artifacts/production-readiness/pr02-world-lifetime-C-applicability';C=R/'artifacts/production-readiness/pr02-world-lifetime-cleanup';F=R/'artifacts/production-readiness/pr02-world-lifetime-fix';H=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();P=json.loads((A/'protocol-before-correction.json').read_text());assert not(A/'receipt.json').exists();old=json.loads((B/'receipt.json').read_text());assert H(B/'receipt.json')==P['prior_receipt_sha256'];proof=json.loads((A/'applicability-proof.json').read_text());assert proof['GPU_behavior_identical']and proof['before']['output_sha256']==proof['after']['output_sha256'];inputs=json.loads((F/'candidate-inputs.json').read_text());assert all(H(R/f)==h for f,h in inputs.items());cinput=json.loads((C/'candidate-inputs.json').read_text());assert all(H(R/f)==h for f,h in cinput.items()if f!='c_abi/world_cleanup_contract.cpp');D={'status':'running','pid':os.getpid(),'protocol_sha256':H(A/'protocol-before-correction.json'),'prior_build_receipt_sha256':H(B/'receipt.json'),'CPU_offline_and_GPU_source_proof_sha256':H(A/'applicability-proof.json'),'runs':[],'started':time.time(),'adapter':old['adapter']}
def save():(A/'receipt.json').write_text(json.dumps(D,indent=2)+'\n')
def env(backend):
 e={k:v for k,v in os.environ.items()if not k.startswith(('GPU_','WGPU_','VK_'))}
 if backend=='native':
  raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],env=e);e.update(x.decode().split('=',1)for x in raw.split(b'\0')if x.startswith((b'GPU_',b'WGPU_')))
 e.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(A/'pipelines'));return e
save()
try:
 for item in old['builds'][1:]:
  cell=item['configuration'];fixture=item['fixture'];binary=B/cell/(fixture+'-executable');assert H(binary)==item['binary_sha256'];folder=A/cell;folder.mkdir(exist_ok=True);e=env(cell.split('-')[0]);print('launch',cell,fixture,flush=True)
  with(folder/(fixture+'.stdout.jsonl')).open('w')as out,(folder/(fixture+'.stderr.log')).open('w')as err:
   proc=subprocess.Popen([str(binary)],stdout=out,stderr=err,env=e,start_new_session=True);D['running']={'pid':proc.pid,'configuration':cell,'fixture':fixture};save()
   try:code=proc.wait(timeout=P['watchdog_seconds'])
   except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGTERM);proc.wait(timeout=30);code=-999
  D.pop('running');rows=[json.loads(l)for l in(folder/(fixture+'.stdout.jsonl')).read_text().splitlines()];summary=rows[-1]if rows else{};r={'configuration':cell,'fixture':fixture,'exit':code,'binary_sha256':H(binary),'stdout_sha256':H(folder/(fixture+'.stdout.jsonl')),'stderr_sha256':H(folder/(fixture+'.stderr.log')),'environment':{k:v for k,v in e.items()if k.startswith(('GPU_','WGPU_','VK_'))},'summary':summary};D['runs'].append(r);save();checks=[x for x in rows if'match'in x];assert code==0 and summary.get('summary')and summary['observations']==len(checks)>0 and summary['mismatches']==0 and all(x['match']for x in checks),(cell,fixture,summary);assert all(H(R/f)==h for f,h in inputs.items());assert all(H(R/f)==h for f,h in cinput.items()if f!='c_abi/world_cleanup_contract.cpp');print('passed',cell,fixture,summary['observations'],flush=True)
 D.update(status='passed',finished=time.time());D.pop('pid');save()
except BaseException as e:
 D.update(status='stopped',failure=repr(e),finished=time.time());D.pop('pid',None);save()
 for f,h in json.loads((C/'protocol-before-change.json').read_text())['baseline_C'].items():assert H(C/'baseline'/f)==h;shutil.copy2(C/'baseline'/f,R/f)
 for f,h in json.loads((F/'protocol-before-change.json').read_text())['baseline'].items():assert H(F/'baseline'/f)==h;shutil.copy2(F/'baseline'/f,R/f)
 (R/'src/api/world/world_lifetime.rs').unlink(missing_ok=True);D['production_restored']=True;save();raise
