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

void PeersList::Broadcast(Packet pckt, const Peer* but_one)
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


