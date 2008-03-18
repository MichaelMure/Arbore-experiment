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
#include "connection.h"
#include "log.h"
#include "libconfig.h"
#include "network_base.h"
#include "tools.h"
#include "job.h"
#include "scheduler.h"

NetworkBase::NetworkBase() throw(CantRunThread, Ssl::CantReadCertificate)
			: running(true),
			serv_sock(-1),
			highsock(-1),
			listening_port(0),
			my_id(0)
{
	FD_ZERO(&global_write_set);
	FD_ZERO(&global_read_set);
	ssl = new SslSsl();

	int r = pthread_create(&thread_id, NULL, StartThread, (void*)this);
	if (r != 0)
		throw CantRunThread();
}

NetworkBase::~NetworkBase()
{
	running = false;
	pthread_join(thread_id, NULL);
	delete ssl;
}

void *NetworkBase::StartThread(void* arg)
{
	NetworkBase* thr = static_cast<NetworkBase*>(arg);
	thr->Main();
	pthread_exit(NULL);
	return NULL;
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
		fd2peer[p->GetFd()] = p;

		if(p->GetFd() > highsock)
			highsock = p->GetFd();

	}

	peer_list.push_back(p);

	return p;
}

void NetworkBase::RemovePeer(Peer* peer)
{
	if(fd2peer[peer->GetFd()] == peer)
		fd2peer.erase(peer->GetFd());
	for(PeerList::iterator it = peer_list.begin(); it != peer_list.end();)
		if(*it == peer)
			it = peer_list.erase(it);
	else
		++it;

	if(FD_ISSET(peer->GetFd(), &global_read_set)) FD_CLR(peer->GetFd(), &global_read_set);
	if(FD_ISSET(peer->GetFd(), &global_write_set)) FD_CLR(peer->GetFd(), &global_write_set);

	log[W_CONNEC] << "<- Removed a peer: " << peer->GetFd() << " (" << peer->GetID() << ")";

	delete peer;
}

void NetworkBase::Main()
{
	while(running)
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
				break;
			}
		}
		else if(events > 0)		  /* events = 0 means that there isn't any event (but timeout expired) */
		{
			for(int i = 0;i <= highsock;++i)
			{
				if(FD_ISSET(i, &tmp_write_set))
				{
					PeerMap::iterator cl = fd2peer.find(i);

					FD_CLR(i, &global_write_set);

					if(cl != fd2peer.end())
						cl->second->Flush();
					else
						log[W_WARNING] << "tmp_write_set(" << i << ") and " << i << " doesn't exist!";
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

						Connection *peer_conn = ssl->Accept(newfd);

						addr.id = peer_conn->GetCertificateID();
						AddPeer(new Peer(addr, peer_conn));

						DelDisconnected(addr);
					}
				}
				else
				{
					try
					{
						while(fd2peer[i]->Receive()) ;
					}
					catch(Connection::RecvError &e)
					{
						log[W_WARNING] << "recv() error: " /*<< strerror(errno)*/;
						if(errno != EINTR)
						{
							Peer* p = fd2peer[i];
							if(p->GetAddr().port > 0)
								AddDisconnected(p->GetAddr());
							RemovePeer(p);
						}
					}
					catch(Packet::Malformated &e)
					{
						log[W_ERR] << "Received malformed message!";
						/* TODO: Take a decision. For now, we wait&see */
					}
					catch(Peer::MustDisconnect &e)
					{
						log[W_WARNING] << "Must disconnected";
						RemovePeer(fd2peer[i]);
					}
				}
			}
		}

		scheduler.HandleJobs();
	}

	/* We leave */
	CloseAll();
}

void NetworkBase::CloseAll()
{
	for(PeerList::iterator it = peer_list.begin(); it != peer_list.end(); ++it)
		delete *it;

	peer_list.clear();

	if(serv_sock >= 0)
	{
		close(serv_sock);
		serv_sock = -1;
	}

	FD_ZERO(&global_write_set);
	FD_ZERO(&global_read_set);
	highsock = -1;
}

Peer* NetworkBase::Connect(const std::string& hostname, uint16_t port)
{
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

	Connection* conn = ssl->Connect(sock);

	addr.id = conn->GetCertificateID();
	Peer* p = AddPeer(new Peer(addr, conn));

	p->SetFlag(Peer::SERVER);
	DelDisconnected(addr);
	return p;
}

void NetworkBase::Listen(uint16_t port, const char* bindaddr) throw(CantOpenSock, CantListen)
{
	unsigned int reuse_addr = 1;
	struct sockaddr_in localhost;		  /* bind info structure */

	/* Already listening */
	if(serv_sock >= 0)
		return;

	/* Clear eventally current peers (it is NOT currently possible) */
	fd2peer.clear();
	for(PeerList::iterator it = peer_list.begin(); it != peer_list.end(); ++it)
		delete *it;
	peer_list.clear();

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

Peer* NetworkBase::Start(MyConfig* conf)
{
	/* Listen a TCP port */
	ConfigSection* section = conf->GetSection("listen");

	Listen(static_cast<uint16_t>(section->GetItem("port")->Integer()),
		section->GetItem("bind")->String().c_str());

	/* Connect to other servers */
	Peer* peer = 0;
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
			/* We doesn't care about error and what it is, we'll so try to connect
			 * to next server.
			 * If there isn't any server, we wait alone.
			 */
			log[W_INFO] << "Unable to connect to " << (*it)->GetItem("host")->String() << ":" << (*it)->GetItem("port")->Integer();
		}
	}
	return peer;
}

id_t NetworkBase::CreateID()
{
	// TODO: optimize me
	id_t new_id = 0;
	while(!new_id)
	{
		new_id = rand();
		for(PeerMap::iterator peer = fd2peer.begin();
			peer != fd2peer.end();
			++peer)
		{
			if(new_id == peer->second->GetID())
			{
				new_id = 0;
				break;
			}
		}
	}

	return new_id;
}
