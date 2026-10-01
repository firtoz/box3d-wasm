from pathlib import Path
import subprocess,hashlib,json,shutil,os
root=Path.cwd();out=root/'artifacts/production-readiness/pr01-speculative/handoff';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
scenes=['single-box','box-stack','sphere-stack','capsule-stack','revolute','weld','stack','pyramid','bounce','mixed','spinner','ramp','spheres','dominoes','high-resistance','mixed-stacks','falling-cubes','mixed-topology','anchored-mechanisms','joint-chain']
protocol=out/'recording-protocol.json'
assert not protocol.exists()
protocol.write_text(json.dumps({'scope':'Pre-commit visual evidence, no performance claims','scenes':scenes,'standard_frames':300,'native_scene':'Issues/s&box Ghost Collisions','native_steps':300,'native_repetitions':{'cpu':1,'gpu-native':1},'order':'real CPU first, then GPU; no competing GPU jobs','settings':'scene defaults, dt1/60,4substeps,1280x720. GPU ordering explicitly enabled. Native clip paced,health scan separate from timing.','capture_repair':'Only failed capture may be replaced; retain failures. A physics failure stops acceptance.'},indent=2)+'\n')
previous=root/'recordings/snapshots/000-box3d-cpu'; archive=out/'previous-cpu-column'
if previous.exists():shutil.copytree(previous,archive)
for tag,command,binary in [('renderer',['cargo','build','--release','--bin','gpu-physics'],root/'target/release/gpu-physics'),('oracle-config',['cmake','-S','oracle','-B',str(out/'oracle-build'),'-DCMAKE_BUILD_TYPE=Release'],None),('oracle',['cmake','--build',str(out/'oracle-build'),'--clean-first','--target','box3d_oracle','-j4'],out/'oracle-build/box3d_oracle')]:
 paths=[]
 for directory in ['src','shaders','compiler/native-backend','oracle']:
  paths.extend(p for p in (root/directory).rglob('*') if p.is_file() and '/build' not in str(p))
 paths.extend(root/p for p in ['Cargo.toml','Cargo.lock','build.rs'])
 paths.extend(root.parent.parent/'box3d'/p for p in subprocess.check_output(['git','-C','../../box3d','ls-files'],text=True).splitlines() if (root.parent.parent/'box3d'/p).is_file())
 inputs={os.path.relpath(p,root):sha(p) for p in paths}
 log=out/f'recording-build-{tag}.log'
 with log.open('w') as f:code=subprocess.run(command,stdout=f,stderr=subprocess.STDOUT).returncode
 assert inputs=={os.path.relpath(p,root):sha(p) for p in paths}
 receipt={'command':command,'exit':code,'source_inputs':inputs,'binary_sha256':sha(binary) if binary and binary.exists() else None,'log_sha256':sha(log),'revision_at_build':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'box3d_revision':subprocess.check_output(['git','-C','../../box3d','rev-parse','HEAD'],text=True).strip(),'toolchains':{x:subprocess.check_output([x,'--version'],text=True).splitlines()[0] for x in ['rustc','cargo','cmake','c++']}}
 (out/f'recording-build-{tag}.json').write_text(json.dumps(receipt,indent=2)+'\n');print(tag,code,flush=True)
 if code:raise SystemExit(code)
