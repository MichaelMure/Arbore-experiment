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


#ifndef NETWORK_BASE_H
#define NETWORK_BASE_H

#include <pthread.h>
#include <exception>
#include <vector>
#include <list>
#include <fcntl.h>
#include <map>
#include <time.h>

#include "scheduler.h"
#include "peer.h"
#include "pf_types.h"

typedef std::vector<Peer*> PeerList;
typedef std::map<unsigned int, Peer*> PeerMap;
class MyConfig;

class NetworkBase
{
public:

	/* Exceptions */
	class CantRunThread : public std::exception {};
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
	bool running;
	pthread_t thread_id;
	static void* StartThread(void*);

	fd_set global_read_set;
	fd_set global_write_set;
	int serv_sock;
	int highsock;

	uint16_t listening_port;

protected:
	id_t my_id;

	PeerList peer_list;
	PeerMap fd2peer;

	void RemovePeer(Peer* peer);

	void Listen(uint16_t port, const char* bind) throw (CantOpenSock, CantListen);
	void CloseAll();

public:
	Scheduler scheduler;

	/* Constructors */
	NetworkBase() throw (CantRunThread);
	virtual ~NetworkBase();

	/** Main loop (select()) */
	void Main();

	const PeerList& GetPeerList() { return peer_list; }
	Peer* AddPeer(Peer* peer);

	bool IsRunning() const { return running; }

	uint16_t GetListeningPort() const { return listening_port; }


	/* When a peer want to send a packet, set its fd on the
	 * global_write_set to call peer->Flush() in Main() loop.
	 */
	void HavePacketToSend(const Peer* peer);

	/* Broadcast a packet to everybody.
	 * If but_one != NULL, do not send a packet to him.
	 */
	virtual void Broadcast(Packet pckt, const Peer* but_one = 0) = 0;

	/* Read configuration and start listener, and connect to other servers */
	virtual Peer* Start(MyConfig* conf);

	/* Connect to a specific host and port.
	 */
	Peer* Connect(const std::string& hostname, uint16_t port);

	/* Connect to a pf_addr.
	 */
	virtual Peer* Connect(const pf_addr addr);

	id_t GetMyID() const { return my_id; }
	void SetMyID(const id_t id) { my_id = id; }

	/* Create an ID not used by any other peer in network */
	id_t CreateID();


	virtual void AddDisconnected(const pf_addr& addr) {}
	virtual void DelDisconnected(const pf_addr& addr) {}
};

#endif
