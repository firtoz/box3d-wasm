#!/usr/bin/env python3
"""Publish retained PR01 evidence without large binaries or generated build trees."""
from pathlib import Path
import argparse,gzip,hashlib,json,subprocess
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
sha=lambda data:hashlib.sha256(data).hexdigest()

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--artifacts',type=Path,default=ROOT/'artifacts/production-readiness/pr01-speculative');a=p.parse_args();source=a.artifacts.resolve()
 exclude={'baseline-source','baseline-cmake','cpu-cmake','candidate-cmake-ordinary-gpu','cmake','pipelines','oracle-build','xvfb','previous-cpu-column','__pycache__'}
 index=[]
 for path in sorted(source.rglob('*')):
  relative=path.relative_to(source)
  if not path.is_file() or any(part in exclude for part in relative.parts):continue
  if path.suffix not in {'.json','.jsonl','.log','.stdout','.stderr','.diff','.py','.cpp','.rs','.wgsl'} and path.name not in {'stdout','stderr'}:continue
  data=path.read_bytes();destination=HERE/'raw'/relative
  if path.suffix=='.jsonl':destination=destination.with_suffix('.jsonl.gz');payload=gzip.compress(data,mtime=0)
  else:payload=data
  destination.parent.mkdir(parents=True,exist_ok=True);destination.write_bytes(payload)
  index.append({'artifact_path':str(relative),'portable_path':str(destination.relative_to(HERE)),'raw_sha256':sha(data),'stored_sha256':sha(payload),'raw_bytes':len(data),'stored_bytes':len(payload)})
 # Every engine receipt gets a source applicability audit. Tracked unchanged files
 # resolve from the build's base commit; dirty variants are retained as overlays.
 overlays=HERE/'source-overlays';source_index=[]
 for receipt in sorted(source.rglob('*.json')):
  if any(part in exclude for part in receipt.relative_to(source).parts):continue
  try:d=json.loads(receipt.read_text())
  except (ValueError,UnicodeError):continue
  inputs=d.get('engine_sources');base=d.get('revision_at_build',d.get('revision'))
  if inputs and not base and receipt.name.startswith('test-build-'):
   library_receipt=receipt.with_name(receipt.name.replace('test-build-','build-'))
   base=json.loads(library_receipt.read_text())['revision_at_build']
  if not inputs or not base:continue
  overlay={};unresolved=[]
  for name,expected in inputs.items():
   data=None
   try:
    candidate=subprocess.check_output(['git','show',f'{base}:experiments/gpu-physics/{name}'],cwd=ROOT,stderr=subprocess.DEVNULL)
    if sha(candidate)==expected:continue
   except subprocess.CalledProcessError:pass
   current=ROOT/name
   if current.is_file() and sha(current.read_bytes())==expected:data=current.read_bytes()
   if data is None:
    for rejected in (source/'rejected-source-inputs').glob(f'*/{name}'):
     if sha(rejected.read_bytes())==expected:data=rejected.read_bytes();break
   if data is None:unresolved.append(name);continue
   destination=overlays/expected[:16]/name;destination.parent.mkdir(parents=True,exist_ok=True);destination.write_bytes(data);overlay[name]=str(destination.relative_to(HERE))
  source_index.append({'receipt':str(receipt.relative_to(source)),'base_revision':base,'base_path':'experiments/gpu-physics','overlays':overlay,'unresolved':unresolved})
  if unresolved:raise SystemExit(f'Unresolved compiled inputs: {receipt}: {unresolved}')
 (HERE/'source-index.json').write_text(json.dumps(source_index,indent=2)+'\n')
 (HERE/'raw-index.json').write_text(json.dumps(index,indent=2)+'\n')
 print('Published',len(index),'raw files;',sum(x['stored_bytes'] for x in index),'bytes; compiled source inputs verified for',len(source_index),'receipts')
if __name__=='__main__':main()
