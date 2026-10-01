from pathlib import Path
import hashlib,json,shutil,subprocess
root=Path.cwd();local=root/'artifacts/production-readiness/pr02-supported';dest=root/'benchmarks/production-readiness/pr02-supported-2026-10-01';old=root/'benchmarks/production-readiness/pr02-diagnostics-2026-10-01'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
entries=[]
for f in sorted(local.rglob('*')):
 if not f.is_file() or f.name=='fixture':continue
 target=dest/'raw'/f.relative_to(local);target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(f,target)
 entries.append({'path':str(target.relative_to(dest)),'sha256':sha(target),'original_path':str(f),'original_sha256':sha(f)})
(dest/'raw-index.json').write_text(json.dumps({'entries':entries},indent=2)+'\n')
p=json.loads((dest/'protocol.json').read_text());r=json.loads((local/'receipt.json').read_text());assert r['status']=='pass';assert r['protocol_sha256']==sha(dest/'protocol.json');assert (local/'protocol-before-runs.json').read_bytes()==(dest/'protocol.json').read_bytes()
audit={'revision_at_audit':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'invocation_revision_is_not_binary_proof':True,'engine_source_matches':{},'fixture_source_matches':{},'portable_prior_receipts':{},'all_linked_archives_match_prior_build_receipts':True,'packaging_failure_retained':'Initial offline audit wrongly required the entire old linked_inputs map, including the different native_diagnostics_fixture.cpp source. All selected archive hashes already matched. Corrected to require every linked archive and separately verify this campaign fixture sources; no GPU retry or evidence change.'}
for backend in ['ordinary','native']:
 proof=p['source_proof'][backend];receipt_path=dest/proof['build_receipt'];assert sha(receipt_path)==proof['receipt_sha256'];b=json.loads(receipt_path.read_text());assert b['engine_sources']==b['engine_sources_after'];matches={name:sha(root/name)==expected for name,expected in b['engine_sources'].items()};assert all(matches.values());audit['engine_source_matches'][backend]=matches
 assert b['binary_sha256']==proof['library_sha256'] and b['test_binary_sha256']==proof['test_sha256']
for name,expected in r['fixture_inputs_before'].items():
 assert expected==r['fixture_inputs_after'][name];audit['fixture_source_matches'][name]=sha(root/name)==expected
assert all(audit['fixture_source_matches'].values())
for cfg in ['ordinary-gpu','native-gpu','ordinary-both','native-both']:
 prev=old/f'raw/c/{cfg}/receipt.json';q=json.loads(prev.read_text());audit['portable_prior_receipts'][cfg]={'path':str(prev.relative_to(dest.parent)),'sha256':sha(prev)}
 for b in [b for b in r['builds'] if b['configuration']==cfg]:
  assert b['adapter_receipt_sha256']==sha(prev)
  assert all(q['linked_inputs'].get(name)==value for name,value in b['linked_inputs'].items())
(dest/'applicability.json').write_text(json.dumps(audit,indent=2)+'\n')
context={'scope':'post-run read-only host context, not proof of compilation or driver immutability during trials','commands':{}}
for cmd in [['g++','--version'],['uname','-a'],['nvidia-smi','--query-gpu=name,driver_version,pci.bus_id','--format=csv,noheader']]:
 q=subprocess.run(cmd,capture_output=True,text=True);context['commands'][' '.join(cmd)]={'exit':q.returncode,'stdout':q.stdout,'stderr':q.stderr}
context['os_release']=Path('/etc/os-release').read_text();(dest/'host-context.json').write_text(json.dumps(context,indent=2)+'\n')
print('portable',len(entries),'files',sum((dest/x['path']).stat().st_size for x in entries),'bytes; current compiled sources and linked archives match')
