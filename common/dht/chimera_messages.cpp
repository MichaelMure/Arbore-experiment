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

#include "dtime.h"
#include "chimera_messages.h"
#include "chimera_routing.h"
#include "pf_log.h"
#include "net/network.h"
#include "net/packet_handler.h"
#include "chimera.h"

class ChimeraBaseMessage : public PacketHandlerBase
{
public:
	void operator() (PacketTypeList& pckt_type_list, const Host& sender, const Packet& pckt)
	{
		ChimeraDHT& chimera = dynamic_cast<ChimeraDHT&>(pckt_type_list);

		if(pckt.HasFlag(Packet::MUSTROUTE))
		{
			if(chimera.Route(pckt))
				return;
		}

		Handle(chimera, sender, pckt);
	}

	virtual void Handle (ChimeraDHT& chimera, const Host& sender, const Packet& pckt) = 0;
};

class ChimeraJoinMessage : public ChimeraBaseMessage
{
public:
	void Handle (ChimeraDHT& chimera, const Host& sender, const Packet& pckt)
	{
		pf_addr addr = pckt.GetArg<pf_addr>(NET_JOIN_ADDRESS);
		Host host = chimera.GetNetwork()->GetHostsList()->GetHost(addr);

		if((dtime() - host.GetFailureTime()) < ChimeraDHT::GRACEPERIOD)
		{
			Packet pckt(ChimeraJoinNAckType, chimera.GetMe().GetKey(), host.GetKey());
			pckt.SetArg(NET_JOIN_NACK_ADDRESS, addr);
			chimera.Send(host, pckt);

			pf_log[W_WARNING] << "JOIN request from node " << host << " rejected, "
			                  << "elapsed time since failure = " << dtime() - host.GetFailureTime() << " sec";
			return;
		}

		std::vector<Host> leafset = chimera.GetRouting()->getLeafset();
		AddrList addresses;
		for(std::vector<Host>::iterator it = leafset.begin(); it != leafset.end(); ++it)
			addresses.push_back(it->GetAddr());

		Packet join_ack(ChimeraJoinAckType, chimera.GetMe().GetKey(), host.GetKey());
		join_ack.SetArg(NET_JOIN_ACK_ADDRESSES, addresses);

		if(!chimera.Send(host, join_ack))
			pf_log[W_WARNING] << "Send join ACK message failed!";

	}
};

class ChimeraJoinAckMessage : public ChimeraBaseMessage
{
public:
	void Handle (ChimeraDHT& chimera, const Host& sender, const Packet& pckt)
	{

	}
};

class ChimeraJoinNAckMessage : public ChimeraBaseMessage
{
public:
	void Handle (ChimeraDHT& chimera, const Host& sender, const Packet& pckt)
	{

	}
};

class ChimeraUpdateMessage : public ChimeraBaseMessage
{
public:
	void Handle (ChimeraDHT& chimera, const Host& sender, const Packet& pckt)
	{
		pf_addr addr = pckt.GetArg<pf_addr>(NET_UPDATE_ADDRESS);
		Host host = chimera.GetNetwork()->GetHostsList()->GetHost(addr);
		chimera.GetRouting()->add(host);
	}
};

class ChimeraPiggyMessage : public ChimeraBaseMessage
{
public:
	void Handle (ChimeraDHT& chimera, const Host& sender, const Packet& pckt)
	{
		AddrList address = pckt.GetArg<AddrList>(NET_PIGGY_ADDRESSES);
		for(AddrList::iterator it = address.begin(); it != address.end(); ++it)
		{
			Host host = chimera.GetNetwork()->GetHostsList()->GetHost(*it);
			if(dtime() - host.GetFailureTime() > ChimeraDHT::GRACEPERIOD)
				chimera.GetRouting()->add(host);
			else
				pf_log[W_WARNING] << "Refused to add " << host << " to routing table";
		}
	}
};

class ChimeraPingMessage : public ChimeraBaseMessage
{
public:
	void Handle (ChimeraDHT& chimera, const Host& sender, const Packet& pckt)
	{
		chimera.GetNetwork()->GetHostsList()->GetHost(pckt.GetArg<pf_addr>(NET_PING_ME));
	}
};

PacketType     ChimeraJoinType(1, new ChimeraJoinMessage,     "JOIN",      /* NET_JOIN_ADDRESS */    T_ADDR,
                                                                                                     T_END);
PacketType  ChimeraJoinAckType(2, new ChimeraJoinAckMessage,  "JOIN_ACK",  T_END);
PacketType   ChimeraUpdateType(3, new ChimeraUpdateMessage,   "UPDATE",    /* NET_UPDATE_ADDRESS */  T_ADDR,
                                                                                                     T_END);
PacketType    ChimeraPiggyType(4, new ChimeraPiggyMessage,    "PIGGY",     /* NET_PIGGY_ADDRESSES */ T_ADDRLIST,
                                                                                                     T_END);
PacketType ChimeraJoinNAckType(5, new ChimeraJoinNAckMessage, "JOIN_NACK", T_END);
PacketType     ChimeraPingType(6, new ChimeraPingMessage,     "PING",      /* NET_PING_ME */ T_ADDR,
                                                                                             T_END);

