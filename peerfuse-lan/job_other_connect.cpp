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

#include <algorithm>
#include "job_other_connect.h"
#include "job_types.h"
#include "peers_list.h"
#include "mutex.h"
#include "network.h"

JobOtherConnect::JobOtherConnect(Peer* _connect_to) : Job(time(NULL), REPEAT_PERIODIC, 1),
			connect_to(_connect_to)
{
}

bool JobOtherConnect::Start()
{
	BlockLockMutex lock(&peers_list);
	AddrList addr_list;

	bool everybody_connected = true;

	if(find(peers_list.begin(), peers_list.end(), connect_to) == peers_list.end())
	{
		// the peer disconnected, no need to ask others to connect to him
		return false;
	}

	// Ask all other peers to check their connection to this peer
	// TODO: replace this loop+find with a while the 3 lists (peers, is_connecting
	// and is connected) to spare some cpu
	for(PeersList::iterator it = peers_list.begin(); it != peers_list.end(); ++it)
	{
		if(*it == connect_to)
			continue;

		// Check if we already asked this peer to connect to the "connect_to" peer
		if(find(is_connecting.begin(), is_connecting.end(), *it) == is_connecting.end())
		{
			Packet p(NET_PEER_CONNECTION);
			p.SetArg(NET_PEER_CONNECTION_ADDRESS, connect_to->GetAddr());
			(*it)->SendMsg(p);
			is_connecting.push_back(*it);
			everybody_connected = false;
		}

		// Check if this peer is already connected to the "connect_to" peer
		if(find(is_connected.begin(), is_connected.end(), *it) == is_connected.end())
			everybody_connected = false;
	}

	if(!everybody_connected)
		return true;
	connect_to->SendMsg(Packet(NET_PEER_ALL_CONNECTED));
	return false;
}

bool JobOtherConnect::IsConnectingTo(pf_addr addr)
{
	BlockLockMutex lock(&peers_list);
	if(find(peers_list.begin(), peers_list.end(), connect_to) == peers_list.end())
	{
		// the peers disconnected
		return false;
	}

	return (connect_to->GetAddr() == addr);
}

void JobOtherConnect::PeerConnected(Peer* peer)
{
	is_connected.push_back(peer);
}
