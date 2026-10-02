#!/usr/bin/env python3
"""Offline validation of exact current PR03 Rust baseline, including failures."""
from pathlib import Path
import json,hashlib,gzip,tarfile,re
B=Path(__file__).resolve().parent;raw=B/'raw';sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();idx=json.loads((B/'raw-index.json').read_text());read=lambda p:json.loads((raw/p).read_text())
assert set(idx)=={str(f.relative_to(B)) for f in raw.rglob('*') if f.is_file()}
for n,v in idx.items():assert sha(B/n)==v['sha256'] and (B/n).stat().st_size==v['bytes'],n
p=read('protocol.json');r=read('receipt.json');assert r['protocol_sha256']==sha(raw/'protocol.json') and r['driver_sha256']==sha(raw/'run.py');assert r['status']=='completed-baseline' and len(p['cases'])==len(r['results'])==p['budget']['processes']==33
assert (p['budget']['ordinary_selected_checks'],p['budget']['native_selected_checks'])==(99,100);assert all(p['budget'][k]==0 for k in ['builds','candidates','retries','headline_timing'])
e=read('producers/engine-build.json');v=read('producers/viewer-parent.json');assert len(e['inputs_after'])==107 and e['inputs_before']==e['inputs_after'];assert len(v['inputs_after'])==723 and v['inputs_before']==v['inputs_after']
with tarfile.open(raw/'compiled-source-inputs.tar.gz') as t:
 for n,h in e['inputs_after'].items():assert hashlib.sha256(t.extractfile(n).read()).hexdigest()==h,n
with tarfile.open(B.parent/'pr02-world-lifetime-viewer-builds-2026-10-02/raw/compiled-source-inputs.tar.gz') as t:
 for n,h in v['inputs_after'].items():assert hashlib.sha256(t.extractfile(n[6:] if n.startswith('../../') else n).read()).hexdigest()==h,n
all_selected={};failures=[];checks=frames=passes=0
for cell,d in p['cells'].items():
 f=raw/(cell+'-host-list.txt');assert sha(f)==d['host_list_sha256'];listed={l[:-6] for l in f.read_text().splitlines() if l.endswith(': test')};assert set(d['exact_selected_names'])<=listed and len(d['exact_selected_names'])==d['selected_count'];assert len([n for n in listed if 'ccd' in n])==d['CCD_selected_count']==29;all_selected[cell]=set()
for c,row in zip(p['cases'],r['results']):
 out=raw/c['name'];assert c['name']==row['name'];names=set(c['selectors']);assert not all_selected[c['cell']]&names;all_selected[c['cell']]|=names
 assert c['command']==[p['cells'][c['cell']]['test_executable']['path'],*c['selectors'],'--exact','--nocapture','--test-threads=1'];assert c['environment']['GPU_PHYSICS_LIVE_CONTACT_ORDER']=='0'
 for channel in ['stdout','stderr']:assert sha(out/(channel+'.log'))==row[channel+'_sha256']
 text=(out/'stdout.log').read_text();err=(out/'stderr.log').read_text();actual=re.findall(r'^test ([^ ]+) \.\.\. (ok|FAILED)',text,re.M);assert set(n for n,status in actual)==names and len(actual)==len(names);checks+=len(actual);passes+=sum(status=='ok' for _,status in actual)
 assert row['actual_NVIDIA_Vulkan'] and 'NVIDIA GeForce RTX 4070 SUPER' in err and 'Vulkan' in err
 result=re.search(r'test result: (ok|FAILED)\. (\d+) passed; (\d+) failed; (\d+) ignored;',text);assert list(result.groups())==row['harness_result'] and int(result[2])+int(result[3])==len(names) and int(result[4])==0
 assert row['exit']==(101 if int(result[3]) else 0);assert row['valid']==(int(result[3])==0)
 for n,status in actual:
  if status=='FAILED':failures.append((c['cell'],n))
 if 'expected_trace_frames' in c:
  assert row['valid'];compressed=out/'state.jsonl.gz';assert sha(compressed)==row['trace_gzip_sha256']
  h=hashlib.sha256();count=0;size=0
  with gzip.open(compressed,'rb') as f:
   for count,line in enumerate(f,1):
    h.update(line);size+=len(line);frame=json.loads(line);assert frame['schema']=='gpu-core-state-v24' and frame['frame']==count and all(isinstance(frame[k],list) for k in ['bodies','contacts','joints'])
  assert count==row['trace_frames']==c['expected_trace_frames'] and size==row['raw_trace_bytes'] and h.hexdigest()==row['raw_trace_sha256'];frames+=count
  if 'replay_hits' in row:
   hits=row['replay_hits'];assert len(hits)==48 and hits[-1]==42 and hits[18]>=16 and hits[30]>=hits[23]+5 and hits[47]>=hits[32]+12
for cell,d in p['cells'].items():assert all_selected[cell]==set(d['exact_selected_names'])
expected={'api::world::world_counter_tests::contact_order_survives_body_capacity_growth_and_invalidates_proxy_changes','gpu_invariants::high_resistance_sleeper_wakes_on_velocity'};assert set(failures)=={(cell,n) for cell in p['cells'] for n in expected};assert checks==199 and passes==195 and frames==2864 and not r['all_assertions_pass']
print(f'Validated {len(idx)} rawfiles:33processes,199selectedchecks,195passes/4retainedfailures,25traces/2864frames. No repeat/performance/full-state qualification claim.')
