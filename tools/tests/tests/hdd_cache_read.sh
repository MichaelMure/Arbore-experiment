#!/bin/bash

source "$TESTS/bash/tests.sh"

## Generate a random file inside of the cache
mkdir -p "$WORKDIR/fs"
dd if=/dev/urandom of="$WORKDIR/fs/tmp" bs=1024 count=4
## Provide the correct entry in tree.cfg
mkdir -p "$WORKDIR/confdir1"
echo "/tmp#size=4096" > "$WORKDIR/confdir1/tree.cfg"

## Start a fs using the cache previously filled
export CACHE_ROOT="$WORKDIR/fs"
start_p2pfs

## Check the file
if [ "$(cat "$WORKDIR/fs/tmp"|md5sum)" != "$(cat "$MNT1/tmp" |md5sum)" ]
then
	exit 1
fi

