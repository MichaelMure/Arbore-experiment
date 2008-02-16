#!/bin/bash

source "$TESTS/bash/tests.sh"

export LISTEN_PORT=9876
export CONNECT_PORT=6789
start_p2pfs

export LISTEN_PORT=8000
export CONNECT_PORT=9876
start_p2pfs

# Check netstat show them as connected
if [ "$(netstat -tanp | grep "ESTABLISHED.*$FS1_PID/\|ESTABLISHED.*$FS2_PID/" | wc -l)" != "2" ]
then
	exit 1
fi

exit 0
