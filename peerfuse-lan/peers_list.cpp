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

#include "peers_list.h"
#include "peers_list_base.h"

PeersList peers_list;

void PeersList::Broadcast(Packet pckt, const Peer* but_one) const
{
	BlockLockMutex lock(this);
	for(const_iterator it = begin(); it != end(); ++it)
		if(*it != but_one)
			(*it)->SendMsg(pckt);
}

void PeersList::SendGetStructDiff(pf_id to) const
{
	BlockLockMutex lock(this);
	Peer* p = PeerFromID(to);
	if(p && p->IsClient())
		p->SendGetStructDiff();
}

bool PeersList::CheckOtherConnection(pf_id connect_to, std::list<pf_id>& is_connecting, std::list<pf_id>& is_connected) const
{
	BlockLockMutex lock(this);

	bool everybody_connected = true;

	Peer* peer = PeerFromID(connect_to);
	if(!peer)
	{
		// the peer disconnected, no need to ask others to connect to him
		return false;
	}

	// Ask all other peers to check their connection to this peer
	// TODO: replace this loop+find with a while the 3 lists (peers, is_connecting
	// and is connected) to spare some cpu
	for(const_iterator it = begin(); it != end(); ++it)
	{
		if((*it)->GetID() == connect_to)
			continue;

		// Check if we already asked this peer to connect to the "connect_to" peer
		if(find(is_connecting.begin(), is_connecting.end(), (*it)->GetID()) == is_connecting.end())
		{
			Packet p(NET_PEER_CONNECTION);
			p.SetArg(NET_PEER_CONNECTION_ADDRESS, peer->GetAddr());
			(*it)->SendMsg(p);
			is_connecting.push_back((*it)->GetID());
			everybody_connected = false;
		}

		// Check if this peer is already connected to the "connect_to" peer
		if(find(is_connected.begin(), is_connected.end(), (*it)->GetID()) == is_connected.end())
			everybody_connected = false;
	}

	if(!everybody_connected)
		return true;
	peer->SendMsg(Packet(NET_PEER_ALL_CONNECTED));
	return false;
}
