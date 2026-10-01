import hashlib,json,os,re,subprocess,tarfile
from pathlib import Path
root=Path.cwd();base=root/'artifacts/production-readiness/pr02-diagnostics';out=base/'viewers';out.mkdir()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources(build):
 paths=[p for folder in ['src','shaders','c_abi','native-samples','scripts'] for p in (root/folder).rglob('*') if p.is_file() and not any(x.startswith('build-') or x=='__pycache__' for x in p.relative_to(root).parts) and p.suffix in ('.rs','.wgsl','.c','.cpp','.h','.py','.sh','.txt','.inc')]
 box=root.parents[1]/'box3d';paths += [p for folder in ['src','include','samples','extern'] for p in (box/folder).rglob('*') if p.is_file() and p.suffix in ('.c','.cpp','.h')]
 return {os.path.relpath(p,root):sha(p) for p in sorted(set(paths))}
for configuration in ['ordinary-gpu','ordinary-both','native-gpu','native-both']:
 build=base/'c'/configuration/'cmake';result=out/configuration;result.mkdir();before=sources(build)
 target='samples_both' if configuration.endswith('both') else 'samples_gpu';cmd=['cmake','--build',str(build),'--target',target,'-j4']
 receipt={'configuration':configuration,'status':'building','command':cmd,'inputs_before':before,'physics_library_receipt_sha256':sha(base/'candidate-complete-inputs'/(configuration.split('-')[0]+'-build.json'))}
 def save():(result/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
 save()
 with (result/'build.log').open('w') as log:code=subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT).returncode
 receipt.update(exit=code,log_sha256=sha(result/'build.log'));save()
 if code:raise SystemExit('viewer build failed: '+configuration)
 assert sources(build)==before,'source changed during viewer build'
 executable=build/'bin'/target;commands=json.loads((build/'compile_commands.json').read_text());files=sorted({Path(c['file']) for c in commands if Path(c['file']).is_file()})
 generated={os.path.relpath(p,build):sha(p) for p in files if p.is_relative_to(build)}
 receipt.update(status='built',inputs_after=sources(build),executable=str(executable),executable_sha256=sha(executable),compile_commands=commands,compiled_translation_units={str(p):sha(p) for p in files},generated_sources=generated)
 with tarfile.open(result/'compiled-viewer-sources.tar.gz','w:gz') as archive:
  for name in before:archive.add(root/name,arcname=name.replace('../../box3d/','box3d/'))
  for p in files:
   if p.is_relative_to(build):archive.add(p,arcname='generated/'+str(p.relative_to(build)))
 # Verify the post-campaign CMake additions did not change tested adapter code.
 c_receipt=json.loads((base/'c'/configuration/'receipt.json').read_text())
 for name,expected in c_receipt['generated_sources'].items():assert sha(build/name)==expected
 receipt['campaign_adapter_sources_unchanged']=True;save();print(configuration,'built',flush=True)
