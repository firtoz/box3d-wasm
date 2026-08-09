#!/usr/bin/env bash
# Idempotent Cloud Agent setup for the box3d-wasm workspace.
# Prepares Bun, the box3d submodule, workspace deps, the pinned Emscripten SDK,
# and a release WASM build so the demo is ready to run.
#
# Native C++ toolchain packages (cmake/clang/g++/libstdc++) belong in
# .cursor/Dockerfile. This script still repairs a broken clang++ link on
# JIT/default images where the Dockerfile has not been baked yet.
set -euo pipefail

cd "$(dirname "$0")/.."

ensure_native_cxx() {
  local test_bin
  test_bin="$(mktemp /tmp/box3d-cxx-link-XXXXXX)"
  if echo 'int main(){return 0;}' | clang++ -x c++ - -o "$test_bin" 2>/dev/null; then
    rm -f "$test_bin"
    echo "[setup] clang++ links libstdc++ OK ($(clang++ --version | head -n1))"
    return 0
  fi
  rm -f "$test_bin"

  echo "[setup] clang++ cannot link libstdc++; installing native C++ toolchain"
  sudo apt-get update -qq
  sudo DEBIAN_FRONTEND=noninteractive apt-get install -y -qq --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    clang \
    g++ \
    libstdc++-14-dev \
    pkg-config \
    python3

  test_bin="$(mktemp /tmp/box3d-cxx-link-XXXXXX)"
  if ! echo 'int main(){return 0;}' | clang++ -x c++ - -o "$test_bin"; then
    echo "[setup] ERROR: clang++ still cannot link after apt install" >&2
    exit 1
  fi
  rm -f "$test_bin"
  echo "[setup] clang++ links libstdc++ OK after apt install"
}

# 0. Native toolchain for tools/reference-dump + compare:sample.
ensure_native_cxx
cmake --version | head -n1

# 1. Bun (pinned to the version in package.json "packageManager").
if ! command -v bun >/dev/null 2>&1; then
  echo "[setup] installing Bun 1.2.0"
  curl -fsSL https://bun.sh/install | bash -s "bun-v1.2.0"
fi
export PATH="$HOME/.bun/bin:$PATH"
bun --version

# 2. box3d submodule. .gitmodules pins an SSH URL; rewrite it to HTTPS so the
#    public upstream engine can be fetched without SSH credentials.
git -c url."https://github.com/".insteadOf="git@github.com:" \
  submodule update --init --recursive

# 3. Workspace dependencies (Bun workspaces + Turborepo).
bun install

# 4. Repo-local Emscripten SDK pin (installs into gitignored .emsdk/; WASM
#    builds source it automatically).
bun run setup:emsdk

# 5. Build the release WASM binary so the demo has artifacts on first boot.
(cd packages/box3d-wasm && bun run build:wasm)

# 6. Prove the C++ reference dumper links (catches missing libstdc++ early).
bun run test:reference-dump

echo "[setup] done"
