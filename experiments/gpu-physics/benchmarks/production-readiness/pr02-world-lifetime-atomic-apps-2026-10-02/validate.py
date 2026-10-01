from pathlib import Path
import json,hashlib,subprocess,sys
b=Path(__file__).resolve().parent;sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();index=json.loads((b/'raw-index.json').read_text())
for n,h in index.items():assert sha(b/n)==h['sha256']and (b/n).stat().st_size==h['bytes'],n
p=json.loads((b/'raw/protocol-before-runs.json').read_text());t=json.loads((b/'raw/terminal.json').read_text());host=json.loads((b/'raw/host-receipt.json').read_text());assert t['status']=='passed-limited-PR02-controls'and t['first_apps']==t['accepted_apps']==4 and t['total_actions']==116 and t['total_screenshots']==44 and t['observer_records']==6409
assert p['budget']['timing_runs']==t['timing_runs']==p['budget']['fresh_cpu_apps']==t['fresh_cpu_apps']==0
assert host['status']=='pass'and host['atomic_writes']==501 and host['concurrent_reads']>0 and host['errors']==[] and host['protocol_sha256']==sha(b/'raw/protocol-before-runs.json')
for n,k in [('viewer-interact.py','driver_sha256'),('stage.py','action_coordinator_sha256'),('validate-app.py','validator_sha256'),('atomic_io.py','atomic_io_sha256')]:assert sha(b/'raw'/n)==p[k]
old=b.parent/'pr02-world-lifetime-adaptive-apps-2026-10-02/raw'
a=(old/'viewer-interact.py').read_text().replace('pr02-world-lifetime-adaptive-apps','pr02-world-lifetime-atomic-apps').replace('from pathlib import Path','from atomic_io import atomic_json\nfrom pathlib import Path',1).replace("def save():(dest/'receipt.json').write_text(json.dumps(r,indent=2)+'\\n')","def save():atomic_json(dest/'receipt.json',r)");assert a==(b/'raw/viewer-interact.py').read_text()
a=(old/'stage.py').read_text().replace('pr02-world-lifetime-adaptive-apps','pr02-world-lifetime-atomic-apps').replace('from pathlib import Path','from atomic_io import atomic_json\nfrom pathlib import Path',1).replace("path.write_text(json.dumps(action)+'\\n')","atomic_json(path,action)");assert a==(b/'raw/stage.py').read_text();assert (old/'validate-app.py').read_bytes()==(b/'raw/validate-app.py').read_bytes()
build=b.parent/'pr02-world-lifetime-viewer-builds-2026-10-02/raw';assert sha(build/'receipt.json')==p['build_receipt_sha256']
for row in t['results']:
 c=row['configuration'];d=b/'raw'/c;r=json.loads((d/'receipt.json').read_text());v=json.loads((build/c/'receipt.json').read_text());assert sha(build/c/'receipt.json')==r['build_receipt_sha256']and r['binary_sha256']==v['executable_sha256']==p['frozen_viewers'][c];assert r['protocol_sha256']==sha(b/'raw/protocol-before-runs.json');assert sha(d/'clip.mp4')==r['video_sha256'];assert sha(d/'acceptance.json')==row['acceptance_sha256'];assert 'GPU adapter: NVIDIA GeForce RTX 4070 SUPER | backend=Vulkan driver=NVIDIA'in(d/'stderr.log').read_text()
 assert len(r['actions'])==29 and len(r['screenshots'])==11 and r['focus_observed']==r['focused_window'][0]
 states=[json.loads(x)for x in(d/'observer.jsonl').read_text().splitlines()];assert len(states)==row['observer_records'];assert all(x['completed_known']==1 and x['submitted']==x['completed']for x in states)
 actions={a['action'].get('label'):a for a in r['actions']};restart=actions['restart-off'];assert restart['state_before']['world']==[1,1]and restart['state_after']['world']==[1,2]
 if c.endswith('both'):assert restart['state_before']['cpu_world']==[1,0]and restart['state_after']['cpu_world']==[1,1]
 for stage in ['flags-off','restart','flags-on','metrics']:
  plan=json.loads((d/(stage+'-reviewed-plan.json')).read_text());assert sha(d/plan['screen'])==plan['screen_sha256'];assert all(a in [a['action']for a in r['actions']]for a in plan['actions'])
 review=json.loads((d/'visual-review.json').read_text())
 for name,h in review['derived_sheets'].items():assert sha(d/name)==h
 # Same original evaluator, existing assertions and event-window semantics.
 subprocess.run([sys.executable,str(b/'raw/validate-app.py'),c,str(d)],check=True)
 assert sha(d/'acceptance.json')==row['acceptance_sha256']
print(len(index),'raw files verified; four limited control passes, original evaluator preserved, all old failures retained')
