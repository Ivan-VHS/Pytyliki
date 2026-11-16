#!/usr/bin/env sh
set -eu
TARGET="${1:-}"
OUT_DIR="${2:-build/cross}"
case "$TARGET" in
  aarch64) CC="${AARCH64_CC:-aarch64-linux-gnu-gcc}" ;;
  riscv64) CC="${RISCV64_CC:-riscv64-linux-gnu-gcc}" ;;
  *) echo "Supported targets: aarch64, riscv64" >&2; exit 2 ;;
esac
if ! command -v "$CC" >/dev/null 2>&1; then
  echo "Cross compiler not found: $CC" >&2
  exit 127
fi
mkdir -p "$OUT_DIR/$TARGET"
COMMON="-std=c11 -O3 -Wall -Wextra -Wpedantic -Werror -I src"
# Compile only: cross-target binaries are never executed on the host.
# Assembly output is retained so reviewers can inspect compiler lowering.
# shellcheck disable=SC2086
"$CC" $COMMON -c src/core.c -o "$OUT_DIR/$TARGET/core.o"
# shellcheck disable=SC2086
"$CC" $COMMON -S src/core.c -o "$OUT_DIR/$TARGET/core.s"
# shellcheck disable=SC2086
"$CC" $COMMON -fsyntax-only src/cli.c
printf 'Cross-ISA compile passed: %s (%s)\n' "$TARGET" "$CC"
