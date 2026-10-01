import hashlib,importlib.util,json,os,shutil,subprocess,tarfile
from pathlib import Path
root=Path.cwd();base=root/'artifacts/production-readiness/pr02-diagnostics';out=base/'c';out.mkdir()
protocol=root/'benchmarks/production-readiness/pr02-diagnostics-2026-10-01/protocol.json'
fixture=root/'c_abi/native_diagnostics_fixture.cpp'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def save(p,v):p.write_text(json.dumps(v,indent=2)+'\n')
def logged(cmd,path):
 with path.open('w') as f:code=subprocess.run(cmd,cwd=root,stdout=f,stderr=subprocess.STDOUT).returncode
 save(path.with_suffix('.receipt.json'),{'command':cmd,'exit':code,'log_sha256':sha(path)})
 if code:raise SystemExit('Build failed, retained: '+str(path))
def sources():
 paths=list((root/'c_abi').glob('*'))+[root/p for p in ['scripts/gen-both-cpu.py','scripts/prepare-samples-link.py','native-samples/CMakeLists.txt']]
 box=root.parents[1]/'box3d'
 paths += [p for folder in ['src','include'] for p in (box/folder).rglob('*') if p.is_file() and p.suffix in ('.c','.h')]
 return {os.path.relpath(p,root):sha(p) for p in sorted(paths) if p.is_file()}
def environment(backend):
 env={k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_','WGPU_'))}
 if backend=='native':
  data=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],cwd=root,env=env)
  env.update(item.decode().split('=',1) for item in data.split(b'\0') if b'=' in item)
 env.update(WGPU_BACKEND='vulkan',GPU_PHYSICS_BACKEND='vulkan',GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(base/'pipelines'))
 return env
def link(result,library,build,both,baseline=False):
 exe=result/'fixture';cmd=['g++','-O2','-std=c++17',str(fixture),'-I',str(root.parents[1]/'box3d/include'),'-I',str(root/'c_abi')]
 inputs=[fixture,library,build/'libgpu_samples_api.a',build/'box3d_src/libbox3d.a']
 if baseline:cmd+=['-DDIAGNOSTIC_BASELINE']
 if both:
  cmd+=['-DGPU_API_DUAL','-Wl,--whole-archive',str(build/'libgpu_both_api.a'),'-Wl,--no-whole-archive'];inputs += [build/'libgpu_both_api.a',build/'libbox3d_cpu.a']
 cmd+=['-Wl,--whole-archive',str(build/'libgpu_samples_api.a'),'-Wl,--no-whole-archive',str(library)]
 if both:cmd.append(str(build/'libbox3d_cpu.a'))
 cmd += [str(build/'box3d_src/libbox3d.a'),'-ldl','-lpthread','-lm','-lgcc_s','-lGL','-o',str(exe)]
 logged(cmd,result/'link.log');return exe,{str(p):sha(p) for p in inputs}
def runs(result,receipt,exe,backend,n,baseline=False):
 env=environment(backend);receipt.update(binary_sha256=sha(exe),environment={k:v for k,v in env.items() if k.startswith(('GPU_','WGPU_'))},runs=[])
 save(result/'receipt.json',receipt)
 for trial in range(1,n+1):
  stdout=result/f'trial-{trial}.stdout';stderr=result/f'trial-{trial}.stderr'
  with stdout.open('w') as s,stderr.open('w') as e:
   proc=subprocess.Popen([str(exe)],cwd=root,env=env,stdout=s,stderr=e);receipt['running']={'pid':proc.pid,'trial':trial};save(result/'receipt.json',receipt);code=proc.wait()
  receipt.pop('running');receipt['runs'].append({'trial':trial,'exit':code,'stdout_sha256':sha(stdout),'stderr_sha256':sha(stderr)});save(result/'receipt.json',receipt);print(result.name,trial,code,flush=True)
  expected='baseline reproduced:' if baseline else 'native diagnostics C contract passed:'
  if code or expected not in stdout.read_text():raise SystemExit('Failed C trial retained; campaign stopped')
 receipt['status']='pass';save(result/'receipt.json',receipt)
# Freeze exact campaign source/protocol snapshots before launching its GPU runs.
shutil.copy2(fixture,out/'compiled-fixture.cpp');shutil.copy2(protocol,out/'protocol-before-runs.json')
for backend in ['ordinary','native']:
 result=out/('baseline-'+backend);result.mkdir()
 previous=root/'artifacts/production-readiness/pr02-api'/('unavailable-candidate' if backend=='ordinary' else 'unavailable-routing')/(backend+'-gpu')
 old=json.loads((previous/'receipt.json').read_text());lib=root/'artifacts/production-readiness/pr01-speculative/handoff'/(backend+'-frozen.a')
 build_receipt=root/'artifacts/production-readiness/pr01-speculative/handoff'/('build-'+backend+'.json')
 assert sha(lib)==json.loads(build_receipt.read_text())['binary_sha256']
 build=previous/'cmake';assert sha(build/'libgpu_samples_api.a')==old['linked_inputs'][str(build/'libgpu_samples_api.a')]
 exe,inputs=link(result,lib,build,False,True)
 receipt={'scope':'retained diagnostic baseline, no acceptance credit','compiled_adapter_receipt':str(previous/'receipt.json'),'compiled_adapter_receipt_sha256':sha(previous/'receipt.json'),'engine_build_receipt':str(build_receipt),'engine_build_receipt_sha256':sha(build_receipt),'linked_inputs':inputs,'protocol_sha256':sha(protocol)}
 shutil.copy2(previous/'receipt.json',result/'baseline-adapter-receipt.json');shutil.copy2(build_receipt,result/'baseline-engine-receipt.json')
 runs(result,receipt,exe,backend,1,True)
for configuration in json.loads(protocol.read_text())['budget']['candidate_C_configurations']:
 backend,linkage=configuration.split('-');both=linkage=='both';result=out/configuration;result.mkdir();before=sources()
 lib=base/'candidate-complete-inputs'/(backend+'-frozen.a');build_receipt=base/'candidate-complete-inputs'/(backend+'-build.json');engine=json.loads(build_receipt.read_text());assert sha(lib)==engine['binary_sha256']
 build=result/'cmake';logged(['cmake','-S',str(root/'native-samples'),'-B',str(build),'-DGPU_PHYSICS_LIB='+str(lib),'-DGPU_PORTABLE_API=ON','-DGPU_SAMPLES='+('OFF' if both else 'ON'),'-DBOTH_SAMPLES='+('ON' if both else 'OFF'),'-DFETCHCONTENT_FULLY_DISCONNECTED=ON','-DCMAKE_BUILD_TYPE=Release'],result/'configure.log')
 targets=['gpu_samples_api','box3d'];targets=(['gpu_both_api']+targets) if both else targets
 logged(['cmake','--build',str(build),'--target',*targets,'-j4'],result/'build.log')
 generated=list((build/'api-sources').glob('*.c'))+([build/'both_passthrough.c'] if both else [])
 if both:
  spec=importlib.util.spec_from_file_location('generator',root/'scripts/gen-both-cpu.py');generator=importlib.util.module_from_spec(spec);spec.loader.exec_module(generator)
  import re
  definitions=set(re.findall(r'B3_API\s+[^;{]*?\b(b3\w+)\([^;]*?\)\s*\{',generated[-1].read_text(),re.S))
  assert not definitions & (generator.UNAVAILABLE|generator.SHARED_GPU_API)
 exe,inputs=link(result,lib,build,both)
 assert sources()==before,'adapter inputs changed during build'
 with tarfile.open(result/'compiled-adapter-sources.tar.gz','w:gz') as archive:
  for name in before:archive.add(root/name,arcname=name.replace('../../box3d/','box3d/'))
  for source in generated:archive.add(source,arcname='generated/'+str(source.relative_to(build)))
 receipt={'configuration':configuration,'scope':'focused linked C diagnostics, not full physical release qualification','engine_build_receipt_sha256':sha(build_receipt),'adapter_sources_before':before,'adapter_sources_after':sources(),'generated_sources':{str(p.relative_to(build)):sha(p) for p in generated},'compile_commands':json.loads((build/'compile_commands.json').read_text()),'linked_inputs':inputs,'protocol_sha256':sha(protocol)}
 runs(result,receipt,exe,backend,2)
