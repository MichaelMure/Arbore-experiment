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
#include <util/pf_log.h>

/** The storage aim is to store the DHT data.
 * There are 2 types of data : keyList or stringList
 * keys are used for filechunks
 * strings are used for directories and files
 */

class Storage
{
public:

	Storage() {};
	virtual ~Storage() {};

	/** Add information (key) in the dataMap
	 *
	 * If there is already an entry with the key k,
	 * add the information in the data.
	 * If not, a entry is created in dataMap with the key k and the info.
	 */
	void addInfo(const Key& k, const Key& info);

	/** Add information (string) in the dataMap
	 *
	 * If there is already an entry with the key k,
	 * add the information in the data.
	 * If not, a entry is created in dataMap with the key k and the info.
	 */
	void addInfo(const Key& k, const std::string& info);

	/** Remove the info (key) from the dataMap
	 *
	 * If the information was the last one for the associated key,
	 * the dataMap entry is also removed
	 */

	void removeInfo(Key k, Key info);

		/** Remove the info (string) from the dataMap
	 *
	 * If the information was the last one for the associated key,
	 * the dataMap entry is also removed
	 */
	void removeInfo(Key k, std::string info);

/** @return true if the data associated with the @param key is a DataKey */
	bool isKeyList(Key k) const;

/** @return true if the data associated with the @param key is a DataString */
	bool isStringList(Key k) const;

/** Remove an entry from the dataMap
 *
 * key and associated data are removed
 */
	void removeKey(Key k);

/** @return true if the dataMap have an entry for this key */
	bool hasKey(Key k) const;

/** @return the dataList associated with the key */
	Data* getInfo(Key k) const;

/** Removed all entries from de the dataMap which are obsolete */
	void clean();

/** Removed all entries from the dataMap */
	void clear();
	std::string GetStr() const;

/* Exceptions */
	class WrongDataType : public std::exception {};
	class UnknowKey : public std::exception {};

private:
	std::map<Key, Data*> dataMap_;
};

template<>
inline Log::flux& Log::flux::operator<< <Storage> (Storage stor)
{
	_str += stor.GetStr();
	return *this;
}

#endif
