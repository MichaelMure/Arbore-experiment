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
** $Id: message.h,v 1.20 2007/04/04 00:04:49 krishnap Exp $
**
** Matthew Allen
** description:
*/

#ifndef _CHIMERA_MESSAGE_H_
#define _CHIMERA_MESSAGE_H_

#include "key.h"
#include "host.h"
#include "jrb.h"
#include "include.h"

#define DEFAULT_SEQNUM 0
#define RETRANSMIT_THREAD_SLEEP 1
#define RETRANSMIT_INTERVAL 1
#define MAX_RETRY 3

typedef struct
{
    Key dest;
    int type;			/* message type */
    size_t size;
    char *payload;
    Key source;			/* for future security enhancement */
    unsigned long seqNum;	/* for future security enhancement */
} Message;


typedef void (*messagehandler_t) (ChimeraDHT *, Message *);

/**
 ** message_init: chstate, port
 ** Initialize messaging subsystem on port and returns the MessageGlobal * which
 ** contains global state of message subsystem.
 ** message_init also initiate the network subsystem
 */
void *message_init (void *chstate, int port);

/**
 ** message_received:
 ** is called by network_activate and will be passed received data and size from socket
 **
 */
void message_received (void *chstate, char *data, size_t size);

/**
 ** registers the handler function #func# with the message type #type#,
 ** it also defines the acknowledgment requirement for this type
 */
void message_handler (void *chstate, int type, messagehandler_t func,
		      int ack);

/**
 ** message_send:
 ** sendt the message to destination #host# the retry arg indicates to the network
 ** layer if this message should be ackd or not
 */
int message_send (void *chstate, ChimeraHost * host, Message * message,
		  Bool retry);

/**
 ** message_create:
 ** creates the message to the destination #dest# the message format would be like:
 **  [ type ] [ size ] [ key ] [ data ]. It return the created message structure.
 **
 */
Message *message_create (Key dest, int type, size_t size, char *payload);

void message_free(Message* msg);

#endif /* _CHIMERA_MESSAGE_H_ */
