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

#ifndef ARBORE_MESSAGES_H
#define ARBORE_MESSAGES_H

#include <net/packet_handler.h>
#include "arbore.h"

class ArboreMessage : public PacketHandlerBase
{
public:
	virtual void Handle (Arbore& arbore, const Host& sender, const Packet& pckt) = 0;
	HandlerType getType() { return HANDLER_TYPE_ARBORE; }
};

enum
{
	ARBORE_CHUNK_SEND_CHUNK
};
extern PacketType ArboreChunkSendType;

#endif /* ARBORE_MESSAGES_H */
