from pathlib import Path
import os,json,hashlib,subprocess,signal,time,shutil,re
R=Path.cwd();A=R/'artifacts/production-readiness/pr02-world-lifetime-rust-validation';B=R/'artifacts/production-readiness/pr02-world-lifetime-fix';P=json.loads((A/'protocol-before-runs.json').read_text());H=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();inputs=json.loads((B/'candidate-inputs.json').read_text());assert not(A/'receipt.json').exists();assert H(B/'candidate-inputs.json')==P['inputs_sha256'];assert all(H(R/f)==h for f,h in inputs.items());D={'status':'running','pid':os.getpid(),'protocol_sha256':H(A/'protocol-before-runs.json'),'runs':[],'started':time.time(),'adapter':subprocess.check_output(['nvidia-smi','--query-gpu=name,uuid,driver_version','--format=csv'],text=True)}
def save():(A/'receipt.json').write_text(json.dumps(D,indent=2)+'\n')
def env(backend):
 e={k:v for k,v in os.environ.items()if not k.startswith(('GPU_','WGPU_','VK_'))}
 if backend=='native':
  raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],env=e);e.update(x.decode().split('=',1)for x in raw.split(b'\0')if x.startswith((b'GPU_',b'WGPU_')))
 e.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(A/'pipelines'));return e
save()
try:
 for index,selector in enumerate(P['filters']):
  for backend in ['ordinary','native']:
   test=P['tests'][backend];binary=Path(test['path']);assert H(binary)==test['sha256'];folder=A/(f'{index:02}-'+backend);folder.mkdir();e=env(backend);cmd=[str(binary),selector,'--nocapture','--test-threads=1'];print('launch',backend,selector,flush=True)
   with(folder/'run.log').open('w')as out:
    proc=subprocess.Popen(cmd,stdout=out,stderr=subprocess.STDOUT,env=e,start_new_session=True);D['running']={'pid':proc.pid,'configuration':backend,'selector':selector};save()
    try:code=proc.wait(timeout=P['watchdog_seconds'])
    except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGTERM);proc.wait(timeout=30);code=-999
   D.pop('running');s=(folder/'run.log').read_text();match=re.search(r'test result: (ok|FAILED)\. (\d+) passed; (\d+) failed;',s);result={'selector':selector,'configuration':backend,'command':cmd,'binary_sha256':H(binary),'environment':{k:v for k,v in e.items()if k.startswith(('GPU_','WGPU_','VK_'))},'exit':code,'log_sha256':H(folder/'run.log'),'passed':int(match[2])if match else 0,'failed':int(match[3])if match else -1};D['runs'].append(result);save();assert code==0 and result['passed']>0 and result['failed']==0,(backend,selector,result);assert all(H(R/f)==h for f,h in inputs.items());print('passed',backend,selector,result['passed'],flush=True)
 D.update(status='passed',finished=time.time());D.pop('pid');save()
except BaseException as e:
 D.update(status='stopped',failure=repr(e),finished=time.time());D.pop('pid',None);save();before=json.loads((B/'protocol-before-change.json').read_text())
 for f,h in before['baseline'].items():assert H(B/'baseline'/f)==h;shutil.copy2(B/'baseline'/f,R/f)
 (R/'src/api/world/world_lifetime.rs').unlink();D['production_restored']=True;save();raise
