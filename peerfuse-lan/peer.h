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

#include "connection.h"
#include "pf_types.h"
#include "packet.h"
#include "peer_base.h"

class Peer;

class Peer : public PeerBase
{
	// Message handling functions
	void Handle_net_hello(struct Packet* pckt);
	void Handle_net_your_id(struct Packet* pckt);
	void Handle_net_get_struct_diff(struct Packet* pckt);
	void Handle_net_mkfile(struct Packet* pckt);
	void Handle_net_rmfile(struct Packet* pckt);
	void Handle_net_file_setattr(struct Packet* pckt);
	void Handle_net_peer_connection(struct Packet* pckt);
	void Handle_net_peer_connection_ack(struct Packet* pckt);
	void Handle_net_peer_connection_rst(struct Packet* pckt);
	void Handle_net_peer_connection_rejected(struct Packet* pckt);
	void Handle_net_peer_all_connected(struct Packet* pckt);
	void Handle_net_end_of_diff(struct Packet* pckt);
	void Handle_net_start_merge(struct Packet* pckt);
	void Handle_net_end_of_merge(struct Packet* pckt);
	void Handle_net_end_of_merge_ack(struct Packet* pckt);
	void Handle_net_who_has_file(struct Packet* pckt);
	void Handle_net_i_have_file(struct Packet* pckt);
public:
	/* Exceptions */
	class PeerCantConnect: public std::exception
	{
		public:
			pf_addr addr;
			PeerCantConnect(const pf_addr _addr) : addr(_addr) {}
	};

	/* Constructors */
	Peer(pf_addr addr, Connection* _conn, unsigned int _flags = 0);
	~Peer();

	void HandleMsg(struct Packet* pckt);

	void SendMsg(const Packet& pckt);
	void SendHello();
	void SendStartMerge();
	void SendGetStructDiff();

	bool Receive();
};
#endif
