#!/usr/bin/env python3
"""Verify current PR01 portable evidence, without launching an engine."""
from pathlib import Path
import json,hashlib,gzip,importlib.util,tempfile
B=Path(__file__).resolve().parent;sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();read=lambda p:json.loads((B/p).read_text());idx=read('raw-index.json')
for n,h in idx.items():assert sha(B/n)==h,n
p=read('protocol.json');s=read('raw/receipt.json');a=read('acceptance.json');assert s['status']=='passed' and s['current_sources_unchanged'] and s['protocol_sha256']==sha(B/'protocol.json');assert len(s['builds'])==6 and len(s['host_lists'])==2 and len(s['runs'])==34;assert all(x['exit']==0 for x in s['builds']+s['host_lists']+s['runs']);assert all(x['status']=='pass' for x in a['requirements']) and not a['release_ready']
assert p['budget']=={'C_link_builds':6,'public_control_fresh_processes':20,'ccd_guard_fresh_processes':2,'state_capture_fresh_processes':10,'regression_serial_processes':2,'regression_test_executions':126,'host_test_list_processes':2,'Rust_or_viewer_builds':0,'headline_timing':0,'retries':0}
for cell in p['cells']:
 rows=[x for x in s['runs'] if x['kind']=='public-control' and x['cell']==cell];assert len(rows)==5 and len({x['stdout_sha256'] for x in rows})==1
for row in s['runs']:
 folder=B/'raw/runs'/row['kind']/row['cell']/str(row['trial']);assert sha(folder/'stdout.log')==row['stdout_sha256'] and sha(folder/'stderr.log')==row['stderr_sha256'];stdout=(folder/'stdout.log').read_text();err=(folder/'stderr.log').read_text();assert 'NVIDIA GeForce RTX 4070 SUPER | backend=Vulkan' in stdout+err
 if row['kind']=='public-control':assert 'speculative-controls=pass' in stdout
 elif row['kind']=='ccd-guard':assert 'speculative-ccd-guard=pass' in stdout
 elif row['kind']=='state-capture':assert '1 passed; 0 failed' in stdout
 elif row['kind']=='regression':assert '63 passed; 0 failed' in stdout
for name,path in [('engine-build',p['engine_build_receipt']),('viewer-parent',p['viewer_parent_receipt'])]:
 d=read('raw/origin-receipts/'+name+'.json');assert d['status']=='built' and d['inputs_before']==d['inputs_after'];assert sha(B/'raw/origin-receipts'/('engine-build.json' if name=='engine-build' else 'viewer-parent.json'))==p['engine_build_receipt_sha256' if name=='engine-build' else 'viewer_parent_receipt_sha256']
for cell,d in p['cells'].items():assert sha(B/'raw/origin-receipts'/(cell+'.json'))==d['receipt_sha256']
# Execute the exact archived comparison/health tools as offline data readers only.
import tarfile
with tempfile.TemporaryDirectory() as temp:
 T=Path(temp)
 with tarfile.open(B/'raw/fixture-and-runner-inputs.tar.gz') as t:
  for name in ['scripts/check-core-state-health.py','scripts/compare-core-state.py']:
   data=t.extractfile(name).read();assert hashlib.sha256(data).hexdigest()==p['sources'][name];(T/Path(name).name).write_bytes(data)
 def mod(name):
  spec=importlib.util.spec_from_file_location(name,T/(name+'.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
 health=mod('check-core-state-health');compare=mod('compare-core-state');compressed=read('compressed-traces.json');assert len(compressed)==10
 for backend in ['ordinary','native']:
  paths=[]
  for trial in range(1,6):
   n=f'raw/runs/state-capture/{backend}/{trial}/state.jsonl.gz';data=gzip.decompress((B/n).read_bytes());assert hashlib.sha256(data).hexdigest()==compressed[n]['decompressed_sha256'] and len(data)==compressed[n]['decompressed_bytes'];f=T/f'{backend}-{trial}.jsonl';f.write_bytes(data);health.check(f,110);paths.append(f)
  result=compare.compare(paths,110);assert result==read('raw/state-comparison-'+backend+'.json') and result['status']=='pass' and result['comparison_mode']=='raw'
print(f'PASS: {len(idx)} raw files,34 current processes,126 unchanged regressions,ten110-frame raw-state repeats; PR01 functional gate only')
