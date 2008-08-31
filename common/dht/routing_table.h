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

#ifndef _CHIMERA_ROUTINGTABLE_H_
#define _CHIMERA_ROUTINGTABLE_H_

#include "key.h"
#include "hosts_list.h"

#define MAX_ROW KEY_SIZE/BASE_B
#define MAX_COL 16 /* 2^4 */
#define MAX_ENTRY 3

class RoutingTable
{
private:
	HostsList* hg;
	Host me;                                 /*!< Local host descriptor */
	Host table[MAX_ROW][MAX_COL][MAX_ENTRY]; /*!< Routing table */

public:
	/*! \brief Constructor
	 *
	 * Constructor, creates an empty routing table
	 *
	 * \param hg  the global host
	 * \param me  the local node
	 */
	RoutingTable(HostsList* hg, Host me);

	/*! \brief Perfoms maintenance caused by a change in DHT key
	 *
	 * When the DHT key is changed, the routing table has to be invalidated.
	 *
	 * \param me  a descriptor of the local peer that contains his dht key
	 */
	void KeyUpdate(Host me);

	/*! \brief Add an entry to the routing table
	 *
	 * Adds en entry to the routing table
	 *
	 * \param entry  the entry that should be added
	 * \return  true if it was added, false if it wasn't
	 */
	bool add(const Host& entry);

	/*! \brief Remove an entry from the routing table
	 *
	 * Remove an entry from the routing table
	 *
	 * \param entry  the entry that should be removed
	 * \return  true if it was removed, false if it wasn't
	 */
	bool remove(const Host& entry);

	/*! \brief Finds the next routing destination
	 *
	 * Finds the best destination for the next step of routing to key.
	 * First we look for an entry that would solve on extra digit in
	 * the key, if we don't find any we get the entry closest to the key
	 * in the whole routing table.
	 *
	 * \param key  routing destination
	 * \param perfectMatch  the function sets it to true if key prefix
	 *                      matching progressed, false otherwise
	 */
	Host routeLookup(const Key& key , bool* perfectMatch) const;

private:

	/*! \brief Clears the routing table
	 *
	 * Clears the routing table, all entries are set to NULL
	 */
	void clear();

	/*! \brief Get the index of the worst entry for a routing table position
	 *
	 * Gets the index of the worst entry for a routing table position. The
	 * choice is made based first on success average and and then on
	 * latency.
	 *
	 * \param line  routing table line index
	 * \param column  routing table column index
	 * \return  the index of the worst entry, -1 if all entries where NULL
	 */
	long findWorstEntry(size_t line, size_t column) const;

	/*! \brief Get the index of the best entry for a routing table position
	 *
	 * Gets the index of the best entry for a routing table position. The
	 * choice is made based first on success average and and then on
	 * latency.
	 *
	 * \param line routing table line index
	 * \param column routing table column index
	 * \return the index of the best entry, -1 if all entries where NULL
	 */
	long findBestEntry(size_t line, size_t column) const;

	/*! \brief Transform a character from a dht key into an int
	 *
	 * The dht key is stored as a string, this function transforms a
	 * character into an int in order to match an index in the routing
	 * table.
	 *
	 * \param c  the character to translate
	 * \return  the integer representation of the character
	 */
	static size_t hexalphaToInt(int c);

	/*! \brief Compares 2 routing table entries and returns the best
	 *
	 * Compares 2 routing table entries and returns the best based on
	 * success average and latency.
	 *
	 * \param e1  the first entry to be compared
	 * \param e2  the second entry to be compared
	 * \return  the best entry between e1 and e2
	 */
	static Host bestEntry(const Host e1, const Host e2);

	/*! \brief Compares 2 routing table entries to route to a destination and returns the best
	 *
	 * Compares 2 routing table entries and returns the best based on
	 * prefix matching with the key, success average and latency.
	 *
	 * \param e1 the first entry to be compared
	 * \param e2 the second entry to be compared
	 * \param key routing destination
	 * \return the best entry between e1 and e2
	 */
	static Host bestEntry(const Host e1, const Host e2, const Key& key);

};
#endif /* _CHIMERA_ROUTINGTABLE_H_ */
