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
#include "scheduler_queue.h"
#include "log.h"

Scheduler scheduler;

// Check if a queued job needs to be started
void Scheduler::Loop()
{
	sleep(1);
	if(scheduler_queue.GetQueueSize() == 0)
		return;

	time_t now = time(NULL);

	while(scheduler_queue.NextJobTime() && scheduler_queue.NextJobTime() < now)
	{
		log[W_DEBUG] << "Begining handling job." << scheduler_queue.GetQueueSize();

		/* We remove job from queue before calling it, to prevent
		 * crash if it tries to change queue list.
		 */
		Job* job = scheduler_queue.PopJob();

		if(job) /* The queue may have been emptied externaly */
		{
			if(job->DoStart())
				scheduler_queue.Queue(job);
			else
				delete job;
		}
	}
}

