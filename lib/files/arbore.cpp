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

#include "arbore.h"
#include "messages.h"

#include <net/packet.h>
#include <util/pf_log.h>

Arbore::Arbore(uint16_t port)
	:dht_(new DHT(this, port))
{
	packet_type_list.RegisterType(ArboreChunkSendType);
}

bool Arbore::Send(const Key& id, const FileChunk& chunk) const
{
	Packet pckt(ArboreChunkSendType, dht_->GetMe(), id);
	pckt.SetArg(ARBORE_CHUNK_SEND_CHUNK, chunk);
	return dht_->GetChimera()->Route(pckt);
}


DHT* Arbore::GetDHT() const
{
	return dht_;
}

void Arbore::HandleMessage(const Host& sender, const Packet& pckt)
{
	PacketHandlerBase *handler_base = pckt.GetPacketType().GetHandler();
	if(handler_base->getType() == HANDLER_TYPE_ARBORE)
	{
		ArboreMessage *handler = (ArboreMessage*) handler_base;
		handler->Handle(*this, sender, pckt);
	}
}

void DataCallback(const Key& id, const Data* data)
{
	pf_log[W_FILE] << "Received data with key " << id;
	pf_log[W_FILE] << data->GetStr();
}
