#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/joint-collision-reference-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/joint_collision_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 c_abi/joint_collision_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
"$OUT/cpu" > "$OUT/cpu.txt" 2> "$OUT/cpu.log"
"$OUT/gpu" > "$OUT/gpu.txt" 2> "$OUT/gpu.log"
python3 - "$OUT" <<'PYCODE'
from pathlib import Path
import sys,json,hashlib
out=Path(sys.argv[1])
expected=['disabled 0 0 0 0 0 0','enable-before-step 1 0 0 0 0 0','other-veto 1 0 0 0 0 0','enabled 1 1 1 0 1 0','enabled-noop 1 1 1 1 1 0','disable-immediate 0 0 0 0 1 0','reenable-immediate 1 0 0 0 1 0','reenabled-step 1 1 1 0 1 1']
for engine in ['cpu','gpu']:
 actual=(out/f'{engine}.txt').read_text().splitlines()
 assert actual==expected,(engine,actual)
report={'status':'pass','rows':len(expected),'coverage':['filter joint runtime enable','immediate contact retirement','old contact ID invalidation','deferred end and fresh begin'],
 'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'cpu',out/'gpu',Path('c_abi/joint_collision_reference.cpp')]}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
PYCODE
