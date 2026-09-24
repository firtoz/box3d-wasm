#!/usr/bin/env python3
"""Verify raw timing agreement and reject corrupted or duplicated summary rows."""
import argparse
import copy
import json
from pathlib import Path
import runpy
from unittest.mock import patch

p=argparse.ArgumentParser(description=__doc__);p.add_argument('run',type=Path);a=p.parse_args()
m=runpy.run_path(str(Path(__file__).with_name('plot-falling-cubes.py')))
report=m['summarize'](a.run)
path=a.run/'trials.json';rows=json.loads(path.read_text());original=Path.read_text
for kind in ['timing','duplicate']:
    bad=copy.deepcopy(rows)
    if kind=='timing':next(r for r in bad if r['status']=='ok')['mean_ms']+=1
    else:bad.append(copy.deepcopy(bad[0]))
    def read(self,*args,**kwargs):
        return json.dumps(bad) if self==path else original(self,*args,**kwargs)
    with patch.object(Path,'read_text',read):
        try:m['summarize'](a.run)
        except AssertionError:pass
        else:raise AssertionError('accepted '+kind+' corruption')
print(f"Validated {len(report['results'])} complete result groups; altered timing and duplicate trials rejected")
