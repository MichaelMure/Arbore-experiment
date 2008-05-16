#!/usr/bin/python
import os, random, sys

class FileEntry:
	def __init__(s, name, parent):
		s.name = name
		s.parent = parent

class DirEntry(FileEntry):
	def __init__(s, name, parent):
		FileEntry.__init__(s, name, parent)
		s.sub = {}
		if parent != None:
			s.sub[".."] = parent

	def mkdir(s, subname):
		if s.sub.has_key(subname):
			return
		s.sub[subname] = DirEntry(subname, s)

	def rmdir(s, subname):
		if not subname in s.sub.keys():
			return
		s.sub.pop(subname)

	def changedir(s, subname):
		return s.sub[subname]

def rnd_name():
	size = random.randint(10,20)
	string = ""
	while size != 0:
		string += chr(random.randint(97,122))
		size -= 1
	return string

def command(name, current_dir):
	if name == "mkdir":
		dirname = rnd_name()
		if dirname in current_dir.sub.keys():
			return current_dir
		current_dir.mkdir(dirname)
		os.mkdir(dirname)
		print "mkdir", dirname

	if name == "rmdir":
		if len(current_dir.sub.keys()) != 0:
			dirname = random.sample(current_dir.sub.keys(), 1)[0]
			if dirname == '..':
				return current_dir
			if len(current_dir.sub[dirname].sub.keys()) != 1:
				return current_dir
			current_dir.rmdir(dirname)
			os.rmdir(dirname)
			print "rmdir", dirname

	if name == "cd":
		if len(current_dir.sub.keys()) != 0:
			dirname = random.sample(current_dir.sub.keys(), 1)[0]
			current_dir = current_dir.changedir(dirname)
			os.chdir(os.getcwd() + '/' + dirname)
			print "cd", dirname

	return current_dir

commands_list = [ "mkdir", "rmdir", "cd" ]
slash = DirEntry("/", None)
current_dir = slash

i = 5000
while i != 0:
	command_name = random.sample(commands_list, 1)[0]

	current_dir = command(command_name, current_dir)
	i -= 1


