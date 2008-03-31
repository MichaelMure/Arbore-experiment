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

#include <pthread.h>
#include "pf_thread.h"

Thread::Thread() : running(false)
{
}

Thread::~Thread()
{
	Stop();
}

void Thread::Start() throw(CantRun)
{
	if(IsRunning())
		return;
	OnStart();
	running = true;
	int r = pthread_create(&thread_id, NULL, StartThread, (void*)this);
	if(r)
		throw CantRun();
}

void *Thread::StartThread(void* arg)
{
	Thread* t = static_cast<Thread*>(arg);
	t->DoLoop();
	pthread_exit(NULL);
	return NULL;
}

void Thread::Stop()
{
	if(!IsRunning())
		return;
	running = false;
	pthread_join(thread_id, NULL);
	OnStop();
}

void Thread::DoLoop()
{
	while(IsRunning())
		Loop();
}

bool Thread::IsRunning()
{
	running_lock.Lock();
	bool r = running;
	running_lock.Unlock();
	return r;
}
