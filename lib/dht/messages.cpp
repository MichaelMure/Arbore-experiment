/*
 * Copyright(C) 2008 Romain Bignon
 * Copyright(C) 2012 Michael Muré <batolettre@gmail.com>
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

#include <util/pf_log.h>
#include <util/time.h>
#include <net/network.h>
#include <net/packet.h>
#include <net/packet_handler.h>
#include <scheduler/scheduler_queue.h>
#include <chimera/chimera.h>

#include "messages.h"

class DHTPublishMessage : public NetworkMessage
{
public:
	void Handle (Chimera&, const Host&, const Packet&)
	{
		/* TODO: unimplemented */
	}
};

class DHTUnpublishMessage : public NetworkMessage
{
public:
	void Handle (Chimera&, const Host&, const Packet&)
	{
		/* TODO: unimplemented */
	}
};

PacketType   DHTPublishType(DHT_PUBLISH,   new DHTPublishMessage,   Packet::REQUESTACK,  "PUBLISH",                                   T_END);
PacketType DHTUnpublishType(DHT_UNPUBLISH, new DHTUnpublishMessage, Packet::REQUESTACK,  "UNPUBLISH",                                 T_END);
