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
