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

start_p2pfs

mkdir -p "$MNT1/blah/sdf"
mkdir -p "$MNT1/blh/sf"
mkdir -p "$MNT1/blh/sf2"
rmdir "$MNT1/blh/sf"
touch "$MNT1/blh/s"
touch "$MNT1/blh/d"
rm "$MNT1/blh/s"

find "$MNT1" | sort | colrm 1 ${#MNT1} > /tmp/p2pfs_tests_find1
find "$CACHE1" | sort | colrm 1 ${#CACHE1} > /tmp/p2pfs_tests_find2

if [ "$(diff /tmp/p2pfs_tests_find1 /tmp/p2pfs_tests_find2)" != "" ]
then
	exit 1
fi

exit 0
