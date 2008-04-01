#ifndef SCHEDULER_QUEUE_H
#define SCHEDULER_QUEUE_H
#include <list>
#include "mutex.h"

class Job;

class SchedulerQueue : public std::list<Job*>, public Mutex /* TODO: Make me private */
{
public:
	SchedulerQueue() : Mutex(RECURSIVE_MUTEX) {}
	~SchedulerQueue() {}

	// Get the nxt job to be executed from the queue
	Job* PopJob();

	// Put a new job into queue
	void Queue(Job* job);

	// Remove a job from the queue
	void Cancel(Job* job);

	// Return the date of the next scheduled job
	time_t NextJobTime();

	size_t GetQueueSize();

};

extern SchedulerQueue scheduler_queue;
#endif /* SCHEDULER_QUEUE_H */
