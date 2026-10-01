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
assert t['status']=='stopped-stimulus-coordinate-failure'and r['status']=='complete'and r['exit']==r['capture_exit']==0 and len(r['actions'])==15 and len(r['screenshots'])==5
assert a['sleep-on']['action']['y']==587 and a['sleep-on']['state_after']['context_flags']==[0,0,0]
print(len(index),'raw files verified; retained harness failure, repaired root/single-step observations, zero complete viewer passes')
