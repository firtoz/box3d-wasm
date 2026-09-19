#!/usr/bin/env bash
# Exact shared-state regression for the diagnosed Gear Lift impact, not whole-scene acceptance.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/gear-impact-reference}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
python3 scripts/prepare-gear-impact-replay.py "$OUT"
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/gear_impact_replay.cpp -I ../../box3d/include -I ../../box3d/samples -I "$OUT" "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 c_abi/gear_impact_replay.cpp -I ../../box3d/include -I ../../box3d/samples -I "$OUT" -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
python3 scripts/run-gear-impact-replay.py "$OUT" --cpu "$OUT/cpu" --gpu "$OUT/gpu"
python3 - "$OUT" <<'PY'
import json,sys,math,hashlib
from pathlib import Path
out=Path(sys.argv[1]);traces={}
for engine in ['cpu','gpu']:
 traces[engine]={int(r[0]):list(map(float,r[2:])) for r in map(str.split,(out/f'cpu-26-single-{engine}.txt').read_text().splitlines())}
assert set(traces['cpu'])==set(traces['gpu'])==set(range(13))
# State includes position, quaternion, linear velocity and angular velocity.
# Bound the first impact, before warm-start/manifold refresh can diverge.
error=max(abs(x-y) for x,y in zip(traces['cpu'][2],traces['gpu'][2]))
assert error<=1e-4,('first-impact mismatch',error)
sys.path.insert(0, 'scripts')
from gear_support_fixture import GEOMETRY
for name, expected in GEOMETRY['sources'].items():
 assert hashlib.sha256((Path('../..')/name).read_bytes()).hexdigest()==expected,('stale rock geometry',name)
handoff={}
for engine in ['cpu','gpu']:
 states={int(r[0]):list(map(float,r[2:])) for r in map(str.split,(out/f'gpu-168-ccd-handoff-{engine}.txt').read_text().splitlines())}
 assert set(states)==set(range(13)),(engine,'incomplete CCD handoff')
 def bottom(s):
  x,y,z,w=s[3:7]
  return min(s[1]+2*(x*y+z*w)*vx+(1-2*(x*x+z*z))*vy+2*(y*z-x*w)*vz
             for vx,vy,vz in GEOMETRY['rock_points'])
 assert 0 < bottom(states[1]) < .02 and states[1][8] < -8, (engine,'CCD stop not exercised')
 minimum=min(map(bottom,states.values()))
 # Preserve the empty contact from before the CCD stop into the next step.
 # It must refresh rather than let the rock fall 14 cm through the floor.
 assert minimum >= -.005,(engine,'CCD handoff lost floor support',minimum)
 assert states[2][8] > -1,(engine,'contact did not arrest downward motion',states[2][8])
 handoff[engine]={'minimum_y':minimum,'post_handoff_vertical_velocity':states[2][8]}
report={'status':'pass','first_impact_max_state_error':error,'tolerance':1e-4,
 'ccd_handoff':handoff,
 'limitations':['Cold-start single-rock first-impact regression; does not certify Gear Lift mechanisms or the complete native scene.'],
 'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'runs.json',Path('scripts/check-gear-impact-reference.sh')]}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
PY
