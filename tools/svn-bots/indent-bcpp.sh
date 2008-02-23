#!/bin/bash
set -e
cd /home/p2pfs-buildbot/peerfuse/indent
svn up > /dev/null
make clean > /dev/null
make >/dev/null || exit 0

for file in $(find -name \*.h)
do
	bcpp -fnc /etc/bcpp/bcpp.cfg "$file" > /dev/null

	# Remove the \t at the start of a line
	# in class declarations
	sed -i -e '/public:\|private:\|protected:/,/^\}/s/^\t//' \
		"$file"
done

for file in $(find -name \*.cpp)
do
	bcpp -fnc /etc/bcpp/bcpp.cfg "$file" > /dev/null

	# Correctly indent class members definitions arguments
	sed -i -e '/^[^\t].*::/,/^{/ { /::\|^{/!{s/^/\t\t\t/}}' \
		"$file"
done

PLUS_LINE=$(svn diff|grep ^+|wc -l)
LESS_LINE=$(svn diff|grep ^-|wc -l)

svn ci -m "+$PLUS_LINE lines  -$LESS_LINE lines"
#echo "+$PLUS_LINE lines  -$LESS_LINE lines"


