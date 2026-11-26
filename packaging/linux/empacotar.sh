#!/usr/bin/env bash
# Build AppImage and installable user-local tar.gz from the Ubuntu 22.04 build.
# Usage: empacotar.sh <build-dir> <deb-dir> <output-dir> <linuxdeploy> <runtime> <license>
set -euo pipefail
BUILD=${1:?missing build directory}
DEB_DIR=${2:?missing deb directory}
OUTPUT=${3:?missing output directory}
LINUXDEPLOY=$(realpath "${4:?missing pinned linuxdeploy}")
RUNTIME=$(realpath "${5:?missing pinned AppImage runtime}")
RUNTIME_LICENSE=$(realpath "${6:?missing runtime license}")
ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)
VERSION=$(sed -n 's/^CMAKE_PROJECT_VERSION:STATIC=//p' "$BUILD/CMakeCache.txt")
[[ -n "$VERSION" ]] || { echo "version not found in CMakeCache" >&2; exit 1; }
[[ "$(uname -m)" == x86_64 ]] || { echo "only x86_64 is supported" >&2; exit 1; }
NAME="rasgo-synth-asterismus-${VERSION}-linux-x86_64"
mkdir -p "$OUTPUT"
OUTPUT=$(cd "$OUTPUT" && pwd -P)
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

APPDIR="$WORK/AppDir"
cmake --install "$BUILD" --config Release --prefix "$APPDIR/usr"
BIN="$APPDIR/usr/bin/rasgo-synth-asterismus"
DESKTOP="$APPDIR/usr/share/applications/rasgo-synth-asterismus.desktop"
ICON="$APPDIR/usr/share/icons/hicolor/512x512/apps/rasgo-synth-asterismus.png"
test -x "$BIN" && test -f "$DESKTOP" && test -f "$ICON"

PORTABLE="$WORK/$NAME"
mkdir -p "$PORTABLE"
cp -a "$APPDIR/usr" "$PORTABLE/usr"
install -m 755 "$ROOT/packaging/linux/install.sh" "$PORTABLE/install.sh"
install -m 644 "$ROOT/packaging/linux/install_portable.py" "$PORTABLE/install_portable.py"
cat >"$PORTABLE/README.txt" <<EOF
Rasgo Synth Asterismus $VERSION — Linux x86_64

Install for this user, without root: ./install.sh
Remove this installation: ./install.sh --remove
The archive also contains the app in ./usr/bin/rasgo-synth-asterismus.
Sessions and recordings remain in the user's music folder.
Installation guide: https://github.com/lucioaraujo/rasgo-synth-asterismus/blob/v$VERSION/INSTALL.md
EOF
(cd "$PORTABLE" && find usr -type f -print0 | sort -z | xargs -0 sha256sum > SHA256SUMS)
tar --sort=name --owner=0 --group=0 --numeric-owner --mtime='UTC 1970-01-01' \
    -czf "$OUTPUT/$NAME.tar.gz" -C "$WORK" "$NAME"

(cd "$ROOT/packaging/linux/appimage/runtime" && sha256sum -c SHA256SUMS)
test "$(sha256sum "$LINUXDEPLOY" | cut -d ' ' -f 1)" = \
    c20cd71e3a4e3b80c3483cef793cda3f4e990aca14014d23c544ca3ce1270b4d
chmod +x "$LINUXDEPLOY"
install -D -m 644 "$RUNTIME_LICENSE" "$APPDIR/usr/share/doc/rasgo-synth-asterismus/appimage-runtime/LICENSE"
export APPIMAGE_EXTRACT_AND_RUN=1 ARCH=x86_64 LDAI_OUTPUT="$OUTPUT/$NAME.AppImage"
export LDAI_RUNTIME_FILE="$RUNTIME"
(cd "$WORK" && "$LINUXDEPLOY" --appdir "$APPDIR" --executable "$BIN" \
    --desktop-file "$DESKTOP" --icon-file "$ICON" --output appimage)
chmod +x "$OUTPUT/$NAME.AppImage"
cp "$DEB_DIR"/*.deb "$OUTPUT/$NAME.deb"
(cd "$OUTPUT" && sha256sum "$NAME.deb" "$NAME.AppImage" "$NAME.tar.gz" > SHA256SUMS)
(cd "$OUTPUT" && sha256sum -c SHA256SUMS)
