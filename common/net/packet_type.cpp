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
#include "packet_type.h"
#include "packet_handler.h"

PacketType::PacketType(uint32_t _type, PacketHandlerBase* _handler, std::string _name, ...)
	: type(_type),
	  name(_name),
	  handler(_handler)
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
	delete handler;
}

PacketType& PacketType::operator=(const PacketType& pckt_type)
{
	type = pckt_type.type;
	name = pckt_type.name;

	for(const_iterator it = pckt_type.begin(); it != pckt_type.end(); ++it)
		push_back(*it);

	return *this;
}
