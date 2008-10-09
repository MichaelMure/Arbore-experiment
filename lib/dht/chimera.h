/*
 * Copyright(C) 2008 Romain Bignon
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

#ifndef CHIMERA_H
#define CHIMERA_H

#include "net/packet_type_list.h"
#include "net/host.h"

class Network;
class ChimeraRouting;
class Packet;

class ChimeraDHT : public PacketTypeList
{
	Network* network;
	ChimeraRouting* routing;
	Host me;
	int fd;

	void sendRowInfo(const Packet& pckt);

public:
	/** GRACEPERIOD is the time that has to be elapsed before a node
	 ** can be accepted to the network again after last send to it failed
	 */
	static const unsigned int GRACEPERIOD = 30;		/* seconds */

	/** Create the ChimeraDHT.
	 *
	 * Create a new ChimeraDHT object to build all of the Distributed
	 * Hash Table which is used by Peerfuse to send messages to other
	 * peers.
	 *
	 * @param network  the Network object used to create a new UDP
	 *                 socket, send and receive messages.
	 * @param port  listened port
	 * @param my_key  key used on the DHT network.
	 */
	ChimeraDHT(Network* network, uint16_t port, Key my_key);

	/** @return  the Host object which represents me on network. */
	Host GetMe() const { return me; }

	/** @return  the Network object. */
	Network* GetNetwork() const { return network; }

	/** Get the ChimeraRouting object.
	 *
	 * TODO: it MUST be private!!
	 *
	 * @return  the ChimeraRouting pointer.
	 */
	ChimeraRouting* GetRouting() const { return routing; }

	/** Join the DHT network.
	 *
	 * It tries to connect to a peer to join a
	 * DHT network.
	 *
	 * @param bootstrap  the peer I try to contact.
	 */
	void Join(const Host& bootstrap);

	/** Send a message to a peer.
	 *
	 * @param destination  this is peer which will receive message.
	 * @param pckt  the Packet which describes all of the message.
	 * @return  true if it success, false if it fails.
	 */
	bool Send(const Host& destination, const Packet& pckt);

	/** Route a packet on the DHT.
	 *
	 * It looks for the destination Key in Packet header,
	 * find a peer in his routing table to route the packet.
	 *
	 * @param pckt  Packet I try to route.
	 * @return  \b true if it success, \b false if it fails.
	 */
	bool Route(const Packet& pckt);

	/** Ping a peer.
	 *
	 * @param dest  the destination host.
	 * @return  true if the host is up.
	 */
	bool Ping(const Host& dest);

	/** Publish an object on DHT */
	bool Publish(Key id);

	/** Unpublish an object on DHT */
	bool Unpublish(Key id);

	/** Send message to owners on an object. */
	bool SendToObj(Key id, const Packet& pckt);
};

#endif /* CHIMERA_H */
