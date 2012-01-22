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

#include "datastring.h"
#include <util/time.h>

DataString::DataString(std::string name):Data()
{
	nameList_.insert(name);
}

void DataString::add(std::string name)
{
	nameList_.insert(name);
	updateTime_=time::dtime();
}

void DataString::remove(std::string name)
{
	nameList_.erase(name);
}

int DataString::getSize() const
{
	return nameList_.size();
}

DataType DataString::getDataType() const
{
	return STRING_LIST;
}

bool DataString::isEmpty() const
{
	return nameList_.empty();
}

std::string DataString::GetStr() const
{
	std::string str;
	std::set<std::string>::const_iterator it;
	for (it=nameList_.begin() ; it != nameList_.end(); it++)
	{
		str += "Name :" + *it + " , ";
	}
	return str;
}
