#!/bin/sh
APP=$*
LOG=$1-valgrind.log
valgrind \
    --verbose \
    --show-reachable=yes \
    --log-file=$LOG \
    --leak-check=full \
    --show-reachable=yes \
    --run-libc-freeres=no \
    $APP
