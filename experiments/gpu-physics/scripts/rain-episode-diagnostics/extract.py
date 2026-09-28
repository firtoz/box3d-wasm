"""Extract a diagnostic Rain cell window without assuming creation-ID offsets.

This subset is NOT a complete-state qualification trace or a replayable world.
Contact partners and step-start transforms are retained for load analysis.
"""
import argparse, gzip, hashlib, json, struct, sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from health_identity import health_to_core_creations
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--trace',type=Path,required=True)
p.add_argument('--health',type=Path,required=True)
p.add_argument('--out',type=Path,required=True)
p.add_argument('--first',type=int,required=True,help='First zero-based health frame')
p.add_argument('--last',type=int,required=True,help='Last zero-based health frame, inclusive')
p.add_argument('--base',type=int,required=True,help='Human creation ID of first body in the 42-body cell')
p.add_argument('--target',type=int,required=True,help='Human creation ID of target spherical joint endpoint B')
a=p.parse_args()
if not 0<=a.first<=a.last or not a.base<=a.target<a.base+42:p.error('invalid window or cell target')
hdata=a.health.read_bytes();health=json.loads(hdata)
if health.get('sample')!='Benchmark/Rain':raise ValueError('not Rain health data')
frames={h['i']:h for h in health['frames']}
if len(frames)!=len(health['frames']):raise ValueError('duplicate health frame')
required=set(range(a.first,a.last+1))
if not required<=frames.keys():raise ValueError('health window incomplete')
a.out.parent.mkdir(parents=True,exist_ok=True)
partial=a.out.with_suffix(a.out.suffix+'.partial')
summary=[];window_hash=hashlib.sha256();seen=set();mapping_history={}
opener=gzip.open if a.trace.suffix=='.gz' else open
with opener(a.trace,'rb') as stream, partial.open('w') as output:
 for number,line in enumerate(stream,1):
  if number<a.first+1:continue
  if number>a.last+1:break
  d=json.loads(line);h=frames[number-1]
  if d['schema']!='gpu-core-state-v19' or d['frame']!=number:raise ValueError('schema/frame mismatch')
  mapping=health_to_core_creations(h,d)
  cell_creations=range(a.base,a.base+42)
  if any(c not in mapping for c in cell_creations):raise ValueError('cell lifetime not live for entire window')
  selected={c:mapping[c] for c in cell_creations};cell=set(selected.values())
  for c,identity in selected.items():
   if mapping_history.setdefault(c,identity)!=identity:raise ValueError('core creation changed within lifetime')
  bodies={b['identity']:b for b in d['bodies']}
  for b in h['bodies']:
   if b['creation'] not in selected:continue
   core=bodies[selected[b['creation']]]
   for hk,sk in [('q','rot'),('v','vel'),('w','omega')]:
    if [struct.pack('>f',v).hex() for v in b[hk]]!=core[sk]:raise ValueError(f'health/core physical mismatch at frame {number}: {hk}')
  joints=[j for j in d['joints'] if j['a'] in cell or j['b'] in cell]
  contacts=[c for c in d['contacts'] if c['a'] in cell or c['b'] in cell]
  targets=[j for j in joints if j['kind']==3 and j['b']==selected[a.target]]
  if len(targets)!=1:raise ValueError('target spherical joint ambiguous or absent')
  connected=cell|{c[k] for c in contacts+joints for k in ('a','b')}
  order=d['body_storage_order'];starts=d['host_state']['step_start_bodies']
  if len(order)!=len(starts) or len(set(order))!=len(order):raise ValueError('step-start body mapping invalid')
  start_by_id=dict(zip(order,starts))
  row=dict(schema='rain-cell-diagnostic-excerpt-v1',frame=number,health_frame=h['i'],
           settings=d['settings'],human_to_core=selected,target_joint=targets[0]['identity'],
           bodies=[bodies[i] for i in sorted(connected)],cell_bodies=sorted(cell),
           step_start_bodies={i:start_by_id[i] for i in sorted(connected)},joints=joints,contacts=contacts,
           health_bodies=[b for b in h['bodies'] if b['creation'] in selected],
           health_joints=[j for j in h['joints'] if any(j[k]==b['id'] and j[k+'_generation']==b['generation'] for k in ('body_a','body_b') for b in h['bodies'] if b['creation']==a.target)])
  output.write(json.dumps(row,separators=(',',':'))+'\n')
  window_hash.update(line);seen.add(h['i'])
  summary.append(dict(frame=number,cell_bodies=len(cell),external_bodies=len(connected-cell),joints=len(joints),contact_patches=len(contacts),touching_patches=sum(c['count']>0 for c in contacts),target_core=selected[a.target]))
if seen!=required:raise ValueError('trace window incomplete')
partial.replace(a.out)
result=dict(scope='diagnostic subset, not a repeatability qualification trace',trace=str(a.trace.resolve()),health=str(a.health.resolve()),health_sha256=hashlib.sha256(hdata).hexdigest(),selected_raw_lines_sha256=window_hash.hexdigest(),first_health=a.first,last_health=a.last,base=a.base,target=a.target,rows=summary)
a.out.with_suffix('.manifest.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(dict(frames=len(summary),first=summary[0],last=summary[-1])))
