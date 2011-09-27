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

#ifndef JOB_NEW_CONNECTION_H
#define JOB_NEW_CONNECTION_H
#include <string>
#include "job.h"
#include "dtime.h"
#include "pf_types.h"

class JobNewConnection : public Job, private pf_addr
{
public:
	JobNewConnection(pf_addr addr) : Job(time::dtime(), REPEAT_LESS_AND_LESS, 1.0), pf_addr(addr) {}

	bool Start();
	bool IsMe(const pf_addr&);
};
#endif
