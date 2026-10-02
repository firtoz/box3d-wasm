from pathlib import Path
import hashlib,json,copy,subprocess,tarfile,os
R=Path.cwd(); O=R/'artifacts/production-readiness/pr02-world-lifetime-scene-captures'
sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
write=lambda p,d:Path(p).write_text(json.dumps(d,indent=2)+'\n')
p=json.loads((O/'protocol-before-captures.json').read_text()); N=O/'native'; N.mkdir(exist_ok=False)
receipts={}
for kind,folder,cell in [('cpu','pr02-controls-current-builds','cpu'),('gpu','pr02-world-lifetime-viewer-builds','native-both')]:
 base=R/'artifacts/production-readiness'/folder; origin=base/cell/'receipt.json'; parent=base/'receipt.json'; d=json.loads(origin.read_text()); b=json.loads(parent.read_text()); assert d['status']==b['status']=='built';assert b['inputs_before']==b['inputs_after']
 mismatches={n:h for n,h in b['inputs_before'].items() if sha(R/n)!=h}
 if kind=='cpu':
  assert set(mismatches)=={'c_abi/both_dual.c','c_abi/shim.c','src/api/world/joint_reaction.rs','src/api/world/joint_separation.rs','src/api/world/shape_geometry.rs','src/api/world.rs'}
  assert not any('c_abi/shim.c' in u['file'] or 'both_dual' in u['file'] or u['file'].endswith('.rs') for u in d['compiled_units']);assert 'gpu_physics' not in d['link_command']
 else: assert not mismatches
 for u in d['compiled_units']:assert sha(u['file'])==u['source_sha256'] and sha(u['object'])==u['object_sha256']
 for n,h in d['linked_archives'].items():assert sha(n)==h
 assert sha(d['executable'])==d['executable_sha256']
 d.update(inputs_before={n:h for n,h in b['inputs_before'].items() if n not in mismatches},inputs_after={n:h for n,h in b['inputs_after'].items() if n not in mismatches},origin_viewer_receipt=str(origin),origin_viewer_receipt_sha256=sha(origin),origin_parent_receipt=str(parent),origin_parent_receipt_sha256=sha(parent),not_compiled_cpu_inputs_excluded=mismatches,applicability_note='Composite applicability audit, not a new build. CPU link contains no Rust/GPU shim; all119 actual CPU units and linked archives unchanged. Native GPU178 actual units and frozen Rust library unchanged. Observer disabled unless explicit env, absent during capture. Cached dependencies disclosed; not PR06.')
 rp=N/(kind+'-applicability-receipt.json');write(rp,d)
 receipts[kind]={'binary':os.path.relpath(d['executable'],R),'binary_sha256':d['executable_sha256'],'receipt':os.path.relpath(rp,R),'receipt_sha256':sha(rp),'label':p[kind+'_label']}
 if kind=='gpu':receipts[kind]['health_mode']='both'
old=json.loads((R/'artifacts/production-readiness/pr02-compound-properties-captures/protocol.json').read_text())
np=copy.deepcopy(old);np.update(order='CPU6 before GPU6',artifact_directory=os.path.relpath(N,R),viewers=receipts,scenes=p['compound_scenes'],build_applicability_exclusions=[],source_files={n:sha(R/n) for n in old['source_files']},assets={n:sha(R/n) for n in old['assets']},watchdog_seconds_per_scene=1200,git_checkout_context_only=p['invocation_revision_context_only'],applicability_note=d['applicability_note'])
np['environments']['gpu']['GPU_PHYSICS_PIPELINE_CACHE_DIR']=str(O/'native-pipelines');write(N/'protocol.json',np)
with tarfile.open(N/'scene-assets.tar.gz','w:gz') as t:
 for n in np['assets']:t.add(R/n,arcname=n)
write(O/'launch-contract.json',{'capture_protocol_sha256':sha(O/'protocol-before-captures.json'),'native_protocol_sha256':sha(N/'protocol.json'),'prepare_sha256':sha(__file__),'no_apps_launched':True,'frozen_binaries':p['recorder'],'cpu_and_native_applicability_receipts':receipts})
print('Prepared native6 protocol',sha(N/'protocol.json'),'CPU units119/native units178; no app run')
