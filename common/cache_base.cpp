#include <string.h>
#include "cache_base.h"
#include "content_list.h"
#include "file_content.h"

void CacheInterface::Write(std::string path, const char* buf, size_t size, off_t off)
{
	/* TODO: modify metadata of the file */
	FileContent& file = content_list.GetFile(path);
	FileChunk chunk(buf, off, size);
	file.SetChunk(chunk);
}

int CacheInterface::Read(std::string path, char* buf, size_t size, off_t off)
{
	FileContent& file = content_list.GetFile(path);

	if(off > file.GetFileSize())
		return 0;

	/* Limit the read to the size of the file */
	size_t to_read = off + size > file.GetFileSize() ? file.GetFileSize() - off : size;

	while(!file.HaveChunk(off, to_read))
		usleep(10000); /* 0.01 sec */
	FileChunk chunk;
	chunk = file.GetChunk(off, to_read);
	memcpy(buf, chunk.GetData(), to_read);
	return to_read;
}

