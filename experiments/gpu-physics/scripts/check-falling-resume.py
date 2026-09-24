#!/usr/bin/env python3
"""Exercise interruption recovery without launching physics or renderer processes."""
import json
from pathlib import Path
import runpy
import sys
import tempfile
from unittest.mock import patch

module=runpy.run_path(str(Path(__file__).with_name('bench-falling-cubes.py')))
main=module['main']
g=main.__globals__
commands=[]
module['platform'].platform()  # Cache platform discovery before mocking subprocess.

def run(cmd, **kwargs):
    commands.append(cmd)
    if '--bench' in cmd:
        raise module['subprocess'].CalledProcessError(1,cmd)
    path=Path(cmd[cmd.index('--metrics')+1])
    count=int(cmd[cmd.index('--bodies')+1])
    path.write_text(json.dumps({'scenes':{'falling-cubes':dict(bodies=count+1,workers=8,
        wall_samples_ms=[2],wall_p50_ms=2,wall_p95_ms=2)}}))

with tempfile.TemporaryDirectory() as tmp:
    out=Path(tmp)/'run'
    argv=['bench',str(out),'--counts','5','10','--modes','physics-cpu','physics-gpu',
          '--trials','1','--warmup','1','--timed','1','--require-idle']
    quiet=dict(quiet=True,cpu_busy=.01,gpu_busy=0)
    idle=iter([quiet,quiet,dict(quiet=False,cpu_busy=.5,gpu_busy=0)])
    with patch.dict(g,command_output=lambda args:'fixture',system=lambda:dict(ac={}),idle_check=lambda:next(idle)), \
         patch.object(module['subprocess'],'run',run), \
         patch.object(module['subprocess'],'check_output',lambda args:b'fixture'), \
         patch.object(sys,'argv',argv):
        try: main()
        except SystemExit as e: assert 'Background load' in str(e)
        else: raise AssertionError('idle gate did not stop the run')
    assert len(commands)==2
    # Simulate a crash after trials.json replacement but before stopped.json.
    (out/'stopped.json').unlink()
    with patch.dict(g,command_output=lambda args:'fixture',system=lambda:dict(ac={}),idle_check=lambda:quiet), \
         patch.object(module['subprocess'],'run',run), \
         patch.object(module['subprocess'],'check_output',lambda args:b'fixture'), \
         patch.object(sys,'argv',argv+['--resume']):
        main()
    assert len(commands)==3 and '--metrics' in commands[-1]
    rows=json.loads((out/'trials.json').read_text())
    assert [(r['count'],r['mode']) for r in rows]==[(5,'physics-cpu'),(5,'physics-gpu'),(10,'physics-cpu')]
    assert json.loads((out/'stopped.json').read_text())['physics-gpu']['count']==5
    assert all(r['idle']['quiet'] for r in rows)
    try: module['write_json'](out/'trials.json',object())
    except TypeError: pass
    else: raise AssertionError('unserializable result accepted')
    assert json.loads((out/'trials.json').read_text())==rows
print('Resume preserves completed trials, terminal failures and idle evidence; failed writes preserve state')

with tempfile.TemporaryDirectory() as tmp:
    out=Path(tmp)/'capacity'
    start=len(commands)
    argv=['bench',str(out),'--counts','70000','--modes','physics-cpu','direct-cpu',
          '--trials','1','--warmup','1','--timed','1']
    with patch.dict(g,command_output=lambda args:'fixture',system=lambda:dict(ac={})), \
         patch.object(module['subprocess'],'run',run), \
         patch.object(module['subprocess'],'check_output',lambda args:b'fixture'), \
         patch.object(sys,'argv',argv):
        main()
    assert len(commands)==start+1 and '--metrics' in commands[-1]
    assert json.loads((out/'stopped.json').read_text())['direct-cpu']['max_dynamic_cubes']==65535
print('CPU-only counts continue beyond the direct viewer\'s 16-bit scene capacity')
