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

#ifndef NETWORK_H
#define NETWORK_H

#include <pthread.h>
#include <exception>
#include <vector>
#include <list>
#include <fcntl.h>
#include <map>
#include <time.h>

#include "net/network_base.h"
#include "pf_types.h"
#include "peer.h"

class Network: public NetworkBase
{
private:

protected:
	void ThrowHandler();
public:
	/* Constructors */
	Network();
	virtual ~Network();

	virtual Peer* AddPeer(Peer* peer);
	virtual void OnRemovePeer(Peer* peer);

	virtual void StartNetwork(MyConfig* conf);
};

extern Network net;
#endif
