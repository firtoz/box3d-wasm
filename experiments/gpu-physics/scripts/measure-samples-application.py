#!/usr/bin/env python3
"""Interleaved real Sokol application trials; build/freeze binaries before running."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
NATIVE = dict(WGPU_BACKEND='vulkan', GPU_PHYSICS_SPIRV_LAYOUT_RUST='1',
    GPU_PHYSICS_NATIVE_RADIX_CACHE='2', GPU_PHYSICS_NATIVE_CONTACT_CACHE='1',
    GPU_PHYSICS_NATIVE_GRAPH_CACHE='1', GPU_PHYSICS_NATIVE_TAIL_CACHE='1',
    GPU_PHYSICS_NATIVE_PAIR_CACHE='1', GPU_PHYSICS_NATIVE_RESET_CACHE='1',
    GPU_PHYSICS_SAMPLES_DEMAND_POSES='0', GPU_PHYSICS_FULL_REPLAY='1', GPU_PHYSICS_GRAPH_MEMO='1',
    GPU_PHYSICS_PAIR_MATRIX='1', GPU_PHYSICS_COMPONENT_TGS='1',
    GPU_PHYSICS_GPU_CCD='1', GPU_PHYSICS_RESIDENT='1', GPU_PHYSICS_GRAPH_SHARED='1',
    GPU_PHYSICS_SMALL_COMPONENT_WG='16', GPU_PHYSICS_AB='bounded-static-sort')

def power():
    return {str(p): p.read_text().strip() for p in Path('/sys/class/power_supply').glob('*/online')
            if p.with_name('type').read_text().strip() == 'Mains'}

def snapshot():
    gpu = subprocess.run(['nvidia-smi', '--query-gpu=temperature.gpu,power.draw,clocks.sm,utilization.gpu',
                          '--format=csv'], capture_output=True, text=True)
    return dict(time=time.time(), power=power(), load=os.getloadavg(), gpu=gpu.stdout)

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('output', type=Path)
    for flag in ['baseline-both', 'candidate-both', 'candidate-gpu', 'cpu']:
        p.add_argument('--'+flag, type=Path)
    p.add_argument('--variants', nargs='+', choices=['baseline-both','candidate-both','candidate-gpu','cpu','native-control'])
    p.add_argument('--baseline-native', action='store_true')
    p.add_argument('--demand-poses', choices=['0','1'], default='0')
    p.add_argument('--staging-only', action='store_true', help='Only alternate demand staging and its same-binary control')
    p.add_argument('--native-control', type=Path, help='Same candidate binary with automatic pose staging retained')
    p.add_argument('--trials', type=int, default=5)
    p.add_argument('--warmup', type=int, default=200)
    p.add_argument('--timed', type=int, default=1000)
    args = p.parse_args()
    if args.trials < 1 or args.warmup < 0 or args.timed < 1:
        p.error('trials/timed must be positive and warmup nonnegative')
    out = args.output.resolve(); out.mkdir(parents=True, exist_ok=False)
    variants = {'baseline-both': (args.baseline_both, args.baseline_native),
                'candidate-both': (args.candidate_both, True),
                'candidate-gpu': (args.candidate_gpu, True), 'cpu': (args.cpu, False)}
    if args.native_control:
        variants['native-control'] = (args.native_control, True)
    if args.staging_only:
        if not args.native_control or args.demand_poses != '1':
            p.error('--staging-only requires --native-control and --demand-poses 1')
        variants = {k:v for k,v in variants.items() if k in ('candidate-both', 'native-control')}
    if args.variants:
        if any(name not in variants for name in args.variants):
            p.error('selected control requires --native-control')
        variants = {k:v for k,v in variants.items() if k in args.variants}
    for name, (binary, _) in variants.items():
        if binary is None:
            p.error('missing --'+name)
    env = {k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_', 'GPU_SOKOL_'))}
    env.update(__NV_PRIME_RENDER_OFFLOAD='1', __GLX_VENDOR_LIBRARY_NAME='nvidia',
               VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json',
               GPU_PHYSICS_PIPELINE_CACHE_DIR=str(Path.home()/'.cache/box3d-gpu-physics/pipelines'))
    manifest = {name: dict(binary=str(binary.resolve()), sha256=hashlib.sha256(binary.read_bytes()).hexdigest(),
                          environment={k:v for k,v in (env | (NATIVE if native else {})).items()
                                       if k.startswith(("GPU_PHYSICS_", "WGPU_", "VK_", "__NV_", "__GLX_"))})
                for name,(binary,native) in variants.items()}
    for name in ('candidate-both', 'candidate-gpu'):
        if name in manifest:
            manifest[name]['environment']['GPU_PHYSICS_SAMPLES_DEMAND_POSES'] = args.demand_poses
    if 'native-control' in manifest:
        manifest['native-control']['environment']['GPU_PHYSICS_SAMPLES_DEMAND_POSES'] = '0'
    (out/'manifest.json').write_text(json.dumps(manifest, indent=2))
    results=[]
    framebuffer=None
    for scene in ['Dominoes', 'GPU Bench/Mixed Stacks 4096']:
        for trial in range(args.trials):
            order=list(variants)
            if trial % 2: order.reverse()
            for name in order:
                before=snapshot()
                if '1' not in before['power'].values():
                    raise SystemExit('AC power unavailable; retaining completed trials, deferring performance run')
                stem=f'{scene.split("/")[-1].replace(" ", "-").lower()}-{name}-{trial+1}'
                cmd=[manifest[name]['binary'], '--sample-name', scene, '--bench-json', str(out/(stem+'.json')),
                     '--warmup', str(args.warmup), '--timed', str(args.timed), '--unpaced', '--no-sleep', '--workers', '8']
                print(stem, flush=True)
                with (out/(stem+'.log')).open('w') as log:
                    subprocess.run(cmd, cwd=ROOT.parents[1]/'box3d', env=env | manifest[name]['environment'],
                                   stdout=log, stderr=log, check=True, timeout=600)
                after=snapshot()
                (out/(stem+'.system.json')).write_text(json.dumps(dict(before=before, after=after),indent=2))
                if '1' not in after['power'].values():
                    raise SystemExit('AC power lost during trial; retaining raw result without qualification')
                data=json.loads((out/(stem+'.json')).read_text())
                assert data['status']=='ok' and data['measured']==args.timed and data['swap_interval']==0, data
                assert data['last_submitted_step']==data['last_completed_step']==args.warmup+args.timed, data
                assert data['last_rendered_pose']==data['last_completed_step'], data
                expected_mode = 'cpu' if name == 'cpu' else ('gpu' if name == 'candidate-gpu' else 'both')
                assert data['mode'] == expected_mode and not data['gpu_fail'], data
                for i,frame in enumerate(data['frames']):
                    size = (frame['framebuffer_width'], frame['framebuffer_height'])
                    if framebuffer is None:
                        framebuffer = size
                    assert size == framebuffer and min(size) > 0, (size, framebuffer, stem)
                    assert not frame['gpu_contact_metrics']['capacity_loss'], frame
                    expected = args.warmup + i + 1
                    assert frame['submitted_step'] == frame['completed_step'] == frame['rendered_pose'] == expected, frame
                    assert frame['body_count'] == (5431 if scene == 'Dominoes' else 4098), frame
                row=dict(scene=scene, variant=name, trial=trial+1, framebuffer=framebuffer, before=before, after=after,
                         summary={k:v for k,v in data.items() if k!='frames'})
                results.append(row)
                (out/'trials.json').write_text(json.dumps(results,indent=2))
    summary={}
    for scene in {r['scene'] for r in results}:
        summary[scene]={name:{metric:statistics.median(r['summary'][metric] for r in results
            if r['scene']==scene and r['variant']==name)
            for metric in ['cadence_p50_ms','cadence_p95_ms','physics_p50_ms','pick_p50_ms','draw_p50_ms','render_p50_ms']}
            for name in variants}
    (out/'summary.json').write_text(json.dumps(dict(cpu_win_validated=False, medians_of_trial_percentiles=summary),indent=2))
    print(json.dumps(summary,indent=2))

if __name__=='__main__': main()
