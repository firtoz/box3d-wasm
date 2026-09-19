#!/usr/bin/env python3
"""Extract native Gear Lift terrain and matched cold-start debris states."""
import json,sys,hashlib
from pathlib import Path
root=Path(__file__).resolve().parents[1]
out=Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
source=root/'../../box3d/samples/sample_joint.cpp'
s=source.read_text().split('class GearLift :')[1]
def method(name):
 start=s.index('\tvoid '+name+'(');body=s.index('{',start);depth=1;i=body+1
 while depth:
  depth+=(s[i]=='{')-(s[i]=='}');i+=1
 return s[start:i]
header='// Generated from upstream sample_joint.cpp, SPDX-License-Identifier: MIT\nstruct GearTerrain {\n b3MeshData* m_mesh=nullptr;\n'+method('CreateMesh')+'\n'+method('PushCap')+'\n};\n'
(out/'gear_terrain_generated.inc').write_text(header)
inputs={}
for path in sorted((root/'c_abi/fixtures/gear-impact').glob('*.state')):
 (out/path.name).write_bytes(path.read_bytes())
 inputs[str(path)]=hashlib.sha256(path.read_bytes()).hexdigest()
assert len(inputs)==5, 'Missing Gear Lift regression fixtures'
inputs[str(source)]=hashlib.sha256(source.read_bytes()).hexdigest()
(out/'inputs.json').write_text(json.dumps({'status':'diagnostic','sha256':inputs,'limitations':['Cold contact caches, no mechanism bodies/joints. Not full native scene acceptance.']},indent=2)+'\n')
