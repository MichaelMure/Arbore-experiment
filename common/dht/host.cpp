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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "host.h"

class CacheEntry
{
	ChimeraHost *host;
	int refrence;
	JRB node;
	Dllist free_list;

public:

	CacheEntry(ChimeraHost*);
	~CacheEntry();

	void SetNode(JRB n) { node = n; }
	const Dllist& GetFreeList() const { return free_list; }
	void SetFreeList(Dllist fl) { free_list = fl; }

	int GetRefrence() const { return refrence; }
	void RefrenceUp() { refrence++; }
	void RefrenceDown() { refrence--; }

	const JRB& GetNode() const { return node; }

	/* TODO: do NOT return ChimeraHost outside of the HostGlobal class,
	 * because HostEntry is a strategic resource which can't be locked with mutex.
	 */
	ChimeraHost* GetHost() const { return host; }

};

CacheEntry::CacheEntry(ChimeraHost* _host)
	: host(_host),
	  refrence(1)
{

}

CacheEntry::~CacheEntry()
{
	delete host;
}

/** host_encode:
 ** encodes the #host# into a string, putting it in #s#, which has
 ** #len# bytes in it.
 */
std::string ChimeraHost::Encode() const
{
	return key.str() + ":" + name + ":" + TypToStr(port);
}

/** host_init:
 ** initialize a host struct with a #size# element cache.
 */
HostGlobal::HostGlobal(size_t size)
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
ChimeraHost* HostGlobal::DecodeHost(const char *hostname)
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
	i = atoi(port);
	host = GetHost(name, i);

	k = key;

	if (host->GetKey() == 0)
		host->SetKey(k);

	free(s);

	return (host);

}

ChimeraHost::ChimeraHost(const std::string& _name, int _port, unsigned long _address)
	: name(_name),
	address(_address),
	failed(0),
	failuretime(0),
	port(_port),
	latency(0),
	loss(0),
	success(0),
	success_win_index(0),
	success_avg(0.5),
	key(0)
{
	size_t i;
	for (i = 0; i < SUCCESS_WINDOW / 2; i++)
		success_win[i] = 0;
	for (i = SUCCESS_WINDOW / 2; i < SUCCESS_WINDOW; i++)
		success_win[i] = 1;

}

/** network_address:
 ** returns the ip address of the #hostname#
 */
unsigned long HostGlobal::network_address (const char *hostname) const
{

    int is_addr;
    struct hostent *he;
    unsigned long addr;
    unsigned long local;
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

    addr = *(unsigned long *) he->h_addr_list[0];
    for (i = 1; he->h_addr_list[i] != NULL && addr == local; i++)
	addr = *(unsigned long *) he->h_addr_list[i];

    return (addr);

}

/** host_get:
 ** gets a host entry for the given host, getting it from the cache if
 ** possible, or alocates memory for it
 */
ChimeraHost* HostGlobal::GetHost(const char *hostname, int port)
{
	JRB node;
	Dllist dllnode;
	unsigned long address;
	CacheEntry *tmp, *entry;
	unsigned char *ip;
	char id[256];
	BlockLockMutex(this);

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
void HostGlobal::ReleaseHost(ChimeraHost * host)
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

	BlockLockMutex(this);
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


/** host_update_stat:
 ** updates the success rate to the host based on the SUCCESS_WINDOW average
 */
void ChimeraHost::UpdateStat (int success)
{

	int i;
	float total = 0;

	this->success_win[this->success_win_index++ % SUCCESS_WINDOW] = success;
	this->success_avg = 0;

	// printf("SUCCESS_WIN[");
	for (i = 0; i < SUCCESS_WINDOW; i++)
	{
		//  printf("%i ",this->success_win[i]);
		total += (float)this->success_win[i];
		//   this->success_avg = this->success_win[i]/SUCCESS_WINDOW;
	}
	// printf("]   ");
	this->success_avg = total / SUCCESS_WINDOW;
	//  printf("Total: %f, avg: %f\n",total,this->success_avg);

}

