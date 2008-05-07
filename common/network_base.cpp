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
 * This product includes cryptographic software written by Eric Younganus
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * $Id$
 */

#include <iostream>
#include <list>
#include <algorithm>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#include "pf_ssl_ssl.h"
#include "pf_ssl_nossl.h"
#include "connection.h"
#include "connection_ssl.h"
#include "log.h"
#include "libconfig.h"
#include "network_base.h"
#include "tools.h"
#include "job_new_connection.h"
#include "scheduler_queue.h"
#include "peers_list.h"
#include "pf_thread.h"
#include "environment.h"
#include "content_list.h"

NetworkBase::NetworkBase()
			: Mutex(RECURSIVE_MUTEX),
			serv_sock(-1),
			ssl(NULL)
{
	FD_ZERO(&global_read_set);
	srand(time(NULL));
}

NetworkBase::~NetworkBase()
{
	delete ssl;
	if(serv_sock != -1)
	{
		shutdown(serv_sock, SHUT_RDWR);
		close(serv_sock);
	}
}

Peer* NetworkBase::AddPeer(Peer* p)
{
	log[W_CONNEC] << "-> Added a new peer: " << p->GetAddr() << " (" << p->GetFd() <<")";

	#if 0
	if(p->GetFd() >= 0)
	{
		Lock();
		FD_SET(p->GetFd(), &global_read_set);

		if(p->GetFd() > highsock)
			highsock = p->GetFd();
		Unlock();
	}
	#endif

	if(peers_list.Size() == 0)
	{
		// This is the first peer to which we connect
		// No need to make more connections attempt to other peers
		scheduler_queue.CancelType(JOB_NEW_CONNECT);
	}

	peers_list.Add(p);

	return p;
}

void NetworkBase::RemovePeer(int fd, bool try_reconnect)
{
	BlockLockMutex lock(this);

	Peer* p;
	peers_list.Pop(fd, &p);
	content_list.RemovePeerRefs(p->GetID());
	content_list.DelReferer(p->GetID());

	OnRemovePeer(p);

	if(p && p->GetAddr().port > 0 && try_reconnect)
		AddDisconnected(p->GetAddr());

	delete p;

	if(FD_ISSET(fd, &global_read_set)) FD_CLR(fd, &global_read_set);
}

void NetworkBase::Loop()
{
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	usleep(1000);				  // sleep 0.001 sec -> give the other thread a chance to lock
	BlockLockMutex lock(this);
	fd_set tmp_read_set = global_read_set;
	int events;

	/* TODO: this is useless to use select() here, because now there is only one fd (server sock) */
	if((events = select(serv_sock + 1, &tmp_read_set, NULL, NULL, &timeout)) < 0)
	{
		if(errno != EINTR)
		{
			log[W_ERR] << "Error in select() (" << errno << ": " << strerror(errno) << ")";
			return;
		}
	}
	else if(events > 0)			  /* events = 0 means that there isn't any event (but timeout expired) */
	{
		if(FD_ISSET(serv_sock, &tmp_read_set))
		{
			struct sockaddr_in newcon;
			unsigned int addrlen = sizeof newcon;
			int newfd = accept(serv_sock, (struct sockaddr *) &newcon, &addrlen);

			if(newfd != -1)
			{
				pf_addr addr;
				addr.ip[3] = newcon.sin_addr.s_addr;

				Connection *peer_conn;
				try
				{
					peer_conn = ssl->Accept(newfd);
					AddPeer(new Peer(addr, peer_conn));
					DelDisconnected(addr);
				}
				catch(SslSsl::SslHandshakeFailed &e)
				{
					log [W_WARNING] << "SSL handshake failure: " << e.GetString();
				}

			}
		}
	}

	BlockLockMutex peers_lock(&peers_list);
	/* About exceptions, when in a catch block we remove a peer,
	 * the p iterator isn't valid anymore.
	 * So, as we can't get a new iterator value for peers_list, we must
	 * exit loop. It is not really a problem, because all active fd will
	 * wait for the next iteration.
	 * There would be a problem if peer wasn't deleted, because the first peers
	 * would have a priority on other peers.
	 * So, don't forget the "break" after a RemovePeer() call!
	 */
	for(PeersList::iterator p = peers_list.begin();
		p != peers_list.end() && !peers_list.IsChanged();
		++p)
	{
		Peer* peer = *p;
		if(peer->GetFd() < 0)
			continue;

		// Perform read operations
		try
		{
			while(peer->Receive())
				;
		}
		catch(Connection::RecvError &e)
		{
			log[W_WARNING] << "recv() error: " << e.GetString();
			RemovePeer(peer->GetFd());
			break;
		}
		catch(Packet::Malformated &e)
		{
			log[W_ERR] << "Received malformed message!";
			RemovePeer(peer->GetFd());
			break;
		}
		// TODO:Rename me into something like BanPeer as we won't reconnect to him
		/* Yes and no, it is not the good name because it's not really a ban -romain */
		catch(Peer::MustDisconnect &e)
		{
			log[W_WARNING] << "Must disconnected";
			RemovePeer(peer->GetFd(), false);
			break;
		}

		// Perform write operations
		try
		{
			peers_list.PeerFlush(peer->GetFd());
		}
		catch(Connection::WriteError &e)
		{
			log[W_WARNING] << "send() error: " << e.GetString();
			RemovePeer(peer->GetFd());
			break;
		}

	}

	peers_list.ClearChanged();
}

void NetworkBase::OnStop()
{
	/* We leave */
	CloseAll();
}

void NetworkBase::CloseAll()
{
	BlockLockMutex lock(this);
	peers_list.CloseAll();

	if(serv_sock >= 0)
	{
		close(serv_sock);
		serv_sock = -1;
	}

	FD_ZERO(&global_read_set);

	delete ssl;
	ssl = 0;
}

pf_addr NetworkBase::MakeAddr(const std::string& hostname, uint16_t port)
{
	assert(ssl);

	const char* ip = hostname.c_str();
	static pf_addr none_addr;
	pf_addr addr = none_addr;

	addr.port = port;

	/* Resolve hostname
	 * TODO: support ipv6
	 */
	if(!is_ip(ip))
	{
		struct hostent *hp = gethostbyname(ip);
		if(!hp)
			throw CantResolvHostname();

		addr.ip[3] = *(in_addr_t *)(*(hp->h_addr_list));
	}
	else
		addr.ip[3] = inet_addr(ip);

	return addr;
}

void NetworkBase::Connect(pf_addr addr)
{
	BlockLockMutex lock(this);
	assert(ssl);

	if(addr.id && peers_list.WhatIsThisID(addr.id) == PeersListBase::IS_CONNECTED)
		return;

	/* Socket creation
	 * Note: during initialisation, as = {0} isn't compatible everywhere, we set it to
	 * a static variable which initialises by itself
	 */
	static struct sockaddr_in fsocket_init;
	struct sockaddr_in fsocket = fsocket_init;
	int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(sock < 0)
		throw CantConnectTo(errno, addr);

	fsocket.sin_addr.s_addr = addr.ip[3];

	struct timeval timeout;
	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = 2;			  // 2seconds timeout
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

	fsocket.sin_family = AF_INET;
	fsocket.sin_port = htons(addr.port);

	log[W_INFO] << "Connecting to " << addr;
	/* Connexion */
	if(connect(sock, (struct sockaddr *) &fsocket, sizeof fsocket) < 0)
	{
		close(sock);
		throw CantConnectTo(errno, addr);
	}

	Connection* conn;
	try
	{
		conn = ssl->Connect(sock);
	}
	catch(SslSsl::SslHandshakeFailed &e)
	{
		log [W_WARNING] << "SSL handshake failure: " << e.GetString();
		throw CantConnectTo(errno, addr);
	}

	AddPeer(new Peer(addr, conn, Peer::SERVER));

	DelDisconnected(addr);
}

void NetworkBase::Listen(uint16_t port, const char* bindaddr) throw(CantOpenSock, CantListen)
{
	BlockLockMutex lock(this);
	assert(ssl);

	unsigned int reuse_addr = 1;
	struct sockaddr_in localhost;		  /* bind info structure */

	/* Already listening */
	if(serv_sock >= 0)
		return;

	/* Clear eventally current peers (it is NOT currently possible) */
	peers_list.CloseAll();

	memset(&localhost, 0, sizeof localhost);

	/* Create sock */
	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(serv_sock < 0)
		throw CantOpenSock();

	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof reuse_addr);

	int flags = fcntl(serv_sock, F_GETFL);
	int res = fcntl(serv_sock, F_SETFL, flags | O_NONBLOCK);
	assert(res != -1);

	localhost.sin_family = AF_INET;
	localhost.sin_addr.s_addr = inet_addr(bindaddr);
	localhost.sin_port = htons(port);

	if(bind(serv_sock, (struct sockaddr *) &localhost, sizeof localhost) < 0)
	{
		close(serv_sock);
		throw CantListen(port);
	}

	listen(serv_sock, 5);
	FD_SET(serv_sock, &global_read_set);

	environment.listening_port.Set(port);
}

void NetworkBase::StartNetwork(MyConfig* conf)
{
	assert(ssl == NULL);

	ConfigSection* section = conf->GetSection("ssl");
	if(section && (!section->GetItem("enabled") || section->GetItem("enabled")->Boolean() == true))
	{
		log[W_INFO] << "Using a SSL wrapper";
		ssl = new SslSsl(section->GetItem("cert")->String(),
			section->GetItem("key")->String(),
			section->GetItem("ca")->String());
	}
	else
	{
		log[W_INFO] << "Not using a SSL wrapper";
		ssl = new SslNoSsl();
	}

	/* Listen a TCP port */
	section = conf->GetSection("listen");

	Listen(static_cast<uint16_t>(section->GetItem("port")->Integer()),
		section->GetItem("bind")->String().c_str());

	/* Connect to other servers */
	std::vector<ConfigSection*> sections = conf->GetSectionClones("connection");
	for(std::vector<ConfigSection*>::iterator it = sections.begin(); it != sections.end(); ++it)
	{
		pf_addr addr = MakeAddr((*it)->GetItem("host")->String(),static_cast<uint16_t>((*it)->GetItem("port")->Integer()));
		scheduler_queue.Queue(new JobNewConnection(addr));
	}
}

void NetworkBase::AddDisconnected(const pf_addr& addr)
{
	BlockLockMutex lock(this);
	if(find(disconnected_list.begin(),
		disconnected_list.end(), addr) == disconnected_list.end())
	{
		disconnected_list.push_back(addr);
		scheduler_queue.Queue(new JobNewConnection(addr));
	}
}

void NetworkBase::DelDisconnected(const pf_addr& addr)
{
	BlockLockMutex lock(this);
	log[W_INFO] << "Removed disconnected: " << addr;
	disconnected_list.remove(addr);

	/* Remove connection from queue. */
	scheduler_queue.Lock();
	for(SchedulerQueue::iterator it = scheduler_queue.begin();
		it != scheduler_queue.end();
		++it)
	{
		JobNewConnection* job = dynamic_cast<JobNewConnection*>(*it);
		if(job && job->IsMe(addr))
		{
			log[W_DEBUG] << "-> removed a job";
			scheduler_queue.Cancel(job);
		}
	}
	scheduler_queue.Unlock();
}
