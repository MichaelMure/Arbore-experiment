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

#ifndef PF_ADDR_H
#define PF_ADDR_H

#include <netinet/in.h>
#include "dht/key.h"
#include "pf_types.h"
#include "pf_log.h"

class pf_addr
{
public:
	uint32_t ip[4];
	uint16_t port;
	Key key;

	pf_addr();
	pf_addr(in_addr_t address, uint16_t port, Key key = 0);

	/** Comparaison between two pf_addr
	 *
	 * @param other this is the other pf_addr which is compared to me
	 * @return true if the two objects are equal.
	 *
	 * \note The key isn't compared if one of the two is 0.
	 */
	bool operator ==(const pf_addr &other) const;

	/** Comparaison between two pf_addr
	 *
	 * @param other this is the other pf_addr which is compared to me
	 * @return true if my address is before the other's.
	 *
	 * \note The key isn't compared if one of the two is 0.
	 */

	bool operator<(const pf_addr &other) const;

	pf_addr pf_addr_ton() const;
	pf_addr nto_pf_addr() const;

	std::string str() const;
};

template<>
inline Log::flux& Log::flux::operator<< <pf_addr> (pf_addr addr)
{
	str += addr.str();
	return *this;
}

#endif /* PF_ADDR_H */
