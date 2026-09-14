#!/usr/bin/env bash
# Diagnostic comparison; the final command enforces the unchanged strict gate.
set -euo pipefail
cd "$(dirname "$0")/.."
OUT="$(realpath -m "${1:-artifacts/drag-phases}")"
mkdir -p "$OUT"
export GPU_PHYSICS_PIPELINE_CACHE_DIR="${GPU_PHYSICS_PIPELINE_CACHE_DIR:-${XDG_CACHE_HOME:-$HOME/.cache}/box3d-gpu-physics/pipelines}"
./scripts/check-both-pointer.sh "$OUT/baseline"
python3 scripts/prepare-drag-phase-oracle.py "$OUT/oracle"
B=native-samples/build-both
g++ -O2 -std=c++17 "$OUT/oracle/both_drag_phase.cpp" ../../box3d/samples/host/camera.cpp \
 -I ../../box3d/include -I c_abi -I native-samples -I ../../box3d/samples -I ../../box3d/extern/sokol \
 -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound \
 -Wl,--whole-archive "$B/libgpu_both_api.a" "$B/libgpu_samples_api.a" -Wl,--no-whole-archive \
 target/release/libgpu_physics.a "$OUT/oracle/libbox3d_cpu.a" "$B/box3d_src/libbox3d.a" \
 -ldl -lpthread -lm -lgcc_s -lGL -o "$OUT/both-drag-phase"
GPU_PHYSICS_AB=phase-capture DRAG_PHASE_RANGE="${DRAG_PHASE_RANGE:-660:900}" \
 "$OUT/both-drag-phase" --ground-strict >"$OUT/result.log" 2>"$OUT/phases.log"
