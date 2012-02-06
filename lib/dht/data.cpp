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

 #include <util/time.h>
 #include "data.h"
 #include <net/netutil.h>
 #include "datastring.h"
 #include "datakey.h"
 #include <net/packet.h>

Data::Data()
	:updateTime_(time::dtime())
{

}

Data* Data::createData(char* buff)
{
	uint32_t type = Netutil::ReadInt32(buff);
	buff += Netutil::getSerialisedSize(type);
	DataType t = (DataType) type;
	switch(t)
	{
		case STRING_LIST:
			return new DataString(buff);
			break;
		case KEY_LIST:
			return new DataKey(buff);
			break;
		case RESERVED1:
		case RESERVED2:
		case RESERVED3:
		case RESERVED4:
		default:
			throw Packet::Malformated();
	}
}

bool Data::isOld() const
{
	double current_time = time::dtime();
	double diff = current_time-updateTime_;
	return diff > DATA_TTL;
}
