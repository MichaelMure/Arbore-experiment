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
#include <scheduler/scheduler_queue.h>

#include "messages.h"
#include "clean_storage_job.h"

DHT::DHT(Arbore* arbore, uint16_t port, const Key& me)
	: arbore_(arbore),
		me_(me),
		chimera_(new Chimera(this, port, me)),
		storage_(new Storage())
{
	packet_type_list.RegisterType(DHTPublishType);
	packet_type_list.RegisterType(DHTRepeatPType);
	packet_type_list.RegisterType(DHTUnpublishType);
	packet_type_list.RegisterType(DHTRepeatUType);
	packet_type_list.RegisterType(DHTGetType);
	packet_type_list.RegisterType(DHTGetAckType);
	packet_type_list.RegisterType(DHTGetNAckType);

	/* Start a clean storage job */
	scheduler_queue.Queue(new CleanStorageJob(storage_));
}

void DHT::Publish(const Key& id, const std::string string) const
{
	Data *d = (Data*) new DataString(string);
	return Publish(id, d);
}

void DHT::Publish(const Key& id, const Key& key) const
{
	Data *d = (Data*) new DataKey(key);
	return Publish(id, d);
}

void DHT::Publish(const Key& id, Data* data) const
{
	assert(data);

	/* Store the value locally. */
	try {
		storage_->addInfo(id, data);
	}
	catch(Storage::WrongDataType e)
	{
		pf_log[W_DHT] << "Publish wrong data type in the DHT";
		return;
	}

	/* Send a Publish packet to the owner of the key */
	Packet pckt(DHTPublishType, me_, id);
	pckt.SetArg(DHT_PUBLISH_KEY, id);
	/* TODO: This Data memory is currently leaked. */
	pckt.SetArg(DHT_PUBLISH_DATA, data);

	if(!chimera_->Route(pckt))
	{
		/* We are the owner, so we replicate data */
		Packet replicate(DHTRepeatPType, me_);
		replicate.SetArg(DHT_REPEAT_P_KEY, id);
		/* TODO: This Data memory is currently leaked. */
		replicate.SetArg(DHT_REPEAT_P_DATA, data);
		chimera_->SendToNeighbours(REDONDANCY, replicate);
	}
}


void DHT::Unpublish(const Key& id, const std::string string) const
{
	Data *d = (Data*) new DataString(string);
	return Unpublish(id, d);
}

void DHT::Unpublish(const Key& id, const Key& key) const
{
	Data *d = (Data*) new DataKey(key);
	return Unpublish(id, d);
}

void DHT::Unpublish(const Key& id, Data* data) const
{
	/* Remove the value locally. */
	try {
		storage_->removeInfo(id, data);
	}
	catch(Storage::WrongDataType e)
	{
		pf_log[W_DHT] << "Unpublish wrong data type in the DHT";
		return;
	}

	/* Send a Unpublish packet to the owner of the key */
	Packet pckt(DHTUnpublishType, me_, id);
	pckt.SetArg(DHT_UNPUBLISH_KEY, id);
	/* TODO: This Data memory is currently leaked. */
	pckt.SetArg(DHT_UNPUBLISH_DATA, data);

	if(!chimera_->Route(pckt))
	{
		/* We are the owner, so we replicate data */
		Packet replicate(DHTRepeatUType, me_);
		replicate.SetArg(DHT_REPEAT_U_KEY, id);
		replicate.SetArg(DHT_REPEAT_U_DATA, data);
		chimera_->SendToNeighbours(REDONDANCY, replicate);
	}
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
	else
	{
		if(arbore_ != NULL)
		arbore_->HandleMessage(sender, pckt);
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

const Arbore* DHT::GetArbore() const
{
	return arbore_;
}


