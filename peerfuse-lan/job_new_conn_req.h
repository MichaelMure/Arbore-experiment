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

#ifndef JOB_NEW_CONN_REQ_H
#define JOB_NEW_CONN_REQ_H
#include "job_new_conn_req_base.h"
#include "pf_types.h"

class JobNewConnReq: public JobNewConnReqBase
{
	pf_id requested_by;
public:
	JobNewConnReq(::pf_addr addr, pf_id _requested_by) : JobNewConnReqBase(addr),  requested_by(_requested_by){}

	void OnSuccess();
	void OnFailure();
};
#endif						  /* JOB_NEW_CONN_REQ_H */
