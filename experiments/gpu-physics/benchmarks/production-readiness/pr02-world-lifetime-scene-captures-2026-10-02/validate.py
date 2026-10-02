#!/usr/bin/env python3
"""Offline verification of portable scene/build records; never launches physics."""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parent
sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
def read(p):return json.loads((R/p).read_text())
idx=read('raw-index.json')
for n,h in idx.items():assert sha(R/n)==h,n
p=read('protocol.json');assert p['budget']=={'standard_cpu_oracle_processes':20,'standard_cpu_replay_render_processes':20,'standard_gpu_capture_processes':20,'native_cpu_capture_processes':6,'native_combined_capture_processes':6,'clips':52,'headline_timing_runs':0,'incidental_cpu_oracle_clock_records':20,'retries':0}
s=read('raw/capture/receipt.json');assert s['status']=='captured' and len(s['results'])==42 and all(x['status']=='pass' and x['exit']==0 for x in s['results']);assert s['protocol_sha256']==sha(R/'raw/capture/protocol-before-captures.json')
assert [(x['kind'],x['scene']) for x in s['results']]==[('cpu',z) for z in p['standard_scenes']]+[('native-cpu','six-compound-scenes')]+[('gpu',z) for z in p['standard_scenes']]+[('native-gpu','six-compound-scenes')]
b=read('raw/receipt.json');assert b['status']=='built' and b['inputs_before']==b['inputs_after'];assert b['protocol_sha256']==sha(R/'raw/protocol-before-build.json');assert p['build_receipt_sha256']==sha(R/'raw/receipt.json')
n=read('raw/capture/native/receipt.json');np=read('raw/capture/native/protocol.json');assert n['status']=='captured' and len(n['results'])==12 and n['protocol_sha256']==sha(R/'raw/capture/native/protocol.json');assert np['budget']=={'cpu':6,'gpu':6,'timing':0,'retries':0}
for x in n['results']:
 assert x['status']=='pass' and x['exit']==x['ffmpeg_exit']==0 and x['encoded_frames']>=300
 h=read(f"raw/capture/native/{x['kind']}/{x['scene']}/health.json");assert h['status']=='ok' and h['frames_observed']==h['measured']==300 and h['last_submitted_step']==h['last_completed_step']==h['last_rendered_pose']==300 and h['in_flight']==0 and not h['gpu_fail'] and not h['village_drop'];assert h['enable_sleep'] and h['completed_step_mode'] and not h['unpaced']
 for i,f in enumerate(h['frames'],1):assert f['submitted_step']==f['completed_step']==f['rendered_pose']==i and f['nan_count']==0 and not f['gpu_contact_metrics']['capacity_loss']
 err=(R/f"raw/capture/native/{x['kind']}/{x['scene']}/stderr").read_text();assert 'samples: 300 frames, 0 sokol errors' in err
 if x['kind']=='gpu':assert 'NVIDIA GeForce RTX 4070 SUPER | backend=Vulkan' in err
 c=read(f"raw/capture/native/{x['kind']}-applicability-receipt.json");assert c['status']=='built' and c['inputs_before']==c['inputs_after'];assert len(c['compiled_units'])==(119 if x['kind']=='cpu' else 178)
clips=read('raw/clip-checks.json')['clips'];assert len(clips)==52
for c in clips:
 f=R/f"raw/clips/{c['engine']}/{c['scene']}.mp4";assert sha(f)==c['sha256'];v=c['stream'];assert (v['width'],v['height'],v['r_frame_rate'])==(1280,720,'30/1') and int(v['nb_frames'])>=300
prev=read('raw/capture/preserved-prior-cpu.json');assert prev['archive_all_files']==p['prior_cpu_column'];assert prev['old_metrics_not_copied']
for f,h in prev['archive_all_files'].items():assert sha(R/'raw/capture/previous-cpu-column'/f)==h
for cell in ['ordinary','native']:
 a=read(f'raw/capture/api-applicability/{cell}.json');assert a['status']=='complete' and a['build_artifacts_verified'] and a['required_stateful_header_symbols']==415 and len(a['declared_unavailable_symbols'])==39
for d in [read('visual-review.json')]:assert len(d['reviews'])==26 and d['status']=='reviewed-with-recorded-limitations';assert d['no_full_physics_or_performance_acceptance']
print(f'PASS: {len(idx)} portable raw files,52 clips,12 native health cases,26 reviewed scene pairs,2 inventories; no performance qualification')
