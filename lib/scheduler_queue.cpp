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
#include "scheduler_queue.h"
#include "mutex.h"
#include "job.h"
#include "pf_log.h"

SchedulerQueue scheduler_queue;

// Queue a new job
void SchedulerQueue::Queue(Job* job)
{
	BlockLockMutex lock(this);
	pf_log[W_DEBUG] << "Queueing job \"" << typeid(job).name() << "\"";

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

void SchedulerQueue::CancelType(std::type_info type)
{
	BlockLockMutex lock(this);

	iterator it = begin();
	while(it != end())
	{
		if(typeid(*it) == type)
		{
			delete *it;
			it = erase(it);
		}
		else
			++it;
	}
}

double SchedulerQueue::NextJobTime()
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
