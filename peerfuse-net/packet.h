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
 * 
 */

#ifndef PACKET_H
#define PACKET_H

#include <vector>
#include <cassert>
#include <string>
#include "pf_log.h"
#include "net/packet_base.h"

class Packet : public PacketBase
{
	pf_id id_src;				  // 0 means want an id
	pf_id id_dst;				  // 0 means "to everybody"

	char* DumpBuffer() const;
public:

	/* Constructors */
	Packet(msg_type _type, pf_id src = 0, pf_id dst = 0);
	Packet(const Packet& packet);
	Packet& operator=(const Packet& packet);
	Packet(char* header);

	pf_id GetSrcID() const { return id_src; }
	pf_id GetDstID() const { return id_dst; }
	Packet& SetSrcID(pf_id id) { id_src = id; return *this; }
	Packet& SetDstID(pf_id id) { id_dst = id; return *this; }

	std::string GetPacketInfo() const;
};
#endif						  /* PACKET_H */
