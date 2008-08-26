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

#include "pf_addr.h"
#include "tools.h"

pf_addr::pf_addr() : port(0), id(0)
{
	ip[0] = 0;
	ip[1] = 0;
	ip[2] = 0;
	ip[3] = 0;
}

bool pf_addr::operator ==(const pf_addr &other) const
{
	return ip[0] == other.ip[0]
		&& ip[1] == other.ip[1]
		&& ip[2] == other.ip[2]
		&& ip[3] == other.ip[3]
		&& port == other.port
		&& id == other.id;
}

std::string pf_addr::str() const
{
	std::string ret = key + ":";
	if(ip[0] == 0 &&
		ip[1] == 0 &&
		ip[2] == 0)
	{
		char str[16];
		unsigned char* nbr = (unsigned char*) &(addr.ip[3]);
		snprintf(str, 16, "%i.%i.%i.%i", nbr[0],
			nbr[1],
			nbr[2],
			nbr[3]);
		ret = std::string(str);
	}
	else
	{
		/* TODO: ipv6 */
	}
	ret += ":" + TypToStr(addr.port);
	return ret;
}

#ifdef WORDS_BIGENDIAN
pf_addr pf_addr::nto_pf_addr() const
{
	pf_addr addr;
	addr.ip[0] = ntohl(ip[0]);
	addr.ip[1] = ntohl(ip[1]);
	addr.ip[2] = ntohl(ip[2]);
	addr.ip[3] = ntohl(ip[3]);
	addr.port = ntohs(port);
	addr.key = key.nto_key();

	return addr;
}

pf_addr pf_addr::pf_addr_ton() const
{
	pf_addr addr;
	addr.ip[0] = htonl(ip[0]);
	addr.ip[1] = htonl(ip[1]);
	addr.ip[2] = htonl(ip[2]);
	addr.ip[3] = htonl(ip[3]);
	addr.port = htons(port);
	addr.key = key.key_ton();

	return addr;
}
#else
pf_addr pf_addr::nto_pf_addr() const
{
	return *this;
}

pf_addr pf_addr:pf_addr_ton() const
{
	return *this;
}
#endif
