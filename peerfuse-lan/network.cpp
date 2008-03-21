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

#include "network.h"

Network net;

Peer* Network::Connect(pf_addr addr)
{
	/* The NetworkBase::_connect() method doesn't send
	 * the Hello message, becaue pfnet must say if this is
	 * a highlink or lowlink before.
	 * As we don't care about link types, we can automatically
	 * send the NET_HELLO message.
	 */
	Peer* p = NetworkBase::Connect(addr);
	if(p)
		p->SendHello();

	return p;
}

void Network::Broadcast(Packet pckt, const Peer* but_one)
{
	for(PeerMap::iterator it = fd2peer.begin(); it != fd2peer.end(); ++it)
		if(it->second != but_one)
			it->second->SendMsg(pckt);
}
