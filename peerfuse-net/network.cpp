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

#include <iostream>
#include <list>
#include <algorithm>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#include "log.h"
#include "libconfig.h"
#include "network_base.h"
#include "network.h"
#include "tools.h"
#include "job.h"
#include "scheduler.h"

Network net;

Network::Network()
{
}

Network::~Network()
{
}

void Network::GivePacketTo(id_t id, Packet* packet) const
{
	PeerList::const_iterator it;
	for(it = peer_list.begin(); it != peer_list.end() && (*it)->GetID() != id; ++it);

	if(it != peer_list.end())
		(*it)->HandleMsg(packet);
	else
		log[W_WARNING] << "Received a packet from unknown peer";
}

void Network::Broadcast(Packet pckt, const Peer* but_one)
{
	pckt.SetDstID(0);
	for(PeerMap::iterator it = fd2peer.begin(); it != fd2peer.end(); ++it)
		if(!it->second->IsAnonymous() &&
		   it->second->IsHighLink() &&
		   it->second != but_one)
			it->second->SendMsg(pckt);
}

Peer* Network::ID2Peer(id_t id) const
{
	PeerList::const_iterator it;
	for(it = peer_list.begin(); it != peer_list.end() && (*it)->GetID() != id; ++it);

	return (it != peer_list.end() ? *it : NULL);
}

PeerList Network::GetDirectHighLinks() const
{
	PeerMap::const_iterator it = fd2peer.begin();
	PeerList list;
	for(; it != fd2peer.end(); ++it)
		if(it->second->IsHighLink())
			list.push_back(it->second);

	return list;
}

void Network::AddDisconnected(const pf_addr& addr)
{
	if(find(disconnected_list.begin(),
		disconnected_list.end(), addr) == disconnected_list.end())
	{
		disconnected_list.push_back(addr);
		scheduler.Queue(new JobNewConnection(addr));
	}
}

void Network::DelDisconnected(const pf_addr& addr)
{
	std::list<pf_addr>::iterator it = find(disconnected_list.begin(), disconnected_list.end(), addr);

	if( it != disconnected_list.end())
		disconnected_list.erase(it);
}

Peer* Network::Start(MyConfig* config)
{
	/*
	 * TODO TODO TODO        TODO        TODO TODO              TODO
	 *      TODO         TODO    TODO    TODO    TODO       TODO    TODO
	 *      TODO       TODO        TODO  TODO       TODO  TODO        TODO
	 *      TODO       TODO        TODO  TODO       TODO  TODO        TODO
	 *      TODO         TODO    TODO    TODO    TODO       TODO    TODO
	 *      TODO             TODO        TODO TODO              TODO
	 *
	 * Do NOT create ID, but get it from certificate!
	 */
	my_id = CreateID();

	Peer* peer = NetworkBase::Start(config);

	if(peer)
	{
		peer->SetHighLink();
		peer->SendHello();
	}

	return peer;
}
