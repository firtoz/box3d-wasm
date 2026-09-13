#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/compound-import-failure-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
cmake -S native-samples -B native-samples/build-both -DCMAKE_BUILD_TYPE=Release \
 -DGPU_SAMPLES=ON -DBOTH_SAMPLES=ON -DBOX3D_DIR="$ROOT/../../box3d" -DGPU_PHYSICS_DIR="$ROOT"
cmake --build native-samples/build-both --target gpu_both_api gpu_samples_api -j8
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
COMMON=(-O2 -std=c++17 c_abi/compound_import_failure.cpp -I ../../box3d/include
 -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound
 -Wl,--wrap=gpu_b3_create_compound_parent -Wl,--wrap=gpu_b3_create_sphere
 -Wl,--wrap=gpu_b3_shape_attach_compound_child_materials)
g++ "${COMMON[@]}" target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" \
 -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
g++ "${COMMON[@]}" -DGPU_REFERENCE_BOTH -Wl,--whole-archive \
 native-samples/build-both/libgpu_both_api.a native-samples/build-both/libgpu_samples_api.a \
 -Wl,--no-whole-archive target/release/libgpu_physics.a native-samples/build-both/libbox3d_cpu.a "$CPU_LIB" \
 -lGL -ldl -lpthread -lm -lgcc_s -o "$OUT/both"
"$OUT/gpu" > "$OUT/gpu.txt" 2> "$OUT/gpu.log"
"$OUT/both" > "$OUT/both.txt" 2> "$OUT/both.log"
python3 - "$OUT" <<'PY'
from pathlib import Path
import json,hashlib,sys
out=Path(sys.argv[1]);expected=[f'fault {i} shape {int(i==0)} invalid {int(i!=0)} ok 1' for i in range(6)]
expected += [f'count_guard {i} ok 1' for i in range(3)]
for backend in ['gpu','both']:
 assert (out/f'{backend}.txt').read_text().splitlines()==expected
report={'status':'pass','backends':['gpu','both'],'cases_per_backend':9,'count_guards':['negative count','int32 sum overflow','65536 children'],'faults':['parent allocation','first child creation','second child creation','first attachment','second attachment'],'control':'successful two-child import and world cleanup','sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'gpu',out/'both',Path('c_abi/compound_import_failure.cpp')]}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
PY
