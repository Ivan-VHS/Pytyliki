#!/usr/bin/env bash
set -euo pipefail

export CC="${E2K_CC:-${CC:-gcc}}"
export CXX="${E2K_CXX:-${CXX:-g++}}"
BUILD_DIR="${BUILD_DIR:-build-elbrus}"

if ! command -v "$CC" >/dev/null 2>&1; then
  echo "E2K C compiler not found: $CC" >&2
  exit 2
fi
if ! printf '\n' | "$CC" -dM -E -x c - 2>/dev/null | grep -Eq '^#define[[:space:]]+__e2k__([[:space:]]|$)'; then
  echo "$CC does not define __e2k__; refusing to label a non-E2K build as native E2K." >&2
  exit 2
fi

cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build "$BUILD_DIR" --parallel
ctest --test-dir "$BUILD_DIR" --output-on-failure
