#!/bin/bash

source "$TESTS/bash/tests.sh"


mkdir -p "$WORKDIR/fs/blah/sdf"
mkdir -p "$WORKDIR/fs/blh/sf"
mkdir -p "$WORKDIR/fs/blh/sf2"
touch "$WORKDIR/fs/blh/s"
touch "$WORKDIR/fs/blh/d"

find "$WORKDIR/fs" | sort | colrm 1 $(echo -n "$WORKDIR/fs"|wc -c) > /tmp/p2pfs_tests_find1

# Start an other fs using the same cache
export CACHE_ROOT="$WORKDIR/fs"
start_p2pfs

# Check files have been reloaded successfully
find "$MNT1" | sort | colrm 1 ${#MNT1} > /tmp/p2pfs_tests_find2

if [ "$(diff /tmp/p2pfs_tests_find1 /tmp/p2pfs_tests_find2)" != "" ]
then
	exit 1
fi

exit 0
