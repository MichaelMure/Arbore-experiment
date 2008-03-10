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

#ifndef JOB_H
#define JOB_H
#include <list>
#include <time.h>
#include "job_types.h"

class Job
{
	time_t start_time;
public:
	Job(time_t start_at) : start_time(start_at) {}
	virtual ~Job() {}

	time_t GetStartTime() const { return start_time; }
	virtual void Start() = 0;
	virtual job_type GetType() const = 0;
};

// TODO: split me
#include "pf_types.h"
#include "network.h"

class JobNewConnection : public Job, private pf_addr
{
	time_t dt;
public:
	JobNewConnection(pf_addr addr) :
	Job(time(NULL)), pf_addr(addr) { dt = 1; }
	JobNewConnection(const JobNewConnection* j) :
	Job(time(NULL) + j->dt),  pf_addr(pf_addr(*j)), dt(j->dt) {}

	void Start();
	job_type GetType() const {return JOB_NEW_CONNECT; }
};
#endif
