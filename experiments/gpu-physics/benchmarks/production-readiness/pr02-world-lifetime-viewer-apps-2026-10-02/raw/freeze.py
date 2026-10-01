from pathlib import Path
import hashlib,json,subprocess,time
root=Path.cwd();a=root/'artifacts/production-readiness';out=a/'pr02-world-lifetime-viewer-apps';build=a/'pr02-world-lifetime-viewer-builds';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
r=json.loads((build/'receipt.json').read_text());assert r['status']=='built'and r['inputs_before']==r['inputs_after']
p=json.loads((a/'pr02-controls-event-window-apps/protocol-before-runs.json').read_text())
p.update(id='pr02-world-lifetime-viewer-apps-2026-10-02',prerequisite='Four first diagnostic viewers from exact passing world/child repair libraries plus freshly compiled C guards; build proof completed before app processes',budget={'fresh_cpu_apps':0,'first_repaired_gpu_apps':4,'host_receiver_processes':0,'production_candidates':0,'timing_runs':0,'retries':0},changed_hypothesis='Per-world epochs and child routing now independently pass72 Rust and162 GPU C observations. Old viewer root failure remains retained. First actual viewer processes for this exact repair; original held-input and event-window requirements unchanged.',frozen_unix=time.time(),driver_sha256=sha(out/'viewer-interact.py'),validator_sha256=sha(out/'validate-app.py'),build_receipt_sha256=sha(build/'receipt.json'))
for k in ['validator_before_sha256','validator_after_sha256','source_evidence']:p.pop(k,None)
p['action_coordinator_sha256']=sha(out/'stage.py')
p['frozen_viewers']={cell:r['viewers'][cell]['executable_sha256']for cell in p['order']}
for cell in p['order']:
 v=json.loads((build/cell/'receipt.json').read_text());assert sha(Path(v['executable']))==v['executable_sha256']
assert not (out/'protocol-before-runs.json').exists();(out/'protocol-before-runs.json').write_text(json.dumps(p,indent=2)+'\n')
(out/'adapter.txt').write_text(subprocess.check_output(['nvidia-smi','--query-gpu=name,uuid,driver_version,pci.bus_id','--format=csv'],text=True))
print('Frozen first four actual apps protocol',sha(out/'protocol-before-runs.json'))
