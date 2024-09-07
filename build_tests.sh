#!/bin/sh

FLAGS="-g -std=c99 -pedantic -Wall -Wextra"

set -xe
cc -o tests tests.c $FLAGS
set +xe
echo

if [ "$1" = "run" ]; then
    ./tests
fi
