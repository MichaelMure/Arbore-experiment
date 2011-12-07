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
#include "routing_table.h"

RoutingTable::RoutingTable(Host _me)
	: me(_me),
	  routing_table(MAX_ROW * MAX_COL * MAX_ENTRY)
{
	this->clear();
}

Host& RoutingTable::getEntry(size_t i, size_t j, size_t k)
{
	return routing_table.at(k * (MAX_COL * MAX_ROW) + j * (MAX_ROW) + i);
}

Host RoutingTable::getEntry(size_t i, size_t j, size_t k) const
{
	return routing_table.at(k * (MAX_COL * MAX_ROW) + j * (MAX_ROW) + i);
}

void RoutingTable::setEntry(size_t i, size_t j, size_t k, Host host)
{
	routing_table[k * (MAX_COL * MAX_ROW) + j * (MAX_ROW) + i] = host;
}

void RoutingTable::clear()
{
	for(std::vector<Host>::iterator it = routing_table.begin(); it != routing_table.end(); ++it)
		*it = InvalidHost;
}

void RoutingTable::print() const
{
    size_t i, j, k;
    bool b;

    fprintf (stderr,
             "------------------------------- TABLE-------------------------------\n");
    for (i = 0; i < MAX_ROW; i++)
        {
            b = false;

            for (j = 0; j < MAX_COL; j++)
                {
                    for (k = 0; k < MAX_ENTRY; k++)
                        if (getEntry(i, j, k) != InvalidHost)
			{
                            fprintf (stderr, "%s ", getEntry(i, j,k).GetKey().GetStr().c_str());
			    b = true;
			}
                }
	    if(b)
	            fprintf (stderr, "\n");
        }
    fprintf (stderr,
             "----------------------------------------------------------------------\n");
}


void RoutingTable::KeyUpdate(Host _me)
{
	this->me = _me;
	//clear the routing table because when the key changes all the entries are at the wrong place
	this->clear();
}

bool RoutingTable::add(const Host& entry)
{
	//original code performs some leafset update... shoud not be needed anymore
	//TODO see if we can remove this sanity check
	//the entry has the same key as the local node, should never happen
	pf_log[W_DEBUG] << "Added an entry in the routing table: " << entry;
	if(this->me.GetKey() == entry.GetKey())
	{
		return false;
	}
	//get the coordinates where the entry should go
	size_t i = this->me.GetKey().key_index(entry.GetKey());
	size_t j = hexalphaToInt(entry.GetKey().GetStr()[i]);
	bool found = false;
	for (size_t k = 0; !found && k < MAX_ENTRY; k++)
	{
		//we found an empty space, add the entry
		if (this->getEntry(i, j, k) == InvalidHost)
		{
			this->setEntry(i, j, k, entry);
			found = true;
		}
		//entry is already in the routing table, simply return
		else if (this->getEntry(i, j, k) != InvalidHost && this->getEntry(i, j, k).GetKey() == entry.GetKey())
		{
			return false;
		}

	}
	//the entry array is full we have to get rid of one
	//replace the new node with the node with the highest latency in the entry array
	//TODO understand why we can't just sometimes keep the entries we have
	if (!found)
	{
		size_t pick = this->findWorstEntry(i,j);
		this->setEntry(i, j, pick, entry);
	}
	return true;
}

bool RoutingTable::remove(const Host& entry)
{
	//original code performs some leafset update... shoud not be needed anymore
	//TODO see if we can remove this sanity check
	//the entry has the same key as the local node, should never happen
	pf_log[W_DEBUG] << "Removed an entry from the routing table: " << entry;
	if(this->me.GetKey() == entry.GetKey())
	{
		return false;
	}
	//get the coordinates where the entry should go
	size_t i = this->me.GetKey().key_index(entry.GetKey());
	size_t j = hexalphaToInt(entry.GetKey().GetStr()[i]);
	for (size_t k = 0; k < MAX_ENTRY; k++)
	{
		if (this->getEntry(i, j, k) != InvalidHost && this->getEntry(i, j, k).GetKey() == entry.GetKey())
		{
			//when we find it, set the entry to null so that we don't use it anymore
			this->setEntry(i, j, k, InvalidHost);
			return true;
		}
	}
	return false;
}

size_t RoutingTable::findWorstEntry(size_t line, size_t column) const
{
	//TODO do a comparator for ChimeraHost instead ?
	size_t worst = MAX_ENTRY;
	for (size_t k = 0; k < MAX_ENTRY && this->getEntry(line, column, k) != InvalidHost; k++)
	{
		if(worst == MAX_ENTRY)
		{
			worst = k;
		}
		//priority is SuccessAvg > Latency
		else if (this->getEntry(line, column, worst).GetSuccessAvg() > this->getEntry(line, column, k).GetSuccessAvg()
			||
			(
				this->getEntry(line, column, worst).GetSuccessAvg() == this->getEntry(line, column, k).GetSuccessAvg()
				&&
				this->getEntry(line, column, worst).GetLatency() < this->getEntry(line, column, k).GetLatency()
			)
		)
		{
			worst = k;
		}
	}
	return worst;
}

size_t RoutingTable::findBestEntry(size_t line, size_t column) const
{
	//TODO do a comparator for ChimeraHost instead ?
	size_t best = MAX_ENTRY;
	for (size_t k = 0; k < MAX_ENTRY && this->getEntry(line, column, k) != InvalidHost; k++)
	{
		if(best == MAX_ENTRY)
		{
			best = k;
		}
		//priority is SuccessAvg > Latency
		else if (this->getEntry(line, column, best).GetSuccessAvg() < this->getEntry(line, column, k).GetSuccessAvg()
			||
			(
				this->getEntry(line, column, best).GetSuccessAvg() == this->getEntry(line, column, k).GetSuccessAvg()
				&&
				this->getEntry(line, column, best).GetLatency() > this->getEntry(line, column, k).GetLatency()
			)
		)
		{
			best = k;
		}
	}
	return best;
}

size_t RoutingTable::hexalphaToInt(int c)
{
	//ugly, see if we can change it or at least move it to Key class
	char hexalpha[] = "0123456789abcdef";
	size_t i;
	size_t answer = 0;
	for (i = 0; answer == 0 && hexalpha[i] != '\0'; i++)
	{
		if (hexalpha[i] == c)
		{
		    answer = i;
		}
	}
	return answer;
}

Host RoutingTable::routeLookup(const Key& key , bool* perfectMatch) const
{
	print();
	if(this->me.GetKey() == key)
	{
		return this->me;
	}
	//try perfect match
	size_t matchLine = this->me.GetKey().key_index(key);
	size_t matchCol = hexalphaToInt(key.GetStr()[matchLine]);
	Host nextHop = InvalidHost;
	for (size_t k = 0; k < MAX_ENTRY; k++)
	{
		if (this->getEntry(matchLine, matchCol, k) != InvalidHost && this->getEntry(matchLine, matchCol, k).GetSuccessAvg() > BAD_LINK)
		{
			if(nextHop == InvalidHost)
			{
				nextHop = this->getEntry(matchLine, matchCol, k);
			}
			else
			{
				nextHop = bestEntry(nextHop, this->getEntry(matchLine, matchCol, k), key);
			}
		}
	}
	if(nextHop != InvalidHost)
	{
		//we got a perfect match, prefix matching got one step further, we route to that peer
		*perfectMatch = true;
		return nextHop;
	}
	//no perfect match, fin the closest entry
	*perfectMatch = false;
	Host clockwiseBest = InvalidHost;
	pf_log[W_DEBUG] << MAX_ROW << "x" << MAX_COL << "x" << MAX_ENTRY;
	size_t i = matchLine;
	size_t j = matchCol;
	while(clockwiseBest == InvalidHost)
	{
		j++;
		//move to the next position in the routing table
		//reached the end of the line without going into local node cell, local node is the best candidate
		if(j >= MAX_COL)
		{
			clockwiseBest = this->me;
		}
		else
		{
			//reached local node cell, move down to the next line
			if(hexalphaToInt(this->me.GetKey().GetStr()[i])	== j)
			{
				i++;
				//if we were at the last line, can't go down any more, local node is the best candidate
				if(i >= MAX_ROW)
				{
					pf_log[W_DEBUG] << "shit";
					clockwiseBest = this->me;
				}
				else
				//we can go to the beginning of the next line
				{
					j = 0;
				}
			}
		}
		//if we have a new position to try, do it
		if(clockwiseBest != InvalidHost)
		{
			size_t index = this->findBestEntry(i,j);
			if(index < MAX_ENTRY)
			{
				clockwiseBest = this->getEntry(i, j, index);
			}
		}
	}
	Host counterClockwiseBest = InvalidHost;
	i = matchLine;
	j = matchCol;
	while(counterClockwiseBest == InvalidHost)
	{
		//move to the next position in the routing table
		//reached the beginning of the line without going into local node cell, local node is the best candidate
		if(j == 0)
		{
			counterClockwiseBest = this->me;
		}
		else
		{
			--j;
			//reached local node cell, move down to the next line
			if(hexalphaToInt(this->me.GetKey().GetStr()[i]) == j)
			{
				i++;
				//if we were at the last line, can't go down any more, local node is the best candidate
				if(i == MAX_ROW)
				{
					counterClockwiseBest = this->me;
				}
				else
				//we can go to the beginning of the next line
				{
					j = MAX_COL - 1;
				}
			}
		}
		//if we have a new position to try, do it
		if(clockwiseBest != InvalidHost)
		{
			size_t index = this->findBestEntry(i,j);
			if(index < MAX_ENTRY)
			{
				counterClockwiseBest = this->getEntry(i, j, index);
			}
		}
	}
	Key distCW = clockwiseBest.GetKey().distance(key);
	Key distCCW = counterClockwiseBest.GetKey().distance(key);
	if(distCW < distCCW)
	{
		return clockwiseBest;
	}
	if(distCW > distCCW)
	{
		return counterClockwiseBest;
	}
	//distance is the same
	if(clockwiseBest == this->me)
	{
		return clockwiseBest;
	}

	/** TODO: Is it really what we want to return in this case? --romain */
	return InvalidHost;
}

std::vector<Host> RoutingTable::getRow(size_t i) const
{
	std::vector<Host> ret;
	size_t j, l;
	//BlockLockMutex(this);
	for (j = 0; j < MAX_COL; j++)
	{
		for (l = 0; l < MAX_ENTRY; l++)
		{
			if (this->getEntry(i, j, l) != InvalidHost)
			{
				ret.insert(ret.end(), this->getEntry(i, j, l));
			}
		}
	}
	ret.insert(ret.end(), this->me);
	return ret;
}

std::vector<Host> RoutingTable::getCopy() const
{
	std::vector<Host> ret;
	int i, j, l;

	//BlockLockMutex(this);

	for (i = 0; i < MAX_ROW; i++)
	{
		for (j = 0; j < MAX_COL; j++)
		{
			for (l = 0; l < MAX_ENTRY; l++)
			{
				if (this->getEntry(i, j, l) != InvalidHost)
				{
					ret.insert(ret.end(), this->getEntry(i, j, l));
				}
			}
		}
	}
	return ret;

}

Host RoutingTable::bestEntry(Host e1, Host e2)
{
	//priority : SuccessAvg > latency
	if(e1.GetSuccessAvg() > e2.GetSuccessAvg() || ((e1.GetSuccessAvg() == e2.GetSuccessAvg()) && (e1.GetLatency() < e2.GetLatency())))
	{
		return e1;
	}
	else
	{
		return e2;
	}
}

Host RoutingTable::bestEntry(Host e1, Host e2, const Key& key)
{
	//priority : PrefixMatching > SuccessAvg > latency
	size_t e1match = e1.GetKey().key_index(key);
	size_t e2match = e2.GetKey().key_index(key);
	if(e1match > e2match)
	{
		return e1;
	}
	if(e2match > e1match)
	{
		return e2;
	}
	return bestEntry(e1,e2);
}
