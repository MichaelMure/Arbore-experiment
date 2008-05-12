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
#include "environment.h"

PeersList peers_list;

void PeersList::GivePacketTo(pf_id id, Packet* packet) const
{
	BlockLockMutex lock(&peers_list);
	const_iterator it;
	/* Packets are not given to anonymous Peers */
	for(it = begin(); it != end() && ((*it)->GetID() != id || (*it)->IsAnonymous()); ++it)
		;

	if(it != end())
		(*it)->HandleMsg(packet);
	else
		log[W_WARNING] << "Received a packet from unknown peer";
}

void PeersList::SendPeerList(Peer* to) const
{
	/* This is an alias, to disallow usage of _send_peer_list
	 * with second argument out of PeersList object.
	 */
	_send_peer_list(to, NULL);
}

void PeersList::_send_peer_list(Peer* to, Peer* from) const
{
	BlockLockMutex lock(&peers_list);
	StaticPeersList peers = from ? GetDownLinks(from) : GetDirectHighLinks();
	uint32_t now = time(NULL);
	for(StaticPeersList::iterator it = peers.begin(); it != peers.end(); ++it)
	{
		/* Do not send information about himself! */
		if(*it == to)
			continue;

		pf_id id = (*it)->GetUpLink() ? (*it)->GetUpLink() : environment.my_id.Get();

		/* It broadcasts. */
		Packet pckt(NET_PEER_CONNECTION, id, 0);
		pckt.SetArg(NET_PEER_CONNECTION_ADDRESS, (*it)->GetAddr());
		pckt.SetArg(NET_PEER_CONNECTION_NOW, (uint32_t)(now - (*it)->GetTimestampDiff()));
		pckt.SetArg(NET_PEER_CONNECTION_CERTIFICATE, (*it)->GetCertificate());

		to->SendMsg(pckt);

		_send_peer_list(to, *it);
	}
}

void PeersList::GetMapOfNetwork(std::vector<std::string>& list, Peer* from, std::string prefix) const
{
	BlockLockMutex lock(this);
	StaticPeersList peers = from ? GetDownLinks(from) : GetDirectHighLinks();
	if(from == NULL)
		list.push_back(std::string("127.0.0.1/") + TypToStr(environment.my_id.Get()));
	for(StaticPeersList::iterator it = peers.begin(); it != peers.end(); ++it)
	{
		std::string s = prefix;
		if(it+1 == peers.end())
		{
			s += "`-";
			prefix += "  ";
		}
		else
		{
			s += "|-";
			prefix += "| ";
		}

		pf_addr addr = (*it)->GetAddr();
		s += (*it)->GetCertificate().GetCommonName();
		s += " (";
		s += pf_addr2string(addr);
		s += ")";

		list.push_back(s);

		GetMapOfNetwork(list, *it, prefix);

		prefix = prefix.substr(0, prefix.size() - 2);
	}
}

StaticPeersList PeersList::GetDirectHighLinks() const
{
	BlockLockMutex lock(&peers_list);
	StaticPeersList list;
	const_iterator it;
	for(it = begin(); it != end(); ++it)
		if((*it)->IsHighLink() && !(*it)->IsAnonymous())
			list.push_back(*it);

	return list;
}

StaticPeersList PeersList::GetDownLinks(Peer* p) const
{
	BlockLockMutex lock(&peers_list);
	StaticPeersList list;
	std::vector<pf_id> downlinks = p->GetDownLinks();
	for(std::vector<pf_id>::iterator it = downlinks.begin();
		it != downlinks.end();
		++it)
	{
		Peer* peer = PeerFromID(*it);
		assert(peer);
		list.push_back(peer);
	}
	return list;
}

std::vector<pf_addr> PeersList::RemoveDownLinks(Peer* p)
{
	BlockLockMutex lock(this);
	std::vector<pf_addr> addr_list;

	/* Remove all downlinks, and recursively all downlinks
	 * of every downlinks */
	std::vector<Peer*> down_links = GetDownLinks(p);
	for(std::vector<Peer*>::iterator it = down_links.begin();
		it != down_links.end();
		++it)
	{
		std::vector<pf_addr> local_addr_lst;
		local_addr_lst = RemoveDownLinks(*it);

		/* Add downlinks and peer addresses */
		addr_list.push_back((*it)->GetAddr());
		addr_list.insert(addr_list.begin(), local_addr_lst.begin(), local_addr_lst.end());

		/* Do not call our RemovePeer() because it would call
		 * RemoveDownLinks() and it can crash. */
		PeersListBase::RemovePeer(*it);

		delete *it;
	}

	/* This addr is used by Network::OnRemove() to trying to
	 * connect to it. */
	return addr_list;
}

Peer* PeersList::RemovePeer(Peer* p)
{
	BlockLockMutex lock(this);

	if(!p->IsAnonymous())
	{
		pf_id id;
		if((id = p->GetUpLink()))
		{
			Peer* uplink = PeerFromID(id);
			uplink->RemoveDownLink(p->GetID());
		}
	}

	return PeersListBase::RemovePeer(p);
}

Peer* PeersList::RemoveFromID(pf_id id)
{
	BlockLockMutex lock(this);
	iterator it;
	for(it = begin(); it != end() && (*it)->GetID() != id; ++it)
		;

	if(it == end())
		return NULL;

	Peer* peer = *it;
	RemovePeer(peer);

	return peer;
}

void PeersList::EraseFromID(pf_id id)
{
	BlockLockMutex lock(this);
	Peer* p = RemoveFromID(id);
	delete p;
}

void PeersList::PeerSetConnection(pf_id id, int fd)
{
	BlockLockMutex lock(this);

	Peer* anonymous = PeerFromFD(fd);
	Peer* lowlink = PeerFromID(id);

	if(!anonymous)
		return;

	if(!lowlink)
	{
		Erase(fd);
		return;
	}

	lowlink->SetConnection(anonymous->GetConnection());
	anonymous->SetConnection(NULL);
	RemovePeer(anonymous);
	fd2peer[fd] = lowlink;
}

void PeersList::Broadcast(Packet pckt, const Peer* but_one) const
{
	BlockLockMutex lock(&peers_list);
	pckt.SetDstID(0);
	PeersList::iterator it;
	for(it = peers_list.begin(); it != peers_list.end(); ++it)
		if(!(*it)->IsAnonymous() &&
		(*it)->IsHighLink() &&
		(*it)->IsConnection() &&
		(*it) != but_one)
			(*it)->SendMsg(pckt);
}

void PeersList::SendMsg(pf_id id, const Packet &p) const
{
	BlockLockMutex lock(this);
	Peer* peer = PeerFromID(id);
	if(peer)
	{
		Packet packet = p;
		if(packet.GetSrcID() == 0)
			packet.SetSrcID(environment.my_id.Get());
		peer->SendMsg(packet);
	}
}

void PeersList::SendMsg(IDList ids, const Packet &p) const
{
	BlockLockMutex lock(this);
	Packet packet = p;
	for(IDList::iterator it = ids.begin(); it != ids.end(); ++it)
	{
		packet.SetDstID(*it);
		SendMsg(*it, packet);
	}
}

