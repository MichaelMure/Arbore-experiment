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

#define OPTIMIZE_ROUTING_WITH_LEAFSET_INTERVAL 1

#define LEAFSET_SIZE 8		/* (must be even) excluding node itself */
#define ONE_SIDE_LEAFSET_SIZE LEAFSET_SIZE/2

#include "key.h"
#include "host.h"

class Leafset
{
private :
	HostGlobal* hg; /*!< Global peer manager */
	ChimeraHost* me; /*!< Local host descriptor */
#if OPTIMIZE_ROUTING_WITH_LEAFSET_INTERVAL
	Key intervalSize; /*!< Interval between the peer ID and a leafset extremity */
#endif
	size_t nbLeavesClockwise; /*!< number of peers in the leafset clockwise */
	size_t nbLeavesCounterclockwise; /*!< number of peers in the leafset counterclockwise */
	ChimeraHost* leavesClockwise[ONE_SIDE_LEAFSET_SIZE]; /*!< array that contains the clowise part of the leafset */
	ChimeraHost* leavesCounterclockwise[ONE_SIDE_LEAFSET_SIZE]; /*!< array that contains the counterclowise part of the leafset */

public :
	/*! \brief Constructor
	* Constructor, creates an empty leafset
	* \param hg the global host
	* \param me the local node
	*/
	Leafset(HostGlobal* hg = NULL, ChimeraHost* me = NULL);

	/*! \brief Perfoms maintenance caused by a change in DHT key
	* When the DHT key is changed, the leafset has to be invalidated.
	* \param me a descriptor of the local peer that contains his dht key
	*/
	void KeyUpdate(ChimeraHost* me);

	/*! \brief Add an entry to the leafset
	* Adds en entry to the leafset. At the beginning of the DHT, it can be added on both sides of the leafset.
	* \param entry the entry that should be added
	* \return true if it was added, false if it wasn't
	*/
	bool add(const ChimeraHost* entry);

	/*! \brief Remove an entry from the leafset
	* Remove an entry from the leafset
	* \param entry the entry that should be removed
	* \return true if it was removed, false if it wasn't
	*/
	bool remove(const ChimeraHost* entry);

	/*! \brief Finds the next routing destination
	* Finds the best destination for the next step of routing to key. First check if the key falls into the leafset. If it is the case, find the peer responsible for it and return it. If the key is outside of the leafset, return the leafset extremity which is the closest to the destination.
	* \param key routing destination
	* \param inLeafset the function sets it to true if the key falls into the leafset
	*/
	ChimeraHost* routeLookup(const Key* key , bool* inLeafset) const;
//	bool isInRange(const Key* k) const;
	//add responsibility interval and size

private :
//	bool clockwiseSideFull() const;
//	bool counterclockwiseSideFull() const;

	/*! \brief Clears the leafset
	* Clears the leafset, all entries are removed
	*/
	void clear();

	/*! \brief Find the index to insert a peer in clockwise part of the leafset.
	* Finds which interval the peer belongs to and returns the position where it should be added. If a peer whith the same key already exists, it returns -1. The return index can be out of the array if all the peers in it are closer to the local peer thant the entry.
	* \param entry the peer we're trying to insert
	* \return the position where the peer should be, -1 if it is already in this part of the leafset, out of the array if it is not close enough to the peer to have a place
	*/
	size_t getClockwiseInsertIndex(const ChimeraHost* entry) const;

	/*! \brief Find the index to insert a peer in counterclockwise part of the leafset.
	* Finds which interval the peer belongs to and returns the position where it should be added. If a peer whith the same key already exists, it returns -1. The return index can be out of the array if all the peers in it are closer to the local peer thant the entry.
	* \param entry the peer we're trying to insert
	* \return the position where the peer should be, -1 if it is already in this part of the leafset, out of the array if it is not close enough to the peer to have a place
	*/
	size_t getCounterclockwiseInsertIndex(const ChimeraHost* entry) const;

	/*! \brief Find the index of the peer if it is present in the clockwise part of the leafset.
	* Finds the index of the peer if it is present in the clockwise part of the leafset, returns -1 if it is not present.
	* \param entry the peer we're looking for
	* \return the position of the peer, -1 if it is not present
	*/
	size_t getClockwiseIndex(const ChimeraHost* entry) const;

	/*! \brief Find the index of the peer if it is present in the counterclockwise part of the leafset.
	* Finds the index of the peer if it is present in the counterclockwise part of the leafset, returns -1 if it is not present.
	* \param entry the peer we're looking for
	* \return the position of the peer, -1 if it is not present
	*/
	size_t getCounterclockwiseIndex(const ChimeraHost* entry) const;
	//bool inLeftRange(const Key* key);
	//bool inRightRange(const Key* key);
#if OPTIMIZE_ROUTING_WITH_LEAFSET_INTERVAL

	/*! \brief Updates the distance between the local peer and is most clockwise distant neighbour in the leafset.
	* Updates the distance between the local peer and is most clockwise distant neighbour in the leafset. The choice of the clockwise side is arbitrary.
	*/
	void updateIntervalSize();
#endif
};
#endif /* _CHIMERA_LEAFSET_H_ */
