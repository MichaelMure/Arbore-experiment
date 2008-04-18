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

#ifndef FILE_CONTENT_BASE_H
#define FILE_CONTENT_BASE_H

#include <string>
#include <list>
#include <set>
#include <time.h>
#include "mutex.h"
#include "file_chunk.h"

class FileContentBase : public Mutex, private std::list<FileChunk>
{
public:
	enum chunk_availability
	{
		CHUNK_READY,		  /** it's ready to be read */
		CHUNK_NOT_READY,	  /** it's not yet loaded from hdd/not yet received */
		CHUNK_UNAVAILABLE	  /** we don't have it on hdd and nobody has it on network */
	};
private:
	/* Information about what's currently stored on hardrive */
	off_t ondisk_offset;
	size_t ondisk_size;
	int ondisk_fd;
	bool ondisk_synced;
	time_t access_time;

	/* Network related */
	std::set<FileChunk, CompFileChunk> net_requested;
	std::set<FileChunk, CompFileChunk> net_unavailable;
	virtual void NetworkRequestChunk(FileChunk chunk) = 0;

	/* Hdd related */
	bool LoadFd();
	bool OnDiskLoad(FileChunk chunk);
	void OnDiskWrite(FileChunk& chunk);

	/** Load a chunk from the hdd or ask it on the network
	 * @param chunk the part to load
	 * @param blockant_load if true and the chunk is available on hardrive, the call will lock until it's loaded
	 * @return returns true if the chunk is available after returning from this method
	 */
	bool FileContentHaveChunk(off_t offset, size_t size);
	bool OnDiskHaveChunk(FileChunk chunk, bool blockant_load = false);
	enum chunk_availability NetworkHaveChunk(FileChunk chunk);

	FileChunk& operator=(const FileChunk &other);

protected:
	std::string filename;

public:
	FileContentBase(std::string _filename);
	FileContentBase(const FileContentBase&);
	virtual ~FileContentBase();

	/* Returns a copy of the chunk */
	FileChunk GetChunk(off_t offset, size_t size);

	/* Return true or false if we have it */
	/* Triggers loading the file from the cache or download from the network */
	enum chunk_availability HaveChunk(off_t offset, size_t size);

	bool HaveAnyChunk();
	void GetOnDiskContent(off_t& offset, size_t& size);

	void SetChunk(FileChunk chunk);

	void Truncate(off_t offset);

	void SyncToHdd(bool force = false);
	time_t GetAccessTime() const;
};
#endif						  /* FILE_CONTENT_BASE_H */
