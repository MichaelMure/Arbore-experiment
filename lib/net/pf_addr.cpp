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
#include <cstdio>
#include <string.h>

#include "pf_addr.h"

pf_addr::pf_addr()
	: port(0)
{
	Init(0, 0, 0, 0);
}

pf_addr::pf_addr(in_addr_t address_v4, uint16_t _port, Key _key)
	: port(_port),
	  key(_key)
{
	Init(0, 0, 0, address_v4);
}

pf_addr::pf_addr(const char* p)
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

pf_addr::pf_addr(std::string hostname, uint16_t port)
	: port(port), key(Key())
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

	Init(0, 0, 0, addr);
}

void pf_addr::Init(uint32_t adr0, uint32_t adr1, uint32_t adr2, uint32_t adr3)
{
	ip[0] = adr0;
	ip[1] = adr1;
	ip[2] = adr2;
	ip[3] = adr3;
}

bool pf_addr::operator ==(const pf_addr &other) const
{
	if (!key || !other.key)
		return false;

	return (ip[0] == other.ip[0]
					&& ip[1] == other.ip[1]
					&& ip[2] == other.ip[2]
					&& ip[3] == other.ip[3]
					&& port == other.port
					&& key == other.key);
}

bool pf_addr::operator<(const pf_addr &other) const
{
	for(size_t i = 0; i < 4; ++i)
		if(ip[i] != other.ip[i])
			return ip[i] < other.ip[i];

	if(port != other.port)
		return port < other.port;

	if(key && other.key)
		return key < other.key;

	return false;
}

std::string pf_addr::GetStr() const
{
	std::string ret = key.GetStr() + ":";
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
		ret += std::string(str);
	}
	else
	{
		/* TODO: ipv6 */
	}
	ret += ":" + TypToStr(port);
	return ret;
}

void pf_addr::dump(char* p) const
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

	key.dump(p);
}
