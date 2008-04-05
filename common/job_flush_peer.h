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

#ifndef JOB_FLUSH_PEER_H
#define JOB_FLUSH_PEER_H
#include "job.h"
#include "job_types.h"
#include "pf_types.h"

class JobFlushPeer : public Job
{
	pf_id id;
public:
	JobFlushPeer(pf_id _id) : Job(0, REPEAT_NONE), id(_id) {};
	~JobFlushPeer() {};

	bool Start();
	job_type GetType() const { return JOB_FLUSH_PEER; }
};

#endif /* JOB_FLUSH_PEER_H */
