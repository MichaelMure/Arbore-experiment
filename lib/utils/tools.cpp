/*
 * Copyright(C) 2008 Laurent Defert, Romain Bignon
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
 */

#include <arpa/inet.h>
#include <string>
#include <stdlib.h>
#include <stdint.h>
#include "tools.h"
#include "pf_types.h"

bool is_ip(const char *ip)
{
	char *ptr = NULL;
	int i = 0, d = 0;

	for(; i < 4; ++i)			  /* 4 dots expected (IPv4) */
	{					  /* Note about strtol: stores in endptr either NULL or '\0' if conversion is complete */
		if(!isdigit((unsigned char) *ip)  /* most current case (not ip, letter host) */
						  /* ok, valid number? */
			|| (d = (int)strtol(ip, &ptr, 10)) < 0 || d > 255
			|| (ptr && *ptr != 0 && (*ptr != '.' || 3 == i) && ptr != ip)) return false;
		if(ptr) ip = ptr + 1, ptr = NULL;  /* jump the dot */
	}
	return true;
}

std::string stringtok(std::string &in, const char * const delimiters)
{
	std::string::size_type i = 0;
	std::string s;

	// eat leading whitespace
	i = in.find_first_not_of (delimiters, i);

	// find the end of the token
	std::string::size_type j = in.find_first_of (delimiters, i);

	if (j == std::string::npos)
	{
		if(i == std::string::npos)
			s = "";
		else
			s = in.substr(i);
		in = "";
		return s;			  // nothing left but white space
	}

	// push token
	s = in.substr(i, j-i);
	in = in.substr(j+1);

	return s;
}

#ifdef WORDS_BIGENDIAN
uint64_t htonll(uint64_t number)
{
	return number;
}

uint64_t ntohll(uint64_t number)
{
	return number;
}

#else						  /* WORDS_BIGENDIAN */

uint64_t htonll(uint64_t number)
{
	return ( htonl((uint32_t) (number >> 32) & 0xFFFFFFFF) |
		((uint64_t) (htonl((uint32_t)number & 0xFFFFFFFF))  << 32));
}

uint64_t ntohll(uint64_t number)
{
	return ( htonl((uint32_t) (number >> 32) & 0xFFFFFFFF) |
		((uint64_t) (htonl((uint32_t)number & 0xFFFFFFFF))  << 32));
}

#endif						  /* WORDS_BIGENDIAN */
