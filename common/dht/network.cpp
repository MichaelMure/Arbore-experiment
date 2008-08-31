/*
 * Copyright(C) 2008 Romain Bignon
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
 * This file contains some code from the Chimera's Distributed Hash Table,
 * written by CURRENT Lab, UCSB.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netdb.h>
#include <pthread.h>
#include <cassert>
#include "network.h"
#include "hosts_list.h"
#include "message.h"
#include "jrb.h"
#include "jval.h"
#include "dtime.h"

extern int errno;

// allocate a new pointer and return it
PQEntry* get_new_pqentry()
{
	PQEntry* entry = (PQEntry *)malloc(sizeof(PQEntry));
	entry->desthost = NULL;
	entry->data = NULL;
	entry->datasize = 0;
	entry->retry = 0;
	entry->seqnum = 0;
	entry->transmittime = 0.0;

	return entry;
}

AckEntry* get_new_ackentry()
{
	AckEntry *entry = (AckEntry *)(malloc(sizeof(AckEntry)));
	entry->acked = 0;
	entry->acktime = 0.0;

	return entry;
}



/** network_init:
 ** initiates the networking layer by creating socket and bind it to #port#
 */

NetworkGlobal::NetworkGlobal(int port)
{
	int sd;
	struct sockaddr_in saddr;
	int one;

	/* create socket */
	sd = socket (AF_INET, SOCK_DGRAM, 0);
	if (sd < 0)
	{
		pf_log[W_ERR] << "network: socket: " << strerror (errno);
		assert (1 == 0);
	}
	if (setsockopt (sd, SOL_SOCKET, SO_REUSEADDR, (void *) &one, sizeof (one)) == -1)
	{
		pf_log[W_ERR] << "network: setsockopt: " << strerror (errno);
		close (sd);
		assert (1 == 0);
	}

	/* attach socket to #port#. */
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl (INADDR_ANY);
	saddr.sin_port = htons ((short) port);
	if (bind (sd, (struct sockaddr *) &saddr, sizeof (saddr)) < 0)
	{
		pf_log[W_ERR] << "network: bind: " << strerror (errno);
		close (sd);
		assert(1 == 0);
	}

	this->sock = sd;
	this->waiting = make_jrb();
	this->seqstart = 0;
	this->seqend = 0;
	this->retransmit = make_jrb();
}

/**
 ** network_send: host, data, size
 ** Sends a message to host, updating the measurement info.
 */
int NetworkGlobal::network_send (Host * host, char *data, size_t size, unsigned int ack)
{
	struct sockaddr_in to;
	ssize_t ret;
	unsigned int seq, seqnumbackup, ntype;
	size_t sizebackup;
	char s[SEND_SIZE];
	JRB node;
	JRB priqueue;
	double start;

	if (size > NETWORK_PACK_SIZE)
	{
		pf_log[W_ERR] << "network_send: cannot send data over " << NETWORK_PACK_SIZE << " bytes!";
		return (0);
	}
	if (ack != 1 && ack != 2)
	{
		pf_log[W_ERR] << "network_send: FAILED, unexpected message ack property " << ack << " !";
		return (0);
	}
	memset (&to, 0, sizeof (to));
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = host->GetAddress();
	to.sin_port = htons ((short) host->GetPort());

	AckEntry *ackentry = get_new_ackentry();
	sizebackup = size;
	/* get sequence number and initialize acknowledgement indicator*/
	Lock();
	node = jrb_insert_int (this->waiting, this->seqend, new_jval_v(ackentry));
	seqnumbackup = this->seqend;
	seq = htonl (this->seqend);
	this->seqend++;		/* needs to be fixed to modplus */
	Unlock();

	/* create network header */
	ntype = htonl (ack);
	memcpy (s, &ntype, sizeof (uint32_t));
	memcpy (s + sizeof (uint32_t), &seq, sizeof (uint32_t));
	memcpy (s + (2 * sizeof (uint32_t)), data, size);
	size += (2 * sizeof (uint32_t));

	/* send data */
	seq = ntohl (seq);
	pf_log[W_DEBUG] << "network_send: sending message seq=" << seq << " ack=" << ack
		                                      << " to " << *host << " data:" << data;
	start = dtime ();

	ret = sendto (this->sock, s, size, 0, (struct sockaddr *) &to, sizeof (to));
	pf_log[W_DEBUG] << "network_send: sent message: " << s;

	if (ret < 0)
	{
		pf_log[W_ERR] << "network_send: sendto: " << strerror (errno);
		host->UpdateStat(0);
		return (0);
	}

	if (ack == 1)
	{
		// insert a record into the priority queue with the following information:
		// key: starttime + next retransmit time
		// other info: destination host, seq num, data, data size
		PQEntry *pqrecord = get_new_pqentry();
		pqrecord->desthost = host;
		pqrecord->data = data;
		pqrecord->datasize = sizebackup;
		pqrecord->retry = 0;
		pqrecord->seqnum = seqnumbackup;
		pqrecord->transmittime = start;

		BlockLockMutex p(this);
		priqueue = jrb_insert_dbl (this->retransmit, (start+RETRANSMIT_INTERVAL), new_jval_v (pqrecord));

	}

	return (1);
}

/**
 ** Resends a message to host
 */
int network_resend (Host *host, char *data, size_t size, int ack, unsigned int seqnum, double *transtime)
{
	struct sockaddr_in to;
	int ret;
	char s[SEND_SIZE];

	memset (&to, 0, sizeof (to));
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = host->GetAddress();
	to.sin_port = htons ((short) host->GetPort());

	uint32_t seq = htonl (seqnum);

	/* create network header */
	uint32_t ntype = htonl (ack);
	memcpy (s, &ntype, sizeof (uint32_t));
	memcpy (s + sizeof (uint32_t), &seq, sizeof (uint32_t));
	memcpy (s + (2 * sizeof (uint32_t)), data, size);
	size += (2 * sizeof (uint32_t));

	/* send data */
	seq = ntohl (seq);
	pf_log[W_DEBUG] << "network_resend: resending message seq=" << seq << " ack=" << ack
		                                          << " to " << *host << " data:" << data;

	*transtime = dtime();
	ret = sendto (ng->sock, s, size, 0, (struct sockaddr *) &to, sizeof (to));
	pf_log[W_DEBUG] << "network_resend: sent message: " << s;
	if (ret < 0)
	{
		pf_log[W_ERR] << "network_send: sendto: " << strerror (errno);
		host->UpdateStat(0);
		return (0);
	}

// kpkp
	//fprintf(stderr, "network_resend: resending message seq=%d ack=%d to %s:%d  datalen:%d\n", seq, ack, host->name, host->port, strlen(data));
	return (1);
}
