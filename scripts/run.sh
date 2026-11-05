#!/usr/bin/env bash
set -euo pipefail
AURORA_SOURCE_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
AURORA_BUILD_DIR="${AURORA_BUILD_DIR:-$AURORA_SOURCE_DIR/../player/build/aurora}"
if [[ ! -x "$AURORA_BUILD_DIR/AuroraPlayer" ]]; then
    cmake -S "$AURORA_SOURCE_DIR" -B "$AURORA_BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
    cmake --build "$AURORA_BUILD_DIR" --parallel 4
fi
exec "$AURORA_BUILD_DIR/AuroraPlayer" "$@"
