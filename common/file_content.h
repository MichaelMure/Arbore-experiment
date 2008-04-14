#ifndef FILE_CONTENT_H
#define FILE_CONTENT_H

#include <list>
#include "mutex.h"
#include "file_chunk.h"

class FileContent : public Mutex, private std::list<FileChunk>
{
public:
	FileChunk GetChunk(size_t offset, size_t size);
	void SetChunk(FileChunk chunk, size_t offset);
};

#endif
