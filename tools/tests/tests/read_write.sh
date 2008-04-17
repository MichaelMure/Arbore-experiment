#!/bin/bash
source "$TESTS/bash/tests.sh"

start_p2pfs

TMPFILE="$(mktemp)"

# Generate random data
dd if=/dev/urandom of="$TMPFILE" bs=1024 count=4
cp "$TMPFILE" "$MNT1/tmp"

if [ "$(cat "$TMPFILE"|md5sum)" != "$(cat "$MNT1/tmp"|md5sum)" ]
then
	exit 1
fi

