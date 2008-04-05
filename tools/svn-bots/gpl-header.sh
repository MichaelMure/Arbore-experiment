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

cd /home/p2pfs-buildbot/peerfuse/gpl_header
svn up >/dev/null
headers_count=0

# Check cpp / h files
for file in $(find -name \*.cpp) $(find -name \*.h)
do
	head -n 5 "$file" | grep -i Copyright > /dev/null && continue
	cat tools/svn-bots/gpl_header_cpp "$file" > tmp
	mv tmp "$file"
	headers_count=$(($headers_count+1))
done

# Check sh files
for file in $(find -name \*.sh)
do
	head -n 5 "$file" | grep Copyright > /dev/null && continue

	cat tools/svn-bots/gpl_header_sh > tmp

	if [ "$(head -n 1 "$file"|grep '#!/bin/')" != "" ]
	then
		# remove the #!/bin/bash
		LINES=$(cat "$file"|wc -l)
		tail -n $(( $LINES - 1 )) "$file" >> tmp
	else
		cat "$file" >> tmp
	fi

	mv tmp "$file"
	headers_count=$(($headers_count+1))
done

if [ "$headers_count" != "0" ]
then
	svn ci --username=bot-gpl -m "$headers_count GPL header(s) added" > /dev/null
fi

