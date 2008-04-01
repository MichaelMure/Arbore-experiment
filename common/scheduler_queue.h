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
