from pathlib import Path
import subprocess,hashlib,json,tarfile,shutil
root=Path.cwd();out=root/'artifacts/production-readiness/pr02-compound-fix';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
p=json.loads((out/'protocol-before-runs.json').read_text())
def sources():
 files=[f for f in (root/'c_abi').glob('*') if f.suffix in ('.c','.h','.inc')]+list((root.parents[1]/'box3d/include').rglob('*.h'))
 return {str(f):sha(f) for f in sorted(files)}
before=sources()
assert before[str(root/'c_abi/both_dual.c')]!=p['baseline_dual_source_sha256']
with tarfile.open(out/'candidate-adapter-inputs.tar.gz','w:gz') as a:
 for name in before:
  f=Path(name);a.add(f,arcname=('box3d/'+str(f.relative_to(root.parents[1]/'box3d'))) if 'box3d/include/' in name else str(f.relative_to(root)))
for cfg in ['ordinary-both','native-both']:
 d=out/'candidate'/cfg;d.mkdir(parents=True,exist_ok=False);base=out/'baseline'/cfg/'libgpu_both_api.a';archive=d/'libgpu_both_api.a';obj=d/'both_dual.c.o'
 assert sha(base)==p['source_proof'][cfg]['baseline_archive_sha256']
 members=subprocess.check_output(['ar','t',str(base)],text=True).splitlines();assert members==['both_map.c.o','both_dual.c.o','both_passthrough.c.o']
 member_sha=lambda ar,n:hashlib.sha256(subprocess.check_output(['ar','p',str(ar),n])).hexdigest()
 r={'configuration':cfg,'status':'building','compiler':subprocess.check_output(['cc','--version'],text=True),'archiver':subprocess.check_output(['ar','--version'],text=True),'sources_before':before,'source_archive_sha256':sha(out/'candidate-adapter-inputs.tar.gz'),'baseline_archive_sha256':sha(base),'members_before':{n:member_sha(base,n) for n in members},'commands':[]}
 receipt=d/'archive-build.json';receipt.write_text(json.dumps(r,indent=2)+'\n')
 commands=[['cc','-O3','-DNDEBUG','-std=gnu17','-w','-I',str(root.parents[1]/'box3d/include'),'-I',str(root/'c_abi'),'-c',str(root/'c_abi/both_dual.c'),'-o',str(obj)]]
 shutil.copy2(base,archive)
 commands += [['ar','rcs',str(archive),str(obj)],['ranlib',str(archive)]]
 with (d/'archive-build.log').open('w') as log:
  for cmd in commands:
   code=subprocess.run(cmd,cwd=root,stdout=log,stderr=subprocess.STDOUT).returncode;r['commands'].append({'command':cmd,'exit':code});receipt.write_text(json.dumps(r,indent=2)+'\n')
   if code:r['status']='failed';receipt.write_text(json.dumps(r,indent=2)+'\n');raise SystemExit('Archive build failed; stop campaign')
 assert before==sources()
 r.update(status='pass',sources_after=sources(),object_sha256=sha(obj),archive_sha256=sha(archive),members_after={n:member_sha(archive,n) for n in members},log_sha256=sha(d/'archive-build.log'))
 for n in ['both_map.c.o','both_passthrough.c.o']:assert r['members_before'][n]==r['members_after'][n]
 assert r['members_after']['both_dual.c.o']==r['object_sha256']
 receipt.write_text(json.dumps(r,indent=2)+'\n');print(cfg,r['archive_sha256'],flush=True)
