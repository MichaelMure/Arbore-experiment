#include <algorithm>
#include "scheduler_queue.h"
#include "mutex.h"
#include "job.h"

SchedulerQueue scheduler_queue;

// Queue a new job
void SchedulerQueue::Queue(Job* job)
{
	BlockLockMutex lock(this);
	for(iterator it = begin();
		it != end();
		++it)
	{
		if(job->GetStartTime() < (*it)->GetStartTime())
		{
			insert(it, job);
			return;
		}
	}

	push_back(job);
}

Job* SchedulerQueue::PopJob()
{
	BlockLockMutex lock(this);
	if(size() == 0)
		return NULL;
	Job* j = front();
	pop_front();
	return j;
}

void SchedulerQueue::Cancel(Job* job)
{
	BlockLockMutex lock(this);
	iterator it = find(begin(), end(), job);

	if(it == end())
		return;

	erase(it);
	delete job;
}

time_t SchedulerQueue::NextJobTime()
{
	BlockLockMutex lock(this);
	if(size() == 0)
		return 0;
	return front()->GetStartTime();
}

size_t SchedulerQueue::GetQueueSize()
{
	BlockLockMutex lock(this);
	return size();
}

