#!/usr/bin/env python3
"""Check idle qualification against quiet, transient and sustained load traces."""
from pathlib import Path
import runpy
from unittest.mock import patch

module=runpy.run_path(str(Path(__file__).with_name('bench-falling-cubes.py')))
check=module['idle_check'];g=check.__globals__

def trace(busy_percent, gpu=0):
    total=idle=0;lines=['cpu 0 0 0 0 0 0 0 0\n']
    for busy in busy_percent:
        total+=100;idle+=100-busy
        lines.append(f'cpu {total-idle} 0 0 {idle} 0 0 0 0\n')
    readings=iter(lines);sleeps=[]
    with patch.object(Path,'read_text',lambda *args,**kwargs:next(readings)), \
         patch.object(module['time'],'sleep',lambda seconds:sleeps.append(seconds)), \
         patch.dict(g,command_output=lambda args:str(gpu)):
        result=check()
    assert result['observed_seconds']==sum(sleeps)
    return result

quiet=trace([10]);assert quiet['quiet'] and quiet['observed_seconds']==2
burst=trace([20,10,10,10,10]);assert burst['quiet'] and burst['observed_seconds']==10
assert abs(burst['cpu_busy']-.12)<1e-9
busy=trace([20]*5);assert not busy['quiet'] and busy['observed_seconds']==10
assert not trace([10]*5,gpu=20)['quiet']
print('Idle gate: quiet fast path, transient CPU spike, sustained CPU load and GPU ceiling passed')
