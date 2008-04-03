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
#include "peers_list.h"

Peer::Peer(pf_addr _addr, Connection* _conn, unsigned int _flags, Peer* parent)
			: addr(_addr),
			conn(dynamic_cast<ConnectionSsl*>(_conn)),
			ts_diff(0),
			incoming(NULL),
			uplink(parent),
			flags((conn ? ANONYMOUS : 0) | _flags)			  /* anonymous is only when this is a real connection */
{
	assert(conn);
	addr.id = conn->GetCertificateID();
	if(uplink)
		uplink->downlinks.push_back(this);
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

void Peer::SendMsg(const Packet& pckt)
{
	/* We aren't directly connected with him, so we send message
	 * to uplink (if connected with).
	 */
	if(!conn)
	{
		if(uplink)
			uplink->SendMsg(pckt);
		else
			log[W_ERR] << "Trying to send packet to " << this << " but this is my highlink"
				<< " and there isn't any connection with him!?";
		return;
	}
	log[W_PARSE] << "-> (" << GetFd() << "/" << GetID() << ") " << pckt.GetPacketInfo();

	send_queue.push(pckt);
	net.HavePacketToSend(this);
}

void Peer::SendHello()
{
	// Make the message
	Packet pckt(NET_HELLO, peers_list.GetMyID(), addr.id);

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

void Peer::Send_net_peer_list(StaticPeersList peers)
{
	for(StaticPeersList::iterator it = peers.begin(); it != peers.end(); ++it)
	{
		/* Do not send information about himself! */
		if(*it == this)
			continue;

		pf_id id = (*it)->uplink ? (*it)->uplink->GetID() : peers_list.GetMyID();

		/* It broadcasts. */
		Packet pckt(NET_PEER_CONNECTION, id, 0);
		pckt.SetArg(NET_PEER_CONNECTION_ADDRESS, (*it)->GetAddr());
		/* TODO: put certificate here! */
		pckt.SetArg(NET_PEER_CONNECTION_CERTIFICATE, std::string("TODO: put certificate here"));

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

	/* Disallow broadcast for this message */
	if(pckt->GetDstID() == 0)
		throw MustDisconnect();

	/* Flags */
	uint32_t flags = pckt->GetArg<uint32_t>(NET_HELLO_FLAGS);
	SetHighLink(flags & NET_HELLO_FLAGS_HIGHLINK);

	ts_diff = static_cast<uint32_t>(time(NULL)) - pckt->GetArg<uint32_t>(NET_HELLO_NOW);
	addr.port = (uint16_t) pckt->GetArg<uint32_t>(NET_HELLO_PORT);

	DelFlag(ANONYMOUS);

	/* If this is a client, we answer an HELLO message. */
	if(IsClient())
		SendHello();

	if(IsHighLink())
	{
		SetFlag(MERGING);

		/* I send all of my links */
		Send_net_peer_list(net.GetDirectHighLinks());
		SendMsg(Packet(NET_END_OF_MERGE, peers_list.GetMyID(), 0));

		/* Tell to all of my other links that this peer is connected. */
		Packet pckt(NET_PEER_CONNECTION, peers_list.GetMyID(), 0);
		pckt.SetArg(NET_PEER_CONNECTION_ADDRESS, GetAddr());
		pckt.SetArg(NET_PEER_CONNECTION_CERTIFICATE, std::string("TODO: put certificate here"));

		peers_list.Broadcast(pckt, this);  /* Don't send to this peer a creation message about him! */
	}
}

void Peer::Handle_net_end_of_merge(struct Packet* msg)
{
	if(HasFlag(MERGING))
	{
		SetFlag(MERGING_ACK);
		SendMsg(Packet(NET_END_OF_MERGE_ACK, peers_list.GetMyID(), GetID()));
	}

	/* Now we can update resp files :)))))))
	 * This function will send all messages to all new
	 * responsibles of files I am not responsible anymore.
	 */
	cache.UpdateRespFiles();
}

void Peer::Handle_net_end_of_merge_ack(struct Packet* msg)
{
	DelFlag(MERGING_ACK);
}

/** NET_PEER_CONNECTION
 *
 * Args:
 * NET_PEER_CONNECTION_ADDRESS
 * NET_PEER_CONNECTION_CERTIFICATE
 *
 * IMPORTANT: Sender of this packet is the uplink of this peer.
 */
void Peer::Handle_net_peer_connection(struct Packet* msg)
{
	pf_addr addr = msg->GetArg<pf_addr>(NET_PEER_CONNECTION_ADDRESS);
	Certificate cert;

	Peer* already_connected = peers_list.PeerFromID(addr.id);

	if(already_connected)
		return;				  /* I'm already connected to him. */

	Peer* p;

	try
	{
		p = net.Connect(addr);

		p->SetHighLink(false);
		p->SendHello();

	}
	catch(Network::CantConnectTo &e)
	{
		// BUG: ??? -lds
		/* No, we can't connect to this peer, so we create
		 * a Peer object with no connection. -romain
		 */
		p = net.AddPeer(new Peer(addr, NULL));
	}

	/* This is my child */
	p->uplink = this;
	downlinks.push_back(p);

}

void Peer::Handle_net_peer_goodbye(struct Packet* msg)
{
	/* */
}

void Peer::Handle_net_mkfile(struct Packet* msg)
{
	std::string filename;

	try
	{
		filename = msg->GetArg<std::string>(NET_MKFILE_PATH);
		pf_stat stat;
		stat.mode = msg->GetArg<uint32_t>(NET_MKFILE_MODE);
		stat.uid = msg->GetArg<uint32_t>(NET_MKFILE_UID);
		stat.gid = msg->GetArg<uint32_t>(NET_MKFILE_GID);
		stat.size = msg->GetArg<uint64_t>(NET_MKFILE_SIZE);
		stat.atime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_ACCESS_TIME));
		stat.mtime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_MODIF_TIME));
		stat.ctime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_CREATE_TIME));
		stat.meta_mtime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_META_MODIF_TIME));

		cache.MkFile(filename, stat, this);
	}
	catch(Cache::NoSuchFileOrDir &e)
	{
		log[W_DESYNCH] << "Unable to create " << filename << ": No such file or directory";
		/* XXX: Desync DO SOMETHING */

		cache.Unlock();
		return;
	}
}

void Peer::Handle_net_rmfile(struct Packet* msg)
{
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
		{ &Peer::Handle_net_mkfile,           0              },
		{ &Peer::Handle_net_rmfile,           0              },
		{ &Peer::Handle_net_peer_connection,  PERM_HIGHLINK  },
		{ &Peer::Handle_net_end_of_merge,     PERM_HIGHLINK  },
		{ &Peer::Handle_net_end_of_merge_ack, PERM_HIGHLINK  },
		{ &Peer::Handle_net_peer_goodbye,     PERM_HIGHLINK  },
	};

	/* Note tha we can safely cast pckt->type to unsigned after check pkct->type > 0 */
	if(pckt->GetType() <= 0 || (unsigned int)pckt->GetType() >= (sizeof handler/ sizeof *handler))
		throw Packet::Malformated();

	/* An anonymous peer can only send anonymous commands,
	 * and a no anonymous peer can NOT send an anonymous command.
	 * TODO: blacklist it?
	 */
	if(HasFlag(ANONYMOUS) ^ !!(handler[pckt->GetType()].perm & PERM_ANONYMOUS))
	{
		log[W_WARNING] << "Received an anonymous command from a registered peer, or a non anonymous command from an anonymous peer";
		throw Peer::MustDisconnect();
	}

	if(GetID() == !IsHighLink() && (handler[pckt->GetType()].perm & PERM_HIGHLINK))
	{
		log[W_WARNING] << "Received an HIGHLINK command from a non highlink peer";
		throw Peer::MustDisconnect();
	}

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
		delete []header;

		/* If there some data in packet, we wait for the rest on the next Receive() call.
		 * In other case, it is because packet only contains headers and we can parse it.
		 */
	}

	if(incoming->GetDataSize() > 0 && !incoming->ReceiveContent(conn))
		return false;

	log[W_PARSE] << "<- (" << GetFd() << "/" << GetID() << ") " << incoming->GetPacketInfo();

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
	if((*packet)->GetDstID() == 0 || (*packet)->GetDstID() == peers_list.GetMyID())
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
	else if((*packet)->GetDstID())
	{
		Peer* relay_to = peers_list.PeerFromID((*packet)->GetDstID());
		if(relay_to)
		{
			log[W_DEBUG] << "Relay packet to " << relay_to->GetID();
			relay_to->SendMsg(**packet);
		}
		else
			log[W_WARNING] << "Received a message to an unknown ID !?"
				<< "from=" << (*packet)->GetSrcID() << " to=" << (*packet)->GetDstID();
	}

	/* Route broadcast packets */
	if((*packet)->GetDstID() == 0)
		peers_list.Broadcast(**packet, this);

	return true;
}
