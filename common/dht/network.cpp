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
#include "network.h"
#include "host.h"
#include "message.h"
#include "log.h"
#include "semaphore.h"
#include "jrb.h"
#include "jval.h"
#include "dtime.h"

extern int errno;
#define SEND_SIZE NETWORK_PACK_SIZE


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
	int ret;
	struct sockaddr_in saddr;
	int one;

	/* create socket */
	sd = socket (AF_INET, SOCK_DGRAM, 0);
	if (sd < 0)
	{
		log[W_ERR] << "network: socket: " << strerror (errno);
		assert (1 == 0);
	}
	if (setsockopt (sd, SOL_SOCKET, SO_REUSEADDR, (void *) &one, sizeof (one)) == -1)
	{
		log[W_ERR] << "network: setsockopt: " << strerror (errno);
		close (sd);
		assert (1 == 0);
	}

	/* attach socket to #port#. */
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl (INADDR_ANY);
	saddr.sin_port = htons ((short) port);
	if (bind (sd, (struct sockaddr *) &saddr, sizeof (saddr)) < 0)
	{
		log[W_ERR] << "network: bind: " << strerror (errno);
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
int NetworkGlobal::network_send (ChimeraHost * host, char *data, int size,
		  unsigned long ack)
{
	struct sockaddr_in to;
	int ret, retval;
	unsigned long seq, seqnumbackup, ntype;
	int sizebackup;
	char s[SEND_SIZE];
	void *semaphore;
	JRB node;
	JRB priqueue;
	double start;
	ChimeraState *chstate = (ChimeraState *) state;
	NetworkGlobal *ng;

	ng = (NetworkGlobal *) chstate->network;

	if (size > NETWORK_PACK_SIZE)
	{
		log[W_ERR] << "network_send: cannot send data over " << NETWORK_PACK_SIZE << " bytes!";
		return (0);
	}
	if (ack != 1 && ack != 2)
	{
		log[W_ERR] << "network_send: FAILED, unexpected message ack property " << ack << " !";
		return (0);
	}
	memset (&to, 0, sizeof (to));
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = host->address;
	to.sin_port = htons ((short) host->port);

	AckEntry *ackentry = get_new_ackentry();
	sizebackup = size;
	/* get sequence number and initialize acknowledgement indicator*/
	pthread_mutex_lock (&(ng->lock));
	node = jrb_insert_int (ng->waiting, ng->seqend, new_jval_v(ackentry));
	seqnumbackup = ng->seqend;
	seq = htonl (ng->seqend);
	ng->seqend++;		/* needs to be fixed to modplus */
	pthread_mutex_unlock (&(ng->lock));

	/* create network header */
	ntype = htonl (ack);
	memcpy (s, &ntype, sizeof (unsigned long));
	memcpy (s + sizeof (unsigned long), &seq, sizeof (unsigned long));
	memcpy (s + (2 * sizeof (unsigned long)), data, size);
	size += (2 * sizeof (unsigned long));

	/* send data */
	seq = ntohl (seq);
	log[W_DEBUG] << "network_send: sending message seq=" << seq << " ack=" << ack
		                                     " to " << *host << " data:" << data;
	start = dtime ();

	ret = sendto (ng->sock, s, size, 0, (struct sockaddr *) &to, sizeof (to));
	log[W_DEBUG] << "network_send: sent message: " << s;

	if (ret < 0)
	{
		log[W_ERR] << "network_send: sendto: " << strerror (errno);
		host_update_stat (host, 0);
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

		pthread_mutex_lock (&(ng->lock));
		priqueue = jrb_insert_dbl (ng->retransmit, (start+RETRANSMIT_INTERVAL), new_jval_v (pqrecord));
		pthread_mutex_unlock (&(ng->lock));

	}

	return (1);
}

/**
 ** Resends a message to host
 */
int network_resend (void *state, ChimeraHost *host, char *data, int size, int ack, unsigned long seqnum, double *transtime)
{
	struct sockaddr_in to;
	int ret, retval;
	char s[SEND_SIZE];
	double start;
    ChimeraState *chstate = (ChimeraState *) state;
    NetworkGlobal *ng = (NetworkGlobal *) chstate->network;

	memset (&to, 0, sizeof (to));
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = host->address;
	to.sin_port = htons ((short) host->port);

	unsigned long seq = htonl (seqnum);

	/* create network header */
	unsigned long ntype = htonl (ack);
	memcpy (s, &ntype, sizeof (unsigned long));
	memcpy (s + sizeof (unsigned long), &seq, sizeof (unsigned long));
	memcpy (s + (2 * sizeof (unsigned long)), data, size);
	size += (2 * sizeof (unsigned long));

	/* send data */
	seq = ntohl (seq);
	if (LOGS)
		log_message (chstate->log, LOG_NETWORKDEBUG,
				"network_resend: resending message seq=%d ack=%d to %s:%d  data:%s\n",
				seq, ack, host->name, host->port, data);

	*transtime = dtime();
	ret = sendto (ng->sock, s, size, 0, (struct sockaddr *) &to, sizeof (to));
	if (LOGS)
		log_message (chstate->log, LOG_NETWORKDEBUG,
				"network_resend: sent message: %s\n", s);
	if (ret < 0)
	{
		if (LOGS)
			log_message (chstate->log, LOG_ERROR,
					"network_resend: sendto: %s\n", strerror (errno));
		host_update_stat (host, 0);
		return (0);
	}

// kpkp
	//fprintf(stderr, "network_resend: resending message seq=%d ack=%d to %s:%d  datalen:%d\n", seq, ack, host->name, host->port, strlen(data));
	return (1);
}
