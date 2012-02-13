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

Arbore::Arbore(uint16_t port)
	:dht_(new DHT(port))
{
	packet_type_list.RegisterType(ArboreChunkSendType);
}

bool Arbore::Send(const Key& id, const FileChunk& chunk) const
{
	Packet pckt(ArboreChunkSendType, dht_->GetMe(), id);
	pckt.SetArg(ARBORE_CHUNK_SEND, id);
	pckt.SetArg(ARBORE_CHUNK_SEND, (FileChunk*) new FileChunk(chunk));
	return dht_->GetChimera()->Route(pckt);
}
