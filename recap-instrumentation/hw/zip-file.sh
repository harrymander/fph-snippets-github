#!/bin/sh

DIR=$1

if [ -z "$DIR" ]; then
    2>&1 echo "usage: $0 [dir]"
fi

git ls-files -z "$DIR" | xargs -0 zip "$(basename "$DIR")-$(git rev-parse HEAD).zip"
