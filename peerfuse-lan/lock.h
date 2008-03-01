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

#ifndef LOCK_H
#define LOCK_H
#include <time.h>
#include <list>
#include "peer.h"

// Once a lock has been acquired, it'll be keeped during 10s after its last
// use, unless some one asked to use it.
const int lock_timeout = 10;

class Lock
{
	time_t last_use; // Last time the lock was used
	Peer* previous_owner; // If the file is already locked by someone else, we ask him first, with the hope he'll release it for us
	Peer* next_owner; // This host asked for the lock
	std::list<Peer*> requested_to; // Hosts to which we requested the lock
	std::list<Peer*> accepted_by; // Hosts that granted us the lock

	bool used; // Tells wether the fs operation that requested it has already been performed

	void Request();
	void Release();
public:
	Lock();
	~Lock();

	void Use() { used = true; } // Tell wether we have performed the operation

	bool IsLocked();
	void AcceptedBy(Peer* p);
	void RefusedBy(Peer* p);
	void RequestedBy(Peer* p);
};

#endif // LOCK
