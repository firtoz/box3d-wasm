#!/usr/bin/env python3
"""Offline byte/capture validation; timeout is retained, never a GPU pass."""
from pathlib import Path
import json,hashlib,gzip,io,sys,importlib.util
B=Path(__file__).resolve().parent;raw=B/'raw';sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();read=lambda n:json.loads((raw/n).read_text());idx=json.loads((B/'raw-index.json').read_text())
assert set(idx)=={str(f.relative_to(B)) for f in raw.rglob('*') if f.is_file()}
for n,v in idx.items():assert sha(B/n)==v['sha256'] and (B/n).stat().st_size==v['bytes'],n
origin=read('original-launcher/protocol.json');failed=read('original-launcher/receipt.json');p=read('display-repair/protocol.json');r=read('display-repair/receipt.json');assert failed['status']=='stopped' and not failed['results'] and 'FileNotFoundError' in failed['error']
for d,receipt in [('original-launcher',failed),('display-repair',r)]:
 assert receipt['protocol_sha256']==sha(raw/d/'protocol.json') and receipt['driver_sha256']==sha(raw/d/'run.py')
assert p['previous_attempt']['protocol_sha256']==sha(raw/'original-launcher/protocol.json') and p['previous_attempt']['receipt_sha256']==sha(raw/'original-launcher/receipt.json');assert p['previous_attempt']['engine_processes_consumed']==0
assert p['budget']==origin['budget']|{'isolated_Xvfb_servers':3} and len(p['cases'])==9 and p['watchdog_seconds_per_process']==900
assert all(p['budget'][n]==0 for n in ['builds','candidates','retries','headline_timing']);assert p['settings']==origin['settings'] and p['source_hashes']==origin['source_hashes']
assert r['status']=='stopped' and len(r['results'])==2 and r['unlaunched']==[c['name'] for c in p['cases'][2:]]
a,b=r['results'];assert a['name']=='cpu-rain' and a['child_exit']==a['wrapper_exit']==0 and a['sokol_errors']==[['600','0']];assert b['name']=='ordinary-rain' and b['wrapper_exit']=='timeout' and b['child_exit'] is None and not b['health_exists'] and b['actual_NVIDIA_Vulkan']
assert 'NVIDIA GeForce RTX 4070 SUPER' in (raw/'display-repair/ordinary-rain/stderr.log').read_text() and 'Vulkan' in (raw/'display-repair/ordinary-rain/stderr.log').read_text();assert not(raw/'display-repair/ordinary-rain/health.json').exists()
for row in r['results']:
 for ch in ['stdout','stderr']:assert sha(raw/'display-repair'/row['name']/(ch+'.log'))==row[ch+'_sha256']
assert read('display-repair/cpu-rain/child-exit.json')['exit']==0 and read('display-repair/cpu-rain/display-exit.json')['child_exit']==0
refs=read('portable-references.json')
for n,v in refs.items():assert sha(B.parent/v['portable'])==v['sha256'],n
for path,h in p['producer_receipts'].items():
 assert refs[path]['sha256']==h;d=json.loads((B.parent/refs[path]['portable']).read_text());assert d['inputs_before']==d['inputs_after']
for name,d in p['viewers'].items():
 v=refs[d['receipt']];assert v['sha256']==d['receipt_sha256'];j=json.loads((B.parent/v['portable']).read_text());assert j['status']=='built' and j['executable_sha256']==d['binary_sha256'] and len(j['compiled_units'])==d['compiled_unit_count']
assert refs[p['fixture_link_receipt']]['sha256']==p['fixture_link_receipt_sha256'] and refs[p['display_dependency']['prior_successful_protocol']]['sha256']==p['display_dependency']['prior_protocol_sha256'];assert sha(raw/'display-repair/display-child.py')==p['display_dependency']['helper_sha256']
for n,h in p['source_hashes'].items():assert sha(raw/'original-launcher/evaluator-source'/n)==h
D=raw/'display-repair/portable-health/cpu-rain';m=json.loads((D/'manifest.json').read_text());roundtrip=read('display-repair/cpu-health-roundtrip.json');assert roundtrip['manifest_sha256']==sha(D/'manifest.json') and roundtrip['verification_recipe_sha256']==sha(raw/'display-repair/verify-compressed-health.py');assert a['health_sha256']==m['raw_sha256']==roundtrip['restored_sha256'] and a['health_bytes']==m['raw_bytes']==roundtrip['restored_bytes']
class Parts:
 def __init__(self):self.paths=iter(D/v['name'] for v in m['parts']);self.f=None
 def read(self,n=-1):
  assert n>=0;chunks=[]
  while n:
   if self.f is None:
    try:self.f=next(self.paths).open('rb')
    except StopIteration:break
   b=self.f.read(n)
   if not b:self.f.close();self.f=None;continue
   chunks.append(b);n-=len(b)
  return b''.join(chunks)
def stream():return gzip.GzipFile(fileobj=Parts(),mode='rb')
for v in m['parts']:assert (D/v['name']).stat().st_size==v['bytes']<=48*1024*1024 and sha(D/v['name'])==v['sha256']
h=hashlib.sha256();size=0
with stream() as f:
 while chunk:=f.read(1024*1024):h.update(chunk);size+=len(chunk)
assert size==m['raw_bytes']==1203549772 and h.hexdigest()==m['raw_sha256'];sys.dont_write_bytecode=True;sys.path.insert(0,str(raw/'original-launcher/evaluator-source/scripts'));spec=importlib.util.spec_from_file_location('rain',raw/'original-launcher/evaluator-source/scripts/compare-rain-lifetimes.py');rain=importlib.util.module_from_spec(spec);spec.loader.exec_module(rain);from native_scene_validate import record_complete
with io.TextIOWrapper(stream(),encoding='utf-8') as f:
 prefix=[]
 for line in f:
  if line.strip()=='"frames": [':break
  prefix.append(line)
 header=json.loads(''.join(prefix)+'"frames": []\n}')
assert (header['worker_count'],header['enable_sleep'],header['unpaced'],header['completed_step_mode'])==(8,True,True,False)
class Health:
 def open(self):return io.TextIOWrapper(stream(),encoding='utf-8')
count=bodies=joints=0
for count,frame in enumerate(rain.records(Health(),600),1):
 status,detail=record_complete(dict(header,warmup=count-1,timed=1,measured=1,frames=[dict(frame,i=0)]),'Benchmark/Rain',1);assert status=='ok',detail
 for joint in frame['joints']:rain.spherical_limits(joint,True)
 bodies+=len(frame['bodies']);joints+=len(frame['joints'])
assert count==600 and bodies==joints==1948800
print(f'Validated{len(idx)}rawfiles: exact CPU600 record/{bodies}body-joint observations; missing-launcher failure and GPU900second timeout retained. No GPU/performance/full-state pass.')
