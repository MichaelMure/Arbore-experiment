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

#include "route.h"
#include "chimera.h"
#include "pf_log.h"
#include "network.h"
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


size_t ChimeraDHT::encode_hosts (char *s, size_t size, ChimeraHost ** host) const
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

ChimeraHost** ChimeraDHT::decode_hosts(char *s)
{

    ChimeraHost **host;
    size_t hostnum;
    size_t i, j, k;

    for (i = 0, hostnum = 0; i < strlen (s); i++)
	{
	    if (s[i] == '\n')
		hostnum++;
	}

    host = (ChimeraHost **) malloc (sizeof (ChimeraHost *) * (hostnum + 1));
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
    ChimeraHost **rowinfo;
    char s[NETWORK_PACK_SIZE];
    ChimeraHost *host;
    Message *msg;

    host = this->host->DecodeHost(this, message->payload);

    /* send one row of our routing table back to joiner #host# */
    rowinfo = route_row_lookup (this, host->GetKey());
    size = encode_hosts (s, NETWORK_PACK_SIZE, rowinfo);
    msg = message_create (host->GetKey(), CHIMERA_PIGGY, size, s);
    if (!message_send (this, host, msg, TRUE))
	pf_log[W_ERR] << "Sending row information to node! " << host->GetName() << ":" << host->GetPort() << " failed";

    free (rowinfo);
    message_free (msg);
}

/** chimera_join_complete:
** internal function that is called at the destination of a JOIN message. This
** call encodes the leaf set of the current host and sends it to the joiner.
**
*/
void ChimeraDHT::join_complete(ChimeraHost * host)
{
    char s[NETWORK_PACK_SIZE];
    Message *m;
    ChimeraHost **leafset;
    ChimeraGlobal *chglob = (ChimeraGlobal *) (this->chimera);

    /* copy myself into the reply */
    chglob->me->Encode(s, NETWORK_PACK_SIZE);
    strcat (s, "\n");		/* add a spacer */

    /* check to see if the node has just left the network or not */
    if ((dtime () - host->GetFailureTime()) < GRACEPERIOD)
	{
	      pf_log[W_WARNING] << "JOIN request from node: " << host->GetName() << ":" << host->GetPort()
		                << " rejected ,elapsed time since failure = " << dtime() - host->GetFailureTime() << " sec";

	    m = message_create (host->GetKey(), CHIMERA_JOIN_NACK, strlen (s) + 1,
				s);
	    if (!message_send (this, host, m, TRUE))
		pf_log[W_WARNING] << "message_send NACK failed!";
	    message_free (m);
	    return;
	}

    /* copy my leaf set into the reply */
    leafset = route_neighbors (this, LEAFSET_SIZE);
    encode_hosts (s + strlen (s),
			 NETWORK_PACK_SIZE - strlen (s), leafset);
    free (leafset);

    m = message_create (host->GetKey(), CHIMERA_JOIN_ACK, strlen (s) + 1, s);
    if (!message_send (this, host, m, TRUE))
	      pf_log[W_WARNING] << "message_send ACK failed!";

    message_free (m);
}

/**
 ** chimera_check_leafset: runs as a separate thread.
 ** it should send a PING message to each member of the leafset frequently and
 ** sends the leafset to other members of its leafset periodically.
 ** pinging frequecy is LEAFSET_CHECK_PERIOD.
 **
 */

void *chimera_check_leafset (void *chstate)
{

    char s[NETWORK_PACK_SIZE];
    Message *m;
    ChimeraHost **leafset;
    ChimeraHost **table;
    int i, count = 0;
    ChimeraDHT *state = (ChimeraDHT *) chstate;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;


    while (1)
	{

	    leafset = route_neighbors (state, LEAFSET_SIZE);
	    for (i = 0; leafset[i] != NULL; i++)
		{
		    if (state->Ping(leafset[i]) == 1)
			{
			    leafset[i]->SetFailureTime(dtime ());
			    pf_log[W_WARNING] << "message send to host: " << leafset[i]->GetName() << ":" << leafset[i]->GetPort()
			                      << " failed at time: " << leafset[i]->GetFailureTime() << "!";
			    if (leafset[i]->success_avg < BAD_LINK)
				{
				    printf ("Deleting %s:%d \n",
					    leafset[i]->name,
					    leafset[i]->port);
				  route_update (state, leafset[i], 0);
				}
			}
		    host_release (state, leafset[i]);
		}
	    free (leafset);

	    table = route_get_table (state);
	    for (i = 0; table[i] != NULL; i++)
		{
		    if (chimera_ping (state, table[i]) == 1)
			{
			    table[i]->GetFailureTime() = dtime ();
			    if (LOGS)
			      log_message (state->log, LOG_WARN,
					   "message send to host: %s:%d failed at time: %f!\n",
					   table[i]->name, table[i]->port,
					   table[i]->GetFailureTime());
			    if (table[i]->success_avg < BAD_LINK)
				{
				    route_update (state, table[i], 0);
				}
			}
		    host_release (state, table[i]);
		}
	    free (table);

	    /* send leafset exchange data every  3 times that pings the leafset */
	    if (count == 2)
		{
		    count = 0;
		    leafset = route_neighbors (state, LEAFSET_SIZE);
		    host_encode (s, NETWORK_PACK_SIZE, chglob->me);
		    strcat (s, "\n");	/* add a spacer */
		    encode_hosts (state->log, s + strlen (s),
					 NETWORK_PACK_SIZE - strlen (s),
					 leafset);

		    for (i = 0; leafset[i] != NULL; i++)
			{
			    m = message_create (leafset[i]->GetKey(),
						CHIMERA_PIGGY, strlen (s) + 1,
						s);
			    if (!message_send (state, leafset[i], m, TRUE))
				{
				  if (LOGS)
				    log_message (state->log, LOG_WARN,
						 "sending leafset update to %s:%d failed!\n",
						 leafset[i]->name,
						 leafset[i]->port);
				    if (leafset[i]->success_avg < BAD_LINK)
					{
					    route_update (state, leafset[i],
							  0);
					}
				}
			    message_free (m);
			    host_release (state, leafset[i]);
			}

		    free (leafset);

		}
	    else
		count++;

	    sleep (LEAFSET_CHECK_PERIOD);
	}
}

/**
 ** chimera_check_leafset_init:
 ** initiates a separate thread that constantly checks to see if the leafset members
 ** and table entries of the node are alive or not.
 **
 */
int chimera_check_leafset_init (ChimeraDHT * state)
{

    pthread_attr_t attr;
    pthread_t tid;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    if (pthread_attr_init (&attr) != 0)
	{
	    if (LOGS)
	      log_message (state->log, LOG_ERROR,
			   "(CHIMERA)pthread_attr_init: %s", strerror (errno));
	    return (0);
	}
    if (pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM) != 0)
	{
	    if (LOGS)
	      log_message (state->log, LOG_ERROR,
			   "(CHIMERA)pthread_attr_setscope: %s",
			   strerror (errno));
	    goto out;
	}
    if (pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED) != 0)
	{
	    if (LOGS)
	      log_message (state->log, LOG_ERROR,
			   "(CHIMERA)pthread_attr_setdetachstate: %s",
			   strerror (errno));
	    goto out;
	}

    if (pthread_create (&tid, &attr, chimera_check_leafset, (void *) state) !=
	0)
	{
	    if (LOGS)
	      log_message (state->log,
			   LOG_ERROR, "(CHIMERA)pthread_create: %s",
			   strerror (errno));
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

void chimera_join_denied (ChimeraDHT * state, Message * message)
{

    ChimeraHost *host;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    host = host_decode (state, message->payload);

    if (LOGS)
      log_message (state->log, LOG_WARN,
		   "JOIN request rejected from %s:%d !\n",
		   host->name, host->port);

    sleep (GRACEPERIOD);

    if (LOGS)
      log_message (state->log, LOG_WARN,
		   "Re-sending JOIN message to %s:%d !\n",
		   chglob->bootstrap->name, chglob->bootstrap->port);
    chimera_join (state, chglob->bootstrap);

}


/**
 ** chimera_route:
 ** routes a message one step closer to its destination key. Delivers
 ** the message to its destination if it is the current host through the
 ** deliver upcall, otherwise it makes the route upcall
 */

void chimera_route (ChimeraDHT * state, Key * key, Message * message,
		    ChimeraHost * host)
{

    ChimeraHost **tmp;
    //ChimeraHost **piggy;
    Message *real;
    unsigned long size;
    char s[NETWORK_PACK_SIZE];
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    real = message;

    tmp = route_lookup (state, *key, 1, 0);

    /* this is to avoid sending JOIN request to the node that
     ** its information is already in the routing table  ****/

    if ((tmp[0] != NULL) && (message->type == CHIMERA_JOIN)
	&& (key_equal (tmp[0]->GetKey(), *key)))
	{
	    free (tmp);
	    tmp = route_lookup (state, *key, 2, 0);
	    if (tmp[1] != NULL && key_equal (tmp[0]->GetKey(), *key))
		tmp[0] = tmp[1];
	}
    if (host == NULL && tmp[0] != chglob->me)
	{
	    host = tmp[0];
	}
    if (tmp[0] == chglob->me)
	{
	    host = NULL;
	}
    free (tmp);
    tmp = NULL;

    /* if I am the only host or the closest host is me, deliver the message */
    if (host == NULL)
	{
	    if (chglob->deliver != NULL)
		{
		    chglob->deliver (key, real);
		}
	    if (message->type == CHIMERA_JOIN)
		{
		    host = host_decode (state, message->payload);
		    chimera_join_complete (state, host);
		}
	}

    /* otherwise, route it */
    else
	{
	    if (chglob->forward != NULL)
		{
		    chglob->forward (&key, &real, &host);
		}
	    message = real;

	    while (!message_send (state, host, message, TRUE))
		{

		    host->GetFailureTime() = dtime ();
		    if (LOGS)
		      log_message (state->log, LOG_WARN,
				   "message send to host: %s:%d at time: %f failed!\n",
				   host->name, host->port, host->GetFailureTime());

		    /* remove the faulty node from the routing table */
		    if (host->success_avg < BAD_LINK)
			route_update (state, host, 0);

		    if (tmp != NULL)
			free (tmp);
		    tmp = route_lookup (state, *key, 1, 0);
		    host = tmp[0];
		    if (LOGS)
		      log_message (state->log, LOG_WARN,
				   "rerouting through %s:%d!\n", host->name,
				   host->port);
		}

	    /* in each hop in the way to the key root nodes
	       send their routing info to the joining node  */

	    if (message->type == CHIMERA_JOIN)
		chimera_send_rowinfo (state, message);

	    if (tmp != NULL)
		free (tmp);
	}

}

/**
 * chimera_join_acknowledge:
 * called when the current host is joining the network and has just revieced
 * its leaf set. This function sends an update message to all nodes in its
 * new leaf set to announce its arrival.
 */

void chimera_join_acknowledged (ChimeraDHT * state, Message * message)
{

    ChimeraHost **host;
    Message *m;
    char s[256];
    int i;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    host_encode (s, 256, chglob->me);
    host = decode_hosts(message->payload);

    /* announce my arrival to the nodes in my leafset */
    for (i = 0; host[i] != NULL; i++)
	{
	    route_update (state, host[i], 1);
	    m = message_create (host[i]->GetKey(), CHIMERA_UPDATE, strlen (s) + 1,
				s);
	    if (!message_send (state, host[i], m, TRUE))
		{
		  if (LOGS)
		    log_message (state->log, LOG_WARN,
				 "chimera_join_acknowledge: failed to update %s:%d\n",
				 host[i]->name, host[i]->port);
		}
	    message_free (m);
	}
    free (host);

    /* announce my arival to the nodes in my routing table */
    host = route_get_table (state);
    for (i = 0; host[i] != NULL; i++)
	{
	    m = message_create (host[i]->GetKey(), CHIMERA_UPDATE, strlen (s) + 1,
				s);
	    if (!message_send (state, host[i], m, TRUE))
		{
		  if (LOGS)
		    log_message (state->log, LOG_WARN,
				 "chimera_join_acknowledge: failed to update %s:%d\n",
				 host[i]->name, host[i]->port);
		}
	    message_free (m);
	}
    free (host);

    /* signal the chimera_join function, which is blocked awaying completion */
    sema_v (chglob->join);

    /* initialize the thread for leafset check and exchange */
    if (!(chimera_check_leafset_init (state)))
	{
	    if (LOGS)
	      log_message (state->log, LOG_ERROR,
			   "chimera_check_leafset_init FAILED \n");
	    return;
	}
}

/**
 ** chimera_message:
 ** routes the message through the chimera_route toward the destination
 **
 */
void chimera_message (ChimeraDHT * state, Message * message)
{
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;
    chimera_route (state, &message->dest, message, NULL);
}

void chimera_register (ChimeraDHT * state, int type, int ack)
{
    if (type < 10)
	{
	    if (LOGS)
	      log_message (state->log, LOG_ERROR,
			 "chimera_register: message integer types < 10 are reserved for system\n");
	    exit (1);
	}

    if (ack != 1 && ack != 2)
	{
	    if (LOGS)
	      log_message (state->log, LOG_ERROR,
			   "chimera_register: message property ack must be either 1 or 2 unrecognized ack value %i\n",
			 ack);
	    exit (1);
	}

    message_handler (state, type, chimera_message, ack);

}

void chimera_update_message (ChimeraDHT * state, Message * message)
{

    ChimeraHost *host;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    host = host_decode (state, message->payload);
    route_update (state, host, 1);

}

/**
 ** chimera_piggy_message:
 ** message handler for message type PIGGY ;) this used to be a piggy backing function
 ** This function is respopnsible to add the piggy backing node information that is sent along with
 ** other ctrl messages or separately to the routing table. the PIGGY message type is a separate
 ** message type.
 */

void chimera_piggy_message (ChimeraDHT * state, Message * message)
{

    ChimeraHost **piggy;
    ChimeraHost **tmp;
    int i;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    piggy = decode_hosts(message->payload);

    for (i = 0; piggy[i] != NULL; i++)
	{

	    if ((dtime () - piggy[i]->GetFailureTime()) > GRACEPERIOD)
		{
		    route_update (state, piggy[i], 1);
		}

	    else if (LOGS)
		log_message (state->log, LOG_WARN,
			     "refused to add:%s to routing table\n",
			     get_key_string (&piggy[i]->GetKey()));
	}
    free (piggy);
}



extern void route_keyupdate ();

void chimera_setkey (ChimeraDHT * state, Key key)
{

    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    key_assign (&(chglob->me->GetKey()), key);
    route_keyupdate (state->route, chglob->me);

}

/**
 ** chimera_ping:
 ** sends a PING message to the host. The message is acknowledged in network layer.
 */
int chimera_ping (ChimeraDHT * state, ChimeraHost * host)
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
	message_create (chglob->me->GetKey(), CHIMERA_PING, strlen (name) + 1,
			name);

    if (!message_send (state, host, message, FALSE))
	{
	    if (LOGS)
	      log_message (state->log, LOG_WARN, "failed to ping host %s:%d\n",
			   host->name, host->port);
	    message_free (message);
	    return 1;
	}

    message_free (message);
    return 0;
}

void chimera_ping_reply (ChimeraDHT * state, Message * message)
{

    ChimeraHost *host;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;


    host = host_decode (state, message->payload);

    // log_message(state->log,LOG_WARN, "received a PING message from %s:%d !\n",host->name,host->port );

}


/**
 ** chimera_init:
 ** Initializes Chimera on port port and returns the ChimeraDHT * which
 ** contains global state of different chimera modules.
*/

ChimeraDHT *chimera_init (int port)
{

    char name[256];
    struct hostent *he;
    ChimeraDHT *state;
    ChimeraGlobal *cg;
    //  mtrace();
    state = (ChimeraDHT *) malloc (sizeof (ChimeraDHT));
    cg = (ChimeraGlobal *) malloc (sizeof (ChimeraGlobal));
    state->chimera = (void *) cg;

    state->log = log_init ();
    log_direct (state->log, LOG_ERROR, stderr);
    key_init ();

    state->message = message_init ((void *) state, port);
    if (state->message == NULL)
	{
	    return (NULL);
	}

    state->host = host_init (state->log, 64);
    if (state->host == NULL)
	{
	    return (NULL);
	}

    if (gethostname (name, 256) != 0)
	{
	    if (LOGS)
	      log_message (state->log, LOG_ERROR,
			   "chimera_init: gethostname: %s\n",
			   strerror (errno));
	    return (NULL);
	}
    if ((he = gethostbyname (name)) == NULL)
	{
	    if (LOGS)
	      log_message (state->log, LOG_ERROR,
			   "chimera_init: gethostbyname: %s\n",
			   strerror (errno));
	    return (NULL);
	}
    strcpy (name, he->h_name);

    cg->me = host_get (state, name, port);

    sprintf (name + strlen (name), ":%d", port);
    key_makehash (state->log, &(cg->me->GetKey()), name);
    cg->deliver = NULL;
    cg->forward = NULL;
    cg->update = NULL;

    state->route = route_init (cg->me);

    message_handler (state, CHIMERA_JOIN, chimera_message, 1);
    message_handler (state, CHIMERA_JOIN_ACK, chimera_join_acknowledged, 1);
    message_handler (state, CHIMERA_UPDATE, chimera_update_message, 1);
    message_handler (state, CHIMERA_PIGGY, chimera_piggy_message, 1);
    message_handler (state, CHIMERA_JOIN_NACK, chimera_join_denied, 1);
    message_handler (state, CHIMERA_PING, chimera_ping_reply, 1);

    /* more message types can be defined here */

    pthread_mutex_init (&cg->lock, NULL);
    cg->join = sema_create (0);

    return (state);
}

/**
 ** chimera_join:
 ** sends a JOIN message to bootstrap node and waits forever for the reply
 **
 */
void chimera_join (ChimeraDHT * state, ChimeraHost * bootstrap)
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
	message_create (chglob->me->GetKey(), CHIMERA_JOIN, strlen (name) + 1,
			name);
    if (!message_send (state, bootstrap, message, TRUE))
	{
	    if (LOGS)
	      log_message (state->log, LOG_ERROR,
			   "chimera_join: failed to contact bootstrap host %s:%d\n",
			   bootstrap->name, bootstrap->port);
	}

    sema_p (chglob->join, 0.0);
    message_free (message);
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

    message = message_create (key, type, size, data);
    chimera_message (state, message);
    message_free (message);
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
void chimera_update_upcall (ChimeraDHT * state, Key * k, ChimeraHost * h,
			    int joined)
{
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;
    if (chglob->update != NULL)
	{
	    chglob->update (k, h, joined);
	}
}
