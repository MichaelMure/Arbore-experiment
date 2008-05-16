/*
 * Copyright(C) 2008 Laurent Defert, Romain Bignon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * This product includes cryptographic software written by Eric Younganus
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * $Id$
 */

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include "session_config.h"
#include "log.h"

SessionConfig session_cfg;
SessionConfig tree_cfg;

SessionConfig::SessionConfig() : Mutex(RECURSIVE_MUTEX)
{
}

SessionConfig::~SessionConfig()
{
	Save();
}

static ssize_t getline(std::string& line, std::fstream& file)
{
	line.clear();
	std::getline(file, line);
	if(file.eof())
		return -1;
	return line.size();
}

void SessionConfig::Load(const std::string& _filename)
{
	BlockLockMutex lock(this);
	filename = _filename;

	std::fstream fin;
	fin.open(filename.c_str(), std::fstream::in);
	if(!fin)
		return;

	ssize_t read;
	int line_nbr;
	std::string line;

	while ((read = getline(line, fin)) >= 0)
	{
		line_nbr++;
		if(line.size() == 0 || line.at(0) == '#' )
			continue;

		/* TODO: Problem if key has a = in name */
		std::string::size_type equ_pos = line.find('=',0);
		if(equ_pos == std::string::npos)
		{
			log[W_ERR] << "SessionConfig: Wrong format on line " << line_nbr;
			continue;
		}

		Parse(line);
	}

	fin.close();
	log[W_INFO] << "SessionConfig: Loaded " << filename;
}

bool SessionConfig::Save()
{
	BlockLockMutex lock(this);

	/* Not loaded */
	if(this->filename.empty())
		return false;

	std::fstream fout;
	fout.open(filename.c_str(), std::fstream::out);
	if(!fout)
	{
		// Save is called by SessionConfig's destructor
		// the log functions may not be available at this moment
		//log[W_ERR] << "SessionConfig: Unable to save config file to " << filename;
		std::cerr << "SessionConfig: Unable to save config file to " << filename << std::endl;
		return false;
	}

	for(std::map<std::string, std::string>::iterator it = list.begin();
		it != list.end();
		++it)
	{
		fout << it->first << "=" << it->second << std::endl;
	}
	fout.close();
	std::cout << "SessionConfig: Config saved in " << filename << std::endl;
	//log[W_INFO] << "SessionConfig: Config saved in " << filename;
	return true;
}

void SessionConfig::Parse(const std::string& line)
{
	BlockLockMutex lock(this);
	std::string::size_type equ_pos = line.find('=',0);
	if(equ_pos == std::string::npos)
		return;

	std::string opt = line.substr(0, equ_pos);
	std::string val = line.substr(equ_pos+1);

	Set(opt, val);
}

void SessionConfig::Display()
{
	BlockLockMutex lock(this);
	for(std::map<std::string, std::string>::iterator it = list.begin();
		it != list.end();
		++it)
	log[W_INFO] << it->first << ":" << it->second;
}
