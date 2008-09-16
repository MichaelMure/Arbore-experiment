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

#include <unistd.h>
#include <netdb.h>

#include "chimera.h"
#include "net/network.h"
#include "chimera_routing.h"
#include "key.h"
#include "chimera_messages.h"
#include "check_leafset_job.h"
#include "scheduler_queue.h"

ChimeraDHT::ChimeraDHT(Network* _network, uint16_t port, Key my_key)
	: network(_network),
	routing(NULL)

{
	char name[256];
	struct hostent* he;
	Key::Init();

	if(gethostname(name, sizeof(name)) != 0)
	{
		return;
	}
	if((he = gethostbyname(name)) == NULL)
	{
		return;
	}
	me = network->GetHostsList()->GetHost(he->h_name, port);
	me.SetKey(my_key);

	routing = new ChimeraRouting(network->GetHostsList(), me);

	RegisterType(ChimeraJoinType);
	RegisterType(ChimeraJoinAckType);
	RegisterType(ChimeraUpdateType);
	RegisterType(ChimeraPiggyType);
	RegisterType(ChimeraJoinNAckType);
	RegisterType(ChimeraPingType);

	fd = network->Listen(this, port, "0.0.0.0");

	scheduler_queue.Queue(new CheckLeafsetJob(this, routing));
}

bool ChimeraDHT::Send(const Host& dest, const Packet& pckt)
{
	return network->Send(fd, dest, pckt);
}

bool ChimeraDHT::Ping(const Host& dest)
{
	Packet pckt(ChimeraPingType, me.GetKey(), dest.GetKey());
	pckt.SetArg(NET_PING_ME, me.GetAddr());

	return Send(dest, pckt);
}

#if 1 == 0

#include <errno.h>
#include <vector>

#include "chimera.h"
#include "pf_log.h"
#include "net/network.h"
#include "dtime.h"

#define CHIMERA_JOIN       1
#define CHIMERA_JOIN_ACK   2
#define CHIMERA_UPDATE     3
#define CHIMERA_PIGGY      4
#define CHIMERA_JOIN_NACK  5
#define CHIMERA_PING       6

/**
 *interval that members of leafset are checked to see if they are alive or not
 */
#define LEAFSET_CHECK_PERIOD 20	/* seconds */

/** GRACEPERIOD is the time that has to be elapsed before a node
 ** can be accepted to the network again after last send to it failed
 */
#define GRACEPERIOD  30		/* seconds */


size_t ChimeraDHT::encode_hosts (char *s, size_t size, Host ** host) const
{

    size_t i, j;

    s[0] = 0;
    for (i = 0; host[i] != NULL; i++)
	{
	    j = strlen (s);
	    host[i]->Encode(s + j, size - j);

	    pf_log[W_DEBUG] << "ENCODED " << i << " = " << s + j;
	    strcat (s, "\n");	/* add a spacer */
	}

    return (strlen (s) + 1);

}

Host** ChimeraDHT::decode_hosts(const char *hostname)
{

    Host **host;
    size_t hostnum;
    size_t i, j, k;
    char* s = strdup(hostname);

    for (i = 0, hostnum = 0; i < strlen (s); i++)
	{
	    if (s[i] == '\n')
		hostnum++;
	}

    host = (Host **) malloc (sizeof (Host *) * (hostnum + 1));
    //memset(host, 0, (sizeof(ChimeraHost *) * (hostnum + 1)));

    /* gets the number of hosts in the lists and goes through them 1 by 1 */
    for (i = 0, j = 0, k = 0; i < hostnum; i++)
	{
	    while (s[k] != '\n')
		{
		    k++;
		}
	    s[k] = 0;
	    /* once you've found the seperater, decode the host and send it an update */
	    host[i] = this->host->DecodeHost(this, s + j);
	    k++;
	    j = k;
	}
    host[i] = NULL;

    free(s);

    return (host);

}

/**
 ** chimera_send_rowinfo:
 ** sends matching row of its table to the joining node while
 ** forwarding the message toward the root of the joining node
 **
 */
void ChimeraDHT::send_rowinfo (Message * message)
{
    size_t size;
    char s[NETWORK_PACK_SIZE];
    Host* host;
    Message *msg;

    host = this->host->DecodeHost(this, message->GetPayload());

    /* send one row of our routing table back to joiner #host# */
    std::vector<Host> rowinfo = this->routingStructure->rowLookup(host->GetKey());
    size = encode_hosts (s, NETWORK_PACK_SIZE, rowinfo);
    msg = new Message (host->GetKey(), CHIMERA_PIGGY, size, s);
    if (!this->message->message_send (host, msg, true))
    {
	pf_log[W_ERR] << "Sending row information to node! " << host->GetName() << ":" << host->GetPort() << " failed";
    }
    delete msg;
}

/** chimera_join_complete:
** internal function that is called at the destination of a JOIN message. This
** call encodes the leaf set of the current host and sends it to the joiner.
**
*/
void ChimeraDHT::join_complete(Host * host)
{
    char s[NETWORK_PACK_SIZE];
    Message *m;
    ChimeraGlobal *chglob = (ChimeraGlobal *) (this->chimera);

    /* copy myself into the reply */
    chglob->me->Encode(s, NETWORK_PACK_SIZE);
    strcat (s, "\n");		/* add a spacer */

    /* check to see if the node has just left the network or not */
    if ((dtime () - host->GetFailureTime()) < GRACEPERIOD)
	{
	      pf_log[W_WARNING] << "JOIN request from node: " << host->GetName() << ":" << host->GetPort()
		                << " rejected ,elapsed time since failure = " << dtime() - host->GetFailureTime() << " sec";

	    m = new Message (host->GetKey(), CHIMERA_JOIN_NACK, strlen (s) + 1,
				s);
	    if (!this->message->message_send (host, m, true))
		pf_log[W_WARNING] << "message_send NACK failed!";
	    delete m;
	    return;
	}

    /* copy my leaf set into the reply */
    std::vector<Host> leafset = this->routingStructure->getLeafset();
    //leafset = route_neighbors (this, LEAFSET_SIZE);
    encode_hosts (s + strlen (s),
			 NETWORK_PACK_SIZE - strlen (s), leafset);
    m = new Message (host->GetKey(), CHIMERA_JOIN_ACK, strlen (s) + 1, s);
    if (!this->message->message_send (host, m, true))
	      pf_log[W_WARNING] << "message_send ACK failed!";

    delete m;
}

/**
 ** chimera_check_leafset: runs as a separate thread.
 ** it should send a PING message to each member of the leafset frequently and
 ** sends the leafset to other members of its leafset periodically.
 ** pinging frequecy is LEAFSET_CHECK_PERIOD.
 **
 */

void *ChimeraDHT::check_leafset (void *chstate)
{
	char s[NETWORK_PACK_SIZE];
	Message *m;
	size_t i, count = 0;
	ChimeraDHT *state = (ChimeraDHT *) chstate;
	ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;


	while (1)
	{

		std::vector<Host> leafset = this->routingStructure->getLeafset();
		//TODO (eld) use an iterator instead ?
		for (i = 0; i < leafset.size(); i++)
		{
			if (state->Ping(leafset[i]) == 1)
			{
				leafset[i].SetFailureTime(dtime ());
				pf_log[W_WARNING] << "message send to host: " << leafset[i].GetName() << ":" << leafset[i].GetPort()
			                          << " failed at time: " << leafset[i].GetFailureTime() << "!";
				if (leafset[i].GetSuccessAvg() < BAD_LINK)
				{
					printf ("Deleting %s:%d \n",
					    leafset[i].GetName(),
					    leafset[i].GetPort());
					this->routingStructure->remove(leafset[i]);
				}
			}
			state->host->Release(leafset[i]);
		}
		std::vector<Host> table = this->routingStructure->getRoutingTable();
		for (i = 0; i < table.size(); i++)
		{
			if (state->Ping(table[i]) == 1)
			{
				table[i]->SetFailureTime(dtime ());
				pf_log[W_WARNING] << "message send to host: " << table[i].GetName() << ":" << table[i].GetPort()
			                          << " failed at time: " << table[i].GetFailureTime() << "!";
				if (table[i].GetSuccessAvg() < BAD_LINK)
				{
					this->routingStructure->remove(table[i]);
					//route_update (state, table[i], 0);
				}
			}
			state->host->Release(table[i]);
		}

		/* send leafset exchange data every  3 times that pings the leafset */
		if (count == 2)
		{
			count = 0;
			std::vector<Host> leafset = this->routingStructure->getLeafset();
			chglob->me->Encode(s, NETWORK_PACK_SIZE);
			strcat (s, "\n");	/* add a spacer */
			state->encode_hosts (s + strlen (s), NETWORK_PACK_SIZE - strlen (s),
			                     leafset);
			//TODO (eld) use iterator instead ?
			for (i = 0; i < leafset.size(); i++)
			{
				m = new Message (leafset[i].GetKey(),
						CHIMERA_PIGGY, strlen (s) + 1,
						s);
				if (!state->message->message_send (leafset[i], m, true))
				{
					pf_log[W_WARNING] << "sending leafset update to "
						          << leafset[i].GetName() << ":"
						          << leafset[i].GetPort() << " failed!";
					if (leafset[i].GetSuccessAvg() < BAD_LINK)
					{
						this->routingStructure->remove(leafset[i]);
					}
				}
				delete m;
				state->host->Release(leafset[i]);
			}
		}
		else
		{
			count++;
		}
		sleep (LEAFSET_CHECK_PERIOD);
	}
}

/**
 ** chimera_check_leafset_init:
 ** initiates a separate thread that constantly checks to see if the leafset members
 ** and table entries of the node are alive or not.
 **
 */
int ChimeraDHT::check_leafset_init ()
{

	pthread_attr_t attr;
	pthread_t tid;

	if (pthread_attr_init (&attr) != 0)
	{
		pf_log[W_ERR] << "(CHIMERA)pthread_attr_init: " << strerror (errno);
		return (0);
	}
	if (pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM) != 0)
	{
		pf_log[W_ERR] << "(CHIMERA)pthread_attr_setscope: " << strerror (errno);
		goto out;
	}
	if (pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED) != 0)
	{
		pf_log[W_ERR] << "(CHIMERA)pthread_attr_setdetachstate: " << strerror (errno);
		goto out;
	}

	if (pthread_create (&tid, &attr, check_leafset, (void *) this) != 0)
	{
		pf_log[W_ERR] << "(CHIMERA)pthread_create: " << strerror (errno);
		goto out;
	}

	return (1);

out:
	pthread_attr_destroy (&attr);
	return (0);
}

/**
 ** chimera_join_denied
 ** internal function that is called when the sender of a JOIN message receives
 ** the JOIN_NACK message type which is join denial from the current key root
 ** in the network.
 */

void ChimeraDHT::join_denied (Message * message)
{
    Host *host = this->host->DecodeHost(this, message->GetPayload());

    pf_log[W_WARNING] << "JOIN request rejected from " << host->GetName() << ":" << host->GetPort();
    sleep (GRACEPERIOD);
    pf_log[W_WARNING] << "Re-sending JOIN message to " << chimera->bootstrap->GetName()
	              << ":" << chimera->bootstrap->GetPort();

    this->Join (this->chimera->bootstrap);
}


/**
 ** chimera_route:
 ** routes a message one step closer to its destination key. Delivers
 ** the message to its destination if it is the current host through the
 ** deliver upcall, otherwise it makes the route upcall
 */

void ChimeraDHT::Route(const Key * key, Message * message)
{
    Host nextDest;
    //ChimeraHost **piggy;
    Message *real;
    /* unused? -romain
     * unsigned long size;
     * char s[NETWORK_PACK_SIZE];
     */
    ChimeraGlobal *chglob = (ChimeraGlobal*) this->chimera;

    real = message;

    nextDest = this->routingStructure->routeLookup(*key);
    //tmp = route_lookup (this, *key, 1, 0);

    /* this is to avoid sending JOIN request to the node that
     ** its information is already in the routing table  ****/

    if (nextDest.GetKey() == *key)
    {
	this->routingStructure->remove(nextDest);
	nextDest = this->routingStructure->routeLookup(*key);
    }
    /* if I am the only host or the closest host is me, deliver the message */
    if (nextDest == chglob->me)
	{
	    if (chglob->deliver != NULL)
		{
		    chglob->deliver (key, real);
		}
	    if (message->GetType() == CHIMERA_JOIN)
		{
		    host = this->host->DecodeHost(this, message->GetPayload());
		    join_complete (host);
		}
	}

    /* otherwise, route it */
    else
	{
	    if (chglob->forward != NULL)
		{
		    chglob->forward (&key, &real, &nextDest);
		}
	    message = real;

	    while (!this->message->message_send (nextDest, message, true))
		{

		    host->SetFailureTime(dtime ());
		    pf_log[W_WARNING] << "message send to host: " << nextDest.GetName() << ":" << nextDest.GetPort()
			              << "at time: " << nextDest.GetFailureTime() << " failed!";

		    /* remove the faulty node from the routing table */
		    if (host->GetSuccessAvg() < BAD_LINK)
		    {
			this->routingStructure->remove(nextDest);
		    }
		    nextDest = routingStructure->routeLookup(*key);
		    pf_log[W_WARNING] << "rerouting through " << nextDest.GetName() << ":" << nextDest.GetPort();
		}

	    /* in each hop in the way to the key root nodes
	       send their routing info to the joining node  */

	    if (message->GetType() == CHIMERA_JOIN)
	    {
		send_rowinfo ( message);
	    }
	}

}

/**
 * chimera_join_acknowledge:
 * called when the current host is joining the network and has just revieced
 * its leaf set. This function sends an update message to all nodes in its
 * new leaf set to announce its arrival.
 */

void ChimeraDHT::join_acknowledged (Message * message)
{
	std::vector<Host> host;
	Message *m;
	char s[256];
	int i;
	ChimeraGlobal *chglob = (ChimeraGlobal *) this->chimera;

	chglob->me->Encode(s, 256);
	host = decode_hosts(message->GetPayload());

	/* announce my arrival to the nodes in my leafset */
	for (i = 0; i<host.size(); i++)
	{
		this->routingStructure->add(host[i]);
		//route_update (this, host[i], 1);
// 		m = new Message (host[i].GetKey(), CHIMERA_UPDATE, strlen (s) + 1,
				s);
		if (!this->message->message_send (host[i], m, true))
		{
			pf_log[W_WARNING] << "chimera_join_acknowledge: failed to update "
					  << host[i].GetName() << ":" << host[i].GetPort();
		}
		delete m;
	}

	/* announce my arival to the nodes in my routing table */
	host = this->routingStructure->getRoutingTable();
	for (i = 0; i<host.size(); i++)
	{
		m = new Message (host[i].GetKey(), CHIMERA_UPDATE, strlen (s) + 1, s);
		if (!this->message->message_send (host[i], m, true))
		{
			pf_log[W_WARNING] << "chimera_join_acknowledge: failed to update"
					  << host[i].GetName() << ":" << host[i].GetPort();
		}
		delete m;
	}

	/* signal the chimera_join function, which is blocked awaying completion */
	sema_v (chglob->join);

	/* initialize the thread for leafset check and exchange */
	if (!(check_leafset_init ()))
	{
		pf_log[W_ERR] << "chimera_check_leafset_init FAILED";
		return;
	}
}

/**
 ** chimera_message:
 ** routes the message through the chimera_route toward the destination
 **
 */
void ChimeraDHT::route_message (Message * message)
{
    Route (message->GetDest(), message, NULL);
}

void ChimeraDHT::Register(int type, int ack)
{
	if (type < 10)
	{
		pf_log[W_ERR] << "chimera_register: message integer types < 10 are reserved for system";
		exit (1);
	}

	if (ack != 1 && ack != 2)
	{
		pf_log[W_ERR] << "chimera_register: message property ack must be either 1 or 2 unrecognized ack value " << ack;
		exit (1);
	}

	this->message->message_handler (type, &ChimeraDHT::route_message, ack);
}

void ChimeraDHT::update_message (Message * message)
{
    Host *host;

    host = this->host->DecodeHost(this, message->GetPayload());
    this->routingStructure->add(*host);
    //route_update (this, host, 1);

}

/**
 ** chimera_piggy_message:
 ** message handler for message type PIGGY ;) this used to be a piggy backing function
 ** This function is respopnsible to add the piggy backing node information that is sent along with
 ** other ctrl messages or separately to the routing table. the PIGGY message type is a separate
 ** message type.
 */

void ChimeraDHT::piggy_message (Message * message)
{
    Host **piggy;
    int i;

    piggy = decode_hosts(message->GetPayload());

    for (i = 0; piggy[i] != NULL; i++)
	{

	    if ((dtime () - piggy[i]->GetFailureTime()) > GRACEPERIOD)
		{
		    this->routingStructure->add(*piggy[i]);
		    //route_update (this, piggy[i], 1);
		}
		else
			pf_log[W_WARNING] << "refused to add:" << get_key_string (&piggy[i]->GetKey())
			                  << " to routing table";
	}
    free (piggy);
}



extern void route_keyupdate ();

void chimera_setkey (ChimeraDHT * state, Key key)
{

    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    key_assign (&(chglob->me->GetKey()), key);
    this->routingStructure->KeyUpdate(chglob->me);
    //route_keyupdate (state->route, chglob->me);

}

/**
 ** chimera_ping:
 ** sends a PING message to the host. The message is acknowledged in network layer.
 */
int chimera_ping (ChimeraDHT * state, Host * host)
{

    char name[256];
    Message *message;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    host_encode (name, 256, chglob->me);

    if (host == NULL)
	{
	    return -1;
	}

    message =
	new Message (chglob->me->GetKey(), CHIMERA_PING, strlen (name) + 1,
			name);

    if (!this->message->message_send (host, message, FALSE))
	{
	    if (LOGS)
	      log_message (state->log, LOG_WARN, "failed to ping host %s:%d\n",
			   host->name, host->port);
	    delete message;
	    return 1;
	}

    delete message;
    return 0;
}

void chimera_ping_reply (ChimeraDHT * state, Message * message)
{

    Host *host;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;


    host = host_decode (state, message->GetPayload());

    // log_message(state->log,LOG_WARN, "received a PING message from %s:%d !\n",host->name,host->port );

}


/**
 ** chimera_init:
 ** Initializes Chimera on port port and returns the ChimeraDHT * which
 ** contains global state of different chimera modules.
*/

ChimeraDHT::ChimeraDHT(int port)
{
	char name[256];
	struct hostent *he;
	ChimeraGlobal *cg = new ChimeraGlobal();
	//  mtrace();
	this->chimera = cg;

	key_init ();

	this->message = new MessageGlobal(port);
	if (this->message == NULL)
		return (NULL);

	this->network = new NetworkGlobal(port);
	this->network_activate = new NetworkActivate(this->network);
	this->network_activate->Start();
	this->network_retransmit = new NetworkRetransmit(this->network);
	this->network_retransmit->Start();

	this->host = new HostGlobal(64);
	if (this->host == NULL)
		return (NULL);

	if (gethostname (name, 256) != 0)
	{
		pf_log[W_ERR] << "chimera_init: gethostname: " << strerror(errno);
		return (NULL);
	}
	if ((he = gethostbyname (name)) == NULL)
	{
		pf_log[W_ERR] << "chimera_init: gethostbyname: " << strerror (errno);
		return (NULL);
	}
	strcpy (name, he->h_name);

	cg->me = host->GetHost(name, port);

	sprintf (name + strlen (name), ":%d", port);
	key_makehash (&(cg->me->GetKey()), name);
	cg->deliver = NULL;
	cg->forward = NULL;
	cg->update = NULL;

	//this->route = new RouteGlobal(this->host, cg->me);
	this->routingStructure = new ChimeraRouting(this->host, cg->me);

	this->message->message_handler (CHIMERA_JOIN, chimera_message, 1);
	this->message->message_handler (CHIMERA_JOIN_ACK, chimera_join_acknowledged, 1);
	this->message->message_handler (CHIMERA_UPDATE, chimera_update_message, 1);
	this->message->message_handler (CHIMERA_PIGGY, chimera_piggy_message, 1);
	this->message->message_handler (CHIMERA_JOIN_NACK, chimera_join_denied, 1);
	this->message->message_handler (CHIMERA_PING, chimera_ping_reply, 1);

	/* more message types can be defined here */

	pthread_mutex_init (&cg->lock, NULL);
	cg->join = sema_create (0);
}

/**
 ** chimera_join:
 ** sends a JOIN message to bootstrap node and waits forever for the reply
 **
 */
void chimera_join (ChimeraDHT * state, Host * bootstrap)
{

    char name[256];
    Message *message;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    host_encode (name, 256, chglob->me);

    if (bootstrap == NULL)
	{
	    if (!(chimera_check_leafset_init (state)))
	      if (LOGS)
		log_message (state->log, LOG_ERROR,
			     "chimera_check_leafset_init FAILED \n");
	    return;
	}

    chglob->bootstrap = host_get (state, bootstrap->name, bootstrap->port);

    message =
	new Message (chglob->me->GetKey(), CHIMERA_JOIN, strlen (name) + 1,
			name);
    if (!this->message->message_send (bootstrap, message, true))
	{
	    if (LOGS)
	      log_message (state->log, LOG_ERROR,
			   "chimera_join: failed to contact bootstrap host %s:%d\n",
			   bootstrap->name, bootstrap->port);
	}

    sema_p (chglob->join, 0.0);
    delete message;
}


/** chimera_send:
 ** Route a message of type to key containing size bytes of data. This will
 ** send data through the Chimera system and deliver it to the host closest to the
 ** key.
 */
void chimera_send (ChimeraDHT * state, Key key, int type, int size,
		   char *data)
{

    Message *message;
    char s[NETWORK_PACK_SIZE];
    unsigned long realsize;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    /*
     * XXXPAT: this effectively does nothing right now, and it's unclear
     * why it's necessary even if it worked.
     */
#if 0
    realsize = htonl ((unsigned long) size);
    memcpy (s, &realsize, sizeof (unsigned long));
    memcpy (s + sizeof (unsigned long), data, size);
    size += sizeof (unsigned long);
#endif

    message = new Message (key, type, size, data);
    chimera_message (state, message);
    delete message;
}

/**
 ** chimera_forward:
 ** is called whenever a message is forwarded toward the destination the func upcall
 ** should be  defined by the user application
 */
void chimera_forward (ChimeraDHT * state, chimera_forward_upcall_t func)
{
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;
    chglob->forward = func;
}

/**
 ** chimera_deliver:
 ** is called whenever a message is delivered toward the destination the func upcall
 ** should be  defined by the user application
 */
void chimera_deliver (ChimeraDHT * state, chimera_deliver_upcall_t func)
{
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;
    chglob->deliver = func;
}

/**
 ** chimera_update:
 ** is called whenever a node joines or leaves the leafset the func upcall
 ** should be  defined by the user application
 */
void chimera_update (ChimeraDHT * state, chimera_update_upcall_t func)
{
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;
    chglob->update = func;
}


/**
 ** called by route_update to upcall into the user's system
 */
void chimera_update_upcall (ChimeraDHT * state, Key * k, Host * h,
			    int joined)
{
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;
    if (chglob->update != NULL)
	{
	    chglob->update (k, h, joined);
	}
}

#endif
