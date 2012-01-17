/*
 * Copyright(C) 2008 Laurent Defert, Romain Bignon
 * Copyright(C) 2012 Muré Michael <batolettre@gmail.com>
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

#include "packet_handler.h"
#include <net/packet_type.h>
#include <chimera/chimera.h>
#include <net/packet.h>

void NetworkMessage::operator() (PacketTypeList& pckt_type_list, const Host& sender, const Packet& pckt)
{
	Chimera& chimera = dynamic_cast<Chimera&>(pckt_type_list);

	if(pckt.HasFlag(Packet::MUSTROUTE))
	{
		if(chimera.Route(pckt))
			return;
	}

	Handle(chimera, sender, pckt);
}
