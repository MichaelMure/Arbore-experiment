#!/usr/bin/python
import re, sys

class CppFunction:
	def __init__(self, name):
		self.name = name
		self.catches = []

	def NewCatch(self, catch):
		self.catches.append(catch)

class CppFunctionsList:
	def __init__(self):
		self.lst = {}
	
	def Parse(self, file):
		f = open(file)
		inside_comment = False
		inside_body = False
		function_begin = False
		acco_count = 0
		last_func = None
		for line in f:
			if inside_comment:
				if re.search('\*/', line):
					inside_comment = False
					line = re.search('.*\*/(.*$)', line).group(1)
				else:
					continue

			# Remove comment on one line
			while re.search('.*/\*.*\*/.*', line):
				m = re.search('(.*)(/\*.*\*/)(.*)', line)
				line = m.group(1) + m.group(3)

			if re.search('.*/\*.*$', line):
				inside_comment = True
				line = re.search('(.*)/\*.*$', line).group(1)

			if re.search('//', line):
				line = re.search('(.*)//.*$', line).group(1)


			# Match a function definition header
			if not inside_body and not function_begin:
				# Match a function with no return type (constructor/destructors)
				m = re.search('^([^ \t(]*::[^ \t(]*)[ \t]*\(([^;]*)', line)
				if not m:
					m = re.search('([^ \t(&*]*)[ \t]*\(([^;]*)', line)
				if m:
					last_func = CppFunction(m.group(1))
					self.lst[last_func.name] = last_func
					if re.search('{.*}', m.group(2)):
						continue # The function is a one-line function skip it
					function_begin = True
					if re.search('{', m.group(2)):
						inside_body = True
						acco_count = 1
					continue

			# Match the beginning of the function body
			if not inside_body and function_begin:
				if re.search('{', line):
					inside_body = True
					acco_count += line.count('{') - line.count('}')
					continue

			if inside_body:
				acco_count += line.count('{') - line.count('}')
				if acco_count < 0:
					print "Unable to match '}'"
					sys.exit(1)

				if acco_count == 0:
					inside_body = False
					function_begin = False

				m = re.match('[ \t]*catch[ \t]*\(([ \t]*[^ \t]*).*\)', line)
				if m:
					last_func.NewCatch(m.group(1))
		f.close()

	def GetCatches(self, function_name):
		# Remove arguments and return type of functions
		m = re.search('([^ \t]*)[ \t]*\(', function_name)

		name = m.group(1)
		if not self.lst.has_key(name):
			print name,"not found in cpp code"
			return []
		return self.lst[name].catches
