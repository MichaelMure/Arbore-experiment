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

#ifndef JOB_SEND_MKFILE_H
#define JOB_SEND_MKFILE_H

#include "job.h"
#include "pf_file.h"
#include "pf_types.h"

class JobSendMkFile : public Job
{
	std::string file;
public:
	JobSendMkFile(std::string _file)
		: Job(time(NULL)+1, REPEAT_NONE),
		file(_file)
		{}
	~JobSendMkFile() {}

	bool Start();

	job_type GetType() const { return JOB_SEND_MKFILE; }
	std::string GetName() const { return "JobSendMkFile"; }
};

#endif						  /* JOB_SEND_MKFILE_H */
