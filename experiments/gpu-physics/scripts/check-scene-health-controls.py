#!/usr/bin/env python3
"""Negative validation controls using a fresh successful native probe as input."""
import copy, importlib.util, json, sys
from pathlib import Path
spec=importlib.util.spec_from_file_location('switch_validator',Path(__file__).with_name('validate-scene-switch.py'))
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
out=Path(sys.argv[1]);cpu=json.loads((out/'cpu-switch.json').read_text());gpu=json.loads((out/'gpu-switch.json').read_text())
m.compare(cpu,gpu)
controls=[]
def reject(name, mutate):
    bad=copy.deepcopy(gpu);mutate(bad)
    try: m.compare(cpu,bad)
    except (AssertionError,KeyError,TypeError,ValueError): controls.append(name);return
    raise AssertionError(f'accepted {name}')
reject('missing frame',lambda d:d['frames'].pop())
reject('wrong actual scene',lambda d:d['frames'][8].update(sample='Compound/Village'))
reject('wrong reset step',lambda d:d['frames'][8].update(submitted_step=9))
reject('incomplete status',lambda d:d.update(status='incomplete'))
reject('capacity failure',lambda d:d.update(gpu_fail='capacity loss'))
reject('empty refused Village',lambda d:d['frames'][0].update(body_count=0))
reject('non-finite body',lambda d:d['frames'][8]['bodies'][0]['p'].__setitem__(0,float('nan')))
reject('lost kinetic energy',lambda d:d['frames'][8]['bodies'][0]['v'].__setitem__(0,0))
reject('escaped enclosure',lambda d:d['frames'][8]['bodies'][0]['p'].__setitem__(0,10))
reject('wrong tangent trajectory',lambda d:d['frames'][8]['bodies'][0]['p'].__setitem__(2,3))
# The Bounce House exception must not leak into ordinary scene health.
regular=copy.deepcopy(gpu);regular['sample']='Events/Hit'
for f in regular['frames']: f['sample']='Events/Hit'
assert m.generic_health(regular)[0]=='fail'
controls.append('ordinary scene retains 120m/s limit')
report={'status':'pass','rejected':controls}
(out/'negative-controls.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
