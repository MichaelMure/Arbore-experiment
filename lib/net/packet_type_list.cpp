/*
 * Copyright(C) 2008 Laurent Defert, Romain Bignon
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
 */

#include <cassert>

#include <util/pf_log.h>

#include "packet_type_list.h"

PacketTypeList packet_type_list;

void PacketTypeList::RegisterType(PacketType type)
{
	BlockLockMutex lock(this);

	assert(find(type.GetType()) == end());
	pf_log[W_DEBUG] << "Register " << type.GetName() << "(" << type.GetType() << ")";
	insert(value_type(type.GetType(), type));
}

ssize_t PacketTypeList::size() const
{
	BlockLockMutex lock(this);
	return PacketTypeMap::size();
}

PacketType PacketTypeList::GetPacketType(uint32_t type) const
{
	BlockLockMutex lock(this);

	PacketTypeMap::const_iterator it = find(type);

	if(it == end())
		throw UnknowType();

	return it->second;
}
