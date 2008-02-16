#!/bin/bash

source "$TESTS/bash/tests.sh"

start_p2pfs

rm "$MNT1/blah" && exit 1

exit 0
