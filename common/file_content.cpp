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

#include "file_content.h"
#include "file_chunk.h"
#include "scheduler_queue.h"
#include "job_change_filesize.h"

FileChunk FileContent::GetChunk(off_t offset, size_t size)
{
	BlockLockMutex lock(this);
	iterator it = begin();
	while(it != end() && it->GetOffset() + (off_t)it->GetSize() <= offset)
		++it;

	if(it == end())
	{
		FileChunk chunk(NULL, 0, 0);
		return chunk;
	}

	/* Build the requested chunk */
	FileChunk chunk;
	while(it != end() && it->GetOffset() + (off_t)it->GetSize() <= offset + (off_t)size)
	{
		chunk.Concatenate(it->GetPart(offset, size));
		++it;
	}

	return chunk;
}

void FileContent::SetChunk(FileChunk chunk)
{
	BlockLockMutex lock(this);
	/* Merge into the chunk set */
	iterator it = begin();
	while(it != end() && it->GetOffset() + (off_t)it->GetSize() <= chunk.GetOffset())
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
	while(it != end() && it->GetOffset() < chunk.GetOffset() + (off_t)chunk.GetSize())
	{
		it->Merge(chunk);
		++it;
	}

	if(it == end())
		return;

	/* Add any remaining data at the end */
	off_t offset = back().GetOffset() + back().GetSize();
	size_t size = (size_t) (chunk.GetOffset() + chunk.GetSize() - offset);
	char* buf = new char[size];
	memcpy(buf, chunk.GetData(), size);
	FileChunk new_chunk(buf, offset, size);
	push_back(new_chunk);

	/* Update the file size */
	file_size = (size_t) back().GetOffset() + back().GetSize();

	scheduler_queue.Queue(new JobChangeFileSize(filename, file_size));
}

bool FileContent::HaveChunk(off_t offset, size_t size)
{
	// TODO: Check reqquested chunks are contiguous
	BlockLockMutex lock(this);
	iterator it = begin();
	while(it != end() && it->GetOffset() + (off_t)it->GetSize() <= offset)
		++it;

	if(it == end())
		return false;

	/* We have the begining of the chunk
	 * Check we have it until the end */

	while(it != end() && it->GetOffset() + (off_t)it->GetSize() < offset + (off_t)size)
		++it;

	return (it != end());
}

void FileContent::Truncate(off_t offset)
{
	BlockLockMutex lock(this);

	if(offset > (off_t)file_size)
		return;

	iterator it = begin();
	while(it != end() && it->GetOffset() + (off_t)it->GetSize() <= offset)
		++it;

	if(it != end() && it->GetOffset() < offset)
	{
		/* Split the first Chunk */
		FileChunk chunk(it->GetData(), it->GetOffset(), (size_t)(offset - it->GetOffset()));
		insert(it, chunk);
	}

	while(it != end())
		it = erase(it);

	file_size = (size_t)offset;
	scheduler_queue.Queue(new JobChangeFileSize(filename, file_size));
}

size_t FileContent::GetFileSize() const
{
	BlockLockMutex lock(this);
	return file_size;
}
