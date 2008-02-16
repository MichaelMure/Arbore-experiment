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
 * $Id$
 */

#ifndef SESSION_CONFIG_H
#define SESSION_CONFIG_H
#include <sstream>
#include <map>
#include <string>
#include "log.h"

template <class A>
class SessionConfigOptList
{
	std::map<std::string, A> lst;
public:
	bool Get(const std::string& opt, A& val)
	{
		typename std::map<std::string, A>::iterator it;
		log[W_INFO] << "Reading option " << opt;
		it = lst.find(opt);
		if(it == lst.end())
			return false;
		val = it->second;
		return true;
	}

	void Set(const std::string& opt, const A& val)
	{
		log[W_INFO] << "Setting option " << opt;
		lst[opt] = val;
	}

	const std::map<std::string, A>& GetMap()
	{
		return lst;
	}

	std::string GetAsString(const std::string& opt);

};

class SessionConfig
{
	std::string filename;

	SessionConfigOptList<std::string> opt_str;
	SessionConfigOptList<uint32_t> opt_uint;

	void Save();
	void Parse(const std::string& line);
public:
	SessionConfig();
	~SessionConfig();

	void Load(const std::string& file);

	bool Get(const std::string& opt, std::string& val) { return opt_str.Get(opt, val); }
	void Set(const std::string& opt, const std::string& val) { return opt_str.Set(opt, val); }
	bool Get(const std::string& opt, uint32_t val) { return opt_uint.Get(opt, val); }
	void Set(const std::string& opt, const uint32_t val) { return opt_uint.Set(opt, val); }
};

extern SessionConfig session_cfg;
#endif
