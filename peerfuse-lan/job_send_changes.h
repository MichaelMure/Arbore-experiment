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
 * This product includes cryptographic software written by Eric Younganus
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * $Id$
 */

#ifndef JOB_SEND_CHANGES_H
#define JOB_SEND_CHANGES_H
#include <time.h>
#include "job.h"
#include "pf_types.h"

class JobSendChanges : public Job
{
	pf_id sender;
	time_t last_view;

public:
	JobSendChanges(pf_id _sender, time_t _last_view) : Job(0, REPEAT_NONE), sender(_sender), last_view(_last_view) {}
	~JobSendChanges() {}

	bool Start();
	job_type GetType() const { return JOB_SEND_CHANGES; }
	std::string GetName() const { return "JobSendChanges"; }
};
#endif						  /* JOB_SEND_CHANGES_H */
