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
 * 
 */

#ifndef PF_THREAD_H
#define PF_THREAD_H
#include <pthread.h>
#include <exception>
#include "mutex.h"

class Thread
{
private:
	Mutex running_lock;
	bool running;
	pthread_t thread_id;

	static void* StartThread(void*);
	void DoLoop();
protected:
	virtual void Loop() = 0;
	/* Called before the main loop starts */
	virtual void OnStart() {};
	/* Called after the main loop finished */
	virtual void OnStop() {};
	/* Use it to catch exceptions from the main loop */
	virtual void ThrowHandler();
public:
	Thread();
	virtual ~Thread();

	class CantRun : std::exception {};
	void Start() throw(CantRun);
	void Stop();
	bool IsRunning();

};
#endif						  /* PF_THREAD_H */
