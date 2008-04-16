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
#include <typeinfo>
#include "log.h"
#include "mutex.h"

class SessionConfigValueBase
{
	std::string type;
public:

	SessionConfigValueBase(std::string _type)
		: type(_type)
		{}
	virtual ~SessionConfigValueBase() {}

	virtual std::string GetAsString() const = 0;
	std::string GetType() const { return type; }
};

template<class A>
class SessionConfigValue : public SessionConfigValueBase
{
	std::string opt;
	A value;
	SessionConfigValue();
public:

	SessionConfigValue(std::string _opt, A _value)
		: SessionConfigValueBase(typeid(_value).name()),
		opt(_opt),
		value(_value)
		{}

	void Set(const A& val)
	{
		log[W_DEBUG] << "Setting option " << opt << "=" << val;
		value = val;
	}
	A Get() const { return value; }

	std::string GetAsString() const
	{
		std::stringstream out;
		out << value;
		return out.str();
	};

};

class SessionConfig : private Mutex
{
	std::string filename;

	std::map<std::string, SessionConfigValueBase*> list;

	void Save();
	void Parse(const std::string& line);
public:
	SessionConfig();
	~SessionConfig();

	void Load(const std::string& file);
	void Display();

	template<class T>
		bool Get(const std::string& opt, T& val)
	{
		BlockLockMutex lock(this);
		std::map<std::string, SessionConfigValueBase*>::iterator it;
		it = list.find(opt);
		if(it == list.end())
			return false;

		if(it->second->GetType() != typeid(val).name())
		{
			log[W_DEBUG] << "Trying to retrieve \"" << opt << "\" with a different type";
			return false;
		}

		val = dynamic_cast<SessionConfigValue<T>*>(it->second)->Get();
		return true;
	}

	template<class T>
		void Set(const std::string& opt, const T& val)
	{
		BlockLockMutex lock(this);
		std::map<std::string, SessionConfigValueBase*>::iterator it;
		it = list.find(opt);
		if(it == list.end())
			list[opt] = new SessionConfigValue<T>(opt, val);
		else if(it->second->GetType() != typeid(val).name())
		{
			log[W_DEBUG] << "Won't set \"" << opt << "\" as it has a different type";
			return;
		}
		else
			dynamic_cast<SessionConfigValue<T> *>(it->second)->Set(val);
	}

};

extern SessionConfig session_cfg;
extern SessionConfig tree_cfg;
#endif
