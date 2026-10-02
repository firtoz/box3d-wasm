from pathlib import Path
import json,hashlib,subprocess,shutil,os,tarfile,shlex,time
root=Path.cwd();out=root/'artifacts/production-readiness/pr02-world-lifetime-capture-build';sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();protocol=out/'protocol-before-build.json';p=json.loads(protocol.read_text());box=root.parents[1]/'box3d'
files=[root/n for n in p['candidate_rust_inputs']]
files += [q for folder in [root/'c_abi',box/'src',box/'include']for q in folder.rglob('*')if q.is_file()and q.suffix in ['.c','.cpp','.h','.hpp']]
files=sorted(set(files));inputs=lambda:{os.path.relpath(q,root):sha(q)for q in files};before=inputs();r={'status':'building','pid':os.getpid(),'protocol_sha256':sha(protocol),'inputs_before':before,'commands':[],'features':p['features'],'source_revision_context_only':p['invocation_revision_context_only'],'started_unix':time.time(),'toolchain':subprocess.check_output(['rustc','-Vv'],text=True)}
def save():(out/'receipt.json').write_text(json.dumps(r,indent=2)+'\n')
save()
try:
 for n,h in p['candidate_rust_inputs'].items():assert before[n]==h,n
 with tarfile.open(out/'compiled-inputs.tar.gz','w:gz')as archive:
  for n in before:archive.add(root/n,arcname=n.replace('../../box3d/','box3d/'))
 env=os.environ.copy();env['CC_ENABLE_DEBUG_OUTPUT']='1';r['build_environment']={k:v for k,v in env.items()if k.startswith(('CARGO_','RUST','CC','CFLAGS','CXX','AR'))};save()
 with (out/'build.log').open('w')as log:code=subprocess.run(p['command'],env=env,stdout=log,stderr=subprocess.STDOUT).returncode
 r['commands'].append({'command':p['command'],'exit':code,'log_sha256':sha(out/'build.log')});save();assert code==0,'first recorder compile failed'
 assert inputs()==before,'source mutation during compile'
 events=[]
 for line in(out/'build.log').read_text().splitlines():
  try:o=json.loads(line)
  except json.JSONDecodeError:continue
  if o.get('reason')=='compiler-artifact':events.append(o)
 artifacts=[x for x in events if x['target']['name']=='gpu-physics'and x['target']['kind']==['bin']];assert len(artifacts)==1;artifact=artifacts[0];exe=Path(artifact['executable']);assert 'replay-diagnostics'in artifact['features']and not artifact['fresh'],'first current recorder must actually compile'
 frozen=out/'gpu-physics-frozen';shutil.copy2(exe,frozen);assert sha(exe)==sha(frozen)
 dep=root/'target/samples/release/gpu-physics.d';shutil.copy2(dep,out/'gpu-physics.d');content=dep.read_text();assert 'src/api/world/world_lifetime.rs'in content
 compiled=[]
 for token in shlex.split(content.replace('\\\n',' ').split(':',1)[1]):
  q=Path(token)
  if q.is_file()and q in files:compiled.append({'source':os.path.relpath(q,root),'sha256':sha(q)})
 c=[]
 for folder in (root/'target/samples/release/build').glob('gpu-physics-*/out'):
  for q in sorted(folder.iterdir()):
   if q.is_file()and q.suffix in ['.a','.o']:c.append({'path':str(q),'sha256':sha(q)})
 assert c; r.update(status='built',inputs_after=inputs(),binary=str(frozen),binary_sha256=sha(frozen),compiler_artifact=artifact,compiler_artifacts=events,local_source_depfile_audit=compiled,c_helpers=c,depfile_sha256=sha(dep),source_archive_sha256=sha(out/'compiled-inputs.tar.gz'),finished_unix=time.time(),cached_helper_disclosure='C helper/root compile commands are retained through CC_ENABLE_DEBUG_OUTPUT; other feature-fingerprint helper objects are inventoried but not asserted linked. Cached dependencies are not PR06 clean-build evidence.');r.pop('pid');save();print('First current-source recorder built',r['binary_sha256'],len(compiled),'source depfile entries',flush=True)
except BaseException as e:
 r.update(status='stopped',failure=repr(e),inputs_after=inputs(),finished_unix=time.time());r.pop('pid',None);save();raise
