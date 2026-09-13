#!/usr/bin/env bash
# Requires the native both build (for the prefixed real Box3D library).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
cargo build --release
if [[ ! -f native-samples/build-both/libbox3d_cpu.a ]]; then
  echo 'Build samples:both first to generate libbox3d_cpu.a' >&2
  exit 1
fi
cc -O2 -std=c17 -D_POSIX_C_SOURCE=200809L -ffunction-sections -fdata-sections \
  -I ../../box3d/include -I c_abi -c c_abi/recycling_both_sample.c -o target/release/recycling_both_sample.o
cc -O2 -std=c17 -ffunction-sections -fdata-sections \
  -I ../../box3d/include -I c_abi -c c_abi/both_map.c -o target/release/recycling_both_map.o
g++ -Wl,--gc-sections -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound \
  target/release/recycling_both_sample.o target/release/recycling_both_map.o \
  target/release/libgpu_physics.a native-samples/build-both/libbox3d_cpu.a \
  oracle/build/box3d-build/src/libbox3d.a -ldl -lpthread -lm -lgcc_s \
  -o target/release/recycling_both_sample
target/release/recycling_both_sample
