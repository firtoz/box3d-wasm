from pathlib import Path
import json,hashlib
b=Path(__file__).resolve().parent;sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();index=json.loads((b/'raw-index.json').read_text())
for n,h in index.items():assert sha(b/n)==h['sha256']and(b/n).stat().st_size==h['bytes'],n
p=json.loads((b/'raw/protocol-before-runs.json').read_text());r=json.loads((b/'raw/ordinary-gpu/receipt.json').read_text());t=json.loads((b/'raw/terminal.json').read_text());assert r['protocol_sha256']==sha(b/'raw/protocol-before-runs.json');assert sha(b/'raw/viewer-interact.py')==p['driver_sha256'];assert sha(b/'raw/stage.py')==p['action_coordinator_sha256'];assert t['first_apps']==1 and t['accepted_apps']==0
build=b.parent/'pr02-world-lifetime-viewer-builds-2026-10-02/raw';v=json.loads((build/'ordinary-gpu/receipt.json').read_text());assert sha(build/'ordinary-gpu/receipt.json')==r['build_receipt_sha256'];assert r['binary_sha256']==v['executable_sha256'];assert p['frozen_viewers']['ordinary-gpu']==r['binary_sha256']
a={a['action'].get('label'):a for a in r['actions']};restart=a['restart-off'];assert restart['state_before']['world']==[1,1]and restart['state_after']['world']==[1,2];assert restart['state_after']['sample_step']==restart['state_after']['submitted']==restart['state_after']['completed']==0
single=a['single-after'];assert all(single['state_after'][f]-single['state_before'][f]==1 for f in ['sample_step','submitted','completed'])
for n,h in r['screenshots'].items():assert sha(b/'raw/ordinary-gpu'/n)==h
assert all(not(b/'raw'/cell).exists()for cell in t['gpu_unlaunched'])
assert t['status']=='stopped-action-publication-race'and r['status']=='failed'and len(r['actions'])==19 and len(r['screenshots'])==6 and 'JSONDecodeError'in r['failure']
assert a['continuous-on']['state_after']['context_flags']==a['continuous-on']['state_after']['world_flags']==[1,1,1]
assert json.loads((b/'raw/ordinary-gpu/action-20.json').read_text())=={'type':'screenshot','name':'metrics-initial'}
assert "path.write_text(json.dumps(action)"in(b/'raw/stage.py').read_text()
print(len(index),'raw files verified; retained harness failure, repaired root/single-step observations, zero complete viewer passes')
