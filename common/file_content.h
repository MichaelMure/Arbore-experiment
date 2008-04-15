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

#ifndef FILE_CONTENT_H
#define FILE_CONTENT_H

#include <string>
#include <list>
#include "mutex.h"
#include "file_chunk.h"

class FileContent : public Mutex, private std::list<FileChunk>
{
	std::string filename;
	size_t file_size;
public:
	FileContent(std::string _filename, size_t _file_size) : filename(_filename), file_size(_file_size) {}

	/* Returns a copy of the chunk */
	FileChunk GetChunk(off_t offset, size_t size);

	/* Return true or false if we have it */
	/* Triggers loading the file from the cache or download from the network */
	bool HaveChunk(off_t offset, size_t size);

	void SetChunk(FileChunk chunk);

	const size_t GetFileSize() const;
};
#endif
