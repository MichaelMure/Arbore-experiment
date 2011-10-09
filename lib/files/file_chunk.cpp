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

#include <assert.h>
#include <string.h>

#include <util/pf_log.h>

#include "file_chunk.h"

FileChunk::FileChunk()
 : access_time(0), data(NULL), hdd_synced(false)
{
}

FileChunk::FileChunk(const char* _data, off_t _offset, size_t _size)
 : FileChunkDesc(_offset, _size), hdd_synced(false)
{
	access_time = time(NULL);
	if(_data)
	{
		data = new char[GetSize()];
		memcpy(data, _data, GetSize());
	}
	else
		data = NULL;
}

FileChunk::FileChunk(const FileChunk& other)
 : FileChunkDesc(other), access_time(other.access_time), hdd_synced(other.hdd_synced)
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

time_t FileChunk::GetAccessTime() const
{
	return access_time;
}

bool FileChunk::GetHddSynced() const
{
	return hdd_synced;
}

void FileChunk::SetHddSynced(bool _hdd_synced)
{
	hdd_synced = _hdd_synced;
}

const char* FileChunk::GetData()
{
	access_time = time(NULL);
	return data;
}

void FileChunk::Merge(FileChunk chunk)
{
	/* Check we are overlapping */
	if(!Overlaps(chunk))
		return;

	access_time = time(NULL);
	hdd_synced = false;

	/* Merge */
	FileChunkDesc common_part = GetCommonPartDesc(chunk);

	off_t buf_start = common_part.GetOffset() - GetOffset();
	off_t chu_start = common_part.GetOffset() - chunk.GetOffset();

	memcpy(data + buf_start, chunk.GetData() + chu_start, common_part.GetSize());

	pf_log[W_DEBUG] << "Chunk merged: off:" << chunk.GetOffset() << ", size:" << chunk.GetSize() << " inside off:" << offset << ", size:" << size;
}

void FileChunk::Concatenate(FileChunk other)
{
	if(!other.data)
		return;

	if(!data)
	{
		/* overwrite this with other */
		*this = other;
		return;
	}

	if(GetEndOffset() != other.GetOffset())
		pf_log[W_ERR] << "Ooops! Concatenating 2 non-contiguous chunks";

	access_time = time(NULL);
	hdd_synced = false;
	size_t new_size = (size_t) (GetSize() + other.GetSize());
	char* new_data = new char[new_size];

	/* zero the newly allocated data, is this really necessary since we overwrite after ? */
	memset(new_data, 0, new_size);

	if(data)
	{
		memcpy(new_data, data, size);
		delete []data;
	}

	if(other.GetData())
		memcpy(new_data + other.GetOffset() - GetOffset(), other.GetData(), other.GetSize());

	size = new_size;
	data = new_data;
	pf_log[W_DEBUG] << "Chunk concatenated: off:" << other.GetOffset() << ", size:" << other.GetSize() << " to off:" << offset << ", size:" << size;
}

FileChunk FileChunk::GetPart(FileChunkDesc chunk_desc)
{
	if(!Overlaps(chunk_desc))
		return FileChunk();

	access_time = time(NULL);
	FileChunkDesc common_part = GetCommonPartDesc(chunk_desc);

	off_t buf_start = common_part.GetOffset() - GetOffset();

	FileChunk chunk(GetData() + buf_start, common_part.GetOffset(), common_part.GetSize());
	if(hdd_synced)
		chunk.SetHddSynced(true);
	return chunk;
}
