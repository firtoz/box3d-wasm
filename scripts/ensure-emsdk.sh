#!/usr/bin/env bash
# Install / activate a repo-local Emscripten SDK pin (does not touch system packages).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VERSION_FILE="$ROOT/tools/emsdk-version"
EMSDK_DIR="${BOX3D_EMSDK_DIR:-$ROOT/.emsdk}"
VERSION="${BOX3D_EMSDK_VERSION:-}"
if [[ -z "$VERSION" && -f "$VERSION_FILE" ]]; then
  VERSION="$(tr -d '[:space:]' < "$VERSION_FILE")"
fi
VERSION="${VERSION:-6.0.2}"

echo "[emsdk] root: $EMSDK_DIR"
echo "[emsdk] version: $VERSION"

if [[ ! -d "$EMSDK_DIR/.git" ]]; then
  echo "[emsdk] cloning emscripten-core/emsdk..."
  rm -rf "$EMSDK_DIR"
  git clone --depth 1 https://github.com/emscripten-core/emsdk.git "$EMSDK_DIR"
fi

cd "$EMSDK_DIR"
./emsdk install "$VERSION"
./emsdk activate "$VERSION"

# Sanity check without permanently mutating the caller's shell.
# shellcheck disable=SC1091
source ./emsdk_env.sh >/dev/null
echo "[emsdk] ready: $(emcc -v 2>&1 | head -n 1)"
echo "[emsdk] next: bun run build (or bun run dev) will prefer this toolchain automatically"
