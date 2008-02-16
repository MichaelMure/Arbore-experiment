#!/bin/bash

source "$TESTS/bash/tests.sh"

export LISTEN_PORT=9876
export CONNECT_PORT=6789
start_p2pfs

export LISTEN_PORT=6789
export CONNECT_PORT=9876
start_p2pfs

mkdir -p "$MNT1/blh/aze"
mkdir -p "$MNT1/blh/ca"
touch "$MNT1/blh/plop"

# Trying create/remove
mkdir "$MNT2/truc/"
touch "$MNT2/truc/bidule"
rm "$MNT2/truc/bidule"
sleep 1
rm -r "$MNT1/truc"

sleep 3

find "$MNT1" | colrm 1 ${#MNT1} > /tmp/p2pfs_tests_find1
find "$MNT2" | colrm 1 ${#MNT2} > /tmp/p2pfs_tests_find2

if [ "$(diff /tmp/p2pfs_tests_find1 /tmp/p2pfs_tests_find2)" != "" ]
then
	exit 1
fi

exit 0
