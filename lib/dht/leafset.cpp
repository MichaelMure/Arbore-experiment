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

#include <stdio.h>
#include <string.h>
#include "leafset.h"
Leafset::Leafset(Host _me)
	: me(_me),
	leavesCW(ONE_SIDE_LEAFSET_SIZE),
	leavesCCW(ONE_SIDE_LEAFSET_SIZE)
{
	this->clear();
}

std::string Leafset::GetStr() const
{
	HostVector::const_iterator it;
	std::string leafset_str = "LEFT: ";

	for(it = leavesCW.begin(); it != leavesCW.end(); it++)
		leafset_str = leafset_str + it->GetKey().GetStr() + " ";

	leafset_str = leafset_str + "\nRIGHT: ";
	for(it = leavesCCW.begin(); it != leavesCCW.end(); it++)
		leafset_str = leafset_str + it->GetKey().GetStr() + " ";

	leafset_str = leafset_str +"\n" ;

	return leafset_str;
}

bool Leafset::isInside(const Host& entry)
{
	HostVector::const_iterator it;

	for (it = leavesCW.begin(); it != leavesCW.end(); ++it)
	{
		if(entry.GetKey() == it->GetKey())
		{
			pf_log[W_DEBUG] << " Already inside (clockwise) " << entry;
			return true;
		}
	}
	for (it = leavesCCW.begin(); it != leavesCCW.end(); ++it)
	{
		if(entry.GetKey() == it->GetKey())
		{
			pf_log[W_DEBUG] << " Already inside (countclockswise) " << entry;
			return true;
		}
	}
	return false;
}

bool Leafset::add(const Host& entry)
{
	pf_log[W_DEBUG] << " Adding an entry in the leafset: " << entry;
	if(me.GetKey() == entry.GetKey())
		return false;

	if(isInside(entry))
		return false;

	HostVector::iterator it;

	/* Try clockwise first */
	if(entry.GetKey().between(me.GetKey(), leavesCW.back().GetKey()))
	{
		/* If needed, remove the last host clockwise */
		if(leavesCW.size() == ONE_SIDE_LEAFSET_SIZE)
			leavesCW.pop_back();

		for(it = leavesCW.begin(); it != leavesCW.end(); it++)
		{
			if(entry.GetKey() < it->GetKey())
			{
				leavesCW.insert(it, entry);
				return true;
			}
		}
		/* Insert in the last position */
		leavesCW.push_back(entry);
		return true;
	}

	/* Then counter-clockwise */
	if(entry.GetKey().between(me.GetKey(), leavesCCW.back().GetKey()))
	{
		/* If needed, remove the last host clockwise */
		if(leavesCCW.size() == ONE_SIDE_LEAFSET_SIZE)
			leavesCCW.pop_back();

		for(it = leavesCCW.begin(); it != leavesCCW.end(); it++)
		{
			if(entry.GetKey() > it->GetKey())
			{
				leavesCCW.insert(it, entry);
				return true;
			}
		}
		/* Insert in the last position */
		leavesCCW.push_back(entry);
		return true;
	}

	return false;
}

bool Leafset::remove(const Host& entry)
{
	pf_log[W_DEBUG] << "Removed an entry from the leafset: " << entry;
	if(entry.GetKey() == this->me.GetKey())
		return false;

	HostVector::iterator it;

	for (it = leavesCW.begin(); it != leavesCW.end(); ++it)
	{
		if(entry.GetKey() == it->GetKey())
		{
			leavesCW.erase(it);
			return true;
		}
	}
	for (it = leavesCCW.begin(); it != leavesCCW.end(); ++it)
	{
		if(entry.GetKey() == it->GetKey())
		{
			leavesCCW.erase(it);
			return true;
		}
	}
	return false;
}

void Leafset::clear()
{
	/* Should we delete previoulsy stored host ? */
	leavesCW.clear();
	leavesCCW.clear();
}

void Leafset::KeyUpdate(Host _me)
{
	/* The leafset is defined around me's key, so when it changes, we drop all the others hosts. */
	this->clear();
	this->me = _me;
}

Host Leafset::routeLookup(const Key& key , bool* inLeafset) const
{
	pf_log[W_DEBUG] << "Leafset contains " << *this;

	HostVector::const_iterator it;
	*inLeafset = true;

	for (it = leavesCW.begin(); it != leavesCW.end(); ++it)
	{
		if(key == it->GetKey())
			return *it;
	}

	for (it = leavesCCW.begin(); it != leavesCCW.end(); ++it)
	{
		if(key == it->GetKey())
			return *it;
	}

	*inLeafset = false;

	if(key > me.GetKey())
	{
		if(!leavesCW.empty())
			return leavesCW.back();
	}
	else
	{
		if(!leavesCCW.empty())
			return leavesCCW.back();
	}

	return me;
}

std::vector<Host> Leafset::getCopy() const
{
	std::vector<Host> ret = leavesCW;

	for(std::vector<Host>::const_iterator it = leavesCCW.begin();
			it != leavesCCW.end();
			it++)
	{
		ret.insert(ret.end(),*it);
	}
	return ret;
}
