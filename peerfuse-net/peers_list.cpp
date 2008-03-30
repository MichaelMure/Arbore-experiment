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

#include "peers_list.h"
#include "peers_list_base.h"

PeersList peers_list;

Peer* PeersList::RemoveFromID(pf_id id)
{
        BlockLockMutex lock(this);
        Peer* peer = NULL;
        iterator it;
        for(it = begin(); it != end() && (*it)->GetID() != id; ++it)
                ;

        if(it == end())
                return NULL;

        erase(it);

        PeerMap::iterator p = fd2peer.find(peer->GetFd());
        if(p != fd2peer.end())
                fd2peer.erase(p);

        return peer;
}

void PeersList::EraseFromID(pf_id id)
{
        BlockLockMutex lock(this);
        Peer* p = RemoveFromID(id);
        delete p;
}

void PeersList::Broadcast(Packet pckt, const Peer* but_one) const
{
	BlockLockMutex lock(&peers_list);
	pckt.SetDstID(0);
	PeersList::iterator it;
	for(it = peers_list.begin(); it != peers_list.end(); ++it)
		if(!(*it)->IsAnonymous() &&
		(*it)->IsHighLink() &&
		(*it) != but_one)
			(*it)->SendMsg(pckt);
}

bool PeersList::IsIDOnNetwork(pf_id id)
{
	return PeerFromID(id);
}
