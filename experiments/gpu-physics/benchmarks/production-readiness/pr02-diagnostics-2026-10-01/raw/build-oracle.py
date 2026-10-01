import hashlib,json,os,subprocess,shutil,tarfile
from pathlib import Path
root=Path.cwd();out=root/'artifacts/production-readiness/pr02-diagnostics/recording-build';receipt_file=out/'oracle-receipt.json';assert not receipt_file.exists()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
box=root.parents[1]/'box3d';paths=[p for directory in [root/'oracle',box/'include',box/'src'] for p in directory.rglob('*') if p.is_file() and not any(x=='build' for x in p.parts) and (p.suffix in ('.c','.cpp','.h') or p.name=='CMakeLists.txt')];paths.append(box/'CMakeLists.txt')
def inputs():return {os.path.relpath(p,root):sha(p) for p in sorted(paths)}
before=inputs();receipt={'status':'building','inputs_before':before,'commands':[]}
def save():receipt_file.write_text(json.dumps(receipt,indent=2)+'\n')
save();build=out/'oracle-cmake'
for i,cmd in enumerate([['cmake','-S',str(root/'oracle'),'-B',str(build),'-DCMAKE_BUILD_TYPE=Release','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'],['cmake','--build',str(build),'--target','box3d_oracle','-j4']],1):
 log=out/f'oracle-build-{i}.log'
 with log.open('w') as f:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT).returncode
 receipt['commands'].append({'command':cmd,'exit':code,'log_sha256':sha(log)});save()
 if code:raise SystemExit('CPU oracle build failed, retained')
assert inputs()==before
binary=build/'box3d_oracle';shutil.copy2(binary,out/'box3d-oracle-frozen')
with tarfile.open(out/'oracle-inputs.tar.gz','w:gz') as archive:
 for name in before:archive.add(root/name,arcname=name.replace('../../box3d/','box3d/'))
receipt.update(status='built',inputs_after=inputs(),binary_sha256=sha(binary),binary=str(out/'box3d-oracle-frozen'),compile_commands=json.loads((build/'compile_commands.json').read_text()));save();print('CPU oracle built',receipt['binary_sha256'],flush=True)
