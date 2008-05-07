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
 * This product includes cryptographic software written by Eric Younganus
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * $Id$
 */

#ifndef TOOLS_H
#define TOOLS_H

#include <string>
#include <sstream>
#include "pf_types.h"

bool is_ip(const char* ip);
std::string stringtok(std::string &in, const char * const delimiters = " \t\n");
std::string pf_addr2string(const pf_addr addr);

// Endian conversion functions
pf_addr nto_pf_addr(pf_addr addr);
pf_addr pf_addr_ton(pf_addr addr);

uint64_t htonll(uint64_t number);
uint64_t ntohll(uint64_t number);

template<typename T>
T StrToTyp(const std::string & Str)
{
	T Dest;
	std::istringstream iss( Str );
	iss >> Dest;
	return Dest;
}

template<typename T>
std::string TypToStr( const T & Value )
{
	std::ostringstream oss;
	oss << Value;
	return oss.str();
}

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif						  /* TOOLS_H */
