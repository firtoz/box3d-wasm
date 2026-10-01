import hashlib,json,subprocess,shutil,tarfile,sys
from pathlib import Path
root=Path.cwd();out=root/'artifacts/production-readiness/pr02-compound-aabb-compile/candidate-complete-inputs';backend=sys.argv[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 paths=[p for folder in ['src','shaders'] for p in (root/folder).rglob('*') if p.is_file()]
 paths += [root/p for p in ['Cargo.toml','Cargo.lock','build.rs','scripts/build-native-cache.sh','scripts/prepare-native-backend.py']]
 paths+=list((root/'patches').glob('*')) if (root/'patches').exists() else []
 box=root.parents[1]/'box3d'
 paths += [p for folder in ['include','src'] for p in (box/folder).rglob('*') if p.is_file() and p.suffix in ('.h','.c')]
 paths += [p for p in (root/'c_abi').glob('*.h')]
 paths += list((root/'patches').rglob('*')) if (root/'patches').exists() else []
 return {__import__('os').path.relpath(p,root):sha(p) for p in sorted(paths) if p.is_file()}
if backend=='native':
 assert json.loads((out/'ordinary-build.json').read_text())['status']=='built'
out.mkdir(parents=True,exist_ok=True)
before=inputs();receipt=out/(backend+'-build.json');assert not receipt.exists()
r={'revision_at_build':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'engine_sources':before,'backend':backend,'toolchain':subprocess.check_output(['rustc','-Vv'],text=True),'commands':[],'status':'building'}
def save():receipt.write_text(json.dumps(r,indent=2)+'\n')
save()
with tarfile.open(out/(backend+'-compiled-sources.tar.gz'),'w:gz') as archive:
 for name in before:archive.add(root/name,arcname=name.replace('../../box3d/','box3d/'))
commands=([['cargo','build','--release','--lib','--features','external-c-shim,replay-diagnostics','--target-dir','target/samples'],['cargo','test','--release','--lib','--features','external-c-shim,replay-diagnostics','--target-dir','target/samples','--no-run']] if backend=='ordinary' else [['bash','scripts/build-native-cache.sh','build','--release','--lib','--features','external-c-shim,replay-diagnostics'],['bash','scripts/build-native-cache.sh','test','--release','--lib','--features','external-c-shim,replay-diagnostics','--no-run']])
for number,cmd in enumerate(commands,1):
 log=out/f'{backend}-build-{number}.log'
 with log.open('w') as f:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT).returncode
 r['commands'].append({'command':cmd,'exit':code,'log_sha256':sha(log)});save()
 if code:
  r['status']='failed';save();raise SystemExit('Build failed: '+str(log))
assert inputs()==before,'compiled source changed during build'
base=root/('target/samples' if backend=='ordinary' else 'target/native-cache-build')/'release'
library=base/'libgpu_physics.a';shutil.copy2(library,out/(backend+'-frozen.a'))
log=(out/f'{backend}-build-2.log').read_text();import re
match=re.search(r'Executable unittests src/lib.rs \((.+)\)',log);assert match,log[-1000:]
test=Path(match[1]);test=test if test.is_absolute() else root/test
assert test.is_file();shutil.copy2(test,out/(backend+'-tests'))
r.update(binary_sha256=sha(library),test_binary_sha256=sha(test),engine_sources_after=inputs(),status='built')
if backend=='native':
 r['native_configuration']={__import__('os').path.relpath(p,root):sha(p) for p in (root/'target/native-backend').glob('*') if p.is_file()}
save();print(backend,'built',r['binary_sha256'],flush=True)
