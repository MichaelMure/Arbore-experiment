#!/bin/bash

source "$TESTS/bash/tests.sh"

export LISTEN_PORT=9876
export CONNECT_PORT=6789
start_p2pfs

SSL_LOG="$(echo blah| openssl s_client -connect localhost:9876 -key $TESTS/conf/certs/key2.key -cert $TESTS/conf/certs/key2.crt 2>&1 >/dev/null)"

if [ "$(echo "$SSL_LOG"|tail -n 1)" != "DONE" ]
then
	echo "$SSL_LOG"
	exit 1
fi

exit 0
