#!/usr/bin/env bash
#
# Build the zifra firmware with arduino-cli, for the stock ESP8285 board
# and for generic ESP8266 modules (1M flash layout, e.g. ESP-01S/ESP-12).
#
# Usage:
#   ./build.sh            # release builds (DEBUG=0)
#   ./build.sh debug      # debug builds with serial logging (DEBUG=1)
#   ./build.sh release    # release builds, updates zifra/zifra.ino.esp8285.bin
#   ./build.sh all        # release and debug builds
#
set -euo pipefail

REPO_DIR="$(cd "$(dirname "$0")" && pwd)"
SKETCH_DIR="$REPO_DIR/zifra"
LIBRARIES_DIR="$REPO_DIR/libraries" # vendored copies of all used libraries

BOARDS="esp8285 esp8266_generic"

fqbn_for_board() {
  case "$1" in
    esp8285)         echo "esp8266:esp8266:esp8285" ;;
    esp8266_generic) echo "esp8266:esp8266:generic:eesz=1M64" ;;
    *)
      echo "Unknown board: $1" >&2
      exit 1
      ;;
  esac
}

build() {
  local board="$1" mode="$2" debug_value="$3"
  local out_dir="$SKETCH_DIR/build/$board/$mode"

  echo "==> Building $board $mode (DEBUG=$debug_value)"
  arduino-cli compile \
    --fqbn "$(fqbn_for_board "$board")" \
    --libraries "$LIBRARIES_DIR" \
    --build-property "compiler.cpp.extra_flags=-DDEBUG=$debug_value" \
    --output-dir "$out_dir" \
    "$SKETCH_DIR"
  echo "==> $board $mode binary: $out_dir/zifra.ino.bin"

  # The ESP8285 release binary is shipped in the repo next to the sketch.
  if [ "$board" = "esp8285" ] && [ "$mode" = "release" ]; then
    cp "$out_dir/zifra.ino.bin" "$SKETCH_DIR/zifra.ino.esp8285.bin"
    echo "==> Updated $SKETCH_DIR/zifra.ino.esp8285.bin"
  fi
}

build_mode() {
  local mode="$1" debug_value="$2" board
  for board in $BOARDS; do
    build "$board" "$mode" "$debug_value"
  done
}

case "${1:-release}" in
  debug)   build_mode debug 1 ;;
  release) build_mode release 0 ;;
  all)     build_mode release 0; build_mode debug 1 ;;
  *)
    echo "Usage: $0 [debug|release|all]" >&2
    exit 1
    ;;
esac
