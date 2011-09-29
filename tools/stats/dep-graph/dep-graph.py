#!/usr/bin/python
#
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
# This product includes cryptographic software written by Eric Young
# (eay@cryptsoft.com).  This product includes software written by Tim
# Hudson (tjh@cryptsoft.com).


import os, sys, re, getopt

class gv:
	def __init__(s, file):
		s.f = open(file, "w")
		s.f.write('digraph G\n{\n')
		s.f.write('\toverlap="scale";\n')
		s.f.write('\tlabelfloat="true";\n')
		s.f.write('\tsplines="true";\n')
		s.links = {}
		s.styles = {}

	def new_link(s, src, dst):
		if src == dst:
			return
		if src not in s.links:
			s.links[src] = []
		if dst not in s.links[src]:
			s.links[src].append(dst)
	def set_style(s, node):
		s.styles[node] = 'style="filled",color="#FF0000"'

	def write(s):
		for node in s.styles.keys():
			s.f.write('\t%s [%s];\n' % (node, s.styles[node]))
		for src in s.links.keys():
			for dst in s.links[src]:
				s.f.write('\t%s->%s;\n' % (src, dst))
		s.f.write('}\n')

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

def graph_deps(file):
	f = open(file)

	# Parse the file to find all the .h it includes
	for line in f:
		m = re.compile('#include "([^"]*)"').search(line)
		if m:
			if file in ignore_list and m.group(1) in ignore_list[file]:
				continue

			# remove the path from the filename
			src = os.path.basename(file)
			# remove the file extension
			src = re.compile('^([^\.]*)\..*$').search(src).group(1)

			#remove the file extension
			dst = m.group(1)
			dst = dst[:len(dst) - 2]
			g.new_link(src, dst)

		m = re.compile('^extern').search(line)
		if m:
			# remove the path from the filename
			src = os.path.basename(file)
			# remove the file extension
			src = re.compile('^([^\.]*)\..*$').search(src).group(1)
			g.set_style(src)

	f.close()

g = gv("out.dot")

ignore_list = load_ignore_list("tools/gcc-wrapper/header_loop_ignore")

for file in sys.argv[1:]:
	graph_deps(file)

g.write()
