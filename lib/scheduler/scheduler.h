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

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>

#include <util/pf_thread.h>

/** This class inherit from Thread and manage a static vector of instance of himself.
  * Each instance will run regularly and run job located in the scheduler_queue.
  * TODO: probably need to keep an instance of scheduler_queue rather than use a static
  * vector and a global variable.
  */
class Scheduler : public Thread
{
public:
	/** Create, add and run nb new instance of Scheduler in the vector. */
	static void StartSchedulers(size_t nb);

	/** Stop all the instance of Scheduler and destroy them. */
	static void StopSchedulers();

private:
	Scheduler() {}
	~Scheduler() {}

	/** Internal run implementation of the Scheduler's thread */
	void Loop();

	typedef std::vector<Scheduler*> SchedulerVector;
	static SchedulerVector schedulers;
};
#endif
