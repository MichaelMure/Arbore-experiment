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
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "route.h"
#include "hosts_list.h"
#include "jrb.h"
#include "jval.h"

int hexalpha_to_int (int c);

/** route_init:
** Ininitiates routing table and leafsets
*/
RouteGlobal::RouteGlobal(HostGlobal* _hg, ChimeraHost * _me)
	: hg(_hg),
	  me(_me)
{
	int i, j, k;

	/* allocate memory for routing table */
	this->table = (ChimeraHost ****) malloc (sizeof (struct ChimeraHost ***) * MAX_ROW);
	for (i = 0; i < MAX_ROW; i++)
	{
		this->table[i] = (ChimeraHost ***) malloc (sizeof (ChimeraHost **) * MAX_COL);
		for (j = 0; j < MAX_COL; j++)
		{
			this->table[i][j] = (ChimeraHost **) malloc (sizeof (ChimeraHost *) * MAX_ENTRY);
			for (k = 0; k < MAX_ENTRY; k++)
				this->table[i][j][k] = NULL;
		}
	}

	this->Rrange = me->GetKey();
	this->Lrange = me->GetKey();

	/* allocate memory for leafsets */
	this->leftleafset = (ChimeraHost **) malloc (sizeof (ChimeraHost *) * ((LEAFSET_SIZE / 2) + 1));
	this->rightleafset = (ChimeraHost **) malloc (sizeof (ChimeraHost *) * ((LEAFSET_SIZE / 2) + 1));
	for (i = 0; i < (LEAFSET_SIZE / 2) + 1; i++)
	{
		this->leftleafset[i] = NULL;
		this->rightleafset[i] = NULL;
	}
}

/** route_get_table:
 ** return the entire routing table
 */
ChimeraHost** RouteGlobal::route_get_table ()
{
	ChimeraHost **ret;
	int i, j, l, k, count;

	BlockLockMutex(this);

	count = 0;

	for (i = 0; i < MAX_ROW; i++)
		for (j = 0; j < MAX_COL; j++)
			for (l = 0; l < MAX_ENTRY; l++)
				if (this->table[i][j][l] != NULL)
					count++;
	ret = (ChimeraHost **) malloc (sizeof (ChimeraHost *) * (count + 1));
	k = 0;
	for (i = 0; i < MAX_ROW; i++)
		for (j = 0; j < MAX_COL; j++)
			for (l = 0; l < MAX_ENTRY; l++)
				if (this->table[i][j][l] != NULL)
				{
					ret[k++] = hg->GetHost(this->table[i][j][l]->GetName().c_str(), this->table[i][j][l]->GetPort());
				}
	ret[k] = NULL;
	return ret;

}

/** route_row_lookup:key
 ** return the row in the routing table that matches the longest prefix with #key#
 */
ChimeraHost** RouteGlobal::route_row_lookup (Key key)
{
	ChimeraHost **ret;
	size_t i, j, k, l, count;
	BlockLockMutex(this);

	i = this->me->GetKey().key_index(key);

	/* find out the number of hosts exists in the matching row */
	count = 0;
	for (j = 0; j < MAX_COL; j++)
		for (l = 0; l < MAX_ENTRY; l++)
			if (this->table[i][j][l] != NULL)
				count++;

	ret = (ChimeraHost **) malloc (sizeof (ChimeraHost *) * (count + 2));
	k = 0;
	for (j = 0; j < MAX_COL; j++)
		for (l = 0; l < MAX_ENTRY; l++)
			if (this->table[i][j][l] != NULL)
				ret[k++] = hg->GetHost(this->table[i][j][l]->GetName().c_str(), this->table[i][j][l]->GetPort());

	ret[k++] = hg->GetHost(this->me->GetName().c_str(), this->me->GetPort());
	ret[k] = NULL;

	return ret;
}

/** route_lookup:
 ** returns an array of #count# nodes that are acceptable next hops for a
 ** message being routed to #key#. #is_save# is ignored for now.
 */
ChimeraHost** RouteGlobal::route_lookup (Key key, unsigned int count, int is_safe)
{
	size_t i, j, k, Lsize, Rsize;
	int index = 0, match_col = 0, next_hop = 0;
	ChimeraHost *leaf, *tmp, *min;
	Key dif1, dif2;
	ChimeraHost **ret, **hosts;
	BlockLockMutex lock(this);

	pf_log[W_ROUTING] << me->GetKey() << ": lookup for key " << key;

	/*calculate the leafset and table size */
	Lsize = leafset_size (this->leftleafset);
	Rsize = leafset_size (this->rightleafset);

	/* if the key is in the leafset range route through leafset */
	/* the additional 2 ChimeraHosts pointed by the #hosts# are to consider the node itself and NULL at the end */
	if (count == 1 && key.between(this->Lrange, this->Rrange))
	{
		pf_log[W_ROUTING] << "Routing through leafset";
		hosts = (ChimeraHost **) malloc (sizeof (ChimeraHost *) * (LEAFSET_SIZE + 2));
		memset ((void *) hosts, 0, (sizeof (ChimeraHost *) * (LEAFSET_SIZE + 2)));
		ret = (ChimeraHost **) malloc (sizeof (ChimeraHost *) * (count + 1));
		memset ((void *) ret, 0, (sizeof (ChimeraHost *) * (count + 1)));

		hosts[index++] = this->me;
		pf_log[W_ROUTING] << "ME: (" << *me << ", " << me->GetKey() << ")";

		/* look left */
		for (i = 0; i < Lsize; i++)
		{
			leaf = this->leftleafset[i];
			pf_log[W_ROUTING] << "Left_leafset[" << i << "]: (" << *leaf << ", " << leaf->GetKey();
			hosts[index++] = leaf;
		}

		/* look right */
		for (i = 0; i < Rsize; i++)
		{
			leaf = this->rightleafset[i];
			pf_log[W_ROUTING] << "Right_leafset[" << i << "]: (" << *leaf << ", " << leaf->GetKey();
			hosts[index++] = leaf;
		}
		hosts[index] = NULL;

		min = find_closest_key (hosts, key, index);

		ret[0] = min;
		ret[1] = NULL;
		pf_log[W_ROUTING] << "++NEXT_HOP = " << ret[0]->GetKey();
		free (hosts);
		return (ret);
	}

	/* check to see if there is a matching next hop (for fast routing) */
	i = this->me->GetKey().key_index(key);
	match_col = hexalpha_to_int (key.str()[i]);

	for (k = 0; k < MAX_ENTRY; k++)
		if (this->table[i][match_col][k] != NULL && this->table[i][match_col][k]->GetSuccessAvg() > BAD_LINK)
		{
			next_hop = 1;
			tmp = this->table[i][match_col][k];
			break;
		}

	if (next_hop == 1 && count == 1)
	{
		for (k = 0; k < MAX_ENTRY; k++)
		{
			if (this->table[i][match_col][k] != NULL)
				if ((this->table[i][match_col][k]->GetSuccessAvg() > tmp->GetSuccessAvg())
				    || (this->table[i][match_col][k]->GetSuccessAvg() == tmp->GetSuccessAvg()
				        && this->table[i][match_col][k]->GetLatency() < tmp->GetLatency()))
				{
					tmp = this->table[i][match_col][k];
				}
		}
		ret = (ChimeraHost **) malloc (sizeof (ChimeraHost *) * (count + 1));
		ret[0] = hg->GetHost(tmp->GetName().c_str(), tmp->GetPort());
		ret[1] = NULL;
		pf_log[W_ROUTING] << "Routing through Table(" << this->me->GetKey()
		                  << ") NEXT_HOP=" << ret[0]->GetKey();
		return (ret);
	}

	/* if there is no matching next hop we have to find the best next hop */
	/* brute force method to solve count requirements */

	hosts = (ChimeraHost **) malloc (sizeof (ChimeraHost *) * (LEAFSET_SIZE + 1 + (MAX_COL * MAX_ENTRY)));
	memset ((void *) hosts, 0, (sizeof (ChimeraHost *) * (LEAFSET_SIZE + 1 + (MAX_COL * MAX_ENTRY))));

	pf_log[W_ROUTING] << "Routing to next closest key I know of:";
	leaf = this->me;

	pf_log[W_ROUTING] << "+me: (" << *leaf << ", " << leaf->GetKey() << ")";
	hosts[index++] = this->me;

	/* look left */
	for (i = 0; i < Lsize; i++)
	{
		leaf = this->leftleafset[i];
		pf_log[W_ROUTING] << "+Left_leafset[" << i << "]: (" << *leaf << ", " << leaf->GetKey();
		hosts[index++] = leaf;
	}

	/* look right */
	for (i = 0; i < Rsize; i++)
	{
		leaf = this->rightleafset[i];
		pf_log[W_ROUTING] << "+Right_leafset[" << i << "]: (" << *leaf << ", " << leaf->GetKey();
		hosts[index++] = leaf;
	}

	/* find the longest prefix match */
	i = this->me->GetKey().key_index(key);

	for (j = 0; j < MAX_COL; j++)
		for (k = 0; k < MAX_ENTRY; k++)
			if (this->table[i][j][k] != NULL
			    && this->table[i][j][k]->GetSuccessAvg() > BAD_LINK)
			{
				leaf = this->table[i][j][k];
				pf_log[W_ROUTING] << "+Table[" << i << "]"
				                        << "[" << j << "]"
							<< "[" << k << "]"
							<< ": (" << *leaf
							<< ", " << leaf->GetKey()
							<< ")";
				hosts[index++] = leaf;
			}

	hosts[index] = NULL;

	ret = (ChimeraHost **) malloc (sizeof (ChimeraHost *) * (count + 1));

	if (count == 1)
	{
		ret[0] = find_closest_key (hosts, key, index);
		ret[1] = NULL;
	}
	else
	{
		sort_hosts (hosts, key, index);
		/* find the best #count# entries that we looked at... could be much better */
		for (i = 0, j = 0; hosts[i] != NULL && (i - j) < count; i++)
		{
			tmp = hosts[i];

			if ((i - j) > 0 && ret[(i - j) - 1]->GetKey() == tmp->GetKey())
			{
				j++;
				continue;
			}
			pf_log[W_ROUTING] << "++Result[" << i << "]: " << "(" << *tmp << ", " << tmp->GetKey() << ")";
			ret[i - j] = hg->GetHost(tmp->GetName().c_str(), tmp->GetPort());
		}
		ret[i - j] = NULL;
	}

	/*  to prevent bouncig */
	if (count == 1)
	{
		dif1 = key.distance(ret[0]->GetKey());
		dif2 = key.distance(this->me->GetKey());
		if (dif1 == dif2)
			ret[0] = this->me;
	}
	free (hosts);

	return (ret);
}

/** sort_hosts_key:
 ** Sorts #hosts# based on their key distance from #Key#, closest node first
 */
void RouteGlobal::sort_hosts_key (ChimeraHost ** hosts, Key key, size_t size) const
{
	size_t i, j;
	ChimeraHost *tmp;
	Key dif1;
	Key dif2;
	for (i = 0; i < size; i++)
	{
		for (j = i + 1; j < size; j++)
		{
			if (hosts[i] != NULL && hosts[j] != NULL)
			{
				dif1 = hosts[i]->GetKey().distance(key);
				dif2 = hosts[j]->GetKey().distance(key);
				if (dif2 < dif1)
				{
					tmp = hosts[i];
					hosts[i] = hosts[j];
					hosts[j] = tmp;
				}
			}
		}
	}
}

/** find_closest_key:
 ** finds the closest node in the array of #hosts# to #key# and put that in min.
 */
ChimeraHost* RouteGlobal::find_closest_key (ChimeraHost ** hosts, Key key, size_t size) const
{
	size_t i;
	Key dif;
	Key mindif;
	ChimeraHost *min, *tmp;

	if (size == 0)
	{
		min = NULL;
		return NULL;
	}

	else
	{
		min = hosts[0];
		mindif = hosts[0]->GetKey().distance(key);
	}

	for (i = 0; i < size; i++)
	{
		if (hosts[i] != NULL)
		{
			dif = hosts[i]->GetKey().distance(key);

			if (dif < mindif)
			{
				min = hosts[i];
				mindif = dif;
			}
		}
	}
	tmp = this->hg->GetHost(min->GetName().c_str(), min->GetPort());
	return (min);
}

/** sort_hosts:
 ** Sorts #hosts# based on common prefix match and key distance from #Key#
 */
void RouteGlobal::sort_hosts (ChimeraHost ** hosts, Key key, size_t size) const
{
	size_t i, j;
	ChimeraHost *tmp;
	Key dif1;
	Key dif2;
	size_t pmatch1 = 0;
	size_t pmatch2 = 0;


	for (i = 0; i < size; i++)
	{
		for (j = i + 1; j < size; j++)
		{
			if (hosts[i] != NULL && hosts[j] != NULL)
			{
				pmatch1 = key.key_index(hosts[i]->GetKey());
				pmatch2 = key.key_index(hosts[j]->GetKey());
				if (pmatch2 > pmatch1)
				{
					tmp = hosts[i];
					hosts[i] = hosts[j];
					hosts[j] = tmp;
				}
				else if (pmatch1 == pmatch2)
				{
					dif1 = hosts[i]->GetKey().distance(key);
					dif2 = hosts[j]->GetKey().distance(key);

					if(dif2 < dif1)
					{
						tmp = hosts[i];
						hosts[i] = hosts[j];
						hosts[j] = tmp;
					}
				}
			}

		}
	}
}

int RouteGlobal::leafset_size (ChimeraHost ** arr) const
{
	int i = 0;
	for (i = 0; arr[i] != NULL; i++)
		;
	return i;
}


/**
 ** leafset_range_update:
 ** updates the leafset range whenever a node leaves or joins to the leafset
 **
 */
void RouteGlobal::leafset_range_update (Key * rrange, Key * lrange) const
{
	int i;

	/* right range */
	for (i = 0; this->rightleafset[i] != NULL; i++)
		*rrange = this->rightleafset[i]->GetKey();
	if (i == 0)
		*lrange = this->me->GetKey(); // XXX: perhaps lvalue must be rrange..

	/* left range */
	if (this->leftleafset[0] != NULL)
		*lrange = this->leftleafset[0]->GetKey();
	else
		*lrange = this->me->GetKey();
}


/**
 ** leafset_update:
 ** this function is called whenever a route_update is called the joined
 ** is 1 if the node has joined and 0 if a node is leaving.
 **
 */
void RouteGlobal::leafset_update (ChimeraHost * host, int joined, ChimeraHost ** deleted, ChimeraHost ** added) const
{
	int Lsize = 0;
	int Rsize = 0;
	Key midpoint;

	Lsize = leafset_size (this->leftleafset);
	Rsize = leafset_size (this->rightleafset);

	midpoint = this->me->GetKey().midpoint();

	if (joined)
	{
		/* key falls in the right side of node */
		if (Rsize < LEAFSET_SIZE / 2
		    || host->GetKey().between(this->me->GetKey(), this->rightleafset[Rsize - 1]->GetKey()))
		{
			/* insert in Right leafset */
			leafset_insert (host, 1, deleted, added);
		}
		/* key falls in the left side of the node */
		if (Lsize < LEAFSET_SIZE / 2
		    || host->GetKey().between(this->leftleafset[Lsize - 1]->GetKey(), this->me->GetKey()))
		{
			/* insert in Left leafset */
			leafset_insert (host, 0, deleted, added);
		}
	}
	else
	{
		if (host->GetKey().between(this->me->GetKey(), midpoint))
		{
			leafset_delete (host, 1, deleted);
		}
		else
			leafset_delete (host, 0, deleted);
	}

}

/**
 ** leafset_delete:
 ** removes the #deleted# node from leafset
 **
 */
void RouteGlobal::leafset_delete (ChimeraHost * host, int right_or_left, ChimeraHost ** deleted) const
{
	int i = 0, size;
	int match = 0;
	ChimeraHost **p;

	/*insert in right leafset */
	if (right_or_left == 1)
	{
		size = leafset_size (this->rightleafset);
		p = this->rightleafset;
	}
	/*insert in left leafset */
	else
	{
		size = leafset_size (this->leftleafset);
		p = this->leftleafset;
	}

	for (i = 0; i < size && p[i]->GetKey() != host->GetKey(); i++)
		;

	if (i < size)
	{
		*deleted = p[i];
		match = 1;
	}

	/* shift leafset members to not have a hole in the leafset */
	if (match)
	{
		do
		{
			p[i] = p[i + 1];
			i++;
		}
		while (i < size - 1);
		p[i] = NULL;
	}


}

/**
 **leafset_insert:
 ** inserts the added node tot the leafset and removes the deleted from the leafset
 ** the deleted node is NULL if the new added node will not cause a node to leave the leafset.
 */
void RouteGlobal::leafset_insert (ChimeraHost * host,
		     int right_or_left, ChimeraHost ** deleted,
		     ChimeraHost ** added) const
{
	int i = 0, size;
	ChimeraHost **p;
	ChimeraHost *tmp1, *tmp2;
	ChimeraHost *input = host;
	Key dif1, dif2;

	/*inert in right leafset */
	if (right_or_left == 1)
	{
		size = leafset_size (this->rightleafset);
		p = this->rightleafset;
	}
	/*insert in left leafset */
	else
	{
		size = leafset_size (this->leftleafset);
		p = this->leftleafset;
	}

	if (size == 0)
	{
		p[0] = input;
		*added = input;
	}

	else
	{
		// to avoid duplicate entries in the same leafset
		if (p[i]->GetKey() == input->GetKey())
		{
			hg->ReleaseHost(input);
			return;
		}

		bool foundKeyPos = false;
		if (right_or_left == 1)
		{
			foundKeyPos = host->GetKey().between(this->me->GetKey(), p[i]->GetKey());
		}
		else
		{
			foundKeyPos = host->GetKey().between(p[i]->GetKey(), this->me->GetKey());
		}

		while ((i < size) && !foundKeyPos)
		{
			if (p[i]->GetKey() == input->GetKey())
			{
				hg->ReleaseHost(input);
				return;
			}
			i++;
			if (i < size)
			{
				if (right_or_left == 1)
				{
					foundKeyPos = host->GetKey().between(this->me->GetKey(), p[i]->GetKey());
				}
				else
				{
					foundKeyPos = host->GetKey().between(p[i]->GetKey(), this->me->GetKey());
				}
			}
		}

		tmp1 = input;
		*added = input;

		while (i < LEAFSET_SIZE / 2)
		{
			tmp2 = p[i];
			p[i++] = tmp1;
			tmp1 = tmp2;
		}

		/* there is leftover */
		if (tmp2 != NULL && size == LEAFSET_SIZE / 2)
			*deleted = tmp2;
	}
}

/** route_neighbors:
** returns an array of #count# neighbor nodes with priority to closer nodes
*/
ChimeraHost **RouteGlobal::route_neighbors (int count) const
{
	int i = 0, Rsize = 0, Lsize = 0, index = 0;
	int ret_size;
	ChimeraHost *tmp;
	ChimeraHost **hosts = (ChimeraHost **) malloc (sizeof (ChimeraHost *) * (LEAFSET_SIZE + 1));
	ChimeraHost **ret = (ChimeraHost **) malloc (sizeof (ChimeraHost *) * (count + 1));

	BlockLockMutex lock(this);

	Lsize = leafset_size (this->leftleafset);
	Rsize = leafset_size (this->rightleafset);

	if (count > Rsize + Lsize)
	{
		ret_size = Rsize + Lsize;
	}
	else
	ret_size = count;

	/* creat a jrb of leafset pointers sorted on distance */
	for (i = 0; i < Lsize; i++)
	{
		tmp = this->leftleafset[i];
		hosts[index++] = tmp;
	}

	for (i = 0; i < Rsize; i++)
	{
		tmp = this->rightleafset[i];
		hosts[index++] = tmp;
	}

	hosts[index] = NULL;
	/* sort aux */
	sort_hosts (hosts, this->me->GetKey(), index);

	for (i = 0; i < ret_size; i++)
	{
		tmp = hosts[i];
		ret[i] = hg->GetHost(tmp->GetName().c_str(), tmp->GetPort());
	}

	ret[i] = NULL;

	free (hosts);

	return ret;
}


/** route_update:
** updated the routing table in regard to #host#. If the host is joining
** the network (and #joined# == 1), then it is added to the routing table
** if it is appropriate. If it is leaving the network (and #joined# == 0),
** then it is removed from the routing tables
*/

void RouteGlobal::route_update (ChimeraHost * host, int joined)
{
	size_t i, j, k, pick;
	bool found;
	ChimeraHost *deleted = NULL, *added = NULL;
	BlockLockMutex p(this);

	if(this->me->GetKey() == host->GetKey())
		return;

	i = this->me->GetKey().key_index(host->GetKey());
	j = hexalpha_to_int (host->GetKey().str()[i]);

	/*join */
	if (joined)
	{
		found = false;
		for (k = 0; k < MAX_ENTRY; k++)
		{
			if (this->table[i][j][k] == NULL)
			{
				this->table[i][j][k] = hg->GetHost(host->GetName().c_str(), host->GetPort());
				leafset_update (host, joined, &deleted, &added);
				found = true;
				break;
			}
			else if (this->table[i][j][k] != NULL && this->table[i][j][k]->GetKey() == host->GetKey())
			{
				return;
			}
		}

		/* the entry array is full we have to get rid of one */
		/* replace the new node with the node with the highest latncy in the entry array */
		if (!found)
		{
			pick = 0;
			for (k = 1; k < MAX_ENTRY; k++)
			{
				if (this->table[i][j][pick]->GetSuccessAvg() > this->table[i][j][k]->GetSuccessAvg())
					pick = k;
			}
			this->hg->ReleaseHost(this->table[i][j][pick]);
			this->table[i][j][pick] = this->hg->GetHost(host->GetName().c_str(), host->GetPort());
			leafset_update (host, joined, &deleted, &added);
		}
	}

	/*delete */
	else
	{
		for (k = 0; k < MAX_ENTRY; k++)
			if (this->table[i][j][k] != NULL && this->table[i][j][k]->GetKey() == host->GetKey())
			{
				this->hg->ReleaseHost(this->table[i][j][k]);
				this->table[i][j][k] = NULL;
				break;
			}

		leafset_update (host, joined, &deleted, &added);

	}

	if (deleted != NULL)
	{
		leafset_range_update (&this->Rrange, &this->Lrange);

		/* TODO
		chimera_update_upcall (&(deleted->key), deleted, 0);
		*/
	}
	if (added != NULL)
	{
		leafset_range_update (&this->Rrange, &this->Lrange);
		/* TODO
		chimera_update_upcall (&(added->key), added, 1);
		*/
	}

	fflush (stderr);
}

int hexalpha_to_int (int c)
{
    char hexalpha[] = "0123456789abcdef";
    int i;
    int answer = 0;

    for (i = 0; answer == 0 && hexalpha[i] != '\0'; i++)
	{
	    if (hexalpha[i] == c)
		{
		    answer = i;
		}
	}

    return answer;
}

void RouteGlobal::printTable () const
{
	size_t i, j, k;

	/* print the table */

	fprintf (stderr,
		 "------------------------------- TABLE-------------------------------\n");
	for (i = 0; i < MAX_ROW; i++)
	{
		for (j = 0; j < MAX_COL; j++)
		{
			for (k = 0; k < MAX_ENTRY; k++)
				if (this->table[i][j][k] != NULL)
					fprintf (stderr, "%s ", this->table[i][j][k]->GetKey().str().c_str());
				else
					fprintf (stderr,
						 "00000000 00000000 00000000 00000000 00000000");
		}
		fprintf (stderr, "\n");
	}
	fprintf (stderr,
		 "----------------------------------------------------------------------\n");
}
