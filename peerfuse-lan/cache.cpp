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

#include <cstring>
#include <stack>
#include <algorithm>

#include "cache.h"
#include "log.h"
#include "tools.h"
#include "peers_list.h"
#include "session_config.h"
#include "hdd.h"
#include "content_list.h"
#include "scheduler_queue.h"
#include "jobs/job_send_mkfile.h"

Cache cache;

Cache::Cache()
{
}

Cache::~Cache()
{
}

Packet Cache::CreateMkFilePacket(FileEntry* f)
{
	Packet pckt(NET_MKFILE);
	pckt.SetArg(NET_MKFILE_PATH, f->GetFullName());
	pckt.SetArg(NET_MKFILE_MODE, (uint32_t)f->GetAttr().mode);
	pckt.SetArg(NET_MKFILE_UID, f->GetAttr().uid);
	pckt.SetArg(NET_MKFILE_GID, f->GetAttr().gid);
	pckt.SetArg(NET_MKFILE_SIZE, (uint64_t)f->GetAttr().size);
	pckt.SetArg(NET_MKFILE_ACCESS_TIME, (uint32_t)f->GetAttr().atime);
	pckt.SetArg(NET_MKFILE_MODIF_TIME, (uint32_t)f->GetAttr().mtime);
	pckt.SetArg(NET_MKFILE_META_MODIF_TIME, (uint32_t)f->GetAttr().meta_mtime);
	pckt.SetArg(NET_MKFILE_CREATE_TIME, (uint32_t)f->GetAttr().ctime);
	pckt.SetArg(NET_MKFILE_PF_MODE, (uint32_t)f->GetAttr().pf_mode);

	return pckt;
}

void Cache::SendChanges(pf_id peer, time_t last_view)
{
	/* Stack used to store states of each directories */
	std::stack<std::pair<FileMap::const_iterator, FileMap::const_iterator> > stack;

	Lock();
	DirEntry* current_dir = GetTree();

	/* GetFiles() returns a reference, so we can use it directly */
	FileMap::const_iterator it = current_dir->GetFiles().begin();
	FileMap::const_iterator end = current_dir->GetFiles().end();

	while(current_dir)
	{
		log[W_DEBUG] << "- We are in " << current_dir->GetFullName();
		for(; it != end; ++it)
		{
			DirEntry* dir = dynamic_cast<DirEntry*>(it->second);
			log[W_DEBUG] << " |- File " << it->second->GetName();

			/* File is newer than Peer's */
			if(it->second->GetAttr().mtime > last_view)
			{
				Packet packet = CreateMkFilePacket(it->second);
				peers_list.SendMsg(peer, packet);
			}

			if(dir)
			{
				current_dir = dir;
				break;
			}
		}
		/* End of dir, we go back on top folder */
		if(it == end)
		{
			log[W_DEBUG] << " `- end of dir, bye";
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

	Packet packet(NET_END_OF_DIFF);
	peers_list.SendMsg(peer, packet);

	BlockLockMutex lock(&peers_list);
	Peer* p = peers_list.PeerFromID(peer);
	if(p->IsClient())
		p->SendGetStructDiff();

	Unlock();
}

void Cache::MkFile(std::string path, pf_stat stat, IDList sharers, pf_id sender)
{
	BlockLockMutex lock(this);

	std::string filename;
	FileEntry* f = Path2File(path, GET_REMOVED_FILE, &filename);
	DirEntry* dir = dynamic_cast<DirEntry*>(f);

	if(!f)
		throw NoSuchFileOrDir();

	if(filename.empty() || !dir)
	{
		if(f->IsRemoved())
		{
			f->ClearRemoved();
			f->SetAttr(stat);
			hdd.UpdateFile(f);
		}
		else
			throw FileAlreadyExists();
	}
	else
	{
		if(stat.mode & S_IFDIR)
			f = new DirEntry(filename, stat, dir);
		else
			f = new FileEntry(filename, stat, dir);

		log[W_DEBUG] << "Created " << (stat.mode & S_IFDIR ? "dir " : "file ") << filename << " in " << path << ". There are " << dir->GetSize() << " files and directories";
		dir->AddFile(f);
		hdd.MkFile(f);
	}

	/* if it's me who created/modified file */
	if(sender == 0)
		peers_list.Broadcast(CreateMkFilePacket(f));
}

void Cache::SetAttr(std::string path, pf_stat stat, IDList sharers, pf_id sender, bool keep_newest, bool erase_on_modification)
{
	BlockLockMutex lock(this);
	FileEntry* f = Path2File(path, GET_REMOVED_FILE);
	if(!f)
		throw NoSuchFileOrDir();

	time_t last_meta_mtime = f->GetAttr().meta_mtime;

	if(!sender)
		stat.meta_mtime = time(NULL);

	if(stat.meta_mtime >= last_meta_mtime || !keep_newest)
	{
		if(erase_on_modification)
		{
			FileContent& file = content_list.GetFile(path);
			file.Truncate(0);
		}
		else if(stat.size < f->GetAttr().size)
		{
			FileContent& file = content_list.GetFile(path);
			file.Truncate(stat.size);
		}

		f->SetAttr(stat);
		if(f->IsRemoved() && !(f->GetAttr().mode & S_IFDIR))
		{
			FileContent& file = content_list.GetFile(path);
			file.Truncate(0);
		}
	}
	else
	{
		log[W_DEBUG] << "Won't update stats";
		return;
	}

	hdd.UpdateFile(f);

	/* if it's me who created/modified file */
	if(sender == 0)
	{
		/* If there is already a modification at the same date, to prevent flood
		 * we don't broadcast NET_MKFILE now, but we delay it with a job in scheduler,
		 * which will be executed the next second.
		 */
		if(last_meta_mtime == stat.meta_mtime)
		{
			std::string filename = f->GetFullName();
			log[W_DEBUG] << "last_meta_mtime == stat.meta_mtime: we delay NET_MKFILE message";
			if(std::find(delayed_mkfile_send.begin(), delayed_mkfile_send.end(), filename) == delayed_mkfile_send.end())
			{
				delayed_mkfile_send.push_back(filename);
				scheduler_queue.Queue(new JobSendMkFile(filename));
			}
		}
		else
			peers_list.Broadcast(CreateMkFilePacket(f));
	}
	/* TODO: Set attribute on hdd */
	/* hdd.SetAttr(stat); */
}

void Cache::SendMkFile(std::string filename)
{
	BlockLockMutex lock(this);
	FileEntry* f = Path2File(filename, GET_REMOVED_FILE);

	if(!f)
		throw NoSuchFileOrDir();

	log[W_DEBUG] << "Send delayed NET_MKFILE:";
	peers_list.Broadcast(CreateMkFilePacket(f));

	for(std::vector<std::string>::iterator it = delayed_mkfile_send.begin();
		it != delayed_mkfile_send.end();)
	{
		if(*it == filename)
			it = delayed_mkfile_send.erase(it);
		else
			++it;
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

	f->SetRemoved();

	if(!(f->GetAttr().mode & S_IFDIR))
	{
		FileContent& file = content_list.GetFile(path);
		file.Truncate(0);
	}
	content_list.RemoveFile(f->GetFullName());

	peers_list.Broadcast(CreateMkFilePacket(f));
}

void Cache::RenameFile(std::string path, std::string new_path, pf_id sender)
{
	Lock();

	/* TODO: implement it. */

	Unlock();
}
