#!/usr/bin/env python3
"""Identify the exact binaries/settings used to generate a comparison column."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import subprocess

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('output',type=Path)
p.add_argument('binary',type=Path)
p.add_argument('--oracle',type=Path)
p.add_argument('--frames',type=int,required=True)
p.add_argument('--metric-runs',type=int,default=1)
a=p.parse_args()
if a.frames <= 0 or a.metric_runs <= 0: p.error('positive frames and metric runs required')
def identity(path):
    path=path.resolve()
    return dict(path=str(path),sha256=hashlib.sha256(path.read_bytes()).hexdigest())
root=Path(__file__).resolve().parents[1]
d=dict(recording_started_utc=datetime.now(timezone.utc).isoformat(),frames=a.frames,
       metric_runs=a.metric_runs,viewer=identity(a.binary),oracle=identity(a.oracle) if a.oracle else None,
       checkout_head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip(),
       environment={k:v for k,v in os.environ.items() if k.startswith(('GPU_','WGPU_','VK_','__NV','__GLX','DISPLAY'))},
       note='Records requested capture settings and binary identity. Completion requires clips and metrics; checkout HEAD alone does not identify a dirty or externally supplied build.')
a.output.mkdir(parents=True,exist_ok=True)
scenes=os.environ.get('RECORD_SCENES','').split()
d['scenes']=scenes or 'all'
# A scoped capture must not relabel older clips with the new binary identity.
names=[f'{scene}-recording-manifest.json' for scene in scenes] if scenes else ['recording-manifest.json']
for name in names:
    if Path(name).name != name: p.error('scene names must not contain path separators')
    (a.output/name).write_text(json.dumps(d,indent=2)+'\n')
