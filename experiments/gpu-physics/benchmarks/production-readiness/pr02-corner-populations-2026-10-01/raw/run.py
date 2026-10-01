from pathlib import Path
import subprocess,hashlib,json,os,tarfile
root=Path.cwd();out=root/'artifacts/production-readiness/pr02-corner-populations';old=root/'artifacts/production-readiness/pr02-diagnostics';protocol=root/'benchmarks/production-readiness/pr02-corner-populations-2026-10-01/protocol.json';p=json.loads(protocol.read_text());sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
r={'status':'building','protocol_sha256':sha(protocol),'budget':p['budget'],'builds':[],'runs':[],'runner_pid':os.getpid(),'toolchain':subprocess.check_output(['g++','--version'],text=True)}
def save():(out/'receipt.json').write_text(json.dumps(r,indent=2)+'\n')
def fail(message):r.update(status='failed',failure=message);r.pop('running',None);save();raise SystemExit(message)
def inputs():
 files=[root/'c_abi/native_diagnostic_populations.cpp',root/'c_abi/native_diagnostics.h'];files += sorted((root.parents[1]/'box3d/include').rglob('*.h'));return {os.path.relpath(f,root):sha(f) for f in files}
def environment(backend):
 e={k:v for k,v in os.environ.items() if not k.startswith(('GPU_','WGPU_'))}
 if backend=='native':
  raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],cwd=root,env=e);e.update(x.decode().split('=',1) for x in raw.split(b'\0') if b'=' in x)
 e.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(old/'pipelines'));return e
before=inputs();assert before['c_abi/native_diagnostic_populations.cpp']==p['fixture_sha256'];r['inputs_before']=before;save()
with tarfile.open(out/'fixture-inputs.tar.gz','w:gz') as tar:
 for name in before:tar.add(root/name,arcname=name.replace('../../box3d/','box3d/'))
for cfg in p['order']:
 dest=out/cfg;dest.mkdir();binary=dest/'fixture';cmd=['g++','-O2','-std=c++17','-ffp-contract=off',str(root/'c_abi/native_diagnostic_populations.cpp'),'-I',str(root.parents[1]/'box3d/include'),'-I',str(root/'c_abi')];linked={}
 if cfg=='cpu':
  lib=root/'artifacts/production-readiness/pr01-speculative/cpu-cmake/box3d_src/libbox3d.a';assert sha(lib)==p['source_proof']['CPU_library_sha256'];cmd.append(str(lib));linked[str(lib)]=sha(lib)
 else:
  backend,linkage=cfg.split('-');both=linkage=='both';build=old/f'c/{cfg}/cmake';lib=old/f'candidate-complete-inputs/{backend}-frozen.a';cmd.append('-DGPU_POPULATIONS')
  files=[build/'libgpu_samples_api.a',lib,build/'box3d_src/libbox3d.a']
  if both:
   cmd+=['-DGPU_API_DUAL','-Wl,--whole-archive',str(build/'libgpu_both_api.a'),'-Wl,--no-whole-archive'];files += [build/'libgpu_both_api.a',build/'libbox3d_cpu.a']
  cmd+=['-Wl,--whole-archive',str(build/'libgpu_samples_api.a'),'-Wl,--no-whole-archive',str(lib)]
  if both:cmd.append(str(build/'libbox3d_cpu.a'))
  cmd.append(str(build/'box3d_src/libbox3d.a'));linked={str(f):sha(f) for f in files};assert all(p['source_proof'][cfg]['linked_inputs'][n]==h for n,h in linked.items())
 cmd+=['-ldl','-lpthread','-lm','-lgcc_s','-lGL','-o',str(binary)]
 with (dest/'build.log').open('w') as log:code=subprocess.run(cmd,cwd=root,stdout=log,stderr=subprocess.STDOUT).returncode
 r['builds'].append({'configuration':cfg,'command':cmd,'linked_inputs':linked,'exit':code,'log_sha256':sha(dest/'build.log'),'binary_sha256':sha(binary) if code==0 else None});save()
 if code:fail('Build failure; retained; no GPU trials launched: '+cfg)
 assert before==inputs()
r.update(status='running',inputs_after=inputs());save()
for cfg in p['order']:
 dest=out/cfg;binary=dest/'fixture';e=environment(cfg.split('-')[0])
 with (dest/'stdout.log').open('w') as stdout,(dest/'stderr.log').open('w') as stderr:
  proc=subprocess.Popen([str(binary)],cwd=root,env=e,stdout=stdout,stderr=stderr);r['running']={'pid':proc.pid,'configuration':cfg};save();code=proc.wait()
 r.pop('running');r['runs'].append({'configuration':cfg,'command':[str(binary)],'exit':code,'binary_sha256':sha(binary),'stdout_sha256':sha(dest/'stdout.log'),'stderr_sha256':sha(dest/'stderr.log'),'environment':{k:v for k,v in e.items() if k.startswith(('GPU_','WGPU_'))}});save();print(cfg,code,flush=True)
 if code or 'native diagnostic populations: sensor/compound/mesh contract passed' not in (dest/'stdout.log').read_text():fail('Contract failure retained; remaining cases unlaunched: '+cfg)
r['status']='pass';save();print('Population campaign passed; viewer qualification remains open',flush=True)
