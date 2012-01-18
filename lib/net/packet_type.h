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

#include <stdint.h>
#include <string>
#include <vector>

#include "packet_arg.h"

class PacketHandlerBase;


/** This is list of all availables type messages.
 *
 * To prevent conflicts in message types numbers, we declare
 * this list which contains all of the type and upcalls messages.
 */
enum
{
	CHIMERA_JOIN        = 1,
	CHIMERA_JOIN_ACK    = 2,
	CHIMERA_UPDATE      = 3,
	CHIMERA_PIGGY       = 4,
	CHIMERA_JOIN_NACK   = 5,
	CIHMERA_PING        = 7,
	CHIMERA_RESERVED1   = 8,
	CHIMERA_RESERVED2   = 9,
	CHIMERA_RESERVED3   = 10,
	CHIMERA_RESERVED4   = 11,

	DHT_PUBLISH         = 12,
	DHT_UNPUBLISH       = 13,
	DHT_GET             = 14,
	DHT_GET_ACK         = 15,
	DHT_GET_NACK        = 16,
	DHT_RESERVED1       = 17,
	DHT_RESERVED2       = 18,
	DHT_RESERVED3       = 19,
	DHT_RESERVED4       = 20
};

class PacketType : public std::vector<PacketArgType>
{
	uint32_t type;
	std::string name;
	PacketHandlerBase* handler;
	uint32_t def_flags;

public:

	/** PacketType constructor.
	 *
	 * @param type  this is the type number.
	 * @param handler  the handler object.
	 * @param def_flags  default flags for packets created by this packet type.
	 * @param name  name of message, for debug information.
	 * @param ...  put here a list of PacketArgType finished by a T_END.
	 *
	 * For example:
	 * @code
	 * PacketType(10, MyHandler, Packet::MUSTROUTE|Packet::REQUESTACK, "BLAH", T_STR, T_UINT32, T_CHUNK, T_END);
	 * @endcode
	 */
	PacketType(uint32_t type, PacketHandlerBase* handler, uint32_t def_flags, std::string name, ...);

	/** Destructor.
	 *
	 * Delete the handler object.
	 */
	~PacketType();

	PacketType& operator=(const PacketType& pckt_type);
	PacketType(const PacketType& pckt_type);

	uint32_t GetType() const { return type; }
	std::string GetName() const { return name; }
	PacketHandlerBase* GetHandler() const { return handler; }
	uint32_t GetDefFlags() const { return def_flags; }
};

#endif /* PACKET_TYPE_H */
