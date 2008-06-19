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
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * $Id$
 */

#include "pf_log.h"
#include "peer_base.h"
#include <algorithm>

PeerBase::PeerBase(pf_addr _addr, Connection* _conn, unsigned int _flags) :
			addr(_addr),
			conn(_conn),
			ts_diff(0),
			incoming(NULL),
			flags(_flags)
{
}

PeerBase::~PeerBase()
{
	if(incoming)
		delete incoming;
	if(conn)
		delete conn;
}

void PeerBase::SetTimestampDiff(uint32_t now)
{
	uint32_t local = (uint32_t)time(NULL);

	if(local > now)
		ts_diff = local - now;
	else
		ts_diff = - (int)(now - local);

	pf_log[W_INFO] << " Peer TS Diff: " << ts_diff << "(" << local << " - " << now << ")";
}

time_t PeerBase::Timestamp(time_t ts) const
{
	if(ts_diff < 0 && ts < (int)(-ts_diff))
		return 0;

	return ts + ts_diff;
}

void PeerBase::Flush()
{
	assert(conn);
	while(!send_queue.empty())
	{
		send_queue.front().Send(conn);
		send_queue.pop();
	}
	conn->SocketWrite();			  /* Flush any pending data */
}

// true when there is still data available, false elsewise
bool PeerBase::ReceivePacket()
{
	// Receive the header
	if(!incoming)
	{
		/* This is a new packet, we only receive the header */
		char* header;

		if(!conn->Read(&header, Packet::GetHeaderSize()))
			return false;		  // All the content couldn't be retrieved yet -> exit

		incoming = new Packet(header);
		free(header);			  // Don't use delete []
	}

	// Continue receiving the packet
	if(incoming->GetDataSize() > 0 && !incoming->ReceiveContent(conn))
		return false;			  // All the content couldn't be retrieved yet -> exit

	pf_log[W_PARSE] << "<- (" << GetFd() << "/" << GetID() << ") " << incoming->GetPacketInfo();

	return true;
}

void PeerBase::AddAskedChunk(uint32_t ref, FileChunkDesc chunk)
{
	std::map<uint32_t, std::list<FileChunkDesc> >::iterator asked_it;
	if((asked_it = asked_chunks.find(ref)) == asked_chunks.end())
	{
		std::list<FileChunkDesc> lst;
		asked_chunks.insert(make_pair(ref, lst));
		asked_it = asked_chunks.find(ref);
		if(asked_it == asked_chunks.end())
			return;
	}

	asked_it->second.push_back(chunk);
}

void PeerBase::DelAskedChunk(uint32_t ref, FileChunkDesc chunk)
{
	std::map<uint32_t, std::list<FileChunkDesc> >::iterator asked_it;
	if((asked_it = asked_chunks.find(ref)) != asked_chunks.end())
	{
		std::list<FileChunkDesc>::iterator chunk_it;
		if((chunk_it = std::find(asked_it->second.begin(),
		                         asked_it->second.end(),
		                         chunk)) != asked_it->second.end())
		{
			asked_it->second.erase(chunk_it);
		}
		else
		{
			pf_log[W_ERR] << "Chunk wasn't asked ??";
		}
	}
}

void PeerBase::ResendAskedChunks(uint32_t ref)
{
	std::map<uint32_t, std::string>::iterator ref_it;
	if((ref_it = file_refs.find(ref)) == file_refs.end())
		return;

	std::string filename = ref_it->second;

	std::map<uint32_t, std::list<FileChunkDesc> >::iterator asked_it;
	if((asked_it = asked_chunks.find(ref)) != asked_chunks.end())
	{
		for(std::list<FileChunkDesc>::iterator chunk_it = asked_it->second.begin();
				chunk_it != asked_it->second.end();
				++chunk_it)
		{
			//scheduler_queue.Queue(new JobResendChunkReq(filename, *chunk_it));
		}
	}
}
