#!/usr/bin/env bash
# Requires the configured native GPU sample bridge and Box3D oracle build.
set -euo pipefail
cd "$(dirname "$0")/.."
OUT="${1:-artifacts/hull-mesh-rest-reference}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
cmake --build native-samples/build-gpu --target gpu_samples_api -j4
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/hull_mesh_rest_reference.cpp -I ../../box3d/include -I ../../box3d/samples "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 c_abi/hull_mesh_rest_reference.cpp -I ../../box3d/include -I ../../box3d/samples -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound -Wl,--whole-archive native-samples/build-gpu/libgpu_samples_api.a -Wl,--no-whole-archive target/release/libgpu_physics.a "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -lGL -o "$OUT/gpu"
python3 - "$OUT" <<'PY'
import os,pathlib,subprocess,sys,json,hashlib
p=pathlib.Path(sys.argv[1]);env={k:v for k,v in os.environ.items() if not k.startswith('GPU_PHYSICS_')}
env['GPU_PHYSICS_PIPELINE_CACHE_DIR']=str(p/'pipeline-cache')
for mode in ['cpu','gpu']:
 with (p/(mode+'.jsonl')).open('w') as out,(p/(mode+'.log')).open('w') as err:
  subprocess.run([str(p/mode)],env=env,stdout=out,stderr=err,check=True)
import math
parsed={}
for mode in ['cpu','gpu']:
 log=(p/(mode+'.log')).read_text()
 assert 'Validation Error' not in log and 'uncaptured WebGPU error' not in log, log
 lines=(p/(mode+'.jsonl')).read_text().splitlines()
 assert lines and lines[0]=='contacts 1', (mode,lines)
 normals=[];separations=[]
 for line in lines[1:]:
  fields=line.split()
  if fields[0]=='normal':
   assert fields[4:] == ['points','1'],line
   normals.append([float(x) for x in fields[1:4]])
  elif fields[0]=='separation':separations.append(float(fields[1]))
  else:raise AssertionError(line)
 assert len(normals)==len(separations)==3,(mode,lines)
 assert all(math.isfinite(x) for n in normals for x in n)
 assert all(math.isfinite(x) for x in separations)
 assert min(separations)<-.06,(mode,separations)
 parsed[mode]=list(zip(normals,separations))
# Compare geometry without requiring mesh traversal order.
remaining=parsed['gpu'][:]
for normal,separation in parsed['cpu']:
 match=next((i for i,(n,s) in enumerate(remaining)
             if abs(s-separation)<1e-5 and max(abs(a-b) for a,b in zip(n,normal))<1e-5),None)
 assert match is not None,(normal,separation,remaining)
 remaining.pop(match)
# A clipped polygon can have more vertices than the four-point solver patch.
# Compare its deepest support witness with native, not clipping visitation order.
clipped={}
for mode in ['cpu','gpu']:
 with (p/(mode+'-clipped-support.txt')).open('w') as out,(p/(mode+'-clipped-support.log')).open('w') as err:
  subprocess.run([str(p/mode),'clipped-support'],env=env,stdout=out,stderr=err,check=True)
 lines=(p/(mode+'-clipped-support.txt')).read_text().splitlines()
 log=(p/(mode+'-clipped-support.log')).read_text()
 assert 'Validation Error' not in log and 'uncaptured WebGPU error' not in log
 assert lines and lines[0]=='contacts 1',(mode,lines)
 depths=[float(v.split()[1]) for v in lines if v.startswith('separation ')]
 normals=[[float(x) for x in v.split()[1:4]] for v in lines if v.startswith('normal ')]
 assert depths and all(math.isfinite(v) for v in depths)
 assert min(depths)<-.06,(mode,depths)
 assert any(n[1]>.999 for n in normals),(mode,normals)
 clipped[mode]={'deepest':min(depths),'normals':normals}
assert abs(clipped['cpu']['deepest']-clipped['gpu']['deepest'])<1e-5,clipped
(p/'result.json').write_text(json.dumps({'status':'pass','manifolds':3,'geometry':parsed,'clipped_support':clipped,'binaries':{m:hashlib.sha256((p/m).read_bytes()).hexdigest() for m in ['cpu','gpu']}},indent=2)+'\n')
PY
