/*
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
 * This file contains some code from the Chimera's Distributed Hash Table,
 * written by CURRENT Lab, UCSB.
 *
 */

#ifndef DHT_MESSAGES_H
#define DHT_MESSAGES_H

#include <net/packet_handler.h>

class DHT;

class DHTMessage : public PacketHandlerBase
{
public:
	virtual void Handle (DHT& dht, const Host& sender, const Packet& pckt) = 0;
	HandlerType getType() { return HANDLER_TYPE_DHT; }
};

enum
{
	DHT_PUBLISH_KEY,
	DHT_PUBLISH_DATA
};
extern PacketType DHTPublishType;

enum
{
	DHT_REPLICATE_KEY,
	DHT_REPLICATE_DATA
};
extern PacketType DHTReplicateType;

enum
{
	DHT_UNPUBLISH_KEY,
	DHT_UNPUBLISH_DATA
};
extern PacketType DHTUnpublishType;

enum
{
	DHT_GET_KEY
};
extern PacketType DHTGetType;

enum
{
	DHT_GET_ACK_KEY,
	DHT_GET_ACK_DATA
};
extern PacketType DHTGetAckType;

enum
{
	DHT_GET_NACK_KEY
};
extern PacketType DHTGetNAckType;



#endif /* DHT_MESSAGES_H */
