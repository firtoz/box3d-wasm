from pathlib import Path
import subprocess,json,os,hashlib
root=Path.cwd();out=root/'artifacts/production-readiness/pr01-speculative/ccd-safe';protocol=json.loads((out/'regression-selectors.json').read_text())
for backend,path in [('ordinary','target/speculative-tests/release/deps/gpu_physics-f3c11a04cf20bcdb'),('native','target/native-cache-build/release/deps/gpu_physics-e01a65b975ea9699')]:
 d=out/('regressions-'+backend);d.mkdir();binary=root/path;env={k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_','WGPU_'))}
 if backend=='native':
  policy=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],env=env)
  env.update(item.decode().split('=',1) for item in policy.split(b'\0') if b'=' in item)
 env.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(out/'pipelines'))
 receipt={'backend':backend,'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),'environment':{k:v for k,v in env.items() if k.startswith(('GPU_PHYSICS_','WGPU_'))},'tests':[]}
 for index,name in enumerate(protocol['unique_tests'],1):
  cmd=[str(binary),name,'--exact','--nocapture','--test-threads=1']
  with (d/f'{index:02d}.stdout').open('w') as s,(d/f'{index:02d}.stderr').open('w') as e:
   p=subprocess.Popen(cmd,stdout=s,stderr=e,env=env);receipt['running']={'pid':p.pid,'test':name,'index':index};(d/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n');code=p.wait()
  receipt.pop('running');receipt['tests'].append({'test':name,'index':index,'exit':code});(d/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n');print(backend,index,name,code,flush=True)
  if code:raise SystemExit('Regression failure retained; stop')
