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

#include <algorithm>
#include "filedist.h"
#include "network.h"
#include "cache.h"
#include "log.h"

FileDistribution::FileDistribution()
{

}

bool FileDistribution::_is_responsible(const id_t peer_id, const FileEntry* file, const std::vector<id_t>& id_list) const
{
	assert(peer_id > 0);
	assert(file != NULL);

	std::vector<id_t>::const_iterator it;
	std::vector<id_t>::const_iterator begin = id_list.begin();
	for(it = begin; it != id_list.end() && *it != peer_id; ++it)
		;

	if(it == id_list.end())
		return false;

	size_t id_n = it - begin;
	size_t i = 0;

	for(; i < NB_PEERS_PER_FILE && file->GetPathSerial() % id_list.size() != id_n % id_list.size(); ++i)
		;

	return (i < NB_PEERS_PER_FILE);
}

bool FileDistribution::IsResponsible(const id_t peer_id, const FileEntry* file) const
{
	return _is_responsible(peer_id, file, id_list);
}

std::set<Peer*> FileDistribution::GetPeers(const FileEntry* f) const
{
	return _get_peers_from_idlist(f, id_list);
}

std::set<Peer*> FileDistribution::_get_peers_from_idlist(const FileEntry* f, const std::vector<id_t>& idl) const
{
	assert(f != NULL);

	std::set<Peer*> list;

	for(size_t i = 0; i < NB_PEERS_PER_FILE; ++i)
	{
		Peer* peer = net.ID2Peer(idl[(f->GetPathSerial()+i) % idl.size()]);

		/* It is possible that there isn't any Peer object for this
		 * ID. For example, for me.
		 */
		if(peer)
			list.insert(peer);
	}

	return list;
}

FileList FileDistribution::GetFiles(id_t id) const
{
	FileList files = cache.GetAllFiles();
	FileList result;

	size_t id_number;
	std::vector<id_t>::const_iterator it;
	std::vector<id_t>::const_iterator begin = id_list.begin();

	for(it = begin; it != id_list.end() && *it != id;  ++it)
		;

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
		    ++i)
			;

		if(i < NB_PEERS_PER_FILE)
			result.insert(*it);
	}

	return result;
}

void FileDistribution::AddFile(FileEntry* f, unsigned int flags)
{
	assert(f != NULL);			  /* exists */
	assert(f->GetParent() != NULL);		  /* isn't the root dir */

	if(IsResponsible(net.GetMyID(), f))
	{
		resp_files.insert(f);

		if(flags & M_PROPAGATE)
		{
			std::set<Peer*> peers = GetPeers(f);
			Packet pckt = cache.CreateMkFilePacket(f);

			for(std::set<Peer*>::iterator p = peers.begin(); p != peers.end(); ++p)
			{
				pckt.SetDstID((*p)->GetID());
				(*p)->SendMsg(pckt);
			}
		}
	}
}

void FileDistribution::RemoveFile(FileEntry* f, unsigned int flags)
{
	assert(f != NULL);			  /* exists */
	assert(f->GetParent() != NULL);		  /* isn't the root dir */

	std::set<Peer*> peers = GetPeers(f);
	Packet pckt = cache.CreateRmFilePacket(f);

	for(std::set<Peer*>::iterator p = peers.begin(); p != peers.end(); ++p)
	{
		pckt.SetDstID((*p)->GetID());
		(*p)->SendMsg(pckt);
	}

	resp_files.erase(f);
}

void FileDistribution::UpdateRespFiles()
{
	/* Store last id list */
	std::vector<id_t> last_id_list = id_list;

	/* First set new list of id */
	id_list.clear();
	PeerList peers = net.GetPeerList();

	id_list.push_back(net.GetMyID());
	for(PeerList::const_iterator it = peers.begin(); it != peers.end(); ++it)
		id_list.push_back((*it)->GetID());

	/* Sort it */
	std::sort(id_list.begin(), id_list.end());

	/* Get all new files I have responsible */
	FileList last_resp = resp_files;
	resp_files.clear();

	resp_files = GetFiles(net.GetMyID());

	for(FileList::iterator it = last_resp.begin(); it != last_resp.end(); ++it)
	{
		Packet pckt = cache.CreateMkFilePacket(*it);
		/* get actual peer list */
		std::set<Peer*> peers = GetPeers(*it);

		log[W_DEBUG] << "- file " << (*it)->GetFullName();

		/* TODO do not send message to peers we know they have already this file version */
		for(std::set<Peer*>::iterator peer = peers.begin(); peer != peers.end(); ++peer)
		{
			log[W_DEBUG] << "  -> " << *peer;
			if(!_is_responsible((*peer)->GetID(), *it, last_id_list))
			{
				pckt.SetDstID((*peer)->GetID());
				(*peer)->SendMsg(pckt);
			}
		}
	}
}
