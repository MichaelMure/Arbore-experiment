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

#ifndef JOB_MKFILE_H
#define JOB_MKFILE_H
#include "job.h"
#include "file_entry.h"
#include "pf_types.h"

class JobMkFile : public Job
{
	std::string file;
	pf_stat stat;
	pf_id sender;
	IDList sharers;
	bool keep_newest;
	bool erase_on_modification;
public:
	JobMkFile(std::string _file, pf_stat _stat, IDList _sharers, pf_id _sender, bool _keep_newest, bool _erase_on_modification)
		: Job(0, REPEAT_NONE),
		file(_file),
		stat(_stat),
		sender(_sender),
		sharers(_sharers),
		keep_newest(_keep_newest),
		erase_on_modification(_erase_on_modification)
		{}
	~JobMkFile() {}

	bool Start();

	job_type GetType() const { return JOB_MKFILE; }
	std::string GetName() const { return "JobMkFile"; }
};
#endif						  /* JOB_MKFILE_H */
