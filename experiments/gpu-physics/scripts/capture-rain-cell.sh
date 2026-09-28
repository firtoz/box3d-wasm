#!/usr/bin/env bash
# Diagnostic isolation, not a replacement for the complete Rain qualification.
set -euo pipefail
cd "$(dirname "$0")/.."
OUT="${1:?usage: capture-rain-cell.sh EMPTY_OUTPUT_DIRECTORY}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
if [[ -n "$(ls -A "$OUT")" ]]; then echo 'output directory must be empty' >&2; exit 2; fi
cargo build --release --lib --features replay-diagnostics
cmake --build native-samples/build-gpu --target gpu_samples_api -j6
for name in human utils; do
 cc -O2 -DNDEBUG -ffunction-sections -fdata-sections -I ../../box3d/include -I ../../box3d/shared -c "../../box3d/shared/$name.c" -o "$OUT/$name.o"
done
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
for mode in cpu gpu; do
 extra=()
 if [[ "$mode" == gpu ]]; then
  extra=(-Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound -Wl,--whole-archive native-samples/build-gpu/libgpu_samples_api.a -Wl,--no-whole-archive target/release/libgpu_physics.a)
 fi
 g++ -O2 -std=c++17 -Wl,--gc-sections c_abi/rain_cell_reference.cpp -I ../../box3d/include -I ../../box3d/shared "$OUT/human.o" "$OUT/utils.o" "${extra[@]}" "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -lGL -o "$OUT/$mode"
done
# Ordinary Rust build, with the same GPU shader/solver policy as native samples.
source scripts/native-samples-cache-env.sh
export GPU_PHYSICS_LIVE_CONTACT_ORDER=0
export GPU_PHYSICS_PIPELINE_CACHE_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/box3d-gpu-physics/pipelines"
python3 - "$OUT" <<'PY'
import hashlib,json,os,sys
from pathlib import Path
p=Path(sys.argv[1]);r=dict(configuration='ordinary build; native-sample shader/solver policy; CCD ablation is diagnostic only',environment={k:v for k,v in os.environ.items() if k.startswith(('GPU_PHYSICS_','WGPU_','VK_'))},sha256={mode:hashlib.sha256((p/mode).read_bytes()).hexdigest() for mode in ['cpu','gpu']})
(p/'manifest.json').write_text(json.dumps(r,indent=2)+'\n')
PY
for mode in cpu gpu; do
 for variant in ccd no-ccd; do
  "$OUT/$mode" 210 "$variant" > "$OUT/$mode-$variant.txt" 2> "$OUT/$mode-$variant.log"
  printf '0\n' > "$OUT/$mode-$variant-exit.txt"
 done
done
