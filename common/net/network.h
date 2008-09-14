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
#include "hosts_list.h"
#include "packet.h"

class MyConfig;
class ResendPacketJob;
class PacketTypeList;

class Network : public Thread, protected Mutex
{
public:
	static const double RETRANSMIT_INTERVAL = 1.0; /**< Seconds before we try to retransmit a packet */
	static const unsigned int MAX_RETRY = 3;       /**< Maximum tries before abording resend a packet */
	static const size_t PACKET_MAX_SIZE = 1024;    /**< Maximum size for packets */

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

	typedef std::map<int, PacketTypeList*> SockMap;
	SockMap socks;    /**< contains all socks listened and the related packettypelist */
	fd_set socks_fd_set;
	int highsock;

	HostsList hosts_list;
	std::vector<ResendPacketJob*> resend_list;
	uint32_t seqend;

public:

	Network();
	~Network();

	/** Listen an UDP port.
	 *
	 * @param packet_type_list  this is the object which describes the packet types and
	 *                          them handlers.
	 * @param port  the listened port
	 * @param bind_addr  the listened address
	 * @return  the file descriptor
	 */
	int Listen(PacketTypeList* packet_type_list, uint16_t port, const char* bind_addr) throw(CantOpenSock, CantListen);

	void CloseAll();
	void Loop();
	void OnStop();

	/* Read configuration and start listener, and connect to other servers */
	virtual void StartNetwork(MyConfig* conf);

	bool Send(int sock, Host host, Packet pckt);
};

#endif /* NETWORK_H */
