#!/usr/bin/env bash
# Native Body Type and torque-free gyro fixtures, with matched setup and substeps.
set -euo pipefail
cd "$(dirname "$0")/.."
OUT="${1:-artifacts/body-dynamics-reference}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
cmake -S native-samples -B native-samples/build-gpu -DCMAKE_BUILD_TYPE=Release \
 -DGPU_SAMPLES=ON -DBOTH_SAMPLES=OFF -DBOX3D_DIR="$(cd ../.. && pwd)/box3d" -DGPU_PHYSICS_DIR="$PWD"
cmake --build native-samples/build-gpu --target gpu_samples_api -j4
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/body_dynamics_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 c_abi/body_dynamics_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound -Wl,--whole-archive native-samples/build-gpu/libgpu_samples_api.a -Wl,--no-whole-archive target/release/libgpu_physics.a "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -lGL -o "$OUT/gpu"
python3 - "$OUT" <<'PY'
import json,math,os,pathlib,subprocess,sys
p=pathlib.Path(sys.argv[1]);env={k:v for k,v in os.environ.items() if not k.startswith('GPU_PHYSICS_')}
env['GPU_PHYSICS_PIPELINE_CACHE_DIR']=os.environ.get('GPU_PHYSICS_PIPELINE_CACHE_DIR',str(p/'pipeline-cache'))
report={}
for scene,steps in [('body',300),('body-callback',300),('gyro',600),('gyro-mass',600)]:
 traces={}
 for engine in ['cpu','gpu']:
  path=p/f'{scene}-{engine}.txt'
  with path.open('w') as out,path.with_suffix('.log').open('w') as err:
   subprocess.run([str(p/engine),scene,str(steps)],env=env,stdout=out,stderr=err,timeout=600,check=True)
  rows=[line.split() for line in path.read_text().splitlines()]
  states={(int(r[1]),int(r[2])):list(map(float,r[3:])) for r in rows if r[0]=='state'}
  bodies=8 if scene.startswith('body') else 2
  assert set(states)=={(step,body) for step in range(steps+1) for body in range(1,bodies+1)}
  assert all(math.isfinite(v) for state in states.values() for v in state)
  assert all(abs(sum(q*q for q in state[3:7])-1)<1e-5 for state in states.values())
  if scene.startswith('body'):
   contacts=[r for r in rows if r[0]=='contact' and r[1]=='37' and r[2] in ('5','6')]
   assert len(contacts)==2 and all(r[-1]=='4' for r in contacts),(engine,contacts)
  traces[engine]=states
 a,b=traces['cpu'],traces['gpu']
 position=max(math.dist(a[k][:3],b[k][:3]) for k in a)
 rotation=max(min(math.dist(a[k][3:7],b[k][3:7]),math.dist(a[k][3:7],[-v for v in b[k][3:7]])) for k in a)
 velocity=max(math.dist(a[k][10:13],b[k][10:13]) for k in a)
 if scene.startswith('body'):assert position<=0.005,position
 else:
  assert position<=1e-5,(scene,'position',position)
  assert rotation<=1e-5,(scene,'quaternion',rotation)
  assert velocity<=1e-4,(scene,'angular velocity',velocity)
 report[scene]={'max_position_delta':position,'max_quaternion_chord':rotation,
  'max_angular_velocity_delta':velocity,'status':'pass',
  'scope':'300-step position and first-impact four-point manifold' if scene.startswith('body') else '600-step CPU orientation, position and angular-velocity agreement (native Vulkan)'}
(p/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
PY
