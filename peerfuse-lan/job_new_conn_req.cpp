#include "peers_list.h"
#include "job_new_conn_req.h"

void JobNewConnReq::OnSuccess()
{
	// acknowledge the peer the connection is established
	Packet p(NET_PEER_CONNECTION_ACK);
	p.SetArg(NET_PEER_CONNECTION_ACK_ADDRESS, GetAddr());
	peers_list.SendMsg(requested_by, p);
}

void JobNewConnReq::OnFailure()
{
	// acknowledge the peer the connection has not been
	Packet p(NET_PEER_CONNECTION_RST);
	p.SetArg(NET_PEER_CONNECTION_RST_ADDRESS, GetAddr());
	peers_list.SendMsg(requested_by, p);
}
