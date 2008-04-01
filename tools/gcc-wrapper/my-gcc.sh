#!/bin/bash
../tools/gcc-wrapper/header_check.py "$@" || exit 1
../tools/gcc-wrapper/header_loop.py "$@"
exec "$@"
