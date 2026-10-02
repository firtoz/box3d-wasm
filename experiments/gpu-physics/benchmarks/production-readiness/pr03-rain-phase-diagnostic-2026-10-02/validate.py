#!/usr/bin/env python3
"""Offline verification of source receipts, lossless captures and retained screens."""
from pathlib import Path
import gzip,hashlib,importlib.util,json,re,shlex,sys,tarfile,tempfile,itertools,statistics
B=Path(__file__).resolve().parent;raw=B/'raw';sys.dont_write_bytecode=True

def sha(path):
    h=hashlib.sha256()
    with Path(path).open('rb') as f:
        while data:=f.read(1024*1024):h.update(data)
    return h.hexdigest()
def read(name):return json.loads((raw/name).read_text())
idx=json.loads((B/'raw-index.json').read_text())
assert set(idx)=={str(f.relative_to(B)) for f in raw.rglob('*') if f.is_file()}
for name,v in idx.items():assert (B/name).stat().st_size==v['bytes'] and sha(B/name)==v['sha256'],name
refs=read('producer-references.json')
for name,v in refs.items():assert sha(B/name)==v['sha256'] and (B/name).stat().st_size==v['bytes'],name
p=read('original/protocol.json');o=read('original/receipt.json');q=read('remaining/protocol.json');r=read('remaining/receipt.json')
assert o['protocol_sha256']==q['original_protocol_sha256']==sha(raw/'original/protocol.json')
assert o['driver_sha256']==q['original_driver_sha256']==sha(raw/'original/run.py')
assert q['original_receipt_sha256']==sha(raw/'original/receipt.json')
assert r['protocol_sha256']==sha(raw/'remaining/protocol.json') and r['driver_sha256']==sha(raw/'remaining/run.py')
assert o['status']=='stopped' and len(o['results'])==1 and o['unlaunched']==['observer30','phase180']
assert "'sokol_errors': [('30', '0')]" in o['error'] and o['results'][0]['exit']==0
assert len(o['builds'])==2 and all(v['exit']==0 for v in o['builds']) and not r['builds']
assert p['budget']['GPU_diagnostic_processes']==3 and p['budget']['CXX_observer_compilations']==p['budget']['observer_links']==1
assert q['budget']['remaining_GPU_processes']==2 and q['budget']['repeated_processes']==q['budget']['builds']==0
assert all(p['budget'][k]==0 for k in ['Rust_builds','CPU_builds','solver_candidates','retries','headline_timing'])
assert r['status']=='completed-diagnostic' and len(r['results'])==2 and not r.get('running')
assert r['observer_equivalence']=={'frames':30,'ordered_body_joint_semantic_fields_exact':True,'scope':'capturedpublicphysics/identity/residual subset;notfullfuture-state'}
assert o['observer_executable_sha256']==q['observer_executable_sha256']==r['observer_executable_sha256']
parent=B.parent/'pr02-world-lifetime-viewer-builds-2026-10-02/raw'
pv=json.loads((parent/'ordinary-gpu/receipt.json').read_text());pp=json.loads((parent/'receipt.json').read_text())
assert sha(parent/'ordinary-gpu/receipt.json')==p['parent_viewer_receipt_sha256'] and sha(parent/'receipt.json')==p['parent_viewer_parent_receipt_sha256']
assert pv['compiled_units']==p['parent_compiled_units'] and len(pv['compiled_units'])==124
assert p['uncompiled_test_only_exception']==['src/gpu_invariants.rs']
assert set(p['local_source_inputs'])==set(pp['inputs_after']) and len(p['local_source_inputs'])==723
for n,h in p['local_source_inputs'].items():
    if n=='src/gpu_invariants.rs':assert h==sha(B.parent/'pr03-sleeper-fixture-2026-10-02/raw/fixture-builds/candidate-gpu_invariants.rs')
    else:assert pp['inputs_after'][n]==h,n
with tarfile.open(parent/'compiled-source-inputs.tar.gz') as t:
    sourcehash={m.name:hashlib.sha256(t.extractfile(m).read()).hexdigest() for m in t.getmembers() if m.isfile()}
for n,h in pp['inputs_after'].items():assert sourcehash[n.replace('../../box3d/','box3d/')] == h,n
with tarfile.open(parent/'ordinary-gpu/generated-inputs.tar.gz') as t:
    generated={m.name:hashlib.sha256(t.extractfile(m).read()).hexdigest() for m in t.getmembers() if m.isfile()}
for unit in pv['compiled_units']:
    name=unit['file']
    if '/ordinary-gpu/cmake/' in name:h=generated[name.split('/ordinary-gpu/cmake/',1)[1]]
    elif '/box3d-wasm/box3d/' in name:h=sourcehash['box3d/'+name.split('/box3d-wasm/box3d/',1)[1]]
    else:h=sourcehash[name.split('/experiments/gpu-physics/',1)[1]]
    assert h==unit['source_sha256'],name
    assert p['reused_object_archive_hashes'][unit['object']]==unit['object_sha256']
for n,h in pv['linked_archives'].items():assert p['reused_object_archive_hashes'][n]==h
old=(raw/'sources/native-samples/sokol_bench_hooks.original.cpp').read_text();new=(raw/'sources/native-samples/sokol_bench_hooks.cpp').read_text()
assert hashlib.sha256(old.encode()).hexdigest()==p['original_source_sha256']
start=new.index('// Diagnostic observer only:');end=new.index('void gpu_sokol_bench_begin_frame(void)',start)
new=new[:start].removesuffix('\n')+new[end:]
for line in ['    phase_progress("phase",g_mark_phase);\n','    phase_progress("health_begin","health");\n','    phase_progress("health_end","health");\n','    phase_progress("frame","end");\n']:assert new.count(line)==1;new=new.replace(line,'')
# The frozen generator inserts an extra newline before the helper.
assert new==old or new.replace('\n\nvoid gpu_sokol_bench_begin_frame(void)','\nvoid gpu_sokol_bench_begin_frame(void)',1)==old
for name,h in p['observer_sources'].items():assert sha(raw/'sources'/name.split('/sources/',1)[1])==h
for build,recipe in zip(o['builds'],p['build_commands'],strict=True):
    assert build['command']==recipe['command'] and build['name']==recipe['name']
    for channel in ['stdout','stderr']:assert sha(raw/build['name']/(channel+'.log'))==build[channel+'_sha256']
assert sha(raw/'display-child.py')==p['display_dependency']['helper_sha256']
for key,h in r['evaluator_hashes'].items():assert sha(raw/'evaluators'/key)==h
sys.path.insert(0,str(raw/'evaluators'));spec=importlib.util.spec_from_file_location('rain',raw/'evaluators/compare-rain-lifetimes.py');rain=importlib.util.module_from_spec(spec);spec.loader.exec_module(rain)
from native_scene_validate import record_complete

def header(path):
    with path.open() as f:
        prefix=[]
        for line in f:
            if line.strip()=='"frames": [':break
            prefix.append(line)
        return json.loads(''.join(prefix)+'"frames": []\n}')

def records_validate(path,steps):
    h=header(path);assert (h['worker_count'],h['enable_sleep'],h['warmup'],h['unpaced'],h['completed_step_mode'])==(8,True,0,True,False)
    bodies=joints=0
    for count,f in enumerate(rain.records(path,steps),1):
        status,detail=record_complete(dict(h,warmup=count-1,timed=1,measured=1,frames=[dict(f,i=0)]),'Benchmark/Rain',1);assert status=='ok',detail
        for joint in f['joints']:rain.spherical_limits(joint,True)
        bodies+=len(f['bodies']);joints+=len(f['joints'])
    assert count==steps
    return {'frames':count,'body_observations':bodies,'joint_observations':joints,'status':'originalcomplete-record/sphericalchecks;no600stepclaim'}

captures=read('captures.json');summary=read('phase-summary.json')
with tempfile.TemporaryDirectory() as td:
    paths={}
    for name,capture in captures.items():
        z=Path(td)/(name+'.gz');target=Path(td)/(name+'.json');paths[name]=target
        with z.open('wb') as out:
            for part in capture['parts']:
                assert sha(B/part['path'])==part['sha256']
                out.write((B/part['path']).read_bytes())
        assert sha(z)==capture['gzip_sha256'] and z.stat().st_size==capture['gzip_bytes']
        with gzip.open(z,'rb') as source,target.open('wb') as out:
            while data:=source.read(1024*1024):out.write(data)
        assert sha(target)==capture['raw_sha256'] and target.stat().st_size==capture['raw_bytes']
    trials=[r['retained_control'],*r['results']]
    for c,row in zip(p['cases'],trials,strict=True):
        name=c['name'];d=raw/'cases'/name;assert row['name']==name and row['exit']==row['child_exit']==0 and row['actual_NVIDIA_Vulkan']
        assert row['sokol_errors']==[[str(c['steps']),'0']]
        assert c['environment']['GPU_PHYSICS_LIVE_CONTACT_ORDER']=='0' and c['environment']['GPU_PHYSICS_BACKEND']=='vulkan'
        assert c['environment']['GPU_BENCH_PHASE_PROGRESS']==str(int(c['observer_enabled']))
        assert c['command'][c['command'].index('--timed')+1]==str(c['steps'])
        for channel in ['stdout','stderr']:assert sha(d/(channel+'.log'))==row[channel+'_sha256']
        assert json.loads((d/'child-exit.json').read_text())['exit']==0
        err=(d/'stderr.log').read_text();assert 'NVIDIA GeForce RTX 4070 SUPER' in err and 'Vulkan' in err
        phases=[json.loads(line[15:]) for line in err.splitlines() if line.startswith('phase-progress ')]
        frames=[v for v in phases if v['kind']=='frame'];assert row['phase_rows']==len(phases) and row['phase_frames']==len(frames)==(c['steps'] if c['observer_enabled'] else 0)
        assert summary[name]['phase_rows']==len(phases) and summary[name]['last_phase_row']==(phases[-1] if phases else None)
        if frames:
            computed={k:{'sum':sum(v[k] for v in frames)/1e6,'median':statistics.median(v[k] for v in frames)/1e6,'min':min(v[k] for v in frames)/1e6,'max':max(v[k] for v in frames)/1e6} for k in ['physics_ns','health_ns','draw_ns','render_ns','profile_ns']}
            assert computed==summary[name]['clocks_ms']
            assert sum(v['health_ns'] for v in frames)/sum(v['physics_ns'] for v in frames)==summary[name]['health_fraction_of_physics_sum']
        for i,v in enumerate(frames,1):assert v['frame']==v['submitted']==v['completed']==i and v['health_ns']<=v['physics_ns']
        assert captures[name]['raw_sha256']==row['health_sha256'] and records_validate(paths[name],c['steps'])==row['record_validation']
    fields=['sample','submitted_step','body_count','joint_count','nan_count','min_y','max_y','max_speed','exploded','bodies','joints']
    for c,g in zip(rain.records(paths['control30'],30),rain.records(paths['observer30'],30),strict=True):assert all(c[k]==g[k] for k in fields)
    a=read('host-analysis/host-analysis-protocol.json');ar=read('host-analysis/host-analysis-receipt.json');assert ar['protocol_sha256']==sha(raw/'host-analysis/host-analysis-protocol.json') and ar['driver_sha256']==sha(raw/'host-analysis/compare-prefix.py')
    assert a['phase_receipt_sha256']==sha(raw/'remaining/receipt.json') and a['evaluator_sha256']==sha(raw/'evaluators/compare-rain-lifetimes.py')
    assert ar['projected_CPU_sha256']==captures['cpu600-prefix180']['raw_sha256'] and a['GPU_source_sha256']==captures['phase180']['raw_sha256']
    original_cpu=B.parent/'pr03-rain-launch-timeout-2026-10-02/raw/display-repair/portable-health/cpu-rain'
    manifest=json.loads((original_cpu/'manifest.json').read_text());assert manifest['raw_sha256']==a['CPU_source_sha256'] and manifest['raw_bytes']==a['CPU_source_bytes']
    assert ar['report_sha256']==sha(raw/'host-analysis/residual-comparison.json')
    for channel in ['stdout','stderr']:assert sha(raw/'host-analysis'/('host-analysis.'+channel+'.log'))==ar[channel+'_sha256']
    # Compare the projection with the original ordered prefix; old full-record
    # validation and hash receipts remain required in the source report.
    parts=sorted(original_cpu.glob('health.json.gz.part*'));z=Path(td)/'cpu600.gz'
    with z.open('wb') as out:
        for part in parts:out.write(part.read_bytes())
    with gzip.open(z,'rt') as f:
        prefix=[]
        for line in f:
            if line.strip()=='"frames": [':break
            prefix.append(line)
        original_header=json.loads(''.join(prefix)+'"frames": []\n}')
        original_header.update(a['CPU_window_header_changes']);assert header(paths['cpu600-prefix180'])==original_header
        digest=hashlib.sha256()
        for i,projected in enumerate(rain.records(paths['cpu600-prefix180'],180)):
            frame=json.loads(f.readline().strip().removesuffix(','));assert frame==projected and frame['i']==i
            digest.update(json.dumps(frame,separators=(',',':')).encode()+b'\n')
        assert digest.hexdigest()==ar['ordered_CPU_frame_semantics_sha256']
    report=rain.compare(paths['cpu600-prefix180'],paths['phase180'],180,True)
    assert report==read('host-analysis/residual-comparison.json') and ar['exit']==1 and report['residual_screen_status']=='fail'
    assert report['first_failures'][0]['frame']==106 and report['matched_joint_observations']==272160
print(f'Validated {len(idx)} raw files: three diagnostic processes, exact30-frame observer semantics, complete180-frame partial capture, losslessCPU-prefix correspondence and retained original residual-screen failure. No production/default change,600-step qualification or headline timing.')
