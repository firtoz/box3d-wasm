#!/usr/bin/env python3
"""Validate portable PR03 baseline evidence; requires no compiled binary/GPU."""
from pathlib import Path
import hashlib,importlib.util,json,math,re,tarfile,tempfile,subprocess,shutil,sys
sys.dont_write_bytecode=True
B=Path(__file__).resolve().parent;raw=B/'raw';sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();read=lambda n:json.loads((raw/n).read_text());index=json.loads((B/'raw-index.json').read_text())
assert set(index)=={str(f.relative_to(B)) for f in raw.rglob('*') if f.is_file()}
for n,v in index.items():assert sha(B/n)==v['sha256'] and (B/n).stat().st_size==v['bytes'],n
first=read('first-build/receipt.json');fp=read('first-build/protocol-before-builds.json');repair=read('dependency-repair/receipt.json');rp=read('dependency-repair/protocol-before-builds.json');distance=read('distance/receipt.json');dp=read('distance/protocol.json');physical=read('ragdoll-mesh/receipt.json');pp=read('ragdoll-mesh/protocol.json');evaluation=read('ragdoll-mesh/evaluation.json')
assert first['status']=='stopped' and len(first['commands'])==4 and not first['fixtures'];assert [x['exit'] for x in first['commands']]==[0,0,0,1]
assert repair['status']=='built' and len(repair['commands'])==10 and len(repair['fixtures'])==10 and all(c['exit']==0 for c in repair['commands'])
for key in ['timing','candidates','retries','engine_processes']:assert fp['budget'][key]==rp['budget'][key]==0
assert rp['budget']['shared_C_compiler_commands']==0 and rp['budget']['fixture_link_commands']==10
assert rp['previous_protocol']['sha256']==sha(raw/'first-build/protocol-before-builds.json');assert rp['previous_receipt']['sha256']==sha(raw/'first-build/receipt.json')
for prefix,receipt in [('first-build',first),('dependency-repair',repair)]:
 assert receipt['protocol_sha256']==sha(raw/prefix/'protocol-before-builds.json');assert receipt['driver_sha256']==sha(raw/prefix/'run.py')
 for command in receipt['commands']:
  rel=Path(command['output']).relative_to(Path(fp['engine_build_receipt']).parents[1]/('pr03-baseline-builds' if prefix=='first-build' else 'pr03-baseline-cpu-link-repair'))
  for channel in ['stdout','stderr']:assert sha(raw/prefix/rel/(channel+'.log'))==command[channel+'_sha256']
# All actual fixture inputs are preserved, including the original rejected prototype.
with tarfile.open(raw/'first-build/fixture-source-inputs.tar.gz') as t:
 for n,h in fp['inputs_before'].items():assert hashlib.sha256(t.extractfile(n.split('/box3d-wasm/',1)[1]).read()).hexdigest()==h,n
assert sha(raw/'first-build/distance-original.cpp')==fp['original_distance_source_sha256']==dp['fixture_source_sha256']
assert 'NDEBUG' not in str([c['command'] for c in repair['commands']])
# Existing source archive provides exact source bytes for both reusable producers.
source_archive=B.parent/'pr02-world-lifetime-viewer-builds-2026-10-02/raw/compiled-source-inputs.tar.gz'
with tarfile.open(source_archive) as t:
 for name,count in [('engine-build.json',107),('viewer-parent.json',723)]:
  d=read('producers/'+name);assert d['inputs_before']==d['inputs_after'] and len(d['inputs_after'])==count
  for n,h in d['inputs_after'].items():assert hashlib.sha256(t.extractfile(n[6:] if n.startswith('../../') else n).read()).hexdigest()==h,n
cpu=read('producers/cpu-applicability.json');origin=read('producers/cpu-origin_viewer_receipt.json');assert len(cpu['compiled_units'])==119 and cpu['compiled_units']==origin['compiled_units'] and cpu['linked_archives']==origin['linked_archives'];assert cpu['inputs_before']==cpu['inputs_after']
for name,f in repair['fixtures'].items():
 command=repair['commands'][f['command_index']]['command'];assert f['binary'] in command and f['source'] in command
 for n,h in f['linked_inputs'].items():
  if n==rp['cells'].get(name.split('/')[0],{}).get('library'):assert h==rp['cells'][name.split('/')[0]]['library_sha256']
  else:
   producer=cpu if name.startswith('cpu/') else read('producers/'+name.split('/')[0]+'.json');assert producer['linked_archives'][n]==h,n
for prefix,p,receipt,number in [('distance',dp,distance,2),('ragdoll-mesh',pp,physical,24)]:
 assert receipt['status']=='completed-baseline' and len(receipt['results'])==len(p['cases'])==number
 assert receipt['protocol_sha256']==sha(raw/prefix/'protocol.json');assert receipt['driver_sha256']==sha(raw/prefix/'run.py')
 for case,row in zip(p['cases'],receipt['results']):
  assert case['name']==row['name'];assert row['exit']==(-6 if prefix=='distance' else 0)
  for channel,extension in [('stdout','log' if prefix=='distance' else 'txt'),('stderr','log')]:assert sha(raw/prefix/case['name']/(channel+'.'+extension))==row[channel+'_sha256']
  if case.get('cell')!='cpu':assert row['actual_NVIDIA_Vulkan'];assert 'NVIDIA GeForce RTX 4070 SUPER' in (raw/prefix/case['name']/'stderr.log').read_text()
  if prefix=='distance':assert row['first_discrepancy']==['distance-joint','0','1','-1.00004232','-1.00028491'] and not row['completed_fixture']
  assert case['fixture']==repair['fixtures'][(case['name']+'-both/distance-original') if prefix=='distance' else ('cpu' if case['cell']=='cpu' else case['cell']+'-gpu')+'/'+('frozen-mesh' if case['scene'].startswith('mesh-') else 'ragdoll')]
  assert case['environment'].get('GPU_PHYSICS_LIVE_CONTACT_ORDER','0')=='0'
  if case.get('cell')!='cpu':assert case['environment']['WGPU_BACKEND']=='vulkan' and case['environment']['VK_DRIVER_FILES']=='/usr/share/vulkan/icd.d/nvidia_icd.json'
 assert p['budget']['candidates']==p['budget']['retries']==p['budget']['headline_timing']==0

def module(name):
 spec=importlib.util.spec_from_file_location(name,raw/'evaluators'/name);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
health=module('check-ragdoll-health.py');mesh=module('compare-frozen-contacts.py');rm=raw/'ragdoll-mesh'
for backend in ['ordinary','native']:
 d=evaluation[backend];assert health.evaluate(health.measure(rm/'cpu/ragdolls/stdout.txt'),health.measure(rm/backend/'ragdolls/stdout.txt'))==d['physical']
 for scene in ['twist','twist-negative','tilted','tilted-negative','ragdolls-no-contacts']:
  rs=[]
  for cell in ['cpu',backend]:
   rows=[list(map(float,l.split())) for l in (rm/cell/scene/'stdout.txt').read_text().splitlines()];bodies=112 if scene=='ragdolls-no-contacts' else 1;steps=60 if bodies==112 else 120;assert len(rows)==(steps+1)*bodies and all(len(r)==16 and all(map(math.isfinite,r)) and r[:2]==list(divmod(i,bodies)) for i,r in enumerate(rows));rs.append(rows)
  err=max(math.dist(a[2:5],b[2:5]) for a,b in zip(*rs)) if bodies==112 else max(abs(x-y) for a,b in zip(*rs) for x,y in zip(a[2:],b[2:]));entry=d['isolated'][scene];assert err==entry['max_position_error' if bodies==112 else 'max_component_error'] and err<=entry['limit']
 for shape,count in [('grid',45),('torus',12)]:
  got=mesh.compare(rm/'cpu'/('mesh-'+shape)/'stdout.txt',rm/backend/('mesh-'+shape)/'stdout.txt',count,1e-5);assert all(d['mesh'][shape][k]==v for k,v in got.items())
  if shape=='grid':
   for cell in ['cpu',backend]:
    cases,ms=mesh.read(rm/cell/'mesh-grid/stdout.txt',count);assert all(x[1]>0 for x in cases.values());assert max(max(abs(x[0]),abs(x[1]-1),abs(x[2])) for g in ms.values() for x in g)==0
 # Run the archived unchanged host validator against derived views, retaining failure.
 with tempfile.TemporaryDirectory() as tmp:
  out=Path(tmp)
  for label,cell in [('cpu','cpu'),('gpu',backend)]:
   for scene in ['twist','twist-negative','tilted','tilted-negative','ragdolls-no-contacts','ragdolls']:
    for filename,suffix in [('stdout.txt','txt'),('stderr.log','log')]:shutil.copyfile(rm/cell/scene/filename,out/(scene+'-'+label+'.'+suffix))
  result=subprocess.run(['python3',str(raw/'evaluators/validate-ragdoll-reference.py'),str(out)],capture_output=True,text=True);assert result.returncode==d['original_full_reference_validator']['exit']==1;assert 'AssertionError: 0.06358576716417953' in result.stderr
print(f'Validated {len(index)} portable rawfiles,10fixturelinks,26baseline processes;physical passes and unchanged distance/trajectory failures retained. PR03/full-readiness remain open.')
