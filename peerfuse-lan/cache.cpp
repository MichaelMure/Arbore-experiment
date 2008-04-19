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

#include <cstring>
#include <stack>

#include "cache.h"
#include "log.h"
#include "tools.h"
#include "peers_list.h"
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

Packet Cache::CreateMkFilePacket(FileEntry* f)
{
	Packet pckt(NET_MKFILE);
	pckt.SetArg(NET_MKFILE_PATH, f->GetFullName());
	pckt.SetArg(NET_MKFILE_MODE, f->stat.mode);
	pckt.SetArg(NET_MKFILE_UID, f->stat.uid);
	pckt.SetArg(NET_MKFILE_GID, f->stat.gid);
	pckt.SetArg(NET_MKFILE_SIZE, (uint64_t)f->stat.size);
	pckt.SetArg(NET_MKFILE_ACCESS_TIME, (uint32_t)f->stat.atime);
	pckt.SetArg(NET_MKFILE_MODIF_TIME, (uint32_t)f->stat.mtime);
	pckt.SetArg(NET_MKFILE_META_MODIF_TIME, (uint32_t)f->stat.meta_mtime);
	pckt.SetArg(NET_MKFILE_CREATE_TIME, (uint32_t)f->stat.ctime);

	return pckt;
}

Packet Cache::CreateRmFilePacket(FileEntry* f)
{
	Packet pckt(NET_RMFILE);
	pckt.SetArg(NET_RMFILE_PATH, f->GetFullName());

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
			if(it->second->stat.mtime > last_view)
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

	FileEntry* f = Path2File(path);
	if(!f)
	{
		// The file doesn't exist -> create it
		std::string filename;
		DirEntry* dir = dynamic_cast<DirEntry*>(Path2File(path, &filename));
	
		if(!dir)
			throw NoSuchFileOrDir();
	
	//	TODO: ????
	//	if(filename.empty())
	//		throw FileAlreadyExists();
	
		if(stat.mode & S_IFDIR)
			f = new DirEntry(filename, stat, dir);
		else
			f = new FileEntry(filename, stat, dir);

		log[W_DEBUG] << "Created " << (stat.mode & S_IFDIR ? "dir " : "file ") << filename << " in " << path << ". There are " << dir->GetSize() << " files and directories";
		dir->AddFile(f);
		hdd.MkFile(f);
		tree_cfg.Set(path + "#size", (uint32_t)f->stat.size);
		tree_cfg.Set(path + "#meta", (uint32_t)f->stat.meta_mtime);
	}
	else
	{
		// Update attribute
		if(stat.size != f->stat.size)
			tree_cfg.Set(path + "#size", (uint32_t)f->stat.size);
		if(stat.meta_mtime != f->stat.meta_mtime)
			tree_cfg.Set(path + "#meta", (uint32_t)f->stat.meta_mtime);
		f->stat = stat;
	}

	/* TODO: Set attribute on hdd */
	/* hdd.SetAttr(stat); */

	/* if it's me who created/modified file */
	if(sender == 0)
		peers_list.Broadcast(CreateMkFilePacket(f));
}

void Cache::RmFile(std::string path, pf_id sender)
{
	BlockLockMutex lock(this);
	FileEntry* f = Path2File(path);

	if(!f)
		throw NoSuchFileOrDir();

	// If it's a dir that isn't empty -> return error
	if (f->stat.mode & S_IFDIR)
	{
		DirEntry* d = static_cast<DirEntry*>(f);
		if(d->GetSize() != 0)
			throw DirNotEmpty();
	}

	if(!f->GetParent())
		throw NoPermission();

	log[W_DEBUG] << "Removed " << path;

	hdd.RmFile(f);
	content_list.RemoveFile(f->GetFullName());

	/* Send before removing file */
	if(sender == 0)
		peers_list.Broadcast(CreateRmFilePacket(f));

	f->GetParent()->RemFile(f);

	delete f;
}

void Cache::RenameFile(std::string path, std::string new_path, pf_id sender)
{
	Lock();

	/* TODO: implement it. */

	Unlock();
}

