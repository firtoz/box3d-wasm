from pathlib import Path
import subprocess,hashlib,json,os,tarfile,shutil
root=Path.cwd();out=root/'artifacts/production-readiness/pr02-supported';old=root/'artifacts/production-readiness/pr02-diagnostics';protocol=root/'benchmarks/production-readiness/pr02-supported-2026-10-01/protocol.json';p=json.loads(protocol.read_text());sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
receipt={'protocol_sha256':sha(protocol),'status':'building','C_runs':[],'Rust_runs':[],'builds':[],'budget':p['budget']}
def save():(out/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
def env(backend):
 e={k:v for k,v in os.environ.items() if not k.startswith(('GPU_','WGPU_'))}
 if backend=='native':
  raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],cwd=root,env=e);e.update(x.decode().split('=',1) for x in raw.split(b'\0') if b'=' in x)
 e.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(old/'pipelines'));return e
def fail(message):receipt.update(status='failed',failure=message);save();raise SystemExit(message)
def snapshot():
 paths=[f for f in (root/'c_abi').glob('*') if f.is_file() and f.suffix in ['.cpp','.c','.h','.inc']]
 paths += [f for f in (root.parents[1]/'box3d/include').rglob('*.h')]
 return {os.path.relpath(f,root):sha(f) for f in sorted(paths)}
save();before=snapshot()
with tarfile.open(out/'compiled-fixture-inputs.tar.gz','w:gz') as archive:
 for name in before:archive.add(root/name,arcname=name.replace('../../box3d/','box3d/'))
receipt['fixture_inputs_before']=before;save()
markers={'api_settings_test':'C API settings: pass','warm_start_fixture':'"timing_only":false','both_substep_forces_test':'substep forces:','both_shape_replacement_test':'shape replacement:','both_joint_reaction_test':'joint reaction:','both_joint_separation_test':'joint separation:'}
for case in p['C_cases']:
 cfg=case['configuration'];backend,linkage=cfg.split('-');both=linkage=='both';name=case['fixture'];build=old/f'c/{cfg}/cmake';result=out/f'C/{cfg}/{name}';result.mkdir(parents=True);lib=old/f'candidate-complete-inputs/{backend}-frozen.a'
 previous=json.loads((old/f'c/{cfg}/receipt.json').read_text());linked=[lib,build/'libgpu_samples_api.a',build/'box3d_src/libbox3d.a']+([build/'libgpu_both_api.a',build/'libbox3d_cpu.a'] if both else [])
 for f in linked:assert sha(f)==previous['linked_inputs'][str(f)],f
 source=root/f'c_abi/{name}.cpp';assert sha(source)==p['fixture_hashes'][f'c_abi/{name}.cpp'];binary=result/'fixture'
 cmd=['g++','-O2','-std=c++17',str(source),'-I',str(root.parents[1]/'box3d/include'),'-I',str(root/'c_abi')]
 if both:cmd+=['-DGPU_API_DUAL','-Wl,--whole-archive',str(build/'libgpu_both_api.a'),'-Wl,--no-whole-archive']
 cmd += ['-Wl,--whole-archive',str(build/'libgpu_samples_api.a'),'-Wl,--no-whole-archive',str(lib)]
 if both:cmd.append(str(build/'libbox3d_cpu.a'))
 cmd += [str(build/'box3d_src/libbox3d.a'),'-ldl','-lpthread','-lm','-lgcc_s','-lGL','-o',str(binary)]
 with (result/'build.log').open('w') as log:code=subprocess.run(cmd,cwd=root,stdout=log,stderr=subprocess.STDOUT).returncode
 receipt['builds'].append({'configuration':cfg,'fixture':name,'command':cmd,'exit':code,'log_sha256':sha(result/'build.log'),'linked_inputs':{str(f):sha(f) for f in linked},'adapter_receipt_sha256':sha(old/f'c/{cfg}/receipt.json'),'binary_sha256':sha(binary) if code==0 else None});save()
 if code:fail('C link failure retained: '+cfg+'/'+name)
 assert before==snapshot()
receipt.update(fixture_inputs_after=snapshot(),status='running');save()
for case in p['C_cases']:
 cfg=case['configuration'];name=case['fixture'];backend=cfg.split('-')[0];result=out/f'C/{cfg}/{name}';binary=result/'fixture';e=env(backend)
 with (result/'stdout.log').open('w') as stdout,(result/'stderr.log').open('w') as stderr:
  proc=subprocess.Popen([str(binary)],cwd=root,env=e,stdout=stdout,stderr=stderr);receipt['running']={'pid':proc.pid,'configuration':cfg,'fixture':name};save();code=proc.wait()
 receipt.pop('running');receipt['C_runs'].append({'configuration':cfg,'fixture':name,'exit':code,'binary_sha256':sha(binary),'stdout_sha256':sha(result/'stdout.log'),'stderr_sha256':sha(result/'stderr.log'),'environment':{k:v for k,v in e.items() if k.startswith(('GPU_','WGPU_'))}});save();print(cfg,name,code,flush=True)
 if code or markers[name] not in (result/'stdout.log').read_text():fail('C contract failure retained: '+cfg+'/'+name)
for i,selector in enumerate(p['Rust_selectors'],1):
 for backend in ['ordinary','native']:
  binary=old/f'candidate-complete-inputs/{backend}-tests';assert sha(binary)==p['source_proof'][backend]['test_sha256'];e=env(backend);log=out/f'{backend}-Rust-{i}.log';cmd=[str(binary),selector,'--exact','--nocapture','--test-threads=1']
  with log.open('w') as f:
   proc=subprocess.Popen(cmd,cwd=root,env=e,stdout=f,stderr=subprocess.STDOUT);receipt['running']={'pid':proc.pid,'configuration':backend,'selector':selector};save();code=proc.wait()
  receipt.pop('running');receipt['Rust_runs'].append({'configuration':backend,'selector':selector,'command':cmd,'exit':code,'binary_sha256':sha(binary),'log_sha256':sha(log),'environment':{k:v for k,v in e.items() if k.startswith(('GPU_','WGPU_'))}});save();print(backend,selector,code,flush=True)
  if code or '1 passed; 0 failed' not in log.read_text():fail('Rust contract failure retained: '+backend+'/'+selector)
receipt['status']='pass';save();print('Focused API campaign passes; remaining PR02 population/UI contracts open',flush=True)
