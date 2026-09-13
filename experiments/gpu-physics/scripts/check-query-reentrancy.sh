#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/query-reentrancy-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/query_reentrancy.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 c_abi/query_reentrancy.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
# Bound the process: a callback lock regression must fail rather than hang the gate.
for backend in cpu gpu; do
 timeout -k 5 90 "$OUT/$backend" > "$OUT/$backend.txt" 2> "$OUT/$backend.log"
done
python3 - "$OUT" <<'PY'
from pathlib import Path
import hashlib,json,sys
out=Path(sys.argv[1]); expected=[f'mode={i} valid=1' for i in range(4)]
for backend in ['cpu','gpu']:
 assert (out/f'{backend}.txt').read_text().splitlines()==expected
report={'status':'pass','coverage':['callback body reads','nested GPU closest hit','nested callback ray','continue/terminate/ignore/clip responses'], 'cpu_win_validated':False,
 'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'cpu',out/'gpu',Path('c_abi/query_reentrancy.cpp'),Path('src/api/query.rs'),Path('src/api/world.rs')]}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
PY
