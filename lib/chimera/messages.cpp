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
#include <util/time.h>
#include <net/network.h>
#include <net/packet.h>
#include <net/addr_list.h>
#include <net/packet_handler.h>
#include <scheduler/scheduler_queue.h>

#include "messages.h"
#include "routing.h"
#include "chimera.h"
#include "check_leafset_job.h"

class ChimeraJoinMessage : public ChimeraMessage
{
public:
	/** The JOIN message is answered by a JOINACK or a JOINNACK message.
	  * If the transmit time is too long, a JOINNACK is returned.
	  * If not, an JOINACK is answered, with all the peer's addresses
	  * that are in the leafset.
	  */
	void Handle (Chimera& chimera, const Host&, const Packet& pckt)
	{
		pf_addr addr = pckt.GetArg<pf_addr>(CHIMERA_JOIN_ADDRESS);
		Host host = hosts_list.GetHost(addr);

		double elapsed_time = time::dtime() - host.GetFailureTime();

		if(elapsed_time < Chimera::GRACEPERIOD)
		{
			Packet join_nack(ChimeraJoinNAckType, chimera.GetMe().GetKey(), host.GetKey());
			join_nack.SetArg(CHIMERA_JOIN_NACK_ADDRESS, addr);
			chimera.Send(host, join_nack);

			pf_log[W_ROUTING] << "JOIN request from node " << host << " rejected, "
			                  << "elapsed time since failure = " << elapsed_time << " sec";
			return;
		}

		std::vector<Host> leafset = chimera.GetRouting()->getLeafset();
		addr_list addresses;
		for(std::vector<Host>::iterator it = leafset.begin(); it != leafset.end(); ++it)
			addresses.push_back(it->GetAddr());
		addresses.push_back(chimera.GetMe().GetAddr());

		Packet join_ack(ChimeraJoinAckType, chimera.GetMe().GetKey(), host.GetKey());
		join_ack.SetArg(CHIMERA_JOIN_ACK_ADDRESSES, addresses);

		if(!chimera.Send(host, join_ack))
			pf_log[W_ROUTING] << "Send join ACK message failed!";
	}
};

class ChimeraJoinAckMessage : public ChimeraMessage
{
public:
	/** After receiving a JOINACK message, we add the received peers addresses
	  * in the routing (leafset + routing table, depending on the peer).
	  * This trigger the send of an update message to each addresses received
	  * and each host in the routing table.
	  * TODO: don't handle this message after a succesfull join.
	  */
	void Handle (Chimera& chimera, const Host&, const Packet& pckt)
	{
		addr_list addresses = pckt.GetArg<addr_list>(CHIMERA_JOIN_ACK_ADDRESSES);
		std::vector<Host> hosts;

		for(addr_list::iterator it = addresses.begin(); it != addresses.end(); ++it)
		{
			Host host = hosts_list.GetHost(*it);
			chimera.GetRouting()->add(host);

			Packet update(ChimeraUpdateType, chimera.GetMe().GetKey(), host.GetKey());
			update.SetArg(CHIMERA_UPDATE_ADDRESS, chimera.GetMe().GetAddr());

			if(!chimera.Send(host, update))
				pf_log[W_ROUTING] << "ChimeraJoinAck: failed to update " << host;
		}

		/* why do we do this ? - Michael */
		hosts = chimera.GetRouting()->getRoutingTable();
		for(std::vector<Host>::iterator it = hosts.begin(); it != hosts.end(); ++it)
		{
			Host host = *it;
			Packet update(ChimeraUpdateType, chimera.GetMe().GetKey(), host.GetKey());

			update.SetArg(CHIMERA_UPDATE_ADDRESS, chimera.GetMe().GetAddr());
			if(!chimera.Send(host, update))
				pf_log[W_ROUTING] << "ChimeraJoinAck: failed to update " << host;
		}

		/* Start the check of leafset repeated job. */
		scheduler_queue.Queue(new CheckLeafsetJob(&chimera, chimera.GetRouting()));
	}
};

class ChimeraJoinNAckMessage : public ChimeraMessage
{
public:
	/** A JOIN_NACK message trigger the sending of another JOIN message
	  * after some times.
	  */
	void Handle (Chimera& chimera, const Host&, const Packet& pckt)
	{
		pf_addr addr = pckt.GetArg<pf_addr>(CHIMERA_JOIN_NACK_ADDRESS);
		Host host = hosts_list.GetHost(addr);

		pf_log[W_ROUTING] << "JOIN request rejected from " << host;
		sleep(Chimera::GRACEPERIOD);
		pf_log[W_ROUTING] << "Re-sending JOIN message to " << host;

		chimera.Join(host);
	}
};

class ChimeraUpdateMessage : public ChimeraMessage
{
public:
	/** The Update message add the sender to the routing infrastructure
	  * (leafset + routing table).
	  */
	void Handle (Chimera& chimera, const Host&, const Packet& pckt)
	{
		pf_addr addr = pckt.GetArg<pf_addr>(CHIMERA_UPDATE_ADDRESS);
		Host host = hosts_list.GetHost(addr);
		chimera.GetRouting()->add(host);
	}
};

class ChimeraPiggyMessage : public ChimeraMessage
{
public:
	/** We update the routing infrastructure with the given addresses */
	void Handle (Chimera& chimera, const Host&, const Packet& pckt)
	{
		addr_list address = pckt.GetArg<addr_list>(CHIMERA_PIGGY_ADDRESSES);
		for(addr_list::iterator it = address.begin(); it != address.end(); ++it)
		{
			Host host = hosts_list.GetHost(*it);

			/* After a peer failed, we wait some time before adding it back. */
			if(time::dtime() - host.GetFailureTime() > Chimera::GRACEPERIOD)
				chimera.GetRouting()->add(host);
			else
				pf_log[W_ROUTING] << "Refused to add " << host << " to routing table";
		}
	}
};

class ChimeraPingMessage : public ChimeraMessage
{
	/** We handle a ping message by simply adding it in the host list.
	  * The ping is already ACKnoledged by the network, due to the REQUESTACK flag.
	  */
public:
	void Handle (Chimera&, const Host&, const Packet& pckt)
	{
		hosts_list.GetHost(pckt.GetArg<pf_addr>(CHIMERA_PING_ME));
	}
};

class ChimeraChatMessage : public ChimeraMessage
{
public:
	void Handle(Chimera&, const Host&, const Packet& pckt)
	{
		std::string message = pckt.GetArg<std::string>(CHIMERA_CHAT_MESSAGE);
		pf_log[W_INFO] << "CHAT[" << pckt.GetSrc() << "] " << message;
	}
};

PacketType      ChimeraJoinType(CHIMERA_JOIN,      new ChimeraJoinMessage,      Packet::REQUESTACK|
                                                                                Packet::MUSTROUTE,   "JOIN",           /* CHIMERA_JOIN_ADDRESS */ T_ADDR,
                                                                                                                                                  T_END);
PacketType   ChimeraJoinAckType(CHIMERA_JOIN_ACK,  new ChimeraJoinAckMessage,   Packet::REQUESTACK,  "JOIN_ACK", /* CHIMERA_JOIN_ACK_ADDRESSES */ T_ADDRLIST,
                                                                                                                                                  T_END);
PacketType    ChimeraUpdateType(CHIMERA_UPDATE,    new ChimeraUpdateMessage,    Packet::REQUESTACK,  "UPDATE",       /* CHIMERA_UPDATE_ADDRESS */ T_ADDR,
                                                                                                                                                  T_END);
PacketType     ChimeraPiggyType(CHIMERA_PIGGY,     new ChimeraPiggyMessage,     Packet::REQUESTACK,  "PIGGY",       /* CHIMERA_PIGGY_ADDRESSES */ T_ADDRLIST,
                                                                                                                                                  T_END);
PacketType  ChimeraJoinNAckType(CHIMERA_JOIN_NACK, new ChimeraJoinNAckMessage,  Packet::REQUESTACK,  "JOIN_NACK", /* CHIMERA_JOIN_NACK_ADDRESS */ T_ADDR,
                                                                                                                                                  T_END);
PacketType      ChimeraPingType(CIHMERA_PING,      new ChimeraPingMessage,      Packet::REQUESTACK,  "PING",                /* CHIMERA_PING_ME */ T_ADDR,
                                                                                                                                                  T_END);
PacketType      ChimeraChatType(CHIMERA_CHAT,      new ChimeraChatMessage,      Packet::REQUESTACK|
                                                                                Packet::MUSTROUTE,   "CHAT",           /* CHIMERA_CHAT_MESSAGE */ T_STR,
                                                                                                                                                  T_END);
