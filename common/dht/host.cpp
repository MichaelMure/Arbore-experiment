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

#include "host.h"

typedef struct
{
	ChimeraHost *host;
	int refrence;
	JRB node;
	Dllist free;
} CacheEntry;

void cacheentry_free (CacheEntry * entry)
{
	if (entry == NULL);
	free (entry->host->name);
	free (entry->host);
	free (entry);
}

/** host_encode:
 ** encodes the #host# into a string, putting it in #s#, which has
 ** #len# bytes in it.
 */
void ChimeraHost::host_encode (char *s, int len) const
{

	snprintf (s, len, "%s:", get_key_string (&key));
	snprintf (s + strlen (s), len - strlen (s), "%s:", host->name);
	snprintf (s + strlen (s), len - strlen (s), "%d", host->port);

}

/** host_init:
 ** initialize a host struct with a #size# element cache.
 */
HostGlobal::HostGlobal(int size)
{
	this->hosts = make_jrb ();
	this->free = new_dllist ();
	this->size = 0;
	this->max = size;
}

/** host_decode:
 ** decodes a string into a chimera host structure. This acts as a
 ** host_get, and should be followed eventually by a host_release.
 */
ChimeraHost* HostGlobal::DecodeHost(ChimeraDHT * state, const char *hostname)
{
	char *key = NULL, *name = NULL, *port = NULL;
	ChimeraHost *host;
	int i;
	Key k;
	char* s = strdup(hostname);

	/* hex representation of key in front bytes */
	key = s;

	/* next is the name of the host */
	for (i = 0; s[i] != ':' && s[i] != 0; i++)
		;
	s[i] = 0;
	name = s + (i + 1);

	/* then a human readable integer of the port */
	for (i++; s[i] != ':' && s[i] != 0; i++)
		;
	s[i] = 0;
	port = s + (i + 1);

	/* allocate space, do the network stuff, and return the host */
	sscanf (port, "%d", &i);
	host = GetHost(state, name, i);

	str_to_key (key, &k);

	if (key_equal_ui (host->key, 0))
		key_assign (&(host->key), k);

	free(s);

	return (host);

}

/** host_get:
 ** gets a host entry for the given host, getting it from the cache if
 ** possible, or alocates memory for it
 */
ChimeraHost* HostGlobal::GetHost(ChimeraDHT * state, char *hostname, int port)
{
	JRB node;
	Dllist dllnode;
	unsigned long address;
	CacheEntry *tmp, *entry;
	unsigned char *ip;
	char id[256];
	int i;

	/* create an id of the form ip:port */
	memset (id, 0, 256);
	address = network_address (state->network, hostname);
	ip = (unsigned char *) &address;
	for (i = 0; i < 4; i++)
		sprintf (id + strlen (id), "%s%d", (i == 0) ? ("") : ("."),
		                                   (int) ip[i]);
	sprintf (id + strlen (id), ":%d", port);

	pthread_mutex_lock (&this->lock);
	node = jrb_find_str (this->hosts, id);

	/* if the node is not in the cache, create an entry and allocate a host */
	if (node == NULL)
	{
		entry = (CacheEntry *) malloc (sizeof (CacheEntry));
		entry->host = (ChimeraHost *) malloc (sizeof (ChimeraHost));
		entry->host->name = strdup (hostname);
		entry->host->port = port;
		entry->host->address = address;
		entry->host->failed = 0;
		entry->host->failuretime = 0;

		key_assign_ui (&(entry->host->key), 0);

		entry->host->success_win_index = 0;
		for (i = 0; i < SUCCESS_WINDOW / 2; i++)
			entry->host->success_win[i] = 0;
		for (i = SUCCESS_WINDOW / 2; i < SUCCESS_WINDOW; i++)
			entry->host->success_win[i] = 1;
		entry->host->success_avg = 0.5;
		entry->refrence = 1;
		jrb_insert_str (this->hosts, strdup (id), new_jval_v (entry));

		entry->node = jrb_find_str (this->hosts, id);
		this->size++;
	}

	/* otherwise, increase the refrence count */
	else
	{
		entry = (CacheEntry *) node->val.v;
		/* if it was in the free list, remove it */
		if (entry->refrence == 0)
		{
			dll_delete_node (entry->free);
		}
		entry->refrence++;
	}

	/* if the cache was overfull, empty it as much as possible */
	while (this->size > this->max && !jrb_empty (this->free))
	{
		dllnode = dll_first (this->free);
		tmp = (CacheEntry *) dllnode->val.v;
		dll_delete_node (dllnode);
		jrb_delete_node (tmp->node);
		cacheentry_free (tmp);
		this->size--;
	}
	pthread_mutex_unlock (&this->lock);
	return (entry->host);
}

/** host_release:
 ** releases a host from the cache, declaring that the memory could be
 ** freed any time. returns NULL if the entry is deleted, otherwise it
 ** returns #host#
 */
void HostGlobal::ReleaseHost(ChimeraHost * host)
{

	JRB node;
	Dllist dllnode;
	CacheEntry *entry, *tmp;
	unsigned char *ip;
	char id[256];
	int i;

	/* create an id of the form ip:port */
	memset (id, 0, 256);
	ip = (unsigned char *) &host->address;
	for (i = 0; i < 4; i++)
		sprintf (id + strlen (id), "%s%d", (i == 0) ? ("") : ("."),
		                                   (int) ip[i]);
	sprintf (id + strlen (id), ":%d", host->port);

	pthread_mutex_lock (&this->lock);
	node = jrb_find_str (this->hosts, id);
	if (node == NULL)
	{
		pthread_mutex_unlock (&this->lock);
		return;
	}
	entry = (CacheEntry *) node->val.v;
	entry->refrence--;

	/* if we reduce the node to 0 refrences, put it in the cache */
	if (entry->refrence == 0)
	{
		dll_append (this->free, new_jval_v (entry));
		entry->free = dll_last (this->free);
	}

	/* if the cache was overfull, empty it as much as possible */
	while (this->size > this->max && !jrb_empty (this->free))
	{
		dllnode = dll_first (this->free);
		tmp = (CacheEntry *) dllnode->val.v;
		dll_delete_node (dllnode);
		jrb_delete_node (tmp->node);
		cacheentry_free (tmp);
		this->size--;
	}
	pthread_mutex_unlock (&this->lock);
}


/** host_update_stat:
 ** updates the success rate to the host based on the SUCCESS_WINDOW average
 */
void host_update_stat (ChimeraHost * host, int success)
{

	int i;
	float total = 0;

	host->success_win[host->success_win_index++ % SUCCESS_WINDOW] = success;
	host->success_avg = 0;

	// printf("SUCCESS_WIN[");
	for (i = 0; i < SUCCESS_WINDOW; i++)
	{
		//  printf("%i ",host->success_win[i]);
		total += host->success_win[i];
		//   host->success_avg = host->success_win[i]/SUCCESS_WINDOW;
	}
	// printf("]   ");
	host->success_avg = total / SUCCESS_WINDOW;
	//  printf("Total: %f, avg: %f\n",total,host->success_avg);

}

/** host_init:
 ** initialize a host struct with a #size# element cache.
 */
HostGlobal::HostGlobal(int size)
{
	this->hosts = make_jrb ();
	this->free = new_dllist ();
	this->size = 0;
	this->max = size;
}
