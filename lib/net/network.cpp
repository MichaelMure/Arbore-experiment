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

#include "pf_log.h"
#include "pf_config.h"
#include "network.h"
#include "tools.h"
#include "scheduler_queue.h"
#include "dtime.h"
#include "pf_thread.h"
#include "jobs/job.h"
//#include "environment.h"
//#include "content_list.h"
#include "net/packet.h"
#include "net/host.h"

/** Resend a packet after a waited time.
 *
 * This job resend frequently a packet until
 * it receives an ACK message.
 */
class ResendPacketJob : public Job
{
	int sock;
	Host desthost;
	Packet packet;
	unsigned int retry;
	double transmittime;
	Network* network;

	bool Start()
	{
		if(!network->Send(sock, desthost, packet))
			return false;
		retry++;
		if(retry < Network::MAX_RETRY)
			return true;

		desthost.UpdateStat(0);
		return false;
	}

public:

	ResendPacketJob(Network* _network, int _sock, const Host& _desthost, const Packet& _packet, double transmit_time)
		: Job(dtime(), REPEAT_PERIODIC, Network::RETRANSMIT_INTERVAL),
		  sock(_sock),
		  desthost(_desthost),
		  packet(_packet),
		  retry(0),
		  transmittime(transmit_time),
		  network(_network)
	{}

	const Packet& GetPacket() const { return packet; }
	double GetTransmitTime() const { return transmittime; }
	Host GetDestHost() const { return desthost; }
};

/** Call the packet handler.
 *
 * Because we don't want to monopolize the Network thread,
 * this job is used to ask a scheduler thread to call the
 * packet handler.
 */
class HandlePacketJob : public Job
{
	Host sender;
	Packet pckt;
	PacketTypeList& pckt_type_list;

	bool Start()
	{
		pckt.Handle(pckt_type_list, sender);
		return false;
	}

public:
	HandlePacketJob(PacketTypeList& _pckt_type_list, const Host& _sender, const Packet& _pckt)
		: Job(0.0, REPEAT_NONE),
		  sender(_sender),
		  pckt(_pckt),
		  pckt_type_list(_pckt_type_list)
	{}
};

/*******************
 *    NETWORK      *
 *******************/

Network::Network()
	: Mutex(RECURSIVE_MUTEX),
	highsock(-1),
	hosts_list(64), /* TODO: magic number, change it. */
	seqend(0)
{
	FD_ZERO(&socks_fd_set);
}

Network::~Network()
{
	CloseAll();
}

int Network::Listen(PacketTypeList* packet_type_list, uint16_t port, const char* bind_addr) throw(CantOpenSock, CantListen)
{
	BlockLockMutex lock(this);
	struct sockaddr_in saddr;
	int one = 0;

	/* create socket */
	int serv_sock = socket (AF_INET, SOCK_DGRAM, 0);
	if (serv_sock < 0)
		throw CantOpenSock();

	if (setsockopt (serv_sock, SOL_SOCKET, SO_REUSEADDR, (void *) &one, sizeof (one)) == -1)
	{
		close (serv_sock);
		throw CantOpenSock();
	}

	/* attach socket to #port#. */
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr(bind_addr);
	saddr.sin_port = htons (port);
	if (bind (serv_sock, (struct sockaddr *) &saddr, sizeof (saddr)) < 0)
	{
		close (serv_sock);
		throw CantListen(port);
	}

	socks[serv_sock] = packet_type_list;
	FD_SET(serv_sock, &socks_fd_set);
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

	if(highsock < 0)
		return;

	Lock();
	tmp_read_set = socks_fd_set;
	Unlock();

	/* TODO: this is useless to use select() here, because now there is only one fd (server sock) */
	if((events = select(highsock + 1, &tmp_read_set, NULL, NULL, NULL)) < 0)
	{
		if(errno != EINTR)
		{
			pf_log[W_ERR] << "Error in select(): #" << errno << " " << strerror(errno);
			return;
		}
	}
	else if(events > 0)			  /* events = 0 means that there isn't any event (but timeout expired) */
	{
		BlockLockMutex lock(this);
		for(SockMap::iterator it = socks.begin(); it != socks.end(); ++it)
		{
			if(!FD_ISSET(it->first, &tmp_read_set))
				continue;

			int sock = it->first;
			PacketTypeList* packet_type_list = it->second;
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
				return;
			}
			try
			{
				Packet pckt(packet_type_list, data, size);
				pf_addr address(from.sin_addr.s_addr, ntohs(from.sin_port));
				Host sender = hosts_list.GetHost(address);

				if(sender.GetKey() == (uint32_t)0)
					sender.SetKey(pckt.GetSrc());

				pf_log[W_PARSE] << "R(" << sender << ") - " << pckt.GetPacketInfo();

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

					job = *it;
					sender.UpdateStat(1);
					double latency = dtime() - job->GetTransmitTime();
					if(latency > 0)
					{
						if(job->GetDestHost().GetLatency() == 0.0)
							job->GetDestHost().SetLatency(latency);
						else
							job->GetDestHost().SetLatency((0.9 * job->GetDestHost().GetLatency())
										    + (0.1 * latency));
					}

					resend_list.erase(it);
					scheduler_queue.Cancel(job);
					return;
				}
				if(pckt.HasFlag(Packet::REQUESTACK))
				{
					/* It request an ACK, so we send it. */
					Packet ack(pckt.GetPacketType(), pckt.GetSrc(), pckt.GetDst());
					ack.SetFlag(Packet::ACK);
					ack.SetSeqNum(pckt.GetSeqNum());
					Send(sock, sender, ack);
				}

				scheduler_queue.Queue(new HandlePacketJob(*packet_type_list, sender, pckt));
			}
			catch(Packet/*::Malformated*/ &e)
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

	for(SockMap::iterator it = socks.begin(); it != socks.end(); ++it)
	{
		shutdown(it->first, SHUT_RDWR);
		close(it->first);
	}
	socks.clear();
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

	start = dtime ();

	BlockLockMutex lock(this);

	/* Check if this sock is opened. */
	if(socks.find(sock) == socks.end())
		return false;

	if(!pckt.GetSeqNum())
		pckt.SetSeqNum(this->seqend++);


	pf_log[W_PARSE] << "S(" << host << ") - " << pckt.GetPacketInfo();

	const char* s = pckt.DumpBuffer();
	ret = sendto (sock, s, pckt.GetSize(), 0, (struct sockaddr *) &to, sizeof (to));
	delete [] s;

	if (ret < 0)
	{
		pf_log[W_ERR] << "network_send: sendto: " << strerror (errno);
		host.UpdateStat(0);
		return false;
	}

	if (pckt.HasFlag(Packet::REQUESTACK))
	{
		ResendPacketJob* job = new ResendPacketJob(this, sock, host, pckt, start);
		resend_list.push_back(job);
		scheduler_queue.Queue(job);
	}

	return true;
}

