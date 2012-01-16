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

#ifndef _CHIMERA_LEAFSET_H_
#define _CHIMERA_LEAFSET_H_

#define LEAFSET_SIZE 8		/* (must be even) excluding node itself */
#define ONE_SIDE_LEAFSET_SIZE LEAFSET_SIZE/2

#include <vector>
#include <net/hosts_list.h>
#include <util/key.h>

/** The leafset contains the current peer's immediate neighbors in key space
 * It's used in order to route packet among the DHT
 */
class Leafset
{
private :
	Host me;                                      /** Local host descriptor */

	typedef std::vector<Host> HostVector;
	HostVector leavesCW;                          /** array that contains the clowise part of the leafset */
	HostVector leavesCCW;                         /** array that contains the counterclowise part of the leafset */

public :
	/** \brief Constructor
	 *
	 * Constructor, creates an empty leafset
	 *
	 * \param me the local node
	 */
	Leafset(Host me);

	/** \brief Perfoms maintenance caused by a change in DHT key
	 *
	 * When the DHT key is changed, the leafset has to be invalidated.
	 *
	 * \param me a descriptor of the local peer that contains his dht key
	 */
	void KeyUpdate(Host me);

	/** \brief Add an entry to the leafset
	 *
	 * Adds en entry to the leafset. At the beginning of the DHT, it can be added on both sides of the leafset.
	 *
	 * \param entry the entry that should be added
	 * \return true if it was added, false if it wasn't
	 */
	bool add(const Host& entry);

	/** \brief Remove an entry from the leafset
	 *
	 * Remove an entry from the leafset
	 *
	 * \param entry the entry that should be removed
	 * \return true if it was removed, false if it wasn't
	 */
	bool remove(const Host& entry);

	/** \brief Finds the next routing destination
	 *
	 * Finds the best destination for the next step of routing to key.
	 * First check if the key falls into the leafset. If it is the case,
	 * find the peer responsible for it and return it. If the key is
	 * outside of the leafset, return the leafset extremity which is the
	 * closest to the destination.
	 *
	 * @param key  routing destination
	 * @param inLeafset  the function sets it to true if the key falls into the leafset
	 */
	Host routeLookup(const Key& key , bool* inLeafset) const;

	/** @return a textual representation of the leafset. */
	std::string GetStr() const;

	/** @return a copy of all the host stored in the leafset. */
	std::vector<Host> getCopy() const;

private :
	/** \brief Clears the leafset
	 *
	 * Clears the leafset, all entries are removed
	 */
	void clear();
};

template<>
inline Log::flux& Log::flux::operator<< <Leafset> (Leafset leafset)
{
	_str += leafset.GetStr();
	return *this;
}

#endif /* _CHIMERA_LEAFSET_H_ */
