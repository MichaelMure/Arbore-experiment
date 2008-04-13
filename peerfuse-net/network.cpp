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
#include "scheduler.h"
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
	/* If this peer is already on my list, this is because I
	 * get a NET_PEER_CONNECTION from my highlink, in other
	 * case, this is a highlink.
	 */
	if((p = peers_list.RemoveFromID(peer->GetID())))
		delete p;
	else
		peer->SetHighLink(peer);

	if(peers_list.Size() == 0 && peer->HasFlag(Peer::SERVER))
	{
		// This is the first peer to which we connected
		peer->SetHighLink();
		peer->SendHello();
	}
	return NetworkBase::AddPeer(peer);
}

void Network::OnRemovePeer(Peer* peer)
{
	if(!peer->IsHighLink() || peer->IsAnonymous())
		return;

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
	peers_list.RemoveDownLinks(peer);
}

void Network::StartNetwork(MyConfig* conf)
{
	NetworkBase::StartNetwork(conf);

	SslSsl* sslssl = dynamic_cast<SslSsl*>(ssl);

	assert(sslssl != NULL);			  /* we MUST use a SSL connection on pfnet. */
	Certificate cert = sslssl->GetCertificate();
	environment.my_id.Set(cert.GetIDFromCertificate());

}
