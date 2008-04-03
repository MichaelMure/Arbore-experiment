#!/bin/bash

source "$TESTS/bash/tests.sh"

export START_ARGS=" "
start_p2pfs

mkdir "$MNT1/blah" || exit 1
rmdir "$MNT1/blah" || exit 1

if [ "$(ls -1 "$MNT1"|grep "blah")" == "blah" ]
then
	exit 1
fi

exit 0
