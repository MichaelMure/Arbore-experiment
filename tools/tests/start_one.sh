#!/bin/bash

export TESTS_LOG="/tmp/p2pfs_tests.log"
export TESTS="$PWD"
export ROOT="$TESTS/../../"
export BINNAME="$1"
#export BINNAME=peerfuse-net
#export BINNAME=peerfuse-lan
export BIN="$TESTS/../../build.$1/$1"
#export TOOL=valgrind

if [ "$BINNAME" == "" ] || [ "$2" == "" ]
then
	echo "Syntax: ./start_one.sh <peerfuse-net/peerfuse-lan> <test_name>"
	exit 1
fi

if [ ! -x "$BIN" ]
then
	echo "Can't start $BIN"
	exit 1
fi

source bash/tests.sh

cd "$ROOT"


start_test "$2" || exit 1

