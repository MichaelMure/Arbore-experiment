import re

class RtlFunctionCall:
	def __init__(self, mangled):
		self.mangled = mangled

class RtlFunction:
	def __init__(self, name, mangled):
		self.name = name
		self.mangled = mangled
		self.call_lst = []
		self.parent = []
		self.throws = []

	def AddCall(self, called_func):
		#print self.name, "calls", called_func.mangled
		self.call_lst.append(called_func)

	def IsExceptions(self, std_exception_mangled):
		# All method calling the std::exception constructor are exceptions
		for call in self.call_lst:
			if call.mangled == std_exception_mangled:
				return True
		return False

	def FindThrow(self, exceptions):
		# All method calling an exception constructor are throws
		for call in self.call_lst:
			if exceptions.has_key(call.mangled):
				if call not in self.throws:
					self.throws.append(call)
		#if len(self.throws) != 0:
		#	print self.name, "throws:\n\t", "\n\t".join(exceptions[call.mangled].name for call in self.throws)

	def ParentsHaveCatch(self, cpp_func, excep):
		my_catch = cpp_func.GetCatches(self.name)
		exc_name = excep.GetPureName()

		for c in my_catch:
			if len(c) < len(exc_name):
				if c == excep.name[len(excep.name) - len(c) :]:
					print c, "matches", excep.name
					return True
		if "..." in my_catch:
			print "Warning:", excep.name, "is catched by a generic exception handler in this func:", self.name
			return True
		else:
			if len(self.parent) == 0:
				print self.name, "(", self.mangled, ")"
				return False
			for p in self.parent:
				if not p.ParentsHaveCatch(cpp_func, excep):
					print self.name, "(", self.mangled, ")"
					return False
			return True # All our parent handle this exception
	def GetPureName(self):
		# returns the name without argument and return type
		m = re.search('([^ \t]*)[ \t]*\(', self.name)
		if not m:
			return None
		return m.group(1)

class RtlFunctionsList:
	def __init__(self):
		self.lst = {}
		self.exceptions = {}
		self.std_exception_mangled = ""

	def Parse(self, f):
		file = open(f)
		current_func = None
		last_func_call = None

		for line in file:
			# search a new function
			m = re.search('^;; Function (.*) \((.*)\)$', line)
			if m:
				#print "Function:", m.group(1)
				#print "Mangled name:", m.group(2)
				if self.lst.has_key(current_func):
					current_func = self.lst[current_func.name]
				else:
					current_func = RtlFunction(m.group(1), m.group(2))
					self.lst[current_func.name] = current_func
				continue
	
			# search a function call
			m = re.search('\(call \(mem:QI \(symbol_ref[^ ]* \("([^"]*)"', line)
			if m and current_func:
				last_func_call = RtlFunctionCall(m.group(1))
				current_func.AddCall(last_func_call)
		if self.lst.has_key("std::exception::exception()"):
			self.std_exception_mangled = self.lst["std::exception::exception()"].mangled
		file.close()

	def DispFuncs(self):
		print len(self.lst), "functions parsed"
		for func in self.lst.values():
			print "Function", func.name, "(", func.mangled, ")"
			if len(func.call_lst) != 0:
				print "calls"
				for call in func.call_lst:
					print "\t", call.mangled
	
	def BuildTree(self):
		for func in self.lst.values():
			for call in func.call_lst:
				for func_called in self.lst.values():
					if call.mangled == func_called.mangled:
						func_called.parent.append(func)
		print "Tree built"

	def DispGraph(self, func_name): # Warning: this method destructs the graphtree when being used
		print "Callgraph of", func_name
		if not self.lst.has_key(func_name):
			return
		caller = self.lst[func_name]

		while caller:
			print caller.name, "(", caller.mangled, ")"
			if caller.parent:
				caller = caller.parent.pop()
			else:
				caller = None

	def FindExceptions(self):
		for func in self.lst.values():
			if func.IsExceptions(self.std_exception_mangled):
				self.exceptions[func.mangled] = func
	def FindThrows(self):
		for func in self.lst.values():
			func.FindThrow(self.exceptions)

	def GetFuncFromMangled(self, mangled):
		for func in self.lst.values():
			if func.mangled == mangled:
				return func

	def MatchThrowCatches(self, cpp_func):
		for func in self.lst.values():
			for throw in func.throws:
				if not func.ParentsHaveCatch(cpp_func, self.GetFuncFromMangled(throw.mangled)):
					print "in this callgraph, exception", self.GetFuncFromMangled(throw.mangled).name, "is not handled\n"

