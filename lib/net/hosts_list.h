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

#ifndef _HOSTS_LIST_H_
#define _HOSTS_LIST_H_

#include <exception>
#include <map>
#include <netinet/in.h>

#include <util/key.h>
#include "host.h"

class HostsList : public Mutex
{
	typedef std::map<pf_addr, Host> HostMap;
	HostMap hosts;
	size_t max;

public:

	class CantResolvHostname : public std::exception {};

	/** Initialize a host struct with a size element cache.
	 */
	HostsList(size_t size);

	/** Gets a host entry for the given host, getting it from the cache if
	 ** possible, or alocates memory for it
	 */
	Host GetHost(std::string hn, uint16_t port);

	/** Get an host from a pf_addr.
	 *
	 * @param address  the pf_addr object which describes host
	 * @return  the Host object.
	 */
	Host GetHost(const pf_addr& address);

	/** Decodes a string into a chimera host structure. This acts as a
	 ** GetHost.
	 */
	Host DecodeHost (std::string s);

	/** @return the ip address of hostname
	 */
	pf_addr MakeAddr(std::string hostname, uint16_t port) const;
};

#endif /* _HOSTS_LIST_H */
