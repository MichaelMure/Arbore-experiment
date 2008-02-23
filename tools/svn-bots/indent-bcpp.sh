#!/bin/bash
set -e
cd /home/p2pfs-buildbot/peerfuse/trunk
svn up > /dev/null
make clean
make || exit 0

for file in $(find -name \*.h)
do
	bcpp -fnc /etc/bcpp/bcpp.cfg "$file"

	# Remove the \t at the start of a line
	# in class declarations
	sed -i -e '/public:\|private:\|protected:/,/^\}/s/^\t//' \
		"$file"
done

for file in $(find -name \*.cpp)
do
	bcpp -fnc /etc/bcpp/bcpp.cfg "$file"

	# Correctly indent class members definitions arguments
	sed -i -e '/^[^\t].*::/,/^{/ { /::\|^{/!{s/^/\t\t\t/}}' \
		"$file"
done

PLUS_LINE=$(svn diff|grep ^+|wc -l)
LESS_LINE=$(svn diff|grep ^-|wc -l)

#svn ci -m "+$PLUS_LINE lines  -$LESS_LINE lines"


