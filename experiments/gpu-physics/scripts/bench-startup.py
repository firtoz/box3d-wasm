#!/usr/bin/env python3
"""Paired startup measurements with isolated persistent caches and retained raw evidence."""
import argparse, fcntl, hashlib, json, math, os, platform, re, runpy, shutil, subprocess, time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def read(path): return json.loads(Path(path).read_text())
def save(path,value):
    temporary=path.with_suffix(path.suffix+'.tmp');temporary.write_text(json.dumps(value,indent=2)+'\n');temporary.replace(path)
def cache_inventory(folder):
    return {str(p.relative_to(folder)):{'bytes':p.stat().st_size,'sha256':digest(p)} for p in sorted(folder.rglob('*')) if p.is_file()}
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('inputs',type=Path,help='JSON with variants before/after, each with direct/sokol binaries, plus environment')
    parser.add_argument('output',type=Path)
    parser.add_argument('--counts',type=int,nargs='+',default=[1000,15000,100000,200000])
    parser.add_argument('--modes',nargs='+',choices=['initialization','direct','sokol'],default=['initialization','direct','sokol'])
    parser.add_argument('--trials',type=int,default=3)
    parser.add_argument('--timeout',type=int,default=900)
    parser.add_argument('--resume',action='store_true')
    args=parser.parse_args();out=args.output.resolve();out.mkdir(parents=True,exist_ok=True)
    lock=(out/'runner.lock').open('a+')
    fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
    source=read(args.inputs); assert set(source['variants'])=={'before','after'}
    identities={label:{kind:{'path':str(Path(path).resolve()),'sha256':digest(path)} for kind,path in binaries.items()} for label,binaries in source['variants'].items()}
    settings={'inputs':source,'binaries':identities,'counts':args.counts,'modes':args.modes,'trials':args.trials,'timeout':args.timeout,
        'cache_definition':'Cold uses a fresh application pipeline cache and private XDG/NVIDIA shader-cache paths. Warm immediately reuses that process-independent directory. Driver internal/kernel caches are not claimed flushed.',
        'time_definition':'Internal phase timers; direct begins before window creation, Sokol begins at OnInit before renderer setup. Process wall time includes teardown; it is not first-frame latency.'}
    manifest_path=out/'manifest.json';rows_path=out/'trials.json'
    if args.resume:
        manifest=read(manifest_path);assert manifest['settings']==settings,'resume settings/binary mismatch'
        rows=read(rows_path)
        for row in rows:
            if row['status']=='running': row.update(status='interrupted',error='Runner restarted after its process lock was released; prior attempt retained.')
            if row['status']=='ok': assert digest(out/row['raw'])==row['raw_sha256'],row['raw']
    else:
        assert not manifest_path.exists(),'output exists; use --resume or a new directory'
        system=runpy.run_path(str(ROOT/'scripts/bench-falling-cubes.py'))['system']
        save(manifest_path,{'settings':settings,'created_at':time.time(),'platform':platform.platform(),'system':system(),
            'head':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()})
        shutil.copy2(__file__,out/'runner.py');rows=[];save(rows_path,rows)
    system=runpy.run_path(str(ROOT/'scripts/bench-falling-cubes.py'))['system']
    env={k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_','GPU_BENCH_'))};env.update(source['environment'])
    env.pop('GPU_PHYSICS_PROFILE_BROADPHASE',None);env.pop('GPU_PHYSICS_CPU_REFERENCE',None)
    # Stable private renderer settings, separate from the user's working directory.
    cwd=out/'settings';cwd.mkdir(exist_ok=True)
    original_settings=source.get('sokol_settings','')
    for count in args.counts:
        for trial in range(1,args.trials+1):
            for mode in args.modes:
                for variant in (['before','after'] if trial%2 else ['after','before']):
                    cache=out/'cache'/f'{count}-{mode}-{variant}-{trial}'
                    for state in ['cold','warm']:
                        key=f'{count}-{mode}-{variant}-{trial}-{state}'
                        if any(r['key']==key and r['status']=='ok' for r in rows):continue
                        # Retain failed attempts and their cache effects; new attempt filenames never overwrite evidence.
                        attempt=1+sum(r['key']==key for r in rows);stem=key+f'-attempt{attempt}'
                        raw=out/(stem+'.json');logpath=out/(stem+'.log')
                        if state=='cold':
                            if cache.exists():cache.rename(cache.with_name(cache.name+f'-failed-cold-{attempt}'))
                            cache.mkdir(parents=True)
                        else:assert cache.exists() and any(cache.rglob('*')),'warm run requires retained cold cache'
                        runenv=env|{'GPU_PHYSICS_PIPELINE_CACHE_DIR':str(cache/'pipelines'),'XDG_CACHE_HOME':str(cache/'xdg'),
                            '__GL_SHADER_DISK_CACHE':'1','__GL_SHADER_DISK_CACHE_PATH':str(cache/'nvidia'), 'GPU_BENCH_CUBES':str(count)}
                        for p in [cache/'pipelines',cache/'xdg',cache/'nvidia']:p.mkdir(parents=True,exist_ok=True)
                        binary=identities[variant]['sokol' if mode=='sokol' else 'direct']['path']
                        if mode=='initialization':
                            runenv['GPU_PHYSICS_STARTUP_BENCH']=str(raw)
                            command=[binary,'--scene','falling-cubes','--bodies',str(count),'--no-sleep']
                        elif mode=='direct':
                            runenv.update(GPU_PHYSICS_STARTUP_REPORT=str(raw),GPU_PHYSICS_STARTUP_EXIT='1')
                            command=[binary,'--scene','falling-cubes','--bodies',str(count),'--no-sleep']
                        else:
                            runenv['GPU_PHYSICS_STARTUP_REPORT']=str(raw)
                            command=[binary,'--sample-name','GPU Bench/Falling Cubes','--bench-json',str(out/(stem+'-frame.json')),
                                '--warmup','0','--timed','1','--unpaced','--no-sleep','--workers','8']
                            (cwd/'settings.ini').write_text(original_settings)
                        row={'key':key,'attempt':attempt,'count':count,'trial':trial,'mode':mode,'variant':variant,'cache':state,
                            'command':command,'environment':{k:v for k,v in runenv.items() if k in source['environment'] or k.startswith(('GPU_','XDG_CACHE','__GL_SHADER'))},
                            'cache_before':cache_inventory(cache),'before':system(),'started_at':time.time(),'raw':raw.name,'log':logpath.name,'status':'running'}
                        rows.append(row);save(rows_path,rows);print(key,flush=True)
                        started=time.monotonic()
                        try:
                            with logpath.open('w') as log:
                                completed=subprocess.run(command,env=runenv,cwd=cwd,stdout=log,stderr=subprocess.STDOUT,timeout=args.timeout)
                            row['exit_code']=completed.returncode;assert completed.returncode==0,'process failed'
                            result=read(raw);assert result['dynamic_cubes']==count
                            for field in ['device_ms','scene_ms','gpu_prepare_ms']:
                                assert math.isfinite(result[field]) and result[field]>=0,(field,result)
                            if mode=='initialization':assert result['bodies']==count+1 and result['first_frame_ms'] is None
                            else:assert result['first_frame_ms']>=result['scene_ms']+result['gpu_prepare_ms']
                            text=logpath.read_text();assert 'GPU pose export:' in text,'missing completed GPU preparation'
                            assert 'panicked' not in text
                            loaded=[int(n) for n in re.findall(r'physics-pipeline-cache: loaded (\d+) bytes',text)]
                            assert loaded and (all(n==0 for n in loaded) if state=='cold' else any(n>0 for n in loaded)), ('unexpected application cache state',loaded)
                            row['pipeline_cache_loaded_bytes']=loaded
                            if mode=='direct':
                                row['framebuffer']=result['framebuffer']
                            if mode=='sokol':
                                assert '0 sokol errors' in text
                                frame=read(out/(stem+'-frame.json'));assert frame['frames_observed']==1 and frame['last_completed_step']==1
                                assert frame['frames'][0]['body_count']==count+1
                                row['framebuffer']=[frame['frames'][0]['framebuffer_width'],frame['frames'][0]['framebuffer_height']]
                            if 'framebuffer' in row:
                                previous=[r['framebuffer'] for r in rows if r['status']=='ok' and r['mode']==mode]
                                assert not previous or row['framebuffer']==previous[0], 'framebuffer changed'
                            row.update(status='ok',result=result,raw_sha256=digest(raw))
                        except Exception as error:
                            row.update(status='failed',error=str(error));raise
                        finally:
                            row.update(process_wall_s=time.monotonic()-started,after=system(),cache_after=cache_inventory(cache),log_sha256=digest(logpath))
                            save(rows_path,rows)
    print('Startup comparison complete:',out,flush=True)
if __name__=='__main__':main()
