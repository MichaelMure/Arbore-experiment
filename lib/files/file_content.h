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
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#ifndef FILE_CONTENT_H
#define FILE_CONTENT_H

#include <string>
#include <list>
#include <map>
#include <time.h>

#include <util/mutex.h>
#include <util/pf_types.h>
#include <util/key.h>
#include "file_chunk.h"
#include "file_chunk_desc.h"
#include "file_chunk_requester_interface.h"

class FileContent : public Mutex, private std::list<FileChunk>
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
	int ondisk_fd; /** Corresponding file descriptor */
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
	FileChunkRequesterInterface* requester;
	std::string filename;
	std::map<Key, struct sharedchunks> sharers;
	Key last_peer_requested;
	time_t ref_request_time;

	void NetworkFlushRequests();
	std::list<FileChunkDesc> net_pending_request;
	chunk_availability NetworkRequestChunk(FileChunkDesc chunk);

private:
	/* Hdd related */

  /** Initialise the internal file descriptor and move the r/w pointer in the end.
   * @return true if successful, false otherwise
   */
	bool LoadFd();

  /** Load a new chunk from the hdd and store it.
   * @return true if successful, false otherwise
   */
	bool OnDiskLoad(FileChunkDesc chunk_desc);

  /** Store the given chunk in the file on the hdd.
   * @return true if successful, false otherwise
   */
	void OnDiskWrite(FileChunk& chunk);

	FileContent& operator=(const FileContent &other);
public:
	/** Constructor
	 * @param _filename  the file name.
	 * @param requester  implementation object of the FileChunkRequesterInterface interface.
	 */
	FileContent(std::string _filename, FileChunkRequesterInterface* requester);

	/** Copy constructor */
	FileContent(const FileContent&);

	/** Destructor */
	virtual ~FileContent();

	/** @return the name of the file. */
	std::string GetFilename() const { return filename; }

	/** @return true if the whole chunk_desc is available. */
	bool FileContentHaveChunk(FileChunkDesc chunk_desc);

	/** Load a chunk from the hdd or ask it on the network
	 * @param chunk_desc the part to load
	 * @param blockant_load if true and the chunk is available on hardrive, the call will lock until it's loaded
	 * @return returns true if the chunk is available after returning from this method
	 */
	bool OnDiskHaveChunk(FileChunkDesc chunk_desc, bool blockant_load = false);
	chunk_availability NetworkHaveChunk(FileChunkDesc chunk_desc);

	/** @return a copy of the chunk. */
	FileChunk GetChunk(FileChunkDesc chunk_desc);

	/** @return true or false if we have the chunk */
	/* Triggers loading the file from the cache or download from the network */
	chunk_availability HaveChunk(FileChunkDesc chunk_desc);

	/** @return true if at least one chunk is available, either on disk or on memory */
	bool HaveAnyChunk();
	void GetSharedContent(off_t& offset, off_t& size);

	virtual void SetChunk(FileChunk chunk);

	void Truncate(off_t offset);

	void SyncToHdd(bool force = false);
	time_t GetAccessTime() const;

	/* Set which part of the file a peer has */
	void SetSharer(Key, off_t offset, off_t size);

	/* Remove the part a peer has */
	void RemoveSharer(Key);

	KeyList GetSharers();
};
#endif						  /* FILE_CONTENT_H */
