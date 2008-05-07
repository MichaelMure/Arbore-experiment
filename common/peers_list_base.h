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

#ifndef PEERS_LIST_BASE_H
#define PEERS_LIST_BASE_H

#include <map>
#include <vector>
#include "peer.h"
#include "mutex.h"
#include "packet.h"
#include "file_chunk.h"

class PeersListBase: public std::vector<Peer*>, public Mutex
{
protected:
	bool changed;
	typedef std::map<int, Peer*> PeerMap;
	PeerMap fd2peer;

	Peer* PeerFromFD(int fd);

	/** Remove a peer from list.
	 * It will *NOT* delete peer!
	 */
	virtual Peer* RemovePeer(Peer* p);

public:
	PeersListBase();
	virtual ~PeersListBase();

	unsigned int Size() const { return size(); }

	/** Add a peer to list */
	void Add(Peer* p);

	/** Remove a peer from the list and delete it. */
	void Erase(int fd);

	/** Remove a peer from the list and get it. */
	void Pop(int fd, Peer** pop);

	/** Flush messages to a peer. */
	void PeerFlush(int fd);

	/** Receive messages from a peer. */
	bool PeerReceive(int fd);

	/** Close all connections. */
	void CloseAll();

	/** This status variable can be used to know if list has changed. */
	bool IsChanged() const;
	void ClearChanged();

	/** Broadcast a packet to everybody.
	 * If but_one != NULL, do not send a packet to him.
	 */
	virtual void Broadcast(Packet pckt, const Peer* but_one = 0) const = 0;

	/** Create an ID not used by any other peer in network */
	pf_id CreateID();

	// TODO: move-me into private section
	Peer* PeerFromID(pf_id id) const;

	enum id_state_t
	{
		IS_UNKNOWN,
		IS_ON_NETWORK,
		IS_CONNECTED,
	};
	/** Return state of this ID on network. */
	enum id_state_t WhatIsThisID(pf_id id);

	/** Send a message to a peer */
	void SendMsg(pf_id id, const Packet &p) const;

	/** Send a message to a list of peers. */
	void SendMsg(IDList ids, const Packet &p) const;

	/** Request a file chunk to a peer. */
	void RequestChunk(std::string filename, pf_id id, off_t offset, size_t size);

	/** Send a file chunk to a peer.
	 * @param ref reference of this file request.
	 * @param id ID of peer
	 * @param chunk file chunk
	 */
	void SendChunk(uint32_t ref, pf_id id, FileChunk& chunk);
};
#endif
