#!/usr/bin/python

import sys, re, os

for filename in sys.argv[1:]:

	f = open(filename)
	filename = os.path.split(filename)[1]
	upper = filename.upper()
	upper = upper.replace(".", "_")

	for line in f:
		if re.match("#define.*_H$", line) and not re.match("#define.*%s" % upper, line):
			print "Wrong define here: %s\n%s" % (filename, line)


