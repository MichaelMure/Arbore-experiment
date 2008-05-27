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

#ifndef PEER_BASE_H
#define PEER_BASE_H

#include <map>
#include <queue>
#include <list>
#include "pf_types.h"
#include "connection.h"
#include "packet.h"
#include "file_chunk_desc.h"

class PeerBase
{
	// how this peer maps refs to files
	std::map<uint32_t, std::list<FileChunkDesc> > asked_chunks;

protected:
	pf_addr addr;
	Connection* conn;

	int ts_diff;			  // diff between our timestamp and its timestamp */
	Packet* incoming;		  // packet we are receiving
	std::queue<Packet> send_queue;	  // packets we are sending (with flush)

	// how this peer maps refs to files
	std::map<uint32_t, std::string> file_refs;

	unsigned int flags;

	bool ReceivePacket();

	/** Add the chunk to the list of asked chunks */
	void AddAskedChunk(uint32_t ref, FileChunkDesc chunk);

	/** Delete the chunk from the list of asked chunks */
	void DelAskedChunk(uint32_t ref, FileChunkDesc chunk);

	/** Resend request for all of this chunk, and forget what was already asked */
	void ResendAskedChunks(uint32_t ref);
public:
	enum
	{
		/* Flags common to pfnet/pflan */
		SERVER      = 1 << 0,	  /**< This peer is a server */
		MERGING     = 1 << 1,	  /**< We are merging with this peer (between HELLO and END_OF_MERGE) */
		/* pfnet only flags */
	#ifdef PF_NET
		MERGING_ACK = 1 << 2,	  /**< We are waiting for an ACK */
		HIGHLINK    = 1 << 3,	  /**< This peer is a highlink */
		ANONYMOUS   = 1 << 4,	  /**< We don't know this peer (between connection and HELLO) */
	#endif
	};

	/* Exceptions */
	class MustDisconnect : public std::exception {};
	class SelfConnect : public std::exception {};

	PeerBase(pf_addr _addr, Connection* _conn, unsigned int _flags);
	virtual ~PeerBase();

	bool IsConnection() const { return !!conn; }
	void SetConnection(Connection* _conn) { conn = _conn; }
	Connection* GetConnection() const { return conn; }
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
