#!/bin/bash

source "$TESTS/bash/tests.sh"

start_p2pfs

mkdir "$MNT1/blah"
rmdir "$MNT1/blah"

if [ "$(ls -1 "$MNT1"|grep "blah")" == "blah" ]
then
	exit 1
fi

exit 0
