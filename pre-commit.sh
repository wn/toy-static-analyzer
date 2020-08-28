#!/bin/bash
if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    exit 1
fi

SRC="Team00/Code00/src/spa/src/*"

case "$1" in
        -f) clang-format -i -style=file $SRC
            ;;
        -fd) diff -u <(cat $SRC) <(clang-format $SRC)
            ;;
        --*) echo "bad option $1"
            ;;
        *) echo "argument $1"
            ;;
esac
