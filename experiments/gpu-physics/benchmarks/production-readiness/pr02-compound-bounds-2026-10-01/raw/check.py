from pathlib import Path
import hashlib,json,subprocess,tarfile,os
root=Path.cwd();out=root/'artifacts/production-readiness/pr02-compound-bounds';old=root/'artifacts/production-readiness/pr02-diagnostics';proof=root/'artifacts/production-readiness/pr02-compound-fix/baseline-receipt.json';p=json.loads((out/'protocol-before-runs.json').read_text());prev=json.loads(proof.read_text());sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
def sources():
 files=[f for f in (root/'c_abi').glob('*') if f.is_file() and f.suffix in ('.c','.cpp','.h','.inc')]+list((root.parents[1]/'box3d/include').rglob('*.h'))
 return {os.path.relpath(f,root):sha(f) for f in sorted(files)}
r={'status':'building','protocol_sha256':sha(out/'protocol-before-runs.json'),'compiler':subprocess.check_output(['g++','--version'],text=True),'builds':[],'runs':[],'sources_before':sources()}
def save():(out/'receipt.json').write_text(json.dumps(r,indent=2)+'\n')
def fail(msg):r.update(status='failed',failure=msg);save();raise SystemExit(msg)
save();assert r['sources_before']['c_abi/compound_bounds_diagnostic.cpp']==p['fixture_sha256'];assert r['sources_before']['c_abi/both_dual.c']==p['baseline_dual_source_sha256']
with tarfile.open(out/'fixture-inputs.tar.gz','w:gz') as a:
 for n in r['sources_before']:a.add(root/n,arcname=n.replace('../../box3d/','box3d/'))
for cfg in p['order']:
 prior=next(b for b in prev['builds'] if b['configuration']==cfg);cmd=prior['command'].copy();dest=out/cfg;dest.mkdir();cmd[4]=str(root/'c_abi/compound_bounds_diagnostic.cpp');cmd[-1]=str(dest/'fixture')
 for name,h in prior['linked_inputs'].items():assert sha(Path(name))==h
 with (dest/'build.log').open('w') as log:code=subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT).returncode
 r['builds'].append({'configuration':cfg,'command':cmd,'exit':code,'binary_sha256':sha(dest/'fixture') if code==0 else None,'linked_inputs':prior['linked_inputs'],'log_sha256':sha(dest/'build.log')});save()
 if code:fail('Build failure '+cfg)
 assert r['sources_before']==sources()
r.update(status='running',sources_after=sources());save()
for cfg in p['order']:
 prior=next(x for x in prev['runs'] if x['configuration']==cfg);e={k:v for k,v in os.environ.items() if not k.startswith(('GPU_','WGPU_'))};e.update(prior['environment']);d=out/cfg;cmd=[str(d/'fixture')]
 with (d/'stdout.log').open('w') as o,(d/'stderr.log').open('w') as err:
  proc=subprocess.Popen(cmd,env=e,stdout=o,stderr=err);r['running']={'pid':proc.pid,'configuration':cfg};save();code=proc.wait()
 r.pop('running');r['runs'].append({'configuration':cfg,'command':cmd,'environment':prior['environment'],'exit':code,'binary_sha256':sha(d/'fixture'),'stdout_sha256':sha(d/'stdout.log'),'stderr_sha256':sha(d/'stderr.log')});save();print(cfg,code,flush=True)
 if code or 'eight cases complete; observed GPU discrepancies are not acceptance' not in (d/'stdout.log').read_text():fail('Diagnostic failure '+cfg)
r['status']='diagnosis-complete';save();print('Bounds diagnosis complete; GPU discrepancies retained, no acceptance.',flush=True)
