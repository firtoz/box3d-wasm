"""Read the native writer's one-frame-per-line JSON without materializing 1GB JSON.
Preserve original evidence; emit explicitly labelled diagnostic cell selections.
"""
from pathlib import Path
import argparse,json,hashlib
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('mode', choices=['ordinary','native','cpu'])
p.add_argument('--source',type=Path,help='Complete Rain health report; defaults to the historical batch')
p.add_argument('--out-dir',type=Path,help='Directory for labelled selections and validation receipt')
p.add_argument('--windows',type=Path,help='JSON object mapping labels to [first, last, cell base creation, target creation]')
p.add_argument('--frames',type=int,default=600)
a=p.parse_args();mode=a.mode
root=Path('artifacts/gpu-solver-qualification')
source=a.source or root/'rain-buffered-five'/mode/'run-1/attempt-0001/health.json'
out_dir=a.out_dir or root/'rain-later-full-state'
if a.source and not a.windows:p.error('--source requires --windows; episode identities must be selected for this capture')
windows=json.loads(a.windows.read_text()) if a.windows else {'twist':(374,463,2353,2379),'cone':(514,525,4999,5004)}
if not isinstance(windows,dict) or not windows:p.error('nonempty windows object required')
for name,window in windows.items():
 if not name or any(c not in 'abcdefghijklmnopqrstuvwxyz0123456789-_' for c in name):p.error('window labels must use lowercase letters, digits, - or _')
 if not isinstance(window,(list,tuple)) or len(window)!=4 or any(type(v) is not int for v in window):p.error('window must contain four integers')
 first,last,base,target=window
 if not 0<=first<=last<a.frames or base<1 or not base<=target<base+42:p.error('invalid window or cell target')
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
if metadata['status']!='ok' or metadata['frames_observed']!=count or count!=a.frames:raise ValueError('incomplete health document')
summary={'source':str(source.resolve()),'sha256':sha.hexdigest(),'frames':count,'status':metadata['status'],'selections':{}}
out_dir.mkdir(parents=True,exist_ok=True)
for name,rows in selected.items():
 first,last,base,target=windows[name]
 if len(rows)!=last-first+1:raise ValueError('incomplete selection')
 out=out_dir/f'{mode}-{name}-health-selection.json'
 out.write_text(json.dumps({**metadata,'frames':rows,'diagnostic_selection':{'original_frames':count,'source_sha256':sha.hexdigest(),'first':first,'last':last,'base':base,'target':target}},separators=(',',':'))+'\n')
 summary['selections'][name]={'path':str(out),'frames':len(rows),'base':base,'target':target}
(out_dir/f'{mode}-health-validation.json').write_text(json.dumps(summary,indent=2)+'\n')
print(json.dumps(summary,indent=2))
