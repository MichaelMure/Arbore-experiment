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

#include "datakey.h"
#include <util/time.h>
#include <net/netutil.h>

DataKey::DataKey(Key k)
	: Data()
{
	keySet_.insert(k);
}

DataKey::DataKey(char* buff)
{
	uint32_t s = Netutil::ReadInt32(buff);
	buff += Netutil::getSerialisedSize(s);
	for (uint32_t i=0 ; i < s; i++)
	{
		Key k = Key(buff);
		keySet_.insert(k);
		buff += Key::size;
	}
}

void DataKey::add(Key k)
{
	keySet_.insert(k);
	updateTime_=time::dtime();
}

void DataKey::remove(Key k)
{
	keySet_.erase(k);
}

size_t DataKey::getSize() const
{
	return keySet_.size();
}

DataType DataKey::getDataType() const
{
	return KEY_LIST;
}

bool DataKey::isEmpty() const
{
	return keySet_.empty();
}

void DataKey::dump(char* buff) const
{
	/* Type */
	uint32_t type = (uint32_t) this->getDataType();
	Netutil::dump(type, buff);
	buff += Netutil::getSerialisedSize(type);

	/* Number of keys */
	uint32_t s = (uint32_t) this->getSize();
	Netutil::dump(s, buff);
	buff += Netutil::getSerialisedSize(s);

	/* Keys */
	KeySet::const_iterator it;
	for (it=keySet_.begin() ; it != keySet_.end(); it++)
	{
		it->dump(buff);
		buff += Key::size;
	}
}

size_t DataKey::getSerialisedSize() const
{
	return sizeof(uint32_t) /* Type */
	     + sizeof(uint32_t) /* Number of keys */
	     + keySet_.size() * Key::size; /* Keys */
}

std::string DataKey::GetStr() const
{
	std::string str;
	KeySet::const_iterator it;

	for (it=keySet_.begin() ; it != keySet_.end(); it++)
	{
		if(str.size())
			str += ",";
		str += *it;
	}
	return "Keys[" + str + "]";
}

DataKey::KeySet::const_iterator DataKey::begin() const
{
	return keySet_.begin();
}

DataKey::KeySet::const_iterator DataKey::end() const
{
	return keySet_.end();
}
