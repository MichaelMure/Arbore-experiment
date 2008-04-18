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

#ifndef JOB_NEW_CONNECTION_QUEUE_H
#define JOB_NEW_CONNECTION_QUEUE_H

#include <string>
#include <vector>
#include "job.h"
#include "pf_types.h"

class JobNewConnQueue : public Job, private std::vector<pf_addr>
{
	iterator current;
public:
	JobNewConnQueue(std::vector<pf_addr> addr_list)
		: Job(time(NULL), REPEAT_LESS_AND_LESS, 1),
		std::vector<pf_addr>(addr_list),
		current(begin())
		{}

	bool Start();
	job_type GetType() const {return JOB_NEW_CONN_QUEUE; }
	std::string GetName() const {return "JobNewConnQueue"; }
};
#endif
