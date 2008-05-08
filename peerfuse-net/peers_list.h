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

#ifndef PEERS_LIST_H
#define PEERS_LIST_H

#include "peers_list_base.h"

class PeersList: public PeersListBase
{
	StaticPeersList GetDownLinks(Peer* p) const;

	void _send_peer_list(Peer* to, Peer* from) const;

	virtual Peer* RemovePeer(Peer* p);

public:
	PeersList() {}
	~PeersList() {}

	void EraseFromID(pf_id id);
	Peer* RemoveFromID(pf_id id);

	/** Remove all downlinks from a peer
	 * @param p remove all downlinks of this peer
	 * @return a vector of all pf_addr removed
	 */
	std::vector<pf_addr> RemoveDownLinks(Peer* p);

	/** Return a list of connected high linkes */
	StaticPeersList GetDirectHighLinks() const;

	void GivePacketTo(pf_id id, Packet* packet) const;

	virtual void Broadcast(Packet pckt, const Peer* but_one = NULL) const;
	void SendPeerList(Peer* to) const;

	void GetMapOfNetwork(std::vector<std::string> &list, Peer* from = 0, std::string prefix = "") const;

	void PeerSetConnection(pf_id id, int fd);
};

extern PeersList peers_list;
#endif
