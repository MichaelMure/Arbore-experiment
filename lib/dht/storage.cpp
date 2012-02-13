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

#include "storage.h"
#include <util/time.h>

void Storage::addInfo(const Key& k, const Data* data)
{
	if(!data)
		return;

	if(data->getDataType() == STRING_LIST)
	{
		DataString *ds = (DataString*) data;
		for(DataString::NameSet::const_iterator it = ds->begin(); it != ds->end(); it++)
			addInfo(k,*it);
	}
	else if(data->getDataType() == KEY_LIST)
	{
		DataKey *dk = (DataKey*) data;
		for(DataKey::KeySet::const_iterator it = dk->begin(); it != dk->end(); it++)
			addInfo(k,*it);
	}
}

	void Storage::addInfo(const Key& k, const Key& info)
	{
		if(hasKey(k) && !isKeyList(k))
			throw WrongDataType();

		std::map<Key,Data*>::iterator it;
		it = dataMap_.find(k);
		if(it!=dataMap_.end())
		{
			DataKey *data_key = (DataKey*) it->second;
			data_key->add(info);
		}
		else
		{
			Data *data_info = new DataKey(info);
			dataMap_[k] = data_info;
		}
	}

	void Storage::addInfo(const Key& k, const std::string& info)
	{
		if(hasKey(k) && !isStringList(k))
			throw WrongDataType();

		std::map<Key,Data*>::iterator it;
		it = dataMap_.find(k);
		if(it!=dataMap_.end())
		{
			DataString *data_string = (DataString*) it->second;
			data_string->add(info);
		}
		else
		{
			Data *data_info = new DataString(info);
			dataMap_[k] = data_info;
		}
	}

void Storage::removeInfo(const Key& k, const Data* data)
{
	if(!data)
		return;

	if(data->getDataType() == STRING_LIST)
	{
		DataString *ds = (DataString*) data;
		for(DataString::NameSet::const_iterator it = ds->begin(); it != ds->end(); it++)
			removeInfo(k,*it);
	}
	else if(data->getDataType() == KEY_LIST)
	{
		DataKey *dk = (DataKey*) data;
		for(DataKey::KeySet::const_iterator it = dk->begin(); it != dk->end(); it++)
			removeInfo(k,*it);
	}
}

	void Storage::removeInfo(Key k, Key info)
	{
		if(hasKey(k) && !isKeyList(k))
			throw WrongDataType();

		std::map<Key,Data*>::iterator it;
		it = dataMap_.find(k);
		if(it!=dataMap_.end())
		{
			DataKey *data_key = (DataKey*) it->second;
			data_key->remove(info);
			if(data_key->isEmpty())
				removeKey(k);
		}
	}

	void Storage::removeInfo(Key k, std::string info)
	{
		if(hasKey(k) && !isStringList(k))
			throw WrongDataType();

		std::map<Key,Data*>::iterator it;
		it = dataMap_.find(k);
		if(it!=dataMap_.end())
		{
			DataString *data_string = (DataString*) it->second;
			data_string->remove(info);
			if(data_string->isEmpty())
				removeKey(k);
		}
	}

	bool Storage::isKeyList(Key k) const
	{
		DataMap::const_iterator it = dataMap_.find(k);
		if(it == dataMap_.end())
			return false;
		return it->second->getDataType()==KEY_LIST;
	}

	bool Storage::isStringList(Key k) const
	{
		DataMap::const_iterator it = dataMap_.find(k);
		if(it == dataMap_.end())
			return false;
		return it->second->getDataType()==STRING_LIST;
	}

	void Storage::removeKey(Key k)
	{
		if(!hasKey(k))
			throw UnknowKey();

		std::map<Key,Data*>::iterator it;
		it = dataMap_.find(k);
		if(it!=dataMap_.end())
			dataMap_.erase(it);
	}

	bool Storage::hasKey(Key k) const
	{
		return dataMap_.count(k) > 0;
	}

	Data* Storage::getInfo(Key k) const
	{
		if(!hasKey(k))
			throw UnknowKey();

		std::map<Key,Data*>::const_iterator it;
		it = dataMap_.find(k);
		if(isKeyList(k))
		{
			DataKey *data_key = (DataKey*) it->second;
			return data_key;
		}
		else
		{
			DataString *data_string = (DataString*) it->second;
			return data_string;
		}
	}

	void Storage::clean()
	{
		uint32_t count = 0;
		std::map<Key,Data*>::iterator it;
		for (it=dataMap_.begin() ; it != dataMap_.end(); it++)
		{
			if(it->second->isOld())
			{
				dataMap_.erase(it);
				count++;
			}
		}
		pf_log[W_DHT] << "Storage: " << count << " value(s) cleaned.";
	}

	void Storage::clear()
	{
		dataMap_.clear();
	}

	std::string Storage::GetStr() const
	{
		std::string str;
		std::map<Key,Data*>::const_iterator it;
		for (it=dataMap_.begin() ; it != dataMap_.end(); it++)
		{
			str += "Key : " + it->first.GetStr() + it->second->GetStr();
		}
		return str;
	}
