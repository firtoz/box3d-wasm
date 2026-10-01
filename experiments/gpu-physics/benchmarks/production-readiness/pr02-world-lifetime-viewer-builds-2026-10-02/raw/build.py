from pathlib import Path
import hashlib,json,subprocess,os,tarfile,shlex,shutil,time
root=Path.cwd();out=root/'artifacts/production-readiness/pr02-world-lifetime-viewer-builds';box=root.parents[1]/'box3d'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
protocol=out/'protocol-before-builds.json';r={'status':'building','pid':os.getpid(),'revision':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'protocol_sha256':sha(protocol),'libraries':{},'viewers':{},'commands':[],'started_unix':time.time()}
assert r['revision'].startswith(json.loads(protocol.read_text())['prerequisite_revision'])
def save():(out/'receipt.json').write_text(json.dumps(r,indent=2)+'\n')
def run(cmd,dest):
 dest.parent.mkdir(parents=True,exist_ok=True)
 with dest.open('w')as log:code=subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT).returncode
 r['commands'].append({'command':cmd,'exit':code,'log':str(dest.relative_to(out)),'log_sha256':sha(dest)});save()
 assert code==0,f'command failed: {dest}'
def inputs():
 files=[p for folder in ['src','shaders','c_abi','native-samples','scripts','patches']for p in(root/folder).rglob('*')if p.is_file()and '__pycache__'not in p.parts and p.suffix not in ['.o','.a'] and not any(part=='build' or part.startswith('build-') for part in p.relative_to(root).parts) and p.suffix in ['.rs','.wgsl','.c','.cpp','.h','.hpp','.py','.sh','.ts','.toml','.txt','.patch','.json','.bin']]
 files += [root/p for p in ['Cargo.toml','Cargo.lock','build.rs']]
 files += [p for folder in ['src','include','samples','shared','extern']for p in(box/folder).rglob('*')if p.is_file()and p.suffix in ['.c','.cpp','.h','.hpp']]
 files += [p for dep in ['imgui-src','implot-src','nfd-src']for p in(box/'.fetchcontent-cache'/dep).rglob('*')if p.is_file()and p.suffix in ['.c','.cpp','.h','.hpp']and '.git'not in p.parts]
 return {os.path.relpath(p,root):sha(p)for p in sorted(set(files))}
def archive(mapping,dest):
 with tarfile.open(dest,'w:gz')as a:
  for n in mapping:a.add(root/n,arcname=n.replace('../../box3d/','box3d/'))
save()
try:
 before=inputs();r['inputs_before']=before;r['toolchain']={n:subprocess.check_output([n,'--version'],text=True)for n in ['rustc','cargo','cc','g++','cmake','ar']};archive(before,out/'source-inputs.tar.gz');save()
 r['libraries']=json.loads(protocol.read_text())['libraries']
 for v in r['libraries'].values():assert sha(Path(v['path']))==v['sha256']
 for cell in ['ordinary-gpu','native-gpu','ordinary-both','native-both']:
  folder=out/cell;folder.mkdir(exist_ok=False);build=folder/'cmake';gpu=cell.endswith('-gpu');both=cell.endswith('-both');target='samples_both'if both else'samples_gpu'if gpu else'samples_cpu'
  cmd=['cmake','-S',str(root/'native-samples'),'-B',str(build),'-DGPU_PORTABLE_API='+('ON'if gpu or both else'OFF'),'-DGPU_SAMPLES='+('ON'if gpu else'OFF'),'-DBOTH_SAMPLES='+('ON'if both else'OFF'),'-DFETCHCONTENT_FULLY_DISCONNECTED=ON','-DCMAKE_BUILD_TYPE=Release']
  if gpu or both:cmd+=['-DGPU_PHYSICS_LIB='+r['libraries'][cell.split('-')[0]]['path']]
  run(cmd,folder/'configure.log')
  inject=['python3','scripts/inject-sokol-bench.py']
  for src,dst,flag in [('main.cpp','main_bench.cpp',''),('sample.cpp','sample_bench.cpp','sample-'),('sample_continuous.cpp','sample_continuous_bench.cpp','continuous-'),('sample_character.cpp','sample_character_loading.cpp','character-'),('sample_joint.cpp','sample_joint_bench.cpp','joint-')]:inject+=['--'+flag+'src',str(box/'samples'/src),'--'+flag+'dst',str(build/dst)]
  if gpu or both:inject+=['--gpu-sidebar']
  run(inject,folder/'generation.log');main=build/'main_bench.cpp';original=main.read_text();(folder/'main.original.cpp').write_text(original)
  assert original.count('static SampleContext s_context;')==1 and original.count('\ts_context.sample->Step();\n')==1
  shutil.copy2(out/'observer.inc',build/'controls_observer.inc')
  modified=original.replace('static SampleContext s_context;','static SampleContext s_context;\n#include "controls_observer.inc"').replace('\ts_context.sample->Step();\n','\ts_context.sample->Step();\n\tcontrols_observe(s_context, s_frame);\n');main.write_text(modified);(folder/'main.observed.cpp').write_text(modified)
  run(['cmake','--build',str(build),'--target',target,'-j4'],folder/'build.log');assert main.read_text()==modified,'generation overwrote observer';assert before==inputs()
  units=[]
  for c in json.loads((build/'compile_commands.json').read_text()):
   tokens=shlex.split(c['command']);obj=Path(c['directory'])/tokens[tokens.index('-o')+1]
   if obj.is_file():units.append({**c,'source_sha256':sha(Path(c['file'])),'object':str(obj),'object_sha256':sha(obj)})
  with tarfile.open(folder/'generated-inputs.tar.gz','w:gz')as a:
   for p in build.rglob('*'):
    if p.is_file()and p.suffix in ['.c','.cpp','.h','.inc']and 'CMakeFiles'not in p.parts and '_deps'not in p.parts:a.add(p,arcname=str(p.relative_to(build)))
  exe=build/'bin'/target
  v={'status':'built','executable':str(exe),'executable_sha256':sha(exe),'compiled_units':units,'linked_archives':{str(p):sha(p)for p in build.rglob('*.a')},'link_command':(build/f'CMakeFiles/{target}.dir/link.txt').read_text(),'original_main_sha256':sha(folder/'main.original.cpp'),'observed_main_sha256':sha(main),'observer_sha256':sha(out/'observer.inc'),'generation_sha256':sha(folder/'generated-inputs.tar.gz'),'cached_object_disclosure':'NFD uses shared FetchContent build; inventory includes reused object. Not a clean PR06 build.'}
  (folder/'receipt.json').write_text(json.dumps(v,indent=2)+'\n');r['viewers'][cell]={'receipt_sha256':sha(folder/'receipt.json'),'executable_sha256':sha(exe)};save();print(cell,'viewer',sha(exe),flush=True)
 r.update(status='built',inputs_after=inputs(),finished_unix=time.time(),source_archive_sha256=sha(out/'source-inputs.tar.gz'));r.pop('pid');save()
except BaseException as e:
 r.update(status='failed',failure=repr(e),inputs_after=inputs(),finished_unix=time.time());r.pop('pid',None);save();raise
