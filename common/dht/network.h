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

/*
** $Id: network.h,v 1.14 2007/04/04 00:04:49 krishnap Exp $
**
** Matthew Allen
** description:
*/

#ifndef _CHIMERA_NETWORK_H_
#define _CHIMERA_NETWORK_H_

#include "host.h"
#include "jrb.h"

/**
 ** NETWORK_PACK_SIZE is the maximum packet size that will be handled by chimera network layer
 */
#define NETWORK_PACK_SIZE 65536
/**
 ** TIMEOUT is the number of seconds to wait for receiving ack from the destination, if you want
 ** the sender to wait forever put 0 for TIMEOUT.
 */
#define TIMEOUT 1.0

typedef struct PriqueueEntry{
	ChimeraHost *desthost; // who should this message be sent to?
	char *data; // what to send?
	size_t datasize; // how big is it?
	int retry; // number of retries
	unsigned long seqnum; // seqnum to identify the packet to be retransmitted
	double transmittime; // this is the time the packet is transmitted (or retransmitted)
}PQEntry;

typedef struct AcknowledgEntry{
	int acked;
	double acktime; // the time when the packet is acked
}AckEntry;

class NetworkGlobal : protected Mutex
{
	int sock;
	JRB waiting;
	unsigned int seqstart, seqend;
	JRB retransmit;

public:

	/** network_init:
	 ** initiates the networking layer by creating socket and bind it to #port#
	 */
	NetworkGlobal(int port);

	/**
	 ** network_send: host, data, size
	 ** Sends a message to host, updating the measurement info.
	 ** type are 1 or 2, 1 indicates that the data should be acknowledged by the
	 ** receiver, and 2 indicates that no ack is necessary.
	 */
	int network_send (ChimeraHost * host, char *data, size_t size, unsigned int type);

};

#endif /* _CHIMERA_NETWORK_H_ */
