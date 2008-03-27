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


alias valgrind="valgrind --tool=memcheck \
				--leak-check=yes \
				--show-reachable=yes \
				--trace-children=yes"
source "$TESTS/bash/$BINNAME-conf.sh"

function start_p2pfs
{
	# Arguments:
	# $1: conf file

	export NB_P2PFS="$(($NB_P2PFS + 1))"

	# Start 2 p2pfs
	LOG="LOG$NB_P2PFS"
	export $LOG="$WORKDIR/p2pfs_log$NB_P2PFS"
	MNT="MNT$NB_P2PFS"
	export $MNT="$WORKDIR/mnt$NB_P2PFS"
	CONF="CONF$NB_P2PFS"
	export $CONF="$WORKDIR/conf$NB_P2PFS"
	CONFDIR="CONFDIR$NB_P2PFS"
	export $CONFDIR="$WORKDIR/confdir$NB_P2PFS"
	mkdir ${!CONFDIR} || true 2>&1 # don't exit on error

	CACHE="CACHE$NB_P2PFS"
	if [ "$CACHE_ROOT" == "" ]
	then
		# if CACHE_ROOT is set, use it
		export $CACHE="$WORKDIR/cache$NB_P2PFS"
		export CACHE_ROOT="${!CACHE}"
	else
		export $CACHE="$CACHE_ROOT"
	fi

	build_conf "${!CONF}"
	unset CACHE_ROOT

	if [ ! -d ${!MNT} ]
	then
		mkdir "${!MNT}" 2>/dev/null
	fi
	if [ ! -d ${!CACHE} ]
	then
		mkdir "${!CACHE}" 2>/dev/null
	fi

	# Start a peer and store its pid
	case "$TOOL" in
		"")
			"$BIN" "${!CONF}" "${!MNT}" -d > "${!LOG}" 2>&1 &
			;;
		"valgrind")
			export VAL="VAL$NB_P2PFS"
			export $VAL="$WORKDIR/p2pfs_val$NB_P2PFS"
			valgrind --log-file="${!VAL}" "$BIN" "$1" "${!MNT}" -d > "${!LOG}" 2>&1 &
			;;
	esac

	COUNT=0
	while [ 1 ]
	do
		mount | grep "on ${!MNT} " && break
		sleep 1s
		COUNT=$(($COUNT + 1))
		if [ $COUNT == 20 ]
		then
			echo "Could not start:"
			cat "${!LOG}" 
			exit 1
		fi
	done

	FS_PID="FS${NB_P2PFS}_PID"
	export $FS_PID=$!

	set |grep '^MNT[0-9]*=\|^LOG[0-9]*=\|^FS[0-9]*_PID=\|^NB_P2PFS=\|^VAL[0-9]*=\|^CACHE[0-9]*=' > "$WORKDIR/p2pfs_tests_vars"
}

function start_test
{
	# Arguments:
	# $1: test script to start

	export NB_P2PFS=0
	export WORKDIR="/tmp/p2pfs_root"
	mkdir -p "$WORKDIR" 2>/dev/null

	echo -n "Test: $1"

	bash -e -x "tools/tests/$1" > "$TESTS_LOG" 2>&1
	RET=$?

	if [ -f "$WORKDIR/p2pfs_tests_vars" ]
	then
		source "$WORKDIR/p2pfs_tests_vars"
	fi

	if [ "$RET" == "0" ]
	then
		echo $'\t'$'\t'"[Success]"
	else
		echo $'\t'$'\t'"[Failed]"
		echo
		echo
		echo "Test log:"
		cat "$TESTS_LOG" > /dev/stderr

		if [ "$NB_P2PFS" != "0" ]
		then
			for((i=1;i<=$NB_P2PFS;i++))
			do
				echo
				echo
				echo "Server $i log:"
				LOG="LOG$i"
				MNT="MNT$i"
				cat "${!LOG}"
				echo
				echo "Files in MNT$i:"
				ls "${!MNT}"
			done
		fi
	fi

	# Clean everything
	cd "$WORKDIR"

	if [ "$NB_P2PFS" != "0" ]
	then
		for((i=1;i<=$NB_P2PFS;i++))
		do
			MNT="MNT$i"
			FS_PIF="FS${i}_PID"
			fusermount -u "${!MNT}" 2>/dev/null
			kill -9 ${!FS_PID} 2> /dev/null
		done
	fi

	sleep 1

	cd - >/dev/null
	rm -rf "$WORKDIR"

	return $RET
}

