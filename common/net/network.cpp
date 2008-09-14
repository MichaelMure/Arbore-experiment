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
#include "job_new_connection.h"
#include "scheduler_queue.h"
#include "pf_thread.h"
#include "jobs/job.h"
#include "environment.h"
//#include "content_list.h"
#include "net/packet.h"
#include "dht/host.h"

class ResendPacketJob : public Job
{
	Host desthost;
	Packet packet;
	unsigned int retry;
	double transmittime;
	Network* network;

	bool Start()
	{
		if(!network->Send(desthost, packet))
			return false;
		retry++;
		if(retry < Network::MAX_RETRY)
			return true;

		desthost.UpdateStat(0);
		return false;
	}

public:

	ResendPacketJob(Network* _network, const Host& _desthost, const Packet& _packet, double transmit_time)
		: Job(dtime(), REPEAT_PERIODIC, Network::RETRANSMIT_INTERVAL),
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

Network::Network()
	: Mutex(RECURSIVE_MUTEX),
	serv_sock(-1),
	seqend(0)
{
}

Network::~Network()
{
	if(serv_sock != -1)
	{
		shutdown(serv_sock, SHUT_RDWR);
		close(serv_sock);
	}
}

void Network::Listen(uint16_t port, const char* bind_addr) throw(CantOpenSock, CantListen)
{
	BlockLockMutex lock(this);
	struct sockaddr_in saddr;
	int one;

	/* create socket */
	serv_sock = socket (AF_INET, SOCK_DGRAM, 0);
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

	environment.listening_port.Set(port);
}

void Network::Loop()
{
	fd_set tmp_read_set;
	int events;

	Lock();
	FD_ZERO(&tmp_read_set);
	FD_SET(serv_sock, &tmp_read_set);
	Unlock();

	/* TODO: this is useless to use select() here, because now there is only one fd (server sock) */
	if((events = select(serv_sock + 1, &tmp_read_set, NULL, NULL, NULL)) < 0)
	{
		if(errno != EINTR)
		{
			pf_log[W_ERR] << "Error in select(): #" << errno << " " << strerror(errno);
			return;
		}
	}
	else if(events > 0)			  /* events = 0 means that there isn't any event (but timeout expired) */
	{
		static char data[PACKET_MAX_SIZE];
		struct sockaddr_in from;
		ssize_t size;
		socklen_t socklen = sizeof(from);

		size = recvfrom(serv_sock, data, sizeof(data), 0,
		                (struct sockaddr *) &from, &socklen);

		if(size < 0)
		{
			pf_log[W_ERR] << "Error in recvfrom(): #" << errno << " " << strerror(errno);
			return;
		}
		if(size < Packet::GetHeaderSize())
		{
			pf_log[W_ERR] << "Received a packet too light (size: " << size << " < " << Packet::GetHeaderSize() << ")";
			return;
		}
		try
		{
			Packet pckt(data);

			if(size != pckt.GetSize())
			{
				pf_log[W_ERR] << "There isn't exacly the same length of data that header says."
				              << "(" << size << " vs " << pckt.GetSize() << ")";
				return;
			}

			if(pckt.HasFlag(Packet::ACK))
			{
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
				sender.UpdateState(1);
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
				Packet ack(pckt.GetPacketType(), pckt.GetSrc(), pckt.GetDst());
				ack.SetFlag(Packet::ACK);
				ack.SetSeqNum(pckt.GetSeqNum());
				Send(sender, ack);
			}
		}
		catch(Packet::Malformated &e)
		{
			pf_log[W_ERR] << "Received malformed message!";
			return;
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

	if(serv_sock >= 0)
	{
		shutdown(serv_sock, SHUT_RDWR);
		close(serv_sock);
		serv_sock = -1;
	}
}

void Network::StartNetwork(MyConfig* conf)
{
	/* Listen a TCP port */
	ConfigSection* section = conf->GetSection("listen");

	Listen(static_cast<uint16_t>(section->GetItem("port")->Integer()),
		section->GetItem("bind")->String().c_str());

#if 0
	/* Connect to other servers */
	std::vector<ConfigSection*> sections = conf->GetSectionClones("connection");
	for(std::vector<ConfigSection*>::iterator it = sections.begin(); it != sections.end(); ++it)
	{
		pf_addr addr = MakeAddr((*it)->GetItem("host")->String(),static_cast<uint16_t>((*it)->GetItem("port")->Integer()));
		scheduler_queue.Queue(new JobNewConnection(addr));
	}
#endif
}

bool Network::Send(Host host, Packet pckt)
{
	struct sockaddr_in to;
	ssize_t ret;
	double start;

	memset (&to, 0, sizeof (to));
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = host.GetAddr().ip[3]; /* TODO: ipv6! */
	to.sin_port = htons (host.GetAddr().port);

	start = dtime ();

	BlockLockMutex lock(this);

	if(!pckt.GetSeqNum())
		pckt.SetSeqNum(this->seqend++);

	const char* s = pckt.DumpBuffer();
	ret = sendto (this->serv_sock, s, pckt.GetSize(), 0, (struct sockaddr *) &to, sizeof (to));

	if (ret < 0)
	{
		pf_log[W_ERR] << "network_send: sendto: " << strerror (errno);
		host.UpdateStat(0);
		return false;
	}

	if (pckt.HasFlag(Packet::REQUESTACK))
	{
		ResendPacketJob* job = new ResendPacketJob(this, host, pckt, start);
		resend_list.push_back(job);
		scheduler_queue.Queue(job);
	}

	return true;
}

