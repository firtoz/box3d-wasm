#!/usr/bin/env python3
"""Offline capture evidence validation; never launches physics."""
from pathlib import Path
import hashlib,json,subprocess,tarfile
BASE=Path(__file__).resolve().parent
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
p=json.loads((BASE/'protocol.json').read_text());index=json.loads((BASE/'raw-index.json').read_text())
for name,digest in index.items():assert sha(BASE/name)==digest,name
assert (BASE/'protocol.json').read_bytes()==(BASE/'raw/protocol.json').read_bytes()
state=json.loads((BASE/'raw/receipt.json').read_text())
assert state['protocol_sha256']==sha(BASE/'protocol.json')
assert p['budget']=={'cpu':6,'gpu':6,'timing':0,'retries':0}
expected=[(kind,scene['id'])for kind in ['cpu','gpu']for scene in p['scenes']]
actual=[(v['kind'],v['scene'])for v in state['results']]
assert actual==expected[:len(actual)]
assert len(set(actual))==len(actual)
if state['status']=='captured':assert len(actual)==12 and all(x['status']=='pass'for x in state['results'])
else:assert state['status']=='stopped'
engine=BASE.parents[2]
assert engine.name=='gpu-physics'
for kind in ['cpu','gpu']:
 receipt=BASE.parent/'pr02-compound-aabb-captures-2026-10-01/raw/cpu-viewer/receipt.json' if kind=='cpu'else BASE/'raw/native-viewer/receipt.json'
 assert sha(receipt)==p['viewers'][kind]['receipt_sha256']
 build=json.loads(receipt.read_text());assert build['status']=='built'and build['inputs_before']==build['inputs_after']
 assert build['executable_sha256']==p['viewers'][kind]['binary_sha256']
 archive=receipt.parent/'compiled-viewer-inputs.tar.gz';assert sha(archive)==build['source_archive_sha256']
 with tarfile.open(archive)as t,tarfile.open(BASE.parent/'pr02-compound-aabb-captures-2026-10-01/raw/viewer-dependencies.tar.gz')as deps:
  for unit in build['compiled_units']:
   source=unit['file'];name='generated/'+source.split('/cmake/',1)[1]if '/cmake/'in source else source.split('/experiments/gpu-physics/',1)[1]if '/experiments/gpu-physics/'in source else 'box3d/'+source.split('/box3d/',1)[1]
   assert hashlib.sha256((deps if kind=='cpu'and '.fetchcontent-cache/'in name else t).extractfile(name).read()).hexdigest()==unit['source_sha256'],name
for row in state['results']:
 folder=BASE/'raw'/row['kind']/row['scene']
 launch=json.loads((folder/'launch.json').read_text());assert launch['protocol_sha256']==sha(BASE/'protocol.json')
 assert launch['binary_sha256']==p['viewers'][row['kind']]['binary_sha256']
 assert '--no-sleep'not in launch['command']and '--hide-ui'in launch['command']and launch['initial_settings']=='{}\n'
 if row['status']!='pass':continue
 health=json.loads((folder/'health.json').read_text());assert sha(folder/'health.json')==row['health_sha256']
 assert health['mode']==p['viewers'][row['kind']].get('health_mode',row['kind'])
 assert health['status']=='ok'and health['frames_observed']==health['measured']==300 and len(health['frames'])==300
 assert health['enable_sleep']and health['completed_step_mode']and not health['unpaced']and not health['village_drop']and not health['gpu_fail']
 for i,f in enumerate(health['frames'],1):
  assert f['submitted_step']==f['completed_step']==f['rendered_pose']==i
  assert f['nan_count']==0 and not f['gpu_contact_metrics']['capacity_loss']
 stderr=(folder/'stderr').read_text();assert 'samples: 300 frames, 0 sokol errors'in stderr
 if row['kind']=='gpu':assert 'NVIDIA GeForce RTX 4070 SUPER | backend=Vulkan'in stderr
 # CPU clips are immutable in this report; the live grid can receive new CPU captures.
 video=(BASE/'raw/recordings/cpu' if row['kind']=='cpu' else engine/'recordings/snapshots'/p['viewers'][row['kind']]['label'])/(row['scene']+'.mp4')
 assert sha(video)==row['video_sha256']
 probe=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_streams','-of','json',str(video)],text=True))
 stream=next(s for s in probe['streams']if s['codec_type']=='video')
 assert stream['width']==1280 and stream['height']==720 and stream['r_frame_rate']=='30/1'and int(stream['nb_frames'])>=300
print('Validated',len(index),'raw files;',sum(v['status']=='pass'for v in state['results']),'passing captures; state',state['status'],'No timing or final physical/readiness acceptance.')
