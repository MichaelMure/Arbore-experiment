#!/bin/bash
# Copyright(C) 2008 Laurent Defert, Romain Bignon
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# $Id$
#

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

