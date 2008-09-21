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

#ifndef JOB_NEW_CONN_REQ_BASE_H
#define JOB_NEW_CONN_REQ_BASE_H
#include "job.h"
#include "pf_types.h"

class JobNewConnReqBase : public Job, private pf_addr
{
public:
	JobNewConnReqBase(pf_addr addr) : Job(0 , REPEAT_NONE), pf_addr(addr) {}

	bool Start();
	virtual void OnSuccess() = 0;
	virtual void OnFailure() = 0;
	pf_addr GetAddr() const { return (pf_addr)*this; }
};
#endif						  /* NEW_CONN_REQ_BASE_H */
