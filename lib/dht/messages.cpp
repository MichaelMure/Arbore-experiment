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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com). This product includes software written by Tim
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

#include "messages.h"
#include "dht.h"

class DHTPublishMessage : public DHTMessage
{
public:
	void Handle (DHT& dht, const Host&, const Packet& pckt)
	{
		Key k = pckt.GetArg<Key>(DHT_PUBLISH_KEY);
		pf_log[W_DHT] << "Got Publish message for key " << k;

		Data *data = pckt.GetArg<Data*>(DHT_PUBLISH_DATA);
		pf_log[W_DHT] << "Data: " << data->GetStr();

		try {
			dht.GetStorage()->addInfo(k, data);
		}
		catch(Storage::WrongDataType e) {
			pf_log[W_DHT] << "Asked to store wrong data type, data not stored.";
		}
	}
};

class DHTReplicateMessage : public DHTMessage
{
public:
	void Handle (DHT&, const Host&, const Packet&)
	{

	}
};

class DHTUnpublishMessage : public DHTMessage
{
public:
	void Handle (DHT&, const Host&, const Packet&)
	{
		/* TODO: unimplemented */
	}
};

class DHTGetMessage : public DHTMessage
{
public:
	void Handle (DHT& dht, const Host& host, const Packet& pckt)
	{
		Key k = pckt.GetArg<Key>(DHT_GET_KEY);
		pf_log[W_DHT] << "Get received with key " << k;
		if(dht.GetStorage()->hasKey(k))
		{
			pf_log[W_DHT] << "Answer with data " << dht.GetStorage()->getInfo(k)->GetStr();
			Packet get_ack(DHTGetAckType, dht.GetMe(), host.GetKey());
			get_ack.SetArg(DHT_GET_ACK_KEY, k);
			get_ack.SetArg(DHT_GET_ACK_DATA, dht.GetStorage()->getInfo(k));
			if(!dht.GetChimera()->Send(host, get_ack))
				pf_log[W_DHT] << "Send get ACK message failed!";
		}
		else
		{
			pf_log[W_DHT] << "Data unknow, answer with GET_NACK.";
			Packet get_nack(DHTGetNAckType, dht.GetMe(), host.GetKey());
			get_nack.SetArg(DHT_GET_NACK_KEY, k);
			if(!dht.GetChimera()->Send(host, get_nack))
				pf_log[W_DHT] << "Send get NACK message failed!";
		}
	}
};

class DHTGetAckMessage : public DHTMessage
{
public:
	void Handle (DHT&, const Host&, const Packet& pckt)
	{
		Key k = pckt.GetArg<Key>(DHT_GET_ACK_KEY);
		pf_log[W_DHT] << "Received data with key " << k;
		pf_log[W_DHT] << pckt.GetArg<Data*>(DHT_GET_ACK_DATA)->GetStr();
	}
};

class DHTGetNAckMessage : public DHTMessage
{
public:
	void Handle (DHT&, const Host&, const Packet& pckt)
	{
		Key k = pckt.GetArg<Key>(DHT_GET_NACK_KEY);
		pf_log[W_DHT] << "Received NACK for data with key " << k;
	}
};


PacketType   DHTPublishType(DHT_PUBLISH,   new DHTPublishMessage,   Packet::REQUESTACK, "PUBLISH",    /* DHT_PUBLISH_KEY */    T_KEY,
                                                                                                      /* DHT_PUBLISH_DATA */   T_DATA,
                                                                                                                               T_END);
PacketType DHTReplicateType(DHT_REPLICATE, new DHTReplicateMessage, Packet::REQUESTACK, "REPLICATE",  /* DHT_REPLICATE_KEY */  T_KEY,
                                                                                                      /* DHT_REPLICATE_DATA */ T_DATA,
                                                                                                                               T_END);
PacketType DHTUnpublishType(DHT_UNPUBLISH, new DHTUnpublishMessage, Packet::REQUESTACK, "UNPUBLISH",  /* DHT_UNPUBLISH_KEY */  T_KEY,
                                                                                                      /* DHT_UNPUBLISH_DATA */ T_DATA,
                                                                                                                               T_END);
PacketType       DHTGetType(DHT_GET,       new DHTGetMessage,       Packet::REQUESTACK, "GET",        /* DHT_GET_KEY */        T_KEY,
                                                                                                                               T_END);
PacketType    DHTGetAckType(DHT_GET_ACK,   new DHTGetAckMessage,    Packet::REQUESTACK, "GET_ACK",    /* DHT_GET_ACK_KEY */    T_KEY,
                                                                                                      /* DHT_GET_ACK_DATA */   T_DATA,
                                                                                                                               T_END);
PacketType   DHTGetNAckType(DHT_GET_NACK,  new DHTGetNAckMessage,   Packet::REQUESTACK, "GET_NACK",   /* DHT_GET_NACK_KEY */   T_KEY,
                                                                                                                               T_END);


