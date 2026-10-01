from pathlib import Path
import hashlib,json,subprocess,os,tarfile,shlex
root=Path.cwd();out=root/'artifacts/production-readiness/pr02-compound-properties-viewer/build';out.mkdir(exist_ok=False);build=out/'cmake';lib=root/'artifacts/production-readiness/pr02-compound-properties-compile/candidate-complete-inputs/native-frozen.a';engine=lib.parent/'native-build.json';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();assert sha(lib)==json.loads(engine.read_text())['binary_sha256']
P=json.loads((out.parent/'protocol-before-build.json').read_text());assert json.loads((root/'artifacts/production-readiness/pr02-compound-properties-lifetime/receipt.json').read_text())['status']=='pass';assert sha(lib)==P['native_library_sha256']and sha(engine)==P['engine_receipt_sha256']
def inputs():
 files=[p for folder in ['c_abi','native-samples','scripts'] for p in (root/folder).rglob('*') if p.is_file() and p.suffix in ['.c','.cpp','.h','.inc','.py','.sh','.txt'] and '__pycache__' not in p.parts]
 files += [p for folder in ['src','include','samples','shared','extern'] for p in (root.parents[1]/'box3d'/folder).rglob('*') if p.is_file() and p.suffix in ['.c','.cpp','.h']]
 files += [p for dep in ['imgui-src','implot-src','nfd-src'] for p in (root.parents[1]/'box3d/.fetchcontent-cache'/dep).rglob('*') if p.is_file() and p.suffix in ['.c','.cpp','.h','.hpp'] and '.git' not in p.parts]
 return {os.path.relpath(p,root):sha(p) for p in sorted(set(files))}
r={'status':'building','scope':'one native combined CPU/GPU viewer for precommit compound material/property captures; build only, no physics/timing process','inputs_before':inputs(),'commands':[],'physics_library_sha256':sha(lib),'engine_receipt_sha256':sha(engine),'toolchain':{n:subprocess.check_output([n,'--version'],text=True) for n in ['cc','g++','cmake','ar']},'driver_pid':os.getpid(),'protocol_sha256':sha(out.parent/'protocol-before-build.json'),'applicability':P['scope_limit']}
def save():(out/'receipt.json').write_text(json.dumps(r,indent=2)+'\n')
save()
for i,cmd in enumerate([['cmake','-S',str(root/'native-samples'),'-B',str(build),'-DGPU_PHYSICS_LIB='+str(lib),'-DGPU_PORTABLE_API=ON','-DGPU_SAMPLES=OFF','-DBOTH_SAMPLES=ON','-DFETCHCONTENT_FULLY_DISCONNECTED=ON','-DCMAKE_BUILD_TYPE=Release'],['cmake','--build',str(build),'--target','samples_both','-j4']],1):
 log=out/f'build-{i}.log'
 with log.open('w') as f:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT).returncode
 r['commands'].append({'command':cmd,'exit':code,'log_sha256':sha(log)});save()
 if code:r['status']='failed';save();raise SystemExit(code)
assert r['inputs_before']==inputs()
units=[]
for c in json.loads((build/'compile_commands.json').read_text()):
 tokens=shlex.split(c['command']);obj=Path(c['directory'])/tokens[tokens.index('-o')+1]
 if obj.is_file():units.append({**c,'source_sha256':sha(Path(c['file'])),'object':str(obj),'object_sha256':sha(obj)})
with tarfile.open(out/'compiled-viewer-inputs.tar.gz','w:gz') as a:
 for n in r['inputs_before']:a.add(root/n,arcname=n.replace('../../box3d/','box3d/'))
 for p in build.rglob('*'):
  if p.is_file() and p.suffix in ['.c','.cpp','.h'] and 'CMakeFiles' not in p.parts and '_deps' not in p.parts:a.add(p,arcname='generated/'+str(p.relative_to(build)))
binary=build/'bin/samples_both';r.update(status='built',inputs_after=inputs(),compiled_units=units,linked_archives={str(p):sha(p) for p in build.rglob('*.a')},executable=str(binary),executable_sha256=sha(binary),link_command=(build/'CMakeFiles/samples_both.dir/link.txt').read_text(),source_archive_sha256=sha(out/'compiled-viewer-inputs.tar.gz'));r.pop('driver_pid',None);save();print('Native GPU viewer built',r['executable_sha256'],flush=True)
