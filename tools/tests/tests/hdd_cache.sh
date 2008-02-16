#!/bin/bash

source "$TESTS/bash/tests.sh"

start_p2pfs

mkdir -p "$MNT1/blah/sdf"
mkdir -p "$MNT1/blh/sf"
mkdir -p "$MNT1/blh/sf2"
rmdir "$MNT1/blh/sf"
touch "$MNT1/blh/s"
touch "$MNT1/blh/d"
rm "$MNT1/blh/s"

find "$MNT1" | sort | colrm 1 ${#MNT1} > /tmp/p2pfs_tests_find1
find "$CACHE1" | sort | colrm 1 ${#CACHE1} > /tmp/p2pfs_tests_find2

if [ "$(diff /tmp/p2pfs_tests_find1 /tmp/p2pfs_tests_find2)" != "" ]
then
	exit 1
fi

exit 0
