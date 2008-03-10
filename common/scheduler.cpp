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

#include <list>
#include <time.h>
#include <algorithm>
#include "job.h"
#include "scheduler.h"
#include "log.h"

Scheduler::Scheduler()
{
}

Scheduler::~Scheduler()
{
}

// Check if a queued job needs to be started
void Scheduler::HandleJobs()
{
	if(job_queue.size() == 0)
		return;

	time_t now = time(NULL);

	while(job_queue.size() != 0 && job_queue.front()->GetStartTime() < now)
	{
		log[W_INFO] << "Begining handling job." << job_queue.size();
		job_queue.front()->Start();
		delete job_queue.front();
		job_queue.erase(job_queue.begin());
	}
}

// Queue a new job
void Scheduler::Queue(Job* job)
{
	log[W_INFO] << "New job queued";
	for(std::list<Job*>::iterator it = job_queue.begin();
		it != job_queue.end();
		++it)
	{
		if(job->GetStartTime() < (*it)->GetStartTime())
		{
			job_queue.insert(it, job);
			return;
		}
	}

	job_queue.push_back(job);
}

void Scheduler::Cancel(Job* job)
{
	std::list<Job*>::iterator it = std::find(job_queue.begin(), job_queue.end(), job);

	if(it == job_queue.end())
		return;

	job_queue.erase(it);
	delete job;
}

time_t Scheduler::NextJobTime() const
{
	if(job_queue.size() == 0)
		return 0;
	return job_queue.front()->GetStartTime();
}
