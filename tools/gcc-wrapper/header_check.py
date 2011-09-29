#!/usr/bin/python
#
# Copyright(C) 2008 Laurent Defert
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
# This product includes cryptographic software written by Eric Young
# (eay@cryptsoft.com).  This product includes software written by Tim
# Hudson (tjh@cryptsoft.com).

import sys, re, os

# Arguments contains the .cpp file compiled
# Convert it to the corresponding .h file:
filename = sys.argv[len(sys.argv)-1]
if len(filename) < 4 or filename[len(filename)-4:] != ".cpp":
	sys.exit(0)
filename = filename[:len(filename)-4] + ".h"

# Try opening it
try:
	f = open(filename)
except IOError:
	#print "Can't open file", filename
	sys.exit(0)

filename = os.path.split(filename)[1]
upper = filename.upper()
upper = upper.replace(".", "_")

for line in f:
	if re.compile("#define.*_H$").search(line):
		if not re.compile("#define.*%s" % upper).search(line):
			print "Wrong define here: %s\n%s" % (filename, line)
			sys.exit(1)


