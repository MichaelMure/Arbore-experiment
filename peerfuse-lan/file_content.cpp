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
#include "log.h"

void FileContent::NetworkRequestChunk(FileChunk chunk)
{
	BlockLockMutex lock(this);
	if(sharers.size() == 0)
	{
		log[W_DEBUG] << "Waiting for sharers to advertise their files";
		// We don't know who have this file, so ask it first
		if(!waiting_for_sharers)
		{
			Packet packet(NET_WHO_HAS_FILE);
			packet.SetArg(NET_WHO_HAS_FILE_PATH, filename);
			peers_list.Broadcast(packet);
			waiting_for_sharers = true;
		}
	}
	else
		NetworkFlushRequests();
}

bool FileContent::WantsChunks()
{
	BlockLockMutex lock(this);
	return net_pending_request.size() != 0;
}
