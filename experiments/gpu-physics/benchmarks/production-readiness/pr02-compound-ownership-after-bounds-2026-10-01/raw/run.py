from pathlib import Path
import hashlib,json,subprocess,tarfile,os,shlex,signal
ROOT=Path.cwd();OUT=ROOT/'artifacts/production-readiness/pr02-compound-ownership-after-bounds'
sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
P=json.loads((OUT/'protocol.json').read_text());C=json.loads((OUT/'candidate-inputs.json').read_text())
assert C['protocol_sha256']==sha(OUT/'protocol.json')
assert sha(ROOT/P['production_path'])==C['candidate_source_sha256']
assert not (OUT/'receipt.json').exists(), 'Never rerun a closed or interrupted driver'
R={'status':'building','protocol_sha256':sha(OUT/'protocol.json'),'candidate_inputs_sha256':sha(OUT/'candidate-inputs.json'),'builds':[],'runs':[],'driver_pid':os.getpid(),'toolchain':{n:subprocess.check_output([n,'--version'],text=True) for n in ['cc','g++','cmake','ar']},'adapter':subprocess.check_output(['nvidia-smi','--query-gpu=name,uuid,driver_version,pci.bus_id','--format=csv'],text=True)}
def save():(OUT/'receipt.json').write_text(json.dumps(R,indent=2)+'\n')
def sources():
 files=[f for f in (ROOT/'c_abi').glob('*')if f.is_file()]
 files += [ROOT/n for n in ['scripts/gen-both-cpu.py','scripts/prepare-samples-link.py','native-samples/CMakeLists.txt']]
 files += [f for folder in ['src','include']for f in (ROOT.parents[1]/'box3d'/folder).rglob('*')if f.is_file()and f.suffix in ['.c','.h']]
 return {os.path.relpath(f,ROOT):sha(f) for f in sorted(files)}
def archive_inputs(destination, before, generated=()):
 with tarfile.open(destination,'w:gz')as t:
  for n in before:t.add(ROOT/n,arcname=n.replace('../../box3d/','box3d/'))
  for f,rel in generated:t.add(f,arcname='generated/'+rel)
def logged(cmd,log,**extra):
 with log.open('w')as f:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT).returncode
 entry={'command':cmd,'exit':code,'log_sha256':sha(log),**extra};R['builds'].append(entry);save()
 if code:raise RuntimeError('Build failed: '+str(log))
 return entry
def environment(backend):
 e={k:v for k,v in os.environ.items()if not k.startswith(('GPU_','WGPU_','VK_'))}
 if backend=='native':
  raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],env=e)
  e.update(x.decode().split('=',1)for x in raw.split(b'\0')if x.startswith((b'GPU_',b'WGPU_')))
 e.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(ROOT/'artifacts/production-readiness/pr02-diagnostics/pipelines'))
 return e
markers={'both_compound_ownership_test':'combined compound ownership:', 'native_diagnostic_populations':'native diagnostic populations:', 'api_settings_test':'C API settings: pass','both_shape_replacement_test':'shape replacement:','both_substep_forces_test':'substep forces:','compound_aabb_contract':'compound AABB contract:'}
cases=[(b,'both_compound_ownership_test')for b in ['ordinary','native','native','ordinary']]+[(b,n)for n in P['regressions']for b in ['ordinary','native']]
try:
 before=sources();R['sources_before']=before;save()
 assert before['c_abi/both_compound_ownership_test.cpp']==P['ownership_fixture_sha256']
 for n,h in P['regressions'].items():assert before[f'c_abi/{n}.cpp']==h,n
 assert sha(ROOT/'scripts/native-samples-cache-env.sh')==P['runtime_env_source_sha256']
 for backend,proof in P['engine_proof'].items():
  assert sha(ROOT/proof['library'])==proof['library_sha256'] and sha(ROOT/proof['receipt'])==proof['receipt_sha256']
  d=json.loads((ROOT/proof['receipt']).read_text());assert d['engine_sources']==d['engine_sources_after']
  for n,h in d['engine_sources'].items():assert sha(ROOT/n)==h,n
 archive_inputs(OUT/'candidate-fixture-inputs.tar.gz',before)
 binaries={}
 for backend in ['ordinary','native']:
  cfg=backend+'-both';d=OUT/cfg;d.mkdir();build=d/'cmake';lib=ROOT/P['engine_proof'][backend]['library']
  logged(['cmake','-S',str(ROOT/'native-samples'),'-B',str(build),'-DGPU_PHYSICS_LIB='+str(lib),'-DGPU_PORTABLE_API=ON','-DGPU_SAMPLES=OFF','-DBOTH_SAMPLES=ON','-DFETCHCONTENT_FULLY_DISCONNECTED=ON','-DCMAKE_BUILD_TYPE=Release'],d/'configure.log',configuration=cfg)
  logged(['cmake','--build',str(build),'--target','gpu_both_api','gpu_samples_api','box3d','-j4'],d/'build.log',configuration=cfg)
  assert sources()==before, 'Compiled source changed during build'
  linked=[build/'libgpu_both_api.a',build/'libgpu_samples_api.a',lib,build/'libbox3d_cpu.a',build/'box3d_src/libbox3d.a']
  generated=[f for f in (build/'api-sources').glob('*')if f.is_file()]+[build/'both_passthrough.c']
  units=[]
  for unit in json.loads((build/'compile_commands.json').read_text()):
   tok=shlex.split(unit['command']);obj=Path(unit['directory'])/tok[tok.index('-o')+1]
   if obj.is_file():units.append({**unit,'source_sha256':sha(unit['file']),'object':str(obj),'object_sha256':sha(obj)})
  generated.extend(Path(u['file'])for u in units if str(Path(u['file'])).startswith(str(build)) and Path(u['file'])not in generated)
  archive_inputs(d/'compiled-adapter-inputs.tar.gz',before,[(f,str(f.relative_to(build)))for f in generated])
  adapter={'configuration':cfg,'sources_before':before,'sources_after':sources(),'generated_sources':{str(f.relative_to(build)):sha(f)for f in generated},'compiled_units':units,'linked_inputs':{str(f):sha(f)for f in linked},'source_archive_sha256':sha(d/'compiled-adapter-inputs.tar.gz'),'engine_proof':P['engine_proof'][backend]}
  (d/'adapter-receipt.json').write_text(json.dumps(adapter,indent=2)+'\n')
  for name in ['both_compound_ownership_test']+list(P['regressions']):
   source=ROOT/f'c_abi/{name}.cpp';dest=d/name;dest.mkdir();binary=dest/'fixture'
   cmd=['g++','-O2','-std=c++17','-ffp-contract=off',str(source),'-DGPU_API_DUAL','-DGPU_POPULATIONS','-I',str(ROOT.parents[1]/'box3d/include'),'-I',str(ROOT/'c_abi'),'-Wl,--whole-archive',str(linked[0]),'-Wl,--no-whole-archive','-Wl,--whole-archive',str(linked[1]),'-Wl,--no-whole-archive']+[str(f)for f in linked[2:]]+['-ldl','-lpthread','-lm','-lgcc_s','-lGL','-o',str(binary)]
   entry=logged(cmd,dest/'build.log',configuration=cfg,fixture=name,linked_inputs=adapter['linked_inputs']);entry['binary_sha256']=sha(binary);save();binaries[backend,name]=binary
 R.update(status='running',sources_after=sources());save()
 assert R['sources_after']==before
 seen={}
 for index,(backend,name)in enumerate(cases):
  binary=binaries[backend,name];dest=binary.parent;trial=seen.get((backend,name),0)+1;seen[backend,name]=trial;e=environment(backend)
  stdout=dest/f'{trial}-stdout.log';stderr=dest/f'{trial}-stderr.log'
  with stdout.open('w')as o,stderr.open('w')as err:
   proc=subprocess.Popen([str(binary)],env=e,stdout=o,stderr=err,start_new_session=True);R['running']={'pid':proc.pid,'backend':backend,'fixture':name,'trial':trial};save()
   try:code=proc.wait(timeout=P['watchdog_seconds_per_process'])
   except subprocess.TimeoutExpired:
    os.killpg(proc.pid,signal.SIGTERM);proc.wait(timeout=30);code=-999
  R.pop('running',None)
  row={'backend':backend,'fixture':name,'trial':trial,'command':[str(binary)],'binary_sha256':sha(binary),'exit':code,'stdout_path':str(stdout.relative_to(OUT)),'stderr_path':str(stderr.relative_to(OUT)),'stdout_sha256':sha(stdout),'stderr_sha256':sha(stderr),'environment':{k:v for k,v in e.items()if k.startswith(('GPU_','WGPU_','VK_'))}};R['runs'].append(row);save();print(backend,name,trial,code,flush=True)
  assert sources()==before
  if code or markers[name]not in stdout.read_text():raise RuntimeError('Contract failed: '+backend+'/'+name)
 R['status']='pass'
except BaseException as exc:
 R.update(status='stopped',failure=str(exc),unlaunched=cases[len(R['runs']):])
 source=ROOT/P['production_path']
 if sha(source)==C['candidate_source_sha256']:
  source.write_bytes((OUT/'baseline-both_dual.c').read_bytes());R['production_restored_sha256']=sha(source)
 else:R['restore_note']='Unexpected production edits preserved; restoration requires source review'
 raise
finally:
 R.pop('driver_pid',None);R.pop('running',None);save()
assert sha(OUT/'protocol.json')==C['protocol_sha256']
print('Ownership candidate first validation complete:14 processes pass; no timing or final readiness acceptance',flush=True)
