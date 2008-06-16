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
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * $Id: filedist.cpp 1138 2008-06-08 12:22:09Z romain $
 */

#include <algorithm>
#include "filedist.h"
#include "peers_list.h"
#include "cache.h"
#include "pf_log.h"
#include "network.h"
#include "environment.h"
#include "packet.h"
#include "scheduler_queue.h"
#include "jobs/job_new_connection.h"
#include "jobs/job_send_mkfile.h"

FileDistribution::FileDistribution()
{

}

bool FileDistribution::_is_responsible(const pf_id peer_id, const FileEntry* file, const std::vector<pf_id>& id_l) const
{
	assert(peer_id > 0);
	assert(file != NULL);

	std::vector<pf_id>::const_iterator it;
	std::vector<pf_id>::const_iterator begin = id_l.begin();
	for(it = begin; it != id_l.end() && *it != peer_id; ++it)
		;

	if(it == id_l.end())
	{
		pf_log[W_WARNING] << "FileDistribution::_is_responsible(): " << peer_id << " isn't in list";
		return false;
	}

	size_t id_n = it - begin;
	size_t i = 0;

	for(; i < NB_PEERS_PER_FILE && (file->GetPathSerial() % id_l.size() != (id_n + i) % id_l.size()); ++i)
		;

	pf_log[W_DEBUG] << "FileDist::_is_responsible(" << peer_id << ", " << file->GetFullName() << ", " << file->GetPathSerial() << ") = "
		<< (i < NB_PEERS_PER_FILE ? "true" : "false");
	return (i < NB_PEERS_PER_FILE);
}

bool FileDistribution::IsResponsible(const pf_id peer_id, const FileEntry* file) const
{
	return _is_responsible(peer_id, file, id_list);
}

IDList FileDistribution::GetRespPeers(const FileEntry* f) const
{
	return _get_resp_peers_from_idlist(f, id_list);
}

IDList FileDistribution::_get_resp_peers_from_idlist(const FileEntry* f, const std::vector<pf_id>& idl) const
{
	assert(f != NULL);

	IDList list;
	BlockLockMutex lock(&peers_list);

	std::string debug;

	for(size_t i = 0; i < NB_PEERS_PER_FILE; ++i)
	{
		size_t num = (f->GetPathSerial() > i ? (f->GetPathSerial() - i) : (f->GetPathSerial() + idl.size() - i)) % idl.size();
		pf_id peer = idl[num];
		debug += TypToStr(idl[num]) + ", ";

		/* It is possible that there isn't any Peer object for this
		 * ID. For example, for me.
		 */
		if(peer)
			list.insert(peer);
	}

	pf_log[W_DEBUG] << "FileDist::_get_resp_peers(" << f->GetFullName() << ", " << f->GetPathSerial() << ") = " << debug;

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
			((*it)->GetPathSerial() % id_list.size() != (id_number + i) % id_list.size());
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
	pckt.SetArg(NET_MKFILE_MODE, (uint32_t)f->GetAttr().mode);
	pckt.SetArg(NET_MKFILE_UID, f->GetAttr().uid);
	pckt.SetArg(NET_MKFILE_GID, f->GetAttr().gid);
	pckt.SetArg(NET_MKFILE_SIZE, (uint64_t)f->GetAttr().size);
	pckt.SetArg(NET_MKFILE_ACCESS_TIME, (uint32_t)f->GetAttr().atime);
	pckt.SetArg(NET_MKFILE_MODIF_TIME, (uint32_t)f->GetAttr().mtime);
	pckt.SetArg(NET_MKFILE_META_MODIF_TIME, (uint32_t)f->GetAttr().meta_mtime);
	pckt.SetArg(NET_MKFILE_CREATE_TIME, (uint32_t)f->GetAttr().ctime);
	pckt.SetArg(NET_MKFILE_SHARERS, f->GetSharers());
	pckt.SetArg(NET_MKFILE_PF_MODE, (uint32_t)f->GetAttr().pf_mode);

	return pckt;
}

void FileDistribution::SendDirFiles(DirEntry* dir, pf_id to)
{
	assert(dir != NULL);
	assert(to != 0);

	const FileMap& files = dir->GetFiles();
	for(FileMap::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		Packet pckt = CreateMkFilePacket(it->second);
		pckt.SetDstID(to);
		peers_list.SendMsg(to, pckt);
	}

	Packet pckt = Packet(NET_END_OF_LS, environment.my_id.Get(), to);
	pckt.SetArg(NET_END_OF_LS_PATH, dir->GetFullName());
	peers_list.SendMsg(to, pckt);
}

void FileDistribution::AddFile(FileEntry* f, pf_id sender)
{
	assert(f != NULL);			  /* exists */

	BlockLockMutex net_lock(&peers_list);
	IDList relayed_peers;

	/* I'm responsible of this file. */
	if(IsResponsible(environment.my_id.Get(), f))
	{
		/* this function can be called when this file is updated, so add
		 * it in list only if it isn't already in. */
		if(resp_files.find(f) == resp_files.end())
			resp_files.insert(f);

		/* send it to parent directory's responsibles */
		if(f->GetParent() && (!sender || !IsResponsible(sender, f)))
		{
			IDList peers = GetRespPeers(f->GetParent());

			/* If I'm also responsible of this file's parent, I send message
			 * to all other responsibles. In other case, I only send it
			 * to the a responsible who will relay message to other responsibles.
			 */
			if(IsResponsible(environment.my_id.Get(), f->GetParent()))
			{
				pf_log[W_DEBUG] << "I'm responsible of this file and I'm responsible "
					<< "of parent dir, so I send creation to them.";
				relayed_peers.insert(peers.begin(), peers.end());
			}
			else if(peers.empty() == false)
			{
				pf_log[W_DEBUG] << "I'm responsible of this file so I send it "
					<< " to a parent dir responsible";
				relayed_peers.insert(*peers.begin());
			}
			else
				pf_log[W_WARNING] << "There isn't any responsible for this file !?" << f->GetParent();
		}

		/* Send packet to other responsibles peers if it's me that
		 * created file, or if sender isn't responsible of this
		 * packet. */
		if(!sender || !IsResponsible(sender, f))
		{
			if(!sender)
				pf_log[W_DEBUG] << "I've created this file, so I send it to other resps";
			else
				pf_log[W_DEBUG] << "Sender of this file isn't a responsible of it, so I relay on other resps";
			IDList lp = GetRespPeers(f);
			relayed_peers.insert(lp.begin(), lp.end());
		}
	}
	else if(sender == 0)
	{
		/* I created this file and I'm not the responsible,
		 * so I send it to a responsible.
		 */

		IDList resps = GetRespPeers(f);

		pf_log[W_DEBUG] << "I created this file but I'm not responsible of it. So I send info to a responsible";

		if(resps.empty())
			pf_log[W_WARNING] << "There isn't any responsible of this file !?!?!? " << f;
		else
			relayed_peers.insert(*resps.begin());
	}

	if(f->GetParent() && sender != 0 &&
		IsResponsible(environment.my_id.Get(), f->GetParent()) &&
		!IsResponsible(sender, f->GetParent()))
	{
		/* Someone sent this file to me, and I'm responsible of its parent directory,
		 * but him isn't responsible of it.
		 * So I relay message to others file's parent responsibles.
		 */
		pf_log[W_DEBUG] << "Someone sent this file to me, and I'm responsible of its parent "
			<< "directory, but him isn't responsible of it. So I relay message to "
			<< "others file's parent responsibles";
		IDList peers = GetRespPeers(f->GetParent());
		relayed_peers.insert(peers.begin(), peers.end());
	}

	/* Create and send message to relayed peers. */
	Packet pckt = CreateMkFilePacket(f);

	if(sender != 0)
		relayed_peers.erase(sender);

	if(relayed_peers.empty())
		return;

	std::string filename = f->GetFullName();
	std::map<std::string, IDList>::iterator delayed_mkfile = cache.delayed_mkfile_send.find(filename);
	if(delayed_mkfile != cache.delayed_mkfile_send.end())
	{
		pf_log[W_DEBUG] << "Delay mkfile";
		if(delayed_mkfile->second.empty())
			scheduler_queue.Queue(new JobSendMkFile(filename));
		for(IDList::iterator p = relayed_peers.begin(); p != relayed_peers.end(); ++p)
			delayed_mkfile->second.insert(*p);
	}
	else
	{
		for(IDList::iterator p = relayed_peers.begin(); p != relayed_peers.end(); ++p)
		{
			pckt.SetDstID(*p);
			peers_list.SendMsg(*p, pckt);
		}
	}

}

void FileDistribution::RequestFileRefs(const FileEntry* f)
{
	IDList idlist = f->GetSharers();

	Packet packet(NET_WANT_REF_FILE, environment.my_id.Get());
	packet.SetArg(NET_WANT_REF_FILE_PATH, f->GetFullName());
	for(IDList::iterator id = idlist.begin(); id != idlist.end(); ++id)
	{
		switch(peers_list.WhatIsThisID(*id))
		{
			case PeersList::IS_UNKNOWN:
				continue;
			case PeersList::IS_ON_NETWORK:
				scheduler_queue.Queue(new JobNewConnection(peers_list.GetPeerAddr(*id)));
			case PeersList::IS_CONNECTED:
				packet.SetDstID(*id);
				peers_list.SendMsg(*id, packet);
				break;
		}
	}
}

void FileDistribution::UpdateRespFiles()
{
	/* Store last id list */
	std::vector<pf_id> last_id_list = id_list;

	pf_log[W_DEBUG] << "Updating responsible files";

	/* First set new list of id */
	id_list.clear();
	IDList new_idl = peers_list.GetIDList();
	for(IDList::iterator it = new_idl.begin(); it != new_idl.end(); ++it)
		id_list.push_back(*it);
	id_list.push_back(environment.my_id.Get());

	/* Sort it */
	std::sort(id_list.begin(), id_list.end());

	for(std::vector<pf_id>::const_iterator it = id_list.begin(); it != id_list.end(); ++it)
		pf_log[W_DEBUG] << "     LISTpeer: " << *it;

	/* Get all new files I have responsible */
	FileList last_resp = resp_files;
	resp_files.clear();

	resp_files = GetFiles(environment.my_id.Get());

	for(FileList::iterator it = last_resp.begin(); it != last_resp.end(); ++it)
	{
		pf_log[W_DEBUG] << "- file " << (*it)->GetFullName();

		Packet pckt = CreateMkFilePacket(*it);
		/* get actual peer list */
		IDList peers = GetRespPeers(*it);

		/* TODO do not send message to peers we know they have already this file version */
		for(IDList::iterator peer = peers.begin(); peer != peers.end(); ++peer)
		{
			pf_log[W_DEBUG] << "  -> " << *peer;
			if(!_is_responsible(*peer, *it, last_id_list))
			{
				pf_log[W_DEBUG] << "     sending file to " << *peer << " who wasn't responsible of it.";
				pckt.SetDstID(*peer);
				peers_list.SendMsg(*peer, pckt);
			}
		}
	}
	pf_log[W_DEBUG] << "End of updating.";
}
