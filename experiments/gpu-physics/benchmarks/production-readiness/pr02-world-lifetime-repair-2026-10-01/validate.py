from pathlib import Path
import json,hashlib,tarfile
B=Path(__file__).resolve().parent;H=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();J=lambda p:json.loads(p.read_text());index=J(B/'raw-index.json')
for name,record in index.items():
 p=B/name;assert p.stat().st_size==record['bytes']and H(p)==record['sha256'],name
raw=B/'raw';get=lambda campaign,file:J(raw/campaign/file)
first=get('pr02-world-lifetime-fix','build-receipt.json');assert first['status']=='stopped'and first['production_restored'];assert [(x['name'],x['exit'])for x in first['builds']]==[('ordinary-lib',0),('ordinary-tests',0)];assert 'IsADirectoryError'in first['failure']
repair=get('pr02-world-lifetime-artifact-repair','build-receipt.json');assert repair['status']=='built'and repair['inputs_before']==repair['inputs_after'];assert [(x['name'],x['exit'])for x in repair['builds']]==[('native-lib',0),('native-tests',0)]
recovered=get('pr02-world-lifetime-artifact-repair','ordinary-recovered.json');assert recovered['existing_library']==first['libraries']['ordinary'];assert recovered['compiler_artifact']['profile']['test']
validation=get('pr02-world-lifetime-rust-validation','receipt.json');assert validation['status']=='passed'and len(validation['runs'])==18;assert sum(x['passed']for x in validation['runs'])==72
for result in validation['runs']:
 assert result['exit']==result['failed']==0 and result['passed']>0
 backend=result['configuration'];expect=recovered['sha256']if backend=='ordinary'else repair['tests']['native']['sha256'];assert result['binary_sha256']==expect
 assert result['environment']['GPU_PHYSICS_BACKEND']=='vulkan'and result['environment']['GPU_PHYSICS_ADAPTER']=='nvidia'
cleanup=get('pr02-world-lifetime-cleanup','receipt.json');assert cleanup['status']=='stopped'and cleanup['production_restored'];assert len(cleanup['unit_compiles'])==2 and len(cleanup['generators'])==3 and not cleanup['builds']and not cleanup['runs'];assert 'ordinary-both'in cleanup['failure']
C=get('pr02-world-lifetime-C-applicability','receipt.json');assert C['status']=='stopped'and C['production_restored']and len(C['unit_compiles'])==4 and len(C['builds'])==9 and len(C['runs'])==1;cpu=C['runs'][0];assert cpu['configuration']=='cpu'and cpu['summary']=={'summary':True,'observations':10,'mismatches':1}
proof=get('pr02-world-lifetime-CPU-contract','applicability-proof.json');assert proof['GPU_behavior_identical']and proof['before']['exit']==proof['after']['exit']==0;assert proof['before']['output_sha256']==proof['after']['output_sha256'];assert proof['CPU_raw_sha256']==cpu['stdout_sha256'];rows=[json.loads(l)for l in(raw/'pr02-world-lifetime-C-applicability/cpu/cleanup.stdout.jsonl').read_text().splitlines()];failed=[x for x in rows if x.get('match')is False];assert failed==[proof['CPU_original_observation']];assert failed[0]['field']=='stale-shape-invalid'and failed[0]['got']==1 and failed[0]['expected']==0
last=get('pr02-world-lifetime-CPU-contract','receipt.json');assert last['status']=='passed'and len(last['runs'])==8;assert last['prior_build_receipt_sha256']==H(raw/'pr02-world-lifetime-C-applicability/receipt.json');total=0
for result in last['runs']:
 cell=result['configuration'];fixture=result['fixture'];built=[x for x in C['builds']if x['configuration']==cell and x['fixture']==fixture];assert len(built)==1 and built[0]['binary_sha256']==result['binary_sha256'];expected=17 if fixture=='cleanup'else 13;assert result['exit']==0 and result['summary']=={'summary':True,'observations':expected,'mismatches':0};rows=[json.loads(l)for l in(raw/'pr02-world-lifetime-CPU-contract'/cell/(fixture+'.stdout.jsonl')).read_text().splitlines()];checks=[x for x in rows if'match'in x];assert len(checks)==expected and all(x['match']for x in checks);total+=expected
assert total==120
sources={}
with tarfile.open(raw/'source-inputs.tar.gz')as tar:
 for m in tar.getmembers():
  if m.isfile():assert not m.name.endswith(('.a','.o'));sources[m.name]=hashlib.sha256(tar.extractfile(m).read()).hexdigest()
for f,h in repair['inputs_before'].items():assert sources['pr02-world-lifetime-fix/candidate-inputs/'+f]==h
for f,h in cleanup['candidate_C'].items():assert sources['pr02-world-lifetime-cleanup/candidate-inputs/'+f]==h
artifact=get('pr02-world-lifetime-CPU-contract','compiler-artifact-audit.json')
for backend in ['ordinary','native']:
 audit=artifact['backends'][backend];roots=[x for x in audit['artifacts']if x['target']['name']=='gpu_physics'];assert len(roots)==1 and not roots[0]['fresh'];assert 'external-c-shim'in roots[0]['features']
 for f,h in audit['actual_Rust_shader_dependencies'].items():assert repair['inputs_before'][f]==h
 for cell in [backend+'-gpu',backend+'-both']:
  p=C['proof'][cell];assert p['linked_inputs'][C['libraries'][backend]['path']]==C['libraries'][backend]['sha256']
print(len(index),'raw files verified; 18 Rust processes/72 tests and 8 first GPU C processes/120 observations pass. Original receipt/source/CPU failures retained. No viewer/capture/release/performance acceptance.')
