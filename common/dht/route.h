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

#ifndef _CHIMERA_ROUTE_H_
#define _CHIMERA_ROUTE_H_

#include "key.h"
#include "mutex.h"

#define MAX_ROW KEY_SIZE/BASE_B
#define MAX_COL 16 /* 2^4 */
#define MAX_ENTRY 3
#define LEAFSET_SIZE 8		/* (must be even) excluding node itself */

class ChimeraHost;
class HostGlobal;

class RouteGlobal : protected Mutex
{
	HostGlobal* hg;
	ChimeraHost *me;
	char *keystr;
	ChimeraHost ****table;
	ChimeraHost **leftleafset;
	ChimeraHost **rightleafset;
	Key Rrange;
	Key Lrange;

	void sort_hosts (ChimeraHost ** hosts, Key key, size_t size) const;
	void sort_hosts_key (ChimeraHost ** hosts, Key key, size_t size) const;

	ChimeraHost* find_closest_key (ChimeraHost ** hosts, Key key, size_t size) const;
	int leafset_size (ChimeraHost ** arr) const;
	void leafset_range_update (Key * rrange, Key * lrange) const;
	void leafset_update (ChimeraHost * host, int joined, ChimeraHost ** deleted, ChimeraHost ** added) const;
	void leafset_delete (ChimeraHost * host, int right_or_left, ChimeraHost ** deleted) const;
	void leafset_insert (ChimeraHost * host, int right_or_left, ChimeraHost ** deleted,
			     ChimeraHost ** added) const;
	ChimeraHost **route_neighbors (int count) const;

public:

	RouteGlobal(HostGlobal* hg, ChimeraHost* me);

	void KeyUpdate(ChimeraHost* me) { this->me = me; }

	/** route_lookup:
	** returns an array of count nodes that are acceptable next hops for a
	** message being routed to key. is_save is ignored for now.
	 */

	ChimeraHost **route_lookup (Key key, unsigned int count, int is_safe);


	/** route_neighbors:
	** returns an array of count neighbor nodes with priority to closer nodes.
	*/

	ChimeraHost **route_neighbors (int count);


	/** route_update:
	** updates the routing table in regard to host. If the host is joining
	** the network (and joined == 1), then it is added to the routing table
	** if it is appropriate. If it is leaving the network (and joined == 0),
	** then it is removed from the routing tables.
	*/

	void route_update (ChimeraHost * host, int joined);


	/** route_row_lookup:
	** return the row in the routing table that matches the longest prefix with key.
	*/

	ChimeraHost **route_row_lookup (Key key);


	/** route_get_table:
	** returns all the entries in the routing table in an array of ChimeraHost.
	*/

	ChimeraHost **route_get_table ();

	/** prints routing table,
	*/
	void printTable () const;

};


#endif /* _CHIMERA_ROUTE_H_ */
