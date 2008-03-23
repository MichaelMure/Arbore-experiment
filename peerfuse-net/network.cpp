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
#include "ssl/pf_ssl_ssl.h"

Network net;

Network::Network()
{
}

Network::~Network()
{
}

void Network::GivePacketTo(pf_id id, Packet* packet) const
{
	PeerList::const_iterator it;
	for(it = peer_list.begin(); it != peer_list.end() && (*it)->GetID() != id; ++it)
		;

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

Peer* Network::ID2Peer(pf_id id) const
{
	PeerList::const_iterator it;
	for(it = peer_list.begin(); it != peer_list.end() && (*it)->GetID() != id; ++it)
		;

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
	log[W_INFO] << "Removed disconnected: " << addr;
	disconnected_list.remove(addr);

	/* Remove connection from queue. */
	std::list<Job*> job_list = scheduler.GetQueue();
	for(std::list<Job*>::iterator it = job_list.begin();
		it != job_list.end();
		++it)
	{
		JobNewConnection* job = dynamic_cast<JobNewConnection*>(*it);
		if(job && job->IsMe(addr))
		{
			log[W_DEBUG] << "-> removed a job";
			scheduler.Cancel(job);
		}
	}
}

Peer* Network::Start(MyConfig* config)
{
	Peer* peer = NetworkBase::Start(config);

	SslSsl* sslssl = dynamic_cast<SslSsl*>(ssl);

	assert(sslssl != NULL);			  /* we MUST use a SSL connection on pfnet. */
	Certificate cert = sslssl->GetCertificate();
	my_id = cert.GetIDFromCertificate();

	if(peer)
	{
		peer->SetHighLink();
		peer->SendHello();
	}

	return peer;
}
