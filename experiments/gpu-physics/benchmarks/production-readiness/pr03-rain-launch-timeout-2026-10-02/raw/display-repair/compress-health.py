from pathlib import Path
import hashlib,json,gzip,os
A=Path(__file__).resolve().parent;limit=48*1024*1024
class Parts:
 def __init__(self,d):self.d=d;self.f=None;self.size=0;self.paths=[]
 def write(self,data):
  total=len(data)
  while data:
   if self.f is None or self.size==limit:
    if self.f:self.f.close()
    p=self.d/('health.json.gz.part%03d'%len(self.paths));assert not p.exists();self.paths.append(p);self.f=p.open('wb');self.size=0
   n=min(len(data),limit-self.size);self.f.write(data[:n]);self.size+=n;data=data[n:]
  return total
 def flush(self):
  if self.f:self.f.flush()
 def close(self):
  if self.f:self.f.close()
for cell in ['cpu','ordinary','native']:
 raw=A/(cell+'-rain')/'health.json'
 if not raw.exists():continue
 d=A/'portable-health'/(cell+'-rain');d.mkdir(parents=True,exist_ok=True);manifest=d/'manifest.json';assert not manifest.exists();receipt=json.loads((A/'receipt.json').read_text());row=next((r for r in receipt['results'] if r['name']==cell+'-rain'),None);assert row and row['child_exit']==row['wrapper_exit']==0
 sha=hashlib.sha256();parts=Parts(d);size=0
 with raw.open('rb') as inp,gzip.GzipFile(filename='',mode='wb',fileobj=parts,compresslevel=1,mtime=0) as dest:
  while chunk:=inp.read(1024*1024):sha.update(chunk);size+=len(chunk);dest.write(chunk)
 parts.close();assert sha.hexdigest()==row['health_sha256'];data={'raw_bytes':size,'raw_sha256':sha.hexdigest(),'format':'Concatenate parts in order, decompress single gzip stream; exact original health.json bytes.48MiB maximum part size.','parts':[{'name':p.name,'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in parts.paths]};manifest.write_text(json.dumps(data,indent=2)+'\n');print(cell,'compressed',size,'bytes to',sum(x['bytes'] for x in data['parts']),'bytes in',len(parts.paths),'parts',flush=True)
