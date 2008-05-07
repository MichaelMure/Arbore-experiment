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

#ifndef JOB_ADD_SHARER_H
#define JOB_ADD_SHARER_H

#include <string.h>
#include "job.h"
#include "pf_types.h"

class JobAddSharer : public Job
{
	std::string filename;
	pf_id id;
public:
	JobAddSharer(std::string _filename, pf_id _id) : Job(0, REPEAT_NONE), filename(_filename), id(_id) {}

	bool Start();

	job_type GetType() const { return JOB_ADD_SHARER; }
	std::string GetName() const { return "JobAddSharer"; }
};
#endif						  /* JOB_ADD_SHARER_H */
