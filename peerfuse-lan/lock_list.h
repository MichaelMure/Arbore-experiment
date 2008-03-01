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

