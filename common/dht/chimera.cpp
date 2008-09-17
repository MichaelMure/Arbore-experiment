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

#include "chimera.h"
#include "net/network.h"
#include "chimera_routing.h"
#include "key.h"
#include "chimera_messages.h"
#include "scheduler_queue.h"
#include "check_leafset_job.h"

ChimeraDHT::ChimeraDHT(Network* _network, uint16_t port, Key my_key)
	: network(_network),
	routing(NULL)

{
	char name[256];
	struct hostent* he;
	Key::Init();

	if(gethostname(name, sizeof(name)) != 0)
	{
		return;
	}
	if((he = gethostbyname(name)) == NULL)
	{
		return;
	}
	me = network->GetHostsList()->GetHost(he->h_name, port);
	me.SetKey(my_key);

	routing = new ChimeraRouting(network->GetHostsList(), me);

	RegisterType(ChimeraJoinType);
	RegisterType(ChimeraJoinAckType);
	RegisterType(ChimeraUpdateType);
	RegisterType(ChimeraPiggyType);
	RegisterType(ChimeraJoinNAckType);
	RegisterType(ChimeraPingType);

	fd = network->Listen(this, port, "0.0.0.0");
}

bool ChimeraDHT::Send(const Host& dest, const Packet& pckt)
{
	return network->Send(fd, dest, pckt);
}

bool ChimeraDHT::Ping(const Host& dest)
{
	Packet pckt(ChimeraPingType, me.GetKey(), dest.GetKey());
	pckt.SetArg(NET_PING_ME, me.GetAddr());

	return Send(dest, pckt);
}

void ChimeraDHT::Join(const Host& bootstrap)
{
	if(bootstrap == InvalidHost)
	{
		/* We are the first peer here */
		scheduler_queue.Queue(new CheckLeafsetJob(this, GetRouting()));
		return;
	}

	Packet pckt(ChimeraJoinType, GetMe().GetKey(), GetMe().GetKey());
	pckt.SetArg(NET_JOIN_ADDRESS, GetMe().GetAddr());
	if(!Send(bootstrap, pckt))
		pf_log[W_WARNING] << "ChimeraDHT::Join: failed to contact bootstrap host " << bootstrap;

	/* TODO: lock here until join is done. */
}

bool ChimeraDHT::Route(const Packet& pckt)
{
	Key key = pckt.GetDst();
	Host nextDest = GetRouting()->routeLookup(key);

	/* this is to avoid sending JOIN request to the node that
	 * its information is already in the routing table
	 */
	if(nextDest.GetKey() == key)
	{
		GetRouting()->remove(nextDest);
		nextDest = GetRouting()->routeLookup(key);
	}

	/* if I am the only host or the closest host is me, deliver the message */
	if(nextDest == GetMe())
		return false;

	/* XXX possibily an infinite loop? */
	while(!Send(nextDest, pckt))
	{
		nextDest.SetFailureTime(dtime());
		pf_log[W_WARNING] << "message sent to host: " << nextDest
		                  << " at time: " << nextDest.GetFailureTime()
		                  << " failed!";

		/* remove the faulty node from the routing table */
		if(nextDest.GetSuccessAvg() < BAD_LINK)
			GetRouting()->remove(nextDest);

		nextDest = GetRouting()->routeLookup(key);
		pf_log[W_WARNING] << "rerouting through " << nextDest;
	}

	/* in each hop in the way to the key root nodes
	 * send their routing info to the joining node. */
	if(pckt.GetPacketType() == ChimeraJoinType)
		sendRowInfo(pckt);

	return true;
}

void ChimeraDHT::sendRowInfo(const Packet& pckt)
{
	Host host = GetNetwork()->GetHostsList()->GetHost(pckt.GetArg<pf_addr>(NET_JOIN_ADDRESS));

	std::vector<Host> rowset = GetRouting()->rowLookup(host.GetKey());
	std::vector<pf_addr> addresses;
	for(std::vector<Host>::iterator it = rowset.begin(); it != rowset.end(); ++it)
		addresses.push_back(it->GetAddr());

	Packet rowinfo(ChimeraPiggyType, GetMe().GetKey(), host.GetKey());
	rowinfo.SetArg(NET_PIGGY_ADDRESSES, addresses);
	if(!Send(host, rowinfo))
		pf_log[W_ERR] << "Sending row information to node " << host << " failed";
}

#if 1 == 0

#include <errno.h>
#include <vector>

#include "chimera.h"
#include "pf_log.h"
#include "net/network.h"
#include "dtime.h"

#define CHIMERA_JOIN       1
#define CHIMERA_JOIN_ACK   2
#define CHIMERA_UPDATE     3
#define CHIMERA_PIGGY      4
#define CHIMERA_JOIN_NACK  5
#define CHIMERA_PING       6

/**
 *interval that members of leafset are checked to see if they are alive or not
 */
#define LEAFSET_CHECK_PERIOD 20	/* seconds */

/** GRACEPERIOD is the time that has to be elapsed before a node
 ** can be accepted to the network again after last send to it failed
 */
#define GRACEPERIOD  30		/* seconds */


void ChimeraDHT::Register(int type, int ack)
{
	if (type < 10)
	{
		pf_log[W_ERR] << "chimera_register: message integer types < 10 are reserved for system";
		exit (1);
	}

	if (ack != 1 && ack != 2)
	{
		pf_log[W_ERR] << "chimera_register: message property ack must be either 1 or 2 unrecognized ack value " << ack;
		exit (1);
	}

	this->message->message_handler (type, &ChimeraDHT::route_message, ack);
}


#endif
