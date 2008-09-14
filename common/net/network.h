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

#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <exception>
#include <vector>
#include <list>
#include <fcntl.h>
#include <map>
#include <time.h>

#include "pf_thread.h"
#include "pf_addr.h"
#include "dht/host.h"
#include "packet.h"

class MyConfig;

class Network : public Thread, protected Mutex
{
public:
	static const double RETRANSMIT_INTERVAL = 1.0;

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
			CantConnectTo(int _errno, const pf_addr _addr) : err(_errno), addr(_addr) {}
	};

private:

	/** Seconds before we try to retransmit a packet */
	int serv_sock;
	uint32_t seqend;

	void Listen(uint16_t port, const char* bind_addr) throw(CantOpenSock, CantListen);
public:

	Network();
	~Network();

	void CloseAll();
	void Loop();
	void OnStop();

	/* Read configuration and start listener, and connect to other servers */
	virtual void StartNetwork(MyConfig* conf);

	int Send(Host host, Packet pckt);
};

#endif /* NETWORK_H */
