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

pf_addr::pf_addr() throw()
	: port(0)
{
	ip[0] = 0;
	ip[1] = 0;
	ip[2] = 0;
	ip[3] = 0;
}

pf_addr::pf_addr(in_addr_t address, uint16_t _port, Key _key) throw()
	: port(_port),
	  key(_key)
{
	ip[0] = 0;
	ip[1] = 0;
	ip[2] = 0;
	ip[3] = address;
}

bool pf_addr::operator ==(const pf_addr &other) const
{
	return (ip[0] == other.ip[0]
		&& ip[1] == other.ip[1]
		&& ip[2] == other.ip[2]
		&& ip[3] == other.ip[3]
		&& port == other.port
		&& (!key || !other.key || key == other.key));
}

bool pf_addr::operator<(const pf_addr &other) const
{
	for(size_t i = 0; i < 4; ++i)
		if(ip[i] < other.ip[i])
			return true;

	if(port < other.port)
		return true;

	if(key && other.key)
		return key < other.key;
	return false;
}

std::string pf_addr::str() const
{
	std::string ret = key.str() + ":";
	if(ip[0] == 0 &&
		ip[1] == 0 &&
		ip[2] == 0)
	{
		char str[16];
		unsigned char* nbr = (unsigned char*) &(ip[3]);
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
	ret += ":" + TypToStr(port);
	return ret;
}

pf_addr::pf_addr(const char* p) throw()
{
	for(size_t i = 0; i < ip_t_len; ++i)
	{
		ip[i] = ntohl(*(uint32_t*)p);
		p += sizeof(ip[i]);
	}

	port = ntohs(*(uint16_t*)p);
	p += sizeof(port);

	uint32_t array[Key::nlen];
	for(size_t i = 0; i < Key::nlen; ++i)
	{
		array[i] = ntohl(*(uint32_t*)p);
		p += sizeof(array[i]);
	}
	key = Key(array);
}

void pf_addr::dump(char* p)
{
	for(size_t i = 0; i < ip_t_len; ++i)
	{
		uint32_t nbr = htonl(ip[i]);
		memcpy(p, &nbr, sizeof(nbr));
		p += sizeof(nbr);
	}

	uint16_t nport = htons(port);
	memcpy(p, &nport, sizeof(nport));
	p += sizeof(nport);

	const uint32_t* array = key.GetArray();
	for(size_t i = 0; i < Key::nlen; ++i)
	{
		uint32_t nbr = htonl(array[i]);
		memcpy(p, &nbr, sizeof(nbr));
		p += sizeof(nbr);
	}
}
