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
 * $Id$
 */

#include <algorithm>
#include "job_other_connect.h"
#include "job_types.h"
#include "peers_list.h"
#include "mutex.h"
#include "network.h"

JobOtherConnect::JobOtherConnect(pf_id _connect_to) : Job(time(NULL), REPEAT_PERIODIC, 1),
			connect_to(_connect_to)
{
}

bool JobOtherConnect::Start()
{
	return peers_list.CheckOtherConnection(connect_to, is_connecting, is_connected);
}

bool JobOtherConnect::IsConnectingTo(pf_addr addr)
{
	return (peers_list.GetPeerAddr(connect_to) == addr);
}

void JobOtherConnect::PeerConnected(pf_id peer)
{
	is_connected.push_back(peer);
}
