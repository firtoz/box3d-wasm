#!/usr/bin/env bash
# Native Village interaction on the ordinary compound import path.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${1:-$ROOT/artifacts/village-probe-$(date +%Y%m%d-%H%M%S)}"
mkdir -p "$(dirname "$OUT")"
mkdir "$OUT" # never reuse prior success
OUT="$(cd "$OUT" && pwd)"
cargo build --release --manifest-path "$ROOT/Cargo.toml"
for mode in cpu gpu; do
 cmake --build "$ROOT/native-samples/build-$mode" --target "samples_$mode" -j8
 unset GPU_PHYSICS_AB
 export GPU_SOKOL_VILLAGE_DROP=1
 export __NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia
 export VK_DRIVER_FILES="${VK_DRIVER_FILES:-/usr/share/vulkan/icd.d/nvidia_icd.json}"
 (cd "$ROOT/../../box3d" && timeout 240 xvfb-run -a \
   "$ROOT/native-samples/build-$mode/bin/samples_$mode" --sample-name Compound/Village \
   --unpaced --health-scan --warmup 0 --timed 240 --bench-json "$OUT/$mode-drop.json") \
   > "$OUT/$mode-drop.log" 2>&1
 sha256sum "$ROOT/native-samples/build-$mode/bin/samples_$mode" >> "$OUT/binaries.sha256"
done
python3 "$ROOT/scripts/validate-village-probe.py" "$OUT"
