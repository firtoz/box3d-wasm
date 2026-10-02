from pathlib import Path
import json,hashlib,subprocess,tarfile
R=Path.cwd();A=R/'artifacts/production-readiness/pr03-baseline-builds';sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();p=json.loads((R/'benchmarks/production-readiness/pr02-current-acceptance-2026-10-02/protocol.json').read_text());assert not (A/'protocol-before-builds.json').exists()
prototype=R/'artifacts/warm-start/distance-reference-mismatch/warm_start_fixture.cpp';fixture=A/'distance-original.cpp';fixture.write_bytes(prototype.read_bytes());assert sha(fixture)==sha(prototype)
cases=[{'name':'ragdoll','cell':c,'source':'c_abi/ragdoll_reference.cpp','shared_objects':True} for c in ['cpu','ordinary-gpu','native-gpu']]+[{'name':'frozen-mesh','cell':c,'source':'c_abi/rain_frozen_contact.cpp','shared_objects':True} for c in ['cpu','ordinary-gpu','native-gpu']]+[{'name':'drag','cell':c,'source':'c_abi/both_drag_test.cpp','shared_objects':False} for c in ['ordinary-both','native-both']]+[{'name':'distance-original','cell':c,'source':str(fixture.relative_to(R)),'shared_objects':False} for c in ['ordinary-both','native-both']]
paths=[R/c['source'] for c in cases]+[R.parents[1]/('box3d/shared/'+n+'.c') for n in ['human','determinism','utils']]+[R.parents[1]/'box3d/samples/host/camera.cpp']
for folder in [R.parents[1]/'box3d/include',R.parents[1]/'box3d/shared',R.parents[1]/'box3d/samples',R/'c_abi',R/'native-samples']:
 paths +=[f for f in folder.rglob('*.h') if '.fetchcontent' not in str(f) and 'build-' not in str(f)]
inputs={str(f.resolve()):sha(f) for f in sorted(set(paths))}
engine=Path(p['engine_build_receipt']);viewer=Path(p['cells']['ordinary-gpu']['receipt']).parent.parent/'receipt.json'
for receipt in [engine,viewer]:
 b=json.loads(receipt.read_text());assert b['inputs_before']==b['inputs_after']
 for n,h in b['inputs_after'].items():assert sha(R/n)==h
linked={}
for cell,info in p['cells'].items():
 assert sha(info['library'])==info['library_sha256'];build=Path(info['build']);record=json.loads(Path(info['receipt']).read_text());assert sha(info['receipt'])==info['receipt_sha256']
 names=['libgpu_samples_api.a','box3d_src/libbox3d.a']+(['libgpu_both_api.a','libbox3d_cpu.a'] if cell.endswith('both') else [])
 for n in names:f=build/n;assert sha(f)==record['linked_archives'][str(f)];linked[str(f)]=sha(f)
 linked[info['library']]=sha(info['library'])
protocol={'purpose':'Build exact current independent CPU/GPU baseline fixtures for PR03 original physical matrix; no engine execution/candidate change','budget':{'shared_C_compiler_commands':3,'fixture_link_commands':10,'compiled_translation_units':15,'Rust_builds':0,'viewer_builds':0,'engine_processes':0,'timing':0,'candidates':0,'retries':0},'cases':cases,'inputs_before':inputs,'linked_inputs':linked,'cells':p['cells'],'engine_build_receipt':str(engine),'engine_receipt_sha256':sha(engine),'viewer_parent_receipt':str(viewer),'viewer_parent_sha256':sha(viewer),'original_distance_source_sha256':sha(fixture),'original_distance_contract':'Original sphere-ground/distance fixtures,240steps each1/4substeps,1e-5 CPU pose/velocity check,first-step discrepancy retained; no --timing/no physics default or assertion changes','stop_rule':'Stop first input/compiler/link failure,retain all results/unlaunched; no rebuild retry in this protocol','watchdog_seconds':600,'source_milestone':'abc0a54a3ce3b285c9984f2c7c6b5baefff00d1b','invocation_revision_context_only':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'toolchain':{'g++':subprocess.check_output(['g++','--version'],text=True),'cc':subprocess.check_output(['cc','--version'],text=True)},'box3d_revision':subprocess.check_output(['git','-C',str(R.parents[1]/'box3d'),'rev-parse','HEAD'],text=True).strip()}
(A/'protocol-before-builds.json').write_text(json.dumps(protocol,indent=2)+'\n')
with tarfile.open(A/'fixture-source-inputs.tar.gz','w:gz') as t:
 for n in inputs:t.add(n,arcname=str(Path(n).relative_to(R.parents[1])))
print('Frozen build-only budget13compiler/link commands,15translation units,10fixtures;0engine/timing runs. Protocol',sha(A/'protocol-before-builds.json'))
