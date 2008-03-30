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
protected:
	typedef enum
	{
		REPEAT_NONE,
		REPEAT_PERIODIC,
		REPEAT_LESS_AND_LESS
	} repeat_type_t;

private:
	time_t start_time;
	repeat_type_t repeat_type;
	int repeat_delta;

protected:
	// Start the job, returns true if the job needs to be restarted later
	virtual bool Start() = 0;
public:
	Job(time_t start_at, repeat_type_t _repeat_type, int _repeat_delta = 0) : start_time(start_at), repeat_type(_repeat_type), repeat_delta(_repeat_delta) {}
	virtual ~Job() {}

	// Start the job, returns true if the job needs to be restarted later
	bool DoStart();
	time_t GetStartTime() const { return start_time; }
	virtual job_type GetType() const = 0;
};
#endif /* JOB_H */
