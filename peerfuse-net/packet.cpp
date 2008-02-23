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
 * $Id$
 */

#include "packet.h"
#include <netinet/in.h>				  // htonl, ntohl

Packet::Packet(msg_type _type, id_t src, id_t dst)
			: PacketBase(_type),
			id_src(src),
			id_dst(dst)
{
}

Packet::Packet(char* header)
			: PacketBase(NET_NONE)
{
	uint32_t* h = (uint32_t*)header;
	id_src = ntohl(h[0]);
	id_dst = ntohl(h[1]);
	type = (msg_type)ntohl(h[2]);
	size = ntohl(h[3]);

	datas = (char*)malloc(size);
}

Packet::Packet(const Packet& p)
			: PacketBase(p),
			id_src(p.id_src),
			id_dst(p.id_dst)
{
}

Packet& Packet::operator=(const Packet& p)
{
	PacketBase::operator=(p);
	id_src = p.id_src;
	id_dst = p.id_dst;

	return *this;
}

char* Packet::DumpBuffer() const
{
	char* dump = (char*)malloc(GetSize());
	id_t _src = htonl(id_src);
	id_t _dst = htonl(id_dst);
	uint32_t _type = htonl(type);
	uint32_t _size = htonl(size);
	char* ptr = dump;
	memcpy(ptr, &_src, sizeof(_src));
	ptr += sizeof _src;
	memcpy(ptr, &_dst, sizeof(_dst));
	ptr += sizeof _dst;
	memcpy(ptr, &_type, sizeof(_type));
	ptr += sizeof _type;
	memcpy(ptr, &_size, sizeof(_size));
	ptr += sizeof _size;
	memcpy(ptr, datas, GetDataSize());
	return dump;
}

uint32_t Packet::GetHeaderSize()
{
	return sizeof(id_t)			  // id_src
		+ sizeof(id_t)			  // id_dst
		+ sizeof(uint32_t)		  // size of the packet
		+ sizeof(uint32_t);		  // size of the type
}

uint32_t Packet::GetSize() const
{
	return PacketBase::GetSize() - PacketBase::GetHeaderSize() + GetHeaderSize();
}

void Packet::Send(int fd)
{
	PacketBase::Send(fd);
	log[W_PARSE] << "                       srcid=" << id_src << ", " <<
		"dstid=" << id_dst << ", ";

}
