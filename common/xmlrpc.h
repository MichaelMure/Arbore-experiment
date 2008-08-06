/*
 * Copyright(C) 2008 Mathieu Virbel <tito@bankiz.org>
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
 */

#ifndef PF_XMLRPC_H
#define PF_XMLRPC_H

#include <list>
#include <XmlRpc.h>
#include "pf_thread.h"
#include "xmlrpc_server.h"
#include "net/network_base.h"
#include "net/peers_list_base.h"

#define XMLRPC_VERSION	0

class XmlRpcThread : public Thread
{
public:
	XmlRpcThread();
	~XmlRpcThread();

	void SetNetworkBase(NetworkBase& network) { this->network = & network; };
	NetworkBase* GetNetworkBase() { return this->network; }
	void SetPeersListBase(PeersListBase& peers_list) { this->peers_list = & peers_list; };
	PeersListBase* GetPeersListBase() { return this->peers_list; }
	
protected:
	void Loop();
	void OnStart();
	void OnStop();

private:
	int port;
	bool started;

	NetworkBase *network;
	PeersListBase *peers_list;

	PfXmlRpcServer s;
	std::list<XmlRpc::XmlRpcServerMethod*> methods;
};

extern XmlRpcThread xmlrpc;

#endif
