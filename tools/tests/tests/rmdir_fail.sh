#!/bin/bash

source "$TESTS/bash/tests.sh"

start_p2pfs

rmdir "$MNT1/blah" && exit 1
mkdir -p "$MNT1/blah/blah"
rmdir "$MNT1/blah" && exit 1

exit 0
