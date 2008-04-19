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
#include "peers_list.h"
#include "cache.h"
#include "log.h"
#include "network.h"
#include "environment.h"
#include "packet.h"

FileDistribution::FileDistribution()
{

}

bool FileDistribution::_is_responsible(const pf_id peer_id, const FileEntry* file, const std::vector<pf_id>& id_list) const
{
	assert(peer_id > 0);
	assert(file != NULL);

	std::vector<pf_id>::const_iterator it;
	std::vector<pf_id>::const_iterator begin = id_list.begin();
	for(it = begin; it != id_list.end() && *it != peer_id; ++it)
		;

	if(it == id_list.end())
	{
		log[W_WARNING] << "FileDistribution::_is_responsible(): " << peer_id << " isn't in list";
		return false;
	}

	size_t id_n = it - begin;
	size_t i = 0;

	for(; i < NB_PEERS_PER_FILE && file->GetPathSerial() % id_list.size() != id_n % id_list.size(); ++i)
		;

	return (i < NB_PEERS_PER_FILE);
}

bool FileDistribution::IsResponsible(const pf_id peer_id, const FileEntry* file) const
{
	return _is_responsible(peer_id, file, id_list);
}

std::set<Peer*> FileDistribution::GetRespPeers(const FileEntry* f) const
{
	return _get_resp_peers_from_idlist(f, id_list);
}

std::set<Peer*> FileDistribution::_get_resp_peers_from_idlist(const FileEntry* f, const std::vector<pf_id>& idl) const
{
	assert(f != NULL);

	std::set<Peer*> list;

	for(size_t i = 0; i < NB_PEERS_PER_FILE; ++i)
	{
		BlockLockMutex lock(&peers_list);
		Peer* peer = peers_list.PeerFromID(idl[(f->GetPathSerial()+i) % idl.size()]);

		/* It is possible that there isn't any Peer object for this
		 * ID. For example, for me.
		 */
		if(peer)
			list.insert(peer);
	}

	return list;
}

FileList FileDistribution::GetFiles(pf_id id) const
{
	FileList files = cache.GetAllFiles();
	FileList result;

	size_t id_number;
	std::vector<pf_id>::const_iterator it;
	std::vector<pf_id>::const_iterator begin = id_list.begin();

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

Packet FileDistribution::CreateMkFilePacket(FileEntry* f)
{
	Packet pckt(NET_MKFILE, environment.my_id.Get());
	pckt.SetArg(NET_MKFILE_PATH, f->GetFullName());
	pckt.SetArg(NET_MKFILE_MODE, f->GetAttr().mode);
	pckt.SetArg(NET_MKFILE_UID, f->GetAttr().uid);
	pckt.SetArg(NET_MKFILE_GID, f->GetAttr().gid);
	pckt.SetArg(NET_MKFILE_SIZE, (uint64_t)f->GetAttr().size);
	pckt.SetArg(NET_MKFILE_ACCESS_TIME, (uint32_t)f->GetAttr().atime);
	pckt.SetArg(NET_MKFILE_MODIF_TIME, (uint32_t)f->GetAttr().mtime);
	pckt.SetArg(NET_MKFILE_META_MODIF_TIME, (uint32_t)f->GetAttr().meta_mtime);
	pckt.SetArg(NET_MKFILE_CREATE_TIME, (uint32_t)f->GetAttr().ctime);
	pckt.SetArg(NET_MKFILE_SHARERS, f->GetSharers());

	return pckt;
}

void FileDistribution::AddFile(FileEntry* f, Peer* sender)
{
	assert(f != NULL);			  /* exists */

	std::set<Peer*> relayed_peers;

	/* I'm responsible of this file. */
	if(IsResponsible(environment.my_id.Get(), f))
	{
		/* this function can be called when this file is updated, so add
		 * it in list only if it isn't already in. */
		if(resp_files.find(f) == resp_files.end())
		{
			resp_files.insert(f);

			/* if file has just been created, send it to parent
			 * directory's responsibles */
			if(sender != NULL && f->GetParent() && !IsResponsible(sender->GetID(), f))
			{
				std::set<Peer*> peers = GetRespPeers(f->GetParent());

				/* If I'm also responsible of this file's parent, I send message
				 * to all other responsibles. In other case, I only send it
				 * to the a responsible who will relay message to other responsibles.
				 */
				if(IsResponsible(environment.my_id.Get(), f->GetParent()))
				{
					log[W_DEBUG] << "I'm responsible of this file which has been created, and I'm responsible "
						<< "of parent dir, so I send creation to them.";
					relayed_peers.insert(peers.begin(), peers.end());
				}
				else if(peers.empty() == false)
				{
					log[W_DEBUG] << "I'm responsible of this file which has been created, so I send it "
						<< " to a parent dir responsible";
					relayed_peers.insert(*peers.begin());
				}
				else
					log[W_WARNING] << "There isn't any responsible for this file !?" << f->GetParent();
			}
		}

		/* Send packet to other responsibles peers if it's me that
		 * created file, or if sender isn't responsible of this
		 * packet. */
		if(!sender || !IsResponsible(sender->GetID(), f))
		{
			if(!sender)
				log[W_DEBUG] << "I've created this file, so I send it to other resps";
			else
				log[W_DEBUG] << "Sender of this file isn't a responsible of it, so I relay on other resps";
			std::set<Peer*> lp = GetRespPeers(f);
			relayed_peers.insert(lp.begin(), lp.end());
		}
	}
	else if(sender == NULL)
	{
		/* I created this file and I'm not the responsible,
		 * so I send it to a responsible.
		 */

		std::set<Peer*> resps = GetRespPeers(f);

		log[W_DEBUG] << "I created this file but I'm not responsible of it. So I send info to a responsible";

		if(resps.empty())
			log[W_WARNING] << "There isn't any responsible of this file !?!?!? " << f;
		else
			relayed_peers.insert(*resps.begin());
	}

	if(f->GetParent() && sender != NULL &&
		IsResponsible(environment.my_id.Get(), f->GetParent()) &&
		!IsResponsible(sender->GetID(), f->GetParent()))
	{
		/* Someone sent this file to me, and I'm responsible of its parent directory,
		 * but him isn't responsible of it.
		 * So I relay message to others file's parent responsibles.
		 */
		log[W_DEBUG] << "Someone sent this file to me, and I'm responsible of its parent "
			<< "directory, but him isn't responsible of it. So I relay message to "
			<< "others file's parent responsibles";
		std::set<Peer*> peers = GetRespPeers(f->GetParent());
		relayed_peers.insert(peers.begin(), peers.end());
	}

	/* Create and send message to relayed peers. */
	Packet pckt = CreateMkFilePacket(f);

	if(sender != NULL)
		relayed_peers.erase(sender);

	for(std::set<Peer*>::iterator p = relayed_peers.begin(); p != relayed_peers.end(); ++p)
	{
		pckt.SetDstID((*p)->GetID());
		(*p)->SendMsg(pckt);
	}

}

Packet FileDistribution::CreateRmFilePacket(FileEntry* f)
{
	Packet pckt(NET_RMFILE, environment.my_id.Get());
	pckt.SetArg(NET_RMFILE_PATH, f->GetFullName());

	return pckt;
}

void FileDistribution::RemoveFile(FileEntry* f, Peer* sender)
{
	assert(f != NULL);			  /* exists */
	assert(f->GetParent() != NULL);		  /* isn't the root dir */

	std::set<Peer*> peers = GetRespPeers(f);
	Packet pckt = CreateRmFilePacket(f);

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
	std::vector<pf_id> last_id_list = id_list;

	log[W_DEBUG] << "Updating responsible files";

	/* First set new list of id */
	id_list.clear();
	id_list.push_back(environment.my_id.Get());

	BlockLockMutex lock(&peers_list);
	for(PeersList::const_iterator it = peers_list.begin(); it != peers_list.end(); ++it)
		id_list.push_back((*it)->GetID());

	/* Sort it */
	std::sort(id_list.begin(), id_list.end());

	/* Get all new files I have responsible */
	FileList last_resp = resp_files;
	resp_files.clear();

	resp_files = GetFiles(environment.my_id.Get());

	for(FileList::iterator it = last_resp.begin(); it != last_resp.end(); ++it)
	{
		Packet pckt = CreateMkFilePacket(*it);
		/* get actual peer list */
		std::set<Peer*> peers = GetRespPeers(*it);

		log[W_DEBUG] << "- file " << (*it)->GetFullName();

		/* TODO do not send message to peers we know they have already this file version */
		for(std::set<Peer*>::iterator peer = peers.begin(); peer != peers.end(); ++peer)
		{
			log[W_DEBUG] << "  -> " << *peer;
			if(!_is_responsible((*peer)->GetID(), *it, last_id_list))
			{
				log[W_DEBUG] << "     sending file to " << (*peer)->GetID() << " who wasn't responsible of it.";
				pckt.SetDstID((*peer)->GetID());
				(*peer)->SendMsg(pckt);
			}
		}
	}
	log[W_DEBUG] << "End of updating.";
}
