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
#include "peers_list.h"
#include "packet.h"
#include "cache.h"

FileContentBase::chunk_availability FileContent::NetworkRequestChunk(FileChunkDesc chunk)
{
	if(sharers.size() == 0 && !waiting_for_sharers)
	{
		// We don't know who have which part of the file
		log[W_DEBUG] << "Request for file refs";
		cache.RequestFileRefs(filename);
		waiting_for_sharers = true;
	}
	else
	{
		// Check the chunk presence on the network
		if(/* TODO: enter this only if all FILE_REF have been received */ true)
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
				log[W_INFO] << "Some parts of \"" << filename << "\" are not available on the network.";
				return CHUNK_UNAVAILABLE;
			}
		}
		NetworkFlushRequests();
	}
	return CHUNK_NOT_READY;
}
