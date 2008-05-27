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

#include <string.h>
#include "cache_base.h"
#include "content_list.h"
#include "file_content.h"
#include "hdd.h"
#include "session_config.h"
#include "environment.h"

CacheBase::CacheBase() : Mutex(RECURSIVE_MUTEX)
{
	tree = NULL;
}

CacheBase::~CacheBase()
{
	if(tree)
		delete tree;
}

FileEntry* CacheBase::Path2File(std::string path, unsigned int flags, std::string* filename)
{
	BlockLockMutex lock(this);
	DirEntry* current_dir = tree;

	std::string name;

	while((name = stringtok(path, "/")).empty() == false)
	{
		FileEntry* child_file = current_dir->GetFile(name);
		if(!child_file || child_file->IsRemoved())
		{
			if(path.empty())
			{
				/* we are in last dir, but this file doesn't exist */
				if(child_file && (flags & (RESTORE_REMOVED_FILE|GET_REMOVED_FILE)))
				{
					if(flags & RESTORE_REMOVED_FILE)
						child_file->ClearRemoved();
					return child_file;
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
				if(child_file && !dynamic_cast<DirEntry*>(child_file))
				{
					current_dir->RemFile(child_file);
					child_file = NULL;
				}

				if(!child_file)
				{
					pf_stat stat;
					stat.uid = 0;
					stat.gid = 0;
					child_file = new DirEntry(name, stat, current_dir);
					current_dir->AddFile(child_file);
				}
				if(flags & RESTORE_REMOVED_FILE)
					child_file->ClearRemoved();
			}
			else if(!(flags & GET_REMOVED_FILE) || !child_file)
				return NULL;
		}

		if(!(current_dir = dynamic_cast<DirEntry*>(child_file)))
		{
			/* This isn't a directory. */
			if(path.empty())
			{
				/* We are on last dir, so it is the requested file. */
				return child_file;
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
	tree = new DirEntry("", pf_stat(), NULL);
	pf_stat s = tree->GetAttr();
	s.mode = S_IFDIR | S_IRWXU;
	tree->SetAttr(s);
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
	IDList idlist;
	idlist.insert(environment.my_id.Get());
	if(off + (off_t)size > (off_t)stat.size)
	{
		stat.size = (size_t)off + size;
		SetAttr(path, stat, idlist);
	}

	//content_list.RefreshPeersRef(path);
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

	FileChunkDesc chunk_to_read(off, to_read);
	FileContent::chunk_availability chunk_state;
	while((chunk_state = file.HaveChunk(chunk_to_read)) == FileContent::CHUNK_NOT_READY)
		usleep(100000);			  /* 0.1 sec */

	if(chunk_state == FileContent::CHUNK_UNAVAILABLE)
		throw FileUnavailable();

	FileChunk chunk = file.GetChunk(chunk_to_read);

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

	IDList idlist;
	idlist.insert(environment.my_id.Get());

	SetAttr(path, stat, idlist);

	FileContent& file = content_list.GetFile(path);
	file.Truncate(offset);

	return 0;
}

bool CacheBase::FileExists(std::string path)
{
	BlockLockMutex lock(this);
	return Path2File(path) != NULL;
}
