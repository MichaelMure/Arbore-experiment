/*
 * Copyright(C) 2008 Romain Bignon
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

#ifndef CHIMERA_MESSAGES_H
#define CHIMERA_MESSAGES_H

#include <stdint.h>

#include "net/packet_type.h"
#include "net/packet_handler.h"

class ChimeraDHT;

class ChimeraBaseMessage : public PacketHandlerBase
{
public:
	void operator() (PacketTypeList& pckt_type_list, const Host& sender, const Packet& pckt);

	virtual void Handle (ChimeraDHT& chimera, const Host& sender, const Packet& pckt) = 0;
};

enum
{
	CHIMERA_JOIN        = 1,
	CHIMERA_JOIN_ACK    = 2,
	CHIMERA_UPDATE      = 3,
	CHIMERA_PIGGY       = 4,
	CHIMERA_JOIN_NACK   = 5,
	CIHMERA_PING        = 7,
	CHIMERA_RESERVED1   = 8,
	CHIMERA_RESERVED2   = 9,
	CHIMERA_RESERVED3   = 10,
	CHIMERA_RESERVED4   = 11,
	CHIMERA_RESERVED5   = 12,
	CHIMERA_RESERVED6   = 13,
	CHIMERA_RESERVED7   = 14,
};

enum
{
	CHIMERA_JOIN_ADDRESS,
};
extern PacketType ChimeraJoinType;

enum
{
	CHIMERA_JOIN_ACK_ADDRESSES,
};
extern PacketType ChimeraJoinAckType;

enum
{
	CHIMERA_UPDATE_ADDRESS,
};
extern PacketType ChimeraUpdateType;

enum
{
	CHIMERA_PIGGY_ADDRESSES
};

extern PacketType ChimeraPiggyType;

enum
{
	CHIMERA_JOIN_NACK_ADDRESS,
};
extern PacketType ChimeraJoinNAckType;

enum
{
	CHIMERA_PING_ME,
};
extern PacketType ChimeraPingType;

#endif /* CHIMERA_MESSAGES_H */
