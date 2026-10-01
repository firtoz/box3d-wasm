from pathlib import Path
import hashlib,json
B=Path(__file__).resolve().parent;sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();I=json.loads((B/'raw-index.json').read_text())
for n,h in I.items():assert sha(B/n)==h['sha256']and (B/n).stat().st_size==h['bytes'],n
R=json.loads((B/'raw/cpu/receipt.json').read_text());T=json.loads((B/'raw/terminal.json').read_text());P=json.loads((B/'raw/protocol-before-runs.json').read_text());assert R['status']=='failed'and 'FileNotFoundError'in R['failure'];assert R['actions']==[]and R['screenshots']=={};assert R['protocol_sha256']==sha(B/'raw/protocol-before-runs.json');assert T['apps_consumed']==1 and T['accepted_apps']==T['control_actions_sent']==T['timing_runs']==0;assert all(not r['process_exists']for r in T['process_state'].values());assert all(not(B/'raw'/c).exists()for c in P['order'][1:]);assert P['source_audit_amendment_before_any_app']['initial_protocol_sha256']==sha(B/'raw/protocol-initial-before-source-audit.json')
for n,h in T['files'].items():assert sha(B/'raw/cpu'/n)==h['sha256']
assert (B/'raw/cpu/settings.ini').read_text()=='{}\n';states=[json.loads(line)for line in(B/'raw/cpu/observer.jsonl').read_text().splitlines()];assert states and all(s['context_flags']==s['world_flags']==[1,1,1]and s['pause']==0 and s['hertz']==60 and s['substeps']==4 for s in states)
print(len(I),'raw files verified;',len(states),'initial CPU records; focus infrastructure failure retained, no UI acceptance')
