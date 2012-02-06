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

#ifndef DATA_H
#define DATA_H

#include <string>

/* Time to live for data, 3600 secondes = 1h */
#define DATA_TTL 3600.0

enum DataType
{
	STRING_LIST = 1,
	KEY_LIST    = 2,
	RESERVED1   = 3,
	RESERVED2   = 4,
	RESERVED3   = 5,
	RESERVED4   = 6
};

class Data
{
public:
	Data* createData(char* buff);
	virtual ~Data() {};
	/** @return the type of the data */
	virtual DataType getDataType() const =0;
	/** @return true if the data is empty */
	virtual bool isEmpty() const =0;
	/** @return the number of elements in the data */
	virtual size_t getSize() const =0;
	/** @return true if the TTL is expired */
	bool isOld() const;
	/** @return the size of the seralised data */
	virtual size_t getSerialisedSize() const=0;
	/** Serialize the data in binary format */
	virtual void dump(char* buff) const =0;
	virtual std::string GetStr() const =0;

protected:
	Data();
	double updateTime_;

};

#endif
