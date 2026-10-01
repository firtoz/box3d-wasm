#!/usr/bin/env python3
"""Offline PR01 applicability review from retained scene data and pinned sources."""
from pathlib import Path
import hashlib,json,struct,subprocess
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
f32=lambda x:struct.unpack('f',struct.pack('f',x))[0]
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
# Source-native float32 setup and semi-implicit substep integration; independent
# of the measured scene's collision results. No new GPU run or candidate.
h=f32(f32(1/60)/4);scale=f32(2.03);delta_v=f32(f32(h*scale)*f32(-10))
half_height=f32(f32(36)*f32(.0254));half_width=f32(f32(16)*f32(.0254));y=f32(half_height+f32(.1));v=0.
raw={k:json.loads((HERE/f'raw/handoff/native-scene-{k}/health.json').read_text()) for k in ['cpu','gpu']}
rows=[]
for step in range(1,7):
 previous=y;delta_y=0.
 for _ in range(4):
  v=f32(v+delta_v);delta_y=f32(delta_y+f32(h*v))
 y=f32(y+delta_y)
 comparisons={}
 for kind,data in raw.items():
  frame=data['frames'][step-1];body=frame['bodies'][0]
  matches=f32(body['p'][1])==y and f32(body['v'][1])==v
  comparisons[kind]={'height':body['p'][1],'vertical_velocity':body['v'][1],'matches_free_fall_float32':matches}
  if not matches:raise SystemExit(f'{kind}: step{step} is not explained by free flight')
 radius=max(f32(.5*half_width),f32(.005))
 rows.append({'step':step,'computed_height':y,'computed_vertical_velocity':v,'start_height':previous,'bottom_height':f32(y-half_height),'floor_plane_glancing_predicate':previous-y<radius and y>radius,'data':comparisons})
source_paths=[ROOT/'src/api/query.rs',ROOT/'shaders/physics/integrate.wgsl',ROOT/'shaders/physics/collide.wgsl',ROOT.parent.parent/'box3d/src/solver.c',ROOT.parent.parent/'box3d/src/shape.c',ROOT.parent.parent/'box3d/samples/sample_issues.cpp',ROOT/'native-samples/sokol_loading.cpp',ROOT/'src/sim.rs']
# Preserve the exact failure; do not silently change it to a pass or a new bound.
failed_screen={k:json.loads((HERE/f'raw/handoff/native-scene-{k}/review.json').read_text()) for k in raw}
assert all(x['floor_screen_status']=='fail' and len(x['floor_screen_failures'])==1 and x['floor_screen_failures'][0]['step']==6 for x in failed_screen.values())
result={'kind':'source/retained-data applicability review','new_gpu_runs':0,'new_candidates':0,'original_floor_screen':'FAIL retained at0.9094m; no raised threshold or replacement passing screen','isolated_ccd_floor_bound':'0.495m unchanged; original focused confirmations and guards retain all assertions','free_fall_rows':rows,'body_half_height':half_height,'body_half_width':half_width,'mesh_fallback_radius':max(f32(.5*half_width),f32(.005)),'source_sha256':{str(p.relative_to(ROOT)) if p.is_relative_to(ROOT) else str(p):sha(p) for p in source_paths},'box3d_revision':subprocess.check_output(['git','-C',str(ROOT.parent.parent/'box3d'),'rev-parse','HEAD'],text=True).strip(),'interpretation':'Both engines reach the failed screen through exact discrete free flight before contact response. Native CPU fast classification uses half extent; GPU activates sooner but deliberately preserves the native mesh glancing-plane filter. Floor-plane CCD is skipped here. The added all-window5mm bound was copied from the distinct150m/s normal-impact control; it is not a valid assertion of that control’s scope in this glancing/discrete scene. No additional accepted physical tolerance is inferred. Original upstream ghost-launch screen passes; physical whole-matrix qualification remains forPR03/PR04/PR07.','baseline_timeout':'No completed attribution result/qualification credit. Last log is before collision preparation cache-save/ready; source permits first-time compilation to take minutes. A180s timeout cannot distinguish slow compilation from a hang. No retry or fabricated baseline result.'}
(HERE/'source-review.json').write_text(json.dumps(result,indent=2)+'\n');print('Both engines: six exact float32 free-flight steps; original added screen failure retained; zero new GPU runs')
if __name__=='__main__':pass
