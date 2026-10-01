from pathlib import Path
import json,hashlib,os,subprocess,tarfile
r=Path.cwd();a=r/'artifacts/production-readiness/pr02-compound-aabb-captures';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
base={k:v for k,v in os.environ.items() if not k.startswith(('GPU_','WGPU_','VK_','__NV','__GLX','LIBGL'))}
raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],env=base)
gpu={x.decode().split('=',1)[0]:x.decode().split('=',1)[1] for x in raw.split(b'\0') if x.startswith((b'GPU_',b'WGPU_'))}
common={'DISPLAY':':97','GPU_BENCH_WIDTH':'1280','GPU_BENCH_HEIGHT':'720','__NV_PRIME_RENDER_OFFLOAD':'0','__GLX_VENDOR_LIBRARY_NAME':'mesa','LIBGL_ALWAYS_SOFTWARE':'1'}
gpu.update(common);gpu.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(a/'pipelines'))
viewers={}
for kind,b in [('cpu',a/'cpu-viewer/receipt.json'),('gpu',r/'artifacts/production-readiness/pr02-compound-aabb-validation/native-viewer/receipt.json')]:
 d=json.loads(b.read_text());assert d['status']=='built';assert sha(Path(d['executable']))==d['executable_sha256']
 viewers[kind]={'binary':str(Path(d['executable']).relative_to(r)),'binary_sha256':d['executable_sha256'],'receipt':str(b.relative_to(r)),'receipt_sha256':sha(b),'label':'000-box3d-cpu' if kind=='cpu' else '2026-10-01-compound-aabb'}
source=['scripts/record-native-scenes.py','scripts/record-box3d-oracle.sh','scripts/record-snapshot.sh','scripts/refresh-compare.ts','scripts/native-samples-cache-env.sh']
assets={os.path.relpath(p,r):sha(p) for p in sorted((r.parents[1]/'box3d/data').rglob('*')) if p.is_file()}
scenes=[{'id':'compound-'+n,'name':'Compound/'+s,'scope':'unaffected control' if n=='mesh-tile' else 'affected compound scene'} for n,s in [('simple','Simple'),('spheres','Spheres'),('hulls','Hulls'),('tile-floor','Tile Floor'),('village','Village'),('mesh-tile','Mesh Tile')]]
for k,v in viewers.items():
 for s in scenes:assert not (r/'recordings/snapshots'/v['label']/(s['id']+'.mp4')).exists()
p={'purpose':'precommit visual/health evidence, not performance','budget':{'cpu':6,'gpu':6,'timing':0,'retries':0},'order':'six real Box3D CPU processes, then six native Vulkan GPU processes; scenes in listed order','stop_rule':'first launch/health/capture/source failure stops campaign; retain all completed/failed/incomplete artifacts and unlaunched cases, no retries','watchdog_seconds_per_scene':600,'artifact_directory':str(a.relative_to(r)),'viewers':viewers,'recorder_sha256':sha(r/source[0]),'source_files':{n:sha(r/n) for n in source},'build_applicability_exclusions':['scripts/record-box3d-oracle.sh','scripts/record-snapshot.sh'],'applicability_note':'Only recording shell wrappers changed after viewer builds; neither is compiled into a viewer. All other recorded build source inputs and every actual compiled unit/object must match. New native recorder is separately frozen. No engine/viewer production input changed.','scenes':scenes,'assets':assets,'xvfb':'artifacts/production-readiness/pr01-speculative/handoff/xvfb/usr/bin/Xvfb','environments':{'cpu':common,'gpu':gpu},'settings':{'dt':'1/60','substeps':4,'sleep':True,'warm_start':True,'continuous':True,'scene_defaults':'unchanged upstream geometry/materials/gravity','steps':300,'warmup':0,'display':[1280,720],'encode_fps':30,'ui':'hide only UI; fresh {} settings suppress first-run help/replay redirect','renderer':'Mesa software OpenGL on isolated Xvfb; physics GPU uses actual NVIDIA Vulkan','cpu_workers':'viewer automatic clamp(logical cores/2,1,8)','incidental_clocks':'correctness/recording diagnostics; excluded from performance claims'},'git_checkout_context_only':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()}
assert not (a/'protocol.json').exists();(a/'protocol.json').write_text(json.dumps(p,indent=2)+'\n')
(a/'adapter.txt').write_text(subprocess.check_output(['nvidia-smi','--query-gpu=name,uuid,driver_version,pci.bus_id','--format=csv'],text=True))
with tarfile.open(a/'capture-inputs.tar.gz','w:gz') as t:
 for n in source:t.add(r/n,arcname=n)
 for n in assets:t.add(r/n,arcname=n.replace('../../box3d/','box3d/'))
print('Frozen capture protocol:',sha(a/'protocol.json'),flush=True)
