#ifndef FILE_CHUNK_H
#define FILE_CHUNK_H

#include <time.h>

class FileChunk
{
	time_t acces_time;
	char* data;
	size_t offset;
	size_t size;

public:
	FileChunk(char* _data, size_t _offset, size_t _size);
	~FileChunk();

	const time_t GetAccessTime() const { return access_time; }
	const size_t GetOffset() const { return offset; }
	const size_t GetSize() const { return size; }
};

#endif /* FILE_CHUNK_H */
