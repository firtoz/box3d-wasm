#!/usr/bin/env python3
"""Publish portable bounded scheduling evidence; keep existing scaling data intact."""
import argparse
import gzip
import hashlib
import json
from pathlib import Path
import re
import statistics

ROOT = Path(__file__).resolve().parents[1]

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('raw',type=Path)
    p.add_argument('output',type=Path)
    a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
    files={str(f.relative_to(a.raw)):f.read_text() for f in sorted(a.raw.rglob('*'))
           if f.is_file() and f.suffix in {'.json','.log','.patch','.py','.original','.rs'}}
    assert 'baseline-build.json' in files and 'candidate-build.json' in files
    rows=[]
    for name,text in files.items():
        if not name.endswith('/trials.json'):continue
        phase=name.split('/')[0]
        for trial in json.loads(text):
            assert trial['status']=='ok',name
            manifest_name=str(Path(name).with_name('manifest.json'))
            manifest=json.loads(files[manifest_name]);args=manifest['arguments']
            scene=args['scene'];schedule=args['gpu_solver'];cpu=trial['mode']=='physics-cpu'
            assert args['warmup']==90 and args['timed']==240 and args['backend']=='native'
            raw_name=str(Path(name).parent/Path(trial['path']).name) if 'path' in trial else None
            # Collector paths may be absolute; find the measured file by its fixed filename.
            raw_name=str(Path(name).parent/f"{trial['count']}-{trial['mode']}-{trial['trial']}.json")
            raw=json.loads(files[raw_name]);samples=(raw['scenes'][scene]['wall_samples_ms'] if cpu else raw['raw_runs'][0]['completed_step_ms'])
            assert len(samples)==240 and abs(statistics.mean(samples)-trial['mean_ms'])<1e-4
            if not cpu:
                assert raw['adapter']=='NVIDIA GeForce RTX 4070 SUPER' or 'NVIDIA' in raw['adapter']
                assert raw['sub_steps']==4 and not raw['sleep']
                hash_expected=json.loads(files['candidate-build.json' if schedule=='auto' else 'baseline-build.json'])['binary_sha256']
                assert manifest['binaries']['gpu']['sha256']==hash_expected
            rows.append({'phase':phase,'scene':scene,'schedule':'cpu' if cpu else schedule,
                         'mean_ms':statistics.mean(samples),'p50_ms':trial['p50_ms'],'p95_ms':trial['p95_ms'],
                         'steps_per_second':1000/statistics.mean(samples),'raw':raw_name,'manifest':manifest_name})
    phases={}
    for name,text in files.items():
        if name.startswith('diagnostic/') and name.endswith('component.log'):
            # The collector repeats a separate pose-mirror pass. It is excluded.
            matched=list(re.finditer(r'component-profile step=(\d+) ms=(\[[^\n]+\])',text))[:240]
            assert [int(m[1]) for m in matched]==list(range(91,331))
            samples=[json.loads(m[2]) for m in matched]
            phases[name]={'mean_ms':[statistics.mean(x) for x in zip(*samples)],'samples_ms':samples,
                          'names':['reset','count','offsets','color_offsets','scatter','small_solve','large_solve'],
                          'topology':[line for line in text.splitlines() if 'component-topology' in line]}
    bundle=json.dumps({'schema':1,'files':files},sort_keys=True).encode()
    packed=gzip.compress(bundle,mtime=0);raw_path=a.output/'raw.json.gz';raw_path.write_bytes(packed)
    summary={'schema':1,'raw_sha256':hashlib.sha256(packed).hexdigest(),'rows':rows,'diagnostic_component_phases':phases,
             'builds':{k:json.loads(files[k+'-build.json']) for k in ['baseline','candidate','diagnostic'] if k+'-build.json' in files}}
    (a.output/'results.json').write_text(json.dumps(summary,indent=2)+'\n')
    print(f'published {len(rows)} trials, {len(files)} raw files')

if __name__=='__main__':main()
