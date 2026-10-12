#!/usr/bin/env bash

set -e

function log {
     2>&1 echo "$@"
}

if ! [[
    -n "$APPIMAGE_STAGING" && -n "$APPIMAGETOOL" && -n "$APPIMAGE_OUTPUT" && -n "$APPIMAGETOOL"
]]; then
    log "APPIMAGE_STAGING, APPIMAGETOOL and APPIMAGE_OUTPUT must be set"
    exit 1
fi

appdir=$(mktemp -d)
trap 'rm -rf "$appdir"' EXIT

cp -r "$APPIMAGE_STAGING" "$appdir/usr"
ln -s usr/bin/recapctl-gui "$appdir/AppRun"
ln -s usr/share/applications/recapctl-gui.desktop "$appdir/recapctl-gui.desktop"
ln -s usr/share/icons/hicolor/scalable/apps/recapctl-gui.svg "$appdir/recapctl-gui.svg"
ln -s recapctl-gui.svg "$appdir/.DirIcon"

"$APPIMAGETOOL" "$appdir" "$APPIMAGE_OUTPUT"
