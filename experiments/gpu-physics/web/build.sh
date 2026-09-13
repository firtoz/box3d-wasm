#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
rustup target add wasm32-unknown-unknown >/dev/null
cargo build --release --target wasm32-unknown-unknown
mkdir -p web/pkg
wasm-bindgen --target web --out-dir web/pkg \
  target/wasm32-unknown-unknown/release/gpu_physics.wasm
echo "serve: bun run compare   # Vite on :8766 — compare grid at /"
