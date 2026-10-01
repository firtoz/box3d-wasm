from pathlib import Path
import subprocess,os,json,time,hashlib
root=Path.cwd();out=root/'artifacts/production-readiness/pr01-speculative/handoff';receipt=out/'standard-recordings.json';assert json.loads(receipt.read_text())['runs']==[]
receipt=out/'standard-recordings-resume.json'
assert not receipt.exists()
d={'pid':os.getpid(),'status':'resuming-unstarted-standard-captures','runs':[]};receipt.write_text(json.dumps(d,indent=2)+'\n')
# Resume only the unstarted standard capture budget; no diagnostic rerun.
protocol=json.loads((out/'recording-protocol.json').read_text());scenes=protocol['scenes'];regular=[s for s in scenes if s!='mixed-topology']
env={k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_','WGPU_','GPU_RECORD_'))}
env.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(out/'pipelines'),GPU_RECORD_BIN=str(root/'target/release/gpu-physics'),GPU_RECORD_ORACLE=str(out/'oracle-build/box3d_oracle'),FRAMES='300',SKIP_METRICS='1',METRIC_RUNS='1')
for engine,script,label in [('cpu','scripts/record-box3d-oracle.sh','000-box3d-cpu'),('gpu','scripts/record-snapshot.sh','2026-10-01-speculative-handoff')]:
 for group,selected,view in [('regular',regular,None),('mixed',['mixed-topology'],'-220,290,-300,95,8,145')]:
  e=env.copy();e['RECORD_SCENES']=' '.join(selected)
  if view:e['GPU_RECORD_VIEW']=view
  command=['bash',script,label];log=out/f'record-standard-{engine}-{group}.log'
  d.update(status='running',current=f'{engine}-{group}');receipt.write_text(json.dumps(d,indent=2)+'\n')
  with log.open('w') as f:code=subprocess.run(command,env=e,stdout=f,stderr=subprocess.STDOUT).returncode
  d['runs'].append({'engine':engine,'scenes':selected,'frames':300,'view':view,'command':command,'exit':code,'environment':{k:v for k,v in e.items() if k.startswith(('GPU_','WGPU_','VK_'))},'log_sha256':hashlib.sha256(log.read_bytes()).hexdigest()});receipt.write_text(json.dumps(d,indent=2)+'\n')
  if code:raise SystemExit(code)
  print(engine,group,'complete',flush=True)
d.update(status='complete');d.pop('current',None);d.pop('pid',None);receipt.write_text(json.dumps(d,indent=2)+'\n')
