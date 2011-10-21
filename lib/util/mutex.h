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
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#ifndef MUTEX_H
#define MUTEX_H
#include <pthread.h>
#include <exception>

/** Enumeration of mutex type */
enum MutexType
{
	INVALID_MUTEX,
	NORMAL_MUTEX,
	RECURSIVE_MUTEX
};

class Mutex
{
private:
	pthread_mutex_t* _mutex;

	MutexType _type;

	void Init(enum MutexType type);

public:

	class MutexError : public std::exception {};

	/** Create a Mutex with a type
	* 
	* default type is NORMAL_MUTEX
	* 
	* @param _type type of the mutex
	*/
	Mutex(enum MutexType type = NORMAL_MUTEX);

	/** The copy constructor.
	 *  
	 * create a Mutex with the same type from which it's copied
	 *
	 * @param m copy mutex from the other mutex
	 */
	Mutex(const Mutex &m);
	
	/** Copy the type from another Mutex
	*
	* @param m Mutex from which type is copied
	* @return current Mutex initialize with the copied type
	*/
	Mutex& operator=(const Mutex& m);
	~Mutex();
	
	/** Lock the Mutex */
	void Lock() const;
	
	/** Unlock the Mutex */
	void Unlock() const;
};

/** Lock a Mutex locally.
 *
 * On a block, juste create an auto instance of this class.
 * When it will be removed (while exiting this block), mutex will be unlocked.
 *
 * Juste use:
 * BlockLockMutex lock(mutex);
 *
 */
class BlockLockMutex
{
	const Mutex* _p;
	BlockLockMutex(const BlockLockMutex&);
	BlockLockMutex& operator=(const BlockLockMutex&);

public:
	explicit BlockLockMutex(const Mutex* p)
		: _p(p)
	{
		_p->Lock();
	}
	~BlockLockMutex()
	{
		_p->Unlock();
	}
};
#endif
