from pathlib import Path
import subprocess,os,json,time,hashlib,shutil
root=Path.cwd();out=root/'artifacts/production-readiness/pr02-diagnostics';receipt=out/'standard-recordings.json';assert not receipt.exists()
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
protocol=json.loads((out/'recording-protocol.json').read_text())
for name,h in protocol['binaries'].items():assert sha(out/name)==h
old=root/'recordings/snapshots/000-box3d-cpu';archive=out/'previous-cpu-column';assert old.exists() and not archive.exists()
archive_index={p.name:sha(p) for p in old.iterdir() if p.is_file()};shutil.move(str(old),archive)
(out/'previous-cpu-column.json').write_text(json.dumps(archive_index,indent=2)+'\n')
scenes=protocol['scenes'];regular=[s for s in scenes if s!='mixed-topology']
env={k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_','WGPU_','GPU_RECORD_'))}
env.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(out/'pipelines'),GPU_RECORD_BIN=str(out/'recording-build/gpu-physics-frozen'),GPU_RECORD_ORACLE=str(out/'recording-build/box3d-oracle-frozen'),FRAMES='300',SKIP_METRICS='1',METRIC_RUNS='1')
d={'pid':os.getpid(),'status':'running','purpose':protocol['purpose'],'runs':[]};receipt.write_text(json.dumps(d,indent=2)+'\n')
for engine,script,label in [('cpu','scripts/record-box3d-oracle.sh','000-box3d-cpu'),('gpu','scripts/record-snapshot.sh','2026-10-01-native-diagnostics')]:
 for group,selected,view in [('regular',regular,None),('mixed',['mixed-topology'],'-220,290,-300,95,8,145')]:
  e=env.copy();e['RECORD_SCENES']=' '.join(selected)
  if view:e['GPU_RECORD_VIEW']=view
  for scene in selected:assert not (root/f'recordings/snapshots/{label}/{scene}.mp4').exists()
  command=['bash',script,label];log=out/f'record-standard-{engine}-{group}.log'
  d.update(status='running',current=f'{engine}-{group}');receipt.write_text(json.dumps(d,indent=2)+'\n')
  with log.open('w') as f:code=subprocess.run(command,env=e,stdout=f,stderr=subprocess.STDOUT).returncode
  d['runs'].append({'engine':engine,'scenes':selected,'frames':300,'view':view,'command':command,'exit':code,'environment':{k:v for k,v in e.items() if k.startswith(('GPU_','WGPU_','VK_'))},'log_sha256':sha(log)});receipt.write_text(json.dumps(d,indent=2)+'\n')
  if code:d['status']='failed';receipt.write_text(json.dumps(d,indent=2)+'\n');raise SystemExit(code)
  print(engine,group,'complete',flush=True)
d.update(status='complete');d.pop('current',None);d.pop('pid',None);receipt.write_text(json.dumps(d,indent=2)+'\n')
