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

#ifndef JOB_ADVERTISE_FILE_H
#define JOB_ADVERTISE_FILE_H
#include "job.h"
#include "pf_types.h"

class JobAdvertiseFile : public Job
{
	std::string filename;
	pf_id advertise_to;
public:
	JobAdvertiseFile(std::string _filename, pf_id _advertise_to) : Job(0, REPEAT_NONE), filename(_filename), advertise_to(_advertise_to) {}

	bool Start();
	job_type GetType() const { return JOB_ADVERTISE_FILE; }
	std::string GetName() const { return "JobAdvertiseFile"; }
};
#endif						  /* JOB_ADVERTISE_FILE_H */
