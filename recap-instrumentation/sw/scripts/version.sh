#!/bin/bash

vstring=$(git describe --tags --always --dirty --long)

case "$vstring" in
    v*)
        echo "${vstring:1}"
        ;;
    *)
        1>&2 echo "Version string must start with 'v': $vstring"
        exit 1
        ;;
esac
