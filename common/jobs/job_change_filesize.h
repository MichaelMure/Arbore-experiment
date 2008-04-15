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

#ifndef JOB_CHANGE_FILESIZE_H
#define JOB_CHANGE_FILESIZE_H
#include <string.h>
#include "job.h"

class JobChangeFileSize : public Job
{
	std::string path;
	size_t size;
public:
	JobChangeFileSize(std::string _path, size_t _size) : Job(0, REPEAT_NONE), path(_path), size(_size) {}

	bool Start();
	job_type GetType() const { return JOB_CHANGE_FILESIZE; }
	std::string GetName() const { return "JobChangeFileSize"; }
};

#endif /* JOB_CHANGE_FILESIZE_H */
