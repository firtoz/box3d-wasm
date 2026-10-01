from pathlib import Path
import json,hashlib,sys
base=Path(__file__).resolve().parent;cell=sys.argv[1];dest=Path(sys.argv[2])if len(sys.argv)>2 else base/cell;r=json.loads((dest/'receipt.json').read_text());sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();assert r['status']=='complete'and r['exit']==r['capture_exit']==0 and not r.get('watchdog');assert sha(dest/'observer.jsonl')==r['observer_sha256'];assert sha(dest/'settings.ini')==r['settings_after_sha256']
for name,h in r['screenshots'].items():assert sha(dest/name)==h
states=[json.loads(line)for line in(dest/'observer.jsonl').read_text().splitlines()];assert states and all(s['hertz']==60 and s['substeps']==4 and s['body_count']==s['shape_count']==2 for s in states)
assert all(s['context_flags']==s['world_flags']for s in states),'context/world mismatch'
if cell.endswith('both'):assert all(s['cpu_flags']==s['context_flags']for s in states),'mapped CPU flag mismatch'
actions={a['action']['label']:a for a in r['actions']if 'label'in a['action']};required=['pause','single-before','sleep-off','warm-off','continuous-off','restart-off','single-after','sleep-on','warm-on','continuous-on','metrics-on','profile','counters','frame-time','metrics-off','quit'];assert all(n in actions for n in required)
pause=actions['pause']['state_after'];assert pause['pause']==1
single=actions['single-before'];assert single['state_before']['pause']==single['state_after']['pause']==1
for field in ['sample_step','submitted','completed']:assert single['state_after'][field]-single['state_before'][field]==1,(field,single)
assert all(s['sample_step']==pause['sample_step']for s in states if pause['frame']<=s['frame']<=single['state_before']['frame'])
for name,index,value in [('sleep-off',0,0),('warm-off',1,0),('continuous-off',2,0),('sleep-on',0,1),('warm-on',1,1),('continuous-on',2,1)]:assert actions[name]['state_after']['context_flags'][index]==value,name
restart=actions['restart-off'];assert restart['state_after']['world']!=restart['state_before']['world'];assert restart['state_after']['sample_step']==restart['state_after']['submitted']==0;assert restart['state_after']['pause']==1 and restart['state_after']['context_flags']==[0,0,0]
single=actions['single-after']
for field in ['sample_step','submitted','completed']:assert single['state_after'][field]-single['state_before'][field]==1,(field,single)
event_window=[s for s in states if single['state_before']['frame']<s['frame']<=single['state_after']['frame'] and s['world']==single['state_after']['world'] and s['sample_step']==single['state_after']['sample_step']]
assert any(s['move_count']>0 and 'first_position'in s for s in event_window),'actual moved body required on completed single-step window'
assert actions['metrics-on']['state_after']['show_metrics']==1 and actions['metrics-off']['state_after']['show_metrics']==0
settings=json.loads((dest/'settings.ini').read_text());assert settings['sampleIndex']>=0;assert all(k in settings for k in ['drawShapes','enableShadows','enableGtao','drawDistance'])
# Visible tab contents/labels must be independently reviewed; recorded clicks or
# filenames alone cannot count. The review names the actual tab/content observed.
review=json.loads((dest/'visual-review.json').read_text());assert review['status']=='pass';assert all(n in review['observed']for n in ['paused','flags-off','restart-off','flags-on','profile','counters','frame-time','policy-exclusions'])
for name,h in review['screenshots'].items():assert r['screenshots'][name]==h
result={'status':'pass','configuration':cell,'observer_records':len(states),'receipt_sha256':sha(dest/'receipt.json'),'observer_sha256':r['observer_sha256'],'visual_review_sha256':sha(dest/'visual-review.json'),'verified':['default settings preserved','actual checkbox flags off/on','combined CPU mapping when applicable','paused stable completed step','exact single-step twice across lifetime','restart with flags preserved','actual moved-body event','reviewed actual metrics tab content and policy/exclusions','metrics toggle','clean quit and upstream-supported persisted rendering settings'],'limits':'Limited PR02 viewer controls, diagnostic observer reads may synchronize/cost time. Not full lifecycle/physical/repeatability/sample-wide UI/raycast/render FPS or performance acceptance.'};(dest/'acceptance.json').write_text(json.dumps(result,indent=2)+'\n');print(cell,'controls pass',len(states),'records')
