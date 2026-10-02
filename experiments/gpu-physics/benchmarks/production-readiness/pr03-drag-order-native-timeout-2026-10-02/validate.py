#!/usr/bin/env python3
"""Validate original complete paired dragging data and retained timeout/failures."""
from pathlib import Path
import json,hashlib,gzip,re,math
B=Path(__file__).resolve().parent;raw=B/'raw';sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();read=lambda n:json.loads((raw/n).read_text());idx=json.loads((B/'raw-index.json').read_text());assert set(idx)=={str(f.relative_to(B)) for f in raw.rglob('*') if f.is_file()}
for n,v in idx.items():assert sha(B/n)==v['sha256'] and (B/n).stat().st_size==v['bytes'],n
p=read('protocol.json');r=read('receipt.json');assert r['protocol_sha256']==sha(raw/'protocol.json') and r['driver_sha256']==sha(raw/'run.py');assert r['status']=='stopped' and len(r['results'])==p['budget']['processes']==7 and not r['unlaunched'];assert p['watchdog_seconds_per_process']==900
assert all(p['budget'][k]==0 for k in ['builds','candidates','retries','headline_timing']);prior=read('previous/receipt.json');assert prior['results'][0]['child_exit']==0 and prior['results'][1]['wrapper_exit']=='timeout';assert set(c['name'] for c in p['cases'])==set(prior['unlaunched'])
for key in ['protocol','receipt']:assert sha(raw/'previous'/(key+'.json'))==p['previous_attempt'][key+'_sha256']
assert sha(raw/'display-child.py')==p['display_dependency']['helper_sha256'];assert p['settings']==read('previous/protocol.json')['settings'];refs=read('portable-references.json')
for n,v in refs.items():assert sha(B.parent/v['portable'])==v['sha256']
for path,h in p['producer_receipts'].items():assert refs[path]['sha256']==h
assert refs[p['fixture_link_receipt']]['sha256']==p['fixture_link_receipt_sha256'];sources=B.parent/'pr03-rain-launch-timeout-2026-10-02/raw/original-launcher/evaluator-source'
for n,h in p['source_hashes'].items():assert sha(sources/n)==h
traces=read('traces.json');analyses={}
for c,row in zip(p['cases'],r['results']):
 assert c['name']==row['name'];out=raw/c['name'];text=(out/'stdout.log').read_text();err=(out/'stderr.log').read_text();assert row['actual_NVIDIA_Vulkan'] and 'NVIDIA GeForce RTX 4070 SUPER' in err and 'Vulkan' in err
 for ch in ['stdout','stderr']:assert sha(out/(ch+'.log'))==row[ch+'_sha256']
 if c['kind']=='rain':
  assert row['wrapper_exit']=='timeout' and row['child_exit'] is None and not row['health_exists'] and not(out/'health.json').exists();continue
 if c['kind']=='order-cache':
  assert c['environment']['GPU_PHYSICS_LIVE_CONTACT_ORDER']=='1' and row['wrapper_exit']==0 and row['harness_result']==['ok','1','0'];assert 'test result: ok. 1 passed; 0 failed; 0 ignored;' in text;continue
 assert c['environment']['BOTH_DRAG_TRACE_BODY']=='-1' and c['environment']['GPU_PHYSICS_LIVE_CONTACT_ORDER']=='0';ground='--ground' in c['command'];expected=3060 if ground else 1320;assert row['trace_last_frame']==expected-1;v=traces[c['name']];gz=out/'comparison.txt.gz';assert sha(gz)==v['gzip_sha256'];data=gzip.decompress(gz.read_bytes());assert len(data)==v['raw_bytes'] and hashlib.sha256(data).hexdigest()==v['raw_sha256']==row['paired_trace_sha256'];poses={};current=None
 for line in data.decode().splitlines():
  t=line.split()
  if t[0]=='B':current=(int(t[1]),int(t[2]));assert 0<=current[0]<expected and 0<=current[1]<5
  elif t[0]=='F':
   assert len(t)==20 and (t[3],t[7],t[12],t[16])==('p','q','v','w');key=(int(t[1]),current[1],int(t[2]));assert key[0]==current[0] and key not in poses and key[2] in (0,1);values=tuple(map(float,t[4:7]+t[8:12]+t[13:16]+t[17:20]));assert len(values)==13 and all(math.isfinite(x) for x in values) and abs(sum(x*x for x in values[3:7])-1)<.01;poses[key]=values
  else:assert t[0] in ('M','P') and all(math.isfinite(float(x)) for x in t if x not in ['M','P','count','normal','id','a','b','sep','imp'])
 assert len(poses)==expected*5*2;assert set(poses)=={(f,b,e) for f in range(expected) for b in range(5) for e in range(2)};first={};peak={k:{'value':0,'frame':0,'body':0} for k in ['position','quaternion_chord','velocity','held_position','held_quaternion_chord','held_velocity']}
 for f in range(expected):
  for b in range(5):
   a=poses[f,b,0];g=poses[f,b,1];sign=-1 if sum(x*y for x,y in zip(a[3:7],g[3:7]))<0 else 1;errors={'position':math.dist(a[:3],g[:3]),'quaternion_chord':math.sqrt(sum((x-sign*y)**2 for x,y in zip(a[3:7],g[3:7]))),'velocity':math.dist(a[7:10],g[7:10])};held=ground and f>=60 and (f-60)%300<180 and b==((f-60)//300)%5
   if held:errors|={'held_'+k:value for k,value in errors.items()}
   limits={'position':.025 if ground else .0005,'quaternion_chord':.025 if ground else .0005,'velocity':.5 if ground else .005,'held_position':.005,'held_quaternion_chord':.01,'held_velocity':.1}
   for k,value in errors.items():
    if value>peak[k]['value']:peak[k]={'value':value,'frame':f,'body':b}
    if value>limits[k] and k not in first:first[k]={'frame':f,'body':b,'value':value,'limit':limits[k]}
   if ground and f>=60 and (f-60)%300==299:
    assert all(math.sqrt(sum(x*x for x in q[7:10]))<.1 and math.sqrt(sum(x*x for x in q[10:13]))<.1 for q in [a,g])
 assert row['wrapper_exit']==(1 if ground else 0);assert ('FAIL ground-contact comparison' if ground else 'PASS isolated drag comparison') in text
 analyses[c['name']]={'frames':expected,'paired_body_observations':expected*5,'source_assertion_result':'FAIL' if ground else 'PASS','derived_first_screen_exceedances':first,'derived_peaks':peak,'scope':'Trace diagnosis uses printed9significant-digit values; original executable assertions/exit remain authoritative. No changed physical tolerance or full-state claim.'}
assert set(traces)==set(analyses)
if __name__=='__main__':
 if '--write-analysis' in __import__('sys').argv:(B/'trace-analysis.json').write_text(json.dumps(analyses,indent=2)+'\n')
 print(f'Validated{len(idx)}rawfiles: fourcomplete paired traces/43800paired-body observations; isolateddrag/order1PASS and grounddragFAIL bothbackends; nativeRain900second timeout retained.')
