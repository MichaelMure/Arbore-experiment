#include "log.h"
#include "peer_base.h"

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

void PeerBase::Flush()
{
	while(!send_queue.empty())
	{
		send_queue.front().Send(conn);
		send_queue.pop();
	}
}

// true when there is still data available, false elsewise
bool PeerBase::Receive()
{
	// Receive the header
	if(!incoming)
	{
		/* This is a new packet, we only receive the header */
		char* header;

		if(!conn->Read(&header, Packet::GetHeaderSize()))
			return false;		  // All the content couldn't be retrieved yet -> exit

		incoming = new Packet(header);
		delete []header;

		log[W_PARSE] << "Received a message header: type=" << incoming->GetType() << ", " <<
			" size=" << incoming->GetDataSize();

	}

	// Continue receiving the packet
	if(incoming->GetDataSize() > 0 && !incoming->ReceiveContent(conn))
		return false;			  // All the content couldn't be retrieved yet -> exit

	log[W_PARSE] << "<- (" << GetFd() << "/" << GetID() << ") " << incoming->GetPacketInfo();

	/* We use the Deleter class because we don't know how we will
	 * exit this function. With it, we are *sure* than Packet instance
	 * will be free'd.
	 */
	Deleter<Packet> packet(incoming);
	incoming = NULL;

	HandleMsg(*packet);
	return true;
}

