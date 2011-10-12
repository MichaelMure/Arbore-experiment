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

#include <algorithm>

#include <util/mutex.h>
#include <util/pf_log.h>
#include "job.h"
#include "scheduler_queue.h"

SchedulerQueue scheduler_queue;

SchedulerQueue::SchedulerQueue()
  : Mutex(RECURSIVE_MUTEX)
{
}

SchedulerQueue::~SchedulerQueue()
{
}

void SchedulerQueue::Queue(Job* job)
{
	BlockLockMutex lock(this);
	pf_log[W_DEBUG] << "Queueing job \"" << typeid(job).name() << "\"";

	for(JobList::iterator it = jobs.begin();
		it != jobs.end();
		++it)
	{
		if(job->GetStartTime() < (*it)->GetStartTime())
		{
			jobs.insert(it, job);
			return;
		}
	}

	jobs.push_back(job);
}

Job* SchedulerQueue::PopJob()
{
	BlockLockMutex lock(this);
	if(jobs.size() == 0)
		return NULL;
	Job* j = jobs.front();
	jobs.pop_front();
	return j;
}

void SchedulerQueue::Cancel(Job* job)
{
	BlockLockMutex lock(this);
	JobList::iterator it = find(jobs.begin(), jobs.end(), job);

	if(it == jobs.end())
		return;

	jobs.erase(it);
	delete job;
}

void SchedulerQueue::CancelType(std::type_info type)
{
	BlockLockMutex lock(this);

	JobList::iterator it = jobs.begin();
	while(it != jobs.end())
	{
		if(typeid(*it) == type)
		{
			delete *it;
			it = jobs.erase(it);
		}
		else
			++it;
	}
}

double SchedulerQueue::NextJobTime()
{
	BlockLockMutex lock(this);
	if(jobs.size() == 0)
		return 0;
	return jobs.front()->GetStartTime();
}

size_t SchedulerQueue::GetQueueSize()
{
	BlockLockMutex lock(this);
	return jobs.size();
}
