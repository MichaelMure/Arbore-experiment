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
#include<time.h>
#include "file_content_base.h"
#include "file_chunk_desc.h"

class FileContent : public FileContentBase
{
	time_t ref_request_time;
public:
	FileContent(std::string path) : FileContentBase(path), ref_request_time(0) {}
	~FileContent() {}

	chunk_availability NetworkRequestChunk(FileChunkDesc chunk);
	bool WaitsForNetChunks();
};
#endif						  /* FILE_CONTENT_H */
