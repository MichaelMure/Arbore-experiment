#!/bin/bash

source "$TESTS/bash/tests.sh"

start_p2pfs
sleep 2

## Generate a random file inside of the cache
dd if=/dev/urandom of="$MNT1/tmp" bs=1024 count=4
MD5="$(cat "$MNT1/tmp"|md5sum)"

## Unmount the fs to force writing datas to hdd
fusermount -u "$MNT1"
sleep 2

## Check the file
if [ "$(cat "$CACHE1/tmp"|md5sum)" != "$MD5" ]
then
	exit 1
fi

