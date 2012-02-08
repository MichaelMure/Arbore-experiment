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
#include <net/netutil.h>

DataString::DataString(std::string name):Data()
{
	nameList_.insert(name);
}

DataString::DataString(char* buff)
{
	uint32_t s = Netutil::ReadInt32(buff);
	buff += Netutil::getSerialisedSize(s);
	for (uint32_t i=0 ; i < s; i++)
	{
		std::string str = Netutil::ReadStr(buff);
		nameList_.insert(str);
		buff += Netutil::getSerialisedSize(str);
	}
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

size_t DataString::getSize() const
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

void DataString::dump(char* buff) const
{
	uint32_t type = (uint32_t) this->getDataType();
	Netutil::dump(type, buff);
	buff += Netutil::getSerialisedSize(type);
	uint32_t s = (uint32_t) this->getSize();
	Netutil::dump(s, buff);
	buff += Netutil::getSerialisedSize(s);
	std::set<std::string>::const_iterator it;
	for (it=nameList_.begin() ; it != nameList_.end(); it++)
	{
		Netutil::dump(*it, buff);
		buff += Netutil::getSerialisedSize(*it);
	}
}

size_t DataString::getSerialisedSize() const
{
	uint32_t len = (uint32_t) this->getSize();
	size_t s = Netutil::getSerialisedSize(len);
	std::set<std::string>::const_iterator it;
	for (it=nameList_.begin() ; it != nameList_.end(); it++)
	{
		s += Netutil::getSerialisedSize(*it);
	}
	return s;
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

DataString::NameList::const_iterator DataString::begin() const
{
	return nameList_.begin();
}

DataString::NameList::const_iterator DataString::end() const
{
	return nameList_.end();
}
