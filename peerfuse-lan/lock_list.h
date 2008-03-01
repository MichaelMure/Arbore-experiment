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

#ifndef LOCK_LIST_H
#define LOCK_LIST_H
#include <map>
#include "peer.h"
#include "lock.h"

class LockList
{
	std::map<std::string, Lock> my_locks_list; // Maps the filename to its lock
	std::map<std::string, Peer*> peers_locks;

public:

	LockList();
	~LockList();

	//  a peer wants to lock a file
	bool LockRequest(Peer* p, std::string file);

	// we want to lock a file
	// true when the lock succeded
	// false when we don't have it
	bool TryLock(std:string file);

	// the lock expired delete it, if it still exists
	void TryDelete(std::string file);
};

#endif //LOCK_LIST_H

