#!/usr/bin/env python3
"""Check freshly built native CPU/GPU/both metric records, not performance."""
from pathlib import Path
import os,sys,json,subprocess,hashlib,time
root=Path(__file__).resolve().parent.parent;out=Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=False)
env=os.environ.copy();env.update(__NV_PRIME_RENDER_OFFLOAD='1',__GLX_VENDOR_LIBRARY_NAME='nvidia',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json')
reports={}
for mode in ['cpu','gpu','both']:
 binary=root/f'native-samples/build-{mode}/bin/samples_{mode}';digest=hashlib.sha256(binary.read_bytes()).hexdigest()
 args=['timeout','180','xvfb-run','-a',str(binary),'--sample-name','Shapes/High Resistance','--unpaced','--health-scan','--warmup','0','--timed','32','--bench-json',str(out/f'{mode}.json')]
 start=time.monotonic()
 with (out/f'{mode}.log').open('w') as log: process=subprocess.run(args,cwd=root/'../../box3d',env=env,stdout=log,stderr=subprocess.STDOUT)
 (out/f'{mode}-process.json').write_text(json.dumps({'exit':process.returncode,'elapsed':time.monotonic()-start,'sha256':digest,'command':args},indent=2)+'\n')
 assert process.returncode==0 and digest==hashlib.sha256(binary.read_bytes()).hexdigest()
 doc=json.loads((out/f'{mode}.json').read_text());assert doc['status']=='ok' and doc['mode']==mode and not doc['gpu_fail']
 assert doc['measured']==doc['timed']==len(doc['frames'])==32
 known=0;roots=0
 for i,f in enumerate(doc['frames']):
  assert f['sample']=='Shapes/High Resistance' and f['submitted_step']==i+1
  assert f['contact_count_known'] == (mode=='cpu')
  m=f['gpu_contact_metrics'];assert m['phase']=='contact_scheduling' and not m['capacity_loss']
  if mode=='cpu':assert not m['known'];continue
  if not m['known']:assert not m['current'];continue
  known+=1;roots=max(roots,m['allocated_roots'])
  assert 0<m['snapshot_step']<=m['submitted_step']==i+1
  assert m['snapshot_topology']<=m['current_topology'] and m['snapshot_state']<=m['current_state']
  assert m['allocated_manifold_slots']>=m['allocated_roots']>=m['touching_roots']
  assert m['non_sensor_roots']==m['allocated_roots'], 'High Resistance has no sensors'
  if m['current']:
   assert m['snapshot_step']==m['submitted_step'] and m['snapshot_topology']==m['current_topology'] and m['snapshot_state']==m['current_state']
 if mode!='cpu':assert known>0 and roots>0
 reports[mode]={'known_frames':known,'peak_roots':roots,'status':'pass'}
 print(mode,reports[mode],flush=True)
(out/'result.json').write_text(json.dumps({'status':'pass','modes':reports,'scope':'native metric plumbing and snapshot identity','public_contact_count_implemented':False,'cpu_win_validated':False},indent=2)+'\n')
