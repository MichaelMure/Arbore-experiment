#!/bin/bash

source "$TESTS/bash/tests.sh"

export LISTEN_PORT=9876
export CONNECT_PORT=6789
start_p2pfs

export LISTEN_PORT=6789
export CONNECT_PORT=9876
start_p2pfs

mkdir -p "$MNT1/blh/aze"
mkdir -p "$MNT2/blh/ca"
touch "$MNT1/blh/plop"

export LISTEN_PORT=1414
export CONNECT_PORT=9876
start_p2pfs

sleep 3

rmdir "$MNT3/blh/ca"
touch "$MNT1/yop"

sleep 2

find "$MNT1" | colrm 1 ${#MNT1} > /tmp/p2pfs_tests_find1
find "$MNT3" | colrm 1 ${#MNT3} > /tmp/p2pfs_tests_find2

if [ "$(diff /tmp/p2pfs_tests_find1 /tmp/p2pfs_tests_find2)" != "" ]
then
	exit 1
fi

exit 0
