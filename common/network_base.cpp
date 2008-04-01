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

NetworkBase::NetworkBase()
			: serv_sock(-1),
			highsock(-1),
			listening_port(0),
			ssl(NULL)
{
	FD_ZERO(&global_write_set);
	FD_ZERO(&global_read_set);

	srand(time(NULL));
	Start();				  // Start the network's loop
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

void NetworkBase::HavePacketToSend(const Peer* peer)
{
	FD_SET(peer->GetFd(), &global_write_set);
}

Peer* NetworkBase::AddPeer(Peer* p)
{
	log[W_CONNEC] << "-> Added a new peer: " << p->GetFd();

	if(p->GetFd() >= 0)
	{
		FD_SET(p->GetFd(), &global_read_set);

		if(p->GetFd() > highsock)
			highsock = p->GetFd();
	}

	peers_list.Add(p);

	return p;
}

void NetworkBase::RemovePeer(int fd, bool try_reconnect)
{
	Peer* p = peers_list.Remove(fd);

	OnRemovePeer(p);

	if(p && p->GetAddr().port > 0 && try_reconnect)
		AddDisconnected(p->GetAddr());

	delete p;
	if(FD_ISSET(fd, &global_read_set)) FD_CLR(fd, &global_read_set);
	if(FD_ISSET(fd, &global_write_set)) FD_CLR(fd, &global_write_set);
}

void NetworkBase::Loop()
{
	/* We wait only 1000ms because when an other thread wants to send a
	 * message to someone, we change global_write_set, and it needs to call
	 * an other time select().
	 * If timeout was something like one minute, we may wait one minute before
	 * message was sent...
	 */
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 1000;

	fd_set tmp_read_set = global_read_set;
	fd_set tmp_write_set = global_write_set;
	int events;

	if((events = select(highsock + 1, &tmp_read_set, &tmp_write_set, NULL, &timeout)) < 0)
	{
		if(errno != EINTR)
		{
			log[W_ERR] << "Error in select() (" << errno << ": " << strerror(errno) << ")";
			return;
		}
	}
	else if(events > 0)			  /* events = 0 means that there isn't any event (but timeout expired) */
	{
		for(int i = 0;i <= highsock;++i)
		{
			if(FD_ISSET(i, &tmp_write_set))
			{
				FD_CLR(i, &global_write_set);

				try
				{
					peers_list.PeerFlush(i);
				}
				catch(Connection::WriteError &e)
				{
					log[W_WARNING] << "send() error: " << e.GetString();
					RemovePeer(i);
				}
			}
			if(!FD_ISSET(i, &tmp_read_set)) continue;

			if(i == serv_sock)
			{
				struct sockaddr_in newcon;
				unsigned int addrlen = sizeof newcon;
				int newfd = accept(serv_sock, (struct sockaddr *) &newcon, &addrlen);

				if(newfd)
				{
					/* none_addr is initialized to NULL */
					static pf_addr none_addr;
					pf_addr addr = none_addr;
					addr.ip[3] = newcon.sin_addr.s_addr;

					Connection *peer_conn;
					try
					{
						peer_conn = ssl->Accept(newfd);
					}
					catch(SslSsl::SslHandshakeFailed &e)
					{
						log [W_WARNING] << "SSL handshake failure: " << e.GetString();
						continue;
					}

					AddPeer(new Peer(addr, peer_conn));

					DelDisconnected(addr);
				}
			}
			else
			{
				try
				{
					while(peers_list.PeerReceive(i)) ;
				}
				catch(Connection::RecvError &e)
				{
					log[W_WARNING] << "recv() error: " << e.GetString();
					RemovePeer(i);
				}
				catch(Packet::Malformated &e)
				{
					log[W_ERR] << "Received malformed message!";
					RemovePeer(i);
				}
				// TODO:Rename me into something like BanPeer as we won't reconnect to him
				catch(Peer::MustDisconnect &e)
				{
					log[W_WARNING] << "Must disconnected";
					RemovePeer(i, false);
				}
			}
		}
	}
}

void NetworkBase::OnStop()
{
	/* We leave */
	CloseAll();
}

void NetworkBase::CloseAll()
{
	peers_list.CloseAll();

	if(serv_sock >= 0)
	{
		close(serv_sock);
		serv_sock = -1;
	}

	FD_ZERO(&global_write_set);
	FD_ZERO(&global_read_set);
	highsock = -1;

	delete ssl;
	ssl = 0;
}

Peer* NetworkBase::Connect(const std::string& hostname, uint16_t port)
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

	return Connect(addr);
}

Peer* NetworkBase::Connect(pf_addr addr)
{
	assert(ssl);

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

	Peer* p = AddPeer(new Peer(addr, conn));

	p->SetFlag(Peer::SERVER);
	DelDisconnected(addr);
	return p;
}

void NetworkBase::Listen(uint16_t port, const char* bindaddr) throw(CantOpenSock, CantListen)
{
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

	highsock = 0;

	listen(serv_sock, 5);
	if(serv_sock > highsock)
		highsock = serv_sock;

	FD_SET(serv_sock, &global_read_set);
	FD_SET(serv_sock, &global_write_set);

	listening_port = port;
}

Peer* NetworkBase::StartNetwork(MyConfig* conf)
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
	Peer* peer = NULL;
	std::vector<ConfigSection*> sections = conf->GetSectionClones("connection");
	for(std::vector<ConfigSection*>::iterator it = sections.begin(); it != sections.end(); ++it)
	{
		try
		{
			peer = Connect((*it)->GetItem("host")->String(),
				static_cast<uint16_t>((*it)->GetItem("port")->Integer()));

			/* If Connect() doesn't raise any exception, we are connected and we leave loop */
			break;
		}
		catch(...)
		{
			/* We don't care about error and what it is, we'll so try to connect
			 * to next server.
			 * If there isn't any server, we wait alone.
			 */
			log[W_INFO] << "Unable to connect to " << (*it)->GetItem("host")->String() << ":" << (*it)->GetItem("port")->Integer();
		}
	}
	return peer;
}

void NetworkBase::AddDisconnected(const pf_addr& addr)
{
	if(find(disconnected_list.begin(),
		disconnected_list.end(), addr) == disconnected_list.end())
	{
		disconnected_list.push_back(addr);
		scheduler_queue.Queue(new JobNewConnection(addr));
	}
}

void NetworkBase::DelDisconnected(const pf_addr& addr)
{
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
