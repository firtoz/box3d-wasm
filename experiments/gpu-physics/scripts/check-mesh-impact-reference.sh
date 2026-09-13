#!/usr/bin/env bash
# Shared-state native Mesh Drop impact; failure remains a required physics failure.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-$ROOT/artifacts/mesh-impact-reference}"
CASE="${2:-233}"
case "$CASE" in 233|842) ;; *) echo "expected reference body 233 or 842" >&2; exit 64 ;; esac
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json" "$OUT/cpu.txt" "$OUT/gpu.txt"
python3 "$ROOT/scripts/validate-mesh-impact.py" --self-check
cargo build --release
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/mesh_impact_reference.cpp -I ../../box3d/include \
  "$CPU_LIB" -lpthread -lm -o target/release/mesh_impact_cpu
g++ -O2 -std=c++17 -DGPU_REFERENCE c_abi/mesh_impact_reference.cpp -I ../../box3d/include \
  -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound \
  target/release/libgpu_physics.a "$CPU_LIB" -ldl -lpthread -lm -lgcc_s \
  -o target/release/mesh_impact_gpu
ARGS=("$CASE")
if [[ "$CASE" == 842 ]]; then ARGS+=(frozen57); fi
target/release/mesh_impact_cpu "${ARGS[@]}" > "$OUT/cpu.txt" 2> "$OUT/cpu.log"
GPU_PHYSICS_TRACE_BODY=2 GPU_PHYSICS_TRACE_CONTACTS=1 \
  target/release/mesh_impact_gpu "${ARGS[@]}" > "$OUT/gpu.txt" 2> "$OUT/gpu.log"
python3 "$ROOT/scripts/validate-mesh-impact.py" "$OUT"
