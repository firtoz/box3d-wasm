from pathlib import Path
import hashlib,json,tarfile
base=Path(__file__).resolve().parent;sha=lambda b:hashlib.sha256(b).hexdigest();index=json.loads((base/'raw-index.json').read_text())
for name,r in index.items():
 data=(base/name).read_bytes();assert len(data)==r['bytes'] and sha(data)==r['sha256'],name
r=json.loads((base/'raw/receipt.json').read_text());assert r['status']=='built',r.get('failure');assert r['inputs_before']==r['inputs_after'];assert all(c['exit']==0 for c in r['commands']);assert len(r['commands'])==12 # four configure/generate/build triples; exact Rust libraries reused
P=json.loads((base/'raw/protocol-before-builds.json').read_text());assert sha((base/'raw/protocol-before-builds.json').read_bytes())==r['protocol_sha256'];assert P['budget']['app_processes']==P['budget']['timing_runs']==0
sources={}
with tarfile.open(base/'raw/compiled-source-inputs.tar.gz')as archive:
 for m in archive.getmembers():
  if m.isfile():sources[m.name]=sha(archive.extractfile(m).read())
assert not any(n.startswith('native-samples/build-')for n in sources)
for cell in P['order']:
 cell=cell.removesuffix('-viewer');v=json.loads((base/'raw'/cell/'receipt.json').read_text());assert v['status']=='built';assert sha((base/'raw'/cell/'receipt.json').read_bytes())==r['viewers'][cell]['receipt_sha256']
 original=(base/'raw'/cell/'main.original.cpp').read_text();observed=(base/'raw'/cell/'main.observed.cpp').read_text();assert original.count('static SampleContext s_context;')==original.count('\ts_context.sample->Step();\n')==1
 expected=original.replace('static SampleContext s_context;','static SampleContext s_context;\n#include "controls_observer.inc"').replace('\ts_context.sample->Step();\n','\ts_context.sample->Step();\n\tcontrols_observe(s_context, s_frame);\n');assert observed==expected;assert sha(observed.encode())==v['observed_main_sha256'];assert sha(original.encode())==v['original_main_sha256'];assert sha((base/'raw/observer.inc').read_bytes())==v['observer_sha256']
 generated={}
 with tarfile.open(base/'raw'/cell/'generated-inputs.tar.gz')as archive:
  for m in archive.getmembers():
   if m.isfile():generated[m.name]=sha(archive.extractfile(m).read())
 count=0
 for unit in v['compiled_units']:
  file=unit['file'];key=None
  if f'/pr02-world-lifetime-viewer-builds/{cell}/cmake/'in file:key=file.split(f'/pr02-world-lifetime-viewer-builds/{cell}/cmake/',1)[1];actual=generated[key]
  elif '/box3d-wasm/box3d/'in file:key='box3d/'+file.split('/box3d-wasm/box3d/',1)[1];actual=sources[key]
  elif '/experiments/gpu-physics/'in file:key=file.split('/experiments/gpu-physics/',1)[1];actual=sources[key]
  else:raise AssertionError(file)
  assert actual==unit['source_sha256'],(cell,key);count+=1
 assert count>100
 if cell!='cpu':
  backend=cell.split('-')[0];assert r['libraries'][backend]['path']in v['link_command'];assert 'gpu_physics'in v['link_command']or r['libraries'][backend]['path']in v['link_command']
 print(cell,count,'compiled sources and observer patch verified')
print(len(index),'raw files verified; build proof only, UI and physics acceptance remain open')
