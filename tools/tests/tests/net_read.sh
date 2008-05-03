#!/bin/bash
source "$TESTS/bash/tests.sh"

LISTEN_PORT=9000
CONNECT_PORT=8999
start_p2pfs

LISTEN_PORT=9001
CONNECT_PORT=9000
start_p2pfs

LISTEN_PORT=9002
CONNECT_PORT=9001
start_p2pfs

sleep 10s

TMPFILE="$(mktemp)"

# Generate random data
dd if=/dev/urandom of="$TMPFILE" bs=1024 count=4
cp "$TMPFILE" "$MNT1/tmp"

sleep 5

## Check local read
if [ "$(cat "$TMPFILE"|md5sum)" != "$(cat "$MNT1/tmp"|md5sum)" ]
then
	rm "$TMPFILE"
	exit 1
fi
## Check A->B transfert
if [ "$(cat "$TMPFILE"|md5sum)" != "$(cat "$MNT2/tmp"|md5sum)" ]
then
	rm "$TMPFILE"
	exit 1
fi
## Check A+B->C transfert
if [ "$(cat "$TMPFILE"|md5sum)" != "$(cat "$MNT3/tmp"|md5sum)" ]
then
	rm "$TMPFILE"
	exit 1
fi
rm "$TMPFILE"

