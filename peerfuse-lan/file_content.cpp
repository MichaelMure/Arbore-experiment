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
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * 
 */

#include <time.h>
#include "file_content.h"
#include "peers_list.h"
#include "packet.h"
#include "pf_log.h"

const time_t ref_request_timeout = 5;
const time_t ref_request_refresh = 30;

FileContentBase::chunk_availability FileContent::NetworkRequestChunk(FileChunkDesc chunk)
{
	BlockLockMutex lock(this);
	time_t now = time(NULL);
	if(now > ref_request_time + ref_request_refresh)
	{
		// We don't know who have this file, so ask it first
		Packet packet(NET_WHO_HAS_FILE);
		packet.SetArg(NET_WHO_HAS_FILE_PATH, filename);
		peers_list.Broadcast(packet);
		ref_request_time = time(NULL);
		pf_log[W_DEBUG] << "Now waiting for sharers to advertise their files";
	}
	// Check the chunk presence on the network
	if(now > ref_request_time + ref_request_timeout)
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

bool FileContent::WaitsForNetChunks()
{
	BlockLockMutex lock(this);
	return net_pending_request.size() != 0;
}
