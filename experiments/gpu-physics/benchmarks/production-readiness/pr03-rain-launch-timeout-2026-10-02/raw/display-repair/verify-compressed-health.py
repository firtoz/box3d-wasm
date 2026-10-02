from pathlib import Path
import hashlib,json,gzip
A=Path(__file__).resolve().parent;D=A/'portable-health/cpu-rain';M=D/'manifest.json';m=json.loads(M.read_text())
class PartStream:
 def __init__(self,paths):self.paths=iter(paths);self.f=None
 def read(self,n=-1):
  assert n>=0
  chunks=[]
  while n:
   if self.f is None:
    try:self.f=next(self.paths).open('rb')
    except StopIteration:break
   b=self.f.read(n)
   if not b:self.f.close();self.f=None;continue
   chunks.append(b);n-=len(b)
  return b''.join(chunks)
paths=[]
for part in m['parts']:
 f=D/part['name'];assert f.stat().st_size==part['bytes']<=48*1024*1024;assert hashlib.sha256(f.read_bytes()).hexdigest()==part['sha256'];paths.append(f)
h=hashlib.sha256();size=0
with gzip.GzipFile(fileobj=PartStream(paths),mode='rb') as f:
 while b:=f.read(1024*1024):h.update(b);size+=len(b)
assert size==m['raw_bytes'] and h.hexdigest()==m['raw_sha256']
r=json.loads((A/'receipt.json').read_text());row=next(x for x in r['results'] if x['name']=='cpu-rain');assert row['health_sha256']==h.hexdigest() and row['health_bytes']==size
result={'complete':True,'manifest_sha256':hashlib.sha256(M.read_bytes()).hexdigest(),'verification_recipe_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'restored_sha256':h.hexdigest(),'restored_bytes':size,'parts_verified':len(paths),'CPU_observations':json.loads((A/'cpu-health-summary.json').read_text()),'claim':'Lossless decompression and original receipt identity; no GPU or performance qualification.'};(A/'cpu-health-roundtrip.json').write_text(json.dumps(result,indent=2)+'\n');print('Verified exactCPUhealth roundtrip',size,'bytes',len(paths),'parts')
