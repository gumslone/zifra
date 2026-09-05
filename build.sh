#!/usr/bin/env bash
#
# Build the zifra firmware with arduino-cli, for the stock ESP8285 board
# and for generic ESP8266 modules (1M flash layout, e.g. ESP-01S/ESP-12).
#
# Usage:
#   ./build.sh            # release builds (DEBUG=0)
#   ./build.sh debug      # debug builds with serial logging (DEBUG=1)
#   ./build.sh release    # release builds, updates firmware/zifra_esp8285.bin
#   ./build.sh all        # release and debug builds
#
set -euo pipefail

REPO_DIR="$(cd "$(dirname "$0")" && pwd)"
SKETCH_DIR="$REPO_DIR/zifra"
LIBRARIES_DIR="$REPO_DIR/libraries" # vendored copies of all used libraries

BOARDS="esp8285 esp8266_generic"

# 117 flash sectors: half of the 1MB layout's sketch area (0xEB000 bytes),
# rounded down to a sector - the largest image that can still OTA itself.
OTA_MAX_BYTES=479232

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
  local out_dir="$REPO_DIR/.build/$board/$mode"

  echo "==> Building $board $mode (DEBUG=$debug_value)"
  arduino-cli compile \
    --fqbn "$(fqbn_for_board "$board")" \
    --libraries "$LIBRARIES_DIR" \
    --build-property "compiler.cpp.extra_flags=-DDEBUG=$debug_value -DWM_NODEBUG" \
    --output-dir "$out_dir" \
    "$SKETCH_DIR"
  echo "==> $board $mode binary: $out_dir/zifra.ino.bin"

  # On the 1MB flash layout an OTA image has to fit above the running
  # sketch, so a firmware bigger than half the sketch area could never be
  # updated over WiFi again. Refuse to build such a release.
  local size
  size=$(wc -c < "$out_dir/zifra.ino.bin" | tr -d ' ')
  echo "==> $board $mode size: $size bytes (OTA ceiling $OTA_MAX_BYTES)"
  if [ "$mode" = "release" ] && [ "$size" -gt "$OTA_MAX_BYTES" ]; then
    echo "ERROR: $board release binary exceeds the OTA ceiling by $((size - OTA_MAX_BYTES)) bytes" >&2
    exit 1
  fi

  # The ESP8285 release binary is shipped in the repo under firmware/.
  if [ "$board" = "esp8285" ] && [ "$mode" = "release" ]; then
    cp "$out_dir/zifra.ino.bin" "$REPO_DIR/firmware/zifra_esp8285.bin"
    echo "==> Updated $REPO_DIR/firmware/zifra_esp8285.bin"
  fi
}

build_mode() {
  local mode="$1" debug_value="$2" board
  for board in $BOARDS; do
    build "$board" "$mode" "$debug_value"
  done
}

# The clock serves the web app itself: refresh the embedded, gzipped copy
# so a build never ships a stale UI.
python3 "$REPO_DIR/tools/web/gzip_pages.py"

case "${1:-release}" in
  debug)   build_mode debug 1 ;;
  release) build_mode release 0 ;;
  all)     build_mode release 0; build_mode debug 1 ;;
  *)
    echo "Usage: $0 [debug|release|all]" >&2
    exit 1
    ;;
esac
