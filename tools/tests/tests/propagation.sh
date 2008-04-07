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

export LISTEN_PORT=9876
export CONNECT_PORT=6789
start_p2pfs

export LISTEN_PORT=6789
export CONNECT_PORT=9876
start_p2pfs

mkdir -p "$MNT1/blh/aze"
mkdir -p "$MNT1/blh/ca"
touch "$MNT1/blh/plop"

# Trying create/remove
mkdir "$MNT2/truc/"
touch "$MNT2/truc/bidule"
rm "$MNT2/truc/bidule"
sleep 2
rm -r "$MNT1/truc"

sleep 3

find "$MNT1" | colrm 1 ${#MNT1} > /tmp/p2pfs_tests_find1
find "$MNT2" | colrm 1 ${#MNT2} > /tmp/p2pfs_tests_find2

if [ "$(diff /tmp/p2pfs_tests_find1 /tmp/p2pfs_tests_find2)" != "" ]
then
	exit 1
fi

exit 0
