#!/usr/bin/env bash
set -euo pipefail

# Thin wrapper around the BugZapper submodule (tools/bugzapper): the GUI flasher
# + serial monitor, branded for ZIFRA and pointed at the firmware binary that
# ships in zifra/ (zifra.ino.esp8285.bin). Pass a folder to list a different set
# of .bin files. See tools/bugzapper/README.md for requirements (a python3 with
# tkinter; esptool is bundled).

DIR="$(cd "$(dirname "$0")" && pwd)"

export BUGZAPPER_TITLE="ZIFRA Flasher"
export BUGZAPPER_FW_DIR="$DIR/zifra"

exec "$DIR/tools/bugzapper/bugzapper.sh" "$@"
