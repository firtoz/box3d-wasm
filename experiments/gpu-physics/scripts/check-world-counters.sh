#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/world-counters-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/world_counters_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 -DGPU_REFERENCE c_abi/world_counters_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
for backend in cpu gpu; do
 timeout -k 5 90 "$OUT/$backend" > "$OUT/$backend.txt" 2> "$OUT/$backend.log"
done
python3 - "$OUT" <<'PY'
from pathlib import Path
import json,hashlib,sys
out=Path(sys.argv[1]);expected=['0 2 2 2','1 2 2 1','2 2 1 0','3 2 0 0']
for backend in ['cpu','gpu']:assert (out/f'{backend}.txt').read_text().splitlines()==expected
r={'status':'pass','scope':'native public body/compound shape/live joint counters via GPU FFI used by samples_api.c','contact_count_validated':False,
'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'cpu',out/'gpu',Path('c_abi/world_counters_reference.cpp')]}}
(out/'result.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r,indent=2))
PY
