#!/usr/bin/python
import sys
import rtl, cpp

rtl_funcs = rtl.RtlFunctionsList()

if len(sys.argv) != 3:
	print "Syntax except_check.py <rtl_list_file> <cpp_source_list_file>"

print "Parsing RTL files"
rtl_files = open(sys.argv[1])
for file in rtl_files:
	rtl_funcs.Parse(file.rstrip('\n'))
rtl_files.close()

print "Parsing c++ files"
cpp_funcs = cpp.CppFunctionsList()

cpp_files = open(sys.argv[2])
for file in cpp_files:
	cpp_funcs.Parse(file.rstrip('\n'))
cpp_files.close()

print "Building callgraph"
rtl_funcs.BuildTree()

print "Finding exceptions"
rtl_funcs.FindExceptions()

print "Finding throws"
rtl_funcs.FindThrows()

print "Matching throw/catch"
rtl_funcs.MatchThrowCatches(cpp_funcs)

