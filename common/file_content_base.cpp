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
#include "file_content_base.h"
#include "file_chunk.h"
#include "scheduler_queue.h"
#include "job_change_filesize.h"
#include "session_config.h"
#include "hdd.h"
#include "peers_list.h"

const time_t write_to_hdd_timeout = 10;

FileContentBase::FileContentBase(std::string _filename) :
			Mutex(RECURSIVE_MUTEX),
			ondisk_offset(0),
			ondisk_size(0),
			ondisk_fd(-1),
			ondisk_synced(true),
			filename(_filename)
{
	uint32_t nbr = 0;
	tree_cfg.Get(filename + "#ondisk_off", nbr);
	ondisk_offset = nbr;
	access_time = time(NULL);
}

FileContentBase::FileContentBase(const FileContentBase& other) :
			Mutex(RECURSIVE_MUTEX),
std::list<FileChunk>(),				  /* to avoid a warning */
			ondisk_offset(other.ondisk_offset),
			ondisk_size(other.ondisk_size),
			ondisk_fd(-1),
			filename(other.filename)
{
	if(other.ondisk_fd != -1)
		ondisk_fd = dup(other.ondisk_fd);
	for(const_iterator it = other.begin(); it != other.end(); ++it)
		push_back(*it);
	access_time = other.access_time;
}

FileContentBase::~FileContentBase()
{
	if(ondisk_fd != -1)
		close(ondisk_fd);
}

bool FileContentBase::LoadFd()
{
	BlockLockMutex lock(this);
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

void FileContentBase::OnDiskWrite(FileChunk& chunk)
{
	BlockLockMutex lock(this);
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

bool FileContentBase::OnDiskLoad(FileChunk chunk)
{
	BlockLockMutex lock(this);
	if(!LoadFd())
		return false;

	if(lseek(ondisk_fd, chunk.GetOffset() - ondisk_offset, SEEK_SET) == (off_t)-1)
		return false;

	log[W_DEBUG] << "Loading from cache \"" << filename << "\" off:" << chunk.GetOffset() << ", size:" << chunk.GetSize();
	char* buf = new char[chunk.GetSize()];
	if(read(ondisk_fd, buf, chunk.GetSize()) == -1)
		log[W_ERR] << "Error while loading \"" << filename << "\": "<< strerror(errno);
	FileChunk new_chunk(buf, chunk.GetOffset(), chunk.GetSize());
	new_chunk.SetHddSynced(true);
	delete []buf;
	SetChunk(new_chunk);

	return true;
}

bool FileContentBase::OnDiskHaveChunk(FileChunk chunk, bool blockant_load)
{
	BlockLockMutex lock(this);
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

FileChunk FileContentBase::GetChunk(off_t offset, size_t size)
{
	BlockLockMutex lock(this);
	access_time = time(NULL);
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

bool FileContentBase::FileContentHaveChunk(off_t offset, size_t size)
{
	iterator it = begin();
	while(it != end() && it->GetOffset() + (off_t)it->GetSize() <= offset)
		++it;

	if(it == end() || it->GetOffset() > offset)
		return false;

	/* We have the begining of the chunk
	 * Check we have it until the end */
	off_t next_off = it->GetOffset();
	while(it != end()
		&& it->GetOffset() + (off_t)it->GetSize() < offset + (off_t)size
		&& next_off == it->GetOffset())
	{
		/* Blocks must follow themself */
		next_off = it->GetOffset() + (off_t)it->GetSize();
		++it;
	}

	return (it != end()) && next_off == it->GetOffset();
}

enum FileContentBase::chunk_availability FileContentBase::NetworkHaveChunk(FileChunk chunk)
{
	/* If nobody's connected we won't receive anything */
	if(peers_list.Size() == 0)
		return CHUNK_UNAVAILABLE;

	std::set<FileChunk>::iterator net_it;
	if((net_it = net_unavailable.find(chunk)) != net_unavailable.end())
	{
		net_unavailable.erase(*net_it);
		return CHUNK_UNAVAILABLE;
	}

	if(net_requested.find(chunk) != net_requested.end())
		return CHUNK_NOT_READY;

	net_requested.insert(chunk);
	NetworkRequestChunk(chunk);
	return CHUNK_NOT_READY;
}

enum FileContentBase::chunk_availability FileContentBase::HaveChunk(off_t offset, size_t size)
{
	BlockLockMutex lock(this);
	access_time = time(NULL);

	/* Check if the chunk is already loaded */
	if(FileContentHaveChunk(offset, size))
		return CHUNK_READY;

	/* Check if we have it on Hdd */
	if(OnDiskHaveChunk(FileChunk(NULL, offset, size), true))
		return CHUNK_READY;

	/* Ask it on the network */
	return NetworkHaveChunk(FileChunk(NULL, offset, size));
}

bool FileContentBase::HaveAnyChunk()
{
	BlockLockMutex lock(this);
	LoadFd();				  /* Needed to update the ondisk_size value */
	return size() != 0 || ondisk_size != 0;
}

void FileContentBase::SetChunk(FileChunk chunk)
{
	BlockLockMutex lock(this);
	ondisk_synced = false;
	access_time = time(NULL);

	std::set<FileChunk>::iterator net_it;
	if((net_it = net_requested.find(chunk)) != net_requested.end())
		net_requested.erase(*net_it);

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

void FileContentBase::Truncate(off_t offset)
{
	BlockLockMutex lock(this);
	access_time = time(NULL);

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
			ondisk_size = (size_t) (offset - ondisk_offset);
		tree_cfg.Set(filename + "#ondisk_off", (uint32_t)ondisk_offset);
	}
}

void FileContentBase::SyncToHdd(bool force)
{
	BlockLockMutex lock(this);
	if(ondisk_synced)
		return;

	ondisk_synced = true;
	log[W_DEBUG] << "Trying to sync " << filename;
	/* Write on disk, without doing "blanks" in the file */
	iterator it = begin();
	while(it != end() && it->GetOffset() <= ondisk_offset + (off_t)ondisk_size)
	{
		if(it->GetAccessTime() + write_to_hdd_timeout < time(NULL) || force)
		{
			if(!it->GetHddSynced())
				OnDiskWrite(*it);
			it = erase(it);
		}
		else
		{
			ondisk_synced = false;
			++it;
		}
	}
}

void FileContentBase::GetOnDiskContent(off_t& offset, size_t& size)
{
	BlockLockMutex lock(this);
	access_time = time(NULL);

	LoadFd();
	offset = ondisk_offset;
	size = ondisk_size;
}

time_t FileContentBase::GetAccessTime() const
{
	BlockLockMutex lock(this);
	return access_time;
}

void FileContentBase::NetworkFlushRequests()
{
	BlockLockMutex lock(this);
	access_time = time(NULL);
}

void FileContentBase::SetSharer(pf_id sharer, off_t offset, size_t size)
{
	struct sharedchunks shared_part;
	shared_part.sharer = sharer;
	shared_part.part = FileChunk(NULL, offset, size);
	sharers.push_back(shared_part);
}
