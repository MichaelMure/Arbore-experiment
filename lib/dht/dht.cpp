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

DHT::DHT(uint16_t port, const Key& me)
	: me_(me),
		chimera_(new Chimera(this, port, me)),
		storage_(new Storage())
{
}

bool DHT::Publish(const Key& id, const std::string string) const
{
	DataString d = DataString(string);
	return Publish(id, d);
}

bool DHT::Publish(const Key& id, const DataString& strings) const
{
	/* Store the value locally. */
	try {
		DataString::NameSet::const_iterator it;
		for (it = strings.begin(); it != strings.end(); it++)
			storage_->addInfo(id, *it);
	}
	catch(Storage::WrongDataType e)
	{
		pf_log[W_DEBUG] << "Received wrong data type to store in the DHT";
	}

	/* Send a Publish packet to the owner of the key */
	Packet pckt(DHTPublishType, me_, id);
	pckt.SetArg(DHT_PUBLISH_KEY, id);
	pckt.SetArg(DHT_PUBLISH_DATA, new DataString(strings));

	return chimera_->Route(pckt);
}

bool DHT::Publish(const Key& id, const Key& key) const
{
	return false;
}

bool DHT::Publish(const Key& id, const DataKey& keys) const
{
	return false;
}

bool DHT::Unpublish(const Key& id)
{
	return false;
}

bool DHT::RequestData(const Key& id)
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

Storage* DHT::GetStorage() const
{
	return storage_;
}
