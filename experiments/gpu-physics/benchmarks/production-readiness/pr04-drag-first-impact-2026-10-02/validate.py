#!/usr/bin/env python3
from pathlib import Path
import gzip,hashlib,json,subprocess,sys
R=Path(__file__).resolve().parent
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
for n,v in json.loads((R/'raw-index.json').read_text()).items():assert sha(R/n)==v['sha256'] and (R/n).stat().st_size==v['bytes'],n
p=json.loads((R/'protocol.json').read_text());assert p['budget']=={'archived_traces':2,'selected_window':'220–232/body0','engine_processes':0,'builds':0,'candidates':0,'timing':0}
old=(R/'analysis.json').read_bytes();subprocess.run([sys.executable,str(R/'analyze.py')],check=True,stdout=subprocess.DEVNULL);assert (R/'analysis.json').read_bytes()==old
s=json.loads(old)
prior=json.loads((R/'raw/baseline-trace-analysis.json').read_text())
for k in ['ordinary','native']:
 data=s['observations'][k];assert data['paired_body0_frames']==3060
 assert hashlib.sha256(gzip.decompress((R/'raw'/f'{k}-comparison.txt.gz').read_bytes())).hexdigest()==data['raw_decompressed_sha256']
 assert [x['frame'] for x in data['window']]==list(range(220,233))
 impact=next(x for x in data['window'] if x['frame']==227)
 assert impact['position_difference']>0.005 and impact['velocity_difference']<1e-5
 assert abs(impact['observations'][1]['minimum_corner_y']-.005)<1e-7
 assert impact['observations'][0]['minimum_corner_y']<-.010
 assert prior[k+'-drag-ground']['source_assertion_result']=='FAIL'
code=(R/'raw/gpu-world.rs').read_text();cpu=(R/'raw/cpu-solver.c').read_text();pointer=(R/'raw/both_dual.c').read_text()
assert '(0.5 * cpu.min_extent).min(crate::types::SPECULATIVE_DISTANCE)' in code
assert 'maxMotion > safetyFactor * sim->minExtent' in cpu and 'b3MaxFloat( maxDeltaPosition, maxVelocity * timeStep )' in cpu
part=pointer.split('void both_pointer_down(',1)[1].split('static void both_drag_step(',1)[0]
assert 'cpu_b3CreateMotorJoint' in part and 'create_gpu_motor' in part and 'DistanceJoint' not in part
print('PASS:two original3060-step traces preserved; current CCD activation discrepancy and first-impact window reproducible offline. Failed dragging gates stay failed;zero new engine/build/candidate/timing.')
