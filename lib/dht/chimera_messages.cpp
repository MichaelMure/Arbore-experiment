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

#include <util/pf_log.h>
#include <util/dtime.h>
#include <net/network.h>
#include <net/packet.h>
#include <net/packet_handler.h>
#include <scheduler/scheduler_queue.h>

#include "chimera_messages.h"
#include "chimera_routing.h"
#include "chimera.h"
#include "check_leafset_job.h"

void ChimeraBaseMessage::operator() (PacketTypeList& pckt_type_list, const Host& sender, const Packet& pckt)
{
	ChimeraDHT& chimera = dynamic_cast<ChimeraDHT&>(pckt_type_list);

	if(pckt.HasFlag(Packet::MUSTROUTE))
	{
		if(chimera.Route(pckt))
			return;
	}

	Handle(chimera, sender, pckt);
}


class ChimeraJoinMessage : public ChimeraBaseMessage
{
public:
	void Handle (ChimeraDHT& chimera, const Host& sender, const Packet& pckt)
	{
		pf_addr addr = pckt.GetArg<pf_addr>(CHIMERA_JOIN_ADDRESS);
		Host host = chimera.GetNetwork()->GetHostsList()->GetHost(addr);

		if((dtime() - host.GetFailureTime()) < ChimeraDHT::GRACEPERIOD)
		{
			Packet pckt(ChimeraJoinNAckType, chimera.GetMe().GetKey(), host.GetKey());
			pckt.SetArg(CHIMERA_JOIN_NACK_ADDRESS, addr);
			chimera.Send(host, pckt);

			pf_log[W_WARNING] << "JOIN request from node " << host << " rejected, "
			                  << "elapsed time since failure = " << dtime() - host.GetFailureTime() << " sec";
			return;
		}

		std::vector<Host> leafset = chimera.GetRouting()->getLeafset();
		AddrList addresses;
		for(std::vector<Host>::iterator it = leafset.begin(); it != leafset.end(); ++it)
			addresses.push_back(it->GetAddr());
		addresses.push_back(chimera.GetMe().GetAddr());

		Packet join_ack(ChimeraJoinAckType, chimera.GetMe().GetKey(), host.GetKey());
		join_ack.SetArg(CHIMERA_JOIN_ACK_ADDRESSES, addresses);

		if(!chimera.Send(host, join_ack))
			pf_log[W_WARNING] << "Send join ACK message failed!";
	}
};

class ChimeraJoinAckMessage : public ChimeraBaseMessage
{
public:
	void Handle (ChimeraDHT& chimera, const Host& sender, const Packet& pckt)
	{
		AddrList addresses = pckt.GetArg<AddrList>(CHIMERA_JOIN_ACK_ADDRESSES);
		std::vector<Host> hosts;

		for(AddrList::iterator it = addresses.begin(); it != addresses.end(); ++it)
		{
			Host host = chimera.GetNetwork()->GetHostsList()->GetHost(*it);
			chimera.GetRouting()->add(host);

			Packet update(ChimeraUpdateType, chimera.GetMe().GetKey(), host.GetKey());
			update.SetArg(CHIMERA_UPDATE_ADDRESS, chimera.GetMe().GetAddr());

			if(!chimera.Send(host, update))
				pf_log[W_WARNING] << "ChimeraJoinAck: failed to update " << host;
		}

		hosts = chimera.GetRouting()->getRoutingTable();
		for(std::vector<Host>::iterator it = hosts.begin(); it != hosts.end(); ++it)
		{
			Host host = *it;
			Packet update(ChimeraUpdateType, chimera.GetMe().GetKey(), host.GetKey());

			update.SetArg(CHIMERA_UPDATE_ADDRESS, chimera.GetMe().GetAddr());
			if(!chimera.Send(host, update))
				pf_log[W_WARNING] << "ChimeraJoinAck: failed to update " << host;
		}

		/* Start the check of leafset repeated job. */
		scheduler_queue.Queue(new CheckLeafsetJob(&chimera, chimera.GetRouting()));
	}
};

class ChimeraJoinNAckMessage : public ChimeraBaseMessage
{
public:
	void Handle (ChimeraDHT& chimera, const Host& sender, const Packet& pckt)
	{
		pf_addr addr = pckt.GetArg<pf_addr>(CHIMERA_JOIN_NACK_ADDRESS);
		Host host = chimera.GetNetwork()->GetHostsList()->GetHost(addr);

		pf_log[W_WARNING] << "JOIN request rejected from " << host;
		sleep(ChimeraDHT::GRACEPERIOD);
		pf_log[W_WARNING] << "Re-sending JOIN message to " << host;

		chimera.Join(host);
	}
};

class ChimeraUpdateMessage : public ChimeraBaseMessage
{
public:
	void Handle (ChimeraDHT& chimera, const Host& sender, const Packet& pckt)
	{
		pf_addr addr = pckt.GetArg<pf_addr>(CHIMERA_UPDATE_ADDRESS);
		Host host = chimera.GetNetwork()->GetHostsList()->GetHost(addr);
		chimera.GetRouting()->add(host);
	}
};

class ChimeraPiggyMessage : public ChimeraBaseMessage
{
public:
	void Handle (ChimeraDHT& chimera, const Host& sender, const Packet& pckt)
	{
		AddrList address = pckt.GetArg<AddrList>(CHIMERA_PIGGY_ADDRESSES);
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
		chimera.GetNetwork()->GetHostsList()->GetHost(pckt.GetArg<pf_addr>(CHIMERA_PING_ME));
	}
};

class ChimeraPublishMessage : public ChimeraBaseMessage
{
public:
	void Handle (ChimeraDHT& chimera, const Host& sender, const Packet& pckt)
	{
	}
};

class ChimeraUnpublishMessage : public ChimeraBaseMessage
{
public:
	void Handle (ChimeraDHT& chimera, const Host& sender, const Packet& pckt)
	{
	}
};

PacketType      ChimeraJoinType(CHIMERA_JOIN,      new ChimeraJoinMessage,      Packet::REQUESTACK|
                                                                                Packet::MUSTROUTE,   "JOIN",        /* CHIMERA_JOIN_ADDRESS */    T_ADDR,
                                                                                                                                                  T_END);
PacketType   ChimeraJoinAckType(CHIMERA_JOIN_ACK,  new ChimeraJoinAckMessage,   Packet::REQUESTACK,  "JOIN_ACK", /* CHIMERA_JOIN_ACK_ADDRESSES */ T_ADDRLIST,
                                                                                                                                                  T_END);
PacketType    ChimeraUpdateType(CHIMERA_UPDATE,    new ChimeraUpdateMessage,    Packet::REQUESTACK,  "UPDATE",      /* CHIMERA_UPDATE_ADDRESS */  T_ADDR,
                                                                                                                                                  T_END);
PacketType     ChimeraPiggyType(CHIMERA_PIGGY,     new ChimeraPiggyMessage,     Packet::REQUESTACK,  "PIGGY",       /* CHIMERA_PIGGY_ADDRESSES */ T_ADDRLIST,
                                                                                                                                                  T_END);
PacketType  ChimeraJoinNAckType(CHIMERA_JOIN_NACK, new ChimeraJoinNAckMessage,  Packet::REQUESTACK,  "JOIN_NACK", /* CHIMERA_JOIN_NACK_ADDRESS */ T_ADDR,
                                                                                                                                                  T_END);
PacketType      ChimeraPingType(CIHMERA_PING,      new ChimeraPingMessage,      Packet::REQUESTACK,  "PING",                /* CHIMERA_PING_ME */ T_ADDR,
                                                                                                                                                  T_END);
PacketType   ChimeraPublishType(CHIMERA_PUBLISH,   new ChimeraPublishMessage,   Packet::REQUESTACK,  "PUBLISH",                                   T_END);
PacketType ChimeraUnpublishType(CHIMERA_UNPUBLISH, new ChimeraUnpublishMessage, Packet::REQUESTACK,  "UNPUBLISH",                                 T_END);

