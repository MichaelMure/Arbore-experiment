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

## Generate a random file inside of the cache
mkdir -p "$WORKDIR/fs"
dd if=/dev/urandom of="$WORKDIR/fs/tmp" bs=1024 count=4
## Provide the correct entry in tree.cfg
mkdir -p "$WORKDIR/confdir1"
echo "/tmp#size=4096" > "$WORKDIR/confdir1/tree.cfg"

## Start a fs using the cache previously filled
export CACHE_ROOT="$WORKDIR/fs"
start_p2pfs

## Check the file
if [ "$(cat "$WORKDIR/fs/tmp"|md5sum)" != "$(cat "$MNT1/tmp" |md5sum)" ]
then
	exit 1
fi

