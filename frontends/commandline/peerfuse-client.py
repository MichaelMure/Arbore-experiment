#!/usr/bin/env python
# peerfuse-client.py: the Command Line Client for PeerFuse
#
# Copyright (C) 2008 Mathieu Virbel <tito@bankiz.org>
#
# This file is part of peerfuse.
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

import cmd, xmlrpclib, re, sys
import os, time
from stat import S_ISDIR

class PeerfuseInterpretor(cmd.Cmd):

	proxy = False
	host = 'localhost'
	port = '1778'
	cwd = '/'

	def __init__(self):
		self.__set_prompt()
		self.intro = '''Welcome to peerfuse-client. Type ? for help.'''

		cmd.Cmd.__init__(self)

	def default(self, line):
		print '*** Unknown Syntax : %s (type help for a list of valid command)' % line

	def emptyline(self):
		print "Type 'exit' to terminate the session or type ? for help."

	def do_exit(self, arg):
		"""\nTerminate the session\n"""
		return -1

	def do_quit(self, arg):
		"""\nTerminate the session\n"""
		return -1

	def do_EOF(self, arg):
		"""\nTerminate the session\n"""
		print
		return -1

	def do_ls(self, arg):
		"""\nShow content of current directory
		-l : long listing format
		-R : recursive (include -l)\n"""
		arg = [a for a in arg.split(' ') if a != '']

		# Define options for LS
		opt_format = 'normal'
		opt_show_path = False
		opt_recursive = False

		# Rewrite all path with current path
		new_arg = []
		for path in arg:
			if path.startswith('/'):
				new_arg.append(path)
			elif path == '.':
				new_arg.append(self.cwd)
			elif path == '-l':
				opt_format = 'advanced'
			elif path == '-R':
				opt_recursive = True
				opt_show_path = True
				opt_format = 'advanced'
			else:
				new_path = os.path.join(self.cwd, path)
				new_path = os.path.normpath(new_path)
				# python os.path.normpath bug ?
				if new_path == '//':
					new_path = '/'
				new_arg.append(new_path)
		arg = new_arg

		# If no path found, use current path
		if len(arg) < 1:
			arg.append(self.cwd)

		# If they are more than one path, show directories
		if len(arg) > 1:
			opt_show_path = True

		# Read directory for all path
		for path in arg:
			try:
				# First, check it's a directory
				stat = self.proxy.browser.getattr(path)
				if type(stat) != dict:
					print 'ls: can\'t access to %s: no such file or directory' % path
					continue

				# If it's not a directory, show him in ls, and continue
				if not S_ISDIR(stat['mode']):
					file = os.path.basename(path)
					if opt_format == 'normal':
						print file
					else:
						self.__show_ls(file, stat, opt_format=opt_format, opt_show_path=opt_show_path)
					continue

				# Show path if multipath
				if opt_show_path:
					print '%s:' % (path)

				# Read file in directory
				files = self.proxy.browser.readdir(path)
				if type(files) != list:
					print 'ls: can\'t access to %s: no such file or directory' % path
					continue

				print 'total %d' % (len(files))

				if opt_format == 'normal':
					print ', '.join(files)
				else:
					# Optimize getattr with doing a multicall
					mc = xmlrpclib.MultiCall(self.proxy)
					for file in files:
						mc.browser.getattr(os.path.join(path, file))
					stats = mc()

					# Read stats from multicall
					i = 0
					for stat in stats:
						file = files[i]
						self.__show_ls(file, stat, opt_format=opt_format, opt_show_path=opt_show_path)
						i = i + 1
						# Recursive call
						if opt_recursive and S_ISDIR(stat['mode']):
							arg.append(os.path.join(path, file))

				# Newline between each path
				if path != arg[-1]:
					print ""

			except Exception, e:
				print 'Error: ' + str(e)
				continue

	def do_connect(self, arg):
		"""\nConnect to peerfuse daemon\nSyntaxe : connect <host> [port=1778]"""
		arg = [a for a in arg.split(' ') if a != '']
		if len(arg) < 1:
			print 'No argument given'
			return
		self.port = '1778'
		if len(arg) >= 1:
			self.host = arg[0]
		if len(arg) > 1:
			self.port = arg[1]
		self.__set_prompt()
		self.proxy = xmlrpclib.ServerProxy('http://%s:%s/' % (self.host, self.port))

	def do_plist(self, arg):
		"""\nReturn a list of all peers\n"""
		try:
			list = self.proxy.peers.list()
			if len(list) <= 0:
				print "No peer connected"
				return
			print "| Adresse           | Port    | State        | ID"
			for row in list:
				peer = self.proxy.peers.info(row)
				info = re.split(r'(.*):(.*)\/(.*)', peer['pf_addr'])
				print "| %17s | %7s | %12s | %s" % (info[1], info[2], peer['state'], info[3])
		except Exception, e:
			print 'Error: ' + str(e)

	def __set_prompt(self):
		self.prompt = '%s:%s %s > ' % (self.host, self.port, self.cwd)

	def __show_ls(self, file, stat, opt_format='normal', opt_show_path=False):
		try:
			size = stat['size']
			mtime = time.localtime(stat['mtime'])
			if S_ISDIR(stat['mode']):
				file = '\033[1;34m'+ file + '/\033[00m'
			print '%6d %6d %9s %4d-%02d-%02d %02d:%02d %s' % (
					stat['uid'], stat['gid'], size,
					mtime[0], mtime[1], mtime[2],
					mtime[3], mtime[4], file )
		except Exception, e:
			print file
			print 'Error: ' + str(e)

if __name__ == '__main__':

	connect = ''
	if len(sys.argv) > 2:
		connect = sys.argv[1] + ' ' + sys.argv[2]
	elif len(sys.argv) > 1:
		connect = sys.argv[1]
	else:
		connect = 'localhost'

	client = PeerfuseInterpretor()
	client.do_connect(connect)
	client.cmdloop()
