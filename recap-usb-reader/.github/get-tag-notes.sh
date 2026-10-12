#!/bin/sh

set -e

tagname=$1
[ -n "$tagname" ] || (echo "Usage: $0 <tagname>" && false)

tagtype=$(git for-each-ref --format='%(objecttype)' -- "refs/tags/$tagname")
[ -n "$tagtype" ] || (echo "Tag $tagname not found" && false)

if [ "$tagtype" = "tag" ]; then
    # Annotated tag
    git tag -l --format='%(contents)' -- "$tagname"
fi
