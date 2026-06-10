#!/usr/bin/env bash
#
# Build the zifra firmware with arduino-cli.
#
# Usage:
#   ./build.sh            # release (DEBUG=0)
#   ./build.sh debug      # debug build with serial logging (DEBUG=1)
#   ./build.sh release    # release build (DEBUG=0), updates zifra/zifra.ino.esp8285.bin
#   ./build.sh all        # both
#
set -euo pipefail

FQBN="esp8266:esp8266:esp8285"
SKETCH_DIR="$(cd "$(dirname "$0")/zifra" && pwd)"

build() {
  local mode="$1" debug_value="$2"
  local out_dir="$SKETCH_DIR/build/$mode"

  echo "==> Building $mode (DEBUG=$debug_value)"
  arduino-cli compile \
    --fqbn "$FQBN" \
    --build-property "compiler.cpp.extra_flags=-DDEBUG=$debug_value" \
    --output-dir "$out_dir" \
    "$SKETCH_DIR"
  echo "==> $mode binary: $out_dir/zifra.ino.bin"

  # The release binary is shipped in the repo next to the sketch.
  if [ "$mode" = "release" ]; then
    cp "$out_dir/zifra.ino.bin" "$SKETCH_DIR/zifra.ino.esp8285.bin"
    echo "==> Updated $SKETCH_DIR/zifra.ino.esp8285.bin"
  fi
}

case "${1:-release}" in
  debug)   build debug 1 ;;
  release) build release 0 ;;
  all)     build release 0; build debug 1 ;;
  *)
    echo "Usage: $0 [debug|release|all]" >&2
    exit 1
    ;;
esac
