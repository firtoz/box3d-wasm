#!/usr/bin/env python3
"""Matched falling-cubes v1 sweep. Build binaries first; runs are sequential.

Keep the raw output directory for audit/replotting on another machine. The caller
selects Vulkan/GL drivers as usual; --adapter selects the physics wgpu adapter.
"""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import runpy
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
NATIVE = runpy.run_path(str(ROOT/'scripts/measure-samples-application.py'))['NATIVE']
COUNTS = [5,10,50,100,200,400,800,1000,2000,5000,10000,15000,20000,30000,40000,50000,60000,80000,100000,150000,200000,400000,800000,1000000]
MODES = ['physics-cpu','physics-gpu','sokol-cpu','sokol-gpu','direct-cpu','direct-gpu']

def sokol_settings():
    path=ROOT.parents[1]/'box3d/settings.ini'
    return json.loads(path.read_text()) if path.exists() else None

def render_settings(settings):
    if settings is None: return None
    keys=['drawShapes','enableShadows','enableGtao','gtaoQuality','enableIbl',
          'exposure','sunStrength','shadowSplitLambda','debugView','showHullEdges','showEdgeConvexity']
    return {key:settings.get(key) for key in keys}

def write_json(path, data):
    temporary=path.with_suffix(path.suffix+'.tmp')
    temporary.write_text(json.dumps(data,indent=2)+'\n')
    temporary.replace(path)

def command_output(args):
    try:
        return subprocess.run(args, capture_output=True, text=True, timeout=15).stdout.strip()
    except (OSError, subprocess.TimeoutExpired):
        return 'unavailable'

def system():
    return dict(time=time.time(), load=os.getloadavg(), ac={str(p):p.read_text().strip()
        for p in Path('/sys/class/power_supply').glob('*/online')},
        gpu=command_output(['nvidia-smi','--query-gpu=name,temperature.gpu,power.draw,clocks.sm,clocks.mem,utilization.gpu','--format=csv']),
        gpu_power_limits=command_output(['nvidia-smi','-q','-d','POWER,PERFORMANCE']))

def idle_check(seconds=2):
    """Confirm a busy short reading over ten seconds before rejecting a trial.

    A quiet short reading passes immediately. CPU load is averaged from kernel
    counters over the entire confirmation window, not the maximum short spike.
    GPU samples retain the 10% ceiling. No applications or power settings change.
    """
    def cpu():
        values=list(map(int,Path('/proc/stat').read_text().splitlines()[0].split()[1:9]))
        return sum(values),values[3]+values[4]
    start=cpu();samples=[];elapsed=0
    while True:
        duration=seconds if elapsed==0 else min(seconds,10-elapsed)
        time.sleep(duration);elapsed+=duration;end=cpu()
        busy=1-(end[1]-start[1])/max(1,end[0]-start[0])
        raw=command_output(['nvidia-smi','--query-gpu=utilization.gpu','--format=csv,noheader,nounits'])
        utilization=[float(x) for x in raw.splitlines() if x.strip().isdigit()]
        samples.append(max(utilization,default=0))
        quiet=busy<=0.15 and max(samples)<=10
        if (len(samples)==1 and quiet) or elapsed>=10:
            return dict(cpu_busy=busy,gpu_busy=max(samples),quiet=quiet,
                        observed_seconds=elapsed,policy='confirm-busy-10s',gpu_samples=samples)


def metrics(data, mode, path, args, count):
    scene=getattr(args,'scene','falling-cubes')
    expected_bodies=count+(2 if scene=='mixed-stacks' else 1)
    if mode == 'physics-cpu':
        d=data['scenes'][scene]
        assert d['bodies']==expected_bodies and d['workers']==args.workers, d
        samples=d['wall_samples_ms']
        assert len(samples)==args.timed and all(x>0 for x in samples)
        return dict(p50_ms=d['wall_p50_ms'],p95_ms=d['wall_p95_ms'],mean_ms=statistics.mean(samples))
    if mode == 'physics-gpu':
        assert data['bodies']==expected_bodies and not data['sleep'],data
        assert data['raw_runs'][0]['live_contacts']>0, data
        if getattr(args,'profile_broadphase',False):
            profile=data['raw_runs'][0]['broadphase_profile']
            assert len(profile['stages'])==7 and len(profile['samples_ms'])==args.timed
            assert all(len(row)==7 and all(math.isfinite(v) and v>=0 for v in row) for row in profile['samples_ms'])
        samples=data['raw_runs'][0]['completed_step_ms']
        assert len(samples)==args.timed and data['raw_runs'][0]['physics_step']==args.warmup+args.timed
        assert all(x>0 for x in samples)
        return data['completed_step'] | dict(mean_ms=statistics.mean(samples),adapter=data['adapter'],contacts=data['raw_runs'][0]['live_contacts'])
    if mode.startswith('sokol'):
        assert data['status']=='ok' and data['measured']==args.timed and data['swap_interval']==0, data
        assert not data['enable_sleep'] and data['worker_count']==args.workers
        assert data['last_completed_step']==args.warmup+args.timed and data['last_rendered_pose']==data['last_completed_step']
        assert not data['gpu_fail']
        frames=data['frames']
        assert len(frames)==args.timed
        assert all(not f['exploded'] and f['nan_count']==0 for f in frames)
        assert all(f['body_count']==count+1 and not f['gpu_contact_metrics']['capacity_loss'] for f in frames)
        if mode=='sokol-gpu' and any('gpu_draw_shapes' in f for f in frames):
            assert all(f.get('gpu_draw_shapes')==count+1 for f in frames), 'incomplete GPU shape draw list'
        if any('renderer_instances' in f for f in frames):
            assert all(f.get('renderer_instances')==count+1 for f in frames), 'incomplete renderer instance upload'
        if mode=='sokol-gpu': assert any(f['gpu_contact_metrics']['known'] and f['gpu_contact_metrics']['touching_roots']>0 for f in frames)
        assert len({(f['framebuffer_width'],f['framebuffer_height']) for f in frames})==1
        return dict(p50_ms=data['cadence_p50_ms'],p95_ms=data['cadence_p95_ms'],mean_ms=statistics.mean(f['cadence_ms'] for f in frames),
            framebuffer=[frames[0]['framebuffer_width'],frames[0]['framebuffer_height']],physics_ms=data['physics_p50_ms'])
    assert data['timed']==args.timed and not data['sleep'] and data['physics_step']==args.warmup+args.timed, data
    cadence=json.loads(path.with_suffix('.cadence.json').read_text())
    assert cadence['samples']==args.timed
    log=path.with_suffix('.log').read_text()
    assert 'GPU presentation mode: Immediate' in log
    import re
    sizes=re.findall(r'matched-window-(?:start|end): (\d+)x(\d+)',log)
    assert len(sizes)==2 and sizes[0]==sizes[1], sizes
    return dict(p50_ms=cadence['p50_ms'],p95_ms=cadence['p95_ms'],mean_ms=cadence['sum_ms']/args.timed,
        framebuffer=list(map(int,sizes[0])),physics_submit_ms=data['physics_submit']['p50_ms'])

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('output',type=Path)
    p.add_argument('--counts',type=int,nargs='+',default=COUNTS)
    p.add_argument('--modes',nargs='+',choices=MODES,default=MODES)
    p.add_argument('--selected-binaries-only',action='store_true',help='Require/hash only binaries needed by selected modes')
    p.add_argument('--min-rate',type=float,default=10,help='Stop a path at this steps/s or FPS; 0 tests every requested count')
    p.add_argument('--backend',choices=['native','ordinary'],default='native')
    p.add_argument('--trials',type=int,default=3)
    p.add_argument('--warmup',type=int,default=90)
    p.add_argument('--timed',type=int,default=240)
    p.add_argument('--workers',type=int,default=8)
    p.add_argument('--adapter',default='nvidia')
    p.add_argument('--scene',choices=['falling-cubes','mixed-stacks'],default='falling-cubes',help='Mixed stacks: independent two-box groups, physics modes only')
    p.add_argument('--gpu-color-prefix',choices=['auto',*[str(i) for i in range(24)]],default='20')
    p.add_argument('--global-replay',choices=['0','1'],default=None,help='Override global replay; omission preserves binary default')
    p.add_argument('--gpu-solver',choices=['component','global'],default='component',
        help='Select comparable component or global-color scheduling; recorded in the manifest')
    p.add_argument('--gpu-binary',type=Path,help='Use a preserved GPU executable for physics/direct modes')
    p.add_argument('--sokol-gpu-binary',type=Path,help='Use a preserved GPU executable for Sokol mode')
    p.add_argument('--width',type=int,default=1280)
    p.add_argument('--height',type=int,default=720)
    p.add_argument('--timeout',type=int,default=600)
    p.add_argument('--profile-broadphase',action='store_true',help='Diagnostic timestamps inside native cached broadphase; physics-gpu only, counts >=1000')
    p.add_argument('--require-idle',action='store_true',help='Stop before a trial if background CPU >15%% or NVIDIA GPU >10%%; resume later')
    p.add_argument('--resume',action='store_true',help='Resume an interrupted directory with identical binaries and measurement settings')
    a=p.parse_args()
    if not(0<a.trials and 0<a.warmup and 0<a.timed<=4096 and 0<a.workers<=64 and a.counts==sorted(set(a.counts)) and min(a.counts)>0 and max(a.counts)<=1000000):
        p.error('positive trials/window/workers, increasing unique counts <=1000000 required')
    if not math.isfinite(a.min_rate) or a.min_rate < 0: p.error('--min-rate must be finite and nonnegative')
    if a.profile_broadphase and (a.modes!=['physics-gpu'] or min(a.counts)<1000):
        p.error('--profile-broadphase requires physics-gpu only and counts >=1000')
    if a.scene=='mixed-stacks' and (any(not m.startswith('physics-') for m in a.modes) or min(a.counts)<2):
        p.error('mixed-stacks requires physics modes and counts >=2')
    out=a.output.resolve()
    if a.resume:
        if not (out/'manifest.json').exists(): p.error('--resume requires an existing manifest')
    else: out.mkdir(parents=True,exist_ok=False)
    binaries=dict(cpu=ROOT/'oracle/build/box3d_oracle',gpu=ROOT/'target/native-cache-build/release/gpu-physics',
        sokol_cpu=ROOT/'native-samples/build-cpu-portable/bin/samples_cpu',
        sokol_gpu=ROOT/'native-samples/build-gpu-native-cache-portable/bin/samples_gpu',
        cpu_bridge=ROOT/'oracle/build-viewer/libbox3d_viewer_cpu.so')
    if a.gpu_binary is not None: binaries['gpu']=a.gpu_binary.resolve()
    if a.sokol_gpu_binary is not None: binaries['sokol_gpu']=a.sokol_gpu_binary.resolve()
    env={k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_','GPU_SOKOL_','GPU_BENCH_'))}
    env.pop('WAYLAND_DISPLAY',None)
    env.update(NATIVE if a.backend=='native' else {}, WINIT_X11_SCALE_FACTOR='1', GPU_PHYSICS_ADAPTER=a.adapter,GPU_PHYSICS_CPU_WORKERS=str(a.workers),
        GPU_PHYSICS_DEMAND_POSES='1',GPU_PHYSICS_PRESENT_MODE='immediate',
        GPU_PHYSICS_PIPELINE_CACHE_DIR=str(Path.home()/'.cache/box3d-gpu-physics/pipelines'),
        GPU_BENCH_WIDTH=str(a.width),GPU_BENCH_HEIGHT=str(a.height))
    env['GPU_PHYSICS_COMPONENT_TGS']='1' if a.gpu_solver=='component' else '0'
    if a.gpu_solver=='global': env.update(GPU_PHYSICS_COMPONENT_TGS='0',GPU_PHYSICS_COLOR_PREFIX=a.gpu_color_prefix)
    if a.global_replay is not None: env['GPU_PHYSICS_GLOBAL_REPLAY']=a.global_replay
    if a.profile_broadphase: env['GPU_PHYSICS_PROFILE_BROADPHASE']='1'
    needed=set()
    for mode in a.modes:
        needed.update({'physics-cpu':['cpu'],'physics-gpu':['gpu'],'sokol-cpu':['sokol_cpu'],'sokol-gpu':['sokol_gpu'],'direct-cpu':['gpu','cpu_bridge'],'direct-gpu':['gpu']}[mode])
    if a.selected_binaries_only:
        binaries={k:v for k,v in binaries.items() if k in needed}
    manifest=dict(workload=a.scene+'-v1',arguments=vars(a)|dict(output=str(out),gpu_binary=str(a.gpu_binary) if a.gpu_binary else None,sokol_gpu_binary=str(a.sokol_gpu_binary) if a.sokol_gpu_binary else None),platform=platform.platform(),
        cpu=command_output(['lscpu']),gpu=command_output(['nvidia-smi','--query-gpu=name,uuid,memory.total,driver_version,power.limit,clocks.max.sm,clocks.max.memory','--format=csv']),
        vulkan=command_output(['vulkaninfo','--summary']),
        git=command_output(['git','rev-parse','HEAD']),diff_sha256=hashlib.sha256(subprocess.check_output(['git','diff'])).hexdigest(),
        binaries={k:dict(path=str(v),sha256=hashlib.sha256(v.read_bytes()).hexdigest()) for k,v in binaries.items()},
        environment={k:v for k,v in env.items() if k.startswith(('GPU_','VK_','WGPU_','__NV','__GLX','DISPLAY'))})
    manifest['sokol_settings_before']=sokol_settings()
    manifest['sokol_draw_distance_m']=1000
    if a.resume:
        old=json.loads((out/'manifest.json').read_text())
        for key in ['counts','modes','trials','warmup','timed','workers','adapter','width','height','scene','gpu_color_prefix','global_replay','gpu_solver','backend','min_rate']:
            if old['arguments'].get(key,{'scene':'falling-cubes','gpu_color_prefix':'20','gpu_solver':'component','backend':'native','min_rate':10}.get(key))!=manifest['arguments'].get(key): p.error('resume setting changed: '+key)
        if old['binaries']!=manifest['binaries'] or old['environment']!=manifest['environment']:
            p.error('resume requires unchanged binaries and environment; start a new dataset')
        history=json.loads((out/'resume-history.json').read_text()) if (out/'resume-history.json').exists() else []
        history.append(dict(time=time.time(),require_idle=a.require_idle))
        write_json(out/'resume-history.json',history)
        rows=json.loads((out/'trials.json').read_text()) if (out/'trials.json').exists() else []
        stopped=json.loads((out/'stopped.json').read_text()) if (out/'stopped.json').exists() else {}
    else:
        (out/'manifest.json').write_text(json.dumps(manifest,indent=2))
        rows=[];stopped={}
    # Recover terminal failures even if interrupted before stopped.json was saved.
    for row in rows:
        if row['status']=='invalid':
            stopped[row['mode']]=dict(count=row['count'],reason='invalid; inspect raw log',error=row.get('error',''))
    sizes={tuple(r['framebuffer']) for r in rows if r['status']=='ok' and 'framebuffer' in r}
    assert len(sizes)<=1
    framebuffer=list(next(iter(sizes))) if sizes else None
    renderer_settings=render_settings(manifest['sokol_settings_before'])
    for row in rows:
        if row['status']=='ok' and row['mode'].startswith('sokol-'):
            renderer_settings=render_settings(row.get('sokol_settings_after'))
            break
    for count in a.counts:
        write_json(out/'stopped.json',stopped)
        active=[m for m in a.modes if m not in stopped]
        if not active: break
        grouped={m:[r for r in rows if r['count']==count and r['mode']==m and r['status']=='ok'] for m in active}
        for trial in range(a.trials):
            for mode in (active if trial%2==0 else active[::-1]):
                if mode in stopped: continue
                if any(r['count']==count and r['mode']==mode and r['trial']==trial+1 for r in rows): continue
                idle=None
                if a.require_idle:
                    idle=idle_check()
                    if not idle['quiet']:
                        (out/'idle-wait.json').write_text(json.dumps(dict(time=time.time(),count=count,mode=mode,trial=trial+1,**idle),indent=2))
                        raise SystemExit('Background load is active; retained results. Resume with --resume when quiet.')
                path=out/f'{count}-{mode}-{trial+1}.json'
                before=system()
                if before['ac'] and '1' not in before['ac'].values(): raise RuntimeError('AC power disconnected')
                runenv=env|dict(GPU_BENCH_CUBES=str(count))
                common=['--scene',a.scene,'--bodies',str(count),'--warmup',str(a.warmup),'--frames',str(a.timed),'--no-sleep']
                cwd=ROOT
                if mode=='physics-cpu': cmd=[str(binaries['cpu']),*common,'--workers',str(a.workers),'--metrics',str(path)]
                elif mode=='physics-gpu': cmd=[str(binaries['gpu']),*common,'--bench',str(path),'--metric-runs','1']
                elif mode.startswith('sokol'):
                    cmd=[str(binaries['sokol_cpu' if mode.endswith('cpu') else 'sokol_gpu']),'--sample-name','GPU Bench/Falling Cubes','--bench-json',str(path),
                         '--warmup',str(a.warmup),'--timed',str(a.timed),'--unpaced','--no-sleep','--workers',str(a.workers)]
                    cwd=ROOT.parents[1]/'box3d'
                else:
                    cmd=[str(binaries['gpu']),*common,'--native-timeline',str(path)]
                    if mode.endswith('cpu'): runenv['GPU_PHYSICS_CPU_REFERENCE']=str(binaries['cpu_bridge'])
                print(f'{count} cubes {mode} trial {trial+1}',flush=True)
                record=dict(count=count,mode=mode,trial=trial+1,command=cmd,before=before,idle=idle,load_gate_enabled=a.require_idle)
                if mode.startswith('sokol-'): record['sokol_settings_before']=sokol_settings()
                try:
                    with path.with_suffix('.log').open('w') as log:
                        subprocess.run(cmd,cwd=cwd,env=runenv,stdout=log,stderr=log,check=True,timeout=a.timeout)
                    record['after']=system()
                    if mode.startswith('sokol-'): record['sokol_settings_after']=sokol_settings()
                    if record['after']['ac'] and '1' not in record['after']['ac'].values(): raise RuntimeError('AC power disconnected')
                    data=json.loads(path.read_text())
                    if mode=='sokol-gpu':
                        observed={f.get('gpu_draw_shapes') for f in data['frames']}
                        assert observed=={count+1}, f'GPU draw list expected {count+1} shapes; observed {observed}'
                    if mode.startswith('sokol-'):
                        observed={f.get('renderer_instances') for f in data['frames']}
                        assert observed=={count+1}, f'renderer upload expected {count+1} instances; observed {observed}'
                        current_settings=render_settings(record['sokol_settings_after'])
                        if renderer_settings is None: renderer_settings=current_settings
                        assert current_settings==renderer_settings, 'Sokol rendering settings changed during sweep'
                        assert record['sokol_settings_after']['drawDistance']==1000, 'benchmark draw distance changed'
                    record.update(metrics(data,mode,path,a,count),status='ok')
                    if 'framebuffer' in record:
                        assert record['framebuffer']==[a.width,a.height], 'actual framebuffer differs from requested dimensions'
                        if framebuffer is None: framebuffer=record['framebuffer']
                        assert framebuffer==record['framebuffer'],(framebuffer,record['framebuffer'])
                    grouped[mode].append(record)
                    print(f"  {1000/record['mean_ms']:.1f} {'steps/s' if mode.startswith('physics-') else 'FPS'}; p50/p95 {record['p50_ms']:.2f}/{record['p95_ms']:.2f} ms",flush=True)
                except (subprocess.SubprocessError,AssertionError,KeyError,ValueError) as e:
                    record.update(status='invalid',error=str(e)[-1500:]);stopped[mode]=dict(count=count,reason='invalid; inspect raw log',error=record['error'])
                    print('  invalid:',str(e)[-300:],flush=True)
                rows.append(record)
                write_json(out/'trials.json',rows)
                write_json(out/'stopped.json',stopped)
        for mode,records in grouped.items():
            if a.min_rate > 0 and mode not in stopped and records and statistics.median(r['mean_ms'] for r in records)>=1000/a.min_rate:
                stopped[mode]=dict(count=count,reason=f'{a.min_rate:g}/s threshold')
        write_json(out/'stopped.json',stopped)
    print('Done:',out,flush=True)

if __name__=='__main__': main()
