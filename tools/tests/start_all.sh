#!/bin/bash

export TESTS_LOG="/tmp/p2pfs_tests.log"
export TESTS="$PWD"
export ROOT="$TESTS/../../"
export BINNAME="$1"
#export BINNAME=peerfuse-net
#export BINNAME=peerfuse-lan
export BIN="$TESTS/../../build.$1/$1"
#export TOOL=valgrind

if [ ! -x "$BIN" ] || [ "$BINNAME" == "" ]
then
	echo "Syntax: ./start_all.sh <peerfuse-net/peerfuse-lan>"
	exit 1
fi

source bash/tests.sh

cd "$ROOT"

for test in $(grep -v '^#\|^$' "$TESTS/$BINNAME.conf")
do
	start_test "$test" || exit 1
done

