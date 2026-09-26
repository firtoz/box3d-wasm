#!/usr/bin/env bash
# Independent Box3D C oracle and GPU traces using upstream ragdoll construction.
set -euo pipefail
cd "$(dirname "$0")/.."
OUT="${1:-artifacts/ragdoll-reference-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
export GPU_PHYSICS_PIPELINE_CACHE_DIR="${GPU_PHYSICS_PIPELINE_CACHE_DIR:-${XDG_CACHE_HOME:-$HOME/.cache}/box3d-gpu-physics/pipelines}"
if [[ "${GPU_RAGDOLL_NATIVE_CACHE:-0}" == 1 ]]; then
    GPU_PHYSICS_SAMPLES_NATIVE_CACHE=1 python3 scripts/run-native-samples.py gpu --build-only
    source scripts/native-samples-cache-env.sh
else
    cargo build --release --lib
fi
cmake -S oracle -B oracle/build -DCMAKE_BUILD_TYPE=Release
cmake --build oracle/build --target box3d -j4
for name in human determinism utils; do
    cc -O2 -DNDEBUG -ffunction-sections -fdata-sections -I ../../box3d/include -I ../../box3d/shared \
        -c "../../box3d/shared/$name.c" -o "$OUT/$name.o"
done
cc -O2 -I ../../box3d/include -c c_abi/samples_stubs.c -o "$OUT/stubs.o"
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
for mode in cpu gpu; do
    extra=()
    engine_lib="$CPU_LIB"
    if [[ "$mode" == gpu && "${GPU_RAGDOLL_NATIVE_CACHE:-0}" == 1 ]]; then
        build=native-samples/build-gpu-native-cache-portable
        extra=(-Wl,--whole-archive target/native-cache-build/release/libgpu_physics.a "$build/libgpu_samples_api.a" -Wl,--no-whole-archive)
        engine_lib="$build/box3d_src/libbox3d.a"
    elif [[ "$mode" == gpu ]]; then
        extra=(-Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a "$OUT/stubs.o")
    fi
    g++ -O2 -std=c++17 -Wl,--gc-sections c_abi/ragdoll_reference.cpp -I ../../box3d/include -I ../../box3d/shared \
        "$OUT/human.o" "$OUT/determinism.o" "$OUT/utils.o" "${extra[@]}" "$engine_lib" \
        -ldl -lpthread -lm -lgcc_s -lGL -o "$OUT/$mode"
    for scene in twist twist-negative tilted tilted-negative ragdolls; do
        steps=120
        if [[ "$scene" == ragdolls ]]; then steps=600; fi
        "$OUT/$mode" "$scene" "$steps" > "$OUT/$scene-$mode.txt" 2> "$OUT/$scene-$mode.log"
    done
done
python3 scripts/validate-ragdoll-reference.py "$OUT"
