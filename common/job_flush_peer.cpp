#include "job_flush_peer.h"
#include "mutex.h"
#include "network.h"
#include "peers_list.h"

bool JobFlushPeer::Start()
{
	BlockLockMutex lock(&peers_list);
	Peer* p = peers_list.PeerFromID(id);
	if(p)
		net.HavePacketToSend(p->GetFd());
	return false;
}
