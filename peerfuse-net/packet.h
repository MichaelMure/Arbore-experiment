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

#ifndef PACKET_H
#define PACKET_H

#include <vector>
#include <cassert>
#include <string>
#include "log.h"
#include "packet_base.h"

class Packet : public PacketBase
{
	id_t id_src;				  // 0 means want an id
	id_t id_dst;				  // 0 means "to everybody"

	char* DumpBuffer() const;
public:

	/* Constructors */
	Packet(msg_type _type, id_t src = 0, id_t dst = 0);
	Packet(const Packet& packet);
	Packet& operator=(const Packet& packet);
	Packet(char* header);

	id_t GetSrcID() const { return id_src; }
	id_t GetDstID() const { return id_dst; }
	Packet& SetSrcID(id_t id) { id_src = id; return *this; }
	Packet& SetDstID(id_t id) { id_dst = id; return *this; }

	virtual void Send(Connection* conn);
};
#endif						  /* PACKET_H */
