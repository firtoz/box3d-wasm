#!/usr/bin/env bash
# Same prescribed state on both engines. No contact/joint cache import.
set -euo pipefail
cd "$(dirname "$0")/.."
STATE="$(realpath "${1:?provide a 13-float position/quaternion/linear/angular velocity state file}")"
OUT="$(realpath -m "${2:-artifacts/impact-replay}")"
STEPS="${3:-120}"
mkdir -p "$OUT"
export GPU_PHYSICS_PIPELINE_CACHE_DIR="${GPU_PHYSICS_PIPELINE_CACHE_DIR:-${XDG_CACHE_HOME:-$HOME/.cache}/box3d-gpu-physics/pipelines}"
cargo build --release --lib
B=native-samples/build-both
cmake -S native-samples -B "$B" -DBOTH_SAMPLES=ON -DGPU_SAMPLES=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build "$B" --target gpu_both_api gpu_samples_api -j4
g++ -O2 -std=c++17 c_abi/both_impact_test.cpp ../../box3d/samples/host/camera.cpp \
 -I ../../box3d/include -I c_abi -I native-samples -I ../../box3d/samples -I ../../box3d/extern/sokol \
 -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound \
 -Wl,--whole-archive "$B/libgpu_both_api.a" "$B/libgpu_samples_api.a" -Wl,--no-whole-archive \
 target/release/libgpu_physics.a "$B/libbox3d_cpu.a" "$B/box3d_src/libbox3d.a" \
 -ldl -lpthread -lm -lgcc_s -lGL -o "$OUT/both-impact"
"$OUT/both-impact" "$STATE" "$STEPS" >"$OUT/replay.txt" 2>"$OUT/replay.log"
python3 - "$OUT/replay.txt" "$STEPS" <<'PY'
import sys,math
from pathlib import Path
states={}
for line in Path(sys.argv[1]).read_text().splitlines():
    r=line.split()
    if r[0]!='F': continue
    key=(int(r[1]),int(r[2]))
    assert key not in states
    states[key]=list(map(float,r[4:7]+r[8:12]+r[13:16]+r[17:20]))
assert set(states)=={(s,e) for s in range(int(sys.argv[2])+1) for e in range(2)}
assert all(math.isfinite(x) for values in states.values() for x in values)
assert states[0,0]==states[0,1], 'initial states differ'
for label,lo,hi in [('position',0,3),('velocity',7,10),('angular velocity',10,13)]:
    error,step=max((math.dist(states[s,0][lo:hi],states[s,1][lo:hi]),s) for s in range(int(sys.argv[2])+1))
    print(f'DIAGNOSTIC {label}: max delta {error:.9g} at step {step}')
print('Cold-start replay only: this does not certify the retained-history drag gate.')
PY
