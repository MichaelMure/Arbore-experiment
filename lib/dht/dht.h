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

#ifndef DHT_H
#define DHT_H

#include <chimera/chimera.h>
#include <util/key.h>

#include "data.h"
#include "datakey.h"
#include "datastring.h"
#include "storage.h"

class DHT
{
public:
	/* DHT constructor.
	 * @param port the port that we listen on
	 * @param me the key used on the routing layer
	 */
	DHT(uint16_t port, const Key& me);
	virtual ~DHT() {}

	/** Publish a string on DHT
	 * @return true if the request was send successfully, false otherwise. */
	bool Publish(const Key& id, const std::string string) const;

	/** Publish a list of string on DHT
	 * @return true if the request was send successfully, false otherwise. */
	bool Publish(const Key& id, const DataString& strings) const;

	/** Publish a key on DHT
	 * @return true if the request was send successfully, false otherwise. */
	bool Publish(const Key& id, const Key& key) const;

	/** Publish a list of keys on DHT
	 * @return true if the request was send successfully, false otherwise. */
	bool Publish(const Key& id, const DataKey& keys) const;

	/** Unpublish an object on DHT
	 * @return true if the request was send successfully, false otherwise. */
	bool Unpublish(const Key& id);

	/* Request a value in the DHT
	 * @return true if the request was send successfully, false otherwise. */
	bool RequestData(const Key& id) const;

	/** Handle a network message.
	 * @return true if the request was send successfully, false otherwise. */
	void HandleMessage(const Host& sender, const Packet& pckt);

	/** @return the chimera routing layer */
	Chimera* GetChimera() const;

	/** @return the DHT storage object */
	Storage* GetStorage() const;

private:
	const Key& me_;
	Chimera *chimera_;
	Storage *storage_;
};

#endif
