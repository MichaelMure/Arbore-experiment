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

#include <assert.h>
#include <stdio.h>
#include <netinet/in.h>				  // htonl, ntohl
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include "packet.h"
#include "packet_arg.h"
#include "dht/key.h"
#include "net_proto.h"
#include "tools.h"

#ifdef DEBUG
#define ASSERT assert
#else
#define ASSERT(x) if(!(x)) throw Malformated();
#endif

Packet::Packet(msg_type _type, const Key& _src, const Key& _dst)
			: type(_type),
			size(0),
			src(_src),
			dst(_dst),
			datas(NULL)
{
}

Packet::Packet(const Packet& p)
			: type(p.type),
			size(p.size),
			datas(NULL)
{
	if(size)
	{
		datas = new char [size];
		memcpy(datas, p.datas, size);
	}
	for(std::vector<PacketArgBase*>::const_iterator it = p.arg_lst.begin(); it != p.arg_lst.end(); ++it)
		arg_lst.push_back((*it)->clone());
}

Packet::Packet(char* header)
			: type(NET_NONE),
			size(0),
			datas(NULL)
{
	uint32_t* h = (uint32_t*)header;
	src = ntohl(h[0]);
	dst = ntohl(h[1]);
	type = (msg_type)ntohl(h[2]);
	size = ntohl(h[3]);

	datas = new char [size];
}

char* Packet::DumpBuffer() const
{
	char* dump = new char [GetSize()];
	pf_id _src = htonl(src);
	pf_id _dst = htonl(dst);
	uint32_t _type = htonl(type);
	uint32_t _size = htonl(size);
	char* ptr = dump;
	memcpy(ptr, &_src, sizeof(_src));
	ptr += sizeof _src;
	memcpy(ptr, &_dst, sizeof(_dst));
	ptr += sizeof _dst;
	memcpy(ptr, &_type, sizeof(_type));
	ptr += sizeof _type;
	memcpy(ptr, &_size, sizeof(_size));
	ptr += sizeof _size;
	memcpy(ptr, datas, GetDataSize());
	return dump;
}

uint32_t Packet::GetHeaderSize()
{
	return    sizeof(Key::nlen * sizeof(uint32_t))	// src
		+ sizeof(Key::nlen * sizeof(uint32_t))	// dst
		+ sizeof(uint32_t)		        // size of the packet
		+ sizeof(uint32_t);		        // size of the type
}

uint32_t Packet::GetSize() const
{
	return size + GetHeaderSize();
}


Packet& Packet::operator=(const Packet& p)
{
	type = p.type;
	size = p.size;

	if(datas)
		delete []datas;

	if(size)
	{
		datas = new char [size];
		memcpy(datas, p.datas, size);
	}
	else
		datas = NULL;

	for(std::vector<PacketArgBase*>::const_iterator it = p.arg_lst.begin(); it != p.arg_lst.end(); ++it)
		arg_lst.push_back((*it)->clone());

	return *this;
}

Packet::~Packet()
{
	if(datas)
		delete []datas;

	for(std::vector<PacketArgBase*>::iterator it = arg_lst.begin(); it != arg_lst.end(); ++it)
		delete *it;
}

#if 0
bool Packet::ReceiveContent(Connection* conn)
{
	char* buf;
	if(!conn->Read(&buf, GetDataSize()))
		return false;

	memcpy(datas, buf, GetDataSize());
	free(buf);				  // Don't use delete[]

	BuildArgsFromData();
	return true;
}

void Packet::Send(Connection* conn)
{
	BuildDataFromArgs();
	char* buf = DumpBuffer();
	conn->Write(buf, GetSize());
	delete []buf;
}

#endif

void Packet::BuildArgsFromData()
{
	for(size_t arg_no = 0; packet_args[type][arg_no] != T_NONE; ++arg_no)
		switch(packet_args[type][arg_no])
		{
			case T_UINT32: SetArg(arg_no, ReadInt32()); break;
			case T_UINT64: SetArg(arg_no, ReadInt64()); break;
			case T_STR: SetArg(arg_no, ReadStr()); break;
			case T_ADDRLIST: SetArg(arg_no, ReadAddrList()); break;
			case T_ADDR: SetArg(arg_no, ReadAddr()); break;
			case T_CHUNK: SetArg(arg_no, ReadChunk()); break;
			default: throw Malformated();
	}
}

void Packet::BuildDataFromArgs()
{
	for(size_t arg_no = 0; packet_args[type][arg_no] != T_NONE; ++arg_no)
		switch(packet_args[type][arg_no])
		{
			case T_UINT32: Write(GetArg<uint32_t>(arg_no)); break;
			case T_UINT64: Write(GetArg<uint64_t>(arg_no)); break;
			case T_STR: Write(GetArg<std::string>(arg_no)); break;
			case T_ADDRLIST: Write(GetArg<AddrList>(arg_no)); break;
			case T_ADDR: Write(GetArg<pf_addr>(arg_no)); break;
			case T_CHUNK: Write(GetArg<FileChunk>(arg_no)); break;
			default: throw Malformated();
	}
}

std::string Packet::GetPacketInfo() const
{
	std::string s, info;

	info = "[" + GetSrc().str();
	info += "->" + GetDst().str() + "] ";

	info = "<" + std::string(type2str[GetType()]) + "> ";

	for(size_t arg_no = 0; packet_args[type][arg_no] != T_NONE; ++arg_no)
	{
		if(s.empty() == false)
			s += ", ";
		switch(packet_args[type][arg_no])
		{
			case T_UINT32: s += TypToStr(GetArg<uint32_t>(arg_no)); break;
			case T_UINT64: s += TypToStr(GetArg<uint64_t>(arg_no)); break;
			case T_STR: s += "'" + GetArg<std::string>(arg_no) + "'"; break;
			case T_ADDRLIST:
			{
				AddrList v = GetArg<AddrList>(arg_no);
				std::string list;
				for(AddrList::const_iterator it = v.begin();
					it != v.end();
					++it)
				{
					if(!list.empty()) list += ",";
					list += pf_addr2string(*it);
				}
				s += "[" + list + "]";
				break;
			}
			case T_ADDR: s += pf_addr2string(GetArg<pf_addr>(arg_no)); break;
			case T_CHUNK:
				s += "ch(off:" + TypToStr(GetArg<FileChunk>(arg_no).GetOffset())
					+ " size:" +  TypToStr(GetArg<FileChunk>(arg_no).GetSize()) + ")";
				break;
			default: throw Malformated();
		}
	}

	info += s;
	return info;
}
/* T_UINT32 */
uint32_t Packet::ReadInt32()
{
	ASSERT(size >= sizeof(uint32_t));
	uint32_t val = ntohl(*(uint32_t*)datas);

	char* new_datas = NULL;
	size -= (uint32_t)sizeof(uint32_t);
	if(size > 0)
	{
		new_datas = new char [size];
		memcpy(new_datas, datas + sizeof(uint32_t), size);
	}

	delete []datas;

	if(size > 0)
		datas = new_datas;
	else
		datas = NULL;

	return val;
}


Packet& Packet::Write(uint32_t nbr)
{
	ASSERT(((uint32_t)sizeof(nbr)) + size >= size);

	char* new_datas = new char [size + sizeof nbr];

	nbr = htonl(nbr);
	if(datas)
		memcpy(new_datas, datas, size);
	if(datas)
		delete []datas;
	memcpy(new_datas + size, &nbr, sizeof(nbr));
	size += (uint32_t)sizeof(nbr);
	datas = new_datas;

	return *this;
}

/* T_UINT64 */
uint64_t Packet::ReadInt64()
{
	ASSERT(size >= sizeof(uint64_t));
	uint64_t val = ntohll(*(uint64_t*)datas);

	char* new_datas;
	size -= (uint32_t)sizeof(uint64_t);
	if(size > 0)
	{
		new_datas = new char [size];
		memcpy(new_datas, datas + sizeof(uint64_t), size);
	}

	delete []datas;

	if(size > 0)
		datas = new_datas;
	else
		datas = NULL;

	return val;
}

Packet& Packet::Write(uint64_t nbr)
{
	ASSERT(((uint32_t)sizeof(nbr)) + size >= size);

	char* new_datas = new char [size + sizeof nbr];

	nbr = htonll(nbr);
	if(datas)
		memcpy(new_datas, datas, size);
	if(datas)
		delete []datas;
	memcpy(new_datas + size, &nbr, sizeof(nbr));
	size += (uint32_t)sizeof(nbr);
	datas = new_datas;

	return *this;
}

/* T_ADDR */
pf_addr Packet::ReadAddr()
{
	ASSERT(size >= sizeof(pf_addr));
	pf_addr val = nto_pf_addr(*(pf_addr*)datas);

	char* new_datas;
	size -= (uint32_t)sizeof(pf_addr);
	if(size > 0)
	{
		new_datas = new char [size];
		memcpy(new_datas, datas + sizeof(pf_addr), size);
	}

	delete []datas;

	if(size > 0)
		datas = new_datas;
	else
		datas = NULL;

	return val;
}

Packet& Packet::Write(pf_addr addr)
{
	ASSERT(((uint32_t)sizeof(addr)) + size >= size);
	char* new_datas = new char [size + sizeof addr];

	addr = pf_addr_ton(addr);
	if(datas)
		memcpy(new_datas, datas, size);
	if(datas)
		delete []datas;
	memcpy(new_datas + size, &addr, sizeof(addr));
	size += (uint32_t)sizeof(addr);
	datas = new_datas;

	return *this;
}

/* T_ADDRLIST */
AddrList Packet::ReadAddrList()
{
	AddrList addr_list;
	uint32_t list_size = ReadInt32();
	ASSERT((size * sizeof(pf_addr)) >= list_size);

	for(uint32_t i = 0; i < list_size; ++i)
	{
		pf_addr addr;
		memcpy(&addr, datas, sizeof(pf_addr));
		addr_list.push_back(nto_pf_addr(addr));
	}

	char* new_datas;
	size -= list_size * (uint32_t)sizeof(pf_addr);
	if(size > 0)
	{
		new_datas = new char [size];
		memcpy(new_datas, datas + (list_size * sizeof(pf_addr)), size);
	}

	delete []datas;

	if(size > 0)
		datas = new_datas;
	else
		datas = NULL;

	return addr_list;
}

Packet& Packet::Write(const AddrList& addr_list)
{
	ASSERT(addr_list.size() <= UINT_MAX);
	Write((uint32_t)addr_list.size());

	ASSERT((addr_list.size() * sizeof(pf_addr)) + size >= size);
	char* new_datas = new char [size + (addr_list.size() * sizeof(pf_addr))];

	if(datas)
		memcpy(new_datas, datas, size);
	if(datas)
		delete []datas;

	char* ptr = new_datas + size;

	for(AddrList::const_iterator it = addr_list.begin(); it != addr_list.end(); ++it)
	{
		pf_addr addr = pf_addr_ton(*it);
		memcpy(ptr, &addr, sizeof(pf_addr));
		ptr += sizeof(pf_addr);
	}
	size += (uint32_t)(addr_list.size() * sizeof(pf_addr));
	datas = new_datas;

	return *this;
}

/* T_STR */
std::string Packet::ReadStr()
{
	uint32_t str_size = ReadInt32();
	ASSERT(size >= str_size);
	char* str = new char [str_size+1];

	memcpy(str, datas, str_size);
	str[str_size] = '\0';
	std::string val = std::string(str);

	char* new_datas;
	size -= str_size;
	if(size > 0)
	{
		new_datas = new char [size];
		memcpy(new_datas, datas + str_size, size);
	}

	delete []datas;
	delete []str;

	if(size > 0)
		datas = new_datas;
	else
		datas = NULL;

	return val;
}

Packet& Packet::Write(const std::string& str)
{
	ASSERT(str.size() <= UINT_MAX);

	uint32_t str_len = (uint32_t)str.size();
	ASSERT(str_len + size >= size);

	Write(str_len);

	char* new_datas = new char [size + str_len];
	if(datas)
		memcpy(new_datas, datas, size);
	if(datas)
		delete []datas;

	memcpy(new_datas + size, str.c_str(), str_len);
	size += str_len;
	datas = new_datas;

	return *this;
}

/* T_CHUNK */
FileChunk Packet::ReadChunk()
{
	off_t c_offset = ReadInt64();
	uint32_t c_size = ReadInt32();

	FileChunk chunk(datas, c_offset, c_size);

	char* new_datas;
	size -= c_size;
	if(size > 0)
	{
		new_datas = new char [size];
		memcpy(new_datas, datas + c_size, size);
	}

	delete []datas;

	if(size > 0)
		datas = new_datas;
	else
		datas = NULL;

	return chunk;
}

Packet& Packet::Write(FileChunk chunk)
{
	ASSERT(chunk.GetSize() <= UINT_MAX);
	ASSERT(((uint32_t)sizeof(chunk.GetOffset()) + ((uint32_t)sizeof(chunk.GetSize())) + chunk.GetSize()) + size >= size);
	Write((uint64_t)chunk.GetOffset());
	Write((uint32_t)chunk.GetSize());

	char* new_datas = new char [size + chunk.GetSize()];

	if(datas)
		memcpy(new_datas, datas, size);
	if(datas)
		delete []datas;

	char* ptr = new_datas + size;
	memcpy(ptr, chunk.GetData(), chunk.GetSize());

	size += (uint32_t)chunk.GetSize();
	datas = new_datas;

	return *this;
}



