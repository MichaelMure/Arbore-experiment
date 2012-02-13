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

#include <unistd.h>
#include <netdb.h>

#include <net/network.h>
#include <net/hosts_list.h>
#include <net/addr_list.h>
#include <scheduler/scheduler_queue.h>
#include <util/key.h>
#include <dht/dht.h>

#include "check_leafset_job.h"
#include "chimera.h"
#include "messages.h"
#include "routing.h"

Chimera::Chimera(DHT *dht, uint16_t port, const Key& my_key)
	: network(new Network(this)),
	dht_(dht),
	routing(NULL)
{
	network->Start();

	char name[256];
	struct hostent* he;

	if(gethostname(name, sizeof(name)) != 0)
	{
		return;
	}
	if((he = gethostbyname(name)) == NULL)
	{
		return;
	}
	pf_log[W_ROUTING] << he->h_name;
	me = hosts_list.GetHost(he->h_name, port);
	me.SetKey(my_key);

	routing = new Routing(me);

	packet_type_list.RegisterType(ChimeraJoinType);
	packet_type_list.RegisterType(ChimeraJoinAckType);
	packet_type_list.RegisterType(ChimeraUpdateType);
	packet_type_list.RegisterType(ChimeraPiggyType);
	packet_type_list.RegisterType(ChimeraJoinNAckType);
	packet_type_list.RegisterType(ChimeraPingType);
	packet_type_list.RegisterType(ChimeraChatType);

	fd = network->Listen(port, "0.0.0.0");

	pf_log[W_INFO] << "Started Chimera with key " << my_key;
}

bool Chimera::Send(const Host& dest, const Packet& pckt)
{
	return network->Send(fd, dest, pckt);
}

bool Chimera::SendToNeighbours(const uint32_t number, const Packet& pckt)
{
	bool success = false;
	std::vector<Host>::const_iterator it;
	uint32_t i = 0;

	std::vector<Host> leafsetCW = routing->getCWLeafset();
	for(it= leafsetCW.begin();
	    it != leafsetCW.end() && i < number;
	    it++, i++)
	{
		if(Send(*it, pckt))
			success = true;
	}

	i = 0;
	std::vector<Host> leafsetCCW = routing->getCCWLeafset();
	for(it= leafsetCCW.begin();
	    it != leafsetCCW.end() && i < number;
	    it++, i++)
	{
		if(Send(*it, pckt))
			success = true;
	}

	return success;
}

bool Chimera::ClosestTo(const Key& key) const
{
	std::vector<Host> leafsetCW = routing->getCWLeafset();
	std::vector<Host> leafsetCCW = routing->getCCWLeafset();

	bool ownerCW = true;
	bool ownerCCW = true;

	if(!leafsetCW.empty())
		if(key.distance(me.GetKey()) > key.distance(leafsetCW.at(0).GetKey()))
			ownerCW = false;

	if(!leafsetCCW.empty())
		if(key.distance(me.GetKey()) >= key.distance(leafsetCCW.at(0).GetKey()))
			ownerCCW = false;

	return ownerCW && ownerCCW;
}

bool Chimera::Ping(const Host& dest)
{
	Packet pckt(ChimeraPingType, me.GetKey(), dest.GetKey());
	pckt.SetArg(CHIMERA_PING_ME, me.GetAddr());

	return Send(dest, pckt);
}

void Chimera::HandleMessage(const Host& sender, const Packet& pckt)
{
	if(pckt.HasFlag(Packet::MUSTROUTE))
	{
		if(Route(pckt))
			return;
	}

	/* Route() returned false, so we are likely the destination of the message. */

	PacketHandlerBase *handler_base = pckt.GetPacketType().GetHandler();
	if(handler_base->getType() == HANDLER_TYPE_CHIMERA)
	{
		ChimeraMessage *handler = (ChimeraMessage*) handler_base;
		handler->Handle(*this, sender, pckt);
	}
	else
	{
		/* If the message is not aimed to chimera, we chain up the message to the DHT, if any. */
		if(dht_ != NULL)
			dht_->HandleMessage(sender, pckt);
	}
}

void Chimera::Join(const Host& bootstrap)
{
	if(bootstrap == InvalidHost)
	{
		/* We are the first peer here */
		scheduler_queue.Queue(new CheckLeafsetJob(this, GetRouting()));
		return;
	}

	Packet pckt(ChimeraJoinType, GetMe().GetKey(), bootstrap.GetKey());
	pckt.SetArg(CHIMERA_JOIN_ADDRESS, GetMe().GetAddr());
	if(!Send(bootstrap, pckt))
		pf_log[W_WARNING] << "Chimera::Join: failed to contact bootstrap host " << bootstrap;

	/* TODO: lock here until join is done. */
}

bool Chimera::Route(const Packet& pckt)
{
	pf_log[W_ROUTING] << "***** ROUTING ******";

	Key key = pckt.GetDst();

	if(key == me.GetKey())
	{
		pf_log[W_ROUTING] << "I'm the destination, deliver the message.";
		pf_log[W_ROUTING] << "***** END OF ROUTING *****";
		return false;
	}

	Host nextDest = GetRouting()->routeLookup(key);

	pf_log[W_ROUTING] << "Routing to: " << nextDest;

	/* this is to avoid sending JOIN request to the node that
	 * its information is already in the routing table
	 */
	if(nextDest.GetKey() == key && pckt.GetPacketType() == ChimeraJoinType)
	{
		GetRouting()->remove(nextDest);
		nextDest = GetRouting()->routeLookup(key);
		pf_log[W_ROUTING] << "Routing the JOIN message to another host (host already know) " << nextDest;
	}

	/* if I am the only host or the closest host is me, deliver the message */
	if(nextDest == GetMe())
	{
		pf_log[W_ROUTING] << "I'm the closest know host, deliver the message.";
		pf_log[W_ROUTING] << "***** END OF ROUTING *****";
		return false;
	}

	/* XXX possibily an infinite loop? */
	while(!Send(nextDest, pckt))
	{
		nextDest.SetFailureTime(time::dtime());
		pf_log[W_ROUTING] << "message sent to host: " << nextDest
		                  << " at time: " << nextDest.GetFailureTime()
		                  << " failed!";

		/* remove the faulty node from the routing table */
		if(nextDest.GetSuccessAvg() < BAD_LINK)
			GetRouting()->remove(nextDest);

		nextDest = GetRouting()->routeLookup(key);
		pf_log[W_ROUTING] << "rerouting through " << nextDest;
	}

	/* in each hop in the way to the key root nodes
	 * send their routing info to the joining node. */
	if(pckt.GetPacketType() == ChimeraJoinType)
		sendRowInfo(pckt);

	pf_log[W_ROUTING] << "******* END OF ROUTING *******";
	return true;
}

void Chimera::sendRowInfo(const Packet& pckt)
{
	Host host = hosts_list.GetHost(pckt.GetArg<pf_addr>(CHIMERA_JOIN_ADDRESS));

	std::vector<Host> rowset = GetRouting()->rowLookup(host.GetKey());
	addr_list addresses;
	for(std::vector<Host>::iterator it = rowset.begin(); it != rowset.end(); ++it)
		addresses.push_back(it->GetAddr());

	Packet rowinfo(ChimeraPiggyType, GetMe().GetKey(), host.GetKey());
	rowinfo.SetArg(CHIMERA_PIGGY_ADDRESSES, addresses);
	if(!Send(host, rowinfo))
		pf_log[W_ERR] << "Sending row information to node " << host << " failed";
}
