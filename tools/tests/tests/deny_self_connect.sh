#!/bin/bash

source "$TESTS/bash/tests.sh"

export LISTEN_PORT=9876
export CONNECT_PORT=9876

start_p2pfs

sleep 5s
if [ "$(pgrep peerfuse- |grep $FS1_PID)" != "" ]
then
	pgrep peerfuse
	exit 1
fi

