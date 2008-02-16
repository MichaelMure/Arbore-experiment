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

#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include "cache.h"
#include "packet.h"
#include "peer.h"
#include "network.h"
#include "net_proto.h"
#include "pf_types.h"
#include "pfnet.h"
#include "log.h"
#include "session_config.h"

Peer::Peer(int _fd, pf_addr _addr, Peer* parent)
	: fd(_fd),
	  addr(_addr),
	  ts_diff(0),
	  incoming(NULL),
	  uplink(parent),
	  flags(_fd >= 0 ? ANONYMOUS : 0) /* anonymous is only when this is a real connection */
{
}

Peer::~Peer()
{
	if(fd)
		close(fd);
	delete incoming;

	if(uplink)
	{
		std::vector<Peer*>::iterator it = uplink->downlinks.begin();
		while(it != uplink->downlinks.end() && *it != this);
		if(it != uplink->downlinks.end())
			uplink->downlinks.erase(it);
	}
}

void Peer::Flush()
{
	while(!send_queue.empty())
	{
		send_queue.front().Send(fd);
		send_queue.pop();
	}
}

/****************************
 *  Send
 */

void Peer::SendMsg(const PacketBase& pckt)
{
	send_queue.push(dynamic_cast<const Packet&>(pckt));
	net.HavePacketToSend(this);
}

void Peer::SendHello()
{
	// Make the message
	Packet pckt(NET_HELLO, net.GetMyID(), addr.id);
	pckt.SetArg(NET_HELLO_NOW, (uint32_t)time(NULL)); // Time / now
	uint32_t flags = 0;
	if(IsHighLink())
		flags |= NET_HELLO_FLAGS_HIGHLINK;
	pckt.SetArg(NET_HELLO_FLAGS, flags);
	pckt.SetArg(NET_HELLO_PORT, (uint32_t)net.GetListeningPort());
	pckt.SetArg(NET_HELLO_VERSION, std::string(PEERFUSE_VERSION));
	SendMsg(pckt);
}

void Peer::Send_net_peer_list(PeerList peers)
{
	for(PeerList::iterator it = peers.begin(); it != peers.end(); ++it)
	{
		/* Do not send information about himself! */
		if(*it == this)
			continue;

		id_t id = (*it)->uplink ? (*it)->uplink->GetID() : net.GetMyID();

		Packet pckt(NET_PEER_CONNECTION, id, 0);
		pckt.SetArg(NET_PEER_CONNECTION_ADDRESS, (*it)->GetAddr());
		pckt.SetArg(NET_PEER_CONNECTION_CERTIFICATE, "TODO: put certificate here");

		SendMsg(pckt);

		if((*it)->downlinks.empty() == false)
			Send_net_peer_list((*it)->downlinks);
	}
}

/*******************************
 * Handers
 */

/** HELLO MESSAGE
 *
 * NET_HELLO_NOW
 * NET_HELLO_FLAGS
 * NET_HELLO_PORT
 * NET_HELLO_VERSION
 */
void Peer::Handle_net_hello(struct Packet* pckt)
{
	/* Version check */
	if(pckt->GetArg<std::string>(NET_HELLO_VERSION) != std::string(PEERFUSE_VERSION))
	{
		log[W_WARNING] << "Versions are different !";
		throw MustDisconnect();
	}

	/* Set timestamp diff between this peer and me. */
	ts_diff = time(NULL) - pckt->GetArg<uint32_t>(NET_HELLO_NOW);

	/* What is port listened by peer */
	addr.port = pckt->GetArg<uint32_t>(NET_HELLO_PORT);

	/* Flags */
	uint32_t flags = pckt->GetArg<uint32_t>(NET_HELLO_FLAGS);
	SetHighLink(flags & NET_HELLO_FLAGS_HIGHLINK);

	/* TODO: we'll know ID with certificate, when connection is SSL. */
	addr.id = pckt->GetSrcID();

	/* Nous sommes maintenant intimes */
	DelFlag(ANONYMOUS);

	if(IsServer())
	{
		/* CLIENT SIDE */

		/* If this is a highlink, I'm waiting for merge */
		if(IsHighLink())
			SetFlag(MERGING);
	}
	else
	{
		/* SERVER SIDE */

		/* TODO: check if ID is already used */

		SendHello();

		if(IsHighLink())
		{
			SetFlag(MERGING);
			net.Broadcast(Packet(NET_START_MERGE, net.GetMyID(), 0));
		}
	}

	/* Change dst ID of packet to NOT broadcast it. */
	pckt->SetDstID(net.GetMyID());
}

void Peer::Handle_net_start_merge(struct Packet* pckt)
{
	SetFlag(MERGING);

	if(IsDirectLink())
	{
		/* This peer is directly connected to me, so I
		 * think this is me who merge with him.
		 */

		Send_net_peer_list(net.GetDirectHighLinks());

		SendMsg(Packet(NET_END_OF_MERGE, net.GetMyID(), 0));
	}
}

void Peer::Handle_net_end_of_merge(struct Packet* msg)
{
	DelFlag(MERGING);
	if(IsClient())
	{
		SendMsg(Packet(NET_END_OF_MERGE_ACK, net.GetMyID(), this->GetID()));
		session_cfg.Set("last_view", time(NULL));
	}
}

void Peer::Handle_net_end_of_merge_ack(struct Packet* msg)
{
	DelFlag(MERGING);
}

void Peer::Handle_net_peer_connection(struct Packet* msg)
{
	pf_addr addr = msg->GetArg<pf_addr>(NET_PEER_CONNECTION_ADDRESS);
#if 0
	AddrList my_addr_list;
	PeerList peers = net.GetPeerList();

	for(PeerList::iterator it = peers.begin(); it != peers.end(); ++it)
		if(*it != this)
			my_addr_list.push_back((*it)->GetAddr());

	for(AddrList::iterator it = addr_list.begin(); it != addr_list.end(); ++it)
	{
		Peer* already_peer = net.ID2Peer(it->id);
		if(already_peer)
		{
			log[W_WARNING] << it->id << " already used! Ask him to change it!";
			Packet change_id_pckt(NET_CHANGE_YOUR_ID, net.GetMyID(), already_peer->GetID());
			change_id_pckt.SetArg(NET_CHANGE_YOUR_ID_ID, net.CreateID());
			already_peer->SendMsg(change_id_pckt);
			it->id = change_id_pckt.GetArg<uint32_t>(NET_CHANGE_YOUR_ID);
			/* TODO: check and complete */
		}

		try
		{
			net.Connect(*it);
		}
		catch(Network::CantConnectTo &e)
		{
			/* TODO: do something (or not) */
			net.AddPeer(new Peer(-1, *it));
		}
	}

	if(HasFlag(MERGING))
	{
		if(IsServer())
		{
			SendMsg(Packet(NET_PEER_CONNECTION, net.GetMyID(), 0).SetArg(NET_PEER_CONNECTION_ADDRESSES, my_addr_list));

			Send_net_get_struct_diff();
		}
		addr_list.push_back(this->GetAddr());
		msg->SetSrcID(net.GetMyID());
		msg->SetArg(NET_PEER_CONNECTION_ADDRESSES, addr_list);
	}
#endif
}

void Peer::Handle_net_mkfile(struct Packet* msg)
{
	cache.Lock();
	std::string filename;
	try
	{
		filename = msg->GetArg<std::string>(NET_MKFILE_PATH);
		mode_t mode = msg->GetArg<uint32_t>(NET_MKFILE_MODE);

		FileEntry* leaf = cache.MkFile(filename, mode);

		leaf->stat.uid = msg->GetArg<uint32_t>(NET_MKFILE_UID);
		leaf->stat.gid = msg->GetArg<uint32_t>(NET_MKFILE_GID);
		leaf->stat.size = msg->GetArg<uint64_t>(NET_MKFILE_SIZE);
		leaf->stat.atime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_ACCESS_TIME));
		leaf->stat.mtime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_MODIF_TIME));
		leaf->stat.ctime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_CREATE_TIME));
	}
	catch(Cache::NoSuchFileOrDir &e)
	{
		log[W_DESYNCH] << "Unable to create " << filename << ": No such file or directory";
		/* XXX: Desync DO SOMETHING */
	}
	catch(Cache::FileAlreadyExists &e)
	{
		log[W_DESYNCH] << "Unable to create " << filename << ": File already exists";
		/* XXX: DO SOMETHING */
	}
	cache.Unlock();
}

void Peer::Handle_net_rmfile(struct Packet* msg)
{
	cache.Lock();
	try
	{
		cache.RmFile(msg->GetArg<std::string>(NET_RMFILE_PATH));
	}
	catch(Cache::NoSuchFileOrDir &e)
	{
		log[W_DESYNCH] << "Unable to remove " << msg->GetArg<std::string>(NET_RMFILE_PATH) << ": No such file or directory";
		/* TODO: Desynch, DO SOMETHING */
	}
	catch(Cache::DirNotEmpty &e)
	{
		log[W_DESYNCH] << "Unable to remove " << msg->GetArg<std::string>(NET_RMFILE_PATH) << ": Dir not empty";
		/* TODO: Desynch, DO SOMETHING */
	}
	cache.Unlock();
}

void Peer::HandleMsg(Packet* pckt)
{
	struct
	{
		void (Peer::*func) (Packet*);
		int perm;
		#define PERM_ANONYMOUS 0x1
		#define PERM_HIGHLINK  0x2
	} handler[NET_NB_MESSAGES] =
	{
		{ NULL,                               0              },
		{ &Peer::Handle_net_hello,            PERM_ANONYMOUS },
		{ &Peer::Handle_net_start_merge,      PERM_HIGHLINK  },
		{ &Peer::Handle_net_mkfile,           PERM_HIGHLINK  },
		{ &Peer::Handle_net_rmfile,           PERM_HIGHLINK  },
		{ &Peer::Handle_net_peer_connection,  PERM_HIGHLINK  },
		{ &Peer::Handle_net_end_of_merge,     PERM_HIGHLINK  },
		{ &Peer::Handle_net_end_of_merge_ack, PERM_HIGHLINK  },
	};

	/* Note tha we can safely cast pckt->type to unsigned after check pkct->type > 0 */
	if(pckt->GetType() <= 0 || (unsigned int)pckt->GetType() >= (sizeof handler/ sizeof *handler))
		throw Packet::Malformated();

	/* An anonymous peer can only send anonymous commands,
	 * and a no anonymous peer can NOT send an anonymous command.
	 * TODO: blacklist it?
	 */
	if(HasFlag(ANONYMOUS) ^ !!(handler[pckt->GetType()].perm & PERM_ANONYMOUS))
		throw Peer::MustDisconnect();

	if(!IsHighLink() && (handler[pckt->GetType()].perm & PERM_HIGHLINK))
		throw Peer::MustDisconnect();

	(this->*handler[pckt->GetType()].func)(pckt);
}

void Peer::Receive()
{
	/* If there was already an incoming packet, we can receive its content */
	if(incoming)
		incoming->ReceiveContent(fd);
	else
	{
		/* This is a new packet, we only receive the header */
		char header[ Packet::GetHeaderSize() ];

		if(recv(fd, &header, Packet::GetHeaderSize(), 0) <= 0)
			throw Packet::RecvError();

		incoming = new Packet(header);

		log[W_PARSE] << "Received a message header: type=" << incoming->GetType() << ", " <<
			                                   " srcid=" << incoming->GetSrcID() << ", " <<
							   " dstid=" << incoming->GetDstID() << ", " <<
							   " size=" << incoming->GetDataSize();

		/* If there some data in packet, we wait for the rest on the next Receive() call.
		 * In other case, it is because packet only contains headers and we can parse it.
		 */
		if(incoming->GetDataSize() > 0)
			return;
	}

	/* We use the Deleter class because we don't know how we will
	 * exit this function. With it, we are *sure* than Packet instance
	 * will be free'd.
	 */
	Deleter<Packet> packet(incoming);
	incoming = NULL;

	/* A packet MUST have a sender. Everybody knows his ID! */
	if((*packet)->GetSrcID() == 0)
		throw Packet::Malformated();

	/* Only handle this packet if it is a broadcast or if it is sent to me */
	if((*packet)->GetDstID() == 0 || (*packet)->GetDstID() == net.GetMyID() || !net.GetMyID())
	{
		/* If source is not me, I translate packet to real source. */
		if((*packet)->GetSrcID() && (*packet)->GetSrcID() != GetID() && GetID())
		{
			log[W_DEBUG] << "Translate packet from " << GetID() << " to " << (*packet)->GetSrcID();
			net.GivePacketTo((*packet)->GetSrcID(), *packet);
		}
		else
			HandleMsg(*packet);
	}

	/* Route broadcast packets */
	if((*packet)->GetDstID() == 0)
		net.Broadcast(**packet, this);
}

