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

#include "chimera_routing.h"

ChimeraRouting::ChimeraRouting(HostGlobal* _hg, ChimeraHost* _me) : hg(_hg), me(_me)
{
	this->leafset = Leafset(_hg, _me);
	this->routingTable = RoutingTable(_hg, _me);
}

void ChimeraRouting::KeyUpdate(ChimeraHost* me)
{
	this->leafset.KeyUpdate(me);
	this->routingTable.KeyUpdate(me);
}

void ChimeraRouting::route_update(const ChimeraHost* host, int joined)
{
	if(joined == 1)
	{
		this->add(host);
	}
	else
	{
		this->remove(host);
	}
}

bool ChimeraRouting::add(const ChimeraHost* host)
{
	bool added = this->leafset.add(host);
	added = added || this->routingTable.add(host);
	return added;
}

bool ChimeraRouting::remove(const ChimeraHost* host)
{
	bool removed = this->leafset.remove(host);
	removed = removed || this->routingTable.remove(host);
	return removed;
}

ChimeraHost* ChimeraRouting::routeLookup(const Key* key) const
{
	bool b;
	ChimeraHost* leafsetBest = this->leafset.routeLookup(key , &b);
	if(b)
	{
		return leafsetBest;
	}
	ChimeraHost* routingTableBest = this->routingTable.routeLookup(key , &b);
	if(b)
	{
		return routingTableBest;
	}
	Key distLB = leafsetBest->GetKey().distance(*key);
	Key distRB = routingTableBest->GetKey().distance(*key);
	if(distLB < distRB)
	{
		return leafsetBest;
	}
	if(distLB > distRB)
	{
		return routingTableBest;
	}
	//distance is the same
	if(leafsetBest == this->me)
	{
		return leafsetBest;
	}
	return routingTableBest;
}
