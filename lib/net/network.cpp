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

#include <algorithm>
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <list>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <time.h>

#include <net/packet.h>
#include <net/host.h>
#include <scheduler/job.h>
#include <scheduler/scheduler_queue.h>
#include <util/time.h>
#include <util/pf_config.h>
#include <util/pf_log.h>
#include <util/pf_thread.h>
#include <util/tools.h>

#include "job_handle_packet.h"
#include "job_resend_packet.h"
#include "network.h"

Network::Network(Chimera *chimera)
	: Mutex(RECURSIVE_MUTEX),
	highsock(-1),
	hosts_list(64), /* TODO: magic number, change it. */
	seqend(0),
	chimera_(chimera)
{
	FD_ZERO(&socks_fd_set);
}

Network::~Network()
{
	CloseAll();
}

int Network::Listen(uint16_t port, const char* bind_addr)
{
	BlockLockMutex lock(this);
	struct sockaddr_in saddr;
	int one = 0;

	/* create an UDP socket */
	int serv_sock = socket (AF_INET, SOCK_DGRAM, 0);
	if (serv_sock < 0)
		throw CantOpenSock();

	if (setsockopt (serv_sock, SOL_SOCKET, SO_REUSEADDR, (void *) &one, sizeof (one)) == -1)
	{
		close (serv_sock);
		throw CantOpenSock();
	}

	/* attach the socket to the address and port */
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr(bind_addr);
	saddr.sin_port = htons (port);
	if (bind (serv_sock, (struct sockaddr *) &saddr, sizeof (saddr)) < 0)
	{
		close (serv_sock);
		throw CantListen(port);
	}

	socks.insert(serv_sock);
	FD_SET(serv_sock, &socks_fd_set);

	/* update the highest socket number */
	if(serv_sock > highsock)
		highsock = serv_sock;

	pf_log[W_INFO] << "Listening on " << bind_addr << ":" << port;

	//TODO environment.listening_port.Set(port);

	return serv_sock;
}

void Network::Loop()
{
	fd_set tmp_read_set;
	int events;

	/* no socket open right now */
	if(highsock < 0)
		return;

	Lock();
	tmp_read_set = socks_fd_set;
	Unlock();

	/* see if at least one socket is ready to be read without blocking */
	if((events = select(highsock + 1, &tmp_read_set, NULL, NULL, NULL)) < 0)
	{
		if(errno != EINTR)
		{
			pf_log[W_ERR] << "Error in select(): #" << errno << " " << strerror(errno);
			return;
		}
	}
	else if(events > 0) /* events = 0 means that there isn't any event (but timeout expired) */
	{
		BlockLockMutex lock(this);
		for(SockSet::iterator it = socks.begin(); it != socks.end(); ++it)
		{
			int sock = *it;
			/* if this socket is not ready, skip it */
			if(!FD_ISSET(sock, &tmp_read_set))
				continue;

			static char data[PACKET_MAX_SIZE];
			struct sockaddr_in from;
			ssize_t size;
			socklen_t socklen = sizeof(from);

			size = recvfrom(sock, data, sizeof(data), 0,
					(struct sockaddr *) &from, &socklen);

			if(size < 0)
			{
				pf_log[W_ERR] << "Error in recvfrom(): #" << errno << " " << strerror(errno);
				return;
			}

			if((size_t)size < Packet::GetHeaderSize())
			{
				pf_log[W_ERR] << "Received a packet too light "
				              << "(size: " << size << " < " << Packet::GetHeaderSize() << ")";
				/* No recover solution here, packet is lost. */
				return;
			}
			try
			{
				Packet pckt(data, size);
				pf_addr address(from.sin_addr.s_addr, ntohs(from.sin_port));
				Host sender = hosts_list.GetHost(address);

				if(!sender.GetKey())
					sender.SetKey(pckt.GetSrc());

				pf_log[W_PARSE] << "R(" << sender << ") - " << pckt;

				if(pckt.HasFlag(Packet::ACK))
				{
					/* We got an ACK message, so we remove the ResendPacketJob, update
					 * the latency information and mark this host as up.
					 */
					ResendPacketJob* job;
					std::vector<ResendPacketJob*>::iterator it;
					for(it = resend_list.begin();
					    it != resend_list.end() && (*it)->GetPacket().GetSeqNum() != pckt.GetSeqNum();
					    ++it)
						;

					if(it == resend_list.end())
					{
						pf_log[W_WARNING] << "Received an ACK for an unknown sent ack request";
						return;
					}

					sender.UpdateStat(1);

					job = *it;
					job->GetDestHost().UpdateLatency(time::dtime() - job->GetTransmitTime());

					resend_list.erase(it);
					scheduler_queue.Cancel(job);
					return;
				}
				if(pckt.HasFlag(Packet::REQUESTACK))
				{
					/* It request an ACK, so we send it. */
					Packet ack(pckt);
					ack.SetSrc(pckt.GetDst());
					ack.SetDst(pckt.GetSrc());
					ack.SetFlags(Packet::ACK);
					Send(sock, sender, ack);
				}

				scheduler_queue.Queue(new HandlePacketJob(chimera_, sender, pckt));
			}
			catch(Packet::Malformated &e)
			{
				pf_log[W_ERR] << "Received malformed message!";
				return;
			}
		}
	}
}

void Network::OnStop()
{
	/* We leave */
	CloseAll();
}

void Network::CloseAll()
{
	BlockLockMutex lock(this);

	for(SockSet::iterator it = socks.begin(); it != socks.end(); ++it)
	{
		shutdown(*it, SHUT_RDWR);
		close(*it);
	}
	socks.clear();
}

HostsList* Network::GetHostsList()
{
	return &hosts_list;
}

void Network::StartNetwork(MyConfig* conf)
{
#if 0
	/* Listen a TCP port */
	ConfigSection* section = conf->GetSection("listen");

	Listen(static_cast<uint16_t>(section->GetItem("port")->Integer()),
		section->GetItem("bind")->String().c_str());

	/* Connect to other servers */
	std::vector<ConfigSection*> sections = conf->GetSectionClones("connection");
	for(std::vector<ConfigSection*>::iterator it = sections.begin(); it != sections.end(); ++it)
	{
		pf_addr addr = MakeAddr((*it)->GetItem("host")->String(),static_cast<uint16_t>((*it)->GetItem("port")->Integer()));
		scheduler_queue.Queue(new JobNewConnection(addr));
	}
#endif
}

bool Network::Send(int sock, Host host, Packet pckt)
{
	struct sockaddr_in to;
	ssize_t ret;
	double start;

	memset (&to, 0, sizeof (to));
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = host.GetAddr().ip[3]; /* TODO: ipv6! */
	to.sin_port = htons(host.GetAddr().port);

	start = time::dtime ();

	BlockLockMutex lock(this);

	/* Check if this sock is opened. */
	if(socks.find(sock) == socks.end())
		return false;

	if(!pckt.GetSeqNum())
		pckt.SetSeqNum(this->seqend++);


	pf_log[W_PARSE] << "S(" << host << ") - " << pckt;

	char* s = pckt.DumpBuffer();
	ret = sendto (sock, s, pckt.GetSize(), 0, (struct sockaddr *) &to, sizeof (to));
	free(s);

	if (ret < 0)
	{
		pf_log[W_ERR] << "network_send: sendto: " << strerror (errno);
		host.UpdateStat(0);
		return false;
	}

	if (pckt.HasFlag(Packet::REQUESTACK))
	{
		std::vector<ResendPacketJob*>::iterator it;
		for(it = resend_list.begin();
		    it != resend_list.end() && (*it)->GetPacket().GetSeqNum() != pckt.GetSeqNum();
		    ++it)
			;

		if(it == resend_list.end())
		{
			/* There isn't any already existing job to retransmit this packet. */
			ResendPacketJob* job = new ResendPacketJob(this, sock, host, pckt, start);
			resend_list.push_back(job);
			scheduler_queue.Queue(job);
		}
	}

	return true;
}

