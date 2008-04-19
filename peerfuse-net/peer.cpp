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
#include "packet.h"
#include "peer.h"
#include "net_proto.h"
#include "pf_types.h"
#include "pfnet.h"
#include "log.h"
#include "session_config.h"
#include "peers_list.h"
#include "scheduler_queue.h"
#include "job_flush_peer.h"
#include "job_mkfile.h"
#include "job_rmfile.h"
#include "job_file_setattr.h"
#include "job_update_resp_files.h"
#include "job_send_ref_file.h"
#include "job_send_chunk.h"
#include "job_set_chunk.h"
#include "job_set_shared_part.h"
#include "connection_ssl.h"
#include "environment.h"

Peer::Peer(pf_addr _addr, Connection* _conn, unsigned int _flags, pf_id parent)
			:						  /* anonymous is only when this is a real connection */
			PeerBase(_addr, _conn, (_conn ? ANONYMOUS : 0) | _flags),
			uplink(parent)
{
	/* If there isn't any connection with this peer, it must have an ID in addr */
	assert(conn != NULL || addr.id > 0);
	if(conn)
		addr.id = static_cast<ConnectionSsl*>(conn)->GetCertificateID();
	if(GetID() == environment.my_id.Get() && IsServer())
		throw SelfConnect();
}

Peer::~Peer()
{
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
		/* TODO: find the fast path */
		if(uplink)
			peers_list.SendMsg(uplink, pckt);
		else
			log[W_ERR] << "Trying to send packet to " << this << " but this is my highlink"
				<< " and there isn't any connection with him!?";
		return;
	}
	log[W_PARSE] << "-> (" << GetFd() << "/" << GetID() << ") " << pckt.GetPacketInfo();

	send_queue.push(pckt);
	scheduler_queue.Queue(new JobFlushPeer(GetFd()));
}

void Peer::SendHello()
{
	// Make the message
	Packet pckt(NET_HELLO, environment.my_id.Get(), addr.id);

	// Time / now
	pckt.SetArg(NET_HELLO_NOW, (uint32_t)time(NULL));
	uint32_t flags = 0;
	if(IsHighLink())
		flags |= NET_HELLO_FLAGS_HIGHLINK;
	pckt.SetArg(NET_HELLO_FLAGS, (uint32_t)flags);
	pckt.SetArg(NET_HELLO_PORT, (uint32_t)environment.listening_port.Get());
	pckt.SetArg(NET_HELLO_VERSION, std::string(PEERFUSE_VERSION));
	SendMsg(pckt);
}

void Peer::RequestChunk(std::string filename, off_t offset, size_t size)
{
	std::map<uint32_t, std::string>::iterator it = file_refs.begin();

	while(it != file_refs.end() && it->second != filename)
		++it;

	if(it == file_refs.end())
		return;

	Packet packet(NET_WANT_CHUNK);
	packet.SetArg(NET_WANT_CHUNK_REF, it->first);
	packet.SetArg(NET_WANT_CHUNK_OFFSET, (uint64_t)offset);
	packet.SetArg(NET_WANT_CHUNK_SIZE, (uint32_t)size);
	SendMsg(packet);
}

void Peer::SendChunk(uint32_t ref, FileChunk& chunk)
{
#if 0
	std::map<uint32_t, std::string>::iterator it = file_refs.begin();

	while(it != file_refs.end() && it->second != filename)
		++it;

	if(it == file_refs.end())
	{
		return;
	}
#endif
	Packet pckt(NET_CHUNK);
	pckt.SetArg(NET_CHUNK_REF, ref);
	pckt.SetArg(NET_CHUNK_CHUNK, chunk);
	SendMsg(pckt);
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

	/* Forbid broadcast for this message */
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
		peers_list.SendPeerList(this);
		SendMsg(Packet(NET_END_OF_MERGE, environment.my_id.Get(), 0));

		/* Tell to all of my other links that this peer is connected. */
		Packet pckt(NET_PEER_CONNECTION, environment.my_id.Get(), 0);
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
		SendMsg(Packet(NET_END_OF_MERGE_ACK, environment.my_id.Get(), GetID()));
	}

	/* Now we can update resp files :)))))))
	 * This function will send all messages to all new
	 * responsibles of files I am not responsible anymore.
	 */
	scheduler_queue.Queue(new JobUpdateRespFiles());
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

	if(peers_list.IsIDOnNetwork(addr.id))
		return;				  /* I'm already connected to him. */

	Peer* p = new Peer(addr, NULL, 0, GetID());
	peers_list.Add(p);

	downlinks.push_back(addr.id);
}

/** NET_PEER_GOODBYE
 *
 * Sender of this message has left network.
 */
void Peer::Handle_net_peer_goodbye(struct Packet* msg)
{
	peers_list.RemoveDownLinks(this);
	peers_list.RemoveFromID(GetID());

}

void Peer::Handle_net_mkfile(struct Packet* msg)
{
	std::string filename;
	filename = msg->GetArg<std::string>(NET_MKFILE_PATH);

	pf_stat stat;
	stat.mode = msg->GetArg<uint32_t>(NET_MKFILE_MODE);
	stat.uid = msg->GetArg<uint32_t>(NET_MKFILE_UID);
	stat.gid = msg->GetArg<uint32_t>(NET_MKFILE_GID);
	stat.size = msg->GetArg<uint64_t>(NET_MKFILE_SIZE);
	stat.atime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_ACCESS_TIME));
	stat.mtime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_MODIF_TIME));
	stat.meta_mtime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_META_MODIF_TIME));
	stat.ctime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_CREATE_TIME));
	IDList sharers = msg->GetArg<IDList>(NET_MKFILE_SHARERS);

	scheduler_queue.Queue(new JobMkFile(filename, stat, sharers, GetID()));
}

void Peer::Handle_net_rmfile(struct Packet* msg)
{
	std::string filename = msg->GetArg<std::string>(NET_RMFILE_PATH);
	scheduler_queue.Queue(new JobRmFile(filename, GetID()));
}

/** NET_I_HAVE_FILE
 *
 * Args:
 *	NET_I_HAVE_FILE_FILENAME
 */
void Peer::Handle_net_i_have_file(struct Packet* msg)
{
	/* TODO: implement it. */
}

/** NET_WANT_REF_FILE
 *
 * Peer wants a reference number for a file (he probably wants to download it)
 *
 * Args:
 *	NET_WANT_REF_FILE_PATH
 */
void Peer::Handle_net_want_ref_file(struct Packet* msg)
{
	std::string filename;
	filename = msg->GetArg<std::string>(NET_WANT_REF_FILE_PATH);
	scheduler_queue.Queue(new JobSendRefFile(filename, GetID()));
}

/** NET_REF_FILE
 *
 * Answer of NET_WANT_REF_FILE
 *
 * Args:
 *	NET_REF_FILE_PATH
 *	NET_REF_FILE_REF
 *	NET_REF_FILE_OFFSET
 *	NET_REF_FILE_SIZE
 */
void Peer::Handle_net_ref_file(struct Packet* msg)
{
	std::string filename;
	filename = msg->GetArg<std::string>(NET_REF_FILE_PATH);
	uint32_t ref = msg->GetArg<uint32_t>(NET_REF_FILE_REF);
	off_t offset = (off_t)msg->GetArg<uint64_t>(NET_REF_FILE_OFFSET);
	size_t size = (size_t)msg->GetArg<uint32_t>(NET_REF_FILE_SIZE);

	file_refs.insert(make_pair(ref, filename));
	scheduler_queue.Queue(new JobSetSharedPart(filename, GetID(), offset, size));
}

/** NET_WANT_CHUNK
 *
 * Want a part of file.
 *
 * Args:
 *	NET_WANT_CHUNK_REF
 *	NET_WANT_CHUNK_OFFSET
 *	NET_WANT_CHUNK_SIZE
 */
void Peer::Handle_net_want_chunk(struct Packet* msg)
{
	uint32_t ref = msg->GetArg<uint32_t>(NET_WANT_CHUNK_REF);
	off_t offset = (off_t)msg->GetArg<uint64_t>(NET_WANT_CHUNK_OFFSET);
	size_t size = (size_t)msg->GetArg<uint32_t>(NET_WANT_CHUNK_SIZE);
	scheduler_queue.Queue(new JobSendChunk(ref, GetID(), offset, size));
}

/** NET_CHUNK
 *
 * Receive a part of a file.
 *
 * Args:
 *	NET_CHUNK_REF
 *	NET_CHUNK_CHUNK
 */
void Peer::Handle_net_chunk(struct Packet* msg)
{
	uint32_t ref = msg->GetArg<uint32_t>(NET_CHUNK_REF);

	std::map<uint32_t, std::string>::iterator it;
	if((it = file_refs.find(ref)) == file_refs.end())
		return;

	FileChunk chunk = msg->GetArg<FileChunk>(NET_CHUNK_CHUNK);
	scheduler_queue.Queue(new JobSetChunk(it->second, chunk));
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
		{ &Peer::Handle_net_i_have_file,      0              },
		{ &Peer::Handle_net_want_ref_file,    0              },
		{ &Peer::Handle_net_ref_file,         0              },
		{ &Peer::Handle_net_want_chunk,       0              },
		{ &Peer::Handle_net_chunk,            0              },
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
	if(!PeerBase::ReceivePacket())
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
	if((*packet)->GetDstID() == 0 || (*packet)->GetDstID() == environment.my_id.Get())
	{
		/* If source is not me, I translate packet to real source. */
		if((*packet)->GetSrcID() && (*packet)->GetSrcID() != GetID() && GetID())
		{
			log[W_DEBUG] << "Translate packet from " << GetID() << " to " << (*packet)->GetSrcID();
			peers_list.GivePacketTo((*packet)->GetSrcID(), *packet);
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
