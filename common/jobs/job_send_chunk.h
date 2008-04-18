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

#ifndef JOB_SEND_CHUNK_H
#define JOB_SEND_CHUNK_H
#include "job.h"
#include "job_types.h"
#include "pf_types.h"

class JobSendChunk : public Job
{
	uint32_t ref;
	pf_id sendto;
	off_t offset;
	size_t size;
public:
	JobSendChunk(uint32_t _ref, pf_id _sendto, off_t _offset, size_t _size) : Job(2, REPEAT_PERIODIC), ref(_ref), sendto(_sendto), offset(_offset), size(_size) {}

	bool Start();

	job_type GetType() const { return JOB_SEND_CHUNK; }
	std::string GetName() const { return "JobSendChunk"; }
};

#endif /* JOB_SEND_CHUNK_H */
