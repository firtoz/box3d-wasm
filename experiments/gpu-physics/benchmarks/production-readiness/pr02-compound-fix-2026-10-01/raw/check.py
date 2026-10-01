from pathlib import Path
import subprocess,json,hashlib,os,sys,tarfile
root=Path.cwd();out=root/'artifacts/production-readiness/pr02-compound-fix';old=root/'artifacts/production-readiness/pr02-diagnostics';p=json.loads((out/'protocol-before-runs.json').read_text());phase=sys.argv[1];assert phase in ['baseline','candidate'];sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest();r={'phase':phase,'status':'building','protocol_sha256':sha(out/'protocol-before-runs.json'),'builds':[],'runs':[],'pid':os.getpid(),'compiler':subprocess.check_output(['g++','--version'],text=True)};receipt=out/f'{phase}-receipt.json'
def save():receipt.write_text(json.dumps(r,indent=2)+'\n')
def fail(message):r.update(status='failed',failure=message);save();raise SystemExit(message)
def env(backend):
 e={k:v for k,v in os.environ.items() if not k.startswith(('GPU_','WGPU_'))}
 if backend=='native':
  raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],cwd=root,env=e);e.update(x.decode().split('=',1) for x in raw.split(b'\0') if b'=' in x)
 e.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(old/'pipelines'));return e
def inputs():
 files=[f for f in (root/'c_abi').glob('*') if f.is_file() and f.suffix in ['.c','.cpp','.h','.inc']];files += [f for f in (root.parents[1]/'box3d/include').rglob('*.h')];return {os.path.relpath(f,root):sha(f) for f in sorted(files)}
save();before=inputs();r['sources_before']=before
with tarfile.open(out/f'{phase}-fixture-inputs.tar.gz','w:gz') as archive:
 for name in before:archive.add(root/name,arcname=name.replace('../../box3d/','box3d/'))
assert before['c_abi/both_compound_ownership_test.cpp']==p['fixture_sha256']
if phase=='baseline':assert before['c_abi/both_dual.c']==p['baseline_dual_source_sha256'];names=['both_compound_ownership_test']
else:
 assert json.loads((out/'baseline-receipt.json').read_text())['status']=='pass'
 assert before['c_abi/both_dual.c']!=p['baseline_dual_source_sha256'];names=['both_compound_ownership_test']+p['regressions']
markers={'both_compound_ownership_test':'baseline reproduced:' if phase=='baseline' else 'combined compound ownership:', 'native_diagnostic_populations':'native diagnostic populations:', 'api_settings_test':'C API settings: pass','both_shape_replacement_test':'shape replacement:','both_substep_forces_test':'substep forces:'}
for backend in ['ordinary','native']:
 cfg=backend+'-both';build=old/f'c/{cfg}/cmake';archive=out/f'{phase}/{cfg}/libgpu_both_api.a'
 if phase=='baseline':assert sha(archive)==p['source_proof'][cfg]['baseline_archive_sha256']
 else:assert sha(archive)==json.loads((out/f'candidate/{cfg}/archive-build.json').read_text())['archive_sha256']
 linked=[archive,build/'libgpu_samples_api.a',old/f'candidate-complete-inputs/{backend}-frozen.a',build/'libbox3d_cpu.a',build/'box3d_src/libbox3d.a']
 for f in linked[1:]:assert sha(f)==p['source_proof'][cfg]['linked_inputs'][str(f)]
 for name in names:
  source=root/f'c_abi/{name}.cpp'
  if name!='both_compound_ownership_test':assert sha(source)==p['regression_source_hashes'][f'c_abi/{name}.cpp']
  dest=out/f'{phase}/{cfg}/{name}';dest.mkdir(exist_ok=False);binary=dest/'fixture';cmd=['g++','-O2','-std=c++17','-ffp-contract=off',str(source),'-DGPU_API_DUAL','-DGPU_POPULATIONS','-I',str(root.parents[1]/'box3d/include'),'-I',str(root/'c_abi'),'-Wl,--whole-archive',str(linked[0]),'-Wl,--no-whole-archive','-Wl,--whole-archive',str(linked[1]),'-Wl,--no-whole-archive']+[str(f) for f in linked[2:]]+['-ldl','-lpthread','-lm','-lgcc_s','-lGL','-o',str(binary)]
  with (dest/'build.log').open('w') as log:code=subprocess.run(cmd,cwd=root,stdout=log,stderr=subprocess.STDOUT).returncode
  r['builds'].append({'configuration':cfg,'fixture':name,'command':cmd,'linked_inputs':{str(f):sha(f) for f in linked},'exit':code,'log_sha256':sha(dest/'build.log'),'binary_sha256':sha(binary) if code==0 else None});save()
  if code:fail('Build failure retained: '+cfg+'/'+name)
  assert before==inputs()
r.update(status='running',sources_after=inputs());save()
if phase=='baseline':cases=[('ordinary','both_compound_ownership_test'),('native','both_compound_ownership_test')]
else:cases=[(b,'both_compound_ownership_test') for b in ['ordinary','native','native','ordinary']]+[(b,n) for n in p['regressions'] for b in ['ordinary','native']]
seen={}
for backend,name in cases:
 cfg=backend+'-both';dest=out/f'{phase}/{cfg}/{name}';key=(cfg,name);trial=seen.get(key,0)+1;seen[key]=trial;binary=dest/'fixture';cmd=[str(binary)]+(['--baseline'] if phase=='baseline' else []);e=env(backend);stdout=dest/f'{trial}-stdout.log';stderr=dest/f'{trial}-stderr.log'
 with stdout.open('w') as o,stderr.open('w') as err:
  proc=subprocess.Popen(cmd,cwd=root,env=e,stdout=o,stderr=err);r['running']={'pid':proc.pid,'configuration':cfg,'fixture':name,'trial':trial};save();code=proc.wait()
 r.pop('running');r['runs'].append({'configuration':cfg,'fixture':name,'trial':trial,'command':cmd,'exit':code,'binary_sha256':sha(binary),'stdout_sha256':sha(stdout),'stderr_sha256':sha(stderr),'environment':{k:v for k,v in e.items() if k.startswith(('GPU_','WGPU_'))}});save();print(phase,cfg,name,trial,code,flush=True)
 if code or markers[name] not in stdout.read_text():fail('Contract failure retained: '+cfg+'/'+name)
r['status']='pass';save();print(phase+' campaign complete',flush=True)
