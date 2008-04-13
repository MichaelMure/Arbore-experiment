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

#ifndef PEERS_LIST_H
#define PEERS_LIST_H

#include "peers_list_base.h"

class PeersList: public PeersListBase
{
	StaticPeersList GetDownLinks(Peer* p) const;

	void _send_peer_list(Peer* to, Peer* from) const;

public:
	PeersList() {}
	~PeersList() {}

	void EraseFromID(pf_id id);
	Peer* RemoveFromID(pf_id id);

	/* Remove all downlinks from a peer */
	void RemoveDownLinks(Peer* p);

	bool IsIDOnNetwork(pf_id id);
	/** Return a list of connected high linkes */
	StaticPeersList GetDirectHighLinks() const;

	void GivePacketTo(pf_id id, Packet* packet) const;

	virtual void Broadcast(Packet pckt, const Peer* but_one = NULL) const;
	void SendPeerList(Peer* to) const;
};

extern PeersList peers_list;
#endif
