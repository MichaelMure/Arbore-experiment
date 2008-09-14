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

#include <list>
#include "dtime.h"
#include <algorithm>
#include "job.h"
#include "scheduler.h"
#include "scheduler_queue.h"
#include "pf_log.h"

std::vector<Scheduler*> Scheduler::schedulers;

void Scheduler::StartSchedulers(size_t nb)
{
	for(size_t i = 0; i < nb; ++i)
	{
		Scheduler* scheduler = new Scheduler;
		scheduler->Start();
		schedulers.push_back(scheduler);
	}
}

void Scheduler::StopSchedulers()
{
	for(std::vector<Scheduler*>::iterator it = schedulers.begin();
	    it != schedulers.end();
	    ++it)
	{
		(*it)->Stop();
		delete *it;
	}

	schedulers.stop();
}

// Check if a queued job needs to be started
void Scheduler::Loop()
{
	usleep(100000);				  // every 0.1 sec

	double now = dtime();

	while(scheduler_queue.GetQueueSize() != 0 && scheduler_queue.NextJobTime() < now)
	{
		/* We remove job from queue before calling it, to prevent
		 * crash if it tries to change queue list.
		 */
		Job* job = scheduler_queue.PopJob();

		if(job)				  /* The queue may have been emptied externaly */
		{
			pf_log[W_DEBUG] << "Begining handling job \"" << job->GetName() << "\". Queue_size:" <<scheduler_queue.GetQueueSize();
			if(job->DoStart())
				scheduler_queue.Queue(job);
			else
				delete job;
		}
	}
}
