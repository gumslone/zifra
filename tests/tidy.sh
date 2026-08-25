#!/usr/bin/env bash
set -euo pipefail

# Runs clang-tidy (config in .clang-tidy at the repo root) over the host-test
# translation units, which pull in the firmware's pure-logic headers.
#
# Usage: ./tests/tidy.sh

DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$DIR/.." && pwd)"

TIDY="${CLANG_TIDY:-}"
if [ -z "$TIDY" ]; then
  for cand in clang-tidy /opt/homebrew/opt/llvm/bin/clang-tidy \
              /usr/local/opt/llvm/bin/clang-tidy; do
    if command -v "$cand" >/dev/null 2>&1; then
      TIDY="$cand"
      break
    fi
  done
fi
if [ -z "$TIDY" ]; then
  echo "Error: clang-tidy not found (brew install llvm / apt install clang-tidy)" >&2
  exit 1
fi

FLAGS=(-std=c++17 -I"$DIR/shims" -I"$DIR" -I"$ROOT/zifra")
# On macOS clang-tidy needs the SDK path to find libc++ headers.
if [ "$(uname)" = "Darwin" ] && command -v xcrun >/dev/null 2>&1; then
  FLAGS+=(-isysroot "$(xcrun --show-sdk-path)")
fi

echo "==> $($TIDY --version | head -n1)"
"$TIDY" --quiet --warnings-as-errors='*' "$DIR"/test_*.cpp -- "${FLAGS[@]}"
echo "==> clang-tidy clean"
