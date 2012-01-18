/*
 * Copyright(C) 2012 Benoît Saccomano
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
 * This file contains some code from the Chimera's Distributed Hash Table,
 * written by CURRENT Lab, UCSB.
 *
 */

#ifndef STORAGE_H
#define STORAGE_H

#include <map>
#include "data.h"
#include "datakey.h"
#include "datastring.h"
#include <string>
#include <util/key.h>

class Storage
{
public:

	Storage() {};
	virtual ~Storage();
	void addInfo(Key k, Key info);
	void addInfo(Key k, std::string info);
	void removeInfo(Key k, Key info);
	void removeInfo(Key k, std::string info);
	bool isKeyList(Key k) const;
	bool isStringList(Key k) const;
	void removeKey(Key k);
	bool hasKey(Key k) const;
	Data* getInfo(Key k) const;
	void clean();
	void clear();

/*Exceptions */
	class WrongDataType : public std::exception {};
	class UnknowKey : public std::exception {};

private:
	std::map<Key, Data*> dataMap_;

	};



#endif
