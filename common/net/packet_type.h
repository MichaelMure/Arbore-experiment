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

#ifndef PACKET_TYPE_H
#define PACKET_TYPE_H

#include <vector>
#include <string>
#include <stdint.h>
#include "packet_arg.h"

class PacketType : public std::vector<PacketArgType>
{
	uint32_t type;
	std::string name;

public:

	/** PacketType constructor.
	 *
	 * @param type this is the type number.
	 * @param ... put here a list of PacketArgType finished by a T_END.
	 *
	 * For example:
	 *        PacketType(10, T_STR, T_UINT32, T_CHUNK, T_END);
	 */
	PacketType(uint32_t type, std::string name, ...);

	PacketType& operator=(const PacketType& pckt_type);

	uint32_t GetType() const { return type; }
	std::string GetName() const { return name; }
};

#endif /* PACKET_TYPE_H */
