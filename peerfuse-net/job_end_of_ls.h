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

#ifndef JOB_END_OF_LS_H
#define JOB_END_OF_LS_H

#ifndef PF_SERVER_MODE

#include <string.h>
#include "job.h"
#include "pf_types.h"

class JobEndOfLs : public Job
{
        std::string path;
public:
        JobEndOfLs(std::string _path)
		: Job(0, REPEAT_NONE),
		path(_path)
	{}

        bool Start();

        job_type GetType() const { return JOB_END_OF_LS; }
        std::string GetName() const { return "JobEndOfLs"; }
};

#endif                                            /* PF_SERVER_MODE */

#endif                                            /* JOB_END_OF_LS_H */

