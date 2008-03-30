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
#include "peer.h"
#include "peers_list.h"

Cache cache;

Cache::Cache()
			: tree("", NULL)
{
}

Cache::~Cache()
{
}

void Cache::Load(std::string hd_path)
{
	Lock();
	try
	{
		hdd.BuildTree(GetTree(), hd_path);
	}
	catch(...)
	{					  /* U G L Y, I think we MUST find a solution. */
		Unlock();
		throw;
	}
	Unlock();
}

FileEntry* Cache::Path2File(std::string path, std::string* filename)
{
	Lock();
	DirEntry* current_dir = &tree;

	std::string name;

	while((name = stringtok(path, "/")).empty() == false)
	{
		FileEntry* tmp = current_dir->GetFile(name);
		if(!tmp)
		{
			if(path.find('/') == std::string::npos && filename)
			{
				*filename = name;
				Unlock();
				return current_dir;
			}
			Unlock();
			return NULL;
		}

		if(!(current_dir = dynamic_cast<DirEntry*>(tmp)))
		{
			if(path.empty())
			{
				Unlock();
				return tmp;
			}

			Unlock();
			return NULL;
		}
	}

	Unlock();
	return current_dir;
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

void Cache::SendChanges(Peer* p, time_t last_view)
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
				p->SendMsg(CreateMkFilePacket(it->second));

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

	Unlock();
	p->SendMsg(Packet(NET_END_OF_DIFF));
}

void Cache::MkFile(std::string path, pf_stat stat, Peer* sender)
{
	BlockLockMutex lock(this);
	std::string filename;
	DirEntry* dir = dynamic_cast<DirEntry*>(Path2File(path, &filename));

	if(!dir)
		throw NoSuchFileOrDir();

	if(filename.empty())
		throw FileAlreadyExists();

	FileEntry* file;

	if(stat.mode & S_IFDIR)
		file = new DirEntry(filename, dir);
	else
		file = new FileEntry(filename, dir);

	/* Copy stat only if it's me who created this file! */
	if(sender != NULL)
		file->stat = stat;

	dir->AddFile(file);

	log[W_DEBUG] << "Created " << (stat.mode & S_IFDIR ? "dir " : "file ") << filename << " in " << path << ". There are " << dir->GetSize() << " files and directories";

	hdd.MkFile(file);

	/* if it's me who created file */
	if(sender == NULL)
		peers_list.Broadcast(CreateMkFilePacket(file));
}

void Cache::RmFile(std::string path, Peer* sender)
{
	BlockLockMutex lock(this);
	FileEntry* f = Path2File(path);

	// If it's a dir that isn't empty -> return error
	if (f->stat.mode & S_IFDIR)
	{
		DirEntry* d = static_cast<DirEntry*>(f);
		if(d->GetSize() != 0)
			throw DirNotEmpty();
	}

	if(!f)
		throw NoSuchFileOrDir();

	if(!f->GetParent())
		throw NoPermission();

	log[W_DEBUG] << "Removed " << path;

	hdd.RmFile(f);

	/* Send before removing file */
	if(sender == NULL)
		peers_list.Broadcast(CreateRmFilePacket(f));

	f->GetParent()->RemFile(f);

	delete f;
}

void Cache::RenameFile(std::string path, std::string new_path, Peer* sender)
{
	Lock();

	/* TODO: implement it. */

	Unlock();
}

void Cache::ChOwn(std::string path, uid_t uid, gid_t gid)
{
	Lock();

	FileEntry* file = Path2File(path);
	if(!file)
	{
		Unlock();
		throw NoSuchFileOrDir();
	}

	file->stat.uid = uid;
	file->stat.gid = gid;
	file->stat.meta_mtime = time(NULL);

	/* TODO propagate it */

	Unlock();
}

void Cache::ChMod(std::string path, mode_t mode)
{
	Lock();

	FileEntry* file = Path2File(path);
	if(!file)
	{
		Unlock();
		throw NoSuchFileOrDir();
	}

	file->stat.mode = mode;
	file->stat.meta_mtime = time(NULL);

	/* TODO propagate it */

	Unlock();
}

pf_stat Cache::GetAttr(std::string path)
{
	pf_stat stat;

	Lock();

	FileEntry* file = Path2File(path);
	if(!file)
	{
		Unlock();
		throw NoSuchFileOrDir();
	}

	stat = file->stat;
	Unlock();

	return stat;
}

void Cache::FillReadDir(const char* path, void *buf, fuse_fill_dir_t filler,
			off_t offset, struct fuse_file_info *fi)
{
	Lock();
	DirEntry* dir = dynamic_cast<DirEntry*>(cache.Path2File(path));

	if(!dir)
	{
		Unlock();
		throw NoSuchFileOrDir();
	}

	FileMap files = dir->GetFiles();
	for(FileMap::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		struct stat st;
		memset(&st, 0, sizeof st);
		/*st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;*/

		if(filler(buf, it->second->GetName().c_str(), &st, 0))
			break;
	}

	Unlock();
}
