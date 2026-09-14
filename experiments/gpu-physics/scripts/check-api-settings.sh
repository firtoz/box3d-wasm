#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
export GPU_PHYSICS_PIPELINE_CACHE_DIR="${GPU_PHYSICS_PIPELINE_CACHE_DIR:-${XDG_CACHE_HOME:-$HOME/.cache}/box3d-gpu-physics/pipelines}"
OUT=artifacts/api-settings
mkdir -p "$OUT"
cargo build --release --lib
for mode in gpu both; do
    B="native-samples/build-$mode"
    if [[ "$mode" == both ]]; then
        cmake -S native-samples -B "$B" -DBOTH_SAMPLES=ON -DGPU_SAMPLES=OFF -DCMAKE_BUILD_TYPE=Release
        cmake --build "$B" --target gpu_both_api gpu_samples_api -j4
        EXTRA=(-DGPU_API_DUAL -Wl,--whole-archive "$B/libgpu_both_api.a" -Wl,--no-whole-archive)
        CPU=("$B/libbox3d_cpu.a")
    else
        cmake -S native-samples -B "$B" -DBOTH_SAMPLES=OFF -DGPU_SAMPLES=ON -DCMAKE_BUILD_TYPE=Release
        cmake --build "$B" --target gpu_samples_api -j4
        EXTRA=()
        CPU=()
    fi
    g++ -O2 -std=c++17 c_abi/api_settings_test.cpp -I ../../box3d/include \
        -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound "${EXTRA[@]}" \
        -Wl,--whole-archive target/release/libgpu_physics.a "$B/libgpu_samples_api.a" -Wl,--no-whole-archive \
        "${CPU[@]}" "$B/box3d_src/libbox3d.a" -ldl -lpthread -lm -lGL -o "$OUT/settings-$mode"
    "$OUT/settings-$mode"
done
