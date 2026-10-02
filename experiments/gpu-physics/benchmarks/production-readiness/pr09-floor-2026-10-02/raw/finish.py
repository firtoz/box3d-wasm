from pathlib import Path
import json,shutil,hashlib,subprocess,re
R=Path.cwd();O=R/'benchmarks/production-readiness/pr09-floor-2026-10-02';A=R/'artifacts/production-readiness/pr09-floor';sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
# Called only when all capture cells are terminal; portable files never need ignored binaries.
for phase in ['before','after']:
 p=json.loads((A/phase/'protocol.json').read_text());s=json.loads((A/phase/'receipt.json').read_text())
 if phase=='after':
  recovery=json.loads((A/'after-remaining/receipt.json').read_text());assert recovery['status']=='captured' and len(recovery['results'])==10
  s={**s,'status':'captured-with-interruption','results':s['results']+[r for r in recovery['results'] if r['kind']=='gpu'],'remaining_receipt_sha256':sha(A/'after-remaining/receipt.json')}
 assert len(s['results'])==25 and all(r['status'] in ['pass','recovered-health-and-video'] for r in s['results'])
 out=O/'raw/captures'/phase;out.mkdir(parents=True,exist_ok=True)
 shutil.copyfile(A/phase/'protocol.json',out/'protocol.json')
 (out/'receipt.json').write_text(json.dumps(s,indent=2)+'\n')
 for row in s['results']:
  if phase=='after' and row['kind']=='cpu':continue
  key=row['viewer'];scene=row['scene'];source=(A/'after-remaining' if phase=='after' and key=='native-both' else A/phase)/key/scene;target=out/key/scene;target.mkdir(parents=True,exist_ok=True)
  for file in ['launch.json','capture.json','health.json','stdout','stderr','ffmpeg.log']:shutil.copyfile(source/file,target/file)
  v=p['viewers']['cpu'] if key=='cpu' else p['gpu_viewers'][key];clip=R/'recordings/snapshots'/v['label']/(scene+'.mp4');assert sha(clip)==row['video_sha256']
  manifest={'engine':'box3d-cpu' if key=='cpu' else 'gpu-physics','viewer':key,'recording_started_utc':'2026-10-02T10:05:00Z' if phase=='before' else '2026-10-02T10:25:00Z','started_timestamp_disclosure':'Ordering marker only; actual launch clocks are not retained by recorder. Before/after order is explicit protocol provenance, not a claimed UTC capture time.','scene':scene,'phase':phase,'clip_sha256':sha(clip),'binary_sha256':v['binary_sha256'],'protocol_sha256':sha(A/phase/'protocol.json'),'steps':60,'purpose':'diagnostic capture, no performance claim'}
  # Derive truthful recording timestamp from launch file mtime while host evidence exists.
  import datetime
  manifest['recording_started_utc']=datetime.datetime.fromtimestamp((source/'launch.json').stat().st_mtime,datetime.timezone.utc).isoformat();manifest['started_timestamp_disclosure']='Derived from original launch-file mtime at retention, approximate recorder start; phase order is fixed by protocols.'
  if row.get('review_window_end_seconds'):manifest['review_end_seconds']=row['review_window_end_seconds']
  (clip.parent/(scene+'-recording-manifest.json')).write_text(json.dumps(manifest,indent=2)+'\n')
 for key in (['cpu'] if phase=='before' else [])+list(p['gpu_viewers']):
  v=p['viewers']['cpu'] if key=='cpu' else p['gpu_viewers'][key];receipt=O/'raw/producers'/(phase+'-'+key+'.json');receipt.parent.mkdir(exist_ok=True);shutil.copyfile(R/v['receipt'],receipt)
for file in ['protocol.json','receipt.json']:shutil.copyfile(A/'build'/file,O/'raw'/('build-'+file))
for v in ['before','after']:shutil.copytree(A/'build'/('focused-'+v),O/'raw'/('focused-'+v),ignore=shutil.ignore_patterns('focused','*.o'),dirs_exist_ok=True)
for key in ['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both']:
 source=A/'build'/key;target=O/'raw/build'/key;target.mkdir(parents=True,exist_ok=True)
 for file in ['receipt.json','compile.log','link.log','archive.log']:shutil.copyfile(source/file,target/file)
for file in ['before-driver.log','before-driver-cpu.log','before-driver-gpu.log','after-driver.log','after-driver-capture.log','build-driver.log','after-remaining-live.log','after-remaining-driver.log']:
 shutil.copyfile(A/file,O/'raw'/file)
# Retain exact source bytes relevant to the one rendering operation.
for n in ['scripts/inject-sokol-capacity.py','scripts/record-native-scenes.py','native-samples/sokol_debug_pool_test.c','c_abi/growable_slots.h','native-samples/sokol_capacity.c','../../box3d/samples/gfx/debug_adapter.c']:
 target=O/'raw/source'/n.replace('../../box3d/','box3d/');target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(R/n,target)
summary=[]
for scene in json.loads((A/'before/protocol.json').read_text())['scenes']:
 for key in ['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both']:
  for phase in ['before','after']:
   if phase=='after' and key=='cpu':continue
   health=json.loads(((A/'after-remaining' if phase=='after' and key=='native-both' else A/phase)/key/scene['id']/'health.json').read_text());frames=health['frames'];fields=['body_count','joint_count','gpu_draw_shapes','renderer_instances']
   summary.append({'scene':scene['name'],'viewer':key,'phase':phase,'steps':len(frames),'mode':health['mode'],'counts':{f:[min(row[f] for row in frames),max(row[f] for row in frames)] for f in fields},'nan_count':max(row['nan_count'] for row in frames),'capacity_loss':any(row['gpu_contact_metrics']['capacity_loss'] for row in frames)})
(O/'capture-summary.json').write_text(json.dumps(summary,indent=2)+'\n')
for file in ['protocol.json','receipt.json']:shutil.copyfile(A/'after-remaining'/file,O/'raw'/('recovery-'+file))
shutil.copyfile(A/'after/receipt.json',O/'raw/after-original-terminal-receipt.json')
# Portable evidence clips are retained once in tracked snapshot columns.
print('retained 45 captures and actual compiled/source receipts')
