#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
OUT="${1:-artifacts/warm-start/current}"
mkdir -p "$OUT"
export GPU_PHYSICS_PIPELINE_CACHE_DIR="${GPU_PHYSICS_PIPELINE_CACHE_DIR:-${XDG_CACHE_HOME:-$HOME/.cache}/box3d-gpu-physics/pipelines}"
LIB="$PWD/target/samples/release/libgpu_physics.a"
SUFFIX=portable
if [[ "${GPU_PHYSICS_SAMPLES_NATIVE_CACHE:-0}" == 1 ]]; then
  source scripts/native-samples-cache-env.sh
  scripts/build-native-cache.sh build --release --lib --features external-c-shim
  LIB="$PWD/target/native-cache-build/release/libgpu_physics.a"
  SUFFIX=native-cache-portable
else
  cargo build --release --lib --features external-c-shim --target-dir target/samples
fi
for mode in gpu both; do
  B="native-samples/build-$mode-$SUFFIX"
  if [[ "$mode" == both ]]; then
    cmake -S native-samples -B "$B" -DGPU_PORTABLE_API=ON -DBOTH_SAMPLES=ON -DGPU_SAMPLES=OFF -DCMAKE_BUILD_TYPE=Release -DGPU_PHYSICS_LIB="$LIB"
    cmake --build "$B" --target gpu_both_api gpu_samples_api box3d -j4
    EXTRA=(-DGPU_API_DUAL -Wl,--whole-archive "$B/libgpu_both_api.a" -Wl,--no-whole-archive)
    CPU=("$B/libbox3d_cpu.a")
  else
    cmake -S native-samples -B "$B" -DGPU_PORTABLE_API=ON -DBOTH_SAMPLES=OFF -DGPU_SAMPLES=ON -DCMAKE_BUILD_TYPE=Release -DGPU_PHYSICS_LIB="$LIB"
    cmake --build "$B" --target gpu_samples_api box3d -j4
    EXTRA=(); CPU=()
  fi
  for fixture in api_settings_test warm_start_fixture; do
    g++ -O2 -std=c++17 "c_abi/$fixture.cpp" -I ../../box3d/include -I c_abi "${EXTRA[@]}" \
      -Wl,--whole-archive "$B/libgpu_samples_api.a" -Wl,--no-whole-archive \
      "$LIB" "${CPU[@]}" "$B/box3d_src/libbox3d.a" -ldl -lpthread -lm -lgcc_s -lGL -o "$OUT/$fixture-$mode"
    "$OUT/$fixture-$mode" > "$OUT/$fixture-$mode.jsonl"
  done
done
python3 scripts/audit-native-api.py --require-built --gpu-build-dir "native-samples/build-gpu-$SUFFIX" --both-build-dir "native-samples/build-both-$SUFFIX" --json "$OUT/api-audit.json"
