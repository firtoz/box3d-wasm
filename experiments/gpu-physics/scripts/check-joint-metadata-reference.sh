#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/joint-metadata-reference-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/joint_metadata_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 c_abi/joint_metadata_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
"$OUT/cpu" > "$OUT/cpu.txt" 2> "$OUT/cpu.log"
"$OUT/gpu" > "$OUT/gpu.txt" 2> "$OUT/gpu.log"
python3 - "$OUT" <<'PY'
from pathlib import Path
import sys,json,hashlib
out=Path(sys.argv[1]);expected=['generation-reused 1','worlds 1 1']+[f'joint {i} {i} 1 1 1' for i in range(9)]+[f'destroyed {i} 0' for i in range(9)]
for engine in ['cpu','gpu']:
 assert (out/f'{engine}.txt').read_text().splitlines()==expected,engine
report={'status':'pass','rows':20,'joint_types':9,'coverage':['body/shape/joint worlds','native joint type enumeration','live endpoint generation after slot reuse','attached joints destroyed with body'],
'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'cpu',out/'gpu',Path('c_abi/joint_metadata_reference.cpp')]}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
PY
