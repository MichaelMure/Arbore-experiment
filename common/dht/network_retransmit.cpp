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

#include "network_retransmit.h"
#include "network.h"

NetworkRetransmit::NetworkRetransmit(NetworkGlobal* _ng)
	: Thread(PTHREAD_SCOPE_SYSTEM, PTHREAD_CREATE_DETACHED),
	  ng(_ng)
{}

void NetworkRetransmit::Loop()
{
	JRB pqnode, node;
	double now = 0;
	PQEntry* pqentry;

	//JRB tempjrb = make_jrb(); -- to try doing send outside the lock block
	while(1)
	{
		// wake up, get all the packets to be transmitted by now, send them again or delete them from the priqueue
		now = dtime();
		int resend = 0;

		pthread_mutex_lock (&(ng->lock));
		for(pqnode = jrb_first(ng->retransmit); pqnode != jrb_nil(ng->retransmit) && pqnode->key.d <= now; pqnode = jrb_next(pqnode))
		{
			pqentry = (PQEntry *) pqnode->val.v;
			//kpkp - debug enable
			//	fprintf(stderr, "processing a packet with retransmit time %f; looking for seqnum %d\n", pqnode->key.d, pqentry->seqnum);

			node = jrb_find_int (ng->waiting, pqentry->seqnum);
			if (node != NULL)
			{
				AckEntry *ackentry = (AckEntry *)node->val.v;
				if(ackentry->acked == 0) // means, if the packet is not yet acknowledged
				{
					// packet is not acked
					//kpkp - debug enable
					//			fprintf(stderr, "packet %d is not acked... retransmitting for %dth time\n", node->key.i, pqentry->retry);

					resend = 1;
					//jrb_insert_dbl(tempjrb, pqnode->key.d, new_jval_v(pqentry));-- to try doing send outside the lock block
					double transmittime = dtime();
					network_resend(state, pqentry->desthost, pqentry->data, pqentry->datasize, 1, pqentry->seqnum, &transmittime);
					pqentry->retry = pqentry->retry + 1;
					if (pqentry->retry < MAX_RETRY)
					{
						PQEntry *newentry = get_new_pqentry();
						newentry->desthost = pqentry->desthost;
						newentry->data = pqentry->data;
						newentry->datasize = pqentry->datasize;
						newentry->retry = pqentry->retry;
						newentry->seqnum = pqentry->seqnum;
						newentry->transmittime = transmittime;

						jrb_insert_dbl (ng->retransmit, (transmittime+RETRANSMIT_INTERVAL), new_jval_v (newentry));
					}
					else
					{
						// max retransmission has expired -- update the host stats, free up the resources
						host_update_stat (pqentry->desthost, 0);

						free(pqentry->data);
					}
					// delete this node
					jrb_delete_node(pqnode);
				}
				else
				{
					//kpkp - enable
					//			fprintf(stderr, "packet %d is acked... deleting from DS\n", node->key.i);
					// packet is acked;
					// update the host latency and the success measurements
					host_update_stat (pqentry->desthost, 1);
					double latency = ackentry->acktime - pqentry->transmittime;
					if(latency > 0)
					{
						//kpkp - debug enable
						//fprintf(stderr, "latency for one host is %f \n", latency);
						if (pqentry->desthost->latency == 0.0)
						{
							pqentry->desthost->latency = latency;
						}
						else
						{
							pqentry->desthost->latency =
								(0.9 * pqentry->desthost->latency) + (0.1 * latency);
						}
					}
					jrb_delete_node(node);
					jrb_delete_node(pqnode);
				}
			}
			else
			{
				//kpkp - debug enable
				//		fprintf(stderr, "didn't find the seqnum in the DS \n");
			}
		}
		pthread_mutex_unlock (&(ng->lock));

		/* -- to try doing send outside the lock block
		   if (resend != 0)
		   for(pqnode = jrb_first(tempjrb); pqnode != jrb_nil(tempjrb); pqnode = jrb_next(pqnode))
		   {
		   pqentry = (PQEntry *) pqnode->val.v;
		   network_resend(state, pqentry->desthost, pqentry->data, pqentry->datasize, 1, pqentry->seqnum);
		   jrb_delete_node(pqnode);
		   }
		 */

		sleep(RETRANSMIT_THREAD_SLEEP);
	}

}
