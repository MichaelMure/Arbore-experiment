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

#include <arpa/inet.h>
#include <netdb.h>
#include "hosts_list.h"
#include "host.h"
#include "tools.h"

/** host_init:
 ** initialize a host struct with a #size# element cache.
 */
HostsList::HostsList(size_t size)
{
	this->hosts = make_jrb ();
	this->free_list = new_dllist ();
	this->size = 0;
	this->max = size;
}

/** host_decode:
 ** decodes a string into a chimera host structure. This acts as a
 ** host_get, and should be followed eventually by a host_release.
 */
Host HostsList::DecodeHost(std::string hostname)
{
	std::string key, name;
	uint16_t port = 0;
	Key k;

	key = stringtok(hostname, ":");;
	name = stringtok(hostname, ":");
	port = StrToTyp<uint16_t>(hostname);

	Host host = GetHost(name, port);

	k = key;

	if (host.GetKey() == 0)
		host.SetKey(k);

	return (host);

}

/** network_address:
 ** returns the ip address of the #hostname#
 */
in_addr_t HostsList::network_address (const char *hostname) const
{

    int is_addr;
    struct hostent *he;
    in_addr_t addr;
    in_addr_t local;
    int i;

    /* apparently gethostbyname does not portably recognize ip addys */

#ifdef SunOS
    is_addr = inet_addr (hostname);
    if (is_addr == -1)
	is_addr = 0;
    else
	{
	    memcpy (&addr, (struct in_addr *) &is_addr, sizeof (addr));
	    is_addr = inet_addr ("127.0.0.1");
	    memcpy (&local, (struct in_addr *) &is_addr, sizeof (addr));
	    is_addr = 1;
	}
#else
    is_addr = inet_aton (hostname, (struct in_addr *) &addr);
    inet_aton ("127.0.0.1", (struct in_addr *) &local);
#endif

    if (is_addr)
	he = gethostbyaddr ((char *) &addr, sizeof (addr), AF_INET);
    else
	he = gethostbyname (hostname);

    if (he == NULL)
	    return (0);

    /* make sure the machine is not returning localhost */

    /* TODO: check why he does that, because I don't understand... */
    addr = *(in_addr_t *) he->h_addr_list[0];
    for (i = 1; he->h_addr_list[i] != NULL && addr == local; i++)
	addr = *(in_addr_t *) he->h_addr_list[i];

    return (addr);

}

/** host_get:
 ** gets a host entry for the given host, getting it from the cache if
 ** possible, or alocates memory for it
 */
Host HostsList::GetHost(std::string hostname, int port)
{
	JRB node;
	Dllist dllnode;
	in_addr_t address;
	CacheEntry *tmp, *entry;
	unsigned char *ip;
	char id[256];
	BlockLockMutex lock(this);

	/* create an id of the form ip:port */
	memset (id, 0, 256);
	address = network_address (hostname);
	ip = (unsigned char *) &address;
	snprintf(id, sizeof id - 1, "%d.%d.%d.%d:%d", ip[0], ip[1], ip[2], ip[3], port);

	node = jrb_find_str (this->hosts, id);

	/* if the node is not in the cache, create an entry and allocate a host */
	if (node == NULL)
	{
		ChimeraHost* host = new ChimeraHost(hostname, port, address);
		entry = new CacheEntry(host);

		jrb_insert_str (this->hosts, strdup (id), new_jval_v (entry));

		entry->SetNode(jrb_find_str (this->hosts, id));
		this->size++;
	}

	/* otherwise, increase the refrence count */
	else
	{
		entry = (CacheEntry *) node->val.v;
		/* if it was in the free list, remove it */
		if (entry->GetRefrence() == 0)
		{
			dll_delete_node (entry->GetFreeList());
		}
		entry->RefrenceUp();
	}

	/* if the cache was overfull, empty it as much as possible */
	while (this->size > this->max && !jrb_empty (this->free_list))
	{
		dllnode = dll_first (this->free_list);
		tmp = (CacheEntry *) dllnode->val.v;
		dll_delete_node (dllnode);
		jrb_delete_node (tmp->GetNode());
		delete tmp;
		this->size--;
	}
	return (entry->GetHost());
}

/** host_release:
 ** releases a host from the cache, declaring that the memory could be
 ** freed any time. returns NULL if the entry is deleted, otherwise it
 ** returns #host#
 */
void HostsList::ReleaseHost(ChimeraHost * host)
{

	JRB node;
	Dllist dllnode;
	CacheEntry *entry, *tmp;
	unsigned char *ip;
	unsigned long ip_long;
	char id[256];
	int i;

	/* create an id of the form ip:port */
	memset (id, 0, 256);
	ip_long = host->GetAddress();
	ip = (unsigned char *) &ip_long;
	for (i = 0; i < 4; i++)
		sprintf (id + strlen (id), "%s%d", (i == 0) ? ("") : ("."),
		                                   (int) ip[i]);
	sprintf (id + strlen (id), ":%d", host->GetPort());

	BlockLockMutex lock(this);
	node = jrb_find_str (this->hosts, id);
	if (node == NULL)
		return;

	entry = (CacheEntry *) node->val.v;
	entry->RefrenceDown();

	/* if we reduce the node to 0 refrences, put it in the cache */
	if (entry->GetRefrence() == 0)
	{
		dll_append (this->free_list, new_jval_v (entry));
		entry->SetFreeList(dll_last (this->free_list));
	}

	/* if the cache was overfull, empty it as much as possible */
	while (this->size > this->max && !jrb_empty (this->free_list))
	{
		dllnode = dll_first (this->free_list);
		tmp = (CacheEntry *) dllnode->val.v;
		dll_delete_node (dllnode);
		jrb_delete_node (tmp->GetNode());
		delete tmp;
		this->size--;
	}
}

