#!/usr/bin/env python3
"""Matched falling-cubes v1 sweep. Build binaries first; runs are sequential.

Keep the raw output directory for audit/replotting on another machine. The caller
selects Vulkan/GL drivers as usual; --adapter selects the physics wgpu adapter.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import runpy
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
NATIVE = runpy.run_path(str(ROOT/'scripts/measure-samples-application.py'))['NATIVE']
COUNTS = [5,10,50,100,200,400,800,1000,2000,5000,10000,15000,20000,30000,40000,50000,60000]
MODES = ['physics-cpu','physics-gpu','sokol-cpu','sokol-gpu','direct-cpu','direct-gpu']

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
    if mode == 'physics-cpu':
        d=data['scenes']['falling-cubes']
        assert d['bodies']==count+1 and d['workers']==args.workers, d
        samples=d['wall_samples_ms']
        assert len(samples)==args.timed and all(x>0 for x in samples)
        return dict(p50_ms=d['wall_p50_ms'],p95_ms=d['wall_p95_ms'],mean_ms=statistics.mean(samples))
    if mode == 'physics-gpu':
        assert data['bodies']==count+1 and not data['sleep'],data
        assert data['raw_runs'][0]['live_contacts']>0, data
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
    p.add_argument('--trials',type=int,default=3)
    p.add_argument('--warmup',type=int,default=90)
    p.add_argument('--timed',type=int,default=240)
    p.add_argument('--workers',type=int,default=8)
    p.add_argument('--adapter',default='nvidia')
    p.add_argument('--width',type=int,default=1280)
    p.add_argument('--height',type=int,default=720)
    p.add_argument('--timeout',type=int,default=600)
    p.add_argument('--require-idle',action='store_true',help='Stop before a trial if background CPU >15%% or NVIDIA GPU >10%%; resume later')
    p.add_argument('--resume',action='store_true',help='Resume an interrupted directory with identical binaries and measurement settings')
    a=p.parse_args()
    if not(0<a.trials and 0<a.warmup and 0<a.timed<=4096 and 0<a.workers<=64 and a.counts==sorted(set(a.counts)) and min(a.counts)>0 and max(a.counts)<=1000000):
        p.error('positive trials/window/workers, increasing unique counts <=1000000 required')
    out=a.output.resolve()
    if a.resume:
        if not (out/'manifest.json').exists(): p.error('--resume requires an existing manifest')
    else: out.mkdir(parents=True,exist_ok=False)
    binaries=dict(cpu=ROOT/'oracle/build/box3d_oracle',gpu=ROOT/'target/native-cache-build/release/gpu-physics',
        sokol_cpu=ROOT/'native-samples/build-cpu-portable/bin/samples_cpu',
        sokol_gpu=ROOT/'native-samples/build-gpu-native-cache-portable/bin/samples_gpu',
        cpu_bridge=ROOT/'oracle/build-viewer/libbox3d_viewer_cpu.so')
    env={k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_','GPU_SOKOL_','GPU_BENCH_'))}
    env.pop('WAYLAND_DISPLAY',None)
    env.update(NATIVE, GPU_PHYSICS_ADAPTER=a.adapter,GPU_PHYSICS_CPU_WORKERS=str(a.workers),
        GPU_PHYSICS_DEMAND_POSES='1',GPU_PHYSICS_PRESENT_MODE='immediate',
        GPU_PHYSICS_PIPELINE_CACHE_DIR=str(Path.home()/'.cache/box3d-gpu-physics/pipelines'),
        GPU_BENCH_WIDTH=str(a.width),GPU_BENCH_HEIGHT=str(a.height))
    manifest=dict(workload='falling-cubes-v1',arguments=vars(a)|dict(output=str(out)),platform=platform.platform(),
        cpu=command_output(['lscpu']),gpu=command_output(['nvidia-smi','--query-gpu=name,uuid,memory.total,driver_version,power.limit,clocks.max.sm,clocks.max.memory','--format=csv']),
        vulkan=command_output(['vulkaninfo','--summary']),
        git=command_output(['git','rev-parse','HEAD']),diff_sha256=hashlib.sha256(subprocess.check_output(['git','diff'])).hexdigest(),
        binaries={k:dict(path=str(v),sha256=hashlib.sha256(v.read_bytes()).hexdigest()) for k,v in binaries.items()},
        environment={k:v for k,v in env.items() if k.startswith(('GPU_','VK_','WGPU_','__NV','__GLX','DISPLAY'))})
    if a.resume:
        old=json.loads((out/'manifest.json').read_text())
        for key in ['counts','modes','trials','warmup','timed','workers','adapter','width','height']:
            if old['arguments'].get(key)!=manifest['arguments'].get(key): p.error('resume setting changed: '+key)
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
    for count in a.counts:
        # The direct CPU viewer still creates a GPU-side world for its renderer.
        # That world has 65,536 body slots, including the static floor. The
        # independent C oracle and CPU Sokol app do not share this limit.
        for mode in a.modes:
            if mode not in stopped and count>65535 and mode not in ['physics-cpu','sokol-cpu']:
                stopped[mode]=dict(count=count,reason='16-bit scene body capacity',max_dynamic_cubes=65535)
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
                common=['--scene','falling-cubes','--bodies',str(count),'--warmup',str(a.warmup),'--frames',str(a.timed),'--no-sleep']
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
                try:
                    with path.with_suffix('.log').open('w') as log:
                        subprocess.run(cmd,cwd=cwd,env=runenv,stdout=log,stderr=log,check=True,timeout=a.timeout)
                    record['after']=system()
                    if record['after']['ac'] and '1' not in record['after']['ac'].values(): raise RuntimeError('AC power disconnected')
                    record.update(metrics(json.loads(path.read_text()),mode,path,a,count),status='ok')
                    if 'framebuffer' in record:
                        if framebuffer is None: framebuffer=record['framebuffer']
                        assert framebuffer==record['framebuffer'],(framebuffer,record['framebuffer'])
                    grouped[mode].append(record)
                    print(f"  {1000/record['mean_ms']:.1f} FPS; p50/p95 {record['p50_ms']:.2f}/{record['p95_ms']:.2f} ms",flush=True)
                except (subprocess.SubprocessError,AssertionError,KeyError,ValueError) as e:
                    record.update(status='invalid',error=str(e)[-1500:]);stopped[mode]=dict(count=count,reason='invalid; inspect raw log',error=record['error'])
                    print('  invalid:',str(e)[-300:],flush=True)
                rows.append(record)
                write_json(out/'trials.json',rows)
                write_json(out/'stopped.json',stopped)
        for mode,records in grouped.items():
            if mode not in stopped and records and statistics.median(r['mean_ms'] for r in records)>=100:
                stopped[mode]=dict(count=count,reason='10 FPS threshold')
        write_json(out/'stopped.json',stopped)
    print('Done:',out,flush=True)

if __name__=='__main__': main()
