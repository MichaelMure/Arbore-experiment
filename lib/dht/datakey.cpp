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

DataKey::DataKey(Key k)
	: Data()
{
	keyList_.insert(k);
}

void DataKey::add(Key k)
{
	keyList_.insert(k);
	updateTime_=time::dtime();
}

void DataKey::remove(Key k)
{
	keyList_.erase(k);
}

size_t DataKey::getSize() const
{
	return keyList_.size();
}

DataType DataKey::getDataType() const
{
	return KEY_LIST;
}

bool DataKey::isEmpty() const
{
	return keyList_.empty();
}

void DataKey::dump(char* buff) const
{
	std::set<Key>::const_iterator it;
	for (it=keyList_.begin() ; it != keyList_.end(); it++)
	{
		it->dump(buff);
	}
}

std::string DataKey::GetStr() const
{
	std::string str;
	std::set<Key>::const_iterator it;
	for (it=keyList_.begin() ; it != keyList_.end(); it++)
	{
		str += "Key :" + it->GetStr() + " , ";
	}
	return str;
}
