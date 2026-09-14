#!/usr/bin/env bash
# One diagnostic step from native cached state; never an end-to-end parity gate.
set -euo pipefail
cd "$(dirname "$0")/.."
FRAME="${1:-2404}"
[[ "$FRAME" =~ ^[0-9]+$ ]] && (( FRAME < 3060 )) || {
  echo 'Expected a frame in the ground drag fixture (0..3059)' >&2; exit 2;
}
OUT="$(realpath -m "${2:-artifacts/drag-state-replay/$FRAME}")"
mkdir -p "$OUT"
export GPU_PHYSICS_PIPELINE_CACHE_DIR="${GPU_PHYSICS_PIPELINE_CACHE_DIR:-${XDG_CACHE_HOME:-$HOME/.cache}/box3d-gpu-physics/pipelines}"
# Build in a separate target directory so the normal viewer library never gains
# the diagnostic state-transfer export, even when this command is interrupted.
REPLAY_TARGET="$(realpath -m artifacts/drag-state-replay-target)"
if [[ ! -f native-samples/build-both/libgpu_both_api.a ]]; then
  echo 'Run scripts/check-both-pointer.sh first to build the native archives.' >&2
  exit 2
fi
cargo build --release --lib --features replay-diagnostics --target-dir "$REPLAY_TARGET"
python3 scripts/prepare-drag-state-replay.py "$OUT/oracle"
B=native-samples/build-both
g++ -O2 -std=c++17 "$OUT/oracle/both_drag_phase.cpp" ../../box3d/samples/host/camera.cpp \
 "$OUT/oracle/drag_snapshot.o" \
 -I ../../box3d/include -I c_abi -I native-samples -I ../../box3d/samples -I ../../box3d/extern/sokol \
 -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound \
 -Wl,--whole-archive "$OUT/oracle/libgpu_both_api.a" "$B/libgpu_samples_api.a" -Wl,--no-whole-archive \
 "$REPLAY_TARGET/release/libgpu_physics.a" "$OUT/oracle/libbox3d_cpu.a" "$B/box3d_src/libbox3d.a" \
 -ldl -lpthread -lm -lgcc_s -lGL -o "$OUT/both-drag-replay"
GPU_PHYSICS_AB=phase-capture DRAG_SEED_FRAME="$FRAME" DRAG_SNAPSHOT="$OUT/snapshot.json" \
 DRAG_PHASE_RANGE="$FRAME:$FRAME" BOTH_DRAG_TRACE="$OUT/trace.txt" BOTH_DRAG_TRACE_BODY=2 \
 "$OUT/both-drag-replay" --ground >"$OUT/result.log" 2>"$OUT/phases.log"
python3 - "$OUT" "$FRAME" <<'PY'
import math
from pathlib import Path
import sys
out, frame = Path(sys.argv[1]), int(sys.argv[2])
lines = [line for line in (out / 'result.log').read_text().splitlines()
         if line.startswith(f'DIAGNOSTIC seeded step {frame} ')]
assert len(lines) == 1, 'Missing completed diagnostic step'
errors = [float(lines[0].split()[i]) for i in (5, 7, 9)]
assert all(math.isfinite(value) for value in errors)
# Identical cached input, one step, all five bodies. This is separate from
# the retained-history ten-drag tolerances, which remain unchanged.
limits = (1e-5, 1e-5, 1e-4)
assert all(value <= limit for value, limit in zip(errors, limits)), (
    f'Cached-state step diverged: position/quaternion/velocity {errors}, limits {limits}')
log = (out / 'phases.log').read_text()
assert 'body and joint readback verified' in log
assert 'anchors/impulses/cache readback verified' in log
print(lines[0])
print('Diagnostic only: the original ten-drag strict gate must run separately.')
PY
