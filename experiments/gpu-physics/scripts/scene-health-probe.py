#!/usr/bin/env python3
"""Run already-built CPU/GPU viewers; correctness probes, not performance trials."""
from pathlib import Path
import subprocess,os,json,time,hashlib,sys
root=Path(__file__).resolve().parent.parent
out=Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=False)
env=os.environ.copy();env.update(__NV_PRIME_RENDER_OFFLOAD='1',__GLX_VENDOR_LIBRARY_NAME='nvidia',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json')
env.pop('GPU_PHYSICS_AB',None)
for backend in ['cpu','gpu']:
 for scene in ['bounce','switch']:
  binary=root/f'native-samples/build-{backend}/bin/samples_{backend}'
  before=hashlib.sha256(binary.read_bytes()).hexdigest()
  args=['timeout','240','xvfb-run','-a',str(binary),'--sample-name','Continuous/Bounce House' if scene=='bounce' else 'Compound/Village','--unpaced','--health-scan','--warmup','0','--timed','120' if scene=='bounce' else '28','--bench-json',str(out/f'{backend}-{scene}.json')]
  if scene=='switch':args+=['--switch-sample-name','Continuous/Bounce House','--switch-after','8']
  start=time.monotonic()
  with (out/f'{backend}-{scene}.log').open('w') as log:r=subprocess.run(args,cwd=root/'../../box3d',env=env,stdout=log,stderr=subprocess.STDOUT)
  result={'exit':r.returncode,'seconds':time.monotonic()-start,'command':args,'binary_sha256':before,'diagnostic_large_compound_import':False}
  (out/f'{backend}-{scene}-process.json').write_text(json.dumps(result,indent=2)+'\n')
  assert before==hashlib.sha256(binary.read_bytes()).hexdigest(), 'binary changed during run'
  assert r.returncode==0,result
  doc=json.loads((out/f'{backend}-{scene}.json').read_text())
  assert doc.get('status')=='ok',doc.get('status')
  print(backend,scene,'complete',flush=True)
subprocess.run([sys.executable,str(root/'scripts/validate-scene-switch.py'),str(out)],check=True)
subprocess.run([sys.executable,str(root/'scripts/check-scene-health-controls.py'),str(out)],check=True)
