#!/usr/bin/env bash
set -euo pipefail

# Compiles and runs the native host tests (no board, no Arduino toolchain
# needed — the shims in tests/shims stand in for the Arduino API).
#
# Usage: ./tests/run.sh

DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$DIR/.." && pwd)"
CXX="${CXX:-g++}"
FLAGS=(-std=c++17 -Wall -Wextra -Werror -I"$DIR/shims" -I"$DIR" -I"$ROOT/zifra/src")

mkdir -p "$DIR/.build"
status=0
for src in "$DIR"/test_*.cpp; do
  bin="$DIR/.build/$(basename "${src%.cpp}")"
  echo "==> $(basename "$src")"
  "$CXX" "${FLAGS[@]}" "$src" -o "$bin"
  "$bin" || status=1
done

if [ "$status" -eq 0 ]; then
  echo "==> All test suites passed"
else
  echo "==> TEST FAILURES" >&2
fi
exit $status
