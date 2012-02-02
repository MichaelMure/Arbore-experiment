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
 *
 */

#ifndef FILE_CHUNK_H
#define FILE_CHUNK_H

#include <time.h>
#include <net/netutil.h>
#include "file_chunk_desc.h"

/** Specialisation of FileChunkDesc to actually hold data of a chunk */
class FileChunk : public FileChunkDesc
{
	time_t access_time;
	char* data;
	bool hdd_synced;

public:
	FileChunk();
	FileChunk(const char* _data, off_t _offset, size_t _size);

	/** Create a Chunk from a serialized chunk
	 *
	 * @param buff is the chunk serialized
	 */
	FileChunk (char* buff);

	/** Copy constructor */
	FileChunk(const FileChunk &other);

	FileChunk& operator=(const FileChunk &other);
	~FileChunk();

	/** @return the time of the last access */
	time_t GetAccessTime() const;

	/** Indicate if the chunk is marked as synced on the disk */
	bool GetHddSynced() const;

	/**
	 * Setter for the synced on disk flag
	 * @param _hdd_synced new value for the synced on disk flag
	 */
	void SetHddSynced(bool _hdd_synced);

	/** @return the data and update the access time */
	const char* GetData();

	/** Overwrite the common data part of this and chunk in this */
	void Merge(FileChunk chunk);

	/** Copy the data of the given chunk and concatenate it in this.
	 * \note if the two chunk are not continous, an error is logged and the operation is done anyway.
	 *
	 * @param chunk a FileChunk
	 */
	void Concatenate(FileChunk chunk);

	/** @return a FilChunk that hold the data of the common part of this and chunk_desc */
	FileChunk GetPart(FileChunkDesc chunk_desc);

	/** Serialyze the chunk in binary format */
	void dump(char* buff) const;
};
#endif						  /* FILE_CHUNK_H */
