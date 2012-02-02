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
 */

#ifndef HANDLEPACKETJOB_H
#define HANDLEPACKETJOB_H

#include <scheduler/job.h>

#include "host.h"
#include "packet.h"
#include <chimera/chimera.h>

/** Call the packet handler.
 *
 * Because we don't want to monopolize the Network thread,
 * this job is used to ask a scheduler thread to call the
 * packet handler.
 */
class HandlePacketJob : public Job
{
	Chimera *chimera_;
	Host sender_;
	Packet pckt_;

	bool Start();

public:
	HandlePacketJob(Chimera        *chimera,
	                const Host&     sender,
	                const Packet&   pckt);
};

#endif // HANDLEPACKETJOB_H
