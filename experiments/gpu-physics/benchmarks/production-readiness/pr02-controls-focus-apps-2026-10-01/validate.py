from pathlib import Path
import hashlib,json
B=Path(__file__).resolve().parent;sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();I=json.loads((B/'raw-index.json').read_text())
for n,h in I.items():assert sha(B/n)==h['sha256']and (B/n).stat().st_size==h['bytes'],n
T=json.loads((B/'raw/terminal.json').read_text());R=json.loads((B/'raw/cpu/receipt.json').read_text());P=json.loads((B/'raw/protocol-before-runs.json').read_text());assert T['status']=='stopped-verifier-sampling-failure'and T['fresh_apps']==1 and T['gpu_apps']==0;assert R['status']=='complete'and R['exit']==R['capture_exit']==0;assert len(R['actions'])==25 and len(R['screenshots'])==9;assert R['protocol_sha256']==sha(B/'raw/protocol-before-runs.json');assert T['old_validator_sha256']==sha(B/'raw/validate-app.py');assert sha(B/'raw/viewer-interact.py')==P['driver_sha256'];assert R['focus_observed']==R['focused_window'][0];assert all(not(B/'raw'/c).exists()for c in P['order'][1:]);assert sha(B/'raw/cpu/observer.jsonl')==R['observer_sha256'];assert sha(B/'raw/cpu/clip.mp4')==R['video_sha256']
for n,h in R['screenshots'].items():assert sha(B/'raw/cpu'/n)==h
s=next(a for a in R['actions']if a['action'].get('label')=='single-after');assert s['state_after']['move_count']==0
states=[json.loads(line)for line in(B/'raw/cpu/observer.jsonl').read_text().splitlines()];assert len(states)==1275;window=[v for v in states if s['state_before']['frame']<v['frame']<=s['state_after']['frame']];assert any(v['move_count']>0 and 'first_position'in v for v in window)
print(len(I),'raw files verified; complete CPU process, original late-sample assertion failed; GPU apps0')
