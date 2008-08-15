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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include "message.h"
#include "message_handlers.h"
#include "scheduler_queue.h"
#include "jobs/job.h"
#include "jval.h"

/* message on the wire is encoded in the following way:
** [ type ] [ size ] [ key ] [ data ] */

#define HEADER_SIZE (sizeof(uint32_t) + sizeof(uint32_t) + KEY_SIZE/BASE_B + 1)

typedef struct
{
    int ack;
    IMessageHandler* handler;
    int priority;
    int reply;
} MessageProperty;

/**
 ** message_create:
 ** creates the message to the destination #dest# the message format would be like:
 **  [ type ] [ size ] [ key ] [ data ]. It return the created message structure.
 **
 */
Message::Message(Key dest, uint32_t type, uint32_t size, char *payload)
{
    this->dest = dest;
    this->type = type;
    this->size = size;
    this->payload = (char*)malloc (sizeof (char) * size);
    memcpy (this->payload, payload, size);
}

/**
 ** message_free:
 ** free the message and the payload
 */
Message::~Message()
{
	free (payload);
}

/**
 ** message_init: chstate, port
 ** Initialize messaging subsystem on port and returns the MessageGlobal * which
 ** contains global state of message subsystem.
 ** message_init also initiate the network subsystem
 */
MessageGlobal::MessageGlobal ()
{
	this->handlers = make_jrb ();
}

MessageGlobal::~MessageGlobal()
{
	jrb_free_tree(this->handlers);
}

/**
 ** message_received:
 ** is called by network_activate and will be passed received data and size from socket
 **
 */
void MessageGlobal::message_received (char *data, size_t size)
{
	uint32_t msgtype;
	uint32_t msgsize;
	//  unsigned long msgdest;
	Key msgdest;
	Message *message;

	/* message format on the wire
	* [ type ] [ size ] [ key ] [ data ]
	*/

	/* decode message and create Message structure */
	memcpy (&msgtype, data, sizeof (unsigned long));
	msgtype = ntohl (msgtype);
	memcpy (&msgsize, data + sizeof (unsigned long), sizeof (unsigned long));
	msgsize = ntohl (msgsize);

	msgdest = (data + (2 * sizeof (unsigned long)));

	message = new Message(msgdest, (int) msgtype, (int) msgsize,
			data + HEADER_SIZE);

	JRB node;
	MessageProperty *msgprop;
	BlockLockMutex(this);

	/* find message handler */
	node = jrb_find_int (handlers, message->GetType());
	if (node == NULL)
	{
		pf_log[W_ERR] << "received unrecognized message type " << message->GetType();
		delete message;
		return;
	}
	msgprop = static_cast<MessageProperty*>(node->val.v);
	if (!msgprop->handler)
	{
		pf_log[W_ERR] << "no message handler has registered for type " << message->GetType();
		delete message;
		return;
	}

	scheduler_queue.Queue(new JobHandleMessage(msgprop->handler, message));
}

/**
 ** registers the handler function #func# with the message type #type#,
 ** it also defines the acknowledgment requirement for this type
 */
void MessageGlobal::message_handler (int type, IMessageHandler* func, int ack)
{
	JRB node;
	MessageProperty *msgprop = (MessageProperty *) malloc (sizeof (MessageProperty));

	msgprop->handler = func;
	msgprop->ack = ack;

	/* add message handler function into the set of all handlers */
	BlockLockMutex(this);
	node = jrb_find_int (handlers, type);

	/* don't allow duplicates */
	if (node != NULL)
	{
		pf_log[W_WARNING] << "message handler already registered with " << type;
		return;
	}
	jrb_insert_int (handlers, type, new_jval_v (msgprop));
}

/**
 ** message_send:
 ** send the message to destination #host# the retry arg indicates to the network
 ** layer if this message should be ackd or not
 */
int MessageGlobal::message_send (ChimeraHost * host, Message * message,
		  bool retry)
{
	char *data;
	uint32_t size, type;
	int ret = 0;
	JRB node;
	MessageProperty *msgprop;

	/* message format on the wire
	 * [ type ] [ size ] [ key ] [ data ]
	 */

	if (host == NULL)
		return (0);

	size = uint32_t(HEADER_SIZE) + message->GetSize();
	data = (char *) malloc (sizeof (char) * size);

	/* encode the message */
	type = htonl (message->GetType());
	memcpy (data, &type, sizeof (uint32_t));
	size = htonl (message->GetSize());
	memcpy (data + sizeof (uint32_t), &size, sizeof (uint32_t));
	memcpy (data + (2 * sizeof (uint32_t)),
			message->GetDestString(),
			strlen (message->GetDestString()));
	memcpy (data + HEADER_SIZE, message->GetPayload(), message->GetSize());
	size = uint32_t(HEADER_SIZE) + message->GetSize();	/*reset due to htonl */

	/* get the message properties */
	Lock();
	node = jrb_find_int (handlers, message->GetType());
	if (node == NULL)
	{
		pf_log[W_ERR] << "fail to send unrecognized message type " << message->GetType();
		Unlock();
		return 0;
	}
	msgprop = static_cast<MessageProperty*>(node->val.v);
	Unlock();

	/* WTF??? -romain */
	/* send the message */
	if (!retry)
		ret = network_send (host, data, size, msgprop->ack);
	else
		ret = network_send (host, data, size, msgprop->ack);
	return (ret);
}
