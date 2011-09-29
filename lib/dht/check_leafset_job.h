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

#ifndef _CHECK_LEAFSET_JOB_H
#define _CHECK_LEAFSET_JOB_H

#include "scheduler/job.h"
#include "util/dtime.h"

class ChimeraDHT;
class ChimeraRouting;

class CheckLeafsetJob : public Job
{
	ChimeraDHT* chimera;
	ChimeraRouting* routing;
	size_t count;

	bool Start();

public:

	CheckLeafsetJob(ChimeraDHT* _chimera, ChimeraRouting* _routing)
		: Job(dtime(), REPEAT_PERIODIC, 20.0),
		  chimera(_chimera),
		  routing(_routing),
		  count(0)
	{}
};

#endif /* _CHECK_LEAFSET_JOB_H */
