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

Peer::Peer(pf_addr _addr, Connection* _conn, Peer* parent)
			: addr(_addr),
			conn(_conn),
			ts_diff(0),
			incoming(NULL),
			uplink(parent),
			flags(conn ? ANONYMOUS : 0)			  /* anonymous is only when this is a real connection */
{
}

Peer::~Peer()
{
	delete incoming;
	delete conn;

	if(uplink)
	{
		std::vector<Peer*>::iterator it = uplink->downlinks.begin();
		while(it != uplink->downlinks.end() && *it != this)
			;
		if(it != uplink->downlinks.end())
			uplink->downlinks.erase(it);
	}
}

void Peer::Flush()
{
	while(!send_queue.empty())
	{
		send_queue.front().Send(conn);
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
						  // Time / now
	pckt.SetArg(NET_HELLO_NOW, (uint32_t)time(NULL));
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

		/* It broadcasts. */
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

	/* Flags */
	uint32_t flags = pckt->GetArg<uint32_t>(NET_HELLO_FLAGS);
	SetHighLink(flags & NET_HELLO_FLAGS_HIGHLINK);

	addr.id = pckt->GetSrcID();		  /* TODO: we'll know ID with certificate, when connection is SSL. */
	ts_diff = static_cast<uint32_t>(time(NULL)) - pckt->GetArg<uint32_t>(NET_HELLO_NOW);
	addr.port = (uint16_t) pckt->GetArg<uint32_t>(NET_HELLO_PORT);

	DelFlag(ANONYMOUS);

	/* If this is a client, we answer an HELLO message. */
	if(IsClient())
		SendHello();

	if(IsHighLink())
	{
		SetFlag(MERGING);

		if(IsClient())
		{

			/* If I'm server, I tell client that merge is starting... He can give
			 * me all of his links.
			 */
			SendMsg(Packet(NET_START_MERGE, net.GetMyID(), GetID()));

			/* Step 1: I send all of my links */
			log[W_DEBUG] << "Starting merge with " << this;

			Send_net_peer_list(net.GetDirectHighLinks());
			SendMsg(Packet(NET_END_OF_MERGE, net.GetMyID(), 0));
		}

		/* Tell to all of my other links that this peer is connected. */
		Packet pckt(NET_PEER_CONNECTION, net.GetMyID(), 0);
		pckt.SetArg(NET_PEER_CONNECTION_ADDRESS, GetAddr());
		pckt.SetArg(NET_PEER_CONNECTION_CERTIFICATE, "TODO: put certificate here");

		net.Broadcast(pckt, this);	  /* Don't send to this peer a creation message about him! */
	}

	/* Change dst ID of packet to NOT broadcast it. */
	pckt->SetDstID(net.GetMyID());
}

void Peer::Handle_net_start_merge(struct Packet* pckt)
{
	/* This peer is directly connected to me, so I
	 * think this is me who merge with him.
	 */
	assert(IsServer());
	assert(HasFlag(MERGING));
	assert(IsDirectLink());

	/* Step 2: client sends all of his links.
	 * Note: this is a broadcast to this part of network (new part).
	 */

	log[W_DEBUG] << "Starting merge with " << this;

	Send_net_peer_list(net.GetDirectHighLinks());

	SendMsg(Packet(NET_END_OF_MERGE, net.GetMyID(), 0));
}

void Peer::Handle_net_end_of_merge(struct Packet* msg)
{
	if(HasFlag(MERGING))
	{
		SetFlag(MERGING_ACK);
		SendMsg(Packet(NET_END_OF_MERGE_ACK, net.GetMyID(), GetID()));
	}

	/* Now we can update resp files :)))))))
	 * This function will send all messages to all new
	 * responsibles of files I am not responsible anymore.
	 */
	log[W_DEBUG] << "End of merge... we send all files";
	cache.UpdateRespFiles();
}

void Peer::Handle_net_end_of_merge_ack(struct Packet* msg)
{
	DelFlag(MERGING_ACK);
}

/** NET_PEER_CONNECTION
 *
 * NET_PEER_CONNECTION_ADDRESS
 * NET_PEER_CONNECTION_CERTIFICATE
 */
void Peer::Handle_net_peer_connection(struct Packet* msg)
{
	pf_addr addr = msg->GetArg<pf_addr>(NET_PEER_CONNECTION_ADDRESS);
	Certificate cert;

	Peer* already_connected = net.ID2Peer(addr.id);

	if(already_connected)
	{
		if(cert == already_connected->GetCertificate())
			return;			  /* I'm already connected to him. */
		else
			return;			  /* TODO: this is an other peer with same ID !? DO SOMETHING! */
	}

	try
	{
		Peer* p = net.Connect(addr);

		p->SendHello();

	}
	catch(Network::CantConnectTo &e)
	{
		// BUG: ???
		net.AddPeer(new Peer(addr, NULL));
	}
}

void Peer::Handle_net_mkfile(struct Packet* msg)
{
	cache.Lock();
	std::string filename;

	FileEntry* leaf;

	try
	{
		filename = msg->GetArg<std::string>(NET_MKFILE_PATH);
		mode_t mode = msg->GetArg<uint32_t>(NET_MKFILE_MODE);

		leaf = cache.MkFile(filename, mode);
	}
	catch(Cache::FileAlreadyExists &e)
	{
		/* This file already exists, but do not panic! We take modifications only if
		 * this file is more recent than mine.
		 */
		leaf = e.file;

		time_t dist_ts = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_MODIF_TIME));

		/* TODO SEE WHAT THE FUCK TO DO WHEN TIMESTAMP ARE EQUAL
		 * Note that we can consider this is the same version, so we do nothing.
		 */
		if(leaf->stat.mtime > dist_ts)
		{
			Packet pckt = cache.CreateMkFilePacket(leaf);
			pckt.SetDstID(GetID());
			SendMsg(pckt);
		}
		else if(leaf->stat.mtime == dist_ts)
		{
			cache.Unlock();
			return;			  /* same version, go out */
		}

		leaf->stat.mode = msg->GetArg<uint32_t>(NET_MKFILE_MODE);
	}
	catch(Cache::NoSuchFileOrDir &e)
	{
		log[W_DESYNCH] << "Unable to create " << filename << ": No such file or directory";
		/* XXX: Desync DO SOMETHING */

		cache.Unlock();
		return;
	}

	leaf->stat.uid = msg->GetArg<uint32_t>(NET_MKFILE_UID);
	leaf->stat.gid = msg->GetArg<uint32_t>(NET_MKFILE_GID);
	leaf->stat.size = msg->GetArg<uint64_t>(NET_MKFILE_SIZE);
	leaf->stat.atime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_ACCESS_TIME));
	leaf->stat.mtime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_MODIF_TIME));
	leaf->stat.ctime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_CREATE_TIME));

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

bool Peer::Receive()
{
	// Receive the header
	if(!incoming)
	{
		/* This is a new packet, we only receive the header */
		char* header;

		if(!conn->Read(&header, Packet::GetHeaderSize()))
			return false;

		incoming = new Packet(header);
		delete header;

		log[W_PARSE] << "Received a message header: type=" << incoming->GetType() << ", " <<
			" srcid=" << incoming->GetSrcID() << ", " <<
			" dstid=" << incoming->GetDstID() << ", " <<
			" size=" << incoming->GetDataSize();

		/* If there some data in packet, we wait for the rest on the next Receive() call.
		 * In other case, it is because packet only contains headers and we can parse it.
		 */
	}

	if(incoming->GetDataSize() > 0 && !incoming->ReceiveContent(conn))
		return false;

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

	return true;
}
