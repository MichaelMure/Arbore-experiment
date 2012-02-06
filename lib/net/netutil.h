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

#ifndef NETUTIL_H
#define NETUTIL_H

#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#include <util/tools.h>

class Netutil
{
public:
	// Writing to buffer functions
	static void dump(uint32_t nbr, char* buff);
	static void dump(uint64_t nbr, char* buff);
	static void dump(const std::string& str, char* buff);

	static size_t getSerialisedSize(uint32_t nbr);
	static size_t getSerialisedSize(uint64_t nbr);
	static size_t getSerialisedSize(std::string& str);

	// Reading from buffer functions
	static uint32_t ReadInt32(char* buff);
	static uint64_t ReadInt64(char* buff);
	static std::string ReadStr(char* buff);
};

#endif
