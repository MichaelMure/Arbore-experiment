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
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * 
 */

#ifndef PEERS_LIST_H
#define PEERS_LIST_H

#include "net/peers_list_base.h"

class PeersList: public PeersListBase
{
public:
	PeersList() {}
	~PeersList() {}

	/* Broadcast a packet to everybody.
	 * If but_one != NULL, do not send a packet to him.
	 */
	virtual void Broadcast(Packet pckt, const Peer* but_one = 0) const;

	void SendGetStructDiff(pf_id to) const;

	bool CheckOtherConnection(pf_id connect_to, std::list<pf_id>& is_connecting, std::list<pf_id>& is_connected) const;
};

extern PeersList peers_list;
#endif
