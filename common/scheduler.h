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

#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <list>
#include <time.h>

class Job;

class Scheduler
{
	std::list<Job*> job_queue;
public:
	Scheduler();
	~Scheduler();

	// Check if a queued job needs to be started
	void HandleJobs();

	// Put a new job into queue
	void Queue(Job* job);

	// Remove a job from the queue
	void Cancel(Job* job);

	// Return the date of the next scheduled job
	time_t NextJobTime() const;

	size_t GetQueueSize() const { return job_queue.size(); }
	std::list<Job*>& GetQueue() { return job_queue; }
};
#endif
