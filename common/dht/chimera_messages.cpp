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

#include "chimera_messages.h"
#include "net/network.h"
#include "net/packet_handler.h"
#include "chimera.h"

class ChimeraJoinMessage : public PacketHandlerBase
{
public:
	void operator() (PacketTypeList& pckt_type_list, const Host& sender, const Packet& pckt)
	{

	}
};

class ChimeraJoinAckMessage : public PacketHandlerBase
{
public:
	void operator() (PacketTypeList& pckt_type_list, const Host& sender, const Packet& pckt)
	{

	}
};

class ChimeraJoinNAckMessage : public PacketHandlerBase
{
public:
	void operator() (PacketTypeList& pckt_type_list, const Host& sender, const Packet& pckt)
	{

	}
};

class ChimeraUpdateMessage : public PacketHandlerBase
{
public:
	void operator() (PacketTypeList& pckt_type_list, const Host& sender, const Packet& pckt)
	{

	}
};

class ChimeraPiggyMessage : public PacketHandlerBase
{
public:
	void operator() (PacketTypeList& pckt_type_list, const Host& sender, const Packet& pckt)
	{

	}
};

class ChimeraPingMessage : public PacketHandlerBase
{
public:
	void operator() (PacketTypeList& pckt_type_list, const Host& sender, const Packet& pckt)
	{
		ChimeraDHT& chimera = dynamic_cast<ChimeraDHT&>(pckt_type_list);

		chimera.GetNetwork()->GetHostsList()->GetHost(pckt.GetArg<pf_addr>(NET_PING_ME));
	}
};

PacketType     ChimeraJoinType(1, new ChimeraJoinMessage,     "JOIN",      T_END);
PacketType  ChimeraJoinAckType(2, new ChimeraJoinAckMessage,  "JOIN_ACK",  T_END);
PacketType   ChimeraUpdateType(3, new ChimeraUpdateMessage,   "UPDATE",    T_END);
PacketType    ChimeraPiggyType(4, new ChimeraPiggyMessage,    "PIGGY",     /* NET_PIGGY_ADDRESSES */ T_ADDRLIST,
                                                                                                     T_END);
PacketType ChimeraJoinNAckType(5, new ChimeraJoinNAckMessage, "JOIN_NACK", T_END);
PacketType     ChimeraPingType(6, new ChimeraPingMessage,     "PING",      /* NET_PING_ME */ T_ADDR,
                                                                                             T_END);

