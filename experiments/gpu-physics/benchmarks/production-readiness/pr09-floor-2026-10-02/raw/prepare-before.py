from pathlib import Path
import json,hashlib
R=Path.cwd();D=R/'benchmarks/production-readiness/pr09-floor-2026-10-02';A=R/'artifacts/production-readiness/pr09-floor';sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
p=json.loads((R/'benchmarks/production-readiness/pr04-distance-captures-2026-10-02/raw/capture/protocol.json').read_text())
p.update(artifact_directory='artifacts/production-readiness/pr09-floor/before', steps_per_capture=60, budget={'cpu':5,'gpu':20,'timing':0,'retries':0},recorder_sha256=sha(D/'raw/recorder.py'))
p['scenes']=[{'id':'pr09-floor-'+i,'name':n} for i,n in zip(['village','tile-floor','mesh-tile','single-box','mesh-grid'],json.loads((D/'protocol.json').read_text())['scenes'])]
p['source_files']={n:sha(R/n) for n in ['native-samples/CMakeLists.txt','scripts/inject-sokol-capacity.py','native-samples/sokol_capacity.c','../../box3d/samples/gfx/debug_adapter.c','../../box3d/samples/sample_compound.cpp','../../box3d/samples/sample_mesh.cpp','../../box3d/samples/sample_stacking.cpp']}
p['viewers']['cpu']['label']='000-box3d-cpu'
for k,v in p['gpu_viewers'].items():v['label']='2026-10-02-pr09-floor-before-'+k
for k,v in {'cpu':p['viewers']['cpu'],**p['gpu_viewers']}.items():
 assert sha(R/v['binary'])==v['binary_sha256'];d=json.loads((R/v['receipt']).read_text());assert sha(R/v['receipt'])==v['receipt_sha256'];assert d['inputs_before']==d['inputs_after']
 for n,h in d['inputs_before'].items():
  if not n.startswith(('src/','shaders/')) and n not in ['Cargo.toml','Cargo.lock','build.rs']: assert sha(R/n)==h,(k,n)
 for u in d['compiled_units']:assert sha(u['file'])==u['source_sha256'] and sha(u['object'])==u['object_sha256'],u['file']
p['environments']['cpu']['DISPLAY']=p['environments']['gpu']['DISPLAY']=':84'
p['settings']='Original upstream scene defaults, dt1/60/four substeps; 60 paced completed/rendered steps, zero warmup; diagnostic Mesa/Xvfb graphics plus NVIDIA Vulkan compute. No Village drop. No trajectory/pixel/performance qualification.'
p['stop_retain']='Frozen PR09 budget: CPU first then four GPU variants; stop on failure, zero retries, no overwrite.'
(A/'before').mkdir();(A/'before/protocol.json').write_text(json.dumps(p,indent=2)+'\n');print('before frozen',sha(A/'before/protocol.json'))
