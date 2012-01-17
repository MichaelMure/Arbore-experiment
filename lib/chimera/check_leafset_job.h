/*
 * Copyright(C) 2008 Romain Bignon
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
 * This file contains some code from the Chimera's Distributed Hash Table,
 * written by CURRENT Lab, UCSB.
 *
 */

#ifndef CHECK_LEAFSET_JOB_H
#define CHECK_LEAFSET_JOB_H

#include <scheduler/job.h>
#include <util/time.h>

class Chimera;
class Routing;

/** Class to make the update of the leafset of a Host
 * When it's started by the scheduler, it will update delete bad link in the leafset and routing table
 * and send leafset update to hosts present in its leafset
 */
class CheckLeafsetJob : public Job
{
	Chimera* chimera_;
	Routing* routing_;
	size_t count_;

	bool Start();

public:

	CheckLeafsetJob(Chimera* chimera, Routing* routing)
		: Job(time::dtime(), REPEAT_PERIODIC, 20.0),
		  chimera_(chimera),
		  routing_(routing),
		  count_(0)
	{}
};

#endif /* CHECK_LEAFSET_JOB_H */
