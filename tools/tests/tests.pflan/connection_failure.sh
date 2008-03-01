#!/bin/bash

source "$TESTS/bash/tests.sh"

export LISTEN_PORT=9876
export CONNECT_PORT=6111
start_p2pfs

export LISTEN_PORT=9877
export CONNECT_PORT=9876
start_p2pfs

sleep 5s

# Check they are connected
netstat -tanp | grep -q ':9876.*ESTABLISHED' || exit 1

# Deny connections on port 9878
sudo /usr/local/scripts/reject_9878

export LISTEN_PORT=9878
export CONNECT_PORT=9876

# This fs can't be contacted by the peer 2
# so it should just exit
start_p2pfs

sleep 5

# Check the fs exited
ps x|grep -q '^${FS3_PID}.*peerfuse-lan' && exit 1

sudo /usr/local/scripts/accept_9878
