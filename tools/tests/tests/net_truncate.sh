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

sleep 10s

TMPFILE="$(mktemp)"

# Generate random data
dd if=/dev/urandom of="$TMPFILE" bs=1024 count=4
cp "$TMPFILE" "$MNT1/tmp"
rm "$TMPFILE"

sleep 5

## Retrieve the file into the 2nd peer's cache
if [ "$(cat "$MNT2/tmp"|md5sum)" != "$(cat "$MNT1/tmp"|md5sum)" ]
then
	exit 1
fi

# Wait for the file to be flushed in the 2nd peer's cache
sleep 20
if [ "$(cat "$CACHE2/tmp"|md5sum)" != "$(cat "$MNT1/tmp"|md5sum)" ]
then
	exit 1
fi

# Re-generate random data
dd if=/dev/urandom of="$TMPFILE" bs=1024 count=5
cp "$TMPFILE" "$MNT1/tmp"
rm "$TMPFILE"
sleep 5

# The file should be truncated on the 2nd peer's cache
if [ "$(cat "$CACHE2/tmp")" != "" ]
then
	exit 1
fi

if [ "$(cat "$MNT2/tmp"|md5sum)" != "$(cat "$MNT1/tmp"|md5sum)" ]
then
	exit 1
fi
