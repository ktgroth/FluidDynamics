#!/bin/bash

CC="gcc"
LD="gcc"

CFLAGS="-O2 -Wall -Wextra -g"
LFLAGS="-lglfw -lGLEW -lGL -lfontconfig"

SRC=src
OBJ=obj
BUILD=build

SRCS=($(find "$SRC" -name "*.c" -o -path "$SRC" -name "*.c"))
OBJSK=()

for src in "${SRCS[@]}"; do
    obj="${OBJ}/$(basename "$src" .c).o"
    OBJSK+=("$obj")
done

OUTPUT=$BUILD/fd

function build {
    mkdir -p $OBJ
    mkdir -p $BUILD

    for i in "${!SRCS[@]}"; do
        src="${SRCS[$i]}"
        obj="${OBJSK[$i]}"
        comp $src $obj
    done

    $LD $LFLAGS "${OBJSK[@]}" -o $OUTPUT -lm
}

function comp {
    $CC $CFLAGS -c $1 -o $2
}

function clean {
    clear

    rm -rf $OBJ
    rm -rf $BUILD
}

for arg in "$@"; do
    if [ "$arg" = "build" ]; then
        build
    elif [ "$arg" = "clean" ]; then
        clean
    fi
done
