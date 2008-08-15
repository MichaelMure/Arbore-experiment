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

#include "network_activate.h"
#include "network.h"
#include "message.h"

NetworkActivate::NetworkActivate(NetworkGlobal* _ng)
	: Thread(PTHREAD_SCOPE_SYSTEM, PTHREAD_CREATE_DETACHED),
	  ng(_ng)
{

}

void NetworkActivate::Loop()
{
    fd_set fds, thisfds;
    int ret, retack;
    char data[SEND_SIZE];
    struct sockaddr_in from;
    int socklen = sizeof (from);
    unsigned long ack, seq;
    JRB node;

    FD_ZERO (&fds);
    FD_SET (ng->sock, &fds);

    while (1)
	{
	    /* block until information becomes available */
	    memcpy (&thisfds, &fds, sizeof (fd_set));
	    ret = select (ng->sock + 1, &thisfds, NULL, NULL, NULL);
	    if (ret < 0)
		{
		  if (LOGS)
		    log_message (chstate->log, LOG_ERROR,
				 "network: select: %s\n", strerror (errno));
		    continue;
		}

	    /* receive the new data */
	    ret =
		recvfrom (ng->sock, data, SEND_SIZE, 0,
			  (struct sockaddr *) &from, &socklen);
	    if (ret < 0)
		{
		  if (LOGS)
		    log_message (chstate->log, LOG_ERROR,
				 "network: recvfrom: %s\n", strerror (errno));
		    continue;
		}
	    memcpy (&ack, data, sizeof (unsigned long));
	    ack = ntohl (ack);
	    memcpy (&seq, data + sizeof (unsigned long),
		    sizeof (unsigned long));
	    seq = ntohl (seq);

	    /* process acknowledgement */
	    if (ack == 0)
		{
		  if (LOGS)
		    log_message (chstate->log, LOG_NETWORKDEBUG,
				 "network_activate: received ack seq=%d from %s:%d\n",
				 seq, inet_ntoa (from.sin_addr),
				 from.sin_port);

		    pthread_mutex_lock (&(ng->lock));
		    node = jrb_find_int (ng->waiting, seq);
		    if (node != NULL)
			{
				AckEntry *entry = (AckEntry *)node->val.v;
				entry->acked = 1;
				entry->acktime = dtime();
		//kpkp - enable
		//		fprintf(stderr, "Received an ack for packet %d at time entry->acktime %f\n", seq, entry->acktime);
			}
		    pthread_mutex_unlock (&(ng->lock));
		}

	    /* process receive and send acknowledgement */
	    else if (ack == 1)
		{
		  if (LOGS)
		    log_message (chstate->log, LOG_NETWORKDEBUG,
				 "network_activate: received message seq=%d  data:%s\n",
				 seq, data + (2 * sizeof (unsigned long)));
		    ack = htonl (0);
		    memcpy (data, &ack, sizeof (unsigned long));
		    retack =
			sendto (ng->sock, data, 2 * sizeof (unsigned long), 0,
				(struct sockaddr *) &from, sizeof (from));
		    if (retack < 0)
			{
			  if (LOGS)
			    log_message (chstate->log, LOG_ERROR,
					 "network: sendto: %s\n",
					 strerror (errno));
			  continue;
			}
		    if (LOGS)
		      log_message (chstate->log, LOG_NETWORKDEBUG,
				   "network_activate: sent out ack for  message seq=%d\n",
				   seq);
		    message_received (state,
				      data + (2 * sizeof (unsigned long)),
				      ret - (2 * sizeof (unsigned long)));
		}
	    else if (ack == 2)
		{
		    message_received (state,
				      data + (2 * sizeof (unsigned long)),
				      ret - (2 * sizeof (unsigned long)));
		}
	    else
		{
		  if (LOGS)
		    log_message (chstate->log, LOG_ERROR,
				 "network: received unrecognized message ack=%d seq=%d\n",
				 ack, seq);
		}
	}

}
