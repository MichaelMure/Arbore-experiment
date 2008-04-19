#include "job_set_chunk.h"
#include "content_list.h"
#include "file_content.h"

bool JobSetChunk::Start()
{
	FileContent& f = content_list.GetFile(filename);
	f.SetChunk(chunk);
	return false;
}
