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

RST2HTML=""
if [ -e /usr/bin/rst2html ]
then
	RST2HTML=/usr/bin/rst2html
fi
if [ -e /usr/bin/rst2html.py ]
then
	RST2HTML=/usr/bin/rst2html.py
fi

if [ "$RST2HTML" == "" ]
then
	echo "Unable to find the RST to html converter, install docutils."
	exit 1
fi

if [ "$1" == "" ]
then
	echo "Syntax: $0 <file_to_htmlize>"
	exit 1
fi

cat html/header.html
"$RST2HTML" "$1" | sed -e '1,7d'
cat html/footer.html

