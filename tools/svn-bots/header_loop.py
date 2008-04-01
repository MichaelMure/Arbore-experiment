#!/usr/bin/python

import os, sys, re, getopt

# Load the list of dependancies to ignore
def load_ignore_list(file):
	f = open(file)
	lst = {}
	for line in f:
		m = re.compile("^([^#].*)->(.*)$").search(line)
		if m:
			if m.group(1) not in lst:
				lst[m.group(1)] = []
			lst[m.group(1)].append(m.group(2))
	return lst

# Recursively check included file for a depency loop
def check_loop(file, top = False):
	# Looks for the file on all directories specified to gcc's command line
	f = None
	try:
		f = open(file)
	except IOError:
		pass
	
	if not f:
		for inc_dir in include_dirs:
			try:
				f = open(inc_dir + "/" + file)
			except IOError:
				pass
			if f:
				break

	# The file couldn't be found (maybe a header without a corresponding .cpp file)
	if not f:
		return False

	# Parse the file to find all the .h it includes
	includes = []
	for line in f:
		m = re.compile('#include "([^"]*)"').search(line)
		if m:
			if m.group(1) == forbidden_header and not top:
				# Check the include is not in the ignore list
				if file in ignore_list and m.group(1) in ignore_list[file]:
					continue

				print "Dependencie loop detected:", arg[0], "->", file
				return True

			includes.append(m.group(1))
	f.close()

	# Remember we checked this file
	header_checked.append(os.path.basename(file))

	# Recursively check all included files
	for inc in includes:
		# Check the include is not in the ignore list
		if file in ignore_list and inc in ignore_list[file]:
			continue

		# Check the the header by itself
		if inc not in header_checked and check_loop(inc):
			print "Included by:", file
			return True

		# Check the corresponding .ccp file
		cpp_file = inc[:len(inc)-1] + "cpp"
		if cpp_file not in header_checked and check_loop(cpp_file):
			print "Included by:", file
			return True
	return False


# Parse options, nicely ignore gcc flags
include_dirs = []
header_checked = []
opt, arg = getopt.gnu_getopt(sys.argv[1:], "I:o:D:W:f:r:l:L:cg")

# Build the path to search for files
for option in opt:
	if option[0] == "-I":
		include_dirs.append(option[1])

if arg[0][len(arg[0])-4:] != ".cpp":
	print "Don't know how to handle", arg[0]
	sys.exit(0)

# We'll emit an error if someone include this file:
forbidden_header = os.path.basename(arg[0])
forbidden_header = forbidden_header[:len(forbidden_header)-4] + ".h"

ignore_list = load_ignore_list("../tools/svn-bots/header_loop_ignore")

# Add to the search path the parent folder of the file
include_dirs.append(os.path.dirname(arg[0]))

if check_loop(os.path.basename(arg[0]), True):
	sys.exit(1)

