#!/usr/bin/env python3
"""Offline validation: portable evidence and tracked clips, no GPU/binaries."""
from pathlib import Path
import hashlib,json,re,gzip
O=Path(__file__).resolve().parent;R=O.parents[2];sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
index=json.loads((O/'raw-index.json').read_text())
for n,h in index.items():assert sha(O/n)==h,n
inventory=json.loads((O/'inventory.json').read_text());assert len(inventory['native'])==165 and len({r['cpu_identity'] for r in inventory['native']})==165
assert len(inventory['browser'])==174 and len(inventory['oracle_cpu_fixtures'])==20
assert sum('floor visibility only' in r['ordinary_review'] for r in inventory['native'])==5
assert sum(not r['native_cpu_matches'] for r in inventory['browser'])==13
assert all(r['performance'].startswith('UNREVIEWED') for r in inventory['native'])
assert next(r for r in inventory['native'] if r['cpu_identity']=='Replay/Viewer')['availability'].startswith('registered but uses excluded API')
p=json.loads((O/'protocol.json').read_text());assert hashlib.sha256(gzip.decompress((O/'raw/preexisting-roadmap.patch.gz').read_bytes())).hexdigest()==p['preexisting_documentation_diff_sha256'];assert p['budget']=={'viewer_builds':5,'focused_test_builds':2,'focused_checks':12,'captures':45,'steps_per_capture':60,'watchdog_seconds_per_build':180,'watchdog_seconds_per_capture':300,'retries':0,'performance_runs':0,'Rust_builds':0}
build=json.loads((O/'raw/build-receipt.json').read_text());assert build['status']=='built' and len(build['focused'])==12 and all(r['exit']==0 for r in build['focused'])
assert all(r['exit']==0 for r in build['results'])
for key in ['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both']:
 receipt=json.loads((O/'raw/build'/key/'receipt.json').read_text());assert receipt['status']=='built' and receipt['inputs_before']==receipt['inputs_after'];u=next(u for u in receipt['compiled_units'] if '/pr09-floor/' in u['file'] and u['file'].endswith('debug_adapter.c'));assert sha(O/'raw/generated'/key/'debug_adapter.c')==u['source_sha256']
# Source identity pins only the rendering operation; original CPU/Rust producer
# provenance is retained in referenced prior reports, not inferred from HEAD.
for variant,expected in [('before','registered=65535 firstMissing=65535'),('after','registered=105004 firstMissing=-1')]:assert expected in (O/'raw'/('focused-'+variant)/'0.log').read_text()
count=0;recovered=0
for phase in ['before','after']:
 q=json.loads((O/'raw/captures'/phase/'protocol.json').read_text());s=json.loads((O/'raw/captures'/phase/'receipt.json').read_text());assert len(s['results'])==25
 assert s['status']==('captured' if phase=='before' else 'captured-with-interruption')
 for row in s['results']:
  if phase=='after' and row['kind']=='cpu':continue
  count+=1;assert row['status'] in ['pass','recovered-health-and-video'];recovered+=row['status']=='recovered-health-and-video'
  directory=O/'raw/captures'/phase/row['viewer']/row['scene'];h=json.loads((directory/'health.json').read_text());assert sha(directory/'health.json')==row['health_sha256'];assert h['status']=='ok' and h['frames_observed']==h['measured']==60 and len(h['frames'])==60 and h['enable_sleep'] and h['completed_step_mode'] and not h['unpaced'] and not h['gpu_fail'] and not h['village_drop']
  assert h['last_submitted_step']==h['last_completed_step']==h['last_rendered_pose']==60 and h['in_flight']==0
  for i,f in enumerate(h['frames'],1):assert f['submitted_step']==f['completed_step']==f['rendered_pose']==i and not f['nan_count'] and not f['gpu_contact_metrics']['capacity_loss']
  if row['kind']=='gpu':assert 'NVIDIA GeForce RTX 4070 SUPER' in (directory/'stderr').read_text() and 'backend=Vulkan' in (directory/'stderr').read_text()
  v=q['viewers']['cpu'] if row['kind']=='cpu' else q['gpu_viewers'][row['viewer']];clip=R/'recordings/snapshots'/v['label']/(row['scene']+'.mp4');assert sha(clip)==row['video_sha256']
  manifest=json.loads(clip.with_name(row['scene']+'-recording-manifest.json').read_text());assert manifest['clip_sha256']==row['video_sha256'] and manifest['steps']==60
  if row['status']=='pass':assert row['exit']==row['ffmpeg_exit']==0
  else:assert row['exit'] is None and row['ffmpeg_exit'] is None and row['review_window_end_seconds']>5
assert count==45 and recovered==1
# Physics population/defaults do not change across the rendering repair.
for key in ['ordinary-gpu','native-gpu','ordinary-both','native-both']:
 for scene in ['village','tile-floor','mesh-tile','single-box','mesh-grid']:
  h=[]
  for phase in ['before','after']:h.append(json.loads((O/'raw/captures'/phase/key/('pr09-floor-'+scene)/'health.json').read_text()))
  for field in ['body_count','joint_count','gpu_draw_shapes']:assert [f[field] for f in h[0]['frames']]==[f[field] for f in h[1]['frames']],(key,scene,field)
  if scene=='village' and key.endswith('both'):
   assert h[0]['frames'][-1]['renderer_instances']==143 and h[1]['frames'][-1]['renderer_instances']==1551
assert len(json.loads((O/'raw/review-sampling-final.json').read_text()))==100
for key in ['ordinary-both','native-both']:
 text=(O/'raw/captures/after'/key/'pr09-floor-village/stderr').read_text();assert '131072 stable slots (was 65536)' in text
print(f'PASS: {len(index)} evidence hashes, 165 native/174 browser/20 oracle entries, 12 calculation checks, 5 viewer builds, 45 healthy captures (one retained transport recovery). PR09 remains open.')
