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

				print "Dependencie loop detected:", filename, "->", file
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

# Build the path to search for files
arg_no = 0
filename = ""
for option in sys.argv[2:]:
	if option == "-I":
		include_dirs.append(sys.argv[arg_no + 3])
	elif option == "-c":
		filename = sys.argv[arg_no + 3]
	arg_no += 1

if len(filename) < 4:
	sys.exit(0)

if filename[len(filename)-4:] != ".cpp":
	print "Don't know how to handle", filename
	sys.exit(0)

# We'll emit an error if someone include this file:
forbidden_header = os.path.basename(filename)
forbidden_header = forbidden_header[:len(forbidden_header)-4] + ".h"

ignore_list = load_ignore_list("../tools/gcc-wrapper/header_loop_ignore")

# Add to the search path the parent folder of the file
include_dirs.append(os.path.dirname(filename))

if check_loop(os.path.basename(filename), True):
	sys.exit(1)

