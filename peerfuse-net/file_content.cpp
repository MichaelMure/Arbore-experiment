#include "file_content.h"
#include "peers_list.h"
#include "packet.h"

void FileContent::NetworkRequestChunk(FileChunk chunk)
{
	if(sharers.size() == 0)
	{
		net_pending_request.insert(chunk);
		// We don't know who have which part of the file
		if(!waiting_for_sharers)
		{
			Packet packet(NET_WANT_REF_FILE);
			packet.SetArg(NET_WANT_REF_FILE_PATH, filename);
			/* TODO: send packet to owners of this file */
			waiting_for_sharers = true;
		}
	}
	else
		NetworkFlushRequests();
}


