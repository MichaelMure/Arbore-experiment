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

#ifndef PACKET_BASE_H
#define PACKET_BASE_H

#include <vector>
#include <cassert>
#include <string>
#include "pf_types.h"
#include "net_proto.h"
#include "packet_arg.h"

class PacketBase
{
	std::vector<PacketArgBase*> arg_lst;

	virtual char* DumpBuffer() const;

	void BuildArgsFromData();
	void BuildDataFromArgs();

	// Writing to buffer functions
	PacketBase& Write(uint32_t nbr);
	PacketBase& Write(uint64_t nbr);
	PacketBase& Write(pf_addr addr);
	PacketBase& Write(std::string str);
	PacketBase& Write(AddrList addr_list);

	// Reading from buffer functions
	uint32_t ReadInt32();
	uint64_t ReadInt64();
	pf_addr ReadAddr();
	std::string ReadStr();
	AddrList ReadAddrList();

protected:
	msg_type type;
	uint32_t size;			  // size of the msg (excluding header)
	char* datas;

public:

	/* Exceptions */
	class RecvError : public std::exception {};
	class Malformated : public std::exception {};

	/* Constructors */
	PacketBase(msg_type _type);
	PacketBase(const PacketBase& packet);
	PacketBase& operator=(const PacketBase& packet);
	virtual ~PacketBase();

	static uint32_t GetHeaderSize();
	virtual uint32_t GetSize() const;
	uint32_t GetDataSize() const { return size; }
	msg_type GetType() const { return type; }

	/** Send packet to a peer.
	 *
	 * @param fd file descriptor of socket
	 */
	virtual void Send(int fd);

	/** Receive content of packet from a peer.
	 *
	 * @param fd file descriptor of socket
	 */
	void ReceiveContent(int fd) throw(RecvError, Malformated);

	template<typename T>
		PacketBase& SetArg(size_t arg, T val)
	{
		if(arg_lst.size() <= arg)
			arg_lst.resize(arg + 1, NULL);
		if(arg_lst[arg] != NULL)
			delete arg_lst[arg];

		arg_lst[arg] = new PacketArg<T>(val);

		return *this;
	}

	template<typename T>
		T GetArg(size_t arg)
	{
		assert(arg_lst.size() > arg);
		assert(arg_lst[arg] != NULL);
		assert(dynamic_cast< PacketArg<T>* >(arg_lst[arg]));

		return (dynamic_cast< PacketArg<T>* >(arg_lst[arg]))->val;
	}
};
#endif						  /* PACKET_BASE_H */
