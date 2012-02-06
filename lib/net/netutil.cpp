/*
 * Copyright(C) 2012 Benoît Saccomano
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

#include "netutil.h"
#include <string>

void Netutil::dump(const uint32_t nbr, char* buff)
{
	uint32_t nbr_net = htonl(nbr);
	memcpy(buff, &nbr_net, sizeof(nbr_net));
}

size_t Netutil::getSerialisedSize(const uint32_t nbr)
{
	return sizeof(nbr);
}

void Netutil::dump(const uint64_t nbr, char* buff)
{
	uint64_t nbr_net = htonll(nbr);
	memcpy(buff, &nbr_net, sizeof(nbr_net));
}

size_t Netutil::getSerialisedSize(const uint64_t nbr)
{
	return sizeof(nbr);
}

void Netutil::dump(const std::string& str, char* buff)
{
	uint32_t str_len = (uint32_t)str.size();
	dump(str_len, buff);
	memcpy(buff + getSerialisedSize(str_len), str.c_str(), str.size());
}

size_t Netutil::getSerialisedSize(const std::string& str)
{
	return str.size() + sizeof(uint32_t);
}

uint32_t Netutil::ReadInt32(char* buff)
{
	uint32_t val = ntohl(*(uint32_t*)buff);
	return val;
}

uint64_t Netutil::ReadInt64(char* buff)
{
	uint64_t val = ntohll(*(uint64_t*)buff);
	return val;
}

std::string Netutil::ReadStr(char* buff)
{
	uint32_t str_size = ReadInt32(buff);
	buff += getSerialisedSize(str_size);

	char* str = new char [str_size+1];
	memcpy(str, buff, str_size);
	str[str_size] = '\0';
	return std::string(str);
}
