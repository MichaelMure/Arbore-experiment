#include "file_content.h"
#include "peers_list.h"
#include "packet.h"

void FileContent::NetworkRequestChunk(FileChunk chunk)
{
	if(sharers.size() == 0)
	{
		// We don't know who have this file, so ask it first
		net_pending_request.insert(chunk);
		if(!waiting_for_sharers)
		{
			Packet packet(NET_WHO_HAS_FILE);
			packet.SetArg(NET_WHO_HAS_FILE_PATH, filename);
			peers_list.Broadcast(packet);
			waiting_for_sharers = true;
		}
	}
	else
		FlushRequests();
}

void FileContent::FlushRequests()
{
}
