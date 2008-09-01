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

#include <string.h>
#include "leafset.h"
Leafset::Leafset(HostsList* _hg, Host _me) : hg(_hg), me(_me)
{
	this->clear();
}

bool Leafset::add(const Host& entry)
{
	if(this->me.GetKey() == entry.GetKey())
	{
		return false;
	}
	//try add clockwise
	//get the index where the leaf should be
	bool added = false;
	//TODO some code copy, not very nice
	long index = this->getClockwiseInsertIndex(entry);
	//if there is not already an entry with that key and it does not fall out of the leafset
	if(index >= 0 && index < ONE_SIDE_LEAFSET_SIZE)
	{
		//move the entries to make some space and add it
		//TODO data copy could be faster
		for(size_t i = MIN(this->nbLeavesClockwise, ONE_SIDE_LEAFSET_SIZE-1); i > (size_t)index; i--)
		{
			this->leavesClockwise[i] = this->leavesClockwise[i-1];
		}
		this->leavesClockwise[index] = entry;
		this->nbLeavesClockwise = MIN(ONE_SIDE_LEAFSET_SIZE, this->nbLeavesClockwise + 1);
		added = true;
	}
	//try add counterclockwise
	//get the index where the leaf should be
	index = this->getCounterclockwiseInsertIndex(entry);
	//if there is not already an entry with that key and it does not fall out of the leafset
	if(index >= 0 && index < ONE_SIDE_LEAFSET_SIZE)
	{
		//move the entries to make some space and add it
		//TODO data copy could be faster
		for(size_t i = MIN(nbLeavesCounterclockwise, ONE_SIDE_LEAFSET_SIZE-1); i > (size_t)index; i--)
		{
			this->leavesCounterclockwise[i] = this->leavesCounterclockwise[i-1];
		}
		this->leavesCounterclockwise[index] = entry;
		this->nbLeavesCounterclockwise = MIN(ONE_SIDE_LEAFSET_SIZE, this->nbLeavesCounterclockwise + 1);
		added = true;
	}
#if OPTIMIZE_ROUTING_WITH_LEAFSET_INTERVAL
	this->updateIntervalSize();
#endif
	return added;
}

bool Leafset::remove(const Host& entry)
{
	if(entry.GetKey() == this->me.GetKey())
	{
		return false;
	}
	//TODO some code copy, not very nice
	bool removed = false;
	//try to remove clockwise
	//if we make sure that the remove is only used when the entry is in the leafset, we can remove that check
	if(this->nbLeavesClockwise > 0 && entry.GetKey().between(this->me.GetKey(), this->leavesClockwise[this->nbLeavesClockwise - 1].GetKey()))
	{
		long pos = this->getClockwiseIndex(entry);
		if(pos >= 0)
		{
			//TODO data copy could be faster
			for(size_t i = pos; i < this->nbLeavesClockwise-1; i++)
			{
				this->leavesClockwise[i] = this->leavesClockwise[i+1];
			}
			this->leavesClockwise[this->nbLeavesClockwise] = InvalidHost;
			this->nbLeavesClockwise--;
			removed = true;
		}
	}
	//try to remove counterclockwise
	//if we make sure that the remove is only used when the entry is in the leafset, we can remove that check
	if(this->nbLeavesCounterclockwise > 0 && entry.GetKey().between(this->leavesCounterclockwise[this->nbLeavesCounterclockwise - 1].GetKey(), this->me.GetKey()))
	{
		long pos = this->getCounterclockwiseIndex(entry);
		if(pos >= 0)
		{
			//TODO data copy could be faster
			for(size_t i = pos; i < this->nbLeavesCounterclockwise-1; i++)
			{
				this->leavesCounterclockwise[i] = this->leavesCounterclockwise[i+1];
			}
			this->leavesCounterclockwise[this->nbLeavesCounterclockwise] = InvalidHost;
			this->nbLeavesCounterclockwise--;
			removed = true;
		}
	}
	return removed;
}

long Leafset::getClockwiseInsertIndex(const Host& entry) const
{
	//this part of the leafset is full and the entry doesn't fall into it
	if(this->nbLeavesClockwise == ONE_SIDE_LEAFSET_SIZE && !entry.GetKey().between(this->me.GetKey(), this->leavesClockwise[this->nbLeavesClockwise-1].GetKey()))
	{
		return ONE_SIDE_LEAFSET_SIZE;
	}
	for(size_t i = 0; i < this->nbLeavesClockwise-1; i++)
	{
		if(entry.GetKey().between(this->leavesClockwise[i].GetKey(), this->leavesClockwise[i+1].GetKey()))
		{
			//we don't allow 2 entries with the same dht id
			if((entry.GetKey() == this->leavesClockwise[i].GetKey()) || (entry.GetKey() == this->leavesClockwise[i+1].GetKey()))
			{
				return -1;
			}
			return i+1;
		}
	}
	if(entry.GetKey().between(this->me.GetKey(), this->leavesClockwise[0].GetKey()))
	{
		if(entry.GetKey() == this->leavesClockwise[0].GetKey())
		{
			return -1;
		}
		return 0;
	}
	return this->nbLeavesClockwise + 1;
}

long Leafset::getCounterclockwiseInsertIndex(const Host& entry) const
{
	//this part of the leafset is full and the entry doesn't fall into it
	if(this->nbLeavesCounterclockwise == ONE_SIDE_LEAFSET_SIZE && !entry.GetKey().between(this->leavesClockwise[this->nbLeavesCounterclockwise-1].GetKey(),this->me.GetKey()))
	{
		return ONE_SIDE_LEAFSET_SIZE;
	}
	for(size_t i = 0; i < this->nbLeavesCounterclockwise-1; i++)
	{
		if(entry.GetKey().between(this->leavesCounterclockwise[i+1].GetKey(), this->leavesCounterclockwise[i].GetKey()))
		{
			//we don't allow 2 entries with the same dht id
			if((entry.GetKey() == this->leavesCounterclockwise[i].GetKey()) || (entry.GetKey() == this->leavesCounterclockwise[i+1].GetKey()))
			{
				return -1;
			}
			return i+1;
		}
	}
	if(entry.GetKey().between(this->leavesCounterclockwise[0].GetKey(), this->me.GetKey()))
	{
		if(entry.GetKey() == this->leavesClockwise[0].GetKey())
		{
			return -1;
		}
		return 0;
	}
	return this->nbLeavesClockwise + 1;
}

long Leafset::getClockwiseIndex(const Host& entry) const
{
	//TODO could find a better algorithm (dichotomic search) but the leafset is quite small...
	for(size_t i = 0; i < this->nbLeavesClockwise; i++)
	{
		if(entry.GetKey() == this->leavesClockwise[i].GetKey())
		{
			return i;
		}
	}
	return -1;
}

//TODO some code copy again...
long Leafset::getCounterclockwiseIndex(const Host& entry) const
{
	//TODO could find a better algorithm (dichotomic search) but the leafset is quite small...
	for(size_t i = 0; i < this->nbLeavesCounterclockwise; i++)
	{
		if(entry.GetKey() == this->leavesCounterclockwise[i].GetKey())
		{
			return i;
		}
	}
	return -1;
}

void Leafset::clear()
{
	bzero(this->leavesClockwise, ONE_SIDE_LEAFSET_SIZE*sizeof(Host));
	bzero(this->leavesCounterclockwise, ONE_SIDE_LEAFSET_SIZE*sizeof(Host));
	this->nbLeavesClockwise = 0;
	this->nbLeavesCounterclockwise = 0;
}

void Leafset::KeyUpdate(Host _me)
{
	this->clear();
	this->me = _me;
}

#if OPTIMIZE_ROUTING_WITH_LEAFSET_INTERVAL
void Leafset::updateIntervalSize()
{
	if(this->nbLeavesClockwise > 0)
	{
		this->intervalSize = this->me.GetKey().intervalSize(this->leavesClockwise[this->nbLeavesClockwise].GetKey());
	}
	else
	{
		this->intervalSize = Key();
	}
}
#endif

Host Leafset::routeLookup(const Key& key , bool* inLeafset) const
{
	//if key is in the range of our clockwise part of the leafset, we route through the leafset
	if(this->nbLeavesClockwise > 0 && key.between(this->me.GetKey(), this->leavesClockwise[this->nbLeavesClockwise-1].GetKey()))
	{
		*inLeafset = true;
		//TODO could find a better algorithm (dichotomic search) but the leafset is quite small...
		Host closest = this->me;
		Key bestDist = this->me.GetKey().distance(key);
		for(size_t i = 0; i < this->nbLeavesClockwise; i++)
		{
			Key newDist = this->leavesClockwise[i].GetKey().distance(key);
			if(newDist > bestDist)
			{
				return closest;
			}
			else
			{
				bestDist = newDist;
				closest = leavesClockwise[i];
			}
		}
		return closest;
	}
	//TODO code copy again...
	//if key is in the range of our counterclockwise part of the leafset, we route through the leafset
	if(this->nbLeavesCounterclockwise > 0 && key.between(this->me.GetKey(), this->leavesCounterclockwise[this->nbLeavesCounterclockwise-1].GetKey()))
	{
		//TODO code copy, again
		*inLeafset = true;
		//TODO could find a better algorithm (dichotomic search) but the leafset is quite small...
		Host closest = this->me;
		Key bestDist = this->me.GetKey().distance(key);
		for(size_t i = 0; i < this->nbLeavesCounterclockwise; i++)
		{
			Key newDist = this->leavesCounterclockwise[i].GetKey().distance(key);
			if(newDist > bestDist)
			{
				return closest;
			}
			else
			{
				bestDist = newDist;
				closest = this->leavesCounterclockwise[i];
			}
		}
		return closest;
	}
	//key doesn't fall in the leafset, we choose the best leafset which is the closest to the key
	*inLeafset = false;
	Host clockwiseExtrem;
	if(this->nbLeavesClockwise == 0)
	{
		clockwiseExtrem = this->me;
	}
	else
	{
		clockwiseExtrem = this->leavesClockwise[this->nbLeavesClockwise-1];
	}
	Host counterclockwiseExtrem;
	if(this->nbLeavesCounterclockwise == 0)
	{
		counterclockwiseExtrem = this->me;
	}
	else
	{
		counterclockwiseExtrem = this->leavesCounterclockwise[this->nbLeavesCounterclockwise-1];
	}
	Key clockwiseDist = clockwiseExtrem.GetKey().distance(key);
	Key counterclockwiseDist = counterclockwiseExtrem.GetKey().distance(key);
	if(clockwiseDist < counterclockwiseDist)
	{
		return clockwiseExtrem;
	}
	if(clockwiseDist > counterclockwiseDist)
	{
		return counterclockwiseExtrem;
	}
	//if distance are equal and one extremity is local node (which means that part of the leafset is dead, should never happen), stay at local node
	if(clockwiseExtrem == this->me)
	{
		return clockwiseExtrem;
	}
	else
	{
		return counterclockwiseExtrem;
	}
}

std::vector<Host> Leafset::getCopy() const
{
	std::vector<Host> ret;
	for(size_t i = 0; i<this->nbLeavesClockwise; i++)
	{
		ret.insert(ret.end(),this->leavesClockwise[i]);
	}
	for(size_t i = 0; i<this->nbLeavesCounterclockwise; i++)
	{
		ret.insert(ret.end(),this->leavesCounterclockwise[i]);
	}
}
