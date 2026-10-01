import hashlib,json,os,subprocess
from pathlib import Path
root=Path.cwd();base=root/'artifacts/production-readiness/pr02-diagnostics';out=base/'regressions';out.mkdir()
protocol=root/'benchmarks/production-readiness/pr02-diagnostics-2026-10-01/protocol.json';selection=json.loads(protocol.read_text())['regression_selectors'];assert len(selection)==8
receipt={'budget':16,'scope':'eight preserved diagnostic/island/capacity/lifetime regressions on both backends; not final physical matrix','protocol_sha256':hashlib.sha256(protocol.read_bytes()).hexdigest(),'runs':[]}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def save():(out/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
save()
for backend in ['ordinary','native']:
 binary=base/'candidate-complete-inputs'/(backend+'-tests');compiled=json.loads((base/'candidate-complete-inputs'/(backend+'-build.json')).read_text());assert sha(binary)==compiled['test_binary_sha256']
 env={k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_','WGPU_'))}
 if backend=='native':
  raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],cwd=root,env=env);env.update(item.decode().split('=',1) for item in raw.split(b'\0') if b'=' in item)
 env.update(WGPU_BACKEND='vulkan',GPU_PHYSICS_BACKEND='vulkan',GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(base/'pipelines'))
 available=subprocess.check_output([str(binary),'--list'],text=True)
 for index,selector in enumerate(selection,1):
  assert selector+': test' in available
  log=out/f'{backend}-{index}.log';cmd=[str(binary),selector,'--exact','--nocapture','--test-threads=1']
  with log.open('w') as f:
   proc=subprocess.Popen(cmd,cwd=root,env=env,stdout=f,stderr=subprocess.STDOUT);receipt['running']={'pid':proc.pid,'selector':selector,'backend':backend};save();code=proc.wait()
  receipt.pop('running');receipt['runs'].append({'configuration':backend,'selector':selector,'command':cmd,'exit':code,'log_sha256':sha(log),'binary_sha256':sha(binary),'environment':{k:v for k,v in env.items() if k.startswith(('GPU_','WGPU_'))}});save();print(backend,index,code,flush=True)
  if code or '1 passed; 0 failed' not in log.read_text():raise SystemExit('Regression failed, retained; campaign stopped')
receipt['status']='pass';save()
