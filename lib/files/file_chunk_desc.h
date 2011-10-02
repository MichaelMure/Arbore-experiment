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
#include <unistd.h>
#include <assert.h>

#include <util/tools.h>

class FileChunkDesc
{
protected:
	off_t offset;
	size_t size;

public:
	FileChunkDesc();
	FileChunkDesc(off_t _offset, size_t _size);
	virtual ~FileChunkDesc() {};

	bool operator==(const FileChunkDesc &other);
	bool operator<(const FileChunkDesc &other);

	off_t GetOffset() const;
	size_t GetSize() const;

	off_t GetEndOffset() const;

	bool Overlaps(const FileChunkDesc& other) const;

	FileChunkDesc GetCommonPartDesc(const FileChunkDesc& other);

	bool Contains(const FileChunkDesc& other) const;
};

struct CompFileChunk
{
	/* currently unimplemented */
	bool operator() (const FileChunkDesc c1, const FileChunkDesc c2);
};
#endif						  /* FILE_CHUNK_DESC_H */
