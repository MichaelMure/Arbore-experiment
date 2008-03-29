#!/bin/bash

function start_fs
{
	echo "Starting fs $i"
	if [ ! -f stdin$i ]
	then
		mkfifo stdin$i
	fi
	if [ ! -f stdout$i ]
	then
		mkfifo stdout$i
	fi
	if [ ! -f pf$i ]
	then
		mkdir -p pf$1/mount
		mkdir -p pf$1/work
		mkdir -p pf$1/cache
		export TESTS=../tests
		export NB_P2PFS=$1
		export CONNECT_PORT=$((8000+$i))
		export LISTEN_PORT=$((8001+$i))
		export CACHE_ROOT=pf$i/cache
		export WORKDIR=pf$i/work
		export CONFDIR=WORKDIR
		build_conf pf$i/pfconf.conf
	fi

	case $TOOL in
		gdb)
			tail -f stdin$i | gdb --args "../../build.$PF/$PF" pf$i/pfconf.conf -d pf$i/mount 2>&1| while read line
			do
				echo "$i $line" >> stdout$i
			done
			;;
		valgrind)
			tail -f stdin$i | valgrind "../../build.$PF/$PF" pf$i/pfconf.conf -d pf$i/mount 2>&1 | while read line
			do
				echo "$i $line" >> stdout$i
			done
			;;
		helgrind)
			tail -f stdin$i | valgrind --tool=helgrind "../../build.$PF/$PF" pf$i/pfconf.conf -d pf$i/mount 2>&1 | while read line
			do
				echo "$i $line" >> stdout$i
			done
			;;
		*)
			echo "Unknown TOOL option"
			exit 1
	esac
}


source debug_env.conf

if [ "$PF" == "" ]
then
	echo "Please modify debug_env.conf before starting this script"
	exit 1
fi

source ../tests/bash/$PF-conf.sh

if [ $# != 1 ]
then
	echo "Syntax: ./start_fs <number of peerfuse to start>"
	exit 1
fi

if [ $TOOL == valgrind ] || [ $TOOL == helgrind ]
then
	if [ $UID != 0 ]
	then
		echo "You need to be root to start peerfuse under valgrind/helgrind"
		exit 1
	fi
fi

for((i=1;i<=$1;i++))
do
	start_fs $i &
	case $TOOL in
		gdb)
			sleep 2
			#echo "set follow-fork-mode child" > stdin$i
			echo "r" > stdin$i
			;;
		valgrind) sleep 10 ;;
		helgrind) sleep 10 ;;
	esac
	tail -f stdout$i &
done

trap break 2
while true
do
	sleep 1h
done

echo "Exiting..."

for((i=1;i<=$1;i++))
do
	fusermount -u pf$i/mount
done

killall tail
case $TOOL in
	gdb) killall gdb ;;
	valgrind) killall valgrind ;;
	helgrind) killall valgrind ;;
esac

if [ "$CLEAN_AFTER_EXIT" == "yes" ]
then
	for((i=1;i<=$1;i++))
	do
		rm -rf pf$i
	done
fi

for((i=1;i<=$1;i++))
do
	rm stdin$i stdout$i
done

