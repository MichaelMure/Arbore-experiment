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

#ifndef SCHEDULER_QUEUE_H
#define SCHEDULER_QUEUE_H
#include <list>
#include <typeinfo>

#include <util/mutex.h>

class Job;

/** Queue of job, used by the Scheduler */
class SchedulerQueue : private std::list<Job*>, public Mutex
{
public:
	SchedulerQueue();
	~SchedulerQueue();

	/** @return the next job to be executed from the queue */
	Job* PopJob();

	/** Put a new job into the queue */
	void Queue(Job* job);

	/** Remove a job from the queue */
	void Cancel(Job* job);

	/** Remove all jobs of a specific type from the queue */
	void CancelType(std::type_info type);

	/** @return the date of the next scheduled job */
	double NextJobTime();

  /** @return the size of the queue */
	size_t GetQueueSize();
};

/* Singleton */
extern SchedulerQueue scheduler_queue;
#endif						  /* SCHEDULER_QUEUE_H */
