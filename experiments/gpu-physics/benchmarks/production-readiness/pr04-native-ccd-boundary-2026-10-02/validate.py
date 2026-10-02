#!/usr/bin/env python3
"""Validate original-limit evidence identity without building or running physics."""
from pathlib import Path
import difflib
import gzip
import hashlib
import json
from importlib.util import module_from_spec, spec_from_file_location
import posixpath
import subprocess
import sys
import tarfile
from analyze import analyze

ROOT=Path(__file__).resolve().parent
PARENT=ROOT.parent/'pr04-drag-ccd-boundary-2026-10-02'
sha=lambda data:hashlib.sha256(data).hexdigest()
load=lambda name:json.loads((ROOT/name).read_text())


def archived(name):
    with tarfile.open(ROOT/name) as tar:
        return {m.name:tar.extractfile(m).read() for m in tar.getmembers() if m.isfile()}


def prefix(name):
    lines=[]
    for line in gzip.open(ROOT/name,'rt'):
        if line.startswith('B ') and int(line.split()[1])>=240:break
        if line.startswith(('B ','F ','M ','P ')):lines.append(line.rstrip('\n'))
    assert len(lines)==14020
    assert sum(x.startswith('B ') for x in lines)==1200
    assert sum(x.startswith('F ') for x in lines)==2400
    return lines


index=load('raw-index.json')
assert len(index)==32
assert {str(f.relative_to(ROOT)) for f in (ROOT/'raw').rglob('*') if f.is_file()}==set(index)
for name,row in index.items():
    data=(ROOT/name).read_bytes()
    assert sha(data)==row['sha256'] and len(data)==row['bytes'],name
for original,row in load('reference-map.json').items():
    assert sha((ROOT/row['portable']).read_bytes())==row['sha256'],original
p=load('raw/protocol.json');r=load('raw/receipt.json')
assert p['budget']==dict(native_Rust_library_builds=1,fixture_CPP_compilations_and_links=1,
                        fresh_native_diagnostic_processes=1,completed_steps=240,observer_sources=3,
                        CPU_solver_builds=0,production_candidates=0,retries=0,headline_timing=0,
                        hull_C_translation_units_included_in_library_build=4)
assert r['status']=='completed-native-boundary-diagnostic' and 'running' not in r
assert sha((ROOT/'raw/protocol.json').read_bytes())==r['protocol_sha256']
assert sha((ROOT/'raw/run.py').read_bytes())==r['driver_sha256']
assert sha((ROOT/'raw/cc-observer.py').read_bytes())==r['compiler_observer_sha256']==p['compiler_observer_sha256']
assert p['build_environment_overrides']=={'CC':p['workspace'].split('/workspace/',1)[0]+'/cc-observer.py'}
old=load('raw/reference/parent-protocol.json');old_receipt=load('raw/reference/parent-receipt.json')
assert sha((ROOT/'raw/reference/parent-protocol.json').read_bytes())==p['parent_sha256']==old_receipt['protocol_sha256']
assert p['original_Rust_inputs']==old['compiled_Rust_inputs_current']
# The earlier portable report supplies complete C-consumer/CPU-observer provenance.
# Its original coverage gap stays historical, with this new direct capture added.
assert (PARENT/'raw/protocol.json').read_bytes()==(ROOT/'raw/reference/parent-protocol.json').read_bytes()
assert (PARENT/'raw/receipt.json').read_bytes()==(ROOT/'raw/reference/parent-receipt.json').read_bytes()
subprocess.run([sys.executable,'-B',str(PARENT/'validate.py')],check=True,stdout=subprocess.DEVNULL)
producer=json.loads((PARENT/'raw/reference/Rust-producer.json').read_text())
sources=archived('raw/observer-source-inputs.tar.gz')
assert {n:sha(data) for n,data in sources.items()}==p['observer_Rust_inputs']==r['compiled_inputs_after']
assert len(sources)==107
changed=[n for n,h in p['observer_Rust_inputs'].items() if h!=p['original_Rust_inputs'][n]]
assert sorted(changed)==sorted(p['changed_observer_sources']+['Cargo.toml','Cargo.lock'])
for name in ['Cargo.toml','Cargo.lock']:
    assert sha(sources[name])==producer['native_generated_inputs']['target/native-workspace/experiments/gpu-physics/'+name]
backend=archived('raw/native-backend-inputs.tar.gz')
assert len(backend)==len(p['native_backend_inputs'])==453
for n,h in p['native_backend_inputs'].items():
    relative=n.split('/target/native-backend/',1)[1]
    assert sha(backend[relative])==h==producer['native_generated_inputs']['target/native-backend/'+relative]

for name in p['changed_observer_sources']:
    before=(ROOT/'raw/original'/name).read_text()
    assert sha(before.encode())==p['original_Rust_inputs'][name]
    after=sources[name].decode()
    patch=''.join(difflib.unified_diff(before.splitlines(True),after.splitlines(True),fromfile='original/'+name,tofile='observer/'+name))
    assert (ROOT/'raw'/(Path(name).stem+'.patch')).read_text()==patch
    if name.endswith('.wgsl'):
        assert all(line.startswith('---') for line in patch.splitlines() if line.startswith('-'))
        assert 'boundary_begin(body,end,flags);' in after and 'boundary_finish(body);' in after
        assert 'ccd_boundary[32u*body+3u]=bitcast<u32>(fraction);' in after

assert len(r['builds'])==len(r['links'])==len(r['results'])==1
for row in r['builds']+r['links']+r['results']:
    assert row['exit']==0
    for stream in ['stdout','stderr']:
        assert sha((ROOT/'raw'/row['name']/(stream+'.log')).read_bytes())==row[stream+'_sha256']
assert r['builds'][0]['command']==p['build_command'] and r['links'][0]['command']==p['link_command']
assert r['binary']['Rust_library_sha256']==r['Rust_library']['sha256']
artifacts=[]
for line in (ROOT/'raw/native-build/stdout.log').read_text().splitlines():
    try:record=json.loads(line)
    except ValueError:continue
    if record.get('reason')=='compiler-artifact' and record['target']['name']=='gpu_physics':artifacts.append(record)
assert artifacts==r['Rust_library']['compiler_artifacts'] and len(artifacts)==1
assert not artifacts[0]['fresh']
assert sorted(artifacts[0]['features'])==sorted(producer['libraries']['native']['compiler_artifacts'][0]['features'])
assert artifacts[0]['profile']==producer['libraries']['native']['compiler_artifacts'][0]['profile']
assert r['binary']['path']==p['case']['command'][0]

invocations=[json.loads(line) for line in (ROOT/'raw/C-compiler-invocations.jsonl').read_text().splitlines()]
units=[x for x in invocations if x['compile_unit']]
assert units==r['actual_C_units'] and len(units)==4 and all(x['exit']==0 for x in units)
deps={n:h for unit in units for n,h in unit['dependencies'].items()}
dependency_bytes=archived('raw/actual-C-compile-dependencies.tar.gz')
assert len(deps)==len(dependency_bytes)==r['C_dependency_count']==107
assert {n.lstrip('/'):h for n,h in deps.items()}=={n:sha(data) for n,data in dependency_bytes.items()}
refs=load('reference-map.json')
original_repo=p['workspace'].split('/experiments/gpu-physics/artifacts/',1)[0]
def canonical_C_path(name):
    # Recover the frozen workspace symlink from prepare.py, without touching
    # original absolute paths or requiring them to exist on the verifier's host.
    prefix=p['workspace']+'/../../box3d'
    if name.startswith(prefix):name=original_repo+'/box3d'+name[len(prefix):]
    return posixpath.normpath(name)
for unit in units:
    assert unit['command'][0]=='/usr/bin/cc' and unit['command'][-3:]==['-MD','-MF',unit['dependency_file']]
    source=unit['command'][unit['command'].index('-c')+1]
    assert deps[canonical_C_path(source)]==p['C_engine_build_inputs'][canonical_C_path(source)]
    data=(ROOT/refs[unit['dependency_file']]['portable']).read_text().replace('\\\n',' ')
    names=data.split(':',1)[1].split()
    assert {canonical_C_path(n) for n in names}==set(unit['dependencies'])
    for n,h in unit['dependencies'].items():
        if n in p['C_engine_build_inputs']:assert h==p['C_engine_build_inputs'][n]

old_link=next(x for x in old['links'] if x['name']=='native')
command=list(old_link['command'])
original_lib=next(n for n in command if n.endswith('/native-candidate.a'))
command[command.index(original_lib)]=r['Rust_library']['path'];command[-1]=r['binary']['path']
assert command==p['link_command']
for n,h in old_link['reused_inputs'].items():
    if n!=original_lib:assert p['reused_link_inputs'][n]==h
CPU_archive=next(n for n in p['reused_link_inputs'] if n.endswith('/libbox3d_cpu_observer.a'))
assert p['reused_link_inputs'][CPU_archive]==old_receipt['CPU_observer_archive_sha256']
old_case=next(x for x in old['cases'] if x['name']=='native')
overrides=dict(old_case['environment_overrides'])
for key in ['GPU_PHYSICS_PIPELINE_CACHE_DIR','BOTH_DRAG_TRACE']:overrides[key]=p['case']['environment_overrides'][key]
overrides['GPU_PHYSICS_NATIVE_CCD_BOUNDARY']='1'
assert overrides==p['case']['environment_overrides']==r['results'][0]['environment_overrides']
row=r['results'][0]
assert row['command']==p['case']['command'] and row['pass'] and row['completed_prefix'] and row['coverage'] and row['observer_neutral']
stderr=(ROOT/'raw/native/stderr.log').read_text()
assert 'NVIDIA GeForce RTX 4070 SUPER' in stderr and 'backend=Vulkan' in stderr
assert 'DIAGNOSTIC completed 240-step original drag prefix' in (ROOT/'raw/native/stdout.log').read_text()
actual=prefix('raw/native/comparison.txt.gz');baseline=prefix('raw/reference/native-baseline.txt.gz')
assert actual==baseline
assert sha(('\n'.join(actual)+'\n').encode())==row['baseline_prefix_sha256']==row['observer_prefix_sha256']
assert sha(gzip.decompress((ROOT/'raw/native/comparison.txt.gz').read_bytes()))==row['comparison_sha256']
analysis=analyze();assert load('analysis.json')==analysis
assert analysis['native_internal_GPU_TOI_observed'] and not analysis['PR04_complete'] and not analysis['original_full_dragging_acceptance']
assert analysis['raw_GPU_records']==39 and analysis['unique_selected_GPU_steps']==13 and analysis['CPU_TOI_records']==0
impact=next(x for x in analysis['window'] if x['frame']==227)
assert impact['branch']==4 and impact['joint_count']==1
assert impact['raw_words_hex'][3]=='3f1f9631'
assert impact['CPU']['fast']==0 and impact['CPU']['motion']<impact['CPU']['threshold']
assert impact['GPU']['motion_m']>impact['GPU']['cutoff_m']
assert impact['pre_position_difference_m']<.002 and impact['post_position_difference_m']>.017
assert impact['correction_norm_m']>.016 and impact['reconstruction_residual_m']<1e-6
assert impact['velocity_difference_m_per_s']<2e-6
spec=spec_from_file_location('ccd_handoff_audit',ROOT/'handoff-audit.py')
module=module_from_spec(spec);spec.loader.exec_module(module)
assert load('handoff-audit.json')==module.audit()
applicability=load('source-applicability.json')
assert applicability['original_Rust_inputs_match'] and applicability['original_Rust_input_count']==107
assert applicability['frozen_native_backend_inputs_match'] and applicability['frozen_C_engine_inputs_match']
assert applicability['reused_reference_and_link_input_hashes_match']
assert applicability['captured_binary_hash_matches'] and applicability['artifact_observer_library_hash_matches']
assert applicability['shared_native_target_output']['sha256']==r['Rust_library']['sha256']
assert applicability['shared_native_target_output']['diagnostic_observer_only'] and not applicability['PR04_complete']
print('PASS:32 raw files, 107 compiled observer inputs, 453 unchanged native backend inputs, four actual C units/107 dependencies, fixed one-build/link/process budget. Native internal TOI directly captured; 14,020 archived printed prefix lines match exactly. Full dragging/PR04/release remain OPEN; no production candidate or timing.')
