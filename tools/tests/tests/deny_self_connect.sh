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
export CONNECT_PORT=9876

start_p2pfs

sleep 5s
if [ "$(pgrep peerfuse- |grep $FS1_PID)" != "" ]
then
	pgrep peerfuse
	exit 1
fi

