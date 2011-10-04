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

#ifndef FILE_CHUNK_DESC_H
#define FILE_CHUNK_DESC_H

#include <time.h>

#include <util/tools.h>

/** Lightweight description of a chunk */
class FileChunkDesc
{
protected:
	off_t offset;
	size_t size;

public:
	FileChunkDesc();
	FileChunkDesc(off_t _offset, size_t _size);
	virtual ~FileChunkDesc() {};

	/** Compare two chunk
	 * @return true if other represent the same chunk
	 */
	bool operator==(const FileChunkDesc &other);

	/** Compare the position of two chunk.
	 * @return true if the left chunk's offset is lower, or if equal, if the size is lower.
	 */
	bool operator<(const FileChunkDesc &other);

	/** @return the offset */
	off_t GetOffset() const;

	/** @return the size */
	size_t GetSize() const;

	/** @return the offset of the next chunk */
	off_t GetEndOffset() const;

	/** @return true if both chunk overlaps*/
	bool Overlaps(const FileChunkDesc& other) const;

	/** @return a FileChunkDesc describing the common part of two chunk */
	FileChunkDesc GetCommonPartDesc(const FileChunkDesc& other);

	/** @return true if the given chunk is completly enclosed in our chunk */
	bool Contains(const FileChunkDesc& other) const;
};

struct CompFileChunk
{
	/* currently unimplemented */
	bool operator() (const FileChunkDesc c1, const FileChunkDesc c2);
};
#endif						  /* FILE_CHUNK_DESC_H */
