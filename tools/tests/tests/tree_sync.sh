#!/bin/bash

source "$TESTS/bash/tests.sh"

export LISTEN_PORT=9876
export CONNECT_PORT=6789
start_p2pfs
mkdir -p "$MNT1/blah/sdf"
mkdir -p "$MNT1/blah/ssdfdf"
mkdir "$MNT1/blh/"
rmdir "$MNT1/blh/"
mkdir "$MNT1/blah2/"

export LISTEN_PORT=6789
export CONNECT_PORT=9876
start_p2pfs

sleep 5

find "$MNT1" | colrm 1 ${#MNT1} > /tmp/p2pfs_tests_find1
find "$MNT2" | colrm 1 ${#MNT2} > /tmp/p2pfs_tests_find2

if [ "$(diff /tmp/p2pfs_tests_find1 /tmp/p2pfs_tests_find2)" != "" ]
then
	exit 1
fi

exit 0
