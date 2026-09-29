#!/usr/bin/env bash
# Linux native-cache protocol; builds before measuring, then publishes portable data.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
if [[ $# -ne 2 ]]; then echo "Usage: $0 machine-run-id 'CPU / GPU label'" >&2; exit 2; fi
ID="$1"
LABEL="$2"
if [[ ! "$ID" =~ ^[a-z0-9][a-z0-9-]{1,79}$ ]]; then echo 'Use a lowercase machine/run identifier' >&2; exit 2; fi
ADAPTER="${CUBE_ADAPTER:-nvidia}"
RAW="$ROOT/artifacts/machine-scaling/$ID"
# Global-color scheduling matches the published large-cube comparison.
read -r -a COUNTS <<< "${CUBE_COUNTS:-100 1000 5000 10000 25000 50000 100000 150000 200000}"
cmake -S oracle -B oracle/build -DCMAKE_BUILD_TYPE=Release
cmake --build oracle/build --target box3d_oracle -j4
cmake -S oracle -B oracle/build-viewer -DCMAKE_BUILD_TYPE=Release
cmake --build oracle/build-viewer --target box3d_viewer_cpu -j4
scripts/build-native-cache.sh build --release --bin gpu-physics
# Resume only with the same binaries and settings; otherwise choose a new run ID.
RESUME=()
if [[ -f "$RAW/manifest.json" ]]; then RESUME=(--resume); fi
python3 scripts/bench-falling-cubes.py "$RAW" \
  --counts "${COUNTS[@]}" \
  --modes physics-cpu physics-gpu direct-cpu direct-gpu \
  --trials 3 --warmup 90 --timed 240 --workers 8 \
  --width 1280 --height 720 --adapter "$ADAPTER" --backend native --gpu-solver global \
  --min-rate 0 --timeout 1200 --selected-binaries-only "${RESUME[@]}"
python3 scripts/publish-cube-machine.py "$RAW" --machine-id "$ID" --label "$LABEL"
python3 scripts/plot-cube-machines.py
