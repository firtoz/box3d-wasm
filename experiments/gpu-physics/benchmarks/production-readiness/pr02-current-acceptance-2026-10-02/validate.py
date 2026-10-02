#!/usr/bin/env python3
"""Offline PR02 evidence validation; launches no engine/compiler/device work."""
from pathlib import Path
import json,hashlib,tarfile,re
B=Path(__file__).resolve().parent;sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();read=lambda n:json.loads((B/n).read_text());idx=read('raw-index.json')
for n,h in idx.items():assert sha(B/n)==h,n
p=read('protocol.json');s=read('raw/receipt.json');a=read('acceptance.json');rest=read('raw/remaining-host/receipt.json');q=read('raw/remaining-host/protocol-before-work.json')
assert s['status']=='stopped' and s['protocol_sha256']==sha(B/'protocol.json');assert len(s['builds'])==32 and len(s['C_runs'])==32 and len(s['Rust_runs'])==19;assert all(x['exit']==0 for x in s['builds']+s['C_runs']+s['Rust_runs']);assert rest['status']=='passed' and rest['exit']==0
assert not s['unlaunched_C'] and s['unlaunched_Rust']==[{'backend':'native','selector':p['Rust_selectors'][-1]}];assert q['original_stopped_receipt_sha256']==sha(B/'raw/receipt.json') and q['original_protocol_sha256']==sha(B/'protocol.json');assert rest['protocol_sha256']==sha(B/'raw/remaining-host/protocol-before-work.json')
assert p['budget']=={'C_links':32,'C_fresh_processes':32,'Rust_fresh_processes':20,'Rust_GPU_processes':18,'Rust_host_processes':2,'new_test_list_processes':0,'engine_or_viewer_builds':0,'timing':0,'candidates':0,'retries':0}
markers={'api_settings_test':'C API settings: pass','warm_start_fixture':'"timing_only":false','native_diagnostics_fixture':'native diagnostics C contract passed:','native_diagnostic_populations':'native diagnostic populations: sensor/compound/mesh contract passed','both_substep_forces_test':'substep forces:','both_shape_replacement_test':'shape replacement:','both_joint_reaction_test':'joint reaction:','both_joint_separation_test':'joint separation:','both_compound_ownership_test':'combined compound ownership: four child kinds, bounds, public constructors and three lifetime cycles passed','compound_aabb_contract':'compound AABB contract: public/native/body/CPU geometry, transforms, disabled access and stale lifetimes passed'}
for row,case in zip(s['C_runs'],p['cases']):
 assert (row['cell'],row['fixture'])==(case['cell'],case['fixture']);folder=B/'raw/runs/C'/row['cell']/row['fixture'];stdout=(folder/'stdout.log').read_text();stderr=(folder/'stderr.log').read_text();assert sha(folder/'stdout.log')==row['stdout_sha256'] and sha(folder/'stderr.log')==row['stderr_sha256'];assert 'NVIDIA GeForce RTX 4070 SUPER | backend=Vulkan' in stdout+stderr
 if row['fixture']=='compound_property_diagnostic':
  records=[json.loads(l) for l in stdout.splitlines() if l.startswith('{')];assert records[-1]=={'summary':True,'observations':1016,'mismatches':0};assert len(records)==1017 and all(x['match'] for x in records[:-1])
 else:assert markers[row['fixture']] in stdout
 build=next(x for x in s['builds'] if x['cell']==row['cell'] and x['fixture']==row['fixture']);assert build['binary_sha256']==row['binary_sha256'];assert build['source_sha256']==p['inputs']['c_abi/'+row['fixture']+'.cpp']
for i,selector in enumerate(p['Rust_selectors'],1):
 for backend in ['ordinary','native']:
  if i==10 and backend=='native':row=rest;folder=B/'raw/remaining-host'
  else:row=next(x for x in s['Rust_runs'] if x['backend']==backend and x['selector']==selector);folder=B/'raw/runs/Rust'/backend/str(i)
  stdout=(folder/'stdout.log').read_text();stderr=(folder/'stderr.log').read_text();assert '1 passed; 0 failed' in stdout;assert sha(folder/'stdout.log')==row['stdout_sha256'] and sha(folder/'stderr.log')==row['stderr_sha256'];assert row['binary_sha256']==p['tests'][backend]['sha256']
  if i not in [1,10]:assert 'NVIDIA GeForce RTX 4070 SUPER | backend=Vulkan' in stdout+stderr
review=read('raw/source-contract-review.json');assert len(review['excluded_methods'])==156 and all(x['exact_definition_match'] for x in review['excluded_methods'])
for backend in ['ordinary','native']:
 audit=read('raw/applicable-inventory/'+backend+'.json');assert audit['status']=='complete' and audit['required_stateful_header_symbols']==415;inventory=read('api-inventory-'+backend+'.json');assert len(inventory)==830
 for cell in [backend+'-gpu',backend+'-both']:
  rows=[x for x in inventory if x['cell']==cell];assert len(rows)==415 and len({x['symbol'] for x in rows})==415;assert sum(x['classification']=='declared-unavailable' for x in rows)==39;assert all(x['linked_in_gpu_inputs'] for x in rows)
 for key in ['additional_missing_stateful_symbols','remaining_stub_definitions','additional_known_placeholders','duplicate_stub_definitions','cpu_only_comparison_passthrough','unavailable_source_error_hooks_missing']:assert audit[key]==[]
for name,key in [('engine-build','engine_build_receipt_sha256'),('viewer-parent',None)]:
 d=read('raw/origin-receipts/'+name+'.json');assert d['status']=='built' and d['inputs_before']==d['inputs_after'];assert sha(B/'raw/origin-receipts'/(name+'.json'))==(p[key] if key else s['viewer_parent_receipt_sha256'])
for cell,d in p['cells'].items():assert sha(B/'raw/origin-receipts'/(cell+'.json'))==d['receipt_sha256']
with tarfile.open(B/'raw/reviewed-source-inputs.tar.gz') as t:
 for n,h in review['sources'].items():assert hashlib.sha256(t.extractfile('sources/'+h+'/'+Path(n).name).read()).hexdigest()==h
post=read('raw/post-campaign-source-review.json')
for name in ['engine-build','viewer-parent']:
 origin=read('raw/origin-receipts/'+name+'.json');assert post[name]['actual_current_source_hashes']==origin['inputs_after'] and post[name]['exact_origin_inputs_match']
assert post['all_C_executables_still_exact']
# Recompute excluded definition identity from archived current C and historical compiled C.
def definition(text,name):
 m=re.search(r'^B3_API\s+[^;{]*?\b'+name+r'\([^;]*?\)\s*\{',text,re.M|re.S)
 if not m:return None
 i=m.end();level=1
 while level and i<len(text):level+=(text[i]=='{')-(text[i]=='}');i+=1
 return text[m.start():i]
with tarfile.open(B/'raw/reviewed-source-inputs.tar.gz') as t:
 texts=[t.extractfile('sources/'+h+'/'+Path(n).name).read().decode() for n,h in review['sources'].items() if Path(n).suffix=='.c']
 for row in review['excluded_methods']:
  old=Path(row['historical_source']);parts=old.parts;at=parts.index('pr02-api-2026-10-01');oldfile=B.parent.joinpath(*parts[at:]);assert sha(oldfile)==row['historical_source_sha256'];olddef=definition(oldfile.read_text(),row['symbol']);assert olddef and hashlib.sha256(olddef.encode()).hexdigest()==row['current_definition_sha256'];assert any(definition(text,row['symbol'])==olddef for text in texts)
assert all(x['status']=='pass' for x in a['requirements']) and not a['release_ready'];assert a['campaign_result']['Rust_GPU_processes']==16 and a['campaign_result']['Rust_host_processes']==4
print(f'PASS: {len(idx)} portable rawfiles;32C worlds/20Rust processes (16GPU+4host),original harness stop retained;415 symbols/backend,39 exclusions;PR02 named contracts only')
