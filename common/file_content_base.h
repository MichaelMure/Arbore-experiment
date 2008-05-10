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

#ifndef FILE_CONTENT_BASE_H
#define FILE_CONTENT_BASE_H

#include <string>
#include <list>
#include <map>
#include <time.h>
#include "mutex.h"
#include "file_chunk.h"
#include "file_chunk_desc.h"
#include "pf_types.h"

class FileContentBase : public Mutex, private std::list<FileChunk>
{
public:
	typedef enum _chunk_availability
	{
		CHUNK_READY,		  /**< it's ready to be read */
		CHUNK_NOT_READY,	  /**< it's not yet loaded from hdd/not yet received */
		CHUNK_UNAVAILABLE	  /**< we don't have it on hdd and nobody has it on network */
	} chunk_availability;
private:
	/* Information about what's currently stored on hardrive */
	off_t ondisk_offset;
	off_t ondisk_size;
	int ondisk_fd;
	bool ondisk_synced;
	time_t access_time;

	/* Network related */
	std::list<FileChunkDesc> net_requested;
	struct sharedchunks
	{
		off_t offset;
		off_t size;
	};
protected:
	std::string filename;
	std::map<pf_id, struct sharedchunks> sharers;
	pf_id last_peer_requested;

	void NetworkFlushRequests();
	bool waiting_for_sharers;
	std::list<FileChunkDesc> net_pending_request;
	virtual chunk_availability NetworkRequestChunk(FileChunkDesc chunk) = 0;

private:
	/* Hdd related */
	bool LoadFd();
	bool OnDiskLoad(FileChunkDesc chunk_desc);
	void OnDiskWrite(FileChunk& chunk);

	FileContentBase& operator=(const FileContentBase &other);
public:
	FileContentBase(std::string _filename);
	FileContentBase(const FileContentBase&);
	virtual ~FileContentBase();

	std::string GetFilename() const { return filename; }
	/** Load a chunk from the hdd or ask it on the network
	 * @param chunk the part to load
	 * @param blockant_load if true and the chunk is available on hardrive, the call will lock until it's loaded
	 * @return returns true if the chunk is available after returning from this method
	 */
	bool FileContentHaveChunk(FileChunkDesc chunk_desc);
	bool OnDiskHaveChunk(FileChunkDesc chunk_desc, bool blockant_load = false);
	chunk_availability NetworkHaveChunk(FileChunkDesc chunk_desc);

	/* Returns a copy of the chunk */
	FileChunk GetChunk(FileChunkDesc chunk_desc);

	/* Return true or false if we have it */
	/* Triggers loading the file from the cache or download from the network */
	chunk_availability HaveChunk(FileChunkDesc chunk_desc);

	bool HaveAnyChunk();
	void GetSharedContent(off_t& offset, off_t& size);

	virtual void SetChunk(FileChunk chunk);

	void Truncate(off_t offset);

	void SyncToHdd(bool force = false);
	time_t GetAccessTime() const;

	/* Set which part of the file a peer has */
	void SetSharer(pf_id, off_t offset, off_t size);

	/* Remove the part a peer has */
	void RemoveSharer(pf_id);

	IDList GetSharers();
};
#endif						  /* FILE_CONTENT_BASE_H */
