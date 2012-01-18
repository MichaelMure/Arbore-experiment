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

#ifndef DATASTRING_H
#define DATASTRING_H

#include "data.h"
#include <set>
#include <string>
#include <util/pf_log.h>

class DataString : public Data
{
public:
	DataString(std::string name);
	void add(std::string name);
	void remove(std::string name);
	int getSize() const;
	data_type getDataType() const;
	bool isEmpty() const;
	std::string GetStr() const;


private:
	std::set<std::string> nameList_;
};

template<>
inline Log::flux& Log::flux::operator<< <DataString> (DataString ds)
{
	_str += ds.GetStr();
	return *this;
}

#endif
