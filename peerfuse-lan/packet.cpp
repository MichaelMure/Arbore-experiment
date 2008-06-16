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
 * $Id$
 */

#include "packet.h"
#include <string.h>
#include <netinet/in.h>				  // htonl, ntohl

Packet::Packet(char* header) :
			PacketBase(NET_NONE)
{
	uint32_t* h = (uint32_t*)header;
	type = (msg_type)ntohl(h[0]);
	size = ntohl(h[1]);

	datas = new char [size];
}

char* Packet::DumpBuffer() const
{
	char* dump = new char [GetSize()];
	uint32_t _type = htonl(type);
	uint32_t _size = htonl(size);
	char* ptr = dump;
	memcpy(ptr, &_type, sizeof(_type));
	ptr += sizeof _type;
	memcpy(ptr, &_size, sizeof(_size));
	ptr += sizeof _size;
	memcpy(ptr, datas, GetDataSize());
	return dump;
}
