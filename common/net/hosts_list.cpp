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
HostsList::HostsList(size_t _size)
	: max(_size)
{

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

	if (!host.GetKey())
		host.SetKey(k);

	return (host);

}

/** network_address:
 ** returns the ip address of the #hostname#
 */
pf_addr HostsList::MakeAddr(std::string hostname, uint16_t port) const
{
	int is_addr;
	struct hostent *he;
	in_addr_t addr;
	in_addr_t local;
	int i;

	/* apparently gethostbyname does not portably recognize ip addys */
#ifdef SunOS
	is_addr = inet_addr (hostname.c_str());
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
	is_addr = inet_aton (hostname.c_str(), (struct in_addr *) &addr);
	inet_aton ("127.0.0.1", (struct in_addr *) &local);
#endif

	if (is_addr)
		he = gethostbyaddr ((char *) &addr, sizeof (addr), AF_INET);
	else
		he = gethostbyname (hostname.c_str());

	if (he == NULL)
		throw CantResolvHostname();

	/* TODO: check why he does that, because I don't understand... --romain */
	/* make sure the machine is not returning localhost */
	addr = *(in_addr_t *) he->h_addr_list[0];
	for (i = 1; he->h_addr_list[i] != NULL && addr == local; i++)
		addr = *(in_addr_t *) he->h_addr_list[i];

	return pf_addr(addr, port);
}

/** host_get:
 ** gets a host entry for the given host, getting it from the cache if
 ** possible, or alocates memory for it
 */
Host HostsList::GetHost(std::string hostname, uint16_t port)
{
	pf_addr address;
	BlockLockMutex lock(this);

	/* create an id of the form ip:port */
	address = MakeAddr(hostname, port);

	HostMap::iterator it = hosts.find(address);

	/* if the node is not in the cache, create an entry and allocate a host */
	if (it == hosts.end())
		it = hosts.insert(std::pair<pf_addr, Host>(address, Host(this, address))).first;

	for(HostMap::iterator free_it = hosts.begin();
	    hosts.size() > max && it != hosts.end();
	    ++it)
	{
		if(it->second.GetReference() == 1)
			hosts.erase(it);
	}

	return it->second;
}
