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

public:
	Thread();
	virtual ~Thread();

	class CantRun : std::exception {};
	void Start() throw(CantRun);
	void Stop();
	bool IsRunning();

};

#endif /* PF_THREAD_H */
