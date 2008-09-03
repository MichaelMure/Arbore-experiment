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
 *
 */

#ifndef PACKET_H
#define PACKET_H

#include <vector>
#include <cassert>
#include <string>
#include "pf_types.h"
#include "packet_arg.h"
#include "files/file_chunk.h"
#include "pf_addr.h"
#include "net_proto.h"

typedef std::vector<pf_addr> AddrList;

class Packet
{
	std::vector<PacketArgBase*> arg_lst;

	virtual char* DumpBuffer() const = 0;

	void BuildArgsFromData();
	void BuildDataFromArgs();

	// Writing to buffer functions
	Packet& Write(uint32_t nbr);
	Packet& Write(uint64_t nbr);
	Packet& Write(Key nbr);
	Packet& Write(pf_addr addr);
	Packet& Write(const std::string& str);
	Packet& Write(const AddrList& addr_list);
	Packet& Write(FileChunk chunk);

	// Reading from buffer functions
	uint32_t ReadInt32();
	uint64_t ReadInt64();
	Key ReadKey();
	pf_addr ReadAddr();
	std::string ReadStr();
	AddrList ReadAddrList();
	FileChunk ReadChunk();

protected:
	msg_type type;
	uint32_t size;			  // size of the msg (excluding header)
	Key src;
	Key dst;
	char* datas;

public:

	/* Exceptions */
	class Malformated : public std::exception {};

	/* Constructors */
	Packet(msg_type _type, const Key& src = Key(), const Key& dst = Key());
	Packet(const Packet& packet);
	Packet(char* data);
	Packet& operator=(const Packet& packet);
	virtual ~Packet();

	static uint32_t GetHeaderSize();
	uint32_t GetSize() const;
	uint32_t GetDataSize() const { return size; }
	msg_type GetType() const { return type; }

	Key GetSrc() const { return src; }
	Key GetDst() const { return dst; }
	Packet& SetSrc(Key id) { src = id; return *this; }
	Packet& SetDst(Key id) { dst = id; return *this; }

	template<typename T>
		void SetArg(size_t arg, T val)
	{
		if(arg_lst.size() <= arg)
			arg_lst.resize(arg + 1, NULL);
		if(arg_lst[arg] != NULL)
			delete arg_lst[arg];

		arg_lst[arg] = new PacketArg<T>(val);
	}

	virtual std::string GetPacketInfo() const;

	template<typename T>
		T GetArg(size_t arg) const
	{
		assert(arg_lst.size() > arg);
		assert(arg_lst[arg] != NULL);
		assert(dynamic_cast< PacketArg<T>* >(arg_lst[arg]));

		return (dynamic_cast< PacketArg<T>* >(arg_lst[arg]))->val;
	}
};
#endif						  /* PACKET_H */
