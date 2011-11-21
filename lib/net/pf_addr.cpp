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

#include <util/time.h>

#include "pf_addr.h"

pf_addr::pf_addr()
{
}

pf_addr::pf_addr(const std::string hostname, const uint16_t port)
	: key_(Key())
{
	struct addrinfo *result;
	int error;

	double time_ref = time::dtime();

	error = getaddrinfo(hostname.c_str(), NULL, NULL, &result);
	if (error != 0)
	{
		pf_log[W_ERR] << "error in getaddrinfo: " << gai_strerror(error);
		throw std::exception();
	}

	if (result == NULL)
		throw std::exception();

	pf_log[W_DEBUG] << "Resolved " << hostname << " in " << (uint32_t) (time::dtime() - time_ref) << " milliseconds.";

	addr_ = *(result->ai_addr);

	switch(addr_.sa_family)
	{
		case AF_INET: /* IPV4 */
		{
			struct sockaddr_in *addr = (struct sockaddr_in *) &addr_;
			addr->sin_port = htons(port);
			break;
		}
		case AF_INET6: /* IPV6 */
		{
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *) &addr_;
			addr->sin6_port = htons(port);
			break;
		}
		default:
			pf_log[W_ERR] << "Address format not recognized, new protocol ?";
			throw InvalidAddr();
	}
	freeaddrinfo(result);
}

pf_addr::pf_addr(const sockaddr addr, const Key key)
	: addr_(addr), key_(key)
{
}

pf_addr::pf_addr(const in_addr address_v4, const in_port_t port, const Key key)
	: key_(key)
{
	sockaddr_in *addr = (sockaddr_in *) &addr_;
	addr->sin_family = AF_INET;
	addr->sin_addr = address_v4;
	addr->sin_port = port;
}

pf_addr::pf_addr(const in6_addr address_v6, const in_port_t port, const Key key)
	: key_(key)
{
	sockaddr_in6 *addr = (sockaddr_in6 *) &addr_;
	addr->sin6_family = AF_INET6;
	addr->sin6_addr = address_v6;
	addr->sin6_port = port;
}

pf_addr::pf_addr(const char* p)
{
	/* read family */
	addr_.sa_family = ntohs(*(uint16_t*)p);
	p += sizeof(addr_.sa_family);

	/* read address */
	memcpy(&addr_.sa_data, p, sizeof(addr_.sa_data));
	p += sizeof(addr_.sa_data);

	/* read and create a Key */
	key_=Key(p);
}

void pf_addr::dump(char* p) const
{
	/* dump family */
	uint16_t nfamily = htons(addr_.sa_family);
	memcpy(p, &nfamily, sizeof(nfamily));
	p += sizeof(nfamily);

	/* dump address */
	memcpy(p, &addr_.sa_data, sizeof(addr_.sa_data));
	p += sizeof(addr_.sa_data);

	/* dump key */
	key_.dump(p);
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

uint16_t pf_addr::GetPort() const
{
	switch(addr_.sa_family)
	{
		case AF_INET: /* IPV4 */
		{
			struct sockaddr_in *addr = (struct sockaddr_in *) &addr_;
			return ntohs(addr->sin_port);
		}
		case AF_INET6: /* IPV6 */
		{
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *) &addr_;
			return ntohs(addr->sin6_port);
		}
		default:
			throw InvalidAddr();
	}
}

void pf_addr::SetPort(const uint16_t port)
{
	switch(addr_.sa_family)
	{
		case AF_INET: /* IPV4 */
		{
			struct sockaddr_in *addr = (struct sockaddr_in *) &addr_;
			addr->sin_port = htons(port);
			break;
		}
		case AF_INET6: /* IPV6 */
		{
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *) &addr_;
			addr->sin6_port = htons(port);
			break;
		}
		default:
			throw InvalidAddr();
	}
}

const Key& pf_addr::GetKey() const
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
			throw InvalidAddr();
	}

	if(res == NULL)
		pf_log[W_ERR] << "Address too long";

	std::string ret = key_.GetStr() + ":" + ip + ":" + port;

	return ret;
}
