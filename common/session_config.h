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
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * $Id$
 */

#ifndef SESSION_CONFIG_H
#define SESSION_CONFIG_H
#include <map>
#include <string>
#include "pf_log.h"
#include "mutex.h"
#include "tools.h"

class SessionConfig : private Mutex
{
	std::string filename;

	std::map<std::string, std::string> list;

	void Parse(const std::string& line);
public:
	SessionConfig();
	~SessionConfig();

	void Load(const std::string& file);
	bool Save();
	void Display();

	template<class T>
		bool Get(const std::string& opt, T& val)
	{
		BlockLockMutex lock(this);
		std::map<std::string, std::string>::iterator it;
		it = list.find(opt);
		if(it == list.end())
			return false;

		val = StrToTyp<T>(it->second);
		return true;
	}

	template<class T>
		void Set(const std::string& opt, const T& val)
	{
		BlockLockMutex lock(this);

		list[opt] = TypToStr(val);
	}

};

extern SessionConfig session_cfg;
extern SessionConfig tree_cfg;
#endif
