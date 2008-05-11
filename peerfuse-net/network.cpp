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
 * This product includes cryptographic software written by Eric Younganus
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
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
#include "network_base.h"
#include "network.h"
#include "tools.h"
#include "scheduler_queue.h"
#include "job_new_connection_queue.h"
#include "job_update_resp_files.h"
#include "pf_ssl_ssl.h"
#include "peers_list.h"
#include "mutex.h"
#include "environment.h"

Network net;

Network::Network()
{
}

Network::~Network()
{
}

Peer* Network::AddPeer(Peer* peer)
{
	Peer* p;

	switch(peers_list.WhatIsThisID(peer->GetID()))
	{
		case PeersList::IS_CONNECTED:
			log[W_WARNING] << "A peer (" << peer->GetAddr() << ") wants to connect to me but someone with the same ID is already connected";
			delete peer;
			return NULL;
		case PeersList::IS_UNKNOWN:
			peer->SetHighLink(true);
			break;
		case PeersList::IS_ON_NETWORK:
			peer->SetHighLink(false);
			break;
	}

	if(peer->HasFlag(Peer::SERVER))
		peer->SendHello();

	p = NetworkBase::AddPeer(peer);

	#ifdef DEBUG
	std::vector<std::string> list;
	peers_list.GetMapOfNetwork(list);

	log[W_INFO] << "Network map:";
	for(std::vector<std::string>::iterator it = list.begin(); it != list.end(); ++it)
		log[W_INFO] << *it;
	#endif

	return p;

}

void Network::OnRemovePeer(Peer* peer)
{
	#ifdef DEBUG
	std::vector<std::string> list;
	peers_list.GetMapOfNetwork(list);

	log[W_INFO] << "Network map:";
	for(std::vector<std::string>::iterator it = list.begin(); it != list.end(); ++it)
		log[W_INFO] << *it;
	#endif

	if(!peer->IsHighLink() || peer->IsAnonymous())
		return;

	BlockLockMutex lock(&peers_list);
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

	/* Tell to everybody that it is disconnected.
	 * Only this peer is anounced in message, because receivers
	 * will find themselves what peers are under it.
	 */
	Packet pckt(NET_PEER_GOODBYE, peer->GetID(), 0);
	peers_list.Broadcast(pckt, peer);

	/* Added direct down_links of this peer in my scheduler queue
	 * to connect to them like a highlink*/
	std::vector<pf_addr> addr_list = peers_list.RemoveDownLinks(peer);

	/* Connect to them. */
	if(addr_list.empty() == false)
		scheduler_queue.Queue(new JobNewConnQueue(addr_list));

	scheduler_queue.Queue(new JobUpdateRespFiles());
}

void Network::StartNetwork(MyConfig* conf)
{
	NetworkBase::StartNetwork(conf);

	SslSsl* sslssl = dynamic_cast<SslSsl*>(ssl);

	assert(sslssl != NULL);			  /* we MUST use a SSL connection on pfnet. */
	Certificate cert = sslssl->GetCertificate();
	environment.my_id.Set(cert.GetIDFromCertificate());

}

void Network::ThrowHandler()
{
	try
	{
		Loop();
	}
	catch(PeerBase::SelfConnect &e)
	{
		/* pfnet throws this while handling the SSL handshake */
		/* We are connecting to ourself, this is quiet dangerous */
		log[W_ERR] << "I'm trying to connect to myself, this is bad.";
		log[W_ERR] << "Check your configuration, and check peerfuse is not already running.";
		exit(EXIT_FAILURE);
	}
	catch(Peer::CopyLowLinkConnection &e)
	{
		/* When a peer is connecting to me as a lowlink, I already has a Peer object
		 * to describe it. Now we have a Connection object to put on it.
		 * The temporary anonymous Peer object previously attached to it
		 * can be removed.
		 */
		log[W_DEBUG] << "Switch lowlink connection to an other Peer for " << e.id;
		BlockLockMutex lock(&peers_list); /* avoid race condition */
		peers_list.PeerFlush(e.fd); /* flush messages before */
		peers_list.PeerSetConnection(e.id, e.fd);
	}
}
