#!/usr/bin/env bash
# Diagnostic comparison using an existing portable BOTH build.
# First build with check-both-pointer.sh; pass its build directory and Rust library
# as arguments 2 and 3 when using the native-cache backend. The strict gate remains enforced.
set -euo pipefail
cd "$(dirname "$0")/.."
OUT="$(realpath -m "${1:-artifacts/drag-phases}")"
mkdir -p "$OUT"
export GPU_PHYSICS_PIPELINE_CACHE_DIR="${GPU_PHYSICS_PIPELINE_CACHE_DIR:-${XDG_CACHE_HOME:-$HOME/.cache}/box3d-gpu-physics/pipelines}"
B="${2:-native-samples/build-both-portable}"
LIB="${3:-target/samples/release/libgpu_physics.a}"
if [[ "$B" == *native-cache* ]]; then
  source scripts/native-samples-cache-env.sh
fi
python3 scripts/prepare-drag-phase-oracle.py "$OUT/oracle" --build-dir "$B"
g++ -O2 -std=c++17 "$OUT/oracle/both_drag_phase.cpp" ../../box3d/samples/host/camera.cpp \
 -I ../../box3d/include -I c_abi -I native-samples -I ../../box3d/samples -I ../../box3d/extern/sokol \
 -Wl,--whole-archive "$B/libgpu_both_api.a" "$B/libgpu_samples_api.a" -Wl,--no-whole-archive \
 "$LIB" "$OUT/oracle/libbox3d_cpu.a" "$B/box3d_src/libbox3d.a" \
 -ldl -lpthread -lm -lgcc_s -lGL -o "$OUT/both-drag-phase"
GPU_PHYSICS_AB="${GPU_PHYSICS_AB:-phase-capture}" DRAG_PHASE_RANGE="${DRAG_PHASE_RANGE:-660:900}" \
 "$OUT/both-drag-phase" --ground-strict >"$OUT/result.log" 2>"$OUT/phases.log"
