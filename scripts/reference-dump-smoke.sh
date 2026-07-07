#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${REFERENCE_DUMP_BUILD_DIR:-/tmp/reference-dump-build}"
OUT_DIR="${REFERENCE_DUMP_OUT_DIR:-/tmp/reference-dump-smoke}"

rm -rf "$BUILD_DIR" "$OUT_DIR"
mkdir -p "$BUILD_DIR" "$OUT_DIR"

cmake -S "$ROOT_DIR/tools/reference-dump" -B "$BUILD_DIR" -DBOX3D_DOUBLE_PRECISION=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build "$BUILD_DIR" -j"$(nproc)"

"$BUILD_DIR/reference-dump" --list-json > "$OUT_DIR/samples.json"
node -e "const fs=require('fs'); const d=JSON.parse(fs.readFileSync('$OUT_DIR/samples.json','utf8')); if (!d.samples.some(s => s.name === 'Single Box')) throw new Error('Single Box missing'); console.log('samples', d.samples.length)"

"$BUILD_DIR/reference-dump" --frames 0,50,100 "Single Box" "$OUT_DIR/single-box.json"
node -e "const fs=require('fs'); const d=JSON.parse(fs.readFileSync('$OUT_DIR/single-box.json','utf8')); const frames=d.checkpoints.map(c=>c.frame).join(','); if (frames !== '0,50,100') throw new Error('unexpected frames '+frames); if (d.checkpoints[0].bodies.length !== 2) throw new Error('unexpected body count'); console.log('single-box', frames, d.checkpoints[0].bodies.length)"
