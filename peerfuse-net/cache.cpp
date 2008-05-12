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
 * This product includes cryptographic software written by Eric Younganus
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * $Id$
 */

#include <stack>
#include <cstring>
#include <algorithm>

#include "cache.h"
#include "log.h"
#include "tools.h"
#include "peers_list.h"
#include "environment.h"
#include "session_config.h"
#include "hdd.h"
#include "content_list.h"

Cache cache;

Cache::Cache()
{
}

Cache::~Cache()
{
}

FileList Cache::GetAllFiles()
{
	/* Stack used to store states of each directories */
	std::stack<std::pair<FileMap::const_iterator, FileMap::const_iterator> > stack;
	FileList list;

	BlockLockMutex lock(this);
	DirEntry* current_dir = GetTree();

	/* GetFiles() returns a reference, so we can use it directly */
	FileMap::const_iterator it = current_dir->GetFiles().begin();
	FileMap::const_iterator end = current_dir->GetFiles().end();

	list.insert(GetTree());

	while(current_dir)
	{
		for(; it != end; ++it)
		{
			DirEntry* dir = dynamic_cast<DirEntry*>(it->second);

			list.insert(it->second);

			if(dir)
			{
				current_dir = dir;
				break;
			}
		}
		/* End of dir, we go back on top folder */
		if(it == end)
		{
			current_dir = current_dir->GetParent();
			if(current_dir)
			{
				/* Restore state of parent dir */
				std::pair<FileMap::const_iterator, FileMap::const_iterator> iterators = stack.top();
				it = iterators.first;
				end = iterators.second;
				stack.pop();
			}
		}
		/* We enter in a subdir */
		else
		{
			/* Store current state in stack */
			stack.push(std::pair<FileMap::const_iterator, FileMap::const_iterator>(++it, end));

			/* And change iterators to next dir */
			it = current_dir->GetFiles().begin();
			end = current_dir->GetFiles().end();
		}

	}

	return list;
}

void Cache::MkFile(std::string path, pf_stat stat, IDList sharers, pf_id sender)
{
	std::string filename;
	FileEntry* file = 0;
	BlockLockMutex lock(this);
	BlockLockMutex peer_lock(&peers_list);
	Peer* peer = 0;

	if(sender > 0)
		peer = peers_list.PeerFromID(sender);

	try
	{
		file = Path2File(path, CREATE_UNKNOWN_DIRS|GET_REMOVED_FILE, &filename);
		DirEntry* dir = dynamic_cast<DirEntry*>(file);

		if(!file)
			throw NoSuchFileOrDir();

		if(filename.empty() || !dir)
			throw FileAlreadyExists();

		if(sender == 0)
		{
			sharers.insert(environment.my_id.Get());
			stat.uid = environment.my_id.Get();
			stat.gid = environment.my_id.Get();
		}

		if(stat.mode & S_IFDIR)
			file = new DirEntry(filename, stat, dir);
		else
			file = new FileEntry(filename, stat, dir);

		file->SetSharers(sharers);

		dir->AddFile(file);

		log[W_DEBUG] << "Created " << (stat.mode & S_IFDIR ? "dir " : "file ") << filename << " in " << path << ". There are " << dir->GetSize() << " files and directories";

		/* Write file on cache */
		hdd.MkFile(file);

		/* Add file on FileDistribution */
		filedist.AddFile(file, peer);

	}
	catch(Cache::FileAlreadyExists &e)
	{
		/* If it me who wants to create this file, we go out
		 * to let fuse returns an error.
		 */
		if(sender == 0)
		{
			if(file->IsRemoved())
				file->ClearRemoved();
			else
				throw;
		}

		log[W_DEBUG] << "File already exists... Update it.";

		_set_attr(file, stat, peer, sharers);
	}
}

void Cache::RmFile(std::string path)
{
	BlockLockMutex lock(this);
	FileEntry* f = Path2File(path);

	if(!f)
		throw NoSuchFileOrDir();

	// If it's a dir that isn't empty -> return error
	if (f->GetAttr().mode & S_IFDIR)
	{
		DirEntry* d = static_cast<DirEntry*>(f);
		if(d->CountExistantFiles() != 0)
			throw DirNotEmpty();
	}

	if(!f->GetParent())
		throw NoPermission();

	log[W_DEBUG] << "Removed " << path;

	pf_stat stat = f->GetAttr();
	stat.pf_mode |= FilePermissions::S_PF_REMOVED;
	stat.meta_mtime = time(NULL);
	stat.mtime = stat.meta_mtime;

	_set_attr(f, stat, NULL, IDList());

	if(!(f->GetAttr().mode & S_IFDIR))
	{
		FileContent& file = content_list.GetFile(path);
		file.Truncate(0);
	}
	content_list.RemoveFile(f->GetFullName());

}

void Cache::SetAttr(std::string path, pf_stat stat, IDList sharers, pf_id sender, bool keep_newest, bool erase_on_modification)
{
	BlockLockMutex lock(this);
	Peer* peer = 0;
	std::string filename;
	FileEntry* file = Path2File(path, GET_REMOVED_FILE, &filename);
	DirEntry* dir = dynamic_cast<DirEntry*>(file);

	if(!file || (filename.empty() == false && dir))
		throw NoSuchFileOrDir();

	BlockLockMutex peer_lock(&peers_list);
	if(sender > 0)
		peer = peers_list.PeerFromID(sender);

	_set_attr(file, stat, peer, sharers);
}

void Cache::_set_attr(FileEntry* file, pf_stat stat, Peer* sender, IDList sharers, bool keep_newest, bool erase_on_modification)
{
	BlockLockMutex cache_lock(this);
	BlockLockMutex peer_lock(&peers_list);

	if(sender)
	{
		/* This file already exists, but do not panic! We take modifications only if
		 * this file is more recent than mine.
		 */
		time_t dist_ts = stat.meta_mtime;

		if(file->GetAttr().meta_mtime > dist_ts)
		{
			/* My file is more recent than peer's, so I send it a mkfile
			 * to correct this.
			 */
			log[W_DEBUG] << "My file is more recent than peer's, so I correct him";
			Packet pckt = filedist.CreateMkFilePacket(file);
			pckt.SetDstID(sender->GetID());
			sender->SendMsg(pckt);
			return;
		}
		#if 0 /* TODO: for now, if local.meta_mtime == dist.meta_mtime, we update file. */
		else if(file->GetAttr().meta_mtime == dist_ts)
		{
			log[W_DEBUG] << "Same timestamp... What can we do ??";
			/* TODO Same timestamp, what can we do?... */
			return;			  /* same version, go out */
		}
		#endif
		else if(erase_on_modification)
		{
			FileContent& file_content = content_list.GetFile(file->GetFullName());
			file_content.Truncate(0);
		}
		else if(stat.size < file->GetAttr().size)
		{
			FileContent& file_content = content_list.GetFile(file->GetFullName());
			file_content.Truncate(stat.size);
		}

	}
	else
	{
		time_t now = time(NULL);

		/* If local meta_mtime == now, to prevent flood we delay NET_MKFILE message
		 * to the next second
		 */
		if(file->GetAttr().meta_mtime == now)
		{
			std::string filename = file->GetFullName();
			if(delayed_mkfile_send.find(filename) == delayed_mkfile_send.end())
				delayed_mkfile_send[filename] = IDList();
		}
		else
			stat.meta_mtime = now;
	}

	log[W_DEBUG] << "Updating file.";

	if(sharers.empty() == false)
	{
		/* If distant file has not the same content that mine, we get distant
		 * sharers list.
		 * In other case, we only merge list.
		 */
		if(file->GetAttr().mtime != stat.mtime)
		{
			log[W_DEBUG] << file->GetAttr().mtime << " != " << stat.mtime << ": we update sharers";
			file->SetSharers(sharers);
		}
		else
		{
			log[W_DEBUG] << file->GetAttr().mtime << " == " << stat.mtime << ": we merge sharers";
			IDList idlist = file->GetSharers();
			idlist.insert(sharers.begin(), sharers.end());
			file->SetSharers(idlist);
		}
	}

	file->SetAttr(stat);
	if(!(file->GetAttr().mode & S_IFDIR) && file->IsRemoved())
	{
		FileContent& f = content_list.GetFile(file->GetFullName());
		f.Truncate(0);
		file->SetSharers(IDList());
	}

	/* Update file */
	hdd.UpdateFile(file);
	filedist.AddFile(file, sender);
}

void Cache::AddSharer(std::string path, pf_id sharer)
{
	BlockLockMutex lock(this);
	FileEntry* f = Path2File(path);

	if(!f)
		throw NoSuchFileOrDir();

	IDList idl = f->GetSharers();
	if(std::find(idl.begin(), idl.end(), sharer) != idl.end())
		return;

	idl.insert(sharer);
	pf_stat stat = f->GetAttr();
	stat.meta_mtime = time(NULL);

	_set_attr(f, stat, 0, idl);

}

void Cache::SendMkFile(std::string filename)
{
	BlockLockMutex lock(this);

	FileEntry* f = Path2File(filename, GET_REMOVED_FILE);

	if(!f)
		throw NoSuchFileOrDir();

	std::map<std::string, IDList>::iterator it;
	if((it = delayed_mkfile_send.find(filename)) == delayed_mkfile_send.end())
		return;

	log[W_DEBUG] << "Send delayed MKFILE:";
	Packet p = filedist.CreateMkFilePacket(f);
	peers_list.SendMsg(it->second, p);

	delayed_mkfile_send.erase(it);

}

void Cache::RenameFile(std::string path, std::string new_path, pf_id sender)
{
	BlockLockMutex lock(this);

	/* TODO implement this */
}

void Cache::SendDirFiles(std::string path, pf_id to)
{
	BlockLockMutex lock(this);
	DirEntry* dir = dynamic_cast<DirEntry*>(Path2File(path));

	if(!dir)
		throw NoSuchFileOrDir();

	BlockLockMutex net_lock(&peers_list);
	Peer* peer = peers_list.PeerFromID(to);

	filedist.SendDirFiles(dir, peer);
}

#ifndef PF_SERVER_MODE
void Cache::SetReadyForList(std::string path)
{
	BlockLockMutex lock(this);
	dir_incoming[path] = FINISHED;
}

bool Cache::IsReadyForList(std::string path)
{
	BlockLockMutex lock(this);

	std::map<std::string, incoming_states_t>::iterator incoming = dir_incoming.find(path);
	if(incoming == dir_incoming.end() || incoming->second == FINISHED)
		return true;
	else
		return false;
}

void Cache::FillReadDir(const char* _path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	std::string path = _path;
	bool wait_for_content = false;

	/* Open a block only to use BlockLockMutex. */
	do
	{
		/* Do NOT lock mutex outside of this block! It might freeze Cache because
		 * of the loop
		 */
		BlockLockMutex lock(this);

		DirEntry* dir = dynamic_cast<DirEntry*>(Path2File(path));

		if(!dir)
			throw NoSuchFileOrDir();

		path = dir->GetFullName();

		/* If I'm not responsible of this directory, send a request to a responsible
		 * to get content.
		 */
		if(!filedist.IsResponsible(environment.my_id.Get(), dir))
		{
			BlockLockMutex lock(&peers_list);
			std::set<Peer*> peers = filedist.GetRespPeers(dir);

			if(peers.empty())
				log[W_WARNING] << "Cache::FillReadDir: there isn't any responsible of " << dir->GetFullName() << " ??";
			else
			{
				std::map<std::string, incoming_states_t>::iterator incoming = dir_incoming.find(path);
				if(incoming == dir_incoming.end() || incoming->second != GETTING)
				{
					Peer* resp = *peers.begin();

					Packet pckt = Packet(NET_LS_DIR, environment.my_id.Get(), resp->GetID());
					pckt.SetArg(NET_LS_DIR_PATH, path);
					resp->SendMsg(pckt);

					dir_incoming[path] = GETTING;
				}

				wait_for_content = true;
			}

		}

	}
	while(0);

	if(wait_for_content)
	{
		while(!IsReadyForList(path))
			usleep(10000);		  /* 0.01 sec */

	}

	CacheBase::FillReadDir(_path, buf, filler, offset, fi);
}
#endif						  /* PF_SERVER_MODE */

void Cache::UpdateRespFiles()
{
	BlockLockMutex lock(this);
	filedist.UpdateRespFiles();
}

void Cache::RequestFileRefs(std::string filename)
{
	BlockLockMutex lock(this);
	FileEntry* f = Path2File(filename);

	if(!f)
		throw NoSuchFileOrDir();

	filedist.RequestFileRefs(f);
}
