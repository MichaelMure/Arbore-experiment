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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "file_content.h"
#include "file_chunk.h"
#include "scheduler_queue.h"
#include "job_change_filesize.h"
#include "session_config.h"
#include "hdd.h"

const time_t write_to_hdd_timeout = 10;

FileContent::FileContent(std::string _filename) :
			Mutex(RECURSIVE_MUTEX),
			filename(_filename),
			ondisk_offset(0),
			ondisk_size(0),
			ondisk_fd(-1),
			ondisk_synced(true)
{
	uint32_t nbr = 0;
	tree_cfg.Get(filename + "#ondisk_off", nbr);
	ondisk_offset = nbr;
}

FileContent::FileContent(const FileContent& other) :
			Mutex(RECURSIVE_MUTEX),
std::list<FileChunk>(),				  /* to avoid a warning */
			filename(other.filename),
			ondisk_offset(other.ondisk_offset),
			ondisk_size(other.ondisk_size),
			ondisk_fd(-1)
{
	if(other.ondisk_fd != -1)
		ondisk_fd = dup(other.ondisk_fd);
	for(const_iterator it = other.begin(); it != other.end(); ++it)
		push_back(*it);
}

FileContent::~FileContent()
{
	if(ondisk_fd != -1)
		close(ondisk_fd);
}

bool FileContent::LoadFd()
{
	if(ondisk_fd == -1)
	{
		ondisk_fd = hdd.GetFd(filename);
		off_t size = lseek(ondisk_fd, 0, SEEK_END);
		if(size == (off_t)-1)
		{
			log[W_ERR] << "Error loading \"" << filename << "\": " << strerror(errno);
			close(ondisk_fd);
			ondisk_fd = -1;
			return false;
		}
		ondisk_size = (size_t) size;
		log[W_DEBUG] << "\"" << filename << "\" have " << ondisk_size << " oct on hdd";
	}
	return (ondisk_fd != -1);
}

void FileContent::OnDiskWrite(FileChunk& chunk)
{
	ondisk_offset = 0;
	if(!LoadFd())
	{
		log[W_ERR] << "Unable to save \"" << filename <<"\" in cache.";
		return;
	}

	if(lseek(ondisk_fd, chunk.GetOffset() - ondisk_offset, SEEK_SET) == (off_t)-1
		|| (size_t)write(ondisk_fd, chunk.GetData(), chunk.GetSize()) != chunk.GetSize())
	{
		log[W_ERR] << "Unable to save \"" << filename <<"\" in cache: " <<strerror(errno);
		return;
	}
	fsync(ondisk_fd);

	log[W_DEBUG] << "Synced \"" << filename << "\" off:" << chunk.GetOffset() << " size:" << chunk.GetSize();
	ondisk_size = (size_t)MAX(chunk.GetOffset() + (off_t)chunk.GetSize(), (off_t)ondisk_size);
	chunk.SetHddSynced(true);
}

bool FileContent::OnDiskLoad(FileChunk chunk)
{
	if(!LoadFd())
		return false;

	if(lseek(ondisk_fd, chunk.GetOffset() - ondisk_offset, SEEK_SET) == (off_t)-1)
		return false;

	log[W_DEBUG] << "Loading from cache \"" << filename << "\" off:" << chunk.GetOffset() << ", size:" << chunk.GetSize();
	char* buf = new char[chunk.GetSize()];
	if(read(ondisk_fd, buf, chunk.GetSize()) == -1)
		log[W_ERR] << "Error while loading \"" << filename << "\": "<< strerror(errno);
	FileChunk new_chunk(buf, chunk.GetOffset(), chunk.GetSize());
	delete []buf;
	SetChunk(new_chunk);

	return true;
}

bool FileContent::LoadChunk(FileChunk chunk, bool blockant_load)
{
	LoadFd();				  /* Needed to update the ondisk_size value */

	/* TODO: load only the missing part of the chunk */
	log[W_DEBUG] << "Loading chunk of \"" << filename << "\" off:" << chunk.GetOffset() << " size:" << chunk.GetSize();
	if(chunk.GetOffset() >= ondisk_offset
		&& (chunk.GetOffset() + (off_t)chunk.GetSize() <= ondisk_offset + (off_t)ondisk_size))
	{
		if(!OnDiskLoad(chunk))
		{
			log[W_ERR] << "Unable to read file from cache !";
			return false;
		}
		return true;
	}
	return false;
}

FileChunk FileContent::GetChunk(off_t offset, size_t size)
{
	BlockLockMutex lock(this);
	iterator it = begin();
	while(it != end() && it->GetOffset() + (off_t)it->GetSize() <= offset)
		++it;

	if(it == end())
	{
		log[W_ERR] << "Oops! Trying to read out of file content";
		FileChunk chunk(NULL, 0, 0);
		return chunk;
	}

	/* Build the requested chunk */
	FileChunk chunk;
	while(it != end() && it->GetOffset() < offset + (off_t)size)
	{
		chunk.Concatenate(it->GetPart(offset, size));
		++it;
	}

	return chunk;
}

void FileContent::SetChunk(FileChunk chunk)
{
	BlockLockMutex lock(this);
	ondisk_synced = false;

	/* Merge into the chunk set */
	iterator it = begin();
	while(it != end() && it->GetOffset() + (off_t)it->GetSize() <= chunk.GetOffset())
		++it;

	if(it == end())
	{
		/* It doesn't overlap with previous data
		 * add it to the end */
		log[W_DEBUG] << "Adding chunk at the end of \"" << filename << "\" off:" << chunk.GetOffset() << " size:" << chunk.GetSize();
		push_back(chunk);
		return;
	}

	/* Replace the current chunks */
	while(it != end() && it->GetOffset() < chunk.GetOffset() + (off_t)chunk.GetSize())
	{
		it->Merge(chunk);
		++it;
	}

	if(it == end())
		return;

	/* Add any remaining data at the end */
	off_t offset = chunk.GetOffset();
	if(it != begin())
	{
		--it;
		offset = it->GetOffset() + it->GetSize();
		++it;
	}
	size_t size = (size_t) (chunk.GetOffset() + chunk.GetSize() - offset);
	log[W_DEBUG] << "Inserting chunk in the middle of \"" << filename << "\" off:" << offset << " size:" << size;
	FileChunk new_chunk = chunk.GetPart(offset, size);
	insert(it, new_chunk);
}

bool FileContent::HaveChunk(off_t offset, size_t size)
{
	BlockLockMutex lock(this);
	iterator it = begin();
	while(it != end() && it->GetOffset() + (off_t)it->GetSize() <= offset)
		++it;

	if(it == end())
		return LoadChunk(FileChunk(NULL, offset, size), true);

	/* We have the begining of the chunk
	 * Check we have it until the end */
	off_t next_off = 0;
	while(it != end()
		&& it->GetOffset() + (off_t)it->GetSize() < offset + (off_t)size
		&& (!next_off || next_off == it->GetOffset()))
	{
		/* Blocks must follow themself */
		next_off = it->GetOffset() + (off_t)it->GetSize();
		++it;
	}

	bool res = (it != end()) && (!next_off || next_off == it->GetOffset());

	/* If we don't have the chunk, try to load it */
	if(!res)
		res = LoadChunk(FileChunk(NULL, offset, size), true);

	return res;
}

void FileContent::Truncate(off_t offset)
{
	BlockLockMutex lock(this);

	iterator it = begin();
	while(it != end() && it->GetOffset() + (off_t)it->GetSize() <= offset)
		++it;

	if(it != end() && it->GetOffset() < offset)
	{
		/* Split the first Chunk */
		FileChunk chunk(it->GetData(), it->GetOffset(), (size_t)(offset - it->GetOffset()));
		insert(it, chunk);
	}

	while(it != end())
		it = erase(it);

	LoadFd();				  /* Needed to update the ondisk_size value */
	if(offset < ondisk_offset + (off_t)ondisk_size)
	{
		if(ondisk_fd == -1)
			return;			  /* TODO:return an error */

		ftruncate(ondisk_fd, offset);	  /* TODO:return an error */

		if(offset <= ondisk_offset)
		{
			ondisk_offset = 0;
			ondisk_size = 0;
		}
		else
			ondisk_size = offset - ondisk_offset;
		tree_cfg.Set(filename + "#ondisk_off", (uint32_t)ondisk_offset);
	}
}

void FileContent::SyncToHdd()
{
	BlockLockMutex lock(this);
	if(ondisk_synced)
		return;

	iterator it = begin();
	/* Blocks must follow themself */
	off_t next_off = 0;
	while(it != end() && (!next_off || next_off == it->GetOffset()))
	{
		if(!it->GetHddSynced() && it->GetAccessTime() + write_to_hdd_timeout > time(NULL))
			return;

		if(!it->GetHddSynced())
			OnDiskWrite(*it);

		next_off = it->GetOffset() + (off_t)it->GetSize();
		++it;
	}

	ondisk_synced = true;
}
