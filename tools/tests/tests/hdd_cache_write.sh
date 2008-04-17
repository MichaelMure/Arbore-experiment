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
sleep 2

## Generate a random file inside of the cache
dd if=/dev/urandom of="$MNT1/tmp" bs=1024 count=4
MD5="$(cat "$MNT1/tmp"|md5sum)"

## Unmount the fs to force writing datas to hdd
fusermount -u "$MNT1"
sleep 2

## Check the file
if [ "$(cat "$CACHE1/tmp"|md5sum)" != "$MD5" ]
then
	exit 1
fi

