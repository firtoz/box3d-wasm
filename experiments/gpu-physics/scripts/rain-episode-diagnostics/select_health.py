"""Read the native writer's one-frame-per-line JSON without materializing 1GB JSON.
Preserve original evidence; emit explicitly labelled diagnostic cell selections.
"""
from pathlib import Path
import json,hashlib,sys
mode=sys.argv[1]
root=Path('artifacts/gpu-solver-qualification')
source=root/'rain-buffered-five'/mode/'run-1/attempt-0001/health.json'
windows={'twist':(374,463,2353,2379),'cone':(514,525,4999,5004)}
selected={k:[] for k in windows};sha=hashlib.sha256();prefix=[];count=0;found=False;trailer=[];last_comma=None
with source.open('rb') as stream:
 for raw in stream:
  sha.update(raw)
  if not found:
   prefix.append(raw.decode())
   if raw.strip()==b'"frames": [':found=True
   continue
  stripped=raw.strip()
  if stripped.startswith(b'{"sample":'):
   if trailer:raise ValueError('frame after trailer')
   if count and not last_comma:raise ValueError('missing frame separator')
   last_comma=stripped.endswith(b',')
   d=json.loads(stripped[:-1] if last_comma else stripped)
   if d['i']!=count or d['completed_step']!=count+1 or d['sample']!='Benchmark/Rain':raise ValueError('health frame sequence mismatch')
   count+=1
   for name,(first,last,base,target) in windows.items():
    if first<=d['i']<=last:
     bodies=[b for b in d['bodies'] if base<=b['creation']<base+42]
     if len(bodies)!=42:raise ValueError('cell not fully live')
     keys={(b['id'],b['generation']) for b in bodies}
     joints=[j for j in d['joints'] if any((j[k],j[k+'_generation']) in keys for k in ['body_a','body_b'])]
     selected[name].append({**d,'bodies':bodies,'joints':joints})
  else:trailer.append(raw.decode())
if not found or ''.join(trailer).strip()!=']\n}' or last_comma:raise ValueError('invalid document ending')
metadata=json.loads(''.join(prefix)+']}')
if metadata['status']!='ok' or metadata['frames_observed']!=count or count!=600:raise ValueError('incomplete health document')
summary={'source':str(source.resolve()),'sha256':sha.hexdigest(),'frames':count,'status':metadata['status'],'selections':{}}
for name,rows in selected.items():
 first,last,base,target=windows[name]
 if len(rows)!=last-first+1:raise ValueError('incomplete selection')
 out=root/'rain-later-full-state'/f'{mode}-{name}-health-selection.json'
 out.write_text(json.dumps({**metadata,'frames':rows,'diagnostic_selection':{'original_frames':count,'source_sha256':sha.hexdigest(),'first':first,'last':last,'base':base,'target':target}},separators=(',',':'))+'\n')
 summary['selections'][name]={'path':str(out),'frames':len(rows),'base':base,'target':target}
(root/'rain-later-full-state'/f'{mode}-health-validation.json').write_text(json.dumps(summary,indent=2)+'\n')
print(json.dumps(summary,indent=2))
