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

svn ci --username=bot-indent -m "+$PLUS_LINE line(s)  -$LESS_LINE line(s)"
#echo "+$PLUS_LINE lines  -$LESS_LINE lines"


