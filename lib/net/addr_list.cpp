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

#include "addr_list.h"

addr_list::addr_list(char* buff)
{
	uint32_t list_size = Netutil::ReadInt32(buff);

	buff += Netutil::size(list_size);

	for(uint32_t i = 0; i < list_size; ++i)
	{
		pf_addr addr = pf_addr(buff);
		this->push_back(addr);
		buff += pf_addr::size;
	}
}

void addr_list::dump(char* buff)
{
	uint32_t s = (uint32_t)this->size();
	Netutil::dump(s, buff);

	buff += Netutil::size(s);

	for(addr_list::const_iterator it = this->begin(); it != this->end(); ++it)
	{
		it->dump(buff);
		buff += pf_addr::size;
	}
}
