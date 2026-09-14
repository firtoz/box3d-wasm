#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
OUT="${1:-artifacts/split-view}"
mkdir -p "$OUT"
export GPU_PHYSICS_PIPELINE_CACHE_DIR="${GPU_PHYSICS_PIPELINE_CACHE_DIR:-${XDG_CACHE_HOME:-$HOME/.cache}/box3d-gpu-physics/pipelines}"
cargo build --release --lib
cmake -S native-samples -B native-samples/build-both -DBOTH_SAMPLES=ON -DGPU_SAMPLES=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build native-samples/build-both --target gpu_both_api gpu_samples_api -j4
B=native-samples/build-both
for FIXTURE in both_pointer both_drag; do
g++ -O2 -std=c++17 "c_abi/${FIXTURE}_test.cpp" ../../box3d/samples/host/camera.cpp -I ../../box3d/include -I c_abi -I native-samples -I ../../box3d/samples -I ../../box3d/extern/sokol \
 -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound \
 -Wl,--whole-archive "$B/libgpu_both_api.a" "$B/libgpu_samples_api.a" -Wl,--no-whole-archive \
 target/release/libgpu_physics.a "$B/libbox3d_cpu.a" "$B/box3d_src/libbox3d.a" \
 -ldl -lpthread -lm -lgcc_s -lGL -o "$OUT/${FIXTURE//_/-}"
"$OUT/${FIXTURE//_/-}"
done
"$OUT/both-drag" --ground
