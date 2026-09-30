#!/usr/bin/env python3
"""Fixed desktop experiment. Every attempt is retained; no scaling or extra rounds."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import runpy
import shutil
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
FIXTURES = [('falling-cubes', 50000), ('mixed-stacks', 4096), ('mixed-topology', 12288)]
NATIVE = runpy.run_path(str(ROOT/'scripts/measure-samples-application.py'))['NATIVE']

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def write(path, data):
    path.write_text(json.dumps(data, indent=2)+'\n')

def output(command):
    return subprocess.run(command, cwd=ROOT, capture_output=True, text=True, check=True).stdout

def sources():
    paths = [ROOT/'Cargo.toml', ROOT/'Cargo.lock', ROOT/'build.rs']
    for directory in ['src', 'shaders', 'compiler/native-backend']:
        paths.extend(p for p in (ROOT/directory).rglob('*') if p.is_file())
    paths.extend([ROOT/'scripts/build-native-cache.sh', ROOT/'scripts/prepare-native-backend.py'])
    return {str(p.relative_to(ROOT)): digest(p) for p in sorted(set(paths))}

def freeze(raw, name):
    # Called only after an explicitly completed successful build.
    target = raw/'binaries'/name
    target.parent.mkdir(exist_ok=True)
    assert not target.exists(), 'never overwrite a frozen binary'
    shutil.copy2(ROOT/'target/native-cache-build/release/gpu-physics', target)
    patch = output(['git', 'diff', 'HEAD', '--', 'src', 'shaders', 'Cargo.toml', 'Cargo.lock', 'build.rs'])
    (raw/f'{name}.patch').write_text(patch)
    write(raw/f'{name}-build.json', {
        'revision': output(['git', 'rev-parse', 'HEAD']).strip(),
        'engine_sources': sources(), 'binary_sha256': digest(target),
        'build_command': 'bash scripts/build-native-cache.sh build --release --bin gpu-physics',
        'build_log': f'build-{name}.log', 'source_patch': f'{name}.patch',
        'patch_sha256': digest(raw/f'{name}.patch'),
        'native_config_sha256': digest(ROOT/'target/native-backend/cargo-config.toml'),
        'rustc': output(['rustc', '-Vv']),
    })

def run(raw, phase, scene, count, schedule, binary_name, trial, diagnostic=False):
    folder = raw/phase
    folder.mkdir(exist_ok=True)
    stem = folder/f'{scene}-{schedule}-{trial}'
    receipt_path = stem.with_suffix('.receipt.json')
    binary = raw/'binaries'/binary_name
    if receipt_path.exists():
        receipt = json.loads(receipt_path.read_text())
        assert receipt['status'] == 'ok', 'retained incomplete/failed run: inspect before resuming'
        assert receipt['binary_sha256'] == digest(binary)
        return
    env = {k:v for k,v in os.environ.items() if not k.startswith(('GPU_', 'VK_', 'WGPU_'))}
    env.update(NATIVE, GPU_PHYSICS_ADAPTER='nvidia', GPU_PHYSICS_COMPONENT_TGS={
        'global':'0', 'component':'1', 'auto':'auto', 'candidate':'split'}[schedule],
        GPU_PHYSICS_COLOR_PREFIX='20', GPU_PHYSICS_BENCH_COMPLETED_ONLY='1',
        GPU_PHYSICS_PIPELINE_CACHE_DIR=str(Path.home()/'.cache/box3d-gpu-physics/pipelines'))
    if diagnostic:
        env.update(GPU_PHYSICS_TOPOLOGY_SAMPLES='1', GPU_PHYSICS_PROFILE_HOST='1', GPU_PHYSICS_FULL_REPLAY='0')
    command = [str(binary), '--scene', scene, '--bodies', str(count), '--warmup', '90',
        '--frames', '240', '--no-sleep', '--bench', str(stem.with_suffix('.json')), '--metric-runs', '1']
    receipt = {'status':'started', 'phase':phase, 'scene':scene, 'count':count,
        'schedule':schedule, 'trial':trial, 'binary':binary_name, 'binary_sha256':digest(binary),
        'command':command, 'environment':{k:v for k,v in env.items() if k.startswith(('GPU_', 'VK_', 'WGPU_'))},
        'pid':None, 'started':time.time(), 'adapter_driver':output(['nvidia-smi', '--query-gpu=name,uuid,driver_version,memory.total', '--format=csv']),
        'cpu':output(['lscpu']), 'before':output(['nvidia-smi', '--query-gpu=temperature.gpu,power.draw,clocks.sm,clocks.mem,utilization.gpu', '--format=csv'])}
    write(receipt_path, receipt)
    print(f'{phase} trial {trial}: {scene} {schedule} ({binary_name})', flush=True)
    with stem.with_suffix('.log').open('w') as log:
        process = subprocess.Popen(command, cwd=ROOT, env=env, stdout=log, stderr=log)
        receipt['pid'] = process.pid
        write(receipt_path, receipt)
        result = process.wait(timeout=600)
    receipt.update(exit=result, finished=time.time(), status='failed' if result else 'ok')
    write(receipt_path, receipt)
    assert result == 0, f'failed run retained: {stem}'
    data = json.loads(stem.with_suffix('.json').read_text())
    samples = data['raw_runs'][0]['completed_step_ms']
    assert len(samples)==240 and data['sub_steps']==4 and not data['sleep']
    assert data['warmup_steps']==90 and data['raw_runs'][0]['physics_step']==330
    assert data['bodies']==count+(1 if scene=='falling-cubes' else 2)
    assert data['adapter']=='NVIDIA GeForce RTX 4070 SUPER'
    receipt.update(mean_ms=statistics.mean(samples), p95_ms=data['completed_step']['p95_ms'],
        raw_sha256=digest(stem.with_suffix('.json')), log_sha256=digest(stem.with_suffix('.log')))
    receipt['after']=output(['nvidia-smi', '--query-gpu=temperature.gpu,power.draw,clocks.sm,clocks.mem,utilization.gpu', '--format=csv'])
    write(receipt_path, receipt)
    print(f"  {receipt['mean_ms']:.4f} ms; p95 {receipt['p95_ms']:.4f} ms", flush=True)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('raw', type=Path)
    parser.add_argument('phase', choices=['freeze','baseline','diagnostic','pilot','confirmation'])
    parser.add_argument('--name', default='baseline')
    args=parser.parse_args()
    raw=args.raw.resolve(); raw.mkdir(parents=True, exist_ok=True)
    protocol={'fixtures':FIXTURES, 'baseline_runs':27, 'diagnostic_limit':6,
        'candidate_limit':2, 'pilots_per_candidate':2, 'confirmation_runs':18,
        'warmup':90, 'timed':240, 'substeps':4, 'dt':'1/60', 'sleep':False,
        'backend':'native Vulkan', 'adapter':'nvidia', 'color_prefix':20,
        'completed_only':True, 'default_policy_change':False}
    if (raw/'protocol.json').exists():
        assert json.loads((raw/'protocol.json').read_text())==json.loads(json.dumps(protocol))
    else: write(raw/'protocol.json', protocol)
    if args.phase=='freeze': return freeze(raw, args.name)
    if args.phase=='pilot':
        assert args.name in ['candidate1','candidate2']
        for trial in [1,2]: run(raw, 'pilot-'+args.name, *FIXTURES[2], 'candidate', args.name, trial)
        return
    if args.phase=='diagnostic':
        # Exactly six: all mixed controls plus explicit component on existing fixtures,
        # and one mixed candidate run after its pilots if justified.
        for scene,count,schedule in [(s,c,'component') for s,c in FIXTURES[:2]] + [(*FIXTURES[2],s) for s in ['global','component','auto']]:
            run(raw,'diagnostic',scene,count,schedule,args.name,1,True)
        return
    for trial in [1,2,3]:
        for scene,count in FIXTURES:
            schedules=['global','component','auto'] if args.phase=='baseline' else ['auto','candidate']
            if trial%2==0: schedules.reverse()
            for schedule in schedules:
                run(raw,args.phase,scene,count,schedule,args.name if schedule=='candidate' else 'baseline',trial)

if __name__=='__main__': main()
