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

#include <stack>
#include <cstring>

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

	Lock();
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

	Unlock();

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
		file = Path2File(path, &filename);
		DirEntry* dir = dynamic_cast<DirEntry*>(file);

		if(!file)
			throw NoSuchFileOrDir();

		if(filename.empty() || !dir)
			throw FileAlreadyExists();

		if(sender == 0)
		{
			sharers.push_back(environment.my_id.Get());
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
			throw;

		log[W_DEBUG] << "File already exists... Update it.";

		_set_attr(file, stat, peer, sharers);
	}
}

void Cache::SetAttr(std::string path, pf_stat stat, IDList sharer, pf_id sender, bool keep_newest)
{
	Peer* peer = 0;

	std::string filename;
	FileEntry* file = Path2File(path, &filename);
	DirEntry* dir = dynamic_cast<DirEntry*>(file);

	if(!file || (filename.empty() == false && dir))
		throw NoSuchFileOrDir();

	if(sender > 0)
		peer = peers_list.PeerFromID(sender);

	_set_attr(file, stat, peer, IDList());
}

void Cache::_set_attr(FileEntry* file, pf_stat stat, Peer* sender, IDList sharers)
{
	BlockLockMutex cache_lock(this);
	BlockLockMutex peer_lock(&peers_list);

	if(sender)
	{
		/* This file already exists, but do not panic! We take modifications only if
		 * this file is more recent than mine.
		 */
		time_t dist_ts = sender->Timestamp(stat.mtime);

		if(file->GetAttr().mtime > dist_ts)
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
		else if(file->GetAttr().mtime == dist_ts)
		{
			log[W_DEBUG] << "Same timestamp... What can we do ??";
			/* TODO Same timestamp, what can we do?... */
			return;			  /* same version, go out */
		}
	}

	file->SetAttr(stat);

	if(sharers.empty() == false)
		file->SetSharers(sharers);

	/* Update file */
	hdd.UpdateFile(file);
	filedist.AddFile(file, sender);

}

void Cache::RmFile(std::string path, pf_id sender)
{
	BlockLockMutex lock(this);
	FileEntry* f = Path2File(path);

	if(!f)
		throw NoSuchFileOrDir();

	// If it's a dir that isn't empty -> return error
	if (f->GetAttr().mode & S_IFDIR)
	{
		DirEntry* d = static_cast<DirEntry*>(f);
		if(d->GetSize() != 0)
			throw DirNotEmpty();
	}

	if(!f->GetParent())
		throw NoPermission();

	log[W_DEBUG] << "Removed " << path;

	hdd.RmFile(f);

	BlockLockMutex net_lock(&peers_list);
	filedist.RemoveFile(f, peers_list.PeerFromID(sender));

	content_list.RemoveFile(f->GetFullName());

	f->GetParent()->RemFile(f);
	delete f;
}

void Cache::AddSharer(std::string path, pf_id sender)
{
	BlockLockMutex lock(this);
	FileEntry* f = Path2File(path);

	if(!f)
		throw NoSuchFileOrDir();

	filedist.AddSharer(f, sender);
}

void Cache::RenameFile(std::string path, std::string new_path, pf_id sender)
{
	BlockLockMutex lock(this);

	/* TODO implement this */
}

void Cache::UpdateRespFiles()
{
	BlockLockMutex lock(this);
	filedist.UpdateRespFiles();
}
