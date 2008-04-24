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

#include <string.h>
#include "cache_base.h"
#include "content_list.h"
#include "file_content.h"
#include "hdd.h"
#include "session_config.h"

FileEntry* CacheBase::Path2File(std::string path, unsigned int flags, std::string* filename)
{
	BlockLockMutex lock(this);
	DirEntry* current_dir = &tree;

	std::string name;

	while((name = stringtok(path, "/")).empty() == false)
	{
		FileEntry* tmp = current_dir->GetFile(name);
		if(!tmp || tmp->IsRemoved())
		{
			if(path.empty())
			{
				/* we are in last dir, but this file doesn't exist */
				if(tmp && flags & RESTORE_REMOVED_FILE)
				{
					tmp->ClearRemoved();
					return tmp;
				}
				else if(filename)
				{
					*filename = name;
					return current_dir;
				}
				else
					return NULL;
			}

			/* we aren't in last dir, so the path isn't found. */
			if(flags & CREATE_UNKNOWN_DIRS)
			{
				if(!tmp)
				{
					pf_stat stat;
					stat.uid = 0;
					stat.gid = 0;
					tmp = new DirEntry(name, stat, current_dir);
					current_dir->AddFile(tmp);
				}
				else
					tmp->ClearRemoved();
			}
			else
				return NULL;
		}

		if(!(current_dir = dynamic_cast<DirEntry*>(tmp)))
		{
			/* This isn't a directory. */
			if(path.empty())
			{
				/* We are on last dir, so it is a file. */
				return tmp;
			}
			/* it isn't a file in path, so the path isn't found. */

			return NULL;
		}
	}

	return current_dir;
}

void CacheBase::Load(std::string hd_path)
{
	BlockLockMutex lock(this);
	try
	{
		hdd.BuildTree(GetTree(), hd_path);
	}
	catch(...)
	{
		throw;
	}
}

void CacheBase::ChOwn(std::string path, uid_t uid, gid_t gid)
{
	BlockLockMutex lock(this);

	FileEntry* file = Path2File(path);
	if(!file)
		throw NoSuchFileOrDir();

	pf_stat stat = file->GetAttr();
	stat.uid = uid;
	stat.gid = gid;
	stat.meta_mtime = time(NULL);
	SetAttr(path, stat);
}

void CacheBase::ChMod(std::string path, mode_t mode)
{
	BlockLockMutex lock(this);

	FileEntry* file = Path2File(path);
	if(!file)
		throw NoSuchFileOrDir();

	pf_stat stat = file->GetAttr();
	stat.mode = mode;
	stat.meta_mtime = time(NULL);
	SetAttr(path, stat);
}

pf_stat CacheBase::GetAttr(std::string path)
{
	BlockLockMutex lock(this);

	FileEntry* file = Path2File(path);
	if(!file)
		throw NoSuchFileOrDir();

	return file->GetAttr();
}

#ifndef PF_SERVER_MODE
void CacheBase::FillReadDir(const char* path, void *buf, fuse_fill_dir_t filler,
			off_t offset, struct fuse_file_info *fi)
{
	BlockLockMutex lock(this);
	DirEntry* dir = dynamic_cast<DirEntry*>(Path2File(path));

	if(!dir)
		throw NoSuchFileOrDir();

	FileMap files = dir->GetFiles();
	for(FileMap::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		if(it->second->IsRemoved())
			continue;

		struct stat st;
		memset(&st, 0, sizeof st);
		/*st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;*/

		if(filler(buf, it->second->GetName().c_str(), &st, 0))
			break;
	}
}
#endif						  /* PF_SERVER_MODE */

void CacheBase::Write(std::string path, const char* buf, size_t size, off_t off)
{
	FileContent& file = content_list.GetFile(path);
	FileChunk chunk(buf, off, size);
	file.SetChunk(chunk);

	/* No need to lock cache, we don't touch its members */
	pf_stat stat = GetAttr(path);
	time_t now = time(NULL);
	stat.meta_mtime = now;
	stat.mtime = now;
	stat.ctime = now;
	if(off + (off_t)size > (off_t)stat.size)
	{
		stat.size = (size_t)off + size;
		SetAttr(path, stat);
	}

	content_list.RefreshPeersRef(path);
}

int CacheBase::Read(std::string path, char* buf, size_t size, off_t off)
{
	off_t file_size = GetAttr(path).size;

	if(off >= file_size)
	{
		log[W_WARNING] << "Fuse trying to read out of file";
		return 0;
	}

	FileContent& file = content_list.GetFile(path);

	/* Limit the read to the size of the file */
	size_t to_read = (size_t) ((off + (off_t)size > file_size) ? file_size - off : size);

	if(!to_read)
	{
		log[W_WARNING] << "Fuse trying to read out of file";
		return 0;
	}

	enum FileContent::chunk_availability chunk_state;
	while((chunk_state = file.HaveChunk(off, to_read)) == FileContent::CHUNK_NOT_READY)
		usleep(10000);			  /* 0.01 sec */

	if(chunk_state == FileContent::CHUNK_UNAVAILABLE)
		throw FileUnavailable();

	FileChunk chunk;
	chunk = file.GetChunk(off, to_read);

	if(!chunk.GetData())			  /* shouldn't happen */
	{
		log[W_WARNING] << "FileContent returned an empty chunk??";
		return 0;
	}

	memcpy(buf, chunk.GetData(), to_read);
	return to_read;
}

int CacheBase::Truncate(std::string path, off_t offset)
{
	log[W_DEBUG] << "Truncating \"" << path << "\" at " << offset;
	pf_stat stat = GetAttr(path);
	stat.size = (size_t)offset;
	stat.ctime = time(NULL);
	stat.mtime = stat.ctime;
	stat.meta_mtime = stat.ctime;
	SetAttr(path, stat);
	FileContent& file = content_list.GetFile(path);
	file.Truncate(offset);

	return 0;
}
