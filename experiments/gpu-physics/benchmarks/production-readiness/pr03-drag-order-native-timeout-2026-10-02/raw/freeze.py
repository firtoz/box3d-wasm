from pathlib import Path
import json,hashlib,copy
R=Path.cwd();old=R/'artifacts/production-readiness/pr03-rain-drag-display-repair';A=R/'artifacts/production-readiness/pr03-rain-drag-remaining';sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest()
p=json.loads((old/'protocol.json').read_text());s=json.loads((old/'receipt.json').read_text());assert s['status']=='stopped' and len(s['results'])==2 and s['results'][0]['child_exit']==0 and s['results'][1]['wrapper_exit']=='timeout'
assert not(A/'protocol.json').exists()
p['purpose']='Consume only seven previously unlaunched baseline cells after retained ordinary Rain900second watchdog failure. No CPU or ordinary Rain repeat, timeout extension or candidate. Original sources/binaries/settings/criteria unchanged; independent drag and order controls precede the unlaunched native Rain cell. Preserve all original outcomes.'
p['previous_attempt']={'protocol_path':str(old/'protocol.json'),'protocol_sha256':sha(old/'protocol.json'),'receipt_path':str(old/'receipt.json'),'receipt_sha256':sha(old/'receipt.json'),'completed':['cpu-rain'],'incomplete':['ordinary-rain'],'original_unlaunched':s['unlaunched']}
cases={c['name']:c for c in p['cases']};order=[n for n in s['unlaunched'] if n!='native-rain']+['native-rain'];p['cases']=[copy.deepcopy(cases[n]) for n in order];assert len(order)==len(set(order))==7 and set(order)==set(s['unlaunched'])
for c in p['cases']:
 c['command']=[v.replace(str(old),str(A)) for v in c['command']];c['cwd']=c['cwd'].replace(str(old),str(A));c['environment']={k:v.replace(str(old),str(A)) for k,v in c['environment'].items()}
p['budget']={'processes':7,'rain_viewers':1,'drag_dual_processes':4,'CPU_compatible_order_controls':2,'builds':0,'candidates':0,'retries':0,'headline_timing':0,'isolated_Xvfb_servers':1};assert p['watchdog_seconds_per_process']==900
(A/'protocol.json').write_text(json.dumps(p,indent=2)+'\n');driver=(old/'run.py').read_text().replace("A=R/'artifacts/production-readiness/pr03-rain-drag-display-repair'","A=R/'artifacts/production-readiness/pr03-rain-drag-remaining'");(A/'run.py').write_text(driver);print('Frozen7unlaunched cells;SHA',sha(A/'protocol.json'))
