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

#ifndef NETWORK_BASE_H
#define NETWORK_BASE_H

#include <exception>
#include <vector>
#include <list>
#include <fcntl.h>
#include <map>
#include <time.h>

#include "pf_thread.h"
#include "pf_ssl.h"
#include "peer.h"
#include "pf_types.h"

class MyConfig;

class NetworkBase : public Thread, protected Mutex
{
public:

	/* Exceptions */
	class CantOpenSock : public std::exception {};
	class CantListen : public std::exception
	{
		public:
			int port;
			CantListen(int _port) : port(_port) {}
	};
	class CantResolvHostname : public std::exception {};
	class CantConnectTo : public std::exception
	{
		public:
			int err;
			pf_addr addr;
			CantConnectTo(int _errno, const pf_addr _addr) : err(_errno),addr(_addr) {}
	};

private:
	fd_set global_read_set;
	int serv_sock;

	std::list<pf_addr> disconnected_list;

protected:
	Ssl *ssl;

	void Listen(uint16_t port, const char* bind) throw (CantOpenSock, CantListen);
	void CloseAll();
	void Loop();
	void OnStop();

	/** Add a peer on network */
	virtual Peer* AddPeer(Peer* peer);
	void RemovePeer(int fd, bool try_reconnect = true);

	void AddDisconnected(const pf_addr& addr);
	void DelDisconnected(const pf_addr& addr);

public:
	/* Constructors */
	NetworkBase();
	virtual ~NetworkBase();

	/** Virtual method called when a peer is removed. */
	virtual void OnRemovePeer(Peer* peer) {}

	/* Read configuration and start listener, and connect to other servers */
	virtual void StartNetwork(MyConfig* conf);

	/* Make a pf_addr from a hostname/port
	 */
	pf_addr MakeAddr(const std::string& hostname, uint16_t port);

	/* Connect to a pf_addr.
	 */
	virtual void Connect(pf_addr addr);
};
#endif