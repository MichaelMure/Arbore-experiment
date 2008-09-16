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

#include "net/packet_type_list.h"
#include "net/host.h"

class Network;
class ChimeraRouting;
class Packet;

class ChimeraDHT : public PacketTypeList
{
	Network* network;
	ChimeraRouting* routing;
	Host me;
	int fd;

	void InitMessages();

public:

	ChimeraDHT(Network* network, uint16_t port, Key my_key);

	Host GetMe() const { return me; }

	Network* GetNetwork() const { return network; }

	bool Send(const Host& destination, const Packet& pckt);

	void Route(const Packet& pckt);

	bool Ping(const Host& dest);
};

#endif /* CHIMERA_H */

#ifndef CHIMERA_H
#define CHIMERA_H

#include "key.h"
#include "net/host.h"
#include "net/hosts_list.h"
#include "mutex.h"
#include "chimera_routing.h"
#include <pthread.h>

typedef void (*chimera_forward_upcall_t) (const Key **, Message **, Host **);
typedef void (*chimera_deliver_upcall_t) (const Key *, Message *);
typedef void (*chimera_update_upcall_t) (const Key *, Host *, int);

class ChimeraGlobal : protected Mutex
{
    Host me;
    Host bootstrap;
    void *join;			/* semaphore */
    chimera_forward_upcall_t forward;
    chimera_deliver_upcall_t deliver;
    chimera_update_upcall_t update;
};

class ChimeraDHT
{
	NetworkGlobal *network;
	NetworkActivate* network_activate;
	NetworkRetransmit* network_retransmit;
	MessageGlobal *message;
	void *route;
	HostsList *host;
	ChimeraRouting* routingStructure;

	//JRB bootstrapMsgStore;	/* for future security enhancement */
	pthread_mutex_t bootstrapMutex;	/* for future security enhancement */
	void *certificateStore;	/* for future security enhancement */
	pthread_mutex_t certificateMutex;	/* for future security enhancement */

	size_t encode_hosts(char* s, size_t size, Host** host) const;

	Host** decode_hosts(const char* s);

	void send_rowinfo(Message* message);

	void join_complete(Host* host);

	void *check_leafset (void *chstate);

	int check_leafset_init();

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
	void Join (Host * bootstrap);

	/**
	 * route:
	 * Send a message msg through the system to key. hint is currently
	 * ignored, but it will one day be the next hop
	*/
	void Route (const Key * key, Message * msg);

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
	void Send (Key key, int type, size_t len,
			   char *data);

	/**
	 ** ping:
	 ** sends a ping message to the host. the message is acknowledged in network layer
	 */
	int Ping (Host * host);

};

#endif /* _CHIMERA_H_ */
