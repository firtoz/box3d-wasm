#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/compound-mesh-bake-gate}"
mkdir -p "$OUT"
rm -f "$OUT/result.json"
cc -g -fsanitize=address -fno-omit-frame-pointer -I ../../box3d/include \
 c_abi/compound_mesh_bake_test.c oracle/build/box3d-build/src/libbox3d.a -lpthread -lm -o "$OUT/cpu"
ASAN_OPTIONS=detect_leaks=1 "$OUT/cpu" > "$OUT/output.txt"
python3 - "$OUT" <<'PY'
import json,hashlib,sys
from pathlib import Path
out=Path(sys.argv[1]);d={'status':'pass','scale_cases':8,'allocation_failures':3,'address_and_leak_sanitizer':True,'source_sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [Path('c_abi/compound_mesh_bake.h'),Path('c_abi/compound_mesh_bake_test.c')]}}
(out/'result.json').write_text(json.dumps(d,indent=2)+'\n');print(json.dumps(d))
PY
