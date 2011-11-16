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
public:

	/** Initialize a host struct with a size element cache.
	 */
	HostsList(const size_t size);

	/** Gets a host entry for the given host, getting it from the cache if
	 * possible, or alocates memory for it.
	 * @param hostname the hostname of the host (can be ipv4/6 address, or a hostname to resolve)
	 * @param port the port of the host
	 */
	Host GetHost(const std::string hostname, const uint16_t port = pf_addr::DEFAULT_PORT);

	/** Get an host from a pf_addr.
	 *
	 * @param address  the pf_addr object which describes host
	 * @return  the Host object.
	 */
	Host GetHost(const pf_addr& address);

private:
	typedef std::map<pf_addr, Host> HostMap;
	typedef HostMap::value_type value_type;

	HostMap hosts_;
	size_t max_; /* not used right now */
};

#endif /* _HOSTS_LIST_H */
