#!/usr/bin/env bash
set -euo pipefail

# Thin wrapper around the BugZapper submodule (tools/bugzapper): the CLI flasher.
# Defaults to the ESP8285 binary that ships in zifra/ when no -f is given, so
# `./flash.sh` just works. All of BugZapper's flags pass through unchanged; run
# `./flash.sh -h` for the full list.

DIR="$(cd "$(dirname "$0")" && pwd)"
DEFAULT_FW="$DIR/zifra/zifra.ino.esp8285.bin"

# If the caller didn't pick a firmware (-f), default to the shipped binary.
# (-l/-h still short-circuit inside flash.sh, so prepending -f is harmless.)
args=()
has_f=0
for a in "$@"; do [ "$a" = "-f" ] && has_f=1; done
if [ "$has_f" -eq 0 ] && [ -f "$DEFAULT_FW" ]; then
  args=(-f "$DEFAULT_FW")
fi

exec "$DIR/tools/bugzapper/flash.sh" "${args[@]}" "$@"
