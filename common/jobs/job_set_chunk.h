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
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * 
 */

#ifndef JOB_SET_CHUNK_H
#define JOB_SET_CHUNK_H
#include "job.h"
#include "job_types.h"
#include "file_content.h"

class JobSetChunk : public Job
{
	std::string filename;
	FileChunk chunk;
public:
	JobSetChunk(std::string _filename, FileChunk _chunk) : Job(0, REPEAT_NONE), filename(_filename), chunk(_chunk) {}

	bool Start();

	job_type GetType() const { return JOB_SET_CHUNK; }
	std::string GetName() const { return "JobSetChunk"; }
};
#endif						  /* JOB_SET_CHUNK_H */
