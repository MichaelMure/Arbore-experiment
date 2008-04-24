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

#ifndef PEER_BASE_H
#define PEER_BASE_H

#include <map>
#include <queue>
#include "pf_types.h"
#include "connection.h"
#include "packet.h"

class PeerBase
{
protected:
	pf_addr addr;
	Connection* conn;

	int ts_diff;			  // diff between our timestamp and its timestamp */
	Packet* incoming;		  // packet we are receiving
	std::queue<Packet> send_queue;	  // packets we are sending (with flush)

	std::map<uint32_t, std::string> file_refs; // how this peer maps refs to files

	unsigned int flags;

	bool ReceivePacket();

public:
	enum
	{
		/* Flags common to pfnet/pflan */
		SERVER      = 1 << 0,	  /**< This peer is a server */
		MERGING     = 1 << 1,	  /**< We are merging with this peer (between HELLO and END_OF_MERGE) */
		/* pfnet only flags */
		MERGING_ACK = 1 << 2,	  /**< We are waiting for an ACK */
		HIGHLINK    = 1 << 3,	  /**< This peer is a highlink */
		ANONYMOUS   = 1 << 4,	  /**< We don't know this peer (between connection and HELLO) */
	};

	/* Exceptions */
	class MustDisconnect : public std::exception {};
	class SelfConnect : public std::exception {};

	PeerBase(pf_addr _addr, Connection* _conn, unsigned int _flags);
	virtual ~PeerBase();

	bool IsConnection() const { return !!conn; }
	int GetFd() const { return conn ? conn->GetFd() : -1; }
	pf_addr GetAddr() const { return addr; }

	time_t Timestamp(time_t ts) const;
	void SetTimestampDiff(uint32_t now);
	int GetTimestampDiff() const { return ts_diff; }

	pf_id GetID() const { return addr.id; }

	bool IsServer() const { return (flags & SERVER); }
	bool IsClient() const { return !(flags & SERVER); }

	void SetFlag(unsigned int f) { flags |= f; }
	void DelFlag(unsigned int f) { flags &= ~f; }
	bool HasFlag(unsigned int f) { return flags & f; }

	virtual void HandleMsg(struct Packet* pckt) = 0;
	virtual void SendMsg(const Packet& pckt) = 0;
	virtual void SendHello() = 0;

	virtual bool Receive() = 0;
	void Flush();
};
#endif						  /* PEER_BASE_H */
