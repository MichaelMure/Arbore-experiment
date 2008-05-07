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

#include "peers_list_base.h"
#include "peer.h"
#include "mutex.h"
#include "log.h"
#include "environment.h"

PeersListBase::PeersListBase() : Mutex(RECURSIVE_MUTEX), changed(false)
{
}

PeersListBase::~PeersListBase()
{
	for(iterator it = begin(); it != end(); ++it)
		delete *it;
}

bool PeersListBase::IsChanged() const
{
	BlockLockMutex lock(this);
	return changed;
}

void PeersListBase::ClearChanged()
{
	BlockLockMutex lock(this);
	changed = false;
}

Peer* PeersListBase::PeerFromFD(int fd)
{
	if(fd < 0) return NULL;

	PeerMap::iterator it = fd2peer.find(fd);
	if(it == fd2peer.end())
		return NULL;
	return it->second;
}

Peer* PeersListBase::PeerFromID(pf_id id) const
{
	if(!id) return NULL;

	BlockLockMutex lock(this);
	const_iterator it = begin();
	while(it != end() && (*it)->GetID() != id
#ifdef PF_NET
		&& !(*it)->IsAnonymous()
#endif
	     )
		++it;

	Peer* p = NULL;

	if(it != end())
		p = *it;

	return p;
}

PeersListBase::id_state_t PeersListBase::WhatIsThisID(pf_id id)
{
	Peer* p = PeerFromID(id);
	enum id_state_t id_state;

	if(!p)
		id_state = IS_UNKNOWN;
	else if(p->IsConnection())
		id_state = IS_CONNECTED;
	else
		id_state = IS_ON_NETWORK;

	log[W_DEBUG] << "PeersList::WhatIsThisID(" << id << ") = " << id_state;

	return id_state;
}

// Public methods
void PeersListBase::Add(Peer* p)
{
	BlockLockMutex lock(this);
	push_back(p);
	fd2peer[p->GetFd()] = p;
	changed = true;
}

Peer* PeersListBase::RemovePeer(Peer* p)
{
	BlockLockMutex lock(this);
	iterator it;
	for(it = begin(); it != end() && *it != p; ++it)
		;

	if(it == end())
		return NULL;

	log[W_DEBUG] << "Removing peer from list: " << p->GetID();

	erase(it);
	if(p->IsConnection())
	{
		PeerMap::iterator fd_it = fd2peer.find(p->GetFd());
		if(fd_it != fd2peer.end())
			fd2peer.erase(fd_it);
	}

	changed = true;

	return p;
}

void PeersListBase::Pop(int fd, Peer** p)
{
	BlockLockMutex lock(this);
	PeerMap::iterator it = fd2peer.find(fd);
	Peer* peer = NULL;
	if(it != fd2peer.end())
	{
		peer = RemovePeer(it->second);
		log[W_CONNEC] << "<- Removing a peer: " << fd << " (" << peer->GetID() << ")";
	}

	*p = peer;
}

void PeersListBase::Erase(int fd)
{
	BlockLockMutex lock(this);
	Peer* p;
	Pop(fd, &p);
	delete p;
}

void PeersListBase::PeerFlush(int fd)
{
	BlockLockMutex lock(this);
	Peer* p = PeerFromFD(fd);
	if(p)
		p->Flush();
}

bool PeersListBase::PeerReceive(int fd)
{
	BlockLockMutex lock(this);
	bool res = 0;
	Peer* p = PeerFromFD(fd);
	if(p)
		res = p->Receive();
	return res;
}

void PeersListBase::CloseAll()
{
	BlockLockMutex lock(this);
	for(iterator it = begin(); it != end(); ++it)
		delete *it;
	clear();
	fd2peer.clear();
	changed = true;
}

pf_id PeersListBase::CreateID()
{
	BlockLockMutex lock(this);
	// TODO: optimize me
	pf_id new_id = 0;
	while(!new_id)
	{
		new_id = rand();
		if(new_id == environment.my_id.Get())
		{
			new_id = 0;
			continue;
		}
		for(PeerMap::iterator peer = fd2peer.begin();
			peer != fd2peer.end();
			++peer)
		{
			if(new_id == peer->second->GetID())
			{
				new_id = 0;
				break;
			}
		}
	}

	return new_id;
}

void PeersListBase::SendMsg(pf_id id, const Packet &p) const
{
	BlockLockMutex lock(this);
	Peer* peer = PeerFromID(id);
	peer->SendMsg(p);
}

void PeersListBase::SendMsg(IDList ids, const Packet &p) const
{
	BlockLockMutex lock(this);
	for(IDList::iterator it = ids.begin(); it != ids.end(); ++it)
		SendMsg(*it, p);
}

void PeersListBase::RequestChunk(std::string filename, pf_id id, off_t offset, size_t size)
{
	BlockLockMutex lock(this);
	Peer* peer = PeerFromID(id);
	peer->RequestChunk(filename, offset, size);
}

void PeersListBase::SendChunk(uint32_t ref, pf_id id, FileChunk& chunk)
{
	BlockLockMutex lock(this);
	Peer* peer = PeerFromID(id);
	peer->SendChunk(ref, chunk);
}
