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

#include <assert.h>
#include <string.h>
#include "file_chunk.h"

FileChunk::FileChunk(const char* _data, off_t _offset, size_t _size) : offset(_offset), size(_size)
{
	access_time = time(NULL);
	if(_data)
	{
		data = new char[size];
		memcpy(data, _data, size);
	}
	else
		data = NULL;
}

FileChunk::FileChunk(const FileChunk& other) : access_time(other.access_time),
			offset(other.offset),
			size(other.size)
{
	if(other.data)
	{
		data = new char[size];
		memcpy(data, other.data, size);
	}
	else
		data = NULL;
}

FileChunk& FileChunk::operator=(const FileChunk& other)
{
	size = other.size;
	offset = other.offset;

	if(data)
		delete []data;

	if(other.data)
	{
		data = new char[size];
		memcpy(data, other.data, size);
	}
	else
		data = NULL;
	return *this;
}

FileChunk::~FileChunk()
{
	if(data)
		delete []data;
}

void FileChunk::Merge(FileChunk chunk)
{
	/* Check we are overlapping */
	if(offset + (off_t)size < chunk.GetOffset())
		return;

	if(offset > chunk.GetOffset() + (off_t)chunk.GetSize())
		return;

	/* Merge */
	off_t begin_off = chunk.offset - offset > 0 ? chunk.offset - offset : 0;
	off_t chunk_off = offset - chunk.offset> 0 ? offset - chunk.offset: 0;
	size_t merge_size = offset + size > chunk.GetOffset() + chunk.GetSize() ? (chunk.GetOffset() + chunk.GetSize() - chunk_off) : offset + size - begin_off;

	assert(begin_off + merge_size <= size);
	assert(chunk_off + merge_size <= chunk.GetSize());
	memcpy(data + begin_off, chunk.GetData() + chunk_off, size);
}
