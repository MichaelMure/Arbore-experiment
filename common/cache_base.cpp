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

#include <string.h>
#include "cache_base.h"
#include "content_list.h"
#include "file_content.h"

void CacheInterface::Write(std::string path, const char* buf, size_t size, off_t off)
{
	/* No need to lock cache, we don't touch its members */

	FileContent& file = content_list.GetFile(path);
	FileChunk chunk(buf, off, size);
	file.SetChunk(chunk);
}

int CacheInterface::Read(std::string path, char* buf, size_t size, off_t off)
{
	/* No need to lock cache, we don't touch its members */

	FileContent& file = content_list.GetFile(path);

	if(off > (off_t)file.GetFileSize())
		return 0;

	/* Limit the read to the size of the file */
	size_t to_read = (size_t) ((off + size > file.GetFileSize()) ? file.GetFileSize() - off : size);

	while(!file.HaveChunk(off, to_read))
		usleep(10000);			  /* 0.01 sec */
	FileChunk chunk;
	chunk = file.GetChunk(off, to_read);
	memcpy(buf, chunk.GetData(), to_read);
	return to_read;
}

int CacheInterface::Truncate(std::string path, off_t offset)
{
	/* No need to lock cache, we don't touch its members */

	FileContent& file = content_list.GetFile(path);
	file.Truncate(offset);
	return 0;
}

