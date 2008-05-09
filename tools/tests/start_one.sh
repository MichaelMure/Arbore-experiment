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


export TESTS_LOG="/tmp/p2pfs_tests.log"
export TESTS="$PWD"
export ROOT="$TESTS/../../"
export BINNAME="$1"
#export BINNAME=peerfuse-net
#export BINNAME=peerfuse-lan
export BIN="$TESTS/../../build.$1/$1"
#export TOOL=valgrind

if [ "$BINNAME" == "" ] || [ "$2" == "" ]
then
	echo "Syntax: ./start_one.sh <peerfuse-net/peerfuse-lan> <test_name>"
	exit 1
fi

if [ ! -x "$BIN" ]
then
	echo "Can't start $BIN"
	exit 1
fi

source bash/tests.sh

cd "$ROOT"


start_test "$2" || exit 1

