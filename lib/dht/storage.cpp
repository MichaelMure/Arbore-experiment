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

	void Storage::addInfo(Key k, Key info)
	{
		//if(isKeyList(k))

	}

	void Storage::addInfo(Key k, std::string info)
	{

	}

	void Storage::removeInfo(Key k, Key info)
	{

	}
	void Storage::removeInfo(Key k, std::string info)
	{

	}
	bool Storage::isKeyList(Key k) const
	{
		return dataMap_.find(k)->second->getDataType()==KEY_LIST;
	}
	bool Storage::isStringList(Key k) const
	{
		return dataMap_.find(k)->second->getDataType()==STRING_LIST;
	}
	void Storage::removeKey(Key k)
	{

	}
	bool Storage::hasKey(Key k) const
	{

	}
	Data* Storage::getInfo(Key k) const
	{

	}
	void Storage::clean()
	{

	}
	void Storage::clear()
	{

	}
