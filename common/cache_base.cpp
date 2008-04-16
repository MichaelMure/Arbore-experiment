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
	FileContent& file = content_list.GetFile(path);
	FileChunk chunk(buf, off, size);
	file.SetChunk(chunk);

	/* No need to lock cache, we don't touch its members */
	pf_stat stat = GetAttr(path);
	if(off + (off_t)size > (off_t)stat.size)
	{
		stat.size = (size_t)off + size;
		SetAttr(path, stat);
	}
}

int CacheInterface::Read(std::string path, char* buf, size_t size, off_t off)
{
	size_t file_size = (size_t)GetAttr(path).size;

	if(off > (off_t)file_size)
		return 0;

	FileContent& file = content_list.GetFile(path);

	/* Limit the read to the size of the file */
	size_t to_read = (size_t) ((off + size > file_size) ? file_size - off : size);

	if(!to_read)
		return 0;

	while(!file.HaveChunk(off, to_read))
		usleep(10000);			  /* 0.01 sec */

	FileChunk chunk;
	chunk = file.GetChunk(off, to_read);

	if(!chunk.GetData())			  /* shouldn't happen */
		return 0;

	memcpy(buf, chunk.GetData(), to_read);
	return to_read;
}

int CacheInterface::Truncate(std::string path, off_t offset)
{
	log[W_DEBUG] << "Truncating \"" << path << "\" at " << offset;
	pf_stat stat = GetAttr(path);
	stat.size = (size_t)offset;
	SetAttr(path, stat);
	FileContent& file = content_list.GetFile(path);
	file.Truncate(offset);

	return 0;
}

