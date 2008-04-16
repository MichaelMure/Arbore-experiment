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
#include "log.h"

FileChunk::FileChunk(const char* _data, off_t _offset, size_t _size) : offset(_offset), size(_size), hdd_synced(false)
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
			size(other.size),
			hdd_synced(other.hdd_synced)
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
	access_time = other.access_time;
	hdd_synced = other.hdd_synced;

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

const char* FileChunk::GetData()
{
	access_time = time(NULL);
	return data;
}

void FileChunk::Merge(FileChunk chunk)
{
	/* Check we are overlapping */
	if(offset + (off_t)size <= chunk.GetOffset())
		return;

	if(offset >= chunk.GetOffset() + (off_t)chunk.GetSize())
		return;

	access_time = time(NULL);
	hdd_synced = false;

	/* Merge */
	off_t begin_off = chunk.offset - offset > 0 ? chunk.offset - offset : 0;
	off_t chunk_off = offset - chunk.offset > 0 ? offset - chunk.offset: 0;
	off_t merge_end = MIN(offset + (off_t)size, chunk.GetOffset() + (off_t)chunk.GetSize());
	size_t merge_size = merge_end - begin_off - offset;

	assert(begin_off + merge_size <= size);
	assert(chunk_off + merge_size <= chunk.GetSize());
	memcpy(data + begin_off, chunk.GetData() + chunk_off, size);

	log[W_DEBUG] << "Chunk merged: off:" << chunk.GetOffset() << ", size:" << chunk.GetSize() << " inside off:" << offset << ", size:" << size;
}

void FileChunk::Concatenate(FileChunk other)
{
	if(!other.data)
		return;

	if(!data)
	{
		*this = other;
		return;
	}

	if(offset + (off_t)size != other.GetOffset())
		log[W_ERR] << "Ooops! Concatenating 2 non-contiguous chunks";

	access_time = time(NULL);
	hdd_synced = false;
	size_t new_size = (size_t) (other.GetOffset() + other.GetSize() - offset);
	char* new_data = new char[new_size];
	memset(new_data, 0, new_size);

	if(data)
	{
		memcpy(new_data, data, size);
		delete []data;
	}

	if(other.GetData())
		memcpy(new_data + other.GetOffset() - offset, other.GetData(), other.GetSize());

	size = new_size;
	data = new_data;
	log[W_DEBUG] << "Chunk concatenated: off:" << other.GetOffset() << ", size:" << other.GetSize() << " to off:" << offset << ", size:" << size;
}

FileChunk FileChunk::GetPart(off_t _offset, size_t _size)
{
	if(_offset >= offset + (off_t)size)
		return FileChunk();
	if(_offset + (off_t)_size <= offset)
		return FileChunk();

	access_time = time(NULL);
	off_t data_offset = _offset > offset ? _offset - offset : 0;
	off_t data_end = MIN(_offset + (off_t)_size, offset + (off_t)size);
	size_t data_size = (size_t) (data_end - offset - data_offset);

	return FileChunk(data + data_offset, offset + data_offset, data_size);
}
