#!/usr/bin/env bash
# Linux C metadata regressions, with ASan/UBSan and actual implementation paths.
set -euo pipefail
cd "$(dirname "$0")/.."
OUT="${1:-artifacts/native-metadata}"
mkdir -p "$OUT"
CC_BIN="${CC:-cc}"
FLAGS=(-std=gnu11 -g -fsanitize=address,undefined -ffunction-sections -fdata-sections -I ../../box3d/include -I c_abi)
"$CC_BIN" "${FLAGS[@]}" -Wall -Wextra -Werror c_abi/growable_slots_test.c c_abi/both_map.c -o "$OUT/slots"
"$CC_BIN" "${FLAGS[@]}" c_abi/geometry_slots_test.c -Wl,--gc-sections -o "$OUT/geometry"
# Build the existing CPU oracle first; only hull/math code is pulled into this test.
"$CC_BIN" "${FLAGS[@]}" c_abi/visual_slots_test.c oracle/build/box3d-build/src/libbox3d.a -Wl,--gc-sections -lm -o "$OUT/visual"
"$OUT/slots"
"$OUT/geometry"
"$OUT/visual"
