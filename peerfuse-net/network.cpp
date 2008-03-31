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
#include "job_new_connection.h"
#include "scheduler.h"
#include "ssl/pf_ssl_ssl.h"
#include "peers_list.h"
#include "mutex.h"

Network net;

Network::Network()
{
}

Network::~Network()
{
}

void Network::GivePacketTo(pf_id id, Packet* packet) const
{
	BlockLockMutex lock(&peers_list);
	PeersList::const_iterator it;
	for(it = peers_list.begin(); it != peers_list.end() && (*it)->GetID() != id; ++it)
		;

	if(it != peers_list.end())
		(*it)->HandleMsg(packet);
	else
		log[W_WARNING] << "Received a packet from unknown peer";
}

StaticPeersList Network::GetDirectHighLinks() const
{
	BlockLockMutex lock(&peers_list);
	StaticPeersList list;
	PeersList::iterator it;
	for(it = peers_list.begin(); it != peers_list.end(); ++it)
		if((*it)->IsHighLink())
			list.push_back(*it);

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

Peer* Network::AddPeer(Peer* peer)
{
	Peer* p;
	/* If this peer is already on my list, this is because I
	 * get a NET_PEER_CONNECTION from my highlink, in other
	 * case, this is a highlink.
	 */
	if((p = peers_list.RemoveFromID(peer->GetID())))
		delete p;
	else
		peer->SetHighLink(peer);

	return NetworkBase::AddPeer(peer);
}

void Network::OnRemovePeer(Peer* peer)
{
	if(peer->IsHighLink() && !peer->IsAnonymous())
	{
		/* A       C
		 * '---B---'
		 *     |       .--F
		 * .---D----E--|
		 * H           '--G
		 *
		 * E = me
		 * D = peer (who is disconnecting)
		 *
		 * I send a broadcast NET_PEER_GOODBYE message to F et G,
		 * and I try to connect to H and B as highlink.
		 */

		BlockLockMutex lock(&peers_list);
		Packet pckt(NET_PEER_GOODBYE, peer->GetID(), 0);
		peers_list.Broadcast(pckt, peer);

		/* Added direct down_links of this peer in my scheduler queue
		 * to connect to them like a highlink*/
		std::vector<Peer*> down_links = peer->GetDownLinksButDontForgetToLockPeersListMutex();
		for(std::vector<Peer*>::iterator it = down_links.begin();
			it != down_links.end();
			++it)
		{
			AddDisconnected((*it)->GetAddr());
		}
	}

	/* TODO: Loop on all down_links of peer to disconnect from them. */
}

Peer* Network::Start(MyConfig* config)
{
	Peer* peer = NetworkBase::Start(config);

	SslSsl* sslssl = dynamic_cast<SslSsl*>(ssl);

	assert(sslssl != NULL);			  /* we MUST use a SSL connection on pfnet. */
	Certificate cert = sslssl->GetCertificate();
	peers_list.SetMyID(cert.GetIDFromCertificate());

	if(peer)
	{
		peer->SetHighLink();
		peer->SendHello();
	}

	return peer;
}
