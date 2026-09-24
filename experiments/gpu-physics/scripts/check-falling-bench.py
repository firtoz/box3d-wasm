#!/usr/bin/env python3
"""Fault-inject real pilot output to check the publication gates (no GPU needed)."""
import argparse
import copy
import json
from pathlib import Path
import runpy
from types import SimpleNamespace

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('run',type=Path)
a=p.parse_args()
metric=runpy.run_path(str(Path(__file__).with_name('bench-falling-cubes.py')))['metrics']
manifest=json.loads((a.run/'manifest.json').read_text())
args=SimpleNamespace(**manifest['arguments'])
count=args.counts[0]
checks=0
for mode in ['physics-cpu','physics-gpu','sokol-cpu','sokol-gpu','direct-cpu','direct-gpu']:
    path=a.run/f'{count}-{mode}-1.json'
    data=json.loads(path.read_text())
    metric(data,mode,path,args,count)
    def reject(mutate):
        global checks
        bad=copy.deepcopy(data); mutate(bad)
        try: metric(bad,mode,path,args,count)
        except (AssertionError,ValueError): checks+=1
        else: raise AssertionError('accepted corrupt '+mode)
    if mode=='physics-cpu':
        reject(lambda d:d['scenes']['falling-cubes'].update(bodies=count))
        reject(lambda d:d['scenes']['falling-cubes']['wall_samples_ms'].pop())
    elif mode=='physics-gpu':
        reject(lambda d:d['raw_runs'][0].update(live_contacts=0))
        reject(lambda d:d['raw_runs'][0].update(physics_step=0))
    elif mode.startswith('sokol'):
        reject(lambda d:d.update(swap_interval=1))
        reject(lambda d:d.update(last_rendered_pose=0))
        reject(lambda d:d['frames'][0]['gpu_contact_metrics'].update(capacity_loss=True))
        reject(lambda d:d['frames'][0].update(nan_count=1))
        reject(lambda d:d['frames'][0].update(framebuffer_width=1))
    else:
        reject(lambda d:d.update(physics_step=0))
        reject(lambda d:d.update(sleep=True))
print(f'Six measured paths accepted; {checks} corrupt-result controls rejected')
