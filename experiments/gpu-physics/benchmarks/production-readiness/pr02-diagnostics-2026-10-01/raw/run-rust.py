import hashlib,json,os,subprocess
from pathlib import Path
root=Path.cwd();base=root/'artifacts/production-readiness/pr02-diagnostics';out=base/'rust';out.mkdir()
protocol=json.loads((root/'benchmarks/production-readiness/pr02-diagnostics-2026-10-01/protocol.json').read_text())
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
receipt={'scope':'PR02 focused diagnostics only; no timing or full physical qualification','budget':4,'runs':[],'adapter_driver':subprocess.check_output(['nvidia-smi','--query-gpu=name,driver_version,pci.bus_id','--format=csv,noheader'],text=True).strip()}
def save(): (out/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
save()
for backend in ['ordinary','native']:
 binary=base/'candidate-complete-inputs'/(backend+'-tests');build=base/'candidate-complete-inputs'/(backend+'-build.json');compiled=json.loads(build.read_text());assert sha(binary)==compiled['test_binary_sha256']
 env={k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_','WGPU_'))}
 if backend=='native':
  data=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],cwd=root,env=env)
  env.update(item.decode().split('=',1) for item in data.split(b'\0') if b'=' in item)
 env.update(WGPU_BACKEND='vulkan',GPU_PHYSICS_BACKEND='vulkan',GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(base/'pipelines'))
 available=subprocess.check_output([str(binary),'--list'],text=True)
 for index,selector in enumerate(protocol['rust_diagnostic_selectors'],1):
  assert selector+': test' in available
  label=f'{backend}-{index}';log=out/(label+'.log');cmd=[str(binary),selector,'--exact','--nocapture','--test-threads=1']
  if index==2:env['GPU_NATIVE_PEAK_TRACE']=str(out/(label+'.jsonl'))
  with log.open('w') as f:
   proc=subprocess.Popen(cmd,cwd=root,env=env,stdout=f,stderr=subprocess.STDOUT)
   receipt['running']={'pid':proc.pid,'configuration':backend,'selector':selector};save();code=proc.wait()
  receipt.pop('running');trial={'configuration':backend,'selector':selector,'exit':code,'binary_sha256':sha(binary),'build_receipt_sha256':sha(build),'log_sha256':sha(log),'environment':{k:v for k,v in env.items() if k.startswith(('GPU_','WGPU_'))}}
  receipt['runs'].append(trial);save();print(label,code,flush=True)
  if code or '1 passed; 0 failed' not in log.read_text():raise SystemExit('Failed focused trial retained; campaign stopped')
receipt['status']='pass';save()
