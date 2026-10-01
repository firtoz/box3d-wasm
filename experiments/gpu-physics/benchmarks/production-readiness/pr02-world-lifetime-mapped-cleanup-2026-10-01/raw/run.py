from pathlib import Path
import os,json,hashlib,subprocess,signal,time,shutil
R=Path.cwd();A=R/'artifacts/production-readiness/pr02-world-lifetime-mapped-cleanup';B=R/'artifacts/production-readiness/pr02-world-lifetime-C-applicability';C=R/'artifacts/production-readiness/pr02-world-lifetime-cleanup';F=R/'artifacts/production-readiness/pr02-world-lifetime-fix';H=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();P=json.loads((A/'protocol-before-builds.json').read_text());assert H(B/'receipt.json')==P['compiled_prerequisite_receipt_sha256'];assert not(A/'receipt.json').exists();old=json.loads((B/'receipt.json').read_text());I=json.loads((A/'fixture-input.json').read_text());assert H(I['file'])==I['sha256'];D={'status':'building','pid':os.getpid(),'protocol_sha256':H(A/'protocol-before-builds.json'),'fixture':I,'prerequisite_receipt_sha256':H(B/'receipt.json'),'builds':[],'runs':[],'started':time.time(),'adapter':old['adapter']}
def save():(A/'receipt.json').write_text(json.dumps(D,indent=2)+'\n')
def env(backend):
 e={k:v for k,v in os.environ.items()if not k.startswith(('GPU_','WGPU_','VK_'))}
 if backend=='native':
  raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],env=e);e.update(x.decode().split('=',1)for x in raw.split(b'\0')if x.startswith((b'GPU_',b'WGPU_')))
 e.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(A/'pipelines'));return e
save()
try:
 for cell in P['order']:
  folder=A/cell;folder.mkdir();item=[x for x in old['builds']if x['configuration']==cell and x['fixture']=='root'][0];cmd=item['command'].copy();cmd[cmd.index('-o')+1]=str(folder/'fixture-executable');cmd[cmd.index('-DGPU_WORLD_DIAGNOSTIC')]='-DGPU_MAPPED_CLEANUP_CONTRACT';source=[i for i,s in enumerate(cmd)if s.endswith('world_lifetime_diagnostic.cpp')];assert len(source)==1;cmd[source[0]]=I['file'];linked=old['proof'][cell]['linked_inputs'];assert all(H(p)==h for p,h in linked.items());save()
  with(folder/'build.log').open('w')as log:code=subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT).returncode
  D['builds'].append({'configuration':cell,'command':cmd,'exit':code,'log_sha256':H(folder/'build.log'),'linked_inputs':linked});save();assert code==0,(cell,code);D['builds'][-1]['binary_sha256']=H(folder/'fixture-executable');save()
 for cell in P['order']:
  folder=A/cell;binary=folder/'fixture-executable';e=env(cell.split('-')[0]);D['status']='running';save();print('launch',cell,flush=True)
  with(folder/'stdout.jsonl').open('w')as out,(folder/'stderr.log').open('w')as err:
   proc=subprocess.Popen([str(binary)],stdout=out,stderr=err,env=e,start_new_session=True);D['running']={'pid':proc.pid,'configuration':cell};save()
   try:code=proc.wait(timeout=P['watchdog_seconds'])
   except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGTERM);proc.wait(timeout=30);code=-999
  D.pop('running');rows=[json.loads(l)for l in(folder/'stdout.jsonl').read_text().splitlines()];summary=rows[-1]if rows else{};D['runs'].append({'configuration':cell,'exit':code,'binary_sha256':H(binary),'stdout_sha256':H(folder/'stdout.jsonl'),'stderr_sha256':H(folder/'stderr.log'),'environment':{k:v for k,v in e.items()if k.startswith(('GPU_','WGPU_','VK_'))},'summary':summary});save();checks=[x for x in rows if'match'in x];assert code==0 and summary=={'summary':True,'observations':21,'mismatches':0}and len(checks)==21 and all(x['match']for x in checks),(cell,summary);print('passed',cell,summary,flush=True)
 D.update(status='passed',finished=time.time());D.pop('pid');save()
except BaseException as e:
 D.update(status='stopped',failure=repr(e),finished=time.time());D.pop('pid',None);save()
 for f,h in json.loads((C/'protocol-before-change.json').read_text())['baseline_C'].items():assert H(C/'baseline'/f)==h;shutil.copy2(C/'baseline'/f,R/f)
 for f,h in json.loads((F/'protocol-before-change.json').read_text())['baseline'].items():assert H(F/'baseline'/f)==h;shutil.copy2(F/'baseline'/f,R/f)
 (R/'src/api/world/world_lifetime.rs').unlink(missing_ok=True);D['production_restored']=True;save();raise
