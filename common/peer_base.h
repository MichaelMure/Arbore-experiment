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

#include "pf_types.h"
#include "packet.h"

class PeerBase
{

public:
	virtual ~PeerBase() {};

	virtual pf_id GetID() const = 0;
	virtual int GetFd() const = 0;
	virtual pf_addr GetAddr() const = 0;

	virtual time_t Timestamp(time_t ts) = 0;

	virtual bool IsServer() const = 0;
	virtual bool IsClient() const = 0;

	virtual void SetFlag(unsigned int f) = 0;
	virtual void DelFlag(unsigned int f) = 0;
	virtual bool HasFlag(unsigned int f) = 0;

	virtual void HandleMsg(struct Packet* pckt) = 0;

	virtual void Flush() = 0;
	virtual void SendMsg(const Packet& pckt) = 0;
	virtual void SendHello() = 0;
	virtual bool Receive() = 0;
};
#endif						  /* PEER_BASE_H */
