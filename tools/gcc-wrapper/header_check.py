#!/usr/bin/python

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


