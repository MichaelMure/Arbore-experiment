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

#ifndef JOB_OTHER_CONNECT_H
#define JOB_OTHER_CONNECT_H

#include <list>
#include "job.h"
#include "peer.h"

class JobOtherConnect : public Job
{
	Peer* connect_to;
	std::list<Peer*> is_connecting;
	std::list<Peer*> is_connected;

public:
	JobOtherConnect(Peer* _connect_to);
	JobOtherConnect(const JobOtherConnect* j);

	bool IsConnectingTo(pf_addr addr);
	void PeerConnected(Peer* peer);
	void Start();
	const job_type GetType() const {return JOB_OTHER_CONNECT; }
};

#endif
