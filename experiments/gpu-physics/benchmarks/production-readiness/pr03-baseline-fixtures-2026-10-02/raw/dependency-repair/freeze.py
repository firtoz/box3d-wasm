from pathlib import Path
import json,hashlib
R=Path.cwd();old=R/'artifacts/production-readiness/pr03-baseline-builds';A=R/'artifacts/production-readiness/pr03-baseline-cpu-link-repair';sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest()
assert not (A/'protocol-before-builds.json').exists()
p=json.loads((old/'protocol-before-builds.json').read_text());receipt=json.loads((old/'receipt.json').read_text());assert receipt['status']=='stopped' and len(receipt['commands'])==4 and not receipt['fixtures'];assert all(r['exit']==0 for r in receipt['commands'][:3]) and receipt['commands'][3]['exit']==1
cpu=R/'artifacts/production-readiness/pr02-world-lifetime-scene-captures/native/cpu-applicability-receipt.json';c=json.loads(cpu.read_text());assert c['status']=='built' and c['inputs_before']==c['inputs_after']
for n,h in c['inputs_after'].items():assert sha(R/n)==h,n
for u in c['compiled_units']:
 assert sha(u['file'])==u['source_sha256'];assert sha(u['object'])==u['object_sha256']
for n,h in c['linked_archives'].items():assert sha(n)==h,n
build=Path(c['executable']).parent.parent;archive=build/'box3d_src/libbox3d.a';assert sha(archive)==c['linked_archives'][str(archive)]
p['cells']['cpu']={'build':str(build),'receipt':str(cpu),'receipt_sha256':sha(cpu),'library':None,'library_sha256':None};p['linked_inputs'][str(archive)]=sha(archive)
p['reused_shared_objects']={str(old/'shared'/n/'object.o'):sha(old/'shared'/n/'object.o') for n in ['human','determinism','utils']}
p['previous_protocol']={'path':str(old/'protocol-before-builds.json'),'sha256':sha(old/'protocol-before-builds.json')};p['previous_receipt']={'path':str(old/'receipt.json'),'sha256':sha(old/'receipt.json')}
p['budget']={'shared_C_compiler_commands':0,'fixture_link_commands':10,'compiled_translation_units':12,'Rust_builds':0,'viewer_builds':0,'engine_processes':0,'timing':0,'candidates':0,'retries':0}
p['purpose']='Correct CPU fixture link dependency using full independent CPU archive; reuse three already successful C objects; nine other fixture links were previously unlaunched. Original failed campaign is retained unchanged. No physics execution or production source change.'
p['dependency_repair']='Original first CPU link mistakenly selected geometry-only GPU Box3D archive. Full CPU archive has119 actual compiled source/object pairs checked against receipt; no current Rust applicability is inferred from older CPU parent receipt.'
p['stop_rule']='Stop first input/compiler/link failure; retain each result and all unlaunched links; zero engine runs/retries. This is a build-harness dependency correction, not an extension of physics/timing budgets.'
(A/'protocol-before-builds.json').write_text(json.dumps(p,indent=2)+'\n');print('Frozen10links/12translationunits, reuse3Cobjects,0engine/timing. SHA',sha(A/'protocol-before-builds.json'))
