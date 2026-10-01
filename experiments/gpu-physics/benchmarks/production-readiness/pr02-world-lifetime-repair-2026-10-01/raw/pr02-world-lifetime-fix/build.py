from pathlib import Path
import os,json,hashlib,subprocess,time,shutil
R=Path.cwd();A=R/'artifacts/production-readiness/pr02-world-lifetime-fix';P=json.loads((A/'protocol-before-change.json').read_text());H=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();inputs=json.loads((A/'candidate-inputs.json').read_text());assert not(A/'build-receipt.json').exists()
D={'status':'building','pid':os.getpid(),'source_revision':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'protocol_sha256':H(A/'protocol-before-change.json'),'inputs_before':inputs,'toolchain':subprocess.check_output(['rustc','-Vv'],text=True),'builds':[],'libraries':{},'tests':{},'started':time.time()}
def save():(A/'build-receipt.json').write_text(json.dumps(D,indent=2)+'\n')
def check():assert all(H(R/f)==h for f,h in inputs.items())
recipes=[('ordinary-lib',['cargo','build','--release','--lib','--features','external-c-shim,replay-diagnostics','--target-dir','target/samples','--message-format=json']),('ordinary-tests',['cargo','test','--release','--no-run','--lib','--features','external-c-shim,replay-diagnostics','--target-dir','target/samples','--message-format=json']),('native-lib',['bash','scripts/build-native-cache.sh','build','--release','--lib','--features','external-c-shim,replay-diagnostics','--message-format=json']),('native-tests',['bash','scripts/build-native-cache.sh','test','--release','--no-run','--lib','--features','external-c-shim,replay-diagnostics','--message-format=json'])]
save()
try:
 for name,command in recipes:
  check();folder=A/name;folder.mkdir()
  with(folder/'build.log').open('w') as out:
   code=subprocess.run(command,stdout=out,stderr=subprocess.STDOUT).returncode
  D['builds'].append({'name':name,'command':command,'exit':code,'log_sha256':H(folder/'build.log')});save();assert code==0,(name,code)
  check();records=[]
  for line in(folder/'build.log').read_text().splitlines():
   try:records.append(json.loads(line))
   except ValueError:pass
  artifacts=[x for x in records if x.get('reason')=='compiler-artifact' and x['target']['name']=='gpu_physics'];assert artifacts
  backend=name.split('-')[0]
  if name.endswith('lib'):
   files=[Path(f) for x in artifacts for f in x['filenames'] if f.endswith('.a')];assert len(files)==1
   dest=A/(backend+'-frozen.a');shutil.copy2(files[0],dest);D['libraries'][backend]={'path':str(dest),'sha256':H(dest),'features':artifacts[-1]['features']}
  else:
   files=[Path(x['executable']) for x in artifacts if x.get('executable')];assert len(files)==1
   dest=A/(backend+'-tests');shutil.copy2(files[0],dest);D['tests'][backend]={'path':str(dest),'sha256':H(dest),'features':artifacts[-1]['features']}
  save();print(name,'built',flush=True)
 D.update(status='built',finished=time.time(),inputs_after={f:H(R/f) for f in inputs});D.pop('pid');save()
except BaseException as e:
 D.update(status='stopped',failure=repr(e),finished=time.time());D.pop('pid',None);save()
 for f,h in P['baseline'].items(): assert H(A/'baseline'/f)==h;shutil.copy2(A/'baseline'/f,R/f)
 new=R/'src/api/world/world_lifetime.rs';shutil.copy2(new,A/'rejected-world-lifetime.rs');new.unlink();D['production_restored']=True;save();raise
