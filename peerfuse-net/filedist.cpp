/*
 * Copyright(C) 2008 Laurent Defert, Romain Bignon
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
 * $Id$
 */

#include "filedist.h"
#include "network.h"
#include "cache.h"

FileDistribution::FileDistribution()
{

}

std::vector<Peer*> FileDistribution::GetPeers(const FileEntry* f) const
{
	std::vector<Peer*> list;

	return list;
}

std::vector<FileEntry*> FileDistribution::GetFiles(id_t id) const
{
	std::vector<FileEntry*> files = cache.GetAllFiles();
	std::vector<FileEntry*> result;

	size_t id_number;
	std::vector<id_t>::const_iterator it;
	std::vector<id_t>::const_iterator begin = peers.begin();
	for(it = begin; it != peers.end() && *it != id;  ++it);

	if(it == peers.end())
		return files;

	id_number = it - begin;

	for(std::vector<FileEntry*>::iterator it = files.begin();
	    it != files.end();
	    ++it)
	{
		size_t i = 0;
		for(; i < NB_PEERS_PER_FILE &&
		      ((*it)->GetPathSerial() % peers.size() != id_number % peers.size());
		    ++i);

		if(i < NB_PEERS_PER_FILE)
			result.push_back(*it);
	}

	return result;
}

