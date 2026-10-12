#!/usr/bin/env bash

set -e

die() {
  2>&1 echo "$1"
  exit 1
}

THISDIR=$(dirname -- "$0")

if ! [[ -n "$RELEASE_TAG" && -n "$GIT_SHA" ]]; then
  die "RELEASE_TAG and GIT_SHA must be defined"
fi

WINDOWS_FILENAME=$(find . -maxdepth 1 -type f -name "recap-usb-reader*.exe" | head -n1)
if [[ ! -f "$WINDOWS_FILENAME" ]]; then
  die "No USB reader executable found in current directory"
fi

RELEASE_NOTES_FILE=$(mktemp)

trap 'rm -rf $RELEASE_NOTES_FILE' EXIT

cat > "${RELEASE_NOTES_FILE}" << EOF
# RECAP USB Reader ${RELEASE_TAG}

Commit for this release: harrymander/recap-usb-reader@${GIT_SHA}
EOF

TAG_NOTES=$("${THISDIR}/get-tag-notes.sh" "$RELEASE_TAG")
if [ -n "$TAG_NOTES" ]; then
  cat >> "${RELEASE_NOTES_FILE}" << EOF

## Release notes

${TAG_NOTES}
EOF
fi

RELEASE_TITLE="${RELEASE_TAG} ($(date +'%Y-%m-%d %T %Z'))"

2>&1 echo "Release title: ${RELEASE_TITLE}"
2>&1 echo "Release notes:"
2>&1 cat "${RELEASE_NOTES_FILE}"

gh release create \
  --latest \
  --title "${RELEASE_TITLE}" \
  --notes-file "${RELEASE_NOTES_FILE}" \
  "${RELEASE_TAG}" \
  "${WINDOWS_FILENAME}"
