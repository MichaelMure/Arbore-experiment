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

#include <netinet/in.h>
#include "jrb.h"
#include "dllist.h"

class HostsList : public Mutex
{
	JRB hosts;
	Dllist free_list;
	size_t size;
	size_t max;

	in_addr_t network_address(const char* hostname) const;

public:

	/** host_init:
	 ** initialize a host struct with a #size# element cache.
	 */
	HostGlobal(size_t size);

	/** host_get:
	 ** gets a host entry for the given host, getting it from the cache if
	 ** possible, or alocates memory for it
	 */
	ChimeraHost* GetHost(const char* hn, int port);

	/** host_decode:
	 ** decodes a string into a chimera host structure. This acts as a
	 ** host_get, and should be followed eventually by a host_release.
	 */
	ChimeraHost* DecodeHost (const char *s);

	/** host_release:
	 ** releases a host from the cache, declaring that the memory could be
	 ** freed any time. returns NULL if the entry is deleted, otherwise it
	 ** returns #host#
	 */
	void ReleaseHost(ChimeraHost* host);
};

#endif /* _HOSTS_LIST_H */
