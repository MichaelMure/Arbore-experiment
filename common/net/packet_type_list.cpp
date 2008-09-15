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

#include "packet_type_list.h"

void PacketTypeList::RegisterType(PacketType type)
{
	BlockLockMutex lock(this);
	insert(std::pair<uint32_t, PacketType>(type.GetType(), type));
}

ssize_t PacketTypeList::size() const
{
	BlockLockMutex lock(this);
	return std::map<uint32_t, PacketType>::size();
}

PacketType PacketTypeList::GetPacketType(uint32_t type) const
{
	BlockLockMutex lock(this);
	return at(type);
}

PacketType PacketTypeList::operator[](uint32_t s) const
{
	BlockLockMutex lock(this);
	return at(s);
}

