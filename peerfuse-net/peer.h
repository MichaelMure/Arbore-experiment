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

#ifndef PEER_H
#define PEER_H

#include "connection_ssl.h"
#include "pf_types.h"
#include "packet.h"
#include "peer_base.h"
#include "file_chunk.h"

class Peer;

typedef std::vector<Peer*> StaticPeersList;

class Peer : public PeerBase
{
	/* Network is representer for me like this:
	 *  me
	 *  |- direct downpeer1
	 *  |  |- downpeer2
	 *  |  |  `- downpear3
	 *  |  `- downpeer4
	 *  `- direct downpeer5
	 *
	 *  If this Peer is "downpeer2", uplink points to "direct downpeer1",
	 *  and downlinks contain "downpeer3".
	 *
	 *  NOTE: IT IS FOR HIGHLINKS ONLY!
	 */
	pf_id uplink;				  /**< Peer where this peer is connected */
	std::vector<pf_id> downlinks;		  /**< Peer connected to this one */

	// Receiving functions
	void Handle_net_hello(struct Packet* pckt);
	void Handle_net_mkfile(struct Packet* pckt);
	void Handle_net_rmfile(struct Packet* pckt);
	void Handle_net_peer_connection(struct Packet* pckt);
	void Handle_net_peer_goodbye(struct Packet* pckt);
	void Handle_net_end_of_merge(struct Packet* pckt);
	void Handle_net_end_of_merge_ack(struct Packet* pckt);
	void Handle_net_i_have_file(struct Packet* pckt);
	void Handle_net_want_ref_file(struct Packet* pckt);
	void Handle_net_ref_file(struct Packet* pckt);
	void Handle_net_want_chunk(struct Packet* pckt);
	void Handle_net_chunk(struct Packet* pckt);
public:
	/* Constructors */
	Peer(pf_addr addr, Connection* _conn, unsigned int _flags = 0, pf_id parent = 0);
	~Peer();

	bool IsHighLink() const { return flags & HIGHLINK; }
	bool IsLowLink() const { return !(flags & HIGHLINK); }
	void SetHighLink(bool l = true) { l ? SetFlag(HIGHLINK) : DelFlag(HIGHLINK); }

	pf_id GetUpLink() const { return uplink; }
	std::vector<pf_id> GetDownLinks() const { return downlinks; }

	bool IsAnonymous() const { return (flags & ANONYMOUS); }

	void HandleMsg(struct Packet* pckt);

	void SendMsg(const Packet& pckt);
	void SendHello();
	virtual bool Receive();

	void RequestChunk(std::string filename, off_t offset, size_t size);
	void SendChunk(std::string filename, FileChunk& chunk);
};
#endif
