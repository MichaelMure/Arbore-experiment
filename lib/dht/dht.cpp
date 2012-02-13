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
	packet_type_list.RegisterType(DHTPublishType);
	packet_type_list.RegisterType(DHTReplicateType);
	packet_type_list.RegisterType(DHTUnpublishType);
	packet_type_list.RegisterType(DHTGetType);
	packet_type_list.RegisterType(DHTGetAckType);
	packet_type_list.RegisterType(DHTGetNAckType);
}

bool DHT::Publish(const Key& id, const std::string string) const
{
	Data *d = (Data*) new DataString(string);
	return Publish(id, d);
}

bool DHT::Publish(const Key& id, const Key& key) const
{
	Data *d = (Data*) new DataKey(key);
	return Publish(id, d);
}

bool DHT::Publish(const Key& id, Data* data) const
{
	assert(data);

	/* Store the value locally. */
	try {
		storage_->addInfo(id, data);
	}
	catch(Storage::WrongDataType e)
	{
		pf_log[W_DHT] << "Publish wrong data type in the DHT";
		return false;
	}

	/* Send a Publish packet to the owner of the key */
	Packet pckt(DHTPublishType, me_, id);
	pckt.SetArg(DHT_PUBLISH_KEY, id);
	/* TODO: This Data memory is currently leaked. */
	pckt.SetArg(DHT_PUBLISH_DATA, data);

	return chimera_->Route(pckt);
}


bool DHT::Unpublish(const Key& id, const std::string string) const
{
	Data *d = (Data*) new DataString(string);
	return Unpublish(id, d);
}

bool DHT::Unpublish(const Key& id, const Key& key) const
{
	Data *d = (Data*) new DataKey(key);
	return Unpublish(id, d);
}

bool DHT::Unpublish(const Key& id, Data* data) const
{
	/* Remove the value locally. */
	try {
		storage_->removeInfo(id, data);
	}
	catch(Storage::WrongDataType e)
	{
		pf_log[W_DHT] << "Unpublish wrong data type in the DHT";
		return false;
	}

	/* Send a Unpublish packet to the owner of the key */
	Packet pckt(DHTUnpublishType, me_, id);
	pckt.SetArg(DHT_UNPUBLISH_KEY, id);
	/* TODO: This Data memory is currently leaked. */
	pckt.SetArg(DHT_UNPUBLISH_DATA, data);

	return chimera_->Route(pckt);
}

bool DHT::RequestData(const Key& id) const
{
	if(chimera_->ClosestTo(id))
	{
		if(storage_->hasKey(id))
		{
			/* TODO: Send data to the upper layer */
			return true;
		}
		return false;
	}

	/* Send a Get packet to the owner of the key */
	Packet pckt(DHTGetType, me_, id);
	pckt.SetArg(DHT_GET_KEY, id);

	return chimera_->Route(pckt);
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

const Key& DHT::GetMe() const
{
	return me_;
}
