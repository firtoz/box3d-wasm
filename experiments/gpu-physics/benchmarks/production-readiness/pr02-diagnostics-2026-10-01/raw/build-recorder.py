import hashlib,json,subprocess,tarfile,shutil
from pathlib import Path
root=Path.cwd();base=root/'artifacts/production-readiness/pr02-diagnostics';out=base/'recording-build';out.mkdir()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
previous=json.loads((base/'candidate-complete-inputs/ordinary-build.json').read_text())
names=sorted(set(previous['engine_sources']) | {'c_abi/shim.c'})
def inputs():return {name:sha(root/name) for name in names}
before=inputs();receipt={'scope':'GPU recording app; separate from diagnostics test libraries and timing','inputs_before':before,'commands':[],'status':'building'}
def save():(out/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
save()
with tarfile.open(out/'compiled-inputs.tar.gz','w:gz') as archive:
 for name in names:archive.add(root/name,arcname=name.replace('../../box3d/','box3d/'))
cmd=['cargo','build','--release','--features','replay-diagnostics','--target-dir','target/samples']
with (out/'build.log').open('w') as log:code=subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT).returncode
receipt['commands'].append({'command':cmd,'exit':code,'log_sha256':sha(out/'build.log')});save()
if code:raise SystemExit('Recording app build failed, retained')
assert inputs()==before,'recording source changed during build'
binary=root/'target/samples/release/gpu-physics';shutil.copy2(binary,out/'gpu-physics-frozen')
receipt.update(status='built',inputs_after=inputs(),binary_sha256=sha(binary),binary=str(out/'gpu-physics-frozen'));save();print('recording app built',receipt['binary_sha256'],flush=True)
