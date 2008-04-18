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
#include "file_content_base.h"
#include "file_chunk.h"
#include "pf_types.h"

class FileContent : public FileContentBase
{
	struct sharedchunks
	{
		pf_id sharer;
		FileChunk part;
	};
	bool waiting_for_sharers;
	std::list<struct sharedchunks> sharers;
	std::set<FileChunk, CompFileChunk> net_pending_request;

	FileChunk& operator=(const FileChunk &other);

	void FlushRequests();
public:
	FileContent(std::string path) : FileContentBase(path), waiting_for_sharers(false) {}
	~FileContent() {}

	void NetworkRequestChunk(FileChunk chunk);
	void SetSharer(pf_id, off_t offset, size_t size);
};
#endif						  /* FILE_CONTENT_H */
