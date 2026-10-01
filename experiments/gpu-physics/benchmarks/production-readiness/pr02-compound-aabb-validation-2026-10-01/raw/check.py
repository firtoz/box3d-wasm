from pathlib import Path
import hashlib,json,subprocess,tarfile,os
root=Path.cwd();out=root/'artifacts/production-readiness/pr02-compound-aabb-validation';pre=root/'artifacts/production-readiness/pr02-compound-aabb-compile/candidate-complete-inputs';p=json.loads((out/'protocol-before-runs.json').read_text());sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
def sources():
 files=[f for f in (root/'c_abi').glob('*') if f.is_file()]+[root/n for n in ['scripts/gen-both-cpu.py','scripts/prepare-samples-link.py','native-samples/CMakeLists.txt']]+[f for folder in ['src','include'] for f in (root.parents[1]/'box3d'/folder).rglob('*') if f.is_file() and f.suffix in ['.c','.h']]
 return {os.path.relpath(f,root):sha(f) for f in sorted(files)}
r={'status':'building','protocol_sha256':sha(out/'protocol-before-runs.json'),'builds':[],'runs':[],'C_sources_before':sources(),'compiler':subprocess.check_output(['g++','--version'],text=True),'driver_adapter':subprocess.check_output(['nvidia-smi','--query-gpu=name,driver_version,pci.bus_id','--format=csv,noheader'],text=True)}
def save():(out/'receipt.json').write_text(json.dumps(r,indent=2)+'\n')
def fail(msg):r.update(status='failed',failure=msg);save();raise SystemExit(msg)
def logged(cmd,log):
 with log.open('w') as f:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT).returncode
 entry={'command':cmd,'exit':code,'log_sha256':sha(log)};r['builds'].append(entry);save()
 if code:fail('Build failure: '+str(log))
 return entry
save()
for n,h in p['production_sources'].items():assert sha(root/n)==h
assert r['C_sources_before']['c_abi/compound_aabb_contract.cpp']==p['fixture_sha256']
with tarfile.open(out/'C-fixture-inputs.tar.gz','w:gz') as a:
 for n in r['C_sources_before']:a.add(root/n,arcname=n.replace('../../box3d/','box3d/'))
binaries={}
for backend in ['ordinary','native']:
 cfg=backend+'-both';d=out/cfg;d.mkdir();build=d/'cmake';lib=pre/(backend+'-frozen.a');assert sha(lib)==p['engine_proof'][backend]['library_sha256']
 logged(['cmake','-S',str(root/'native-samples'),'-B',str(build),'-DGPU_PHYSICS_LIB='+str(lib),'-DGPU_PORTABLE_API=ON','-DGPU_SAMPLES=OFF','-DBOTH_SAMPLES=ON','-DFETCHCONTENT_FULLY_DISCONNECTED=ON','-DCMAKE_BUILD_TYPE=Release'],d/'configure.log')
 logged(['cmake','--build',str(build),'--target','gpu_both_api','gpu_samples_api','box3d','-j4'],d/'build.log')
 generated=list((build/'api-sources').glob('*.c'))+[build/'both_passthrough.c']
 with tarfile.open(d/'compiled-adapter-inputs.tar.gz','w:gz') as a:
  for n in r['C_sources_before']:a.add(root/n,arcname=n.replace('../../box3d/','box3d/'))
  for f in generated:a.add(f,arcname='generated/'+str(f.relative_to(build)))
 linked=[build/'libgpu_both_api.a',build/'libgpu_samples_api.a',lib,build/'libbox3d_cpu.a',build/'box3d_src/libbox3d.a']
 for name in ['compound_aabb_contract']+list(p['C_regressions']):
  source=root/f'c_abi/{name}.cpp';dest=d/name;dest.mkdir();binary=dest/'fixture'
  if name!='compound_aabb_contract':assert sha(source)==p['C_regressions'][name]
  cmd=['g++','-O2','-std=c++17','-ffp-contract=off',str(source),'-DGPU_API_DUAL','-I',str(root.parents[1]/'box3d/include'),'-I',str(root/'c_abi'),'-Wl,--whole-archive',str(linked[0]),'-Wl,--no-whole-archive','-Wl,--whole-archive',str(linked[1]),'-Wl,--no-whole-archive']+[str(f) for f in linked[2:]]+['-ldl','-lpthread','-lm','-lgcc_s','-lGL','-o',str(binary)]
  b=logged(cmd,dest/'build.log');b.update(configuration=cfg,fixture=name,binary_sha256=sha(binary),linked_inputs={str(f):sha(f) for f in linked});save();binaries[backend,name]=binary
 assert sources()==r['C_sources_before']
 (d/'adapter-receipt.json').write_text(json.dumps({'configuration':cfg,'C_sources_before':r['C_sources_before'],'C_sources_after':sources(),'generated_sources':{str(f.relative_to(build)):sha(f) for f in generated},'compile_commands':json.loads((build/'compile_commands.json').read_text()),'linked_inputs':{str(f):sha(f) for f in linked},'engine_receipt_sha256':p['engine_proof'][backend]['receipt_sha256']},indent=2)+'\n')
r.update(status='running',C_sources_after=sources());save()
previous=json.loads((root/'artifacts/production-readiness/pr02-compound-fix/baseline-receipt.json').read_text());seen={}
cases=[('C',b,'compound_aabb_contract') for b in ['ordinary','native','native','ordinary']]+[('C',b,n) for n in p['C_regressions'] for b in ['ordinary','native']]+[('Rust',b,n) for n in p['Rust_suites'] for b in ['ordinary','native']]
for kind,backend,name in cases:
 e={k:v for k,v in os.environ.items() if not k.startswith(('GPU_','WGPU_'))};e.update(next(x['environment'] for x in previous['runs'] if x['configuration']==backend+'-both'))
 key=(kind,backend,name);trial=seen.get(key,0)+1;seen[key]=trial
 if kind=='C':binary=binaries[backend,name];d=binary.parent;cmd=[str(binary)];count=None
 else:
  binary=pre/(backend+'-tests');assert sha(binary)==p['engine_proof'][backend]['test_sha256'];d=out/'rust'/(backend+'-'+('query' if name.endswith('::') else 'diagnostics'));d.mkdir(parents=True,exist_ok=False)
  listing=subprocess.check_output([str(binary),'--list'],text=True);count=sum(line.startswith(name) and line.endswith(': test') for line in listing.splitlines());assert count>0
  cmd=[str(binary),name,'--nocapture','--test-threads=1']+([] if name.endswith('::') else ['--exact'])
 stdout=d/f'{trial}-stdout.log';stderr=d/f'{trial}-stderr.log'
 with stdout.open('w') as o,stderr.open('w') as err:
  proc=subprocess.Popen(cmd,env=e,stdout=o,stderr=err);r['running']={'pid':proc.pid,'kind':kind,'backend':backend,'selector':name,'trial':trial};save();code=proc.wait()
 r.pop('running');entry={'kind':kind,'backend':backend,'selector':name,'trial':trial,'command':cmd,'environment':{k:v for k,v in e.items() if k.startswith(('GPU_','WGPU_'))},'exit':code,'binary_sha256':sha(binary),'stdout_path':str(stdout.relative_to(out)),'stderr_path':str(stderr.relative_to(out)),'stdout_sha256':sha(stdout),'stderr_sha256':sha(stderr),'expected_test_count':count};r['runs'].append(entry);save();print(kind,backend,name,trial,code,flush=True)
 text=stdout.read_text()
 marker=(f'{count} passed; 0 failed; 0 ignored' if kind=='Rust' else {'compound_aabb_contract':'compound AABB contract:','api_settings_test':'C API settings: pass','both_shape_replacement_test':'shape replacement:'}[name])
 if code or marker not in text:fail('Contract failure retained: '+str(key))
r['status']='pass';save();print('Compound AABB API validation complete',flush=True)
