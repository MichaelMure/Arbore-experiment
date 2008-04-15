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
	if(offset + size < chunk.GetOffset())
		return;

	if(offset > chunk.GetOffset() + chunk.GetSize())
		return;

	/* Merge */
	off_t begin_off = chunk.offset - offset > 0 ? chunk.offset - offset : 0;
	off_t chunk_off = offset - chunk.offset> 0 ? offset - chunk.offset: 0;
	size_t merge_size = offset + size > chunk.GetOffset() + chunk.GetSize() ? (chunk.GetOffset() + chunk.GetSize() - chunk_off) : offset + size - begin_off;

	assert(begin_off + merge_size <= size);
	assert(chunk_off + merge_size <= chunk.GetSize());
	memcpy(data + begin_off, chunk.GetData() + chunk_off, size);
}
