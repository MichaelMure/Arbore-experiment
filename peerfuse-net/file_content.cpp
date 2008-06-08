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
 * This product includes cryptographic software written by Eric Younganus
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * $Id$
 */

#include "file_content.h"
#include "peers_list.h"
#include "packet.h"
#include "cache.h"
#include "environment.h"

const time_t ref_request_refresh = 30;

void FileContent::SetChunk(FileChunk chunk)
{
	FileContentBase::SetChunk(chunk);

	cache.AddSharer(filename, environment.my_id.Get());
}

FileContentBase::chunk_availability FileContent::NetworkRequestChunk(FileChunkDesc chunk)
{
	time_t now = time(NULL);
	if(now > ref_request_time + ref_request_refresh)
	{
		// We don't know who have which part of the file
		pf_log[W_DEBUG] << "Request for file refs";
		cache.RequestFileRefs(filename);
		ref_request_time = time(NULL);
	}

	// Check the chunk presence on the network
	if(/* TODO: enter this only if all FILE_REF have been received */ false)
	{
		std::map<pf_id, struct sharedchunks>::iterator it;
		bool found = false;
		for(it = sharers.begin(); it != sharers.end(); ++it)
		{
			if(it->second.offset <= chunk.GetOffset() && it->second.offset + it->second.size >= chunk.GetOffset() + (off_t)chunk.GetSize())
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			pf_log[W_INFO] << "Some parts of \"" << filename << "\" are not available on the network.";
			return CHUNK_UNAVAILABLE;
		}
	}
	if(sharers.size() != 0)
		NetworkFlushRequests();

	return CHUNK_NOT_READY;
}
