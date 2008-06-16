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

#include "network.h"
#include "mutex.h"
#include "pf_log.h"
#include "scheduler_queue.h"
#include "job_types.h"

Network net;

void Network::ThrowHandler()
{
	try
	{
		Loop();
	}
	catch(Peer::PeerCantConnect &e)
	{
		scheduler_queue.CancelType(JOB_NEW_CONNECT);
		CloseAll();
		pf_log[W_ERR]  << "Host " << e.addr << " can't connect.";
		pf_log[W_ERR]  << "We'll stay disconnected.";
	}
	catch(PeerBase::SelfConnect &e)
	{
		/* pfnet throws this while handling the SSL handshake */
		/* We are connecting to ourself, this is quiet dangerous */
		pf_log[W_ERR] << "I'm trying to connect to myself, this is bad.";
		pf_log[W_ERR] << "Check your configuration, and check peerfuse is not already running.";
		exit(EXIT_FAILURE);
	}
}
