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


mkdir -p "$WORKDIR/fs/blah/sdf"
mkdir -p "$WORKDIR/fs/blh/sf"
mkdir -p "$WORKDIR/fs/blh/sf2"
touch "$WORKDIR/fs/blh/s"
touch "$WORKDIR/fs/blh/d"

find "$WORKDIR/fs" | sort | colrm 1 $(echo -n "$WORKDIR/fs"|wc -c) > /tmp/p2pfs_tests_find1

# Start an other fs using the same cache
export CACHE_ROOT="$WORKDIR/fs"
start_p2pfs

# Check files have been reloaded successfully
find "$MNT1" | sort | colrm 1 ${#MNT1} > /tmp/p2pfs_tests_find2

if [ "$(diff /tmp/p2pfs_tests_find1 /tmp/p2pfs_tests_find2)" != "" ]
then
	exit 1
fi

exit 0
