#include "job_send_chunk.h"
#include "content_list.h"
#include "file_content.h"
#include "peers_list.h"

bool JobSendChunk::Start()
{
	FileContent& f = content_list.GetFile(ref);
	FileChunk requested_chunk(NULL, offset, size);

	if(f.FileContentHaveChunk(offset, size)
	|| f.OnDiskHaveChunk(requested_chunk)) /* TODO: make OnDiskHaveChunk non-blockant and repeat the job */
	{
		FileChunk chunk = f.GetChunk(offset, size);
		peers_list.SendChunk(f.GetFilename(), sendto, chunk);
		return false;
	}

	return false;
}
