#include "file_content.h"
#include "file_chunk.h"
#include "scheduler_queue.h"
#include "job_change_filesize.h"

FileChunk FileContent::GetChunk(off_t offset, size_t size)
{
	BlockLockMutex lock(this);
	iterator it;
	for(it = begin(); it != end() && it->GetOffset() != offset; ++it)
		;

	if(it == end())
	{
		FileChunk chunk(NULL, 0, 0);
		return chunk;
	}
	else
		return *it;
}

void FileContent::SetChunk(FileChunk chunk)
{
	BlockLockMutex lock(this);
	/* Merge into the chunk set */
	iterator it = begin();
	while(it != end() && it->GetOffset() + it->GetSize() < chunk.GetOffset())
		++it;

	if(it == end())
	{
		/* It doesn't overlap with previous data
		 * add it to the end */
		push_back(chunk);
		file_size += chunk.GetSize();
		scheduler_queue.Queue(new JobChangeFileSize(filename, file_size));
		return;
	}

	/* Replace the current chunks */
	while(it != end() && it->GetOffset() < chunk.GetOffset() + chunk.GetSize())
		it->Merge(chunk);

	if(it == end())
		return;

	/* Add any remaining data at the end */
	off_t offset = back().GetOffset() + back().GetSize();
	size_t size = chunk.GetOffset() + chunk.GetSize() - offset;
	char* buf = new char[size];
	memcpy(buf, chunk.GetData(), size);
	FileChunk new_chunk(buf, offset, size);
	push_back(new_chunk);

	/* Update the file size */
	file_size = back().GetOffset() + back().GetSize();

	scheduler_queue.Queue(new JobChangeFileSize(filename, file_size));
}

bool FileContent::HaveChunk(off_t offset, size_t size)
{
	BlockLockMutex lock(this);
	iterator it = begin();
	while(it != end() && it->GetOffset() + it->GetSize() < offset)
		++it;

	if(it == end())
		return false;

	/* We have the begining of the chunk
	 * Check we have it until the end */

	while(it != end() && it->GetOffset() + it->GetSize() < offset + size)
		++it;

	return (it != end());
}

const size_t FileContent::GetFileSize() const
{
	BlockLockMutex loch(this);
	return file_size;
}

