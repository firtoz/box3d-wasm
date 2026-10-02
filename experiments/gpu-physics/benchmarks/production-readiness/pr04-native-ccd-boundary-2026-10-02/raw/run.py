from pathlib import Path
import gzip
import hashlib
import json
import os
import resource
import shutil
import signal
import subprocess
import tarfile
import time

REPO = Path('/home/firtoz/work/2026/box3d-wasm')
ENGINE = REPO / 'experiments/gpu-physics'
A = ENGINE / 'artifacts/production-readiness/pr04-native-ccd-boundary'
W = A / 'workspace/experiments/gpu-physics'
P = A / 'protocol.json'
p = json.loads(P.read_text())
assert not (A / 'receipt.json').exists()
resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
sha = lambda f: hashlib.sha256(Path(f).read_bytes()).hexdigest()
s = dict(status='preflight', protocol_sha256=sha(P), driver_sha256=sha(__file__),
         compiler_observer_sha256=sha(A/'cc-observer.py'), started_unix=time.time(),
         toolchains={tool:subprocess.check_output([tool, '--version'],text=True) for tool in ['rustc','cargo','g++','cc']},
         builds=[],links=[],results=[])


def save():
    temporary = A / 'receipt.tmp'
    temporary.write_text(json.dumps(s, indent=2) + '\n')
    os.replace(temporary, A/'receipt.json')


def check():
    for n, h in p['original_Rust_inputs'].items(): assert sha(ENGINE/n)==h,n
    for n, h in p['observer_Rust_inputs'].items(): assert sha(W/n)==h,n
    for field in ['native_backend_inputs','C_engine_build_inputs','references','reused_link_inputs']:
        for n, h in p[field].items(): assert sha(n)==h,n
    old=json.loads(Path(p['parent']).read_text())
    for n,h in old['CPP_consumer_inputs'].items(): assert sha(ENGINE/n)==h,n
    for unit in old['compiled_units']:
        assert sha(unit['file'])==unit['source_sha256'] and sha(unit['object'])==unit['object_sha256']
    assert sha(p['case']['baseline_trace'])==p['case']['baseline_trace_sha256']


def execute(command,name,env,timeout):
    folder=A/name;folder.mkdir();started=time.time()
    with (folder/'stdout.log').open('w') as out,(folder/'stderr.log').open('w') as err:
        proc=subprocess.Popen(command,cwd=ENGINE,env=env,stdout=out,stderr=err,start_new_session=True)
        s['running']=dict(name=name,pid=proc.pid,command=command);save()
        try: code=proc.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            os.killpg(proc.pid,signal.SIGKILL);proc.wait();code='timeout'
    s.pop('running',None)
    return dict(name=name,command=command,cwd=str(ENGINE),exit=code,
                stdout_sha256=sha(folder/'stdout.log'),stderr_sha256=sha(folder/'stderr.log'),
                elapsed_seconds_incidental=time.time()-started)


def prefix(data):
    lines=[]
    for line in data.decode().splitlines():
        if line.startswith('B ') and int(line.split()[1])>=240:break
        if line.startswith(('B ','F ','M ','P ')):lines.append(line)
    assert sum(x.startswith('B ') for x in lines)==1200
    assert sum(x.startswith('F ') for x in lines)==2400
    assert len(lines)==14020
    return lines


save()
try:
    check();s['status']='building';save()
    env=os.environ.copy();env.update(p['build_environment_overrides'])
    row=execute(p['build_command'],'native-build',env,p['build_watchdog_seconds']);s['builds'].append(row);save()
    assert row['exit']==0,row
    artifacts=[]
    for line in (A/'native-build/stdout.log').read_text().splitlines():
        try: record=json.loads(line)
        except ValueError:continue
        if record.get('reason')=='compiler-artifact' and record['target']['name']=='gpu_physics':artifacts.append(record)
    libraries=[Path(n) for record in artifacts for n in record['filenames'] if n.endswith('.a')]
    assert len(libraries)==1 and all('native-command-cache' in x['features'] for x in artifacts)
    shutil.copyfile(libraries[0],A/'native-observer.a')
    s['Rust_library']=dict(path=str(A/'native-observer.a'),sha256=sha(A/'native-observer.a'),compiler_artifacts=artifacts)
    check();s['compiled_inputs_after']={n:sha(W/n) for n in p['observer_Rust_inputs']}
    invocations=[json.loads(line) for line in (A/'C-compiler-invocations.jsonl').read_text().splitlines()]
    units=[x for x in invocations if x['compile_unit']]
    assert len(units)==4 and all(x['exit']==0 for x in units)
    s['actual_C_units']=units
    dependencies={n:h for unit in units for n,h in unit['dependencies'].items()}
    for n,h in dependencies.items(): assert sha(n)==h,n
    with tarfile.open(A/'actual-C-compile-dependencies.tar.gz','w:gz') as tar:
        for n in dependencies:tar.add(n,arcname=n.lstrip('/'),recursive=False)
    s['C_dependency_count']=len(dependencies);s['status']='linking';save()
    row=execute(p['link_command'],'native-link',os.environ.copy(),p['link_watchdog_seconds']);s['links'].append(row);save()
    assert row['exit']==0,row
    s['binary']=dict(path=p['case']['command'][0],sha256=sha(p['case']['command'][0]),Rust_library_sha256=s['Rust_library']['sha256']);save()
    check();s['status']='capturing';save()
    case=p['case'];env={k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_','WGPU_','VK_','BOTH_DRAG_'))};env.update(case['environment_overrides'])
    row=execute(case['command'],'native',env,p['process_watchdog_seconds']);row['environment_overrides']=case['environment_overrides'];s['results'].append(row);save()
    text=(A/'native/stderr.log').read_text();stdout=(A/'native/stdout.log').read_text()
    row['actual_NVIDIA_Vulkan']='NVIDIA GeForce RTX 4070 SUPER' in text and 'backend=Vulkan' in text
    row['completed_prefix']='DIAGNOSTIC completed 240-step original drag prefix' in stdout
    raw=(A/'native/comparison.txt').read_bytes();expected=prefix(gzip.decompress(Path(case['baseline_trace']).read_bytes()));observed=prefix(raw)
    row['observer_neutral']=expected==observed
    row['baseline_prefix_sha256']=hashlib.sha256(('\n'.join(expected)+'\n').encode()).hexdigest()
    row['observer_prefix_sha256']=hashlib.sha256(('\n'.join(observed)+'\n').encode()).hexdigest()
    row['comparison_sha256']=hashlib.sha256(raw).hexdigest()
    if observed!=expected:
        row['first_prefix_difference']=next((dict(line=i,expected=a,actual=b) for i,(a,b) in enumerate(zip(expected,observed)) if a!=b),dict(expected_lines=len(expected),observed_lines=len(observed)))
    records=[json.loads(line.split(' ',1)[1]) for line in text.splitlines() if line.startswith('gpu-native-ccd-boundary ')]
    bystep={}
    for record in records:
        if record['step'] in bystep:assert record==bystep[record['step']],'Duplicate boundary disagreement'
        bystep[record['step']]=record
    row['boundary_records']=records;row['selected_steps']=sorted(bystep)
    row['actual_convex_present']=all(x['convex_ccd_present'] for x in bystep.values())
    row['coverage']=row['selected_steps']==list(range(221,234)) and row['actual_convex_present'] and all(x.get('words',[99])[0]==1 for x in bystep.values())
    (A/'native/comparison.txt.gz').write_bytes(gzip.compress(raw,mtime=0))
    row['pass']=row['exit']==0 and row['actual_NVIDIA_Vulkan'] and row['completed_prefix'] and row['observer_neutral'] and row['coverage']
    save();check();assert row['pass'],row
    s.update(status='completed-native-boundary-diagnostic',finished_unix=time.time(),all_observer_checks_pass=True);save()
    print(json.dumps({k:row[k] for k in ['exit','actual_NVIDIA_Vulkan','completed_prefix','observer_neutral','coverage','pass']}),flush=True)
except BaseException as error:
    s.update(status='stopped',error=repr(error),finished_unix=time.time(),unlaunched_native_process=not s['results']);save();raise
