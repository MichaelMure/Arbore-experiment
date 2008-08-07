#!/usr/bin/env python

import cmd, xmlrpclib, re, sys

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

	def do_EOF(self, arg):
		"""\nTerminate the session\n"""
		print
		return -1

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

if __name__ == '__main__':

	connect = ''
	if len(sys.argv) > 2:
		connect = sys.argv[1] + ' ' + sys.argv[2]
	elif len(sys.argv) > 1:
		connect = sys.argv[1]

	client = PeerfuseInterpretor()
	client.do_connect(connect)
	client.cmdloop()
