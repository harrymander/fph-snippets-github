#!/bin/sh

REPO_DIR=$(realpath -- "$(dirname -- "$0")/..")

if [ -z "$1" ]; then
    set -- /bin/bash
fi

docker run --rm -it \
    --volume "$REPO_DIR":/src \
    --user "$(id -u):$(id -g)" \
    --env COMPILE_COMMANDS_PATH_RENAME="/src:$REPO_DIR" \
    --workdir /src/sw \
    "${RECAP_DEV_CONTAINER:-recap-build:latest}" "$@"
