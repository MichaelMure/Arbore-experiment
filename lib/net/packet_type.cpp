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

#include <cstdarg>

#include "packet_handler.h"
#include "packet_type.h"

PacketType::PacketType(uint32_t _type, PacketHandlerBase* _handler, uint32_t _def_flags, std::string _name, ...)
	: type(_type),
	  name(_name),
	  handler(_handler),
	  def_flags(_def_flags)
{
	va_list ap;
	PacketArgType t;

	va_start(ap, _name);

	for(t = (PacketArgType)va_arg(ap, uint32_t); t != T_END; t = (PacketArgType)va_arg(ap, uint32_t))
	{
		push_back(t);
	}

	va_end(ap);
}

PacketType::~PacketType()
{
	/* TODO if we delete handler one time, other copies of PacketType
	 * will have an invalid pointer. But if we don't delete handler here,
	 * it will never be free'd.
	 * So there are two solutions:
	 *   1) Never delete handler
	 *   2) Implement an ugly clone() copy method in each
	 *      PacketHandlerBase classes (see packet_arg.h)
	 */
	//delete handler;
}

PacketType& PacketType::operator=(const PacketType& pckt_type)
{
	type = pckt_type.type;
	name = pckt_type.name;
	handler = pckt_type.handler;

	for(const_iterator it = pckt_type.begin(); it != pckt_type.end(); ++it)
		push_back(*it);

	return *this;
}

PacketType::PacketType(const PacketType& pckt_type)
	: std::vector<PacketArgType>(pckt_type),
	  type(pckt_type.type),
	  name(pckt_type.name),
	  handler(pckt_type.handler)
{
}
