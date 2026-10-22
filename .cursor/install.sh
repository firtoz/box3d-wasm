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
    python3 \
    unzip \
    xz-utils \
    curl \
    ca-certificates \
    git

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
export PATH="$HOME/.bun/bin:$PATH"

# Keep agent setup aligned with the runtime pin used by CI.
PINNED_BUN_VERSION="$(grep -oE '"packageManager"[[:space:]]*:[[:space:]]*"bun@[0-9.]+"' package.json | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' || true)"
if [ -z "$PINNED_BUN_VERSION" ]; then
    echo "❌ package.json must declare a numeric bun@ version."
    exit 1
fi

if ! command -v bun &> /dev/null || [ "$(bun --version)" != "$PINNED_BUN_VERSION" ]; then
    echo "📦 Installing Bun $PINNED_BUN_VERSION from packageManager..."
    curl -fsSL https://bun.sh/install | bash -s -- "bun-v$PINNED_BUN_VERSION"
    export PATH="$HOME/.bun/bin:$PATH"
fi

if [ "$(bun --version)" != "$PINNED_BUN_VERSION" ]; then
    echo "❌ Bun does not match the packageManager pin."
    exit 1
fi
echo "✅ Bun ready: $(bun --version)"


# Install the exact project Node when this base image has no matching runtime.
PINNED_NODE_VERSION="$(tr -d '[:space:]' < .nvmrc)"
[[ "$PINNED_NODE_VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] || exit 1
if [ "$(node --version 2>/dev/null || true)" != "v$PINNED_NODE_VERSION" ]; then
  case "$(uname -m)" in
    x86_64) node_arch=x64 ;;
    aarch64) node_arch=arm64 ;;
    *) echo "Unsupported Cloud Agent architecture" >&2; exit 1 ;;
  esac
  node_archive="node-v$PINNED_NODE_VERSION-linux-$node_arch.tar.xz"
  node_temp="$(mktemp -d)"
  trap 'rm -rf "$node_temp"' EXIT
  curl -fsSLo "$node_temp/$node_archive" "https://nodejs.org/dist/v$PINNED_NODE_VERSION/$node_archive"
  curl -fsSLo "$node_temp/SHASUMS256.txt" "https://nodejs.org/dist/v$PINNED_NODE_VERSION/SHASUMS256.txt"
  (cd "$node_temp" && grep "  $node_archive$" SHASUMS256.txt | sha256sum --check --status)
  node_home="$HOME/.local/share/box3d-node-$PINNED_NODE_VERSION"
  mkdir -p "$node_home" "$HOME/.bun/bin"
  tar -xJf "$node_temp/$node_archive" --strip-components=1 -C "$node_home"
  for tool in node npm npx; do ln -sfn "$node_home/bin/$tool" "$HOME/.bun/bin/$tool"; done
  rm -rf "$node_temp"
  trap - EXIT
fi
export PATH="$HOME/.bun/bin:$PATH"
[ "$(node --version)" = "v$PINNED_NODE_VERSION" ] || exit 1

# 2. box3d submodule. .gitmodules pins an SSH URL; rewrite it to HTTPS so the
#    public upstream engine can be fetched without SSH credentials.
git -c url."https://github.com/".insteadOf="git@github.com:" \
  submodule update --init --recursive

# 3. Workspace dependencies (Bun workspaces + Turborepo).
bun install --frozen-lockfile

# 4. Repo-local Emscripten SDK pin (installs into gitignored .emsdk/; WASM
#    builds source it automatically).
bun run setup:emsdk

# 5. Build the release WASM binary so the demo has artifacts on first boot.
(cd packages/box3d-wasm && bun run build:wasm)

# 6. Prove the C++ reference dumper links (catches missing libstdc++ early).
bun run test:reference-dump

echo "[setup] done"
