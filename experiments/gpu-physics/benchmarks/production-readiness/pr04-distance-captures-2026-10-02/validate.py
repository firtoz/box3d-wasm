#!/usr/bin/env python3
"""Verify archived captures without a GPU, viewer binary or original checkout."""
from pathlib import Path
import hashlib,json,math
R=Path(__file__).resolve().parent
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def read(n):return json.loads((R/n).read_text())
idx=read('raw-index.json')
for n,h in idx.items():assert sha(R/n)==h,n
p=read('raw/capture/protocol.json');s=read('raw/capture/receipt.json')
assert p['budget']=={'cpu':3,'gpu':12,'timing':0,'retries':0}
assert s['status']=='captured' and s['protocol_sha256']==sha(R/'raw/capture/protocol.json')
viewers={'cpu':p['viewers']['cpu'],**p['gpu_viewers']}
assert [(x['viewer'],x['scene']) for x in s['results']]==[(k,z['id']) for k in viewers for z in p['scenes']]
for n,h in p['source_files'].items():assert sha(R/'raw/sources'/Path(n).name)==h
assert sha(R/'raw/sources/record-native-scenes.py')==p['recorder_sha256']
links=R.parent/'pr04-distance-viewers-2026-10-02'
assert sha(links/'raw/receipt.json')==p['viewer_relink_receipt_sha256']
for k,v in viewers.items():
 receipt=R/'raw/viewer-receipts'/f'{k}.json'
 assert sha(receipt)==v['receipt_sha256']
 b=json.loads(receipt.read_text());assert b['status']=='built' and b['inputs_before']==b['inputs_after']
 assert len(b['compiled_units'])==(119 if k=='cpu' else 178 if 'both' in k else 124)
 if k!='cpu':assert sha(links/'raw'/k/'receipt.json')==sha(receipt) and b['executable_sha256']==v['binary_sha256']
 m=read(f'raw/manifests/{k}.json');assert m['protocol_sha256']==s['protocol_sha256'] and m['binary_sha256']==v['binary_sha256'] and not m['performance_claim']
for x in s['results']:
 assert x['status']=='pass' and x['exit']==x['ffmpeg_exit']==0 and x['encoded_frames']>=300
 key=x['viewer'];scene=x['scene'];base=f'raw/capture/{key}/{scene}'
 h=read(base+'/health.json');assert sha(R/base/'health.json')==x['health_sha256']
 assert h['status']=='ok' and h['mode']==viewers[key].get('health_mode',key) and h['frames_observed']==h['measured']==300
 assert h['enable_sleep'] and h['completed_step_mode'] and not h['unpaced'] and not h['gpu_fail'] and not h['village_drop']
 assert h['last_submitted_step']==h['last_completed_step']==h['last_rendered_pose']==300 and h['in_flight']==0
 assert len(h['frames'])==300
 for i,f in enumerate(h['frames'],1):
  assert f['submitted_step']==f['completed_step']==f['rendered_pose']==i and f['nan_count']==0 and not f['gpu_contact_metrics']['capacity_loss'] and not f['exploded']
  for body in f.get('bodies',[]):assert all(math.isfinite(z) for name in ['p','q','v','w'] for z in body[name])
 launch=read(base+'/launch.json');assert launch['binary_sha256']==viewers[key]['binary_sha256'] and launch['protocol_sha256']==s['protocol_sha256']
 assert launch['initial_settings']=='{}\n' and '--hide-ui' in launch['command']
 log=(R/base/'stderr').read_text();assert 'samples: 300 frames, 0 sokol errors' in log
 if key!='cpu':assert 'NVIDIA GeForce RTX 4070 SUPER | backend=Vulkan' in log
 assert sha(R/f'raw/clips/{key}/{scene}.mp4')==x['video_sha256']
 # capture.json was emitted before the health assertion, so its status remains running.
 # The final receipt above is the authoritative terminal status; retain original bytes.
 c=read(base+'/capture.json');assert c['status']=='running' and c['includes_startup_loading'] and c['video_sha256']==x['video_sha256']
 stream=c['probe']['streams'][0];assert (stream['width'],stream['height'],stream['r_frame_rate'])==(1280,720,'30/1')
review=read('visual-review.json');assert len(review['reviews'])==15 and review['status']=='reviewed-with-recorded-limitations' and review['no_full_physics_or_performance_acceptance']
for n,h in review['sheets'].items():assert sha(R/n)==h
samples=read('raw/review-sampling.json')['samples'];assert len(samples)==60
for x in samples:assert sha(R/x['path'])==x['sha256']
audit=read('source-applicability.json');assert audit['changed_production_inputs']==['shaders/physics/solve.wgsl']
provider=json.loads((links/'raw/candidate_Rust_producer-receipt.json').read_text())
assert audit['current_compiled_inputs']==provider['compiled_inputs_after'] and len(audit['current_compiled_inputs'])==107
print(f'PASS:{len(idx)} archived rawfiles,15 CPU-first clips,4500 completed health steps,60 reviewed samples,107 retained candidate inputs. No performance/release qualification.')
