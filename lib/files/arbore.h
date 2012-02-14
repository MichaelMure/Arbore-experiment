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

#ifndef ARBORE_H
#define ARBORE_H

#include <dht/dht.h>
#include "file_chunk.h"

class Arbore
{
public:

	Arbore(uint16_t port);
	virtual ~Arbore() {}

	/** Send a chunk
	 * @return true if the chunk was sent successfully, false otherwise. */
	bool Send(const Key& id, const FileChunk& chunk) const;

	/** @return the DHT layer */
	DHT* GetDHT() const;

	/** Handle a network message.
	 * @return true if the request was send successfully, false otherwise. */
	void HandleMessage(const Host& sender, const Packet& pckt);

	/** Callback used by the DHT, to give result of a GET request.
	 * @param id the key of the data
	 * @param data the data requested
	 */
	void DataCallback(const Key& id, const Data* data);


private:
	DHT *dht_;
};


#endif /* ARBORE_H */
