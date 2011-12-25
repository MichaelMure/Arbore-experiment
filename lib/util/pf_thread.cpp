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

#include <errno.h>
#include <cstring>
#include <pthread.h>
#include "pf_thread.h"


Thread::Thread() : running(false)
{
	if(pthread_attr_init(&attr) != 0)
		throw CantCreate();
}

Thread::Thread(int scope, int detachstate)
	: running(false)
{
	if (pthread_attr_init (&attr) != 0)
		throw CantCreate();
	if (pthread_attr_setscope (&attr, scope) != 0)
		throw CantCreate();
	if (pthread_attr_setdetachstate (&attr, detachstate) != 0)
		throw CantCreate();
}

Thread::~Thread()
{
	Stop();
	pthread_attr_destroy(&attr);
}

void Thread::Start()
{
	if(IsRunning())
		return;
	OnStart();
	running = true;
	int r = pthread_create(&thread_id, &attr, StartThread, (void*)this);
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
		ThrowHandler();
}

void Thread::ThrowHandler()
{
	Loop();
}

bool Thread::IsRunning()
{
	return running;
}
