#!/usr/bin/env bash
# Requires the configured native GPU sample bridge and Box3D oracle build.
set -euo pipefail
cd "$(dirname "$0")/.."
OUT="${1:-artifacts/hull-edge-reference}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
cmake --build native-samples/build-gpu --target gpu_samples_api -j4
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/hull_edge_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 c_abi/hull_edge_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound -Wl,--whole-archive native-samples/build-gpu/libgpu_samples_api.a -Wl,--no-whole-archive target/release/libgpu_physics.a "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -lGL -o "$OUT/gpu"
python3 - "$OUT" <<'PY'
import os,pathlib,subprocess,sys,json,hashlib
p=pathlib.Path(sys.argv[1]);env={k:v for k,v in os.environ.items() if not k.startswith('GPU_PHYSICS_')}
env['GPU_PHYSICS_PIPELINE_CACHE_DIR']=str(p/'pipeline-cache')
for mode in ['cpu','gpu']:
 with (p/(mode+'.jsonl')).open('w') as out,(p/(mode+'.log')).open('w') as err:
  subprocess.run([str(p/mode)],env=env,stdout=out,stderr=err,check=True)
for mode in ['cpu','gpu']:
 lines=(p/(mode+'.jsonl')).read_text().splitlines()
 assert lines==['case 0 pass: 0 points','case 1 pass: 1 points','case 2 pass: 4 points'], (mode,lines)
(p/'result.json').write_text(json.dumps({'status':'pass','cases':3,'binaries':{m:hashlib.sha256((p/m).read_bytes()).hexdigest() for m in ['cpu','gpu']}},indent=2)+'\n')
PY
