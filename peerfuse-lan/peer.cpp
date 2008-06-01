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
 * This product includes cryptographic software written by Eric Younganus
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
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
#include "environment.h"
#include "net_proto.h"
#include "pf_types.h"
#include "pflan.h"
#include "log.h"
#include "job_other_connect.h"
#include "job_new_conn_req.h"
#include "job_mkfile.h"
#include "job_send_changes.h"
#include "job_advertise_file.h"
#include "job_set_sharer.h"
#include "job_send_ref_file.h"
#include "job_send_chunk.h"
#include "job_set_chunk.h"
#include "job_set_shared_part.h"
#include "session_config.h"
#include "tools.h"
#include "peers_list.h"
#include "scheduler_queue.h"
#include "fs_utils.h"

const int merge_post_lastview_dt = 15 * 60;	  /* When merging, ask a diff of what's happened since last_view - 15m */

Peer::Peer(pf_addr _addr, Connection* _conn, unsigned int _flags) :
			PeerBase(_addr, _conn, _flags)
{
	if(IsServer())
		SendHello();
}

Peer::~Peer()
{
}

/****************************
 *  Send
 */

void Peer::SendMsg(const Packet& pckt)
{
	log[W_PARSE] << "-> (" << GetFd() << "/" << GetID() << ") " << pckt.GetPacketInfo();
	send_queue.push(pckt);
}

void Peer::SendHello()
{
	// Make the message
	Packet pckt(NET_HELLO);
						  // Time / now
	pckt.SetArg(NET_HELLO_NOW, (uint32_t)time(NULL));
	pckt.SetArg(NET_HELLO_PORT, (uint32_t)environment.listening_port.Get());
	pckt.SetArg(NET_HELLO_VERSION, std::string(PEERFUSE_VERSION));
	pckt.SetArg(NET_HELLO_MY_ID, environment.my_id.Get());
	SendMsg(pckt);
}

void Peer::SendStartMerge()
{
	log[W_DEBUG] << "Merge should start";
	// Initiate the merge if it's the first peer we connect to
	if(peers_list.Size() == 1)
	{
		environment.merging.Set(true);
		SetFlag(MERGING);
		SendMsg(Packet(NET_START_MERGE));
	}
}

void Peer::SendGetStructDiff()
{
	Packet pckt(NET_GET_STRUCT_DIFF);
	uint32_t last_v = 0;
	session_cfg.Get("last_view_" + TypToStr(GetID()), last_v);
	pckt.SetArg(NET_GET_STRUCT_DIFF_LAST_CONNECTION, last_v);
	SendMsg(pckt);
}

void Peer::RequestChunk(std::string filename, off_t offset, size_t size)
{
	std::map<uint32_t, std::string>::iterator it = file_refs.begin();

	while(it != file_refs.end() && it->second != filename)
		++it;

	if(it == file_refs.end())
	{
		log[W_ERR] << "This peer has no ref to this file ??";
		return;
	}

	uint32_t ref = it->first;
	Packet packet(NET_WANT_CHUNK);
	packet.SetArg(NET_WANT_CHUNK_REF, ref);
	packet.SetArg(NET_WANT_CHUNK_OFFSET, (uint64_t)offset);
	packet.SetArg(NET_WANT_CHUNK_SIZE, (uint32_t)size);
	SendMsg(packet);
	AddAskedChunk(ref, FileChunkDesc(offset, size));
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

void Peer::Handle_net_hello(struct Packet* pckt)
{
	if(pckt->GetArg<std::string>(NET_HELLO_VERSION) != std::string(PEERFUSE_VERSION))
	{
		log[W_WARNING] << "Versions are different !";
		throw MustDisconnect();
	}

	SetTimestampDiff(pckt->GetArg<uint32_t>(NET_HELLO_NOW));
	addr.port = (uint16_t) pckt->GetArg<uint32_t>(NET_HELLO_PORT);

	if(IsClient())
		SendHello();

	if(pckt->GetArg<uint32_t>(NET_HELLO_MY_ID) == 0)
	{
		// The peer don't have an ID, give him one
		// TODO: check it's not already used
		Packet p(NET_YOUR_ID);
		addr.id = peers_list.CreateID();
		p.SetArg(NET_YOUR_ID_ID, addr.id);
		SendMsg(p);
	}
	else
	if(pckt->GetArg<uint32_t>(NET_HELLO_MY_ID) == environment.my_id.Get())
		throw SelfConnect();
	else
		addr.id = pckt->GetArg<uint32_t>(NET_HELLO_MY_ID);

	if(IsServer() && environment.my_id.Get())  // Wait this peer send us an id before syncing
		SendStartMerge();
}

void Peer::Handle_net_your_id(struct Packet* pckt)
{
	if(environment.my_id.Get() != 0)
		throw MustDisconnect();
	environment.my_id.Set(pckt->GetArg<uint32_t>(NET_YOUR_ID_ID));
	session_cfg.Set("my_id", environment.my_id.Get());
	log[W_INFO] << "My ID now is " << environment.my_id.Get();

	if(IsServer())
		SendStartMerge();
}

void Peer::Handle_net_start_merge(struct Packet* pckt)
{
	SetFlag(MERGING);

	scheduler_queue.Queue(new JobOtherConnect(GetID()));
}

void Peer::Handle_net_peer_connection(struct Packet* msg)
{
	pf_addr new_peer = msg->GetArg<pf_addr>(NET_PEER_CONNECTION_ADDRESS);

	scheduler_queue.Queue(new JobNewConnReq(new_peer, addr.id));
}

void Peer::Handle_net_peer_connection_ack(struct Packet* msg)
{
	pf_addr new_peer = msg->GetArg<pf_addr>(NET_PEER_CONNECTION_ACK_ADDRESS);

	scheduler_queue.Lock();
	for(SchedulerQueue::iterator it = scheduler_queue.begin();
		it != scheduler_queue.end();
		++it)
	{
		if((*it)->GetType() == JOB_OTHER_CONNECT)
		{
			JobOtherConnect* j = static_cast<JobOtherConnect*>(*it);
			if(j->IsConnectingTo(new_peer))
				j->PeerConnected(GetID());
		}
	}
	scheduler_queue.Unlock();
}

void Peer::Handle_net_peer_connection_rst(struct Packet* msg)
{
	pf_addr new_peer = msg->GetArg<pf_addr>(NET_PEER_CONNECTION_RST_ADDRESS);

	scheduler_queue.Lock();
	for(SchedulerQueue::iterator it = scheduler_queue.begin();
		it != scheduler_queue.end();
		++it)
	{
		if((*it)->GetType() == JOB_OTHER_CONNECT)
		{
			JobOtherConnect* j = static_cast<JobOtherConnect*>(*it);
			if(j->IsConnectingTo(new_peer))
			{
				scheduler_queue.Cancel(j);
				break;
			}
		}
	}
	scheduler_queue.Unlock();

	// notifiy the peer he can't be contacted by *this
	Packet pckt(NET_PEER_CONNECTION_REJECTED);
	pckt.SetArg(NET_PEER_CONNECTION_REJECTED_ADDRESS, GetAddr());
	peers_list.SendMsg(new_peer.id, pckt);
}

void Peer::Handle_net_peer_connection_rejected(struct Packet* msg)
{
	pf_addr peer = msg->GetArg<pf_addr>(NET_PEER_CONNECTION_REJECTED_ADDRESS);

	log[W_ERR] << "Host " << peer << " can't connect.";
	throw PeerCantConnect(peer);
}

void Peer::Handle_net_peer_all_connected(struct Packet* msg)
{
	SendGetStructDiff();
}

void Peer::Handle_net_get_struct_diff(struct Packet* pckt)
{
	time_t last_view = Timestamp(pckt->GetArg<uint32_t>(NET_GET_STRUCT_DIFF_LAST_CONNECTION));

	scheduler_queue.Queue(new JobSendChanges(GetID(), last_view - merge_post_lastview_dt ));
}

void Peer::Handle_net_end_of_diff(struct Packet* pckt)
{
	if(IsClient())
		SendMsg(Packet(NET_END_OF_MERGE));
}

void Peer::Handle_net_mkfile(struct Packet* msg)
{
	std::string filename;
	filename = msg->GetArg<std::string>(NET_MKFILE_PATH);

	if(!check_filename(filename))
		throw Packet::Malformated();

	pf_stat stat;
	stat.mode = msg->GetArg<uint32_t>(NET_MKFILE_MODE);
	stat.uid = msg->GetArg<uint32_t>(NET_MKFILE_UID);
	stat.gid = msg->GetArg<uint32_t>(NET_MKFILE_GID);
	stat.size = msg->GetArg<uint64_t>(NET_MKFILE_SIZE);
	stat.atime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_ACCESS_TIME));
	stat.mtime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_MODIF_TIME));
	stat.meta_mtime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_META_MODIF_TIME));
	stat.ctime = Timestamp(msg->GetArg<uint32_t>(NET_MKFILE_CREATE_TIME));
	stat.pf_mode = msg->GetArg<uint32_t>(NET_MKFILE_PF_MODE);

	if(!check_filemode(stat.mode))
		throw Packet::Malformated();

	scheduler_queue.Queue(new JobMkFile(filename, stat, IDList(), GetID(), HasFlag(MERGING),environment.merging.Get()));
	session_cfg.Set("last_view_" + TypToStr(GetID()), time(NULL));
}

void Peer::Handle_net_end_of_merge(struct Packet* msg)
{
	environment.merging.Set(false);
	DelFlag(MERGING);
	if(IsClient())
	{
		SendMsg(Packet(NET_END_OF_MERGE_ACK));
		session_cfg.Set("last_view_" + TypToStr(GetID()), time(NULL));
	}
}

void Peer::Handle_net_end_of_merge_ack(struct Packet* msg)
{
	DelFlag(MERGING);
}

void Peer::Handle_net_who_has_file(struct Packet* msg)
{
	std::string filename;
	filename = msg->GetArg<std::string>(NET_WHO_HAS_FILE_PATH);

	if(!check_filename(filename))
		throw Packet::Malformated();

	scheduler_queue.Queue(new JobAdvertiseFile(filename, GetID()));
}

void Peer::Handle_net_i_have_file(struct Packet* msg)
{
	std::string filename;
	filename = msg->GetArg<std::string>(NET_I_HAVE_FILE_PATH);

	if(!check_filename(filename))
		throw Packet::Malformated();

	scheduler_queue.Queue(new JobSetSharer(filename, GetID()));
}

void Peer::Handle_net_want_ref_file(struct Packet* msg)
{
	std::string filename;
	filename = msg->GetArg<std::string>(NET_WANT_REF_FILE_PATH);

	if(!check_filename(filename))
		throw Packet::Malformated();

	scheduler_queue.Queue(new JobSendRefFile(filename, GetID()));
}

void Peer::Handle_net_ref_file(struct Packet* msg)
{
	std::string filename;
	filename = msg->GetArg<std::string>(NET_REF_FILE_PATH);
	uint32_t ref = msg->GetArg<uint32_t>(NET_REF_FILE_REF);
	off_t offset = (off_t)msg->GetArg<uint64_t>(NET_REF_FILE_OFFSET);
	off_t size = (size_t)msg->GetArg<uint64_t>(NET_REF_FILE_SIZE);

	if(!check_filename(filename))
		throw Packet::Malformated();

	file_refs.insert(make_pair(ref, filename));
	scheduler_queue.Queue(new JobSetSharedPart(filename, GetID(), offset, size));
}

void Peer::Handle_net_unref_file(struct Packet* msg)
{
	uint32_t ref = msg->GetArg<uint32_t>(NET_UNREF_FILE_REF);

	std::map<uint32_t, std::string>::iterator it;

	if((it = file_refs.find(ref)) != file_refs.end())
	{
		// If we was downloading it, erase the already resend the already
		// sent chunk requests:
		ResendAskedChunks(ref);

		log[W_DEBUG] << "Peer " << GetID() << " unreferenced " << it->second;
		file_refs.erase(it);
	}
}

void Peer::Handle_net_refresh_ref_file(struct Packet* msg)
{
	uint32_t ref = msg->GetArg<uint32_t>(NET_REFRESH_REF_FILE_REF);
	off_t offset = (off_t)msg->GetArg<uint64_t>(NET_REFRESH_REF_FILE_OFFSET);
	off_t size = (size_t)msg->GetArg<uint64_t>(NET_REFRESH_REF_FILE_SIZE);

	std::map<uint32_t, std::string>::iterator it;
	if((it = file_refs.find(ref)) == file_refs.end())
		return;

	scheduler_queue.Queue(new JobSetSharedPart(it->second, GetID(), offset, size));
}

void Peer::Handle_net_want_chunk(struct Packet* msg)
{
	uint32_t ref = msg->GetArg<uint32_t>(NET_WANT_CHUNK_REF);
	off_t offset = (off_t)msg->GetArg<uint64_t>(NET_WANT_CHUNK_OFFSET);
	size_t size = (size_t)msg->GetArg<uint32_t>(NET_WANT_CHUNK_SIZE);
	scheduler_queue.Queue(new JobSendChunk(ref, GetID(), offset, size));
}

void Peer::Handle_net_chunk(struct Packet* msg)
{
	uint32_t ref = msg->GetArg<uint32_t>(NET_CHUNK_REF);

	std::map<uint32_t, std::string>::iterator it;
	if((it = file_refs.find(ref)) == file_refs.end())
		return;

	FileChunk chunk = msg->GetArg<FileChunk>(NET_CHUNK_CHUNK);

	DelAskedChunk(ref, chunk);
	scheduler_queue.Queue(new JobSetChunk(it->second, chunk));
}

void Peer::HandleMsg(Packet* pckt)
{
	void (Peer::*handler[NET_NB_MESSAGES]) (Packet*) =
	{
		NULL,
		&Peer::Handle_net_hello,
		&Peer::Handle_net_your_id,
		&Peer::Handle_net_start_merge,
		&Peer::Handle_net_get_struct_diff,
		&Peer::Handle_net_mkfile,
		&Peer::Handle_net_peer_connection,
		&Peer::Handle_net_peer_connection_ack,
		&Peer::Handle_net_peer_connection_rst,
		&Peer::Handle_net_peer_connection_rejected,
		&Peer::Handle_net_peer_all_connected,
		&Peer::Handle_net_end_of_diff,
		&Peer::Handle_net_end_of_merge,
		&Peer::Handle_net_end_of_merge_ack,
		&Peer::Handle_net_who_has_file,
		&Peer::Handle_net_i_have_file,
		&Peer::Handle_net_want_ref_file,
		&Peer::Handle_net_ref_file,
		&Peer::Handle_net_refresh_ref_file,
		&Peer::Handle_net_unref_file,
		&Peer::Handle_net_want_chunk,
		&Peer::Handle_net_chunk,
	};

	/* Note that we can safely cast pckt->type to unsigned after check pkct->type > 0 */
	if(pckt->GetType() <= 0)
		throw Packet::Malformated();
	if((size_t)pckt->GetType() >= (sizeof handler/ sizeof *handler))
		throw Packet::Malformated();
	if((size_t)pckt->GetType() >= NET_NB_MESSAGES)
		throw Packet::Malformated();

	(this->*handler[pckt->GetType()])(pckt);
}

bool Peer::Receive()
{
	if(!PeerBase::ReceivePacket())
		return false;

	/* We use the Deleter class because we don't know how we will
	 *  * exit this function. With it, we are *sure* than Packet instance
	 *   * will be free'd.
	 *    */
	Deleter<Packet> packet(incoming);
	incoming = NULL;

	HandleMsg(*packet);

	return false;
}
