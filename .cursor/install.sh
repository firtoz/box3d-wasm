#!/usr/bin/env bash
# Idempotent Cloud Agent setup for the box3d-wasm workspace.
# Prepares Bun, the box3d submodule, workspace deps, the pinned Emscripten SDK,
# and a release WASM build so the demo is ready to run.
set -euo pipefail

cd "$(dirname "$0")/.."

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
#    builds source it automatically). No system packages are touched.
bun run setup:emsdk

# 5. Build the release WASM binary so the demo has artifacts on first boot.
(cd packages/box3d-wasm && bun run build:wasm)

echo "[setup] done"
