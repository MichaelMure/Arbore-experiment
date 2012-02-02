/*
 * Copyright(C) 2012 Michael Muré <batolettre@gmail.com
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

#include "dht.h"

#include <net/packet.h>

#include "messages.h"

DHT::DHT(uint16_t port, const Key key)
	: chimera_(new Chimera(this, port, key))
{
}

bool DHT::Publish(Key& id, std::string string) const
{
	return false;
}

bool DHT::Publish(Key& id, DataString& strings) const
{
	return false;
}

bool DHT::Publish(Key& id, Key& key) const
{
	return false;
}

bool DHT::Publish(Key& id, DataKey& keys) const
{
	return false;
}

void DHT::HandleMessage(const Host& sender, const Packet& pckt)
{
	PacketHandlerBase *handler_base = pckt.GetPacketType().GetHandler();
	if(handler_base->getType() == HANDLER_TYPE_DHT)
	{
		DHTMessage *handler = (DHTMessage*) handler_base;
		handler->Handle(*this, sender, pckt);
	}
}

Chimera* DHT::GetChimera() const
{
	return chimera_;
}
