#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
REFERENCE_BUILD_DIR="${REFERENCE_DUMP_BUILD_DIR:-$ROOT_DIR/.reference-dumps/reference-build}"
NATIVE_BRIDGE_BUILD_DIR="$ROOT_DIR/packages/box3d-wasm/build-native"
BOX3D_SOURCE_DIR="$ROOT_DIR/box3d"

if [ ! -f "$BOX3D_SOURCE_DIR/CMakeLists.txt" ]; then
  printf 'Missing box3d checkout at %s\n' "$BOX3D_SOURCE_DIR" >&2
  exit 1
fi

mkdir -p "$REFERENCE_BUILD_DIR" "$NATIVE_BRIDGE_BUILD_DIR"

printf 'Configuring reference-dump compile database...\n'
cmake -S "$ROOT_DIR/tools/reference-dump" -B "$REFERENCE_BUILD_DIR" \
  -DBOX3D_DOUBLE_PRECISION=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER="${CC:-clang}" \
  -DCMAKE_CXX_COMPILER="${CXX:-clang++}"

printf 'Configuring native WASM bridge compile database...\n'
cmake -S "$ROOT_DIR/packages/box3d-wasm/cmake" -B "$NATIVE_BRIDGE_BUILD_DIR" \
  -DBOX3D_SOURCE_DIR="$BOX3D_SOURCE_DIR" \
  -DBOX3D_SAMPLES=OFF \
  -DBOX3D_BENCHMARKS=OFF \
  -DBOX3D_DOCS=OFF \
  -DBOX3D_UNIT_TESTS=OFF \
  -DBOX3D_VALIDATE=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER="${CC:-clang}"

printf 'clangd compile databases ready:\n'
printf '  %s\n' "$REFERENCE_BUILD_DIR/compile_commands.json"
printf '  %s\n' "$NATIVE_BRIDGE_BUILD_DIR/compile_commands.json"
