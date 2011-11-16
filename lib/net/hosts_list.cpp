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



#include <util/tools.h>
#include "host.h"
#include "hosts_list.h"

HostsList::HostsList(size_t _size)
	: Mutex(RECURSIVE_MUTEX),
	  max(_size)
{
}

Host HostsList::GetHost(std::string hostname, uint16_t port)
{
	BlockLockMutex lock(this);

	/* create an id of the form ip:port */
	pf_addr address = pf_addr(hostname, port);

	return GetHost(address);
}

Host HostsList::GetHost(const pf_addr& address)
{
	BlockLockMutex lock(this);
	HostMap::iterator it = hosts.find(address);

	pf_log[W_DEBUG] << "Try to get " << address;

	/* if the node is not in the cache, create an entry and allocate a host */
	if (it == hosts.end())
	{
		it = hosts.insert(std::pair<pf_addr, Host>(address, Host(this, address))).first;
		pf_log[W_DEBUG] << "added";
	}

	pf_log[W_DEBUG] << "host entries:";
	for(HostMap::iterator free_it = hosts.begin();
	    /*hosts.size() > max && */free_it != hosts.end();
	    ++free_it)
	{
		pf_log[W_DEBUG] << "  " << free_it->second;

/*		if(free_it->second.GetReference() == 1 && it != free_it)
			hosts.erase(free_it);*/
	}

	pf_log[W_DEBUG] << "returned " << it->second;

	return it->second;
}
