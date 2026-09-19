#!/usr/bin/env bash
# Compatibility entry point; package scripts use the cross-platform launcher.
set -euo pipefail
exec python3 "$(dirname "$0")/run-native-samples.py" "$@"
