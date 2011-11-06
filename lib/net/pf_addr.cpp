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
#include <algorithm>
#include <cctype>

#include "pf_addr.h"

pf_addr::pf_addr()
{
}

pf_addr::pf_addr(const std::string str)
{
	bool success = false;
	int res;
	int colon_nbr = std::count(str.begin(), str.end(), ':');
	int alpha_nbr = std::count_if(str.begin(), str.end(), isalpha);

	if (colon_nbr >= 2) /* IPV6 */
	{
		sockaddr_in6 *addr = (sockaddr_in6 *) &addr_;
		addr->sin6_family = AF_INET6;

		if(str.find("[")) /* port */
		{
			int pos = str.find_last_of(":");
			addr->sin6_port = htons(StrToTyp<in_port_t>(str.substr(pos+1)));
			res = inet_pton(AF_INET6, str.substr(0, pos).c_str(), &(addr->sin6_addr));
		}
		else
		{
			addr->sin6_port = htons(DEFAULT_PORT);
			res = inet_pton(AF_INET6, str.c_str(), &(addr->sin6_addr));
		}
	}
	else if(alpha_nbr > 0) /* Hostname */
	{
		if(colon_nbr == 1) /* port */
		{
			/* TODO */
			throw CantParse();
		}
		else
		{
			/* TODO */
			throw CantParse();
		}
	}
	else /* IPV4 */
	{
		sockaddr_in *addr = (sockaddr_in *) &addr_;
		addr->sin_family = AF_INET;

		if(colon_nbr == 1) /* port */
		{
			int pos = str.find_last_of(":");
			addr->sin_port = htons(StrToTyp<in_port_t>(str.substr(pos+1)));
			res = inet_pton(AF_INET, str.substr(0, pos).c_str(), &(addr->sin_addr));
		}
		else
		{
			addr->sin_port = htons(DEFAULT_PORT);
			res = inet_pton(AF_INET, str.c_str(), &(addr->sin_addr));
		}
	}

	if(res == 1)
		success = true;

	if(!success)
		throw CantParse();
}

pf_addr::pf_addr(sockaddr addr, Key key)
	: addr_(addr), key_(key)
{
}

pf_addr::pf_addr(in_addr address_v4, in_port_t port, Key key)
	: key_(key)
{
	sockaddr_in *addr = (sockaddr_in *) &addr_;
	addr->sin_family = AF_INET;
	addr->sin_addr = address_v4;
	addr->sin_port = port;
}

pf_addr::pf_addr(in6_addr address_v6, in_port_t port, Key key)
	: key_(key)
{
	sockaddr_in6 *addr = (sockaddr_in6 *) &addr_;
	addr->sin6_family = AF_INET6;
	addr->sin6_addr = address_v6;
	addr->sin6_port = port;
}

pf_addr::pf_addr(in_addr_t address_v4, uint16_t port, Key key)
//	: key_(key)
{
/*	sockaddr_in *sock = (sockaddr_in*) &addr_;

	sock->sin_family = AF_INET;
	inet_aton(address_v4, sock->sin_addr->s_addr)
	sock->sin_port = htons(port);*/
}

pf_addr::pf_addr(const char* p)
{
/*	for(size_t i = 0; i < ip_t_len; ++i)
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
	key = Key(array);*/
}

pf_addr::pf_addr(std::string hostname, uint16_t port)
//	: port(port), key(Key())
{

}



bool pf_addr::operator ==(const pf_addr &other) const
{
	if(!key_ || !other.key_)
		return false;

	if(key_ != other.key_)
		return false;

	if(addr_.sa_family != other.addr_.sa_family)
		return false;

	switch(addr_.sa_family)
	{
		case AF_INET: /* IPV4 */
		{
			struct sockaddr_in *addr = (struct sockaddr_in *) &addr_;
			struct sockaddr_in *other_addr = (struct sockaddr_in *) &other.addr_;
			return (addr->sin_port == other_addr->sin_port
							&& memcmp(&addr->sin_addr, &other_addr->sin_addr, 4) == 0);
		}
		case AF_INET6: /* IPV6 */
		{
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *) &addr_;
			struct sockaddr_in6 *other_addr = (struct sockaddr_in6 *) &other.addr_;
			return (addr->sin6_port == other_addr->sin6_port
							&& memcmp(&addr->sin6_addr, &other_addr->sin6_addr, 16) == 0);
		}
		default:
			return false;
	}
}

bool pf_addr::operator<(const pf_addr &other) const
{
	if(addr_.sa_family != other.addr_.sa_family)
		return addr_.sa_family < other.addr_.sa_family;

	for(uint16_t i = 0; i < 14; ++i)
		if(addr_.sa_data[i] != other.addr_.sa_data[i])
			return addr_.sa_data[i] < other.addr_.sa_data[i];

	if(key_ && other.key_)
		return key_ < other.key_;

	return false;
}

bool pf_addr::IsIPV4() const
{
	return (addr_.sa_family == AF_INET);
}

bool pf_addr::IsIPV6() const
{
	return (addr_.sa_family == AF_INET6);
}

Key pf_addr::GetKey() const
{
	return key_;
}

void pf_addr::SetKey(const Key& key)
{
	key_ = key;
}

sockaddr pf_addr::GetSockAddr() const
{
	return addr_;
}

std::string pf_addr::GetStr() const
{
	std::string ip, port;
	const char *res;

	switch(addr_.sa_family)
	{
		case AF_INET:
		{
			struct sockaddr_in *addr = (struct sockaddr_in *) &addr_;
			char str[INET_ADDRSTRLEN];
			res = inet_ntop(AF_INET, &(addr->sin_addr), str, INET_ADDRSTRLEN);
			ip = std::string(str);
			port = TypToStr(ntohs(addr->sin_port));
			break;
		}
		case AF_INET6:
		{
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *) &addr_;
			char str[INET6_ADDRSTRLEN];
			res = inet_ntop(AF_INET6, &(addr->sin6_addr), str, INET6_ADDRSTRLEN);
			ip = "[" + std::string(str) + "]";
			port = TypToStr(ntohs(addr->sin6_port));
			break;
		}
		default:
			pf_log[W_ERR] << "Address format not recognized, new protocol ?";
			return NULL;
	}

	if(res == NULL)
		pf_log[W_ERR] << "Address too long";

	std::string ret = key_.GetStr() + ":" + ip + ":" + port;

	return ret;
}

void pf_addr::dump(char* p)
{
	/* dump family */
	uint16_t nfamily = htons(addr_.sa_family);
	memcpy(p, &nfamily, sizeof(nfamily));
	p += sizeof(nfamily);

	/* dump address */
	memcpy(p, &addr_.sa_data, sizeof(addr_.sa_data));
	p += sizeof(addr_.sa_data);

	/* dump key */
	const uint32_t* array = key_.GetArray();
	for(size_t i = 0; i < Key::nlen; ++i)
	{
		uint32_t nbr = htonl(array[i]);
		memcpy(p, &nbr, sizeof(nbr));
		p += sizeof(nbr);
	}
}
