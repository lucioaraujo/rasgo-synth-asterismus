#!/usr/bin/env bash
# Verify the installed .deb (when present), AppImage and user-local tar install.
set -euo pipefail
PACKAGE_DIR=$(cd "${1:?package directory required}" && pwd -P)
(cd "$PACKAGE_DIR" && sha256sum -c SHA256SUMS)
ARCHIVE=$(find "$PACKAGE_DIR" -maxdepth 1 -type f -name '*.tar.gz' -print -quit)
APPIMAGE=$(find "$PACKAGE_DIR" -maxdepth 1 -type f -name '*.AppImage' -print -quit)
[[ -n "$ARCHIVE" && -n "$APPIMAGE" ]] || { echo 'missing Linux package format' >&2; exit 1; }
NAME=$(basename "$ARCHIVE" .tar.gz)
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
tar -xzf "$ARCHIVE" -C "$WORK"
PACKAGE="$WORK/$NAME"
PREFIX="$WORK/home/.local"
mkdir -p "$WORK/home/Music/Rasgo Synth Asterismus/recordings"
: > "$WORK/home/Music/Rasgo Synth Asterismus/recordings/keep.wav"
"$PACKAGE/install.sh" --prefix "$PREFIX"

xvfb-run -a -s '-screen 0 1600x1000x24' bash -c '
  set -euo pipefail
  package_dir=$1; appimage=$2; prefix=$3
  openbox >openbox.log 2>&1 & wm=$!; trap "kill $wm 2>/dev/null || true" EXIT; sleep 2
  check_window() {
    label="$1"; shift
    "$@" >"$label.log" 2>&1 & app=$!
    found=0
    for i in $(seq 1 30); do
      if wmctrl -l | grep -qi "Asterismus"; then found=1; break; fi
      if ! kill -0 "$app" 2>/dev/null; then break; fi
      sleep 1
    done
    test "$found" = 1 || { cat "$label.log"; echo "$label: no Asterismus window"; return 1; }
    sleep 20
    kill -0 "$app" || { cat "$label.log"; echo "$label: app exited"; return 1; }
    wmctrl -l | grep -qi "Asterismus"
    kill "$app"; wait "$app" 2>/dev/null || true
  }
  if test -x /usr/bin/rasgo-synth-asterismus; then check_window deb /usr/bin/rasgo-synth-asterismus; fi
  check_window appimage env APPIMAGE_EXTRACT_AND_RUN=1 "$appimage"
  check_window portable "$prefix/bin/rasgo-synth-asterismus"
' _ "$PACKAGE" "$APPIMAGE" "$PREFIX"

"$PACKAGE/install.sh" --remove --prefix "$PREFIX"
test -f "$WORK/home/Music/Rasgo Synth Asterismus/recordings/keep.wav"
