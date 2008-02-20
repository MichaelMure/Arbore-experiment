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

	for(size_t i = 0; i < NB_PEERS_PER_FILE; ++i)
		list.push_back(net.ID2Peer(id_list[(f->GetPathSerial()+i) % id_list.size()]));

	return list;
}

FileList FileDistribution::GetFiles(id_t id) const
{
	FileList files = cache.GetAllFiles();
	FileList result;

	size_t id_number;
	std::vector<id_t>::const_iterator it;
	std::vector<id_t>::const_iterator begin = id_list.begin();
	for(it = begin; it != id_list.end() && *it != id;  ++it);

	if(it == id_list.end())
		return files;

	id_number = it - begin;

	for(FileList::iterator it = files.begin();
	    it != files.end();
	    ++it)
	{
		size_t i = 0;
		for(; i < NB_PEERS_PER_FILE &&
		      ((*it)->GetPathSerial() % id_list.size() != id_number % id_list.size());
		    ++i);

		if(i < NB_PEERS_PER_FILE)
			result.insert(*it);
	}

	return result;
}

void FileDistribution::UpdateRespFiles()
{
	/* First set new list of id */
	id_list.clear();
	PeerList peers = net.GetPeerList();

	id_list.push_back(net.GetMyID());
	for(PeerList::const_iterator it = peers.begin(); it != peers.end(); ++it)
		id_list.push_back((*it)->GetID());

	/* Sort it */
	std::sort(id_list.begin(), id_list.end());

	FileList last_resp = resp_files;
	resp_files.clear();

	/* Get all new files I have responsible */
	resp_files = GetFiles(net.GetMyID());

	std::vector<FileEntry*> diff;

	/* Store un last_resp all files I was responsible
	 * but I am not anymore */
	set_difference(last_resp.begin(), last_resp.end(),
	               resp_files.begin(), resp_files.end(),
		       diff.begin());

	/* Now send all files to other peoples which now
	 * have my old files.
	 */
}
