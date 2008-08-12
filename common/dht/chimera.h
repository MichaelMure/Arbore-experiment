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

#ifndef CHIMERA_H
#define CHIMERA_H

#include "host.h"
#include "key.h"
#include "log.h"
#include <pthread.h>
#include "semaphore.h"

class Message;
class HostGlobal;
class MessageGlobal;

typedef void (*chimera_forward_upcall_t) (const Key **, Message **, ChimeraHost **);
typedef void (*chimera_deliver_upcall_t) (const Key *, Message *);
typedef void (*chimera_update_upcall_t) (const Key *, ChimeraHost *, int);

typedef struct
{
    ChimeraHost *me;
    ChimeraHost *bootstrap;
    void *join;			/* semaphore */
    pthread_mutex_t lock;
    chimera_forward_upcall_t forward;
    chimera_deliver_upcall_t deliver;
    chimera_update_upcall_t update;
    Sema globalSeqNum;		/* for future security enhancement */
} ChimeraGlobal;

class ChimeraDHT
{
	void *network;
	MessageGlobal *message;
	void *route;
	HostGlobal *host;

	JRB bootstrapMsgStore;	/* for future security enhancement */
	pthread_mutex_t bootstrapMutex;	/* for future security enhancement */
	void *certificateStore;	/* for future security enhancement */
	pthread_mutex_t certificateMutex;	/* for future security enhancement */

	size_t encode_hosts(char* s, size_t size, ChimeraHost** host) const;

	ChimeraHost** decode_hosts(const char* s);

	void send_rowinfo(Message* message);
	void route_message (Message * message);

	void join_complete(ChimeraHost* host);

	static void *check_leafset (void *chstate);

	int check_leafset_init();

	void join_denied(Message* m);
	void join_acknowledged(Message* m);

public:

	ChimeraGlobal *chimera;
	/**
	 ** chimera_init: port
	 **  Initialize Chimera on port port and returns the ChimeraState * which
	 ** contains global state of different chimera modules.
	 */
	ChimeraDHT(int port);

	/**
	 ** join:
	 ** Join the network that the bootstrap host is a part of
	 */
	void Join (ChimeraHost * bootstrap);

	/**
	 * route:
	 * Send a message msg through the system to key. hint is currently
	 * ignored, but it will one day be the next hop
	*/
	void Route (const Key * key, Message * msg,
			    ChimeraHost * hint);

	/**
	 ** forward:
	 ** Set the chimera forward upcall to func. This handler will be called every
	 ** time a message is routed to a key through the current node. The host argument
	 ** is upsupported, but will allow the programmer to choose the next hop
	 */
	void Forward (chimera_forward_upcall_t func);

	/**
	 ** deliver:
	 ** Set the chimera deliver upcall to func. This handler will be called every
	 ** time a message is delivered to the current node
	 */
	void Deliver (chimera_deliver_upcall_t func);

	/** update:
	 ** Set the chimera update upcall to func. This handler will be called every
	 ** time a host leaves or joins your neighbor set. The final integer is a 1 if
	 ** the host joins and a 0 if the host leaves
	*/
	void Update (chimera_update_upcall_t func);

	/** setkey:
	 ** Manually sets the key for the current node
	 */
	void SetKey (Key key);

	/** register:
	 ** register an integer message type to be routed by the chimera routing layer
	 ** ack is the argument that defines wether this message type should be acked or not
	 ** ack ==1 means message will be acknowledged, ack=2 means no acknowledge is necessary
	 ** for this type of message.
	 */
	void Register (int type, int ack);

	/** send:
	 ** Route a message of type to key containing size bytes of data. This will
	 ** send data through the Chimera system and deliver it to the host closest to the
	 ** key
	 */
	void Send (Key key, int type, int len,
			   char *data);

	/**
	 ** ping:
	 ** sends a ping message to the host. the message is acknowledged in network layer
	 */
	int Ping (ChimeraHost * host);

};

#endif /* _CHIMERA_H_ */
