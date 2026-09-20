#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
OUT="${1:-artifacts/joint-separation}"
mkdir -p "$OUT"
export GPU_PHYSICS_PIPELINE_CACHE_DIR="${GPU_PHYSICS_PIPELINE_CACHE_DIR:-${XDG_CACHE_HOME:-$HOME/.cache}/box3d-gpu-physics/pipelines}"
B=native-samples/build-both-portable
LIB="$PWD/target/samples/release/libgpu_physics.a"
CACHE="${GPU_PHYSICS_SAMPLES_NATIVE_CACHE:-auto}"
case "$CACHE" in auto|0|1) ;; *) echo "GPU_PHYSICS_SAMPLES_NATIVE_CACHE must be auto, 0 or 1" >&2; exit 1 ;; esac
if [[ "$CACHE" == auto ]]; then
  CACHE=0
  if [[ "$(uname -s)" == Linux && "${GPU_PHYSICS_BACKEND:-auto}" =~ ^(auto|vulkan)$ ]] && command -v patch >/dev/null && command -v flock >/dev/null; then CACHE=1; fi
fi
if [[ "$CACHE" == 1 ]]; then
  source scripts/native-samples-cache-env.sh
  scripts/build-native-cache.sh build --release --lib --features external-c-shim
  B=native-samples/build-both-native-cache-portable
  LIB="$PWD/target/native-cache-build/release/libgpu_physics.a"
else
  cargo build --release --lib --features external-c-shim --target-dir target/samples
fi
cmake -S native-samples -B "$B" -DGPU_PORTABLE_API=ON -DBOTH_SAMPLES=ON -DGPU_SAMPLES=OFF -DCMAKE_BUILD_TYPE=Release -DGPU_PHYSICS_LIB="$LIB"
cmake --build "$B" --target gpu_both_api gpu_samples_api -j4
g++ -O2 -std=c++17 c_abi/both_joint_separation_test.cpp -I ../../box3d/include -I c_abi \
 -Wl,--whole-archive "$B/libgpu_both_api.a" "$B/libgpu_samples_api.a" -Wl,--no-whole-archive \
 "$LIB" "$B/libbox3d_cpu.a" "$B/box3d_src/libbox3d.a" \
 -ldl -lpthread -lm -lgcc_s -lGL -o "$OUT/joint-separation"
"$OUT/joint-separation"
